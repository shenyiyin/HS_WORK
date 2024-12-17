#ifndef USER_APP_H
#define USER_APP_H
#include "types.h"
#include "att.h"
#define OTA_MODE_SELECT_REG 0x4000f034
#define DIAL_STEP 50


#define user_app_event_id 0x0001
#define user_notifyenable_event_id 0x0002
#define user_req_timestamp_event_id 0x0004
#define count1s_event_id 0x0001<<3

#define write_watertempeature_event_id 0x0001<<4
#define write_watervolumn_event_id 0x0001<<5
#define write_switchOn_event_id 0x0001<<6

#define user_req_defaultwarmwater_event_id 0x0001<<7
#define adcMeasureTask_EVT 0x0001<<8
#define write_switchOff_event_id 0x0001<<9
#define connect_timeout_event_id 0x0001<<10


typedef struct _Dial_Control {
	  uint8_t direction;
		int16 percent;
	  uint8_t data;

} Dial_Control;

typedef enum {
	 TEMPERATURE1_SETTING, 
	 VOLUME1_SETTING, 
	 TIME_SETTING
}Setting_Mode;
 

typedef struct {
		int bat_quantity ; // 当前水温
	  int setTemperature; // 当前水温
    int setwaterVolume; // 需要的水量
    int currentTemperature; // 当前水温
    int currentwaterVolume; // 需要的水量
	  uint16_t normal_temp_common_volume;
    uint16_t hot_temp_common_volume;
		uint16_t unit_weight;
    //struct tm currentTime; // 当前时间，使用C库的tm结构体
    bool isDispensing; // 是否正在出水
    Setting_Mode mode; // 当前设置模式
} Dispenser;




// 定义一个回调函数类型
typedef void (*ButtonCallback)(void);

// 定义界面结构体
typedef struct {
    int interfaceID;
		int tocnt;
    ButtonCallback buttonCallbacks[4];
		ButtonCallback longbuttonCallbacks[4];
    void (*displayCallback)(void);
	  void (*DialCallback)(void);
	  uint8_t processID;
} U_Interface;

extern U_Interface UI;
extern uint8 User_App_TaskID; 
extern Dispenser Dis_DATA;


void enter_ota(void);
void req_timestamp(void);
void user_exit_sleep(void);
void user_enter_sleep(void);
void user_app_init(uint8_t task_id);
bStatus_t water_switch_on(void);
void water_switch_off(void);
bStatus_t water_volumn_set(void);
bStatus_t water_temperature_set(void);
void req_default_warm_water(void);
uint16 user_app_ProcessEvent( uint8 task_id, uint16 events );

#endif