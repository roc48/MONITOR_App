#ifndef AD7705_H
#define AD7705_H

#define	REG_COMM	 0x00
#define	REG_SETUP	 0x10
#define	REG_CLOCK	 0x20
#define	REG_DATA	 0x30
#define	REG_ZERO_CH1	 0x60
#define	REG_FULL_CH1	 0x70
#define	REG_ZERO_CH2	 0x61
#define	REG_FULL_CH2	 0x71

#define	WRITE 		 0x00
#define	READ 		 0x08
#define	CH_1		 0
#define	CH_2		 1

typedef struct ad7705
{
    int SCK_pin;
	int CS_pin;
	int DN_pin;
	int DOUT_pin;
}AD7705,*PAD7705;

void ad7705_set_pin(PAD7705 ad7705,int cs_pin,int sck_pin,int dn_pin,int dout_pin);
void ad7705_spi_write(PAD7705 ad7705, unsigned char data);
unsigned short ad7705_spi_read(PAD7705 ad7705);
unsigned short ad7705_read_value(PAD7705 ad7705,unsigned char ch);
void ad7705_calibration(PAD7705 ad7705);
void ad7705_init(PAD7705 ad7705);


extern AD7705 ad7705_f1;
extern AD7705 ad7705_f2;

#endif