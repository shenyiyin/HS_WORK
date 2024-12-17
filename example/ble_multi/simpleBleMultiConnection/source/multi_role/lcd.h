#ifndef LCD_H
#define LCD_H
#include "types.h"

#define lcd_event_id 0x0001


#define  FONT_W  16
#define  FONT_H  8

#define BLUE   0x001F
#define WHITE  0xFFFF
#define BLACK  0x0000
#define ROW  64		   
#define COL  128	


void show_tp(void);
void show_cm(void);

void EnterSLP(void);
void lcd_init(uint8_t task_id);
uint16 lcd_ProcessEvent( uint8 task_id, uint16 events );
void DispStr(unsigned char *str,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);
void DispInt(unsigned int i,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);
void common_sleep_exit(void);
#endif