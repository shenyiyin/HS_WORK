#include "user_app.h"
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OSAL_bufmgr.h"
#include "gatt.h"
#include "ll.h"
#include "ll_common.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"

#include <stdint.h>
#include "types.h"
#include "gpio.h"
#include "pwrmgr.h"
#include "log.h"
#include "storage.h"
#include "lcd.h"
#include "PAT.h"
#include "mixo_ots_driver.h"

#include "simpleBLECentral.h"
#include "protocol.h"
#include "buzz.h"



// 定义全局变量存储上次发送数据的DC.percent和时间戳
static int last_DC_percent = 0;
static uint32_t last_send_time = 0;

void write_protocol_data(uint8_t *data,uint8_t len);
void incrementSecond(Timestamp *ts);

extern uint16_t cnt_sleep;


uint8 User_App_TaskID;   // Task ID for internal task/event processing
Dial_Control DC;
Dispenser Dis_DATA;

#define KEY_1 P15 
#define KEY_2 P20  

#define KEY_3 P23
#define KEY_4 P18


#define E1 P31
#define E2 P7

#define Culligen_Handle 0x13

#define get_key_status(pin)   !hal_gpio_read(pin)

#define DOUBLE_CLICK_INTERVAL 300
#define LONG_PRESS_INTERVAL 3000
#define DEBOUNCE_DELAY_MS 50

#define SEND_INTERVAL 210

U_Interface UI;




attWriteReq_t* SpReq;

enum key_state {
    KEY_STATE_RELEASED,
    KEY_STATE_PRESSED,
    KEY_STATE_WAITING_DOUBLE_CLICK,
    KEY_STATE_LONG_PRESS
};

struct key_status {
    uint8_t pin;
    enum key_state state;
    uint32_t last_transition;
    uint8_t waiting_double_click;
		uint8_t long_press_triggered;
};

struct key_status keys[] = {            //0 1 2 3  心 左 锁 右
    { KEY_1, KEY_STATE_RELEASED, 0, 0 },
    { KEY_2, KEY_STATE_RELEASED, 0, 0 },
    { KEY_3, KEY_STATE_RELEASED, 0, 0 },
    { KEY_4, KEY_STATE_RELEASED, 0, 0 },
};

void user_app_check_keys(uint32_t now) {
		uint16_t read_len;
		static uint8_t num=0;
    for (int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        struct key_status *key = &keys[i];
        uint8_t pressed = get_key_status(key->pin);
        switch (key->state) {
        case KEY_STATE_RELEASED:

            if (pressed) {
                key->state = KEY_STATE_PRESSED;
                key->last_transition = now;
            }
            break;

        case KEY_STATE_PRESSED:
						if(UI.tocnt!=0)
						{
							UI.tocnt=5;
						}
            if (!pressed) {
                key->state = KEY_STATE_WAITING_DOUBLE_CLICK;
                key->last_transition = now;
            } else if ((key->long_press_triggered==0)&&(now - key->last_transition >= LONG_PRESS_INTERVAL)) {
                key->state = KEY_STATE_LONG_PRESS;
								dbg_printf(" long KEY_STATE_PRESSED\n");
								if(UI.longbuttonCallbacks[i])
												UI.longbuttonCallbacks[i]();
							  key->long_press_triggered=1;
                // TODO: Handle long press event
            }
//						else if (now - key->last_transition >= LONG_PRESS_INTERVAL)
//						{
//							dbg_printf(" long longbuttonCallbacks\n");
//								if(UI.longbuttonCallbacks[i])
//												UI.longbuttonCallbacks[i]();
//						}
            break;

        case KEY_STATE_WAITING_DOUBLE_CLICK:
					  cnt_sleep=0;
            if (pressed && now - key->last_transition <= DOUBLE_CLICK_INTERVAL) {
                key->state = KEY_STATE_PRESSED;
                key->waiting_double_click ++;
                // TODO: Handle double click event
            } 
						else if (now - key->last_transition > DOUBLE_CLICK_INTERVAL) {
								if(key->waiting_double_click)
								{
									dbg_printf(" double click=%d\n",key->waiting_double_click);
									
									if(get_ble_status()==2)
									{
										req_timestamp();
										 
									}
									
									if(key->waiting_double_click==2)
									{
										enter_ota();
									}
									if(key->waiting_double_click==3)
									{
										osal_memset(&store_setting.pair_addr,0,6);
										save_setting(&store_setting);
									}
									
									if(key->waiting_double_click>3)
									{
										NVIC_SystemReset();
									}
										
								}
								else
								{
									
											if(UI.buttonCallbacks[i])
											{
												UI.buttonCallbacks[i]();
											}
								}
                key->waiting_double_click = 0;
								key->state = KEY_STATE_RELEASED;
            }
            break;

        case KEY_STATE_LONG_PRESS:
					  cnt_sleep=0;
            if (!pressed) {
                key->state = KEY_STATE_RELEASED;
                key->last_transition = now;
								key->long_press_triggered=0;
                // TODO: Handle long press release event
            }
            break;
        }
    }
}


