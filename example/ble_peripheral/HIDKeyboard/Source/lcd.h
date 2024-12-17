#ifndef LCD_H
#define LCD_H
#include "types.h"

#define APP_VERSION 101

#define lcd_event_id 0x0001
#define ble_flag_id 0x0002
#define standby_id 0x0004
#define temperatureShow_id 0x0001<<3
#define volumnShow_id 0x0001<<4
#define showtimeout_id 0x0001<<5
#define upd_param_timecnt_id 0x0001<<6

#define  FONT_W  16
#define  FONT_H  8

#define BLUE   0x001F
#define WHITE  0xFFFF
#define BLACK  0x0000
#define ROW  64		   
#define COL  128	
#define OP_TIME 5//操作超时时间

static uint8 LCD_TaskID;   // Task ID for internal task/event processing

// 定义界面枚举类型
typedef enum {
    STANDBY,
    TEMPERATURE_SETTING,
    VOLUME_SETTING,
		TEMPERATURESHOW,
		VOLUMESHOW,
	  VOLUME_PREFERENCESETTING,
		TEMPERATURE_PREFERENCESETTING,
		TIMESETTING
} Screen_Status;


void show_data(uint32_t ts);
void Init_SSD1315Z(void);
void EnterSLP(void);
void lcd_init(uint8_t task_id);
uint16 lcd_ProcessEvent( uint8 task_id, uint16 events );
void DispStr(unsigned char *str,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);
void DispInt(unsigned int i,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);
void common_sleep_exit(void);
void write_com(uint8_t cmd);

void OLED_DisPlay_Off(void);
void OLED_DisPlay_On(void);
void turn_to_ml(void);
void turn_to_0c(void);
void turn_to_confirm(void);
void turn_to_standby(void);
void turn_to_hobby_first(void);
void turn_to_warmhobby_second(void);
void turn_to_normalhobby_second(void);
void turn_to_hobby_finish(void);
void turn_to_normalhobby_first(void);

void set_hobby_normal_water_out(void);
void set_hobby_warm_water_out(void);

void turn_to_timeset_hour(void);
void turn_to_timeset_minute(void);

void set_warm_water_out(void);
void set_normal_water_out(void);
#endif