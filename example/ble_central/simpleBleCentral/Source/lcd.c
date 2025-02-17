#include "lcd.h"
#include "log.h"
#include "string.h"
#include "ll.h"
#include "PAT.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "pwrmgr.h"
#include "peripheral.h"
#include "buzz.h"
#include "simpleBLECentral.h"
#include "sf_i2c.h"
#include "user_app.h"
#include "protocol.h"
#include "storage.h"
#include "mixo_ots_driver.h"


#define CS0 GPIO_P34
#define RS GPIO_P00
#define LCD_SCL GPIO_P32
#define LCD_SDA GPIO_P33

#define SLAVE_LCD 0x78


// 功能函数声明
void standbyPage();

void volumeSettingPage();
void outWaterPage();
void timeSettingPage();
void myWarmWaterPage();
void myNormalWaterPage();
void stopWaterOut();


// 全局变量，存储上一次的时间
uint32_t last_hour_tens = 0;
uint32_t last_hour_units = 0;
uint32_t last_min_tens = 0;
uint32_t last_min_units = 0,sleep_time=0;
uint8_t low_power_mode=0,last_th=0,last_h=0,last_t=0,last_u=0;
uint8_t blink_h_enable=0;
uint8_t max_temperature=95; 
uint8_t show4data[4]={0};

const static uint8_t mh[]={
	

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x0E,0x0E,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,
0x00,0x00,0x00,0x03,0x03,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\微信图片_20241119142603.bmp",0*/

};


const static  uint8_t shuzi[][16*9] = {//24*42的图像

0x00,0x00,0xF8,0xF8,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xF8,0xF8,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x7F,0x7F,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,
0x60,0x60,0x60,0x60,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 553@1x.bmp",0*/



	

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xF8,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 544@1x.bmp",0*/




0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xF8,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0xF0,0xF0,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0x3F,0x3F,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x7F,0x7F,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,
0x60,0x60,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 550@1x.bmp",0*/




0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xF8,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,
0x60,0x60,0x60,0x60,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 543@1x.bmp",0*/





0x00,0x00,0xF8,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xF8,0xF8,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x3F,0x3F,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 544@1x.bmp",0*/



0x00,0x00,0xF8,0xF8,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x3F,0x3F,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,
0x60,0x60,0x60,0x60,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 551@1x.bmp",0*/




0x00,0x00,0xF8,0xF8,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xFF,0xFF,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0xF0,0xF0,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x7F,0x7F,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,
0x60,0x60,0x60,0x60,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 548@1x.bmp",0*/


0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xF8,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 552@1x.bmp",0*/



0x00,0x00,0xF8,0xF8,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xF8,0xF8,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0xFF,0xFF,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x7F,0x7F,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,
0x60,0x60,0x60,0x60,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 549@1x.bmp",0*/



0x00,0x00,0xF8,0xF8,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xF8,0xF8,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x3F,0x3F,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,
0x60,0x60,0x60,0x60,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\瀹瑰櫒 547@1x.bmp",0*/


};

const static  uint8_t flag_cm[] = {
	
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0xFC,0xFC,
0x7F,0x7F,0x01,0x01,0x01,0x7F,0x7F,0x01,0x01,0x01,0x7F,0x7F,0x00,0x00,0x7F,0x7F,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\selfml.BMP",0*/

};



const static  uint8_t wd[] = {
	

0x00,0x1C,0x14,0x1C,0x00,0x00,0xFC,0xFC,0x0C,0x0C,0x0C,0x0C,0x3C,0x3C,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x7F,0x60,0x60,0x60,0x60,0x60,0x60,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\selfml.BMP",0*/

};


const static  uint8_t connect_flag[] = {

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x60,0xC0,0xFE,0x44,0x28,0x10,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x06,0x03,0x7F,0x22,0x14,0x08,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\selfml.BMP",0*/

};

const static  uint8_t lowpower_flag[] =
{
	0x00,0x00,0x00,0x00,0x00,0xF8,0x08,0x0E,0x02,0x02,0x02,0x0E,0x08,0xF8,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x7F,0x40,0x50,0x50,0x50,0x50,0x50,0x40,0x7F,0x00,0x00,/*"D:\HS_Workspace\workspace\PHY_Dial\test\2像素数字\2鍍忕礌鏁板瓧\selfml.BMP",0*/
};

const static 	uint8_t zero[50]={0};
i2c_drv_struct_t lcd_i2c_drv =
{
    .scl = LCD_SCL,
    .sda = LCD_SDA,
};

