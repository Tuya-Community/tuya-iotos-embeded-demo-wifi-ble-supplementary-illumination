/*
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @LastEditors: zgw
 * @file name: sensor_control.c
 * @Description: 
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-1-5 16:21:15
 * @LastEditTime: 2021-1-5 16:21:15
 */

#include "sensor_control.h"
#include "bh1750.h"
#include "tuya_gpio.h"
#include "app_pwm.h"
/***********************************************************
*************************types define***********************
***********************************************************/
typedef enum
{
    LOW = 0,
    HIGH,
}default_level;



DEVICE_DATA_T device_data = {
    .bright_value = 0,
    .light_switch = FALSE,
    .auto_switch  = FALSE,
    .light_value  = 500,
    .bright_value_set = 500,
};

USER_PWM_DUTY_T user_pwm_duty = {0,0};

#define ACCURACY                         (50)
/***********************************************************
*************************IO control device define***********
***********************************************************/
#define IIC_SDA_PORT                     (9)
#define IIC_SCL_PORT                     (24)



/***********************************************************
*************************about adc init*********************
***********************************************************/



/***********************************************************
*************************about iic init*********************
***********************************************************/
STATIC bh1750_init_t bh1750_int_param = {IIC_SDA_PORT, IIC_SCL_PORT};


/***********************************************************
*************************function***************************
***********************************************************/

STATIC VOID __ctrl_gpio_init(CONST TY_GPIO_PORT_E port, CONST BOOL_T high)
{
    tuya_gpio_inout_set(port, FALSE);
    tuya_gpio_write(port, high);
}

VOID app_device_init(VOID)
{
    INT_T op_ret = 0;
    
    tuya_bh1750_init(&bh1750_int_param);

}



OPERATE_RET app_get_all_sensor_data(VOID)
{
    OPERATE_RET ret = 0;

    device_data.bright_value = tuya_bh1750_get_bright_value();
    PR_NOTICE("light_intensity_value = %d",device_data.bright_value);
    
    return ret;
}

STATIC VOID __passive_ctrl_module_light_auto(VOID)
{
    if(TRUE == device_data.auto_switch) {
        USHORT_T current = device_data.bright_value;
        USHORT_T set = device_data.bright_value_set;

        if((current - set) > ACCURACY) {
            if((current - set) >= 100) {
                if(device_data.light_value >= 100)device_data.light_value -= 100;
            }else if((current - set) > 50) {
                if(device_data.light_value >= 50)device_data.light_value -= 50;
            }else {
                if(device_data.light_value >= 1)device_data.light_value--;
            }
        }else if((set - current) > ACCURACY) {
            if((set - current) >= 100) {
                if(device_data.light_value <= 900)device_data.light_value += 100;
            }else if((set - current) > 50) {
                if(device_data.light_value <= 950)device_data.light_value += 50;
            }else {
                if(device_data.light_value <= 999)device_data.light_value ++;
            }
        }
        //app_pwm_set(&user_pwm_duty);
    }
}


STATIC VOID __initiative_ctrl_module_light(VOID)
{   
    if(TRUE == device_data.light_switch) {
        PR_NOTICE("Ligth open !!!!");
        user_pwm_duty.duty_blue = device_data.light_value;
        user_pwm_duty.duty_red = user_pwm_duty.duty_blue;
        app_pwm_set(&user_pwm_duty);
    }else {
        user_pwm_duty.duty_blue = 0;
        user_pwm_duty.duty_red = user_pwm_duty.duty_blue;
        app_pwm_set(&user_pwm_duty);
    }                                                                           
}

VOID app_ctrl_handle(VOID)
{   
    PR_DEBUG("ctrl handle");
    __initiative_ctrl_module_light();
    __passive_ctrl_module_light_auto();
}

VOID app_ctrl_all_off(VOID)
{   
    ;
}