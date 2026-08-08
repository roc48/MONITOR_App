#include "global_config.h"
#include "serial.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "string.h"

#include "driver/ledc.h"
#include "driver/uart.h"


#include "ble.h"
#include "dynamic_task.h"
#include "fridge_task.h"

const char reply_bad_cmd[] = "Bad command\n";


static void echo_task(void *arg)
{
    char *data = (char *)malloc(SERIAL_BUFFER_SIZE);
    char *cmd = (char *)malloc(SERIAL_BUFFER_SIZE);
    memset(data, 0, SERIAL_BUFFER_SIZE);
    memset(cmd, 0, SERIAL_BUFFER_SIZE);
    int value0 = 0;

    while (1)
    {
        int len = uart_read_bytes(SERIAL_PORT_NUM, data, (SERIAL_BUFFER_SIZE - 1), 20 / portTICK_PERIOD_MS);
        if (len > 0)
        {

            int cnt = sscanf(data, "%s%d", cmd, &value0);

            if (!strcmp(cmd, "fridge") && cnt == 2)
            {
                if (global_status.fridge == 0)
                {
                    xTaskCreate(fridge_task, "fridge_task", SERIAL_STACK_SIZE, (void *)value0, 10, NULL);
                }
            }
            else if (!strcmp(cmd, "dynamic_enable"))
            {
                if (global_status.dynamic == 0)
                {
                    xTaskCreate(dynamic_task, "dynamic_task", SERIAL_STACK_SIZE, (void *)NULL, 10, &dynamic_task_handle);
                }
            }
            else if (!strcmp(cmd, "laser_disable"))
            {
                if (global_status.dynamic == 1)
                {
                    gpio_set_level(LASER_ENABLE_PIN, LOW);
                    vTaskDelete(dynamic_task_handle);
                    gpio_set_level(LASER_ENABLE_PIN, LOW);
                    global_status.laser = 0;
                    global_status.dynamic = 0;
                    printf("dynamic_task stop!\n");
                }
            }
            else
            {
                uart_write_bytes(SERIAL_PORT_NUM, reply_bad_cmd, sizeof(reply_bad_cmd) - 1);
            }
        }
    }
}

void serial_init_and_start(void)
{

    uart_config_t uart_config = {
        .baud_rate = SERIAL_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(SERIAL_PORT_NUM, SERIAL_BUFFER_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(SERIAL_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(SERIAL_PORT_NUM, IO_PIN_SERIAL_TXD, IO_PIN_SERIAL_RXD, SERIAL_RTS, SERIAL_CTS));

    xTaskCreate(echo_task, "serial_task", SERIAL_STACK_SIZE, NULL, 10, NULL);
}
