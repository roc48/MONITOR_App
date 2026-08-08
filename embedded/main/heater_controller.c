#include "heater_controller.h"

void heater_controller_init(PHEATER_CONTROLLER controller, ledc_timer_t ledc_timer_id, ledc_channel_t ledc_channel_id, int pwm_max, int pwm_min, GET_TEMPERATURE_DATA revoke_function)
{
    esp_timer_create_args_t timer0_config = {
        .callback = (esp_timer_cb_t)PID_control,
        .arg = controller,
    };

    gpio_reset_pin(controller->heater_pin);
    gpio_set_direction(controller->heater_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(controller->heater_pin, LOW);

    max6675_init(&controller->thermocouple);

    heater_controller_set_PWM_limits(controller, pwm_max, pwm_min);

    controller->ledc_channel_id = ledc_channel_id;
    controller->revoke_function = revoke_function;

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = ledc_channel_id,
        .timer_sel = ledc_timer_id,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = controller->heater_pin,
        .duty = 0, // Set duty to 0%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = ledc_timer_id,
        .duty_resolution = 8,
        .freq_hz = 100,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    esp_timer_create(&timer0_config, &controller->timer);
}

void heater_controller_set_PWM_limits(PHEATER_CONTROLLER controller, int pwm_max, int pwm_min)
{

    if (pwm_max > 255)
        pwm_max = 255;
    if (pwm_max < 0)
        pwm_max = 255;
    if (pwm_min < 0)
        pwm_min = 0;
    if (pwm_min > 255)
        pwm_min = 0;
    if (pwm_max < pwm_min)
        pwm_max = pwm_min;

    controller->pwm_max = pwm_max;
    controller->pwm_min = pwm_min;
}

void heater_controller_set_PID_args(PHEATER_CONTROLLER controller, double kp, double ki, double kd)
{
    controller->kp = kp;
    controller->ki = ki;
    controller->kd = kd;
}

void heater_controller_set_target_temperature(PHEATER_CONTROLLER controller, double temperature)
{
    controller->temperature_set = temperature;
}

void heater_controller_set_pins(PHEATER_CONTROLLER controller,
                                int MAX6675_THERMOCOUPLE_CS_PIN,
                                int MAX6675_THERMOCOUPLE_SCK_PIN,
                                int MAX6675_THERMOCOUPLE_SO_PIN,
                                int HEATER_PWM_PIN)
{
    controller->thermocouple.CS_pin = MAX6675_THERMOCOUPLE_CS_PIN;
    controller->thermocouple.SCK_pin = MAX6675_THERMOCOUPLE_SCK_PIN;
    controller->thermocouple.SO_pin = MAX6675_THERMOCOUPLE_SO_PIN;
    controller->heater_pin = HEATER_PWM_PIN;
}

void PID_control(PHEATER_CONTROLLER controller)
{
    if (!controller->enable_flag)
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, controller->ledc_channel_id, LOW);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, controller->ledc_channel_id);
        return;
    }

    double temperature_current = max6675_read_celsius(&controller->thermocouple);

    int pwm_out = 0;
    do
    {
        
        if (isnan(temperature_current) || (temperature_current == 0))
        {
            break;
        }

        if(temperature_current > controller->temperature_set + 1){
            pwm_out = controller->pwm_min;
            break;
        }

        if(temperature_current < controller->temperature_set - 2 ){
            pwm_out = controller->pwm_max;
            break;
        }
        // PID控制算法：分别计算比例项、积分项、微分项的值
        double error = controller->temperature_set - temperature_current;
        controller->proportional_term = controller->kp * error;
        controller->integral_term += controller->ki * error;
        if (controller->integral_term > controller->pwm_max)
            controller->integral_term = controller->pwm_max;
        if (controller->integral_term < controller->pwm_min)
            controller->integral_term = controller->pwm_min;
        controller->derivative_term = controller->kd * (temperature_current - controller->last_input);

        // 计算pwm信号输出值
        pwm_out = controller->proportional_term + controller->integral_term + controller->derivative_term;
        if (pwm_out > controller->pwm_max)
            pwm_out = controller->pwm_max;
        if (pwm_out < controller->pwm_min)
            pwm_out = controller->pwm_min;

        controller->last_input = temperature_current;

    } while (false);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, controller->ledc_channel_id, pwm_out);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, controller->ledc_channel_id);
    

    if (controller->revoke_function != NULL)
    {
        (*(GET_TEMPERATURE_DATA)controller->revoke_function)(controller, temperature_current, pwm_out);
    }
}

void heater_controller_enable(PHEATER_CONTROLLER controller)
{
    esp_timer_start_periodic(controller->timer, 250000);
    controller->enable_flag = 1;
    if (controller->ledc_channel_id == 0)
    {
        global_status.heater_1 = 1;
    }
    else if (controller->ledc_channel_id == 1)
    {
        global_status.heater_2 = 1;
    }
}

void heater_controller_disable(PHEATER_CONTROLLER controller)
{
    esp_timer_stop(controller->timer);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, controller->ledc_channel_id, LOW);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, controller->ledc_channel_id);
    controller->enable_flag = 0;
    if (controller->ledc_channel_id == 0)
    {
        global_status.heater_1 = 0;
    }
    else if (controller->ledc_channel_id == 1)
    {
        global_status.heater_2 = 0;
    }
}