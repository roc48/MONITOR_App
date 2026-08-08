#include "fridge_task.h"
#include "driver/gpio.h"

void fridge_task(void *time_ms)
{
    printf("fridge_task start!\n");
    int d_time = (int)time_ms;
    global_status.fridge = 1;
    gpio_set_level(FRIDGE_ENABLE_PIN, HIGH);
    vTaskDelay(d_time / portTICK_PERIOD_MS);
    gpio_set_level(FRIDGE_ENABLE_PIN, LOW);
    global_status.fridge = 0;
    printf("fridge_task stop!\n");
    vTaskDelete(NULL);
}