/**
  ******************************************************************************
  * @file    sf_i2c.c 
  * @author  Xiao Yang 260384793@qq.com
  * @version V1.0.0
  * @date    2021-10-06
  * @brief   This file realizes the software simulation IIC driver library.
  ******************************************************************************
  */

/*! Include--------------------------------------------------------------------*/
#include "sf_i2c.h"

#include <string.h>


i2c_drv_struct_t i2c_drv =
{
    .scl = I2Cx_SCL_PIN,
    .sda = I2Cx_SDA_PIN,
};


//-----------------------------------------------------------------------------
// i2c physical interface
//-----------------------------------------------------------------------------
// SDA pin output low level
// SDA pin output low level


// Read SDA pin level
#define I2C_PIN_SDA_READ(p_dev) \
                        hal_gpio_read(p_dev->sda)

// SDA pin input direction
#define I2C_PIN_SDA_DIR_INPUT(p_dev) \
                        do { \
                           hal_gpio_pin_init(p_dev->sda, GPIO_INPUT); \
                           hal_gpio_pull_set(p_dev->sda, STRONG_PULL_UP); \
                        } while (0)

// SDA pin output direction
#define I2C_PIN_SDA_DIR_OUTPUT(p_dev) \
                        do { \
                           hal_gpio_cfg_analog_io(p_dev->sda, Bit_DISABLE); \
                           hal_gpio_pin_init(p_dev->sda, GPIO_OUTPUT); \
                           hal_gpio_fast_write(p_dev->sda, 1); \
                        } while (0)




//-----------------------------------------------------------------------------
// linked list function
//-----------------------------------------------------------------------------
// Insert node into linked list
#define INSERT_INTO(node) do {                          \
                              node->next = head_handle; \
                              head_handle = node;       \
                          } while (0)
                        

/*! Private function----------------------------------------------------------*/
static void i2c_delay(const i2c_drv_struct_t *dev);
static void i2c_ack(const i2c_drv_struct_t *dev);
static void i2c_nack(const i2c_drv_struct_t *dev);

/* Private variables ---------------------------------------------------------*/
#if (I2C_OBJ_FIND > 0u)
static i2c_dev* head_handle = NULL;
#endif

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  i2c software delay function, used to control the i2c bus speed
 * @param  dev : Pointer to iic structure
 * @return none
 */
													
#define I2C_DELAY()         __NOP();__NOP();__NOP();_NOP();__NOP();__NOP();_NOP();__NOP();__NOP();__NOP();_NOP();__NOP();__NOP();
 
    
		
static void i2c_delay(const i2c_drv_struct_t *dev)
{
   // I2C_DELAY();
}



/**
 * @brief  Initialization i2c
 * @param  dev : Pointer to iic structure
 * @return none
 */
void i2c_init(void)
{
	i2c_drv_struct_t *dev=&i2c_drv;
	LOG("i2c sfoft demo start...\n");
	hal_gpio_pin_init(I2Cx_SDA_PIN,OEN);
	hal_gpio_pin_init(I2Cx_SCL_PIN,OEN);
	hal_gpio_pull_set(I2Cx_SCL_PIN,STRONG_PULL_UP);
	hal_gpio_pull_set(I2Cx_SDA_PIN,STRONG_PULL_UP);

//	hal_gpio_fast_write(I2Cx_SDA_PIN,1);
//	hal_gpio_fast_write(I2Cx_SCL_PIN,1);
	// Initialzation pin high level
	I2C_PIN_SDA_HIGH(dev);
	I2C_PIN_SCL_HIGH(dev);
	LOG("i2c sfoft demo start...\n");
}

/**
 * @brief  Generate start signal
 * @param  dev : Pointer to iic structure
 * @return none
 */

void i2c_start(const i2c_drv_struct_t *dev)
{
    I2C_PIN_SCL_HIGH(dev);
    I2C_PIN_SDA_HIGH(dev);
    i2c_delay(dev);
    I2C_PIN_SDA_LOW(dev);
    i2c_delay(dev);
    I2C_PIN_SCL_LOW(dev);
}


/**
 * @brief  Generate stop signal
 * @param  dev : Pointer to iic structure
 * @return none
 */
void i2c_stop(const i2c_drv_struct_t *dev)
{
    I2C_PIN_SCL_LOW(dev);
    I2C_PIN_SDA_LOW(dev);
    i2c_delay(dev);
    I2C_PIN_SCL_HIGH(dev);
    i2c_delay(dev);
    I2C_PIN_SDA_HIGH(dev);
    i2c_delay(dev);
}

