/**
  ******************************************************************************
  * @file    sf_i2c.h
  * @author  Xiao Yang 260384793@qq.com
  * @version V1.0.0
  * @date    2021-10-06
  * @brief   This file realizes the software simulation IIC driver library.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SF_I2C_H
#define __SF_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include -------------------------------------------------------------------*/
#include <stdint.h>
#include "types.h"
#include "gpio.h"
/* Public define -------------------------------------------------------------*/
#define I2C_WRITE(slave)            (slave & 0xFE) /*! I2C write operation    */
#define I2C_READ(slave)             (slave | 0x01) /*! I2C read operation     */

#define I2C_OBJ_FIND                1u             /*! Find i2c driver object */


#define I2C_PIN_SDA_LOW(p_dev) \
                       hal_gpio_fast_write(p_dev->sda, 0)

// SDA pin output high level
#define I2C_PIN_SDA_HIGH(p_dev) \
                        hal_gpio_fast_write(p_dev->sda, 1)
                                                                         
// SCL pin output low level
#define I2C_PIN_SCL_LOW(p_dev) \
                        hal_gpio_fast_write(p_dev->scl, 0);

// SCL pin output high level
#define I2C_PIN_SCL_HIGH(p_dev) \
                        hal_gpio_fast_write(p_dev->scl, 1)
												

typedef enum
{
    IS_8BIT = 0,
    IS_16BIT
} i2c_reg_addrmode_enum_t;

typedef enum
{
    HW_I2C = 0,
    SW_I2C
} i2c_type_enum_t;

typedef struct
{
    GPIO_Pin_e scl;
    GPIO_Pin_e sda;
} i2c_drv_struct_t;


/**
 * IIC error infomation
 */ 
typedef enum
{
    SF_I2C_SUCCESS = 0, /*! Not error   */
    SF_I2C_TIMEOUT,     /*! ack timeout */
} sf_i2c_err;

/**
 * IIC gpio optinos api
 */ 
typedef struct
{
    void (*sda_pin_out_low)(void);          /*! Set i2c sda pin low level     */
    void (*sda_pin_out_high)(void);         /*! Set i2c sda pin high level    */
    void (*scl_pin_out_low)(void);          /*! Set i2c scl pin low level     */
    void (*scl_pin_out_high)(void);         /*! Set i2c scl pin high level    */
    uint8_t (*sda_pin_read_level)(void);    /*! Read i2c sda pin level        */
    void (*sda_pin_dir_input)(void);        /*! Switch i2c sda pin dir input  */
    void (*sda_pin_dir_output)(void);       /*! Switch i2c sda pin dir output */
} i2c_port;

/**
 * I2C driver object
 */
typedef struct i2c_obj
{
    i2c_port port;          /*! i2c port interface    */
    uint32_t speed;         /*! control i2c bus speed */
#if (I2C_OBJ_FIND > 0u)     /*! if enable boject find */
    const char *name;       /*! i2c driver name       */
    struct i2c_obj *next;   /*! For the linked list   */
#endif
} i2c_dev;

/* Exported functions --------------------------------------------------------*/
void        i2c_init(void);
void        i2c_start(const i2c_drv_struct_t *dev);
void        i2c_stop(const i2c_drv_struct_t *dev);
void stop_output(i2c_drv_struct_t *dev);
#if (I2C_OBJ_FIND > 0u)
i2c_dev*    i2c_obj_find(const char* dev_name);
#endif
sf_i2c_err  i2c_write_byte(const i2c_drv_struct_t *dev, uint8_t byte);
uint8_t     i2c_read_byte(const i2c_drv_struct_t *dev, uint8_t ack);
sf_i2c_err  i2c_write_multi_byte(const i2c_drv_struct_t *dev, uint8_t slave_addr, 
                                 uint8_t reg_addr, uint8_t *pbuf, uint16_t length);
void        i2c_read_multi_byte(const i2c_drv_struct_t *dev, uint8_t slave_addr, 
                                uint8_t reg_addr, uint8_t *pbuf, uint16_t length);
sf_i2c_err  i2c_write_multi_byte_16bit(const i2c_drv_struct_t *dev, uint8_t slave_addr, 
                                       uint16_t reg_addr, void *pbuf, uint16_t length);
void        i2c_read_multi_byte_16bit(const i2c_drv_struct_t *dev, uint8_t slave_addr, 
                                      uint16_t reg_addr, void *pbuf, uint16_t length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __SF_I2C_H */