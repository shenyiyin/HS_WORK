#include "user_app.h"
#include <stdint.h>
#include "types.h"
#include "gpio.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "pwrmgr.h"
#include "log.h"
#include "storage.h"
#include "lcd.h"
#include "PAT.h"
static uint8 User_App_TaskID;   // Task ID for internal task/event processing
Dial_Control DC;

#define KEY_1 P15 
#define KEY_2 P20  

#define KEY_3 P23
#define KEY_4 P24


#define E1 P31
#define E2 P7


#define get_key_status(pin)   !hal_gpio_read(pin)
#define DOUBLE_CLICK_INTERVAL 500
#define LONG_PRESS_INTERVAL 1000
#define DEBOUNCE_DELAY_MS 50

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
};

struct key_status keys[] = {
    { KEY_1, KEY_STATE_RELEASED, 0, 0 },
    { KEY_2, KEY_STATE_RELEASED, 0, 0 },
    { KEY_3, KEY_STATE_RELEASED, 0, 0 },
    { KEY_4, KEY_STATE_RELEASED, 0, 0 },
};

void user_app_check_keys(uint32_t now) {
		uint8_t test[]={"12345678"},read_buf[10];
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
            if (!pressed) {
                key->state = KEY_STATE_WAITING_DOUBLE_CLICK;
                key->last_transition = now;
            } else if (now - key->last_transition >= LONG_PRESS_INTERVAL) {
                key->state = KEY_STATE_LONG_PRESS;
								dbg_printf(" long KEY_STATE_PRESSED\n");
								//test[2]='a'+num;
								//write_data1(test,10);

                // TODO: Handle long press event
            }
            break;

        case KEY_STATE_WAITING_DOUBLE_CLICK:
            if (pressed && now - key->last_transition <= DOUBLE_CLICK_INTERVAL) {
                key->state = KEY_STATE_PRESSED;
                key->waiting_double_click ++;
								//read_data(read_buf,10,&read_len);
								//dbg_printf("read_buf=%s\n",read_buf);

                // TODO: Handle double click event
            } 
						else if (now - key->last_transition > DOUBLE_CLICK_INTERVAL) {
								if(key->waiting_double_click)
										dbg_printf(" double click\n");
								else
									  dbg_printf(" single click\n");
                key->waiting_double_click = 0;
								key->state = KEY_STATE_RELEASED;
               	num+=1;
								if(num%2)show_cm();
								else show_tp();
							
            }
            break;

        case KEY_STATE_LONG_PRESS:
            if (!pressed) {
                key->state = KEY_STATE_RELEASED;
                key->last_transition = now;
								
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
            if(DC.percent>0)DC.percent -= 1; // Increment the angle if a falling edge is detected on E1

        }
        else if(pin == E2 && type == POL_FALLING)
        {
            if(DC.percent<100)DC.percent += 1; // Decrement the angle if a falling edge is detected on E2
        }
    }
    last_interrupt_time = interrupt_time;
}




void input_io_init(void)
{
    hal_gpio_pin_init(E1, GPIO_INPUT); 
    hal_gpio_pull_set(E1, STRONG_PULL_UP); 

    hal_gpio_pin_init(E2, GPIO_INPUT); 
    hal_gpio_pull_set(E2, STRONG_PULL_UP); 

    hal_gpio_pin_init(KEY_1, GPIO_INPUT); 
    hal_gpio_pull_set(KEY_1, STRONG_PULL_UP); 

    hal_gpio_pin_init(KEY_2, GPIO_INPUT); 
    hal_gpio_pull_set(KEY_2, STRONG_PULL_UP); 

    hal_gpio_pin_init(KEY_3, GPIO_INPUT); 
    hal_gpio_pull_set(KEY_3, STRONG_PULL_UP); 

    hal_gpio_pin_init(KEY_4, GPIO_INPUT); 
    hal_gpio_pull_set(KEY_4, STRONG_PULL_UP); 
//	
		hal_gpioin_register(E1, E12_event_handler, E12_event_handler);
		hal_gpioin_register(E2, E12_event_handler, E12_event_handler);
		
}

void user_exit_sleep(void)
{
	input_io_init();
	osal_start_timerEx(User_App_TaskID, user_app_event_id, 10);
}

void user_app_init(uint8_t task_id)
{
	User_App_TaskID=task_id;
	osal_set_event( User_App_TaskID, user_app_event_id );	
	osal_start_timerEx(User_App_TaskID, user_app_event_id, 10);
	input_io_init();
	pat_init();
	//hal_pwrmgr_register(MOD_USR1, common_sleep_enter, NULL);
	//	hal_pwrmgr_register(MOD_USR1, NULL, NULL);
	hal_pwrmgr_lock(MOD_USR1);
	DC.percent=0;
}

extern uint8_t low_power_mode;

uint16 user_app_ProcessEvent( uint8 task_id, uint16 events )
{
	static short angle=0;
	if ( events & user_app_event_id )
	{
		if(!low_power_mode)
		{
			uint32_t now = hal_systick();
      user_app_check_keys(now);
			osal_start_timerEx(User_App_TaskID, user_app_event_id, 10);
		}
		else
		{
			osal_stop_timerEx(User_App_TaskID, user_app_event_id);
		}
		return (events ^ user_app_event_id);
	}
	return 0;
}

