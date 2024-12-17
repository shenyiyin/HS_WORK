#ifndef USER_APP_H
#define USER_APP_H
#include "types.h"

#define user_app_event_id 0x0002



typedef struct _Dial_Control {
	  uint8_t direction;
		uint8_t percent;
	  uint8_t data;
		uint8_t control_id;
} Dial_Control;


typedef enum {
	 TEMPERATURE_SETTING, 
	 VOLUME_SETTING, 
	 TIME_SETTING
}Setting_Mode;
 

typedef struct {
    int currentTemperature; // 当前水温
    int waterVolume; // 需要的水量
    //struct tm currentTime; // 当前时间，使用C库的tm结构体
    bool isDispensing; // 是否正在出水
    Setting_Mode mode; // 当前设置模式
} Dispenser;


void user_exit_sleep(void);
void user_enter_sleep(void);
void user_app_init(uint8_t task_id);
uint16 user_app_ProcessEvent( uint8 task_id, uint16 events );

#endif