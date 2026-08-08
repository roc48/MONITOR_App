#ifndef BLE_H
#define BLE_H

#include <string.h>
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"


/// Declare the static function
void gatts_profile_temperature_data_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void gatts_profile_device_status_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

#define GATTS_SERVICE_UUID_TEMPERATURE_DATA 0x00FF
#define GATTS_CHAR_UUID_TEMPERATURE_DATA 0xFF01
#define GATTS_DESCR_UUID_TEMPERATURE_DATA 0x3333
#define GATTS_NUM_HANDLE_TEMPERATURE_DATA 4

#define GATTS_SERVICE_UUID_TEST_B 0x00EE
#define GATTS_CHAR_UUID_TEST_B 0xEE01
#define GATTS_DESCR_UUID_TEST_B 0x2222
#define GATTS_NUM_HANDLE_TEST_B 4

#define TEST_DEVICE_NAME "ADNBox 8227Q (QS)"
#define TEST_MANUFACTURER_DATA_LEN 17

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40

#define PREPARE_BUF_MAX_SIZE 1024

#define PROFILE_NUM 2
#define PROFILE_TEMPERATURE_DATA_APP_ID 0
#define PROFILE_DEVICE_STATUS_APP_ID 1

typedef struct 
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
}gatts_profile_inst,*pgatts_profile_inst;

extern gatts_profile_inst gl_profile_tab[]; 

typedef struct
{
    uint8_t *prepare_buf;
    int prepare_len;
} prepare_type_env_t;


extern int temperature_data_notify_flag;

extern float pos_th;
int ble_init();

#endif