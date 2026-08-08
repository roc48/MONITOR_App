#ifndef DYNAMIC_TASK_H
#define DYNAMIC_TASK_H

#include "ad7705.h"
#include "global_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ble.h"
#include "esp_err.h"
#include "string.h"

extern TaskHandle_t dynamic_task_handle;
void dynamic_task();

#endif