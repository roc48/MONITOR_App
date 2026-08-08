/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "driver/gpio.h"
#include "global_config.h"
#include "max6675.h"
#include "heater_controller.h"
#include "ble.h"
#include "ad7705.h"
#include "serial.h"
#include "global_timer.h"

DEVICE_STATUS global_status;
HEATER_CONTROLLER controller1;
HEATER_CONTROLLER controller2;
AD7705 ad7705_f1;
AD7705 ad7705_f2;

void gpio_init()
{
    gpio_reset_pin(LASER_ENABLE_PIN);
    gpio_reset_pin(FRIDGE_ENABLE_PIN);
    gpio_reset_pin(LED_C1_PIN);
    gpio_reset_pin(LED_C2_PIN);
    gpio_reset_pin(LED_STATUS_PIN);
    gpio_set_direction(LASER_ENABLE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(FRIDGE_ENABLE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_C1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_C2_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_STATUS_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LASER_ENABLE_PIN, LOW);
    gpio_set_level(FRIDGE_ENABLE_PIN, LOW);
    gpio_set_level(LED_C1_PIN, LOW);
    gpio_set_level(LED_C2_PIN, LOW);
    gpio_set_level(LED_STATUS_PIN, HIGH);
}

void nvs_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void temperature_data_get(PHEATER_CONTROLLER controller, double temperature, int pwm)
{
    if (temperature_data_notify_flag)
    {
        TEMPERATURE_DATA_INFO info = {
            .controller_id = controller->ledc_channel_id,
            .temperature = (float)temperature,
            .pwm_value = (unsigned char)pwm,
            .global_time = global_status.global_time,
            .type = INFO_NOTIFY_TYPE_TEMPERATURE};
        esp_ble_gatts_set_attr_value(gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].char_handle, sizeof(info), (uint8_t *)&info);
        esp_ble_gatts_send_indicate(gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].gatts_if,
                                    gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].conn_id,
                                    gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].char_handle,
                                    sizeof(info), (uint8_t *)&info, false);
    }
    printf("%d\t%lf\t%d\n", controller->ledc_channel_id, temperature,pwm);
}

void app_main(void)
{

    memset(&global_status, 0, sizeof(global_status));
    global_status.heater_1_temp_set = 100;
    global_status.heater_2_temp_set = 37;

    global_status.device_model = DEVICE_MODEL;
    global_status.firmware_version = FIRMWARE_VERSION;

    gpio_init();

    nvs_init();

    ble_init();

    start_global_timer();

    memset(&controller1, 0, sizeof(HEATER_CONTROLLER));

    heater_controller_set_pins(&controller1, MAX6675_THERMOCOUPLE_1_CS_PIN, MAX6675_THERMOCOUPLE_1_SCK_PIN, MAX6675_THERMOCOUPLE_1_SO_PIN, HEATER_1_PWM);
    heater_controller_set_PID_args(&controller1, 30, 0.2, 5);
    heater_controller_set_target_temperature(&controller1, global_status.heater_1_temp_set);
    heater_controller_init(&controller1, LEDC_TIMER_0, LEDC_CHANNEL_0, (int)(OUTPUT_PWM_VALUE_MAX), OUTPUT_PWM_VALUE_MIN, temperature_data_get);

    memset(&controller2, 0, sizeof(HEATER_CONTROLLER));

    heater_controller_set_pins(&controller2, MAX6675_THERMOCOUPLE_2_CS_PIN, MAX6675_THERMOCOUPLE_2_SCK_PIN, MAX6675_THERMOCOUPLE_2_SO_PIN, HEATER_2_PWM);
    heater_controller_set_PID_args(&controller2, 30, 0.2, 5);
    heater_controller_set_target_temperature(&controller2, global_status.heater_2_temp_set);
    heater_controller_init(&controller2, LEDC_TIMER_1, LEDC_CHANNEL_1, (int)(5 * OUTPUT_PWM_VALUE_MAX / 12), OUTPUT_PWM_VALUE_MIN, temperature_data_get);

    ad7705_set_pin(&ad7705_f1, AD7705_1_CS_PIN, AD7705_1_SCK_PIN, AD7705_1_DN_PIN, AD7705_1_DOUT_PIN);
    ad7705_init(&ad7705_f1);

    ad7705_set_pin(&ad7705_f2, AD7705_2_CS_PIN, AD7705_2_SCK_PIN, AD7705_2_DN_PIN, AD7705_2_DOUT_PIN);
    ad7705_init(&ad7705_f2);

    serial_init_and_start();
    //  while (1)
    //  {
    //      double temp = max6675_read_celsius(&controller1.thermocouple);
    //      double temp2 = max6675_read_celsius(&controller2.thermocouple);
    //      printf("%f %f\n", temp,temp2);
    //      vTaskDelay(250 / portTICK_PERIOD_MS);
    //  }
    //  heater_controller_enable(&controller1);
    //  heater_controller_enable(&controller2);
}