/**
 * @brief  Generate response signal
 * @param  dev : Pointer to iic structure
 * @return none
 */
static void i2c_ack(const i2c_drv_struct_t *dev)
{
    I2C_PIN_SCL_LOW(dev);
    I2C_PIN_SDA_DIR_OUTPUT(dev);
    I2C_PIN_SDA_LOW(dev);
    i2c_delay(dev);
    I2C_PIN_SCL_HIGH(dev);
    i2c_delay(dev);
    I2C_PIN_SCL_LOW(dev);
}

/**
 * @brief  Generate no response signal
 * @param  dev : Pointer to iic structure
 * @return none
 */
static void i2c_nack(const i2c_drv_struct_t *dev)
{
    I2C_PIN_SCL_LOW(dev);
    I2C_PIN_SDA_DIR_OUTPUT(dev);
    I2C_PIN_SDA_LOW(dev);
    i2c_delay(dev);
    I2C_PIN_SDA_HIGH(dev);
    i2c_delay(dev);
    I2C_PIN_SCL_HIGH(dev);
    i2c_delay(dev);
    I2C_PIN_SCL_LOW(dev);
}

/**
 * @brief  Detection response signal
 * @param  dev         : Pointer to iic structure
 * @return SF_I2C_SUCCESS : Not error
 *         IIC_TIMEOUT : Timeout,Device response is not received
 */
static sf_i2c_err i2c_wait_ack(const i2c_drv_struct_t *dev)
{
    uint16_t wait_time = 255;
		
    I2C_PIN_SCL_LOW(dev);
    I2C_PIN_SDA_DIR_INPUT(dev);
    i2c_delay(dev);
    I2C_PIN_SCL_HIGH(dev);
    while (I2C_PIN_SDA_READ(dev)) {
        if ((wait_time--) == 0) {
            I2C_PIN_SDA_HIGH(dev);
            I2C_PIN_SDA_DIR_OUTPUT(dev);
            i2c_stop(dev);
            return SF_I2C_TIMEOUT;
        }
    }
    i2c_delay(dev);
    I2C_PIN_SCL_LOW(dev);
    I2C_PIN_SDA_HIGH(dev);
    I2C_PIN_SDA_DIR_OUTPUT(dev);
    return SF_I2C_SUCCESS;
}

/**
 * @brief  Write a byte data to the I2C bus
 * @param  dev         : Pointer to iic structure
 * @param  byte        : Data write to the iic bus
 * @return IIC_SUCCESS : Not error
 *         SF_I2C_TIMEOUT : Timeout,Device response is not received
 */
sf_i2c_err i2c_write_byte(const i2c_drv_struct_t *dev, uint8_t byte)
{
    uint8_t i;
    sf_i2c_err err;
    for (i = 0; i < 8; i++) {
        I2C_PIN_SCL_LOW(dev);
        i2c_delay(dev);
        if (byte & 0x80)
            I2C_PIN_SDA_HIGH(dev);
        else
            I2C_PIN_SDA_LOW(dev);
        I2C_PIN_SCL_HIGH(dev);
        i2c_delay(dev);
        byte <<= 1;
    }
    err = i2c_wait_ack(dev);
    return err;
}

/**
 * @brief  Read a byte data from the I2C bus
 * @param  dev : Pointer to iic structure
 * @param  ack : Indicates whether to return a response signal after data acceptance is completed
 * @return the data read on the iic bus
 */
uint8_t i2c_read_byte(const i2c_drv_struct_t *dev, uint8_t ack)
{
    uint8_t i, byte = 0;

    I2C_PIN_SDA_DIR_INPUT(dev);
    for (i = 0; i < 8; i++) {
        byte <<= 1;
        I2C_PIN_SCL_LOW(dev);
        i2c_delay(dev);
        I2C_PIN_SCL_HIGH(dev);
        if (I2C_PIN_SDA_READ(dev))
            byte |= 0x01;
        i2c_delay(dev);
    }
    if (ack)
        i2c_ack(dev);
    else
        i2c_nack(dev);
    return byte;
}

