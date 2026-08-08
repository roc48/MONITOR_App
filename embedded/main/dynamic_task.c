
#include "dynamic_task.h"
#include "driver/gpio.h"

extern AD7705 ad7705_f;

extern int dynamic_data_notify_flag;

TaskHandle_t dynamic_task_handle;

float pos_th = 1.2;
void dynamic_task()
{
    printf("dynamic_task start!\n");
    global_status.dynamic = 1;
    int d_time = (60 - 2.5) * 1000;
    int times = 0;

    int init = false;
    int ch1_init = 0;
    int ch2_init = 0;

    int ch1_pos = 0;
    int ch2_pos = 0;

unsigned long long start_time = global_status.global_time;

    gpio_set_level(LED_C1_PIN,false);
    gpio_set_level(LED_C2_PIN,false);
    while (1)
    {
        printf("pos_th:\t%f\n",pos_th);
        ad7705_calibration(&ad7705_f1);
        ad7705_calibration(&ad7705_f2);
        global_status.laser = 1;
        gpio_set_level(LASER_ENABLE_PIN, HIGH);
        int average0 = 0;
        int average1 = 0;
        unsigned long long time_average = 0;
        int send_flag = false;
        for (int i = 0; i < 10; i++)
        {
            vTaskDelay(250 / portTICK_PERIOD_MS);
            unsigned short current_data0 = ad7705_read_value(&ad7705_f1, 1);
            unsigned short current_data1 = ad7705_read_value(&ad7705_f2, 1);
            //printf("DATAOUT: %d %d\n", current_data0, current_data1);
            time_average += global_status.global_time;
            if (current_data0 != 0xff)
            {
                average0 += current_data0;
                send_flag = true;
            }
            if (current_data1 != 0xff)
            {
                average1 += current_data1;
                send_flag = true;
            }
        }
        gpio_set_level(LASER_ENABLE_PIN, LOW);
        global_status.laser = 0;

        DYNAMIC_DATA_INFO info = {
            .data0 = (unsigned short)(average0 / 10),
            .data1 = (unsigned short)(average1 / 10),
            .global_time = (time_average / 10 - start_time),
            .type = INFO_NOTIFY_TYPE_DYNAMIC};

       

        if (times != 0 && init == false){
            ch1_init = info.data0;
            ch2_init = info.data1;
            init  =1;
        }

        if( times  == 2){ //31
            if(info.data0 >= ch1_init*pos_th){
                //ch1 pos
                gpio_set_level(LED_C1_PIN,true);
                ch1_pos  = 1;
            }
            if(info.data1 >= ch2_init*pos_th){
                //ch2 pos
                gpio_set_level(LED_C2_PIN,true);
                ch2_pos  = 1;
            }
            ch1_pos |= 0x10;
            ch2_pos |= 0x10;

        }

        info.ch1_pos = ch1_pos;
        info.ch2_pos = ch2_pos;
        
         if (dynamic_data_notify_flag && send_flag)
        {
            esp_ble_gatts_set_attr_value(gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].char_handle, sizeof(info), (uint8_t *)&info);
            esp_ble_gatts_send_indicate(gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].gatts_if,
                                        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].conn_id,
                                        gl_profile_tab[PROFILE_TEMPERATURE_DATA_APP_ID].char_handle,
                                        sizeof(info), (uint8_t *)&info, false);
        }

        printf("fluorescence_0:\t%d\nfluorescence_1:\t%d\n", info.data0, info.data1);

        vTaskDelay(d_time / portTICK_PERIOD_MS);
        times ++;
    }
}
