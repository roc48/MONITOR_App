#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"
#include "driver/gpio.h"

#include "global_config.h"
#include "ad7705.h"

void ad7705_set_pin(PAD7705 ad7705,int cs_pin,int sck_pin,int dn_pin,int dout_pin){
    ad7705->CS_pin = cs_pin;
    ad7705->SCK_pin = sck_pin;
    ad7705->DN_pin = dn_pin;
    ad7705->DOUT_pin = dout_pin;
}

void ad7705_spi_write(PAD7705 ad7705, unsigned char data)
{
    for (int i = 7; i >= 0; --i)
    {
        if (data & 0x80)
        {
            gpio_set_level(ad7705->DN_pin, HIGH);
        }
        else
        {
            gpio_set_level(ad7705->DN_pin, LOW);
        }

        gpio_set_level(ad7705->SCK_pin, LOW);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        data <<= 1;
        gpio_set_level(ad7705->SCK_pin, HIGH);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

unsigned short ad7705_spi_read(PAD7705 ad7705)
{
    unsigned short value = 0;
    for (int i = 15; i >= 0; --i)
    {
        gpio_set_level(ad7705->SCK_pin, LOW);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        if (gpio_get_level(ad7705->DOUT_pin))
        {
            value |= (1 << i);
        }
        gpio_set_level(ad7705->SCK_pin, HIGH);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    return value;
}


unsigned short ad7705_read_value(PAD7705 ad7705,unsigned char ch)
{
    unsigned short value;
    gpio_set_level(ad7705->CS_pin, LOW);
    if (ch == 1)
    {
        ad7705_spi_write(ad7705,0x38);
    }
    else if (ch == 2)
    {
        ad7705_spi_write(ad7705,0x39);
    }
    else
    {
        gpio_set_level(ad7705->CS_pin, HIGH);
        return 0;
    }
    gpio_set_level(ad7705->CS_pin, HIGH);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(ad7705->CS_pin, LOW);
    value = ad7705_spi_read(ad7705);
    gpio_set_level(ad7705->CS_pin, HIGH);
    return value;
}

void ad7705_calibration(PAD7705 ad7705){
    gpio_set_level(ad7705->CS_pin, LOW);
    ad7705_spi_write(ad7705, 0xff);
    ad7705_spi_write(ad7705, 0xff);
    ad7705_spi_write(ad7705, 0xff);
    ad7705_spi_write(ad7705, 0xff);
    gpio_set_level(ad7705->CS_pin, HIGH);
    vTaskDelay(2 / portTICK_PERIOD_MS);

	//Clock Register
 	//   7      6       5        4        3        2      1      0
	//ZERO(0) ZERO(0) ZERO(0) CLKDIS(0) CLKDIV(0) CLK(1) FS1(0) FS0(1)
	//    0     0       0         										000 : keep them always low
	//					   		 0       								0 :clock disable is false
	//                                    1                             1 = clock division (should be 1 with 4.19Mhz)
	//                                              1                   1 if freq > 2Mhz
	// 													  1       1     filter : 00 =50Hz  01=60hz 10=250hz 11=500hz
	
    gpio_set_level(ad7705->CS_pin, LOW);
    ad7705_spi_write(ad7705, REG_CLOCK|WRITE|CH_1);
    gpio_set_level(ad7705->CS_pin, HIGH);
    vTaskDelay(20 / portTICK_PERIOD_MS);

    gpio_set_level(ad7705->CS_pin, LOW);
    ad7705_spi_write(ad7705, 0b00001111);
    gpio_set_level(ad7705->CS_pin, HIGH);
    vTaskDelay(20 / portTICK_PERIOD_MS);

	//Setup Register
	//  7     6     5     4     3      2      1      0
	//MD10) MD0(0) G2(0) G1(0) G0(0) B/U(0) BUF(0) FSYNC(1)
	//  0     0                                              01= self calibration 00 = normal
	//  		    0     0     0                            000 = Gain1 
	//                                  1                    1=unipolar                               
	//                                        0              0=no buff or 1 buff
	//                                               0       0=running

    gpio_set_level(ad7705->CS_pin, LOW);
    ad7705_spi_write(ad7705, REG_SETUP|WRITE|CH_1);
    gpio_set_level(ad7705->CS_pin, HIGH);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    
    gpio_set_level(ad7705->CS_pin, LOW);
    ad7705_spi_write(ad7705, 0b01000110);
    gpio_set_level(ad7705->CS_pin, HIGH);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    
    
    gpio_set_level(ad7705->CS_pin, LOW);
    ad7705_spi_write(ad7705, REG_SETUP|WRITE|CH_1);
    gpio_set_level(ad7705->CS_pin, HIGH);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    
    gpio_set_level(ad7705->CS_pin, LOW);
    ad7705_spi_write(ad7705, 0b00000110);
    gpio_set_level(ad7705->CS_pin, HIGH);
    vTaskDelay(20 / portTICK_PERIOD_MS);
}


void ad7705_init(PAD7705 ad7705)
{

    gpio_reset_pin(ad7705->SCK_pin);
    gpio_reset_pin(ad7705->CS_pin);
    gpio_reset_pin(ad7705->DOUT_pin);
    gpio_reset_pin(ad7705->DN_pin);
    gpio_set_direction(ad7705->SCK_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(ad7705->CS_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(ad7705->DOUT_pin, GPIO_MODE_INPUT);
    gpio_set_direction(ad7705->DN_pin, GPIO_MODE_OUTPUT);

    ad7705_calibration(ad7705);
}