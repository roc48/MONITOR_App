#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_RTS (UART_PIN_NO_CHANGE)
#define SERIAL_CTS (UART_PIN_NO_CHANGE)

#define SERIAL_PORT_NUM 0
#define SERIAL_BAUD_RATE 115200
#define SERIAL_STACK_SIZE 2048
#define SERIAL_BUFFER_SIZE 1024


void serial_init_and_start(void);

#endif