i2c_drv_struct_t *dev=&lcd_i2c_drv;
void lcd_i2c_init(void)
{
	LOG("lcd i2c sfoft demo start...\n");
	hal_gpio_pin_init(RS,OEN);
	hal_gpio_pull_set(RS,STRONG_PULL_UP);
	
	hal_gpio_pin_init(LCD_SDA,OEN);
	hal_gpio_pin_init(LCD_SCL,OEN);
	hal_gpio_pull_set(LCD_SCL,STRONG_PULL_UP);
	hal_gpio_pull_set(LCD_SDA,STRONG_PULL_UP);

	hal_gpio_fast_write(LCD_SDA,1);
	hal_gpio_fast_write(LCD_SCL,1);

	LOG("lcd i2c sfoft demo start  ok...\n");
}




int32_t lcd_I2C_Read( uint8_t Reg, uint8_t *pBuff, uint16_t nBuffSize)
{

	i2c_read_multi_byte(&lcd_i2c_drv, SLAVE_LCD, Reg, pBuff, nBuffSize);
  return 0;
}


int32_t lcd_I2C_Write(uint8_t Reg, uint8_t *pBuff, uint16_t nBuffSize)
{
	

	i2c_write_multi_byte(&lcd_i2c_drv, SLAVE_LCD, Reg, pBuff, nBuffSize);

  return 0;
}

int32_t lcd_I2C_Write_Single(uint8_t Reg, uint8_t data)
{
	uint8_t temp[1]={data};
	
	i2c_write_multi_byte(&lcd_i2c_drv, SLAVE_LCD, Reg, temp, 1);
	
  return 0;
}

void write_com(uint8_t cmd)
{
    lcd_I2C_Write_Single(0x00, cmd);
}

void write_data(uint8_t dat)
{
    lcd_I2C_Write_Single(0x40, dat);
}


void lcd_address(unsigned char page,unsigned char column)
{
	column=column-1; //我们平常所说的第 1 列，在 LCD 驱动 IC 里是第 0 列。所以在这里减去 1.
	page=page-1;
	write_com(0xb0+page); //设置页地址。每页是 8 行。一个画面的 64 行被分成 8 个页。我们平常所说的第 1 页，在 LCD 驱动IC 里是第 0 页，所以在这里减去 1
	write_com(((column>>4)&0x0f)+0x10); //设置列地址的高 4 位
	write_com(column&0x0f); //设置列地址的低 4 位
	
	//上下倒置 左右倒置
	write_com(0xc9); // 0xc9上下反置 0xc8正常
	write_com(0xa1); // 0xa1左右反置 0xa0正常
}

void test(unsigned char data1,unsigned char data2)
{
	int i,j;
	for(i=0;i<8;i++)
	{
		lcd_address(i+1,1);
		for(j=0;j<64;j++)
		{
			write_data(data1);
			write_data(data2);
		}
	}
}


void test_ble(unsigned char data1,unsigned char data2)
{
	int i,j;
	for(i=0;i<8;i++)
	{
		lcd_address(i+1,1);
		for(j=8;j<64;j++)
		{
			write_data(data1);
			write_data(data2);
		}
	}
}

