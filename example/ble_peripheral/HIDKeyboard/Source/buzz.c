#include "buzz.h"
#include "pwm.h"
#include "log.h"
#include "user_app.h"
#define vol_weight 6.6
#define buzz_pin P3
#define bat_adc_pin P24


static gpio_pin_e* led_pin_ptr = NULL;
static pwm_ch_t pwm_ch[1];


void buzz_pwm_init(void)
{
     // ��ʼ��PWMģ��
    hal_pwm_module_init();

    // ����PWMͨ��
    pwm_ch_t pwm_ch_config;
    pwm_ch_config.pwmN = PWM_CH0;
    pwm_ch_config.pwmPin = buzz_pin;
    pwm_ch_config.pwmDiv = PWM_CLK_NO_DIV;
    pwm_ch_config.pwmMode = PWM_CNT_UP;
    pwm_ch_config.pwmPolarity = PWM_POLARITY_RISING;
    pwm_ch_config.cmpVal = 300;
    pwm_ch_config.cntTopVal = 3925;

    // ����PWMͨ��
    hal_pwm_ch_start(pwm_ch_config);
	//buzz_off();
}


void buzz_on(void)
{
  // ����PWM
   hal_pwm_start();
}


void buzz_off(void)
{
	LOG("off");
	 hal_pwm_stop();
}

#define MAX_SAMPLE_POINT    64
uint16_t adc_debug[3][MAX_SAMPLE_POINT];
static uint8_t channel_done_flag = 0;










