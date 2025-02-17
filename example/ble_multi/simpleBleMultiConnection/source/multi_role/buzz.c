#include "buzz.h"
#include "pwm.h"

#define buzz_pin P3



static gpio_pin_e* led_pin_ptr = NULL;
static pwm_ch_t pwm_ch[1];


void buzz_pwm_init(void)
{
     // 初始化PWM模块
    hal_pwm_module_init();

    // 配置PWM通道
    pwm_ch_t pwm_ch_config;
    pwm_ch_config.pwmN = PWM_CH0;
    pwm_ch_config.pwmPin = buzz_pin;
    pwm_ch_config.pwmDiv = PWM_CLK_NO_DIV;
    pwm_ch_config.pwmMode = PWM_CNT_UP;
    pwm_ch_config.pwmPolarity = PWM_POLARITY_RISING;
    pwm_ch_config.cmpVal = 2962;
    pwm_ch_config.cntTopVal = 3925;

    // 启动PWM通道
    hal_pwm_ch_start(pwm_ch_config);
}


void buzz_on(void)
{
  // 启动PWM
   hal_pwm_start();

}


void buzz_off(void)
{
	 hal_pwm_stop();
}