void user_enter_sleep(void)
{
	hal_gpioin_disable(KEY_1);
	hal_gpioin_disable(KEY_2);
	hal_gpioin_disable(KEY_3);
	hal_gpioin_disable(KEY_4);
//	hal_gpio_wakeup_set(KEY_1,POL_FALLING);
//	hal_gpio_wakeup_set(KEY_2,POL_FALLING);
//	hal_gpio_wakeup_set(KEY_3,POL_FALLING);
//	hal_gpio_wakeup_set(KEY_4,POL_FALLING);
	//hal_gpio_wakeup_set(E1,POL_RISING);
	//hal_gpio_wakeup_set(E2,POL_RISING);
	osal_stop_timerEx(User_App_TaskID, user_app_event_id);
	osal_stop_timerEx(User_App_TaskID, adcMeasureTask_EVT);
	osal_stop_timerEx(User_App_TaskID, count1s_event_id);
}






static void E12_event_handler(gpio_pin_e pin, IO_Wakeup_Pol_e type)
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = hal_systick();

    // If interrupts come faster than DEBOUNCE_DELAY_MS, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > DEBOUNCE_DELAY_MS)
    {
        if(pin == E1 && type == POL_FALLING)
        {
						cnt_sleep=0;
						dbg_printf("common_sleep_exi11222222211111111t=%d\n",cnt_sleep);
						common_sleep_exit();
            //if(DC.percent>0)DC.percent -= 1; // Increment the angle if a falling edge is detected on E1
        }
        else if(pin == E2 && type == POL_FALLING)
        {
            //if(DC.percent<100)DC.percent += 1; // Decrement the angle if a falling edge is detected on E2
        }
				
    }
    last_interrupt_time = interrupt_time;
}




void input_io_init(void)
{
//    hal_gpio_pin_init(E1, GPIO_INPUT); 
//    hal_gpio_pull_set(E1, STRONG_PULL_UP); 

//    hal_gpio_pin_init(E2, GPIO_INPUT); 
//    hal_gpio_pull_set(E2, STRONG_PULL_UP); 

    hal_gpio_pin_init(KEY_1, GPIO_INPUT); 
    hal_gpio_pull_set(KEY_1, STRONG_PULL_UP); 

    hal_gpio_pin_init(KEY_2, GPIO_INPUT); 
    hal_gpio_pull_set(KEY_2, STRONG_PULL_UP); 

    hal_gpio_pin_init(KEY_3, GPIO_INPUT); 
    hal_gpio_pull_set(KEY_3, STRONG_PULL_UP); 

    hal_gpio_pin_init(KEY_4, GPIO_INPUT); 
    hal_gpio_pull_set(KEY_4, STRONG_PULL_UP); 
//	
//		hal_gpioin_register(E1, E12_event_handler, E12_event_handler);
//		hal_gpioin_register(E2, E12_event_handler, E12_event_handler);
}