/**
 * @brief  i2c writes multiple bytes to a register consecutively
 * @param  dev Pointer : to iic structure
 * @param  slave_addr  : Device address
 * @param  reg_addr    : Register address
 * @param  pbuf        : Pointer to source buffer
 * @param  length      : The number of bytes that need to be write
 * @return IIC_SUCCESS : Not error
 *         IIC_TIMEOUT : Timeout,Device response is not received
 */
sf_i2c_err i2c_write_multi_byte(const i2c_drv_struct_t *dev, uint8_t slave_addr, 
                                 uint8_t reg_addr, uint8_t *pbuf, uint16_t length)
{
    uint8_t i;
    sf_i2c_err err;
    uint8_t *p = (uint8_t*)pbuf;

    i2c_start(dev);
    i2c_write_byte(dev, I2C_WRITE(slave_addr));
    i2c_write_byte(dev, reg_addr);

    for (i = 0; i < length; i++) {
        err = i2c_write_byte(dev, p[i]);
    }
    i2c_stop(dev);
    return err;
}

/**
 * @brief  i2c reads multiple bytes consecutively from a register
 * @param  dev          : Pointer to iic structure
 * @param  slave_addr   : Device address
 * @param  reg_addr     : Register address
 * @param  pbuf         : Pointer to target buffer
 * @param  length       : The number of bytes that need to be read
 * @return none
 */
void i2c_read_multi_byte(const i2c_drv_struct_t *dev, uint8_t slave_addr, 
                         uint8_t reg_addr, uint8_t* pbuf, uint16_t length)
{
    uint8_t i;
    uint8_t *p = (uint8_t*)pbuf;

    i2c_start(dev);
    i2c_write_byte(dev, I2C_WRITE(slave_addr));
    i2c_write_byte(dev, reg_addr);
    i2c_start(dev);
    i2c_write_byte(dev, I2C_READ(slave_addr));

    for (i = 0; i < length; i++) {
        if (i != (length - 1)) {
            p[i] = i2c_read_byte(dev, 1);
        } else {
            p[i] = i2c_read_byte(dev, 0);
        }
    }
    i2c_stop(dev);
}

/**
 * @brief  i2c writes multiple bytes to a register consecutively
 * @param  dev Pointer : to iic structure
 * @param  slave_addr  : Device address
 * @param  reg_addr    : Register address
 * @param  pbuf        : Pointer to source buffer
 * @param  length      : The number of bytes that need to be write
 * @return IIC_SUCCESS : Not error
 *         IIC_TIMEOUT : Timeout,Device response is not received
 */
sf_i2c_err i2c_write_multi_byte_16bit(const i2c_drv_struct_t *dev, uint8_t slave_addr, 
                                      uint16_t reg_addr, void *pbuf, uint16_t length)
{
    uint8_t i;
    sf_i2c_err err;
    uint8_t *p = (uint8_t*)pbuf;

    i2c_start(dev);
    i2c_write_byte(dev, I2C_WRITE(slave_addr));
    i2c_write_byte(dev, (uint8_t)(reg_addr >> 8));
    i2c_write_byte(dev, (uint8_t)reg_addr);

    for (i = 0; i < length; i++) {
        err = i2c_write_byte(dev, p[i]);
    }
    i2c_stop(dev);
    return err;
}

/**
 * @brief  i2c reads multiple bytes consecutively from a register
 * @param  dev          : Pointer to iic structure
 * @param  slave_addr   : Device address
 * @param  reg_addr     : Register address
 * @param  pbuf         : Pointer to target buffer
 * @param  length       : The number of bytes that need to be read
 * @return none
 */
void i2c_read_multi_byte_16bit(const i2c_drv_struct_t *dev, uint8_t slave_addr, 
                               uint16_t reg_addr, void *pbuf, uint16_t length)
{
    uint8_t i;
    uint8_t *p = (uint8_t*)pbuf;

    i2c_start(dev);
    i2c_write_byte(dev, I2C_WRITE(slave_addr));
    i2c_write_byte(dev, (uint8_t)(reg_addr >> 8));
    i2c_write_byte(dev, (uint8_t)reg_addr);
    i2c_start(dev);
    i2c_write_byte(dev, I2C_READ(slave_addr));

    for (i = 0; i < length; i++) {
        if (i != (length - 1)) {
            p[i] = i2c_read_byte(dev, 1);
        } else {
            p[i] = i2c_read_byte(dev, 0);
        }
    }
    i2c_stop(dev);
}

void stop_output(i2c_drv_struct_t *dev)
{
	I2C_PIN_SDA_DIR_INPUT(dev);
}