/*显示6x12点阵图像、汉字、生僻字或16x16点阵的其他图标*/
void hanzi_6x12(uint8_t page,uint8_t column,uint8_t reverse,const uint8_t *dp)
{
	uint8_t i,j;
	for(j=0;j<2;j++)
	{
		lcd_address(page+j,column);
		for (i=0;i<6;i++)
		{
			if(reverse==1)
			{
				write_data(*dp); /*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			}
			else
				write_data(~*dp);
			dp++;
		}
	}
}


void hanzi_16x12(uint8_t page,uint8_t column,uint8_t reverse,const uint8_t  *dp)
{
	uint8_t i,j;
	for(j=0;j<2;j++)
	{
		lcd_address(page+j,column);
		for (i=0;i<12;i++)
		{
			if(reverse==1)
			{
				write_data(*dp); /*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			}
			else
				write_data(~*dp);
			dp++;
		}
	}
}

//24x42

void hanzi_16x48(uint8_t page, uint8_t column, uint8_t reverse, const uint8_t *dp)
{
	uint8_t i, j;
	for(j = 0; j <6; j++)
	{
		lcd_address(page+j, column);
		for(i = 0; i < 24; i++)
		{
			if(reverse == 1)
			{
				write_data(*dp);
			}
			else
			{
				write_data(~*dp);
			}
			dp++;
		}
	}
}

void hanzi_16x48_zero(uint8_t page, uint8_t column, uint8_t reverse)
{
	uint8_t i, j;
	for(j = 0; j <6; j++)
	{
		lcd_address(page+j, column);
		for(i = 0; i < 24; i++)
		{
			if(reverse == 1)
			{
				write_data(0);
			}
			else
			{
				write_data(1);
			}
		}
	}
}

/*显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标*/
void hanzi_8x16(unsigned char page,unsigned char column,unsigned char reverse,const unsigned char  *dp)
{
	unsigned char i,j;
	for(j=0;j<2;j++)
	{
		lcd_address(page+j,column);
		for (i=0;i<8;i++)
		{
			if(reverse==1)
			{
				write_data(*dp); /*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			}
			else
				write_data(~*dp);
			dp++;
		}
	}
}



/*显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标*/
void hanzi_8x32(unsigned char page,unsigned char column,unsigned char reverse,const unsigned char  *dp)
{
	unsigned char i,j;
	for(j=0;j<6;j++)
	{
		lcd_address(page+j,column);
		for (i=0;i<8;i++)
		{
			if(reverse==1)
			{
				write_data(*dp); /*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			}
			else
				write_data(~*dp);
			dp++;
		}
	}
}

/*显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标*/
void hanzi_16x16(unsigned char page,unsigned char column,unsigned char reverse,const unsigned char  *dp)
{
	unsigned char i,j;
	for(j=0;j<2;j++)
	{
		lcd_address(page+j,column);
		for (i=0;i<16;i++)
		{
			if(reverse==1)
			{
				write_data(*dp); /*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			}
			else
				write_data(~*dp);
			dp++;
		}
	}
}
//OLED 显示模块初始化
void Init_SSD1315Z(void)
{
	lcd_i2c_init();
	hal_gpio_fast_write(RS, 1);
	WaitMs(30);
	hal_gpio_fast_write(RS, 0);
	WaitMs(160);
	hal_gpio_fast_write(RS, 1);
	WaitMs(80);
	LOG("reset finish!\n");
	write_com(0xAE); //Set Display Off
	write_com(0xD5); //Display divide ratio/osc. freq. mode
	write_com(0x80);
	write_com(0xA8); //Multiplex ration mode:63
	write_com(0x3F);
	write_com(0xD3); //Set Display Offset
	write_com(0x00);
	write_com(0x40); //Set Display Start Line
	write_com(0x8D); //DC-DC Control Mode Set
	write_com(0x14); //DC-DC ON/OFF Mode Set
	write_com(0xA0); //Segment Remap
	write_com(0xC0); //Set COM Output Scan Direction
	write_com(0xDA); //Set COM Pins Hardware Configuration
	write_com(0x12);
	write_com(0x81); //Contrast control
	write_com(0x80);

	write_com(0xD9); //Set pre-charge period
	write_com(0x22);
	write_com(0xDB); //VCOM deselect level mode
	write_com(0x40);
	write_com(0xA4); //Set Entire Display On/Off
	write_com(0xA6); //0xA6 Set Normal Display  A7反显
	test(0,0);
	show_data(0xFFFFF);
	

	write_com(0xAF); //Set Display On

	LOG("reset finish1!\n");
}


uint32_t last_toggle_time = 0;
#define toggle_interval  1000 // 200ms  1hz



void show_3flag(uint8_t ml_or_c,uint8_t mh_flag)
{
	static uint8_t ble_connecting_flag=0,temp_mh=0,ble_lp_flag=0;
//	if(get_ble_status()==2)
//	{
//		hanzi_16x16(1,114,1,connect_flag);
//	}
//	else 

	
	if((get_ble_status()==1)||(get_ble_status()==4))
	{
			uint32_t current_time = hal_systick();
		 if (current_time - last_toggle_time >= toggle_interval) {
        // 切换图标和状态
				if(ble_connecting_flag)hanzi_16x16(1,114,1,connect_flag);
						else hanzi_16x16(1,114,1,zero);
				ble_connecting_flag=!ble_connecting_flag;
        last_toggle_time = current_time;
				ble_lp_flag=0;
		}
		
	}
	else
	{
		if(Dis_DATA.bat_quantity&&(Dis_DATA.bat_quantity<3800))
		{
			if(ble_lp_flag==0)
			{
				hanzi_16x16(1,114,1,lowpower_flag);
			}
			ble_lp_flag=1;
		}
		else 
		{
			hanzi_16x16(1,114,1,zero);
			ble_lp_flag=0;
		}
	}
	
	static int previous_ml_or_c = -1; // 初始化为一个无效状态

	if (ml_or_c != previous_ml_or_c)
	{
			previous_ml_or_c = ml_or_c; // 更新前一个状态

			if (ml_or_c == TEMPERATURE_SETTING)
			{
					hanzi_16x16(5, 113, 1, zero);
					hanzi_16x16(5, 113, 1, wd);
			}
			else if (ml_or_c == VOLUME_SETTING)
			{
					hanzi_16x16(5, 113, 1, zero);
					hanzi_16x16(5, 113, 1, flag_cm);
			}
			else if (ml_or_c == STANDBY)
			{
					hanzi_16x16(5, 113, 1, zero);
					hanzi_16x16(5, 113, 1, zero);
			}
	}
	
	if(temp_mh!=mh_flag)
	{
		if(mh_flag)
		{
			hanzi_8x32(2,55,1,mh);
		}
		else
		{
			hanzi_8x32(2,55,1,zero);
		}
		temp_mh=mh_flag;
	}
}


void show_10Ddata(uint32_t ts)
{
    uint32_t th = ts / 1000; // 千位
    uint32_t h = (ts % 1000) / 100; // 百位
    uint32_t t = (ts % 100) / 10; // 十位
    uint32_t u = ts % 10; // 个位
    
    // 检查千位、百位、十位和个位是否有变化，如果有则更新显示

    if(th !=  show4data[0]) {
				if(th!=0)
				{
					hanzi_16x48(2,8,1,shuzi[th]);
				}
				else
				{
					hanzi_16x48_zero(2,8,1);
				}
				show4data[0] = th; // 更新存储的千位
    }

		
		
    if(h != show4data[1]) {
        hanzi_16x48(2,34,1,shuzi[h]);
        show4data[1] = h; // 更新存储的百位
    }
    if(t != show4data[2]) {
        hanzi_16x48(2,60,1,shuzi[t]);
        show4data[2] = t; // 更新存储的十位
    }
    if(u != show4data[3]) {
        hanzi_16x48(2,86,1,shuzi[u]);
        show4data[3] = u; // 更新存储的个位
    }
}


void show0C_data(uint32_t ts)
{
    static uint16_t count = 0;
    uint32_t h = ts / 10; // 十位
    uint32_t t = ts % 10; // 个位
    if(h != show4data[1]) {
        hanzi_16x48(2,37,1,shuzi[h]);
        show4data[1] = h; // 更新存储的百位
    }
    if(t != show4data[2]) {
        hanzi_16x48(2,63,1,shuzi[t]);
        show4data[2] = t; // 更新存储的十位
    }
}



void show_data(uint32_t ts)
{
    static uint16_t count = 0;
    uint32_t hours = ts / 60;
    uint32_t minutes = (ts % 60);

    uint32_t hour_tens = hours / 10;
    uint32_t hour_units = hours % 10;
    uint32_t min_tens = minutes / 10;
    uint32_t min_units = minutes % 10;

    if (blink_h_enable) {
        count++;
				if(count==1)
				{
					
					if(blink_h_enable==2)
					{
            	osal_memset(show4data,11,2);
					}
					else if(blink_h_enable==4)
					{
            	osal_memset(show4data+2,11,2);
					}
					else
					{
							osal_memset(show4data,11,4);
					}
					

				}
    } else {
        count = 0;
    }

    if (count < 6) {
        // 检查小时和分钟是否有变化，如果有则更新显示
        if (hour_tens != show4data[0]) {
            hanzi_16x48(2, 5, 1, shuzi[hour_tens]);
            show4data[0] = hour_tens; // 更新存储的小时十位
        }
        if (hour_units != show4data[1]) {
            hanzi_16x48(2, 31, 1, shuzi[hour_units]);
            show4data[1] = hour_units; // 更新存储的小时个位
        }
        if (min_tens != show4data[2]) {
            hanzi_16x48(2, 64, 1, shuzi[min_tens]);
            show4data[2] = min_tens; // 更新存储的分钟十位
        }
        if (min_units != show4data[3]) {
            hanzi_16x48(2, 90, 1, shuzi[min_units]);
            show4data[3] = min_units; // 更新存储的分钟个位
        }
    } else {
        if (count > 11) {
					if(blink_h_enable==2)
					{
            hanzi_16x48_zero(2, 5, 1);
            hanzi_16x48_zero(2, 31, 1);
					}
					else if(blink_h_enable==4)
					{
            hanzi_16x48_zero(2, 64, 1);
            hanzi_16x48_zero(2, 90, 1);
					}
					else
					{
						hanzi_16x48_zero(2, 5, 1);
            hanzi_16x48_zero(2, 31, 1);
						hanzi_16x48_zero(2, 64, 1);
            hanzi_16x48_zero(2, 90, 1);
					}
            count = 0;
        }
    }
}


void Delay(unsigned int dly)
{
    unsigned int i,j;
    for(i=0;i<dly;i++)
    	for(j=0;j<255;j++);
}



void OLED_DisPlay_On(void)
{
    write_com(0xAF); // 点亮屏幕
		write_com(0xAF); // 点亮屏幕
}

/**
 * @brief 关闭OLED显示，功耗 29 mW
 *
 */
void OLED_DisPlay_Off(void)
{
    write_com(0xAE); // 关闭屏幕
}

void lcd_enter_sleep(void)
{
	OLED_DisPlay_Off();
	hal_gpioretention_register(RS);
	hal_gpioretention_register(LCD_SCL);
	hal_gpioretention_register(LCD_SDA);
}

void lcd_exit_sleep(void)
{
		lcd_i2c_init();
		OLED_DisPlay_On();
		LOG("lcd_exit_sleep\n");
}

void common_sleep_enter(void)
{
	if(low_power_mode==2)
	{
		low_power_mode=1;
		update_enter_lowpower();
		lcd_enter_sleep();
		user_enter_sleep();
		//osal_set_event( LCD_TaskID, upd_param_timecnt_id );	
	}
}

void common_sleep_exit(void)
{
	if(low_power_mode==1)
	{
		LOG("exit\n");
		hal_pwrmgr_lock(MOD_USR1);
		low_power_mode=0;

		osal_start_timerEx(LCD_TaskID, standby_id, 100);
		osal_start_timerEx(LCD_TaskID, lcd_event_id, 50);
		user_exit_sleep();
		lcd_exit_sleep();

	}
}

void lcd_init(uint8_t task_id)
{
	LCD_TaskID=task_id;
	LOG("task_id=%d\n",task_id);
	hal_pwrmgr_register(MOD_USR1, common_sleep_enter, NULL);
	hal_pwrmgr_lock(MOD_USR1);
	Init_SSD1315Z();
	//buzz_pwm_init();
	
	osal_set_event( LCD_TaskID, lcd_event_id );	
	osal_start_timerEx(LCD_TaskID, lcd_event_id, 350);
}

uint16_t cnt_sleep=0;
extern Dial_Control DC;

void judge_Dial_standby_status(void)
{
	if(DC.percent>2)
	{
		osal_memset(show4data,11,4);
		turn_to_0c();
		Dis_DATA.setwaterVolume=500;
		Dis_DATA.currentTemperature=45;
		DC.percent=Dis_DATA.currentTemperature;
		max_temperature=65;
		UI.buttonCallbacks[1]=turn_to_confirm;
		UI.buttonCallbacks[2]=turn_to_standby;
		UI.buttonCallbacks[3]=turn_to_standby;
		UI.buttonCallbacks[0]=turn_to_standby;
	}
	else if(DC.percent<-2)
	{
		osal_memset(show4data,11,4);
		turn_to_0c();
		Dis_DATA.setwaterVolume=500;
		Dis_DATA.currentTemperature=65;
		DC.percent=Dis_DATA.currentTemperature;
		max_temperature=65;
		UI.buttonCallbacks[1]=turn_to_confirm;
		UI.buttonCallbacks[2]=turn_to_standby;
		UI.buttonCallbacks[3]=turn_to_standby;
		UI.buttonCallbacks[0]=turn_to_standby;
	}
}



void show_handle(void)
{
		static uint8_t temp_id=0;
		//page switch
	  if((temp_id!=UI.interfaceID))
		{
			test_ble(0,0);
//			hanzi_16x48_zero(2, 11, 1);
//			hanzi_16x48_zero(2, 35, 1);
//			hanzi_16x48_zero(2, 66, 1);
//			hanzi_16x48_zero(2, 90, 1);
			clear_dial_rawdata();
			if((TIMESETTING!=UI.interfaceID)&&(STANDBY!=UI.interfaceID))//冒号去掉条件
			{
				hanzi_8x32(2,55,1,zero);
			}
			else
			{
				hanzi_8x32(2,55,1,mh);
			}
//			
//			if(((TEMPERATURE_PREFERENCESETTING==UI.interfaceID)||(TEMPERATURE_SETTING==UI.interfaceID)))
//			{
//				hanzi_16x48_zero(2, 11, 1);
//				hanzi_16x48_zero(2, 90, 1);
//			}
		}
		temp_id=UI.interfaceID;//必须放这里
		//id show
    switch(UI.interfaceID)
    {
        case STANDBY:
            show_data(timestamp.hour * 60 + timestamp.minute);
						show_3flag(STANDBY,1);
						judge_Dial_standby_status();
            break;
				case TEMPERATURE_PREFERENCESETTING:
						store_setting.warm_vol=DC.percent;	
        case TEMPERATURE_SETTING:
            if (DC.percent < 45)DC.percent=45;
						if (DC.percent > max_temperature)DC.percent=max_temperature;
						Dis_DATA.setTemperature=DC.percent;
						show_3flag(TEMPERATURE_SETTING,0);
						show0C_data(DC.percent);
            break;
				case 	VOLUME_PREFERENCESETTING:
        case 	VOLUME_SETTING:
            if (DC.percent < 300)DC.percent=300;
						if (DC.percent > 5000)DC.percent=5000;
						Dis_DATA.setwaterVolume=DC.percent;
				    show_3flag(VOLUME_SETTING,0);
            show_10Ddata(DC.percent);
            break;
				case TEMPERATURESHOW:
					  show_3flag(TEMPERATURE_SETTING,0);
						show0C_data(Dis_DATA.currentTemperature);
						break;
				case VOLUMESHOW:
						show_3flag(VOLUME_SETTING,0);
						show_10Ddata(Dis_DATA.currentwaterVolume);
						break;
				case  TIMESETTING:
						if((DC.percent==0)&&(blink_h_enable!=4))
						{
							show_data(timestamp.hour * 60 + timestamp.minute);
						}
						else
						{
							if(blink_h_enable==2)
							{
								show_data(((temp_timestamp.hour + DC.percent + 24) % 24) * 60 + temp_timestamp.minute);
							}
							else 	if(blink_h_enable==4)
							{
								show_data((temp_timestamp.hour) * 60 + ((temp_timestamp.minute + DC.percent) % 60 + 60) % 60);
							}
						}
						show_3flag(STANDBY,1);
						break;
        default:
            break;
    }
		
}

void low_power_check(void)
{
	if((get_ble_status()!=0)&&(get_ble_status()!=2))
	{
		cnt_sleep=0;
		return ;
	}
	if(UI.interfaceID!=STANDBY)
	{
		cnt_sleep=0;
		return ;
	}
	cnt_sleep++;
}

uint16 lcd_ProcessEvent( uint8 task_id, uint16 events )
{
	static short angle=0xFFF,v_page_count=0,t_page_count=0;
	short temp_angle;

	if ( events & lcd_event_id )
	{
		if(!low_power_mode)
		{
			show_handle();
			osal_start_timerEx(LCD_TaskID, lcd_event_id, 50);
			if(cnt_sleep>2*60)
			{
				low_power_mode=2;
				osal_stop_timerEx(LCD_TaskID, lcd_event_id);
				cnt_sleep=0;
				//buzz_on();
				LOG("common_sleep=%d\n",cnt_sleep);
				hal_pwrmgr_unlock(MOD_USR1);
			}
			low_power_check();
		}
		return (events ^ lcd_event_id);
	}
	
	if ( events & standby_id )
	{
		//NVIC_SystemReset();
		update_exit_lowpower();
		return (events ^ standby_id);
	}
	

	
	if ( events & temperatureShow_id )
	{
		t_page_count++;
		
		if((Dis_DATA.isDispensing==0)&&(t_page_count>=5))
		{
			t_page_count=0;
			turn_to_standby();
			return (events ^ temperatureShow_id);
		}
		
	
		
		if(t_page_count<5)
		{
			
			osal_start_timerEx(LCD_TaskID, temperatureShow_id, 1200);
		}
		else
		{
			if(Dis_DATA.setwaterVolume==0)
			{
				t_page_count=0;
			}
			else
			{
				t_page_count=0;
				UI.interfaceID=VOLUMESHOW;
				osal_memset(show4data,11,4);
				osal_start_timerEx(LCD_TaskID, volumnShow_id, 1000);
			}
		}
	
		return (events ^ temperatureShow_id);
	}
	
	
	if ( events & volumnShow_id )
	{
		if(Dis_DATA.isDispensing)
		{

			osal_start_timerEx(LCD_TaskID, volumnShow_id, 500);
		}
		else
		{
			turn_to_standby();
		}
		return (events ^ volumnShow_id);
	}
	
	
	if ( events & showtimeout_id )
	{
		
		UI.tocnt-=1;
		if((UI.tocnt)&&(UI.interfaceID!=STANDBY))
		{
			osal_start_timerEx(LCD_TaskID, showtimeout_id, 1000);
		}
		else
		{
			turn_to_standby();
		}
		return (events ^ showtimeout_id);
	}
	
	return 0;
}




//page switch


// 定义全局变量
int temperature = 45; // 初始水温
int volume = 300; // 初始水量
bool isHeating = false; // 是否加热
int timeHour = 0; // 时间小时
int timeMinute = 0; // 时间分钟

//******************时间设置************************

void turn_to_timeset_finish(void)
{
	temp_timestamp.minute= ((temp_timestamp.minute + DC.percent) % 60 + 60) % 60;
	osal_memcpy(&timestamp,&temp_timestamp,sizeof(timestamp));
	turn_to_standby();
}


void turn_to_timeset_minute(void)
{
	temp_timestamp.hour=(temp_timestamp.hour + DC.percent + 24) % 24;
	osal_memcpy(&timestamp,&temp_timestamp,sizeof(timestamp));
	Dis_DATA.unit_weight=1;
	UI.interfaceID=TIMESETTING;
	blink_h_enable=4;
	osal_memset(show4data,11,4);
	DC.percent=0;
	UI.buttonCallbacks[0]=turn_to_timeset_finish;
	UI.buttonCallbacks[1]=turn_to_standby;
	UI.buttonCallbacks[2]=turn_to_standby;
	UI.buttonCallbacks[3]=turn_to_standby;
}


//******************************************


//******************喜好水量设置*************

void turn_to_95water(void)
{
	turn_to_0c();
	max_temperature=95;
	DC.percent=95;
	osal_memset(show4data,11,4);
	Dis_DATA.currentTemperature=95;
	Dis_DATA.setTemperature=95;
	Dis_DATA.setwaterVolume=500;
	UI.buttonCallbacks[1]=turn_to_confirm;
	UI.buttonCallbacks[2]=turn_to_standby;
	UI.buttonCallbacks[3]=turn_to_standby;
}

//喜好左右设置
void turn_hobby_setwaterout(void)
{
	Dis_DATA.setTemperature=0;
	turn_to_confirm();
}

void turn_hobby_normalwaterout(void)
{
	Dis_DATA.setTemperature=0;
	turn_to_confirm();
}


void turn_hobby_mlwater(void)
{
	turn_to_ml();
	DC.percent=500;
	max_temperature=95;
	Dis_DATA.currentwaterVolume=500;
	Dis_DATA.currentTemperature=95;
	UI.buttonCallbacks[1]=turn_hobby_setwaterout;
	UI.buttonCallbacks[3]=turn_hobby_normalwaterout;
	UI.buttonCallbacks[2]=turn_to_standby;
}

void turn_to_standby(void)
{
	//闪烁复位
	blink_h_enable=0;
	//温度限制复位
	max_temperature=65;
	//dc复位
	DC.percent=0;
	
	osal_memset(show4data,11,4);
	

	
	osal_stop_timerEx(LCD_TaskID, temperatureShow_id);
	osal_stop_timerEx(LCD_TaskID, volumnShow_id);
	osal_stop_timerEx(LCD_TaskID, showtimeout_id);
	osal_memset(&Dis_DATA,0x00,sizeof(Dispenser));
	Dis_DATA.unit_weight=1;
	
	UI.interfaceID=STANDBY;
	UI.displayCallback=volumeSettingPage;
	UI.DialCallback=NULL;
	UI.buttonCallbacks[0]=NULL;//待添加
	UI.buttonCallbacks[1]=set_warm_water_out;
	UI.buttonCallbacks[2]=turn_to_95water;//取水，范围45-95度
	UI.buttonCallbacks[3]=set_normal_water_out;
	
	UI.longbuttonCallbacks[0]=turn_to_hobby_first;
	UI.longbuttonCallbacks[1]=set_hobby_warm_water_out;//set_warm_water_out
	UI.longbuttonCallbacks[2]=start_discovery;
	UI.longbuttonCallbacks[3]=set_hobby_normal_water_out;
	UI.tocnt=0;
}


//normal process
void turn_to_ml(void)
{
	DC.percent=500;
  Dis_DATA.unit_weight=100;
	Dis_DATA.currentwaterVolume=500;
	osal_memset(show4data,11,4);
	LOG("turn to ml_set");
	
	UI.interfaceID=VOLUME_SETTING;
	UI.displayCallback=volumeSettingPage;
	UI.DialCallback=volumeSettingPage;
	UI.buttonCallbacks[0]=turn_to_0c;
	UI.tocnt=OP_TIME;
	osal_start_timerEx(LCD_TaskID, showtimeout_id, 1000);
}

void turn_to_0c(void)
{
	Dis_DATA.unit_weight=1;
	LOG("turn to temperature");
	UI.interfaceID=TEMPERATURE_SETTING;
	UI.displayCallback=volumeSettingPage;
	UI.DialCallback=NULL;
	UI.buttonCallbacks[0]=turn_to_ml;
	UI.tocnt=OP_TIME;
	osal_start_timerEx(LCD_TaskID, showtimeout_id, 1000);
}


//确定
void turn_to_confirm(void)
{
	LOG("turn to confirm");
	if(get_ble_status()!=2)
	{
		turn_to_standby();
		return ;
	}
	osal_stop_timerEx(LCD_TaskID, showtimeout_id);
	Dis_DATA.currentTemperature=0;
	Dis_DATA.currentwaterVolume=0;
	//water_switch_on();
	//UI.interfaceID=TemperatureShow;
	
	UI.interfaceID=TEMPERATURESHOW;
	if(Dis_DATA.setTemperature)
	{
		Dis_DATA.currentTemperature=Dis_DATA.setTemperature;
	}
	
	if(Dis_DATA.setTemperature==0)
	{
		Dis_DATA.currentTemperature=store_setting.defaultnormal_tmp;
	}
	osal_memset(show4data,11,4);

	osal_start_timerEx(LCD_TaskID, temperatureShow_id, 1000);
	osal_start_timerEx(User_App_TaskID, write_watervolumn_event_id, 50);
	
	
	UI.displayCallback=volumeSettingPage;
	UI.DialCallback=volumeSettingPage;
	UI.buttonCallbacks[0]=water_switch_off;
	UI.buttonCallbacks[1]=water_switch_off;
	UI.buttonCallbacks[2]=water_switch_off;
	UI.buttonCallbacks[3]=water_switch_off;
}


//hobby 
void turn_to_hobby_first(void)
{
	LOG("turn to hobby");
	DC.percent=0;
	clear_dial_rawdata();
	UI.interfaceID=TIMESETTING;
	osal_memset(show4data,11,4);
	blink_h_enable=2;
	osal_memcpy(&temp_timestamp,&timestamp,sizeof(timestamp));
	Dis_DATA.unit_weight=1;
		
	UI.displayCallback=volumeSettingPage;
	UI.DialCallback=volumeSettingPage;
	UI.buttonCallbacks[0]=turn_to_timeset_minute;
	UI.buttonCallbacks[1]=turn_to_warmhobby_second;
	UI.buttonCallbacks[2]=turn_to_standby;
	UI.buttonCallbacks[3]=turn_to_normalhobby_second;
	//UI.tocnt=OP_TIME;
	//osal_start_timerEx(LCD_TaskID, showtimeout_id, 1000);
}



void turn_to_normalhobby_second(void)
{
	LOG("turn to hobby=%d,%d\n",store_setting.normal_vol,store_setting.warm_vol);
	blink_h_enable=0;
	//water_switch_on();
	//UI.interfaceID=TemperatureShow;
	UI.interfaceID=VOLUME_PREFERENCESETTING;
	osal_memset(show4data,11,4);

	DC.percent=store_setting.normal_vol;

	Dis_DATA.unit_weight=100;
	UI.processID=2;
	UI.displayCallback=volumeSettingPage;
	UI.DialCallback=volumeSettingPage;
	UI.buttonCallbacks[0]=turn_to_hobby_finish;
	UI.buttonCallbacks[1]=turn_to_standby;
	UI.buttonCallbacks[2]=turn_to_standby;
	UI.buttonCallbacks[3]=turn_to_standby;
	UI.tocnt=OP_TIME;
	osal_start_timerEx(LCD_TaskID, showtimeout_id, 1000);
}

void turn_to_warmhobby_second(void)
{
	LOG("turn to hobby=%d,%d\n",store_setting.normal_vol,store_setting.warm_vol);
	blink_h_enable=0;
	//water_switch_on();
	//UI.interfaceID=TemperatureShow;
	UI.interfaceID=VOLUME_PREFERENCESETTING;
	osal_memset(show4data,11,4);
	UI.processID=1;
	DC.percent=store_setting.warm_vol;
	
	Dis_DATA.unit_weight=100;
	
	UI.displayCallback=volumeSettingPage;
	UI.DialCallback=volumeSettingPage;
	UI.buttonCallbacks[0]=turn_to_hobby_finish;
	UI.buttonCallbacks[1]=turn_to_standby;
	UI.buttonCallbacks[2]=turn_to_standby;
	UI.buttonCallbacks[3]=turn_to_standby;
	UI.tocnt=OP_TIME;
	osal_start_timerEx(LCD_TaskID, showtimeout_id, 1000);
}


void turn_to_hobby_finish(void)
{
	LOG("DC.turn_to_hobby_finish=%d",DC.percent);
	//water_switch_on();
	//UI.interfaceID=TemperatureShow;
	if(UI.processID==1)
	{
		store_setting.warm_vol=DC.percent;
	}
	else if(UI.processID==2)
	{
		store_setting.normal_vol=DC.percent;
	}
	
	UI.processID=0;
	save_setting(&store_setting);
	turn_to_standby();
}

//直出
void set_warm_water_out(void)
{
	Dis_DATA.setwaterVolume=0;
	if(store_setting.defaultwarm_tmp==0)
	{
		Dis_DATA.setTemperature=45;
	}
	else
	{
		Dis_DATA.setTemperature=store_setting.defaultwarm_tmp;
	}

	turn_to_confirm();
	
}


void set_normal_water_out(void)
{
	Dis_DATA.setTemperature=0;
	
	Dis_DATA.setwaterVolume=0;
	
	turn_to_confirm();

}

//喜好直出 长按
void set_hobby_warm_water_out(void)
{
	Dis_DATA.setwaterVolume=store_setting.warm_vol;
	LOG("store_setting.defaultwarm_tmp=%d\n",store_setting.defaultwarm_tmp);
	if((store_setting.defaultwarm_tmp>=45)&&(store_setting.defaultwarm_tmp<=55))
	{
		Dis_DATA.setTemperature=store_setting.defaultwarm_tmp;
	}
	else 
	{
		Dis_DATA.setTemperature=45;
	}
	
	
	if(Dis_DATA.setwaterVolume==0)
	{
		Dis_DATA.setwaterVolume=300;
	}


	turn_to_confirm();
}


void set_hobby_normal_water_out(void)
{
	Dis_DATA.setwaterVolume=store_setting.normal_vol;
	Dis_DATA.setTemperature=0;
	
	if(Dis_DATA.setwaterVolume==0)
	{
		Dis_DATA.setwaterVolume=300;
	}
	
	if(Dis_DATA.setwaterVolume!=0)
	{
		turn_to_confirm();
	}
}

void volumeSettingPage(void)
{
}

// 停止出水
void stopWaterOut() {
    LOG("出水已停止\n");
}