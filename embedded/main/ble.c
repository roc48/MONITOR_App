#include "ble.h"
#include "global_config.h"
#include "heater_controller.h"
#include "dynamic_task.h"
#include "fridge_task.h"

static uint8_t char1_str[] = {0x11, 0x22, 0x33};
static esp_gatt_char_prop_t temperature_data_property = 0;
static esp_gatt_char_prop_t b_property = 0;

static esp_attr_value_t gatts_demo_char1_val =
    {
        .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
        .attr_len = sizeof(char1_str),
        .attr_value = char1_str,
};

static uint8_t adv_config_done = 0;
#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static uint8_t adv_service_uuid128[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0x87,
    0x00,
    0x33,
    0xce,
    0xa9,
    0xbd,
    0x24,
    0x2c,
    0x23,
    0xb9,
    0x83,
    0x0f,
    0x3c,
    0xd4,
    0xaa,
    0x96};

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_TEMPERATURE_DATA_APP_ID] = {
        .gatts_cb = gatts_profile_temperature_data_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
    [PROFILE_DEVICE_STATUS_APP_ID] = {
        .gatts_cb = gatts_profile_device_status_event_handler, /* This demo does not implement, similar as profile A */
        .gatts_if = ESP_GATT_IF_NONE,                          /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

// The length of adv data must be less than 31 bytes
// static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
// adv data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    //.min_interval = 0x0006,
    //.max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static prepare_type_env_t temperature_data_prepare_write_env;
static prepare_type_env_t device_status_prepare_write_env;

void write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
void exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {

    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        // advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            //  printf("Advertising start failed\n");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            // printf("Advertising stop failed\n");
        }
        else
        {
            // printf("Stop adv successfully\n");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        // printf("update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
        //       param->update_conn_params.status,
        //     param->update_conn_params.min_int,
        //   param->update_conn_params.max_int,
        // param->update_conn_params.conn_int,
        // param->update_conn_params.latency,
        // param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

void write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    esp_gatt_status_t status = ESP_GATT_OK;
    if (param->write.need_rsp)
    {
        if (param->write.is_prep)
        {
            if (prepare_write_env->prepare_buf == NULL)
            {
                prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;
                if (prepare_write_env->prepare_buf == NULL)
                {
                    // printf("Gatt_server prep no mem\n");
                    status = ESP_GATT_NO_RESOURCES;
                }
            }
            else
            {
                if (param->write.offset > PREPARE_BUF_MAX_SIZE)
                {
                    status = ESP_GATT_INVALID_OFFSET;
                }
                else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE)
                {
                    status = ESP_GATT_INVALID_ATTR_LEN;
                }
            }

            esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK)
            {
                // printf("Send response error\n");
            }
            free(gatt_rsp);
            if (status != ESP_GATT_OK)
            {
                return;
            }
            memcpy(prepare_write_env->prepare_buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->prepare_len += param->write.len;
        }
        else
        {
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
}

void exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    if (prepare_write_env->prepare_buf)
    {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

int temperature_data_notify_flag = false;
int dynamic_data_notify_flag = false;

void gatts_profile_temperature_data_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        //  printf("REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEMPERATURE_DATA;

        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].service_id, GATTS_NUM_HANDLE_TEMPERATURE_DATA);
        break;
    case ESP_GATTS_READ_EVT:
    {
        // printf("GATT_READ_EVT, conn_id %d, trans_id %ld, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 0;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;
    }
    case ESP_GATTS_WRITE_EVT:
    {
        //  printf("TNN_GATT_WRITE_EVT, conn_id %d, trans_id %ld, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
        if (!param->write.is_prep)
        {
            // printf("TNN_GATT_WRITE_EVT, value len %d, value :", param->write.len);
            if (gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].descr_handle == param->write.handle && param->write.len == 2)
            {
                uint16_t descr_value = param->write.value[1] << 8 | param->write.value[0];
                if (descr_value == 0x0001)
                {
                    if (temperature_data_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY)
                    {
                        //   printf("notify enable");
                        temperature_data_notify_flag = true;
                        dynamic_data_notify_flag = true;
                    }
                }
                else if (descr_value == 0x0002)
                {
                    //  printf("indicate enable");
                }
                else if (descr_value == 0x0000)
                {
                    //  printf("notify/indicate disable ");
                    temperature_data_notify_flag = false;
                    dynamic_data_notify_flag = false;
                }
                else
                {
                    // printf("unknown descr value");
                }
            }
        }
        write_event_env(gatts_if, &temperature_data_prepare_write_env, param);
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT:
        // printf("ESP_GATTS_EXEC_WRITE_EVT");
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        exec_write_event_env(&temperature_data_prepare_write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        //  printf("ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT:
        //  printf("CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEMPERATURE_DATA;

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].service_handle);
        temperature_data_property = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].service_handle, &gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].char_uuid,
                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                        temperature_data_property,
                                                        &gatts_demo_char1_val, NULL);
        if (add_char_ret)
        {
            //   printf("add char failed, error code =%x", add_char_ret);
        }
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
    {
        uint16_t length = 0;
        const uint8_t *prf_char;

        //  printf("ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d\n",
        //         param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle, &length, &prf_char);
        if (get_attr_ret == ESP_FAIL)
        {
            //  printf("ILLEGAL HANDLE");
        }

        esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].service_handle, &gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].descr_uuid,
                                                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
        if (add_descr_ret)
        {
            // printf("add char descr failed, error code =%x", add_descr_ret);
        }
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        // printf("ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
        //       param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        // printf("SERVICE_START_EVT, status %d, service_handle %d\n",
        //       param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONNECT_EVT:
    {
        global_status.ble = 1;
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20; // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10; // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;  // timeout = 400*10ms = 4000ms
                                    // printf("ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
        //       param->connect.conn_id,
        //      param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
        //    param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].conn_id = param->connect.conn_id;
        // start sent the update connection parameters to the peer device.
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
        global_status.ble = 0;
        // printf("ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GATTS_CONF_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}

void device_control(PDEVICE_CONTROL_INFO ctl_info)
{

    switch (ctl_info->target)
    {
    case DEVICE_CONTROL_HEATER1_ENABLE:
        if (ctl_info->value == 0)
        {
            heater_controller_disable(&controller1);
        }
        else
        {
            heater_controller_enable(&controller1);
        }
        break;
    case DEVICE_CONTROL_HEATER1_TEMP:
        global_status.heater_1_temp_set = (unsigned char)ctl_info->value;
        heater_controller_disable(&controller1);
        heater_controller_set_target_temperature(&controller1, global_status.heater_1_temp_set);
        heater_controller_enable(&controller1);
        break;
    case DEVICE_CONTROL_HEATER2_ENABLE:
        if (ctl_info->value == 0)
        {
            heater_controller_disable(&controller2);
        }
        else
        {
            heater_controller_enable(&controller2);
        }
        break;
    case DEVICE_CONTROL_HEATER2_TEMP:
        global_status.heater_2_temp_set = (unsigned char)ctl_info->value;
        heater_controller_disable(&controller2);
        heater_controller_set_target_temperature(&controller2, global_status.heater_2_temp_set);
        heater_controller_enable(&controller2);
        break;
    case DEVICE_CONTROL_FRIDGE:
        if (global_status.fridge == 0)
        {
            xTaskCreate(fridge_task, "fridge_task", DEFAULT_STACK_SIZE, (void *)ctl_info->value, 10, NULL);
        }
        break;
    case DEVICE_CONTROL_LASER:
        if (ctl_info->value == 0)
        {
            if (global_status.dynamic == 1)
            {
                gpio_set_level(LASER_ENABLE_PIN, LOW);
                vTaskDelete(dynamic_task_handle);
                gpio_set_level(LASER_ENABLE_PIN, LOW);
                global_status.laser = 0;
                global_status.dynamic = 0;
                // printf("dynamic_task stop!\n");
            }
        }
        else
        {
            if (global_status.dynamic == 0)
            {
                xTaskCreate(dynamic_task, "dynamic_task", DEFAULT_STACK_SIZE, (void *)NULL, 10, &dynamic_task_handle);
            }
        }
        break;

    case DEVICE_CONTROL_POS_TH:
        pos_th = *(float *)&ctl_info->value;
        global_status.pos_th = pos_th;
        break;
    case DEVICE_CONTROL_REBOOT:
        esp_restart();
        break;
    default:
        break;
    }
}
void gatts_profile_device_status_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        // printf("REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_B;

        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_B);
        break;
    case ESP_GATTS_READ_EVT:
    {
        // printf("GATT_READ_EVT, conn_id %d, trans_id %ld, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        global_status.pos_th = pos_th;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = sizeof(global_status);
        memcpy(rsp.attr_value.value, &global_status, sizeof(global_status));
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;
    }
    case ESP_GATTS_WRITE_EVT:
    {
        // printf("GATT_WRITE_EVT, conn_id %d, trans_id %ld, handle %d\n", param->write.conn_id, param->write.trans_id, param->write.handle);
        if (!param->write.is_prep)
        {
            int leng = param->write.len;
            if (leng == sizeof(DEVICE_CONTROL_INFO))
            {
                PDEVICE_CONTROL_INFO ctl_info = (PDEVICE_CONTROL_INFO)param->write.value;
                device_control(ctl_info);
            }
        }
        write_event_env(gatts_if, &device_status_prepare_write_env, param);
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT:
        // printf("ESP_GATTS_EXEC_WRITE_EVT");
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        exec_write_event_env(&device_status_prepare_write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        // printf("ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT:
        // printf("CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_B;

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].service_handle);
        b_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;
        esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].service_handle, &gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].char_uuid,
                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                        b_property,
                                                        NULL, NULL);
        if (add_char_ret)
        {
            // printf("add char failed, error code =%x", add_char_ret);
        }
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        // printf("ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d\n",
        //        param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].descr_uuid.uuid.uuid16 = GATTS_DESCR_UUID_TEST_B;
        esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].service_handle, &gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].descr_uuid,
                                     ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                     NULL, NULL);
        break;
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        //  printf("ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
        //       param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        // printf("SERVICE_START_EVT, status %d, service_handle %d\n",
        //       param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONNECT_EVT:
        global_status.ble = 1;
        // printf("CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
        //        param->connect.conn_id,
        //        param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
        /////        param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile_tab[PROFILE_DEVICE_STATUS_APP_ID].conn_id = param->connect.conn_id;
        break;
    case ESP_GATTS_CONF_EVT:
        // printf("ESP_GATTS_CONF_EVT status %d attr_handle %d", param->conf.status, param->conf.handle);
        if (param->conf.status != ESP_GATT_OK)
        {
        }
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        global_status.ble = 0;
        break;
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        }
        else
        {
            // printf("Reg app failed, app_id %04x, status %d\n",
            //        param->reg.app_id,
            //        param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                gatts_if == gl_profile_tab[idx].gatts_if)
            {
                if (gl_profile_tab[idx].gatts_cb)
                {
                    gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

int ble_init()
{

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    int ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        // printf("%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return -1;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        // printf("%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return -1;
    }
    ret = esp_bluedroid_init();
    if (ret)
    {
        // printf("%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return -1;
    }
    ret = esp_bluedroid_enable();
    if (ret)
    {
        // printf("%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return -1;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret)
    {
        // printf("gatts register error, error code = %x", ret);
        return -1;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret)
    {
        // printf("gap register error, error code = %x", ret);
        return -1;
    }

    esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(TEST_DEVICE_NAME);
    if (set_dev_name_ret)
    {
        // printf("set device name failed, error code = %x", set_dev_name_ret);
    }

    // config adv data
    ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret)
    {
        // printf("config adv data failed, error code = %x", ret);
    }
    adv_config_done |= adv_config_flag;
    // config scan response data
    ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
    if (ret)
    {
        // printf("config scan response data failed, error code = %x", ret);
    }
    adv_config_done |= scan_rsp_config_flag;

    ret = esp_ble_gatts_app_register(PROFILE_TEMPERATURE_DATA_APP_ID);
    if (ret)
    {
        // printf("gatts app register error, error code = %x", ret);
        return -1;
    }
    ret = esp_ble_gatts_app_register(PROFILE_DEVICE_STATUS_APP_ID);
    if (ret)
    {
        // printf("gatts app register error, error code = %x", ret);
        return -1;
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret)
    {
        // printf("set local  MTU failed, error code = %x", local_mtu_ret);
    }
    return 0;
}