//1 确认 2 选择 0 返回 3出水

void user_exit_sleep(void)
{
	input_io_init();
	osal_start_timerEx(User_App_TaskID, user_app_event_id, 10);
	osal_start_timerEx(User_App_TaskID, adcMeasureTask_EVT, 1000);
	osal_start_timerEx(User_App_TaskID, count1s_event_id, 1000);
}

void user_app_init(uint8_t task_id)
{
	User_App_TaskID=task_id;
	dbg_printf("task_id=%d\n",task_id);
	input_io_init();
	//pat_init();
	mixo_ots_init();
	hal_pwrmgr_lock(MOD_USR1);
	turn_to_standby();
	SpReq = osal_mem_alloc(sizeof(attWriteReq_t));
	osal_memset(&timestamp,0,sizeof(timestamp));
	osal_memset(&temp_timestamp,0,sizeof(timestamp));
	DC.percent=0;
	if(store_setting.defaultnormal_tmp==0)
	{
		store_setting.defaultnormal_tmp=25;
		save_setting(&store_setting);
	}
	
	osal_start_timerEx(User_App_TaskID, adcMeasureTask_EVT, 3000);
	osal_start_timerEx(User_App_TaskID, user_app_event_id, 10);
}

extern uint8_t low_power_mode;

uint16 user_app_ProcessEvent( uint8 task_id, uint16 events )
{
	static short angle=0xFFF;
	static uint8_t csl_cnt=0,ml_cnt=0,s_on=0;
	short temp_angle;
	
	if ( events & user_app_event_id )
	{
		
		if(!low_power_mode)
		{
			uint32_t now = hal_systick();
      user_app_check_keys(now);
			//temp_angle=angle_get();
			temp_angle=(get_motion_angle());
			//dbg_printf("temp_angle=%d\n",temp_angle);

			if(temp_angle!=0xFFF)
			{
				if(UI.tocnt!=0)
				{
					UI.tocnt=5;
				}
				DC.percent += Dis_DATA.unit_weight*temp_angle; // Decrement the angle if a falling edge is detected on E2
				dbg_printf("DC.percent=%d\n",DC.percent);
			}

			
			osal_start_timerEx(User_App_TaskID, user_app_event_id, 10);
		}
		else
		{
			osal_stop_timerEx(User_App_TaskID, user_app_event_id);
		}
		return (events ^ user_app_event_id);
	}
	
	
	
	if(user_notifyenable_event_id&events)
	{
		LOG("USER_APP_DEMO_EVT_1S\n");
//		if(hal_gpio_read(P1))
		{
			#define _NOTIFY_ML	1
				//SpReq = osal_mem_alloc(sizeof(attWriteReq_t));
				SpReq->sig = 0;
				SpReq->cmd = FALSE;
		    SpReq->len = 2;
			#if(_NOTIFY_ML)
				SpReq->handle = Culligen_Handle+2;;
        SpReq->value[0] = (unsigned char)(GATT_CLIENT_CFG_NOTIFY);
        SpReq->value[1] = (unsigned char)(GATT_CLIENT_CFG_NOTIFY >> 8);
			#else
				SpReq->handle = 11+1;//ML_0013_W_HANDLE;
        SpReq->value[0] = (unsigned char)(GATT_CLIENT_CFG_INDICATE);
        SpReq->value[1] = (unsigned char)(GATT_CLIENT_CFG_INDICATE >> 8);
			#endif
			
			bStatus_t status = GATT_WriteCharValue(simpleBLEConnHandle, SpReq,simpleBLETaskId);	

			//osal_mem_free(SpReq);
			if(status == SUCCESS)
			{
					//req_timestamp();
				  osal_start_timerEx(User_App_TaskID, user_req_timestamp_event_id, 200);
					//osal_start_timerEx(User_App_TaskID, user_notifyenable_event_id, 3000);
			}
			else
			{
				  osal_start_timerEx(User_App_TaskID, user_notifyenable_event_id, 1000);
			}		
		}
	
		return(events^user_notifyenable_event_id);
	}
	// init req
	if(user_req_timestamp_event_id&events)
	{
		
		LOG("r GATT_WriteCharValue success \r\n");
		req_timestamp();
		return(events^user_req_timestamp_event_id);
	}
	
	if(user_req_defaultwarmwater_event_id&events)
	{
		LOG("r GATT_WriteCharValue success \r\n");
		req_default_warm_water();
		return(events^user_req_defaultwarmwater_event_id);
	}
	

	// timestampcnt
	if(count1s_event_id&events)
	{
		incrementSecond(&timestamp);
		osal_start_timerEx(User_App_TaskID, count1s_event_id, 1000);
		return(events^count1s_event_id);
	}
	
	//按键设置事件
	if(write_watervolumn_event_id&events)
	{
		if((water_volumn_set()))
		{
			csl_cnt++;
			if(csl_cnt<5)
			{
				osal_start_timerEx(User_App_TaskID, write_watervolumn_event_id, 600);
			}
			else
			{
				csl_cnt=0;
			}
		}
		else
		{
			csl_cnt=0;
			osal_start_timerEx(User_App_TaskID, write_watertempeature_event_id, SEND_INTERVAL);
		}
		return(events^write_watervolumn_event_id);
	}
	
	if(write_watertempeature_event_id&events)
	{
		if(water_temperature_set())
		{
			ml_cnt++;
			if(ml_cnt<5)
			{
				osal_start_timerEx(User_App_TaskID, write_watertempeature_event_id, 600);
			}
			else
			{
				csl_cnt=0;
			}
		}
		else
		{

			ml_cnt=0;
			osal_start_timerEx(User_App_TaskID, write_switchOn_event_id, SEND_INTERVAL);
		}
		return(events^write_watertempeature_event_id);
	}
	
	if(write_switchOn_event_id&events)
	{
		
		if(water_switch_on())
		{
			s_on++;
			if(s_on<5)
			{
					osal_start_timerEx(User_App_TaskID, write_switchOn_event_id, SEND_INTERVAL);
			}
			else
			{
				s_on=0;
			}
		}
		return(events^write_switchOn_event_id);
	}
	
	
	//connect_timeout
	if(events & connect_timeout_event_id)
	{
		//gapCancelLinkReq(task_id,);
		return (events ^ connect_timeout_event_id);
	}
	
	//adc bat
	if ( events & adcMeasureTask_EVT )
	{
			//testconnect();
			// Perform periodic heart rate task
			//LOG("adcMeasureTask_EVT\n");
			adcMeasureTask();
			osal_start_timerEx(User_App_TaskID, adcMeasureTask_EVT, 1000);
			return (events ^ adcMeasureTask_EVT);
	}
	return 0;
}





