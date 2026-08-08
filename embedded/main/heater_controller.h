#ifndef HEATER_CONTROLLER_H
#define HEATER_CONTROLLER_H

#include <math.h>
#include "max6675.h"
#include "global_config.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"


#define OUTPUT_PWM_VALUE_MAX 255
#define OUTPUT_PWM_VALUE_MIN 0


typedef struct heater_controller
{    
    MAX6675 thermocouple;
    int heater_pin;

    //PID 参数
    double kp ;
    double ki ;
    double kd ;
    //温度设置
    double temperature_set ;
    //上次温度采样值
    double last_input;
    double proportional_term ;
    double integral_term ;
    double derivative_term ;

    esp_timer_handle_t timer;
    ledc_channel_t ledc_channel_id;

    int pwm_max;
    int pwm_min;

    int enable_flag;

    void* revoke_function;
}HEATER_CONTROLLER,*PHEATER_CONTROLLER;

typedef  void(*GET_TEMPERATURE_DATA)(PHEATER_CONTROLLER controller,double temperature,int pwm);

void heater_controller_set_PWM_limits(PHEATER_CONTROLLER controller, int pwm_max, int pwm_min);

void heater_controller_set_PID_args(PHEATER_CONTROLLER controller, double kp, double ki, double kd);

void heater_controller_set_target_temperature(PHEATER_CONTROLLER controller,double temperature);

void heater_controller_set_pins(PHEATER_CONTROLLER controller,
                                int MAX6675_THERMOCOUPLE_CS_PIN,
                                int MAX6675_THERMOCOUPLE_SCK_PIN,
                                int MAX6675_THERMOCOUPLE_SO_PIN,
                                int HEATER_PWM_PIN);

void heater_controller_init(PHEATER_CONTROLLER controller,ledc_timer_t ledc_timer_id,ledc_channel_t ledc_channel_id,int pwm_max,int pwm_min,GET_TEMPERATURE_DATA revoke_function);

void PID_control(PHEATER_CONTROLLER controller);

void heater_controller_enable(PHEATER_CONTROLLER controller);

void heater_controller_disable(PHEATER_CONTROLLER controller);


extern HEATER_CONTROLLER controller1;
extern HEATER_CONTROLLER controller2;
#endif