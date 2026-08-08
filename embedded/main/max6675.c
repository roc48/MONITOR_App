#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"
#include "driver/gpio.h"

#include "global_config.h"
#include "max6675.h"

void max6675_init(PMAX6675 max6675)
{
    gpio_reset_pin(max6675->SCK_pin);
    gpio_reset_pin(max6675->CS_pin);
    gpio_reset_pin(max6675->SO_pin);
    gpio_set_direction(max6675->SCK_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(max6675->CS_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(max6675->SO_pin, GPIO_MODE_INPUT);
    gpio_set_level(max6675->CS_pin, HIGH);
}

double max6675_read_celsius(PMAX6675 max6675)
{
    gpio_set_level(max6675->CS_pin, LOW);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    int value = max6675_spi_read(max6675);
    value <<= 8;
    value |= max6675_spi_read(max6675);
    gpio_set_level(max6675->CS_pin, HIGH);
    
    printf("MAX6675 %d\n",value);
    
    if (value & 0x04)
    {
        return NAN;
    }
    value = value >> 3;

    // Bit-14 is the sign
    double temperature = (value & 0x00000FFF);

    // // Check for negative temperature
    // if (value & 0x00002000)
    // {
    //     // 2's complement operation
    //     // Invert
    //     value = ~value;
    //     // Ensure operation involves lower 13-bit only
    //     temperature = value & 0x00001FFF;
    //     // Add 1 to obtain the positive number
    //     temperature += 1;
    //     // Make temperature negative
    //     temperature *= -1;
    // }

    temperature *= 0.25;
    return temperature;
}

unsigned char max6675_spi_read(PMAX6675 max6675)
{
    unsigned char value = 0;
    for (int i = 7; i >= 0; --i)
    {
        gpio_set_level(max6675->SCK_pin, LOW);
        vTaskDelay(2 / portTICK_PERIOD_MS);

        if (gpio_get_level(max6675->SO_pin))
        {
            value |= (1 << i);
        }
        gpio_set_level(max6675->SCK_pin, HIGH);
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
    return value;
}