void write_protocol_data(uint8_t *data,uint8_t len)
{
	// for (int i=0;i<len ;i++)dbg_printf(" 0x%x",data[i]);
	 SpReq->len=len;
	 VOID osal_memcpy( SpReq->value, data, len ) ;
	 if(get_ble_status()==2)
	 {
		  bStatus_t status = GATT_WriteCharValue(simpleBLEConnHandle, SpReq,simpleBLETaskId);	
	 }
}

void req_timestamp(void)
{
	SpReq->handle = Culligen_Handle+1;
	SpReq->sig = 0;
	SpReq->cmd = FALSE;
	SpReq->len=7;

	uint8_t data[7]={0x55,0xaa,3,0x1c,00,00,0x1e};
	VOID osal_memcpy( SpReq->value, data, 7 ) ;
	bStatus_t status = GATT_WriteCharValue(simpleBLEConnHandle, SpReq,simpleBLETaskId);	
	dbg_printf("req stmp=%d\n",status);
}

void req_default_warm_water(void)
{
	SpReq->handle = Culligen_Handle+1;
	SpReq->sig = 0;
	SpReq->cmd = FALSE;
	SpReq->len=7;

	uint8_t data[7]={0x55,0xaa,0,8,00,00,7};
	VOID osal_memcpy( SpReq->value, data, 7 ) ;
	bStatus_t status = GATT_WriteCharValue(simpleBLEConnHandle, SpReq,simpleBLETaskId);	
	dbg_printf("req all point");
}

