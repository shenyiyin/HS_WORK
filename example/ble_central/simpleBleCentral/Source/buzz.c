#include "buzz.h"
#include "pwm.h"
#include "adc.h"
#include "log.h"
#include "user_app.h"
#define vol_weight 6.6
#define buzz_pin P3
#define bat_adc_pin P24


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
    pwm_ch_config.cmpVal = 300;
    pwm_ch_config.cntTopVal = 3925;

    // 启动PWM通道
    hal_pwm_ch_start(pwm_ch_config);
	//buzz_off();
}


void buzz_on(void)
{
  // 启动PWM
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



static adc_Cfg_t adc_cfg =
{
    .channel = ADC_BIT(ADC_CH2N_P24)|ADC_BIT(ADC_CH2N_P24)|ADC_BIT(ADC_CH2N_P24),
    .is_continue_mode = FALSE,
    .is_differential_mode = 0x00,
    .is_high_resolution = 0x00,
};


/*********************************************************************
    LOCAL FUNCTIONS
*/



// !request temp cache buff

/*********************************************************************
    PROFILE CALLBACKS
*/

/*********************************************************************
    PUBLIC FUNCTIONS
*/









static void adc_evt(adc_Evt_t* pev)
{
    float value = 0;
    int i = 0;
    bool is_high_resolution = FALSE;
    bool is_differential_mode = FALSE;
    uint8_t ch = 0;

    if((pev->type != HAL_ADC_EVT_DATA) || (pev->ch < 2))
        return;

    osal_memcpy(adc_debug[pev->ch-2],pev->data,2*(pev->size));
    channel_done_flag |= BIT(pev->ch);

    if(channel_done_flag == adc_cfg.channel)
    {
        for(i=4; i<5; i++)
        {
            if(channel_done_flag & BIT(i))
            {
                is_high_resolution = (adc_cfg.is_high_resolution & BIT(i))?TRUE:FALSE;
                is_differential_mode = (adc_cfg.is_differential_mode & BIT(i))?TRUE:FALSE;
                value = hal_adc_value_cal((adc_CH_t)i,adc_debug[i-2], pev->size, is_high_resolution,is_differential_mode);

                switch(i)
                {
                case ADC_CH1P_P23:
                    ch=23;
                    break;

                case ADC_CH2N_P24:
                    ch=24;
                    break;

                default:
                    break;
                }

                if(ch!=0)
                {
										Dis_DATA.bat_quantity=(int)(vol_weight*value*1000);
                    LOG("P%d %d mv ",ch,(int)(vol_weight*value*1000));
                }
                else
                {
                    LOG("invalid channel\n");
                }
            }
        }

        LOG(" mode:%d \n",adc_cfg.is_continue_mode);
        channel_done_flag = 0;
    }
}
void adcMeasureTask( void )
{
    int ret;
    bool batt_mode = FALSE;
    uint8_t batt_ch = ADC_CH2N_P24;
    GPIO_Pin_e pin;
    if(FALSE == batt_mode)
    {
        ret = hal_adc_config_channel(adc_cfg, adc_evt);
    }
    else
    {
        if(((((1 << batt_ch) & adc_cfg.channel) == 0)) || (adc_cfg.is_differential_mode != 0x00))
        {
            return;
        }

        pin = s_pinmap[batt_ch];
        hal_gpio_cfg_analog_io(pin,Bit_DISABLE);
        hal_gpio_write(pin, 1);
        ret = hal_adc_config_channel(adc_cfg, adc_evt);
        hal_gpio_cfg_analog_io(pin,Bit_DISABLE);
    }

    if(ret)
    {
        LOG("ret = %d\n",ret);
        return;
    }

    hal_adc_start(INTERRUPT_MODE);
}