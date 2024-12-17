/*
 * Copyright (c) 2024 MixoSense Technology Ltd <contact@mixosense.com>.
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by MixoSense Technology
 * Ltd.
 *
 */

/*-----------------------------------------------------------------------------
 * HEADER FILES
 *---------------------------------------------------------------------------*/
#include "mixo_ots_access.h"
#include "sf_i2c.h"
#include "lcd.h"
#include "log.h"
#define mot_PIN                  P2
#define I2C1_SCL_PIN                  P14
#define I2C1_SDA_PIN                  P11
#define SLAVE1_OWN_ADDRESS                  0x74<<1

extern uint16_t cnt_sleep;
static void Motion_event_handler1(gpio_pin_e pin, IO_Wakeup_Pol_e type)
{
    static uint32_t last_interrupt_time = 0;
		if((type == POL_FALLING))
		{
			cnt_sleep=0;
			//dbg_printf("hx\n");
			common_sleep_exit();
			hal_gpio_pin_init(mot_PIN, GPIO_INPUT); 
			hal_gpio_pull_set(mot_PIN, STRONG_PULL_UP); 
			hal_gpioin_register(mot_PIN, Motion_event_handler1, Motion_event_handler1);
			
			hal_gpio_pin_init(I2C1_SDA_PIN,OEN);
			hal_gpio_pin_init(I2C1_SCL_PIN,OEN);
			hal_gpio_pull_set(I2C1_SCL_PIN,STRONG_PULL_UP);
			hal_gpio_pull_set(I2C1_SDA_PIN,STRONG_PULL_UP);

			hal_gpio_fast_write(I2C1_SDA_PIN,0);
			hal_gpio_fast_write(I2C1_SCL_PIN,0);
		}
}


void pat1_i2c_init(void)
{
	
	hal_gpio_pin_init(mot_PIN, GPIO_INPUT); 
  hal_gpio_pull_set(mot_PIN, STRONG_PULL_UP); 
	hal_gpioin_register(mot_PIN, Motion_event_handler1, Motion_event_handler1);
	
	hal_gpio_pin_init(I2C1_SDA_PIN,OEN);
	hal_gpio_pin_init(I2C1_SCL_PIN,OEN);
	hal_gpio_pull_set(I2C1_SCL_PIN,STRONG_PULL_UP);
	hal_gpio_pull_set(I2C1_SDA_PIN,STRONG_PULL_UP);

	hal_gpio_fast_write(I2C1_SDA_PIN,0);
	hal_gpio_fast_write(I2C1_SCL_PIN,0);
}


i2c_drv_struct_t pat_i2c_drv =
{
    .scl = I2C1_SCL_PIN,
    .sda = I2C1_SDA_PIN,
};


int mixo_ots_reg_read(uint8_t regAddr, uint8_t* regData)
{
	
    /* 此为寄存器读接口，需要从regAddr寄存器地址获取一字节数据，并存入regData指向的变量，具体实现由客户自行定义 */
	i2c_read_multi_byte(&pat_i2c_drv, SLAVE1_OWN_ADDRESS, regAddr, regData, 1);
//	hal_gpio_fast_write(I2C1_SDA_PIN,0);
//	hal_gpio_fast_write(I2C1_SCL_PIN,0);
	return 0;
}

int mixo_ots_reg_write(uint8_t regAddr, uint8_t regData)
{
	uint8_t temp[1]={regData};
    /* 此为寄存器写接口，需要将一字节的regData写入regAddr寄存器地址，具体实现由客户自行定义 */
	i2c_write_multi_byte(&pat_i2c_drv, SLAVE1_OWN_ADDRESS, regAddr, temp, 1);
//	hal_gpio_fast_write(I2C1_SDA_PIN,0);
//	hal_gpio_fast_write(I2C1_SCL_PIN,0);
	
	return 0;
}

