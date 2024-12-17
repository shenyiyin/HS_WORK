#ifndef PAT_H
#define PAT_H

#include "gpio.h"
#include "error.h"
#include "log.h"
#include "types.h"
#include "clock.h"
#include "stdio.h"

#define ONE_ROUND_X_NUM 20000*2.85//one round 360
#define INTERVAL_SCALE 350	  
#define SLAVE_OWN_ADDRESS                        0x75 << 1

#define Motion_PIN                              P2
#define Motion_GPIO_PORT                        GPIOB
#define Motion_EXTI_IRQn                        GPIOB_IRQn
#define Motion_IRQHandler(void)                 GPIOB_IRQHandler(void)
#define Motion_EXTI_LINE                        LL_EXTI_LINE_PB4
#define Motion_EXTI_LINE_TRIGGER                LL_EXTI_TRIGGER_BOTH_EDGE
#define Motion_PUSHED                          ((uint32_t)(1))
#define Motion_WAKEUP                           WAKEUP_PB4


#define I2Cx                          I2C1

#define I2Cx_SCL_PORT                 GPIOA
#define I2Cx_SCL_PIN                  P14
#define I2Cx_SCL_AF                   LL_GPIO_AF_0

#define I2Cx_SDA_PORT                 GPIOB
#define I2Cx_SDA_PIN                  P11
#define I2Cx_SDA_AF                   LL_GPIO_AF_4

#define I2Cx_IRQn                     I2C1_IRQn
#define I2Cx_IRQHandler               I2C1_IRQHandler


#define LL_I2Cx_SCL_EnableClock()     LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOA)
#define LL_I2Cx_SDA_EnableClock()     LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOB)
#define LL_I2Cx_EnableClock()         LL_APB1_EnableClock(LL_APB1_PERIPH_I2C1)

typedef struct _MOVE_STATE_
{
	uint8_t IsNormal;	  
	uint8_t IsChange;	  
	short Move_X;		  
	short Move_Y;		  
	int32_t X_Place;	  
	int32_t Y_Place;	  
	short Move_X_500ms; 
	short Move_Y_500ms; 
} MOVE_STATE;



void pat_init(void);
short angle_get(void);
int32_t PAT_I2C_Read( uint8_t Reg, uint8_t *pBuff, uint16_t nBuffSize);
int32_t PAT_I2C_Write(uint8_t Reg, uint8_t *pBuff, uint16_t nBuffSize);
int32_t PAT_I2C_Write_Single(uint8_t Reg, uint8_t data);
void PTA_RegWriteRead(uint8_t address, uint8_t wdata);

void Rotate_Sensor_Into_Lowpower(void);
void Rotate_Sensor_Out_Lowpower(void);
#endif