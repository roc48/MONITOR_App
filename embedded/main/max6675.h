#ifndef MAX6675_H
#define MAX6675_H

#define HISTORY_COUNT 120

typedef struct max6675
{
    int SCK_pin;
	int CS_pin;
	int SO_pin;
	double temp_history[HISTORY_COUNT];
}MAX6675,*PMAX6675;

void max6675_init(PMAX6675 max6675);
double  max6675_read_celsius(PMAX6675 max6675) ;
unsigned char  max6675_spi_read(PMAX6675 max6675);

#endif