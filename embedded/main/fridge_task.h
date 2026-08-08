#ifndef FRIDGE_TASK_H
#define FRIDGE_TASK_H

#include "ad7705.h"
#include "global_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ble.h"
#include "esp_err.h"
#include "string.h"

void fridge_task(void *time_ms);

#endif