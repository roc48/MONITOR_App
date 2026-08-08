#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"
#include "driver/gpio.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "global_timer.h"
#include "global_config.h"

esp_timer_handle_t global_timer;

void global_timer_callback()
{
    global_status.global_time += 50;
}

void start_global_timer()
{
    global_status.global_time = 0;
    esp_timer_create_args_t timer0_config = {
        .callback = (esp_timer_cb_t)global_timer_callback,
    };

    esp_timer_create(&timer0_config, &global_timer);
    esp_timer_start_periodic(global_timer, 50000);
}