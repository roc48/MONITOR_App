#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#define DEVICE_MODEL 0x2023
#define FIRMWARE_VERSION 19

#define HEATER_1_PWM 13
#define MAX6675_THERMOCOUPLE_1_SCK_PIN 4
#define MAX6675_THERMOCOUPLE_1_CS_PIN 15
#define MAX6675_THERMOCOUPLE_1_SO_PIN 33
#define HEATER_2_PWM 27
#define MAX6675_THERMOCOUPLE_2_SCK_PIN 17
#define MAX6675_THERMOCOUPLE_2_CS_PIN 16
#define MAX6675_THERMOCOUPLE_2_SO_PIN 32

#define AD7705_1_CS_PIN 18
#define AD7705_1_SCK_PIN 5
#define AD7705_1_DN_PIN 19
#define AD7705_1_DOUT_PIN 35

#define AD7705_2_CS_PIN 22
#define AD7705_2_SCK_PIN 21
#define AD7705_2_DN_PIN 23
#define AD7705_2_DOUT_PIN 34

#define LASER_ENABLE_PIN 26

#define FRIDGE_ENABLE_PIN 25

#define LED_C1_PIN 12
#define LED_STATUS_PIN 2
#define LED_C2_PIN 14

#define IO_PIN_SERIAL_TXD 1
#define IO_PIN_SERIAL_RXD 3

#define HIGH 1
#define LOW 0

#define INFO_NOTIFY_TYPE_TEMPERATURE 0xA1AB //AdnaLAB
#define INFO_NOTIFY_TYPE_DYNAMIC 0xBABE //BABE

#define DEFAULT_STACK_SIZE (2048)

#pragma pack(1) 
typedef struct temperature_data_info
{
    unsigned short type;
    unsigned long long global_time;
    unsigned char controller_id;
    unsigned char pwm_value;
    float temperature;
} TEMPERATURE_DATA_INFO, *PTEMPERATURE_DATA_INFO;

typedef struct dynamic_data_info
{
    unsigned short type;
    unsigned long long global_time;
    unsigned short  data0;
    unsigned short  data1;
    unsigned char ch1_pos;
    unsigned char ch2_pos;
} DYNAMIC_DATA_INFO, *PDYNAMIC_DATA_INFO;

#define DEVICE_CONTROL_INVALID 0
#define DEVICE_CONTROL_HEATER1_ENABLE 1
#define DEVICE_CONTROL_HEATER1_TEMP 2
#define DEVICE_CONTROL_HEATER2_ENABLE 3
#define DEVICE_CONTROL_HEATER2_TEMP 4
#define DEVICE_CONTROL_FRIDGE 5
#define DEVICE_CONTROL_LASER 6
#define DEVICE_CONTROL_POS_TH 7
#define DEVICE_CONTROL_REBOOT 8

typedef struct device_control_info{

    int target;
    int value;

}DEVICE_CONTROL_INFO,*PDEVICE_CONTROL_INFO;

typedef struct device_status{
    unsigned short device_model;
    unsigned long long global_time;
    unsigned char firmware_version;
    unsigned char heater_1;
    unsigned char heater_1_temp_set;
    unsigned char heater_2;
    unsigned char heater_2_temp_set;
    unsigned char fridge : 2;
    unsigned char laser: 2;
    unsigned char dynamic: 2;
    unsigned char ble: 2;
    float pos_th;
}DEVICE_STATUS,*PDEVICE_STATUS;

#pragma pack()

extern DEVICE_STATUS global_status;

#endif