bStatus_t water_switch_on(void)
{	
	SpReq->handle = Culligen_Handle+1;
	SpReq->sig = 0;
	SpReq->cmd = FALSE;
	SpReq->len=12;
	uint8_t data[32]={0};
	uint8_t len=send_water_switch(data,1);
	SpReq->len=len;
	VOID osal_memcpy( SpReq->value, data, len ) ;
	bStatus_t status = GATT_WriteCharValue(simpleBLEConnHandle, SpReq,simpleBLETaskId);	
	dbg_printf("write water_switch_on=%d\n",status);
	return status;
}

void water_switch_off(void)
{			 
	osal_stop_timerEx(User_App_TaskID, write_watervolumn_event_id);
	osal_stop_timerEx(User_App_TaskID, write_watertempeature_event_id);
	osal_stop_timerEx(User_App_TaskID, write_switchOn_event_id);
	SpReq->handle = Culligen_Handle+1;
	SpReq->sig = 0;
	SpReq->cmd = FALSE;//FALSE//TRUE;
	SpReq->len=12;
	uint8_t data[32]={0};
	uint8_t len=send_water_switch(data,0);
	SpReq->len=len;
	VOID osal_memcpy( SpReq->value, data, len ) ;
	bStatus_t status = GATT_WriteCharValue(simpleBLEConnHandle, SpReq,simpleBLETaskId);	
}

bStatus_t water_volumn_set(void)
{			 
	if(!Dis_DATA.setwaterVolume)
		return 0;
	SpReq->handle = Culligen_Handle+1;
	SpReq->sig = 0;
	SpReq->cmd = FALSE;
	SpReq->len=12;
	uint8_t data[32]={0};
	uint8_t len=send_water_amount(data,Dis_DATA.setwaterVolume);
	dbg_printf("chushuiliang =%d\n",((data[12]<<8)+data[13]));
	SpReq->len=len;
	VOID osal_memcpy( SpReq->value, data, len ) ;
	bStatus_t status = GATT_WriteCharValue(simpleBLEConnHandle, SpReq,simpleBLETaskId);	
	return status;

}


bStatus_t water_temperature_set(void)
{	
	SpReq->handle = Culligen_Handle+1;
	SpReq->sig = 0;
	SpReq->cmd = FALSE;//FALSE//TRUE;
	SpReq->len=12;
	uint8_t data[32]={0};
	uint8_t len=send_temperature_setting(data,Dis_DATA.setTemperature);
	dbg_printf("wendu =%d\n",data[13]);
	SpReq->len=len;
	VOID osal_memcpy( SpReq->value, data, len ) ;
	bStatus_t status = GATT_WriteCharValue(simpleBLEConnHandle, SpReq,simpleBLETaskId);	
	dbg_printf("write status2=%d\n",status);
	return status;
}


int set_ota_mode(uint8_t mode)
{
    write_reg(OTA_MODE_SELECT_REG, mode);
    return PPlus_SUCCESS;
}


void enter_ota(void)
{

	 uint8_t rsp = PPlus_SUCCESS;
	 rsp = set_ota_mode(1);
	 NVIC_SystemReset();
}


void incrementSecond(Timestamp *ts) {
    ts->second++;
    if (ts->second >= 60) {
        ts->second = 0;
        ts->minute++;
        if (ts->minute >= 60) {
            ts->minute = 0;
            ts->hour++;
            if (ts->hour >= 24) {
                ts->hour = 0;
                ts->day++;
            }
        }
    }
}