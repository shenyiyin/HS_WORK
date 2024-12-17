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

#ifndef _MXS_REG_ACCESS_H_
#define _MXS_REG_ACCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * HEADER FILES
 *---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*-----------------------------------------------------------------------------
 * MACRO DEFINITION
 *---------------------------------------------------------------------------*/
#ifdef _WIN32
#ifdef MXS_DLL_EXPORTS
#define MXS_API __declspec(dllexport)
#else
#define MXS_API
#endif
#else
#define MXS_API
#endif

/*-----------------------------------------------------------------------------
 * FUNCTIONS DEFINITION
 *---------------------------------------------------------------------------*/

/**
 * @brief mxs寄存器写入接口
 *
 * @param {uint8_t} regAddr
 * @param {uint8_t} regData
 *
 * @return {*}
 */
int mixo_ots_reg_write(uint8_t regAddr, uint8_t regData);

/**
 * @brief mxs寄存器读取接口
 *
 * @param {uint8_t} regAddr
 * @param {uint8_t} *regData
 *
 * @return {*}
 */
int mixo_ots_reg_read(uint8_t regAddr, uint8_t* regData);
void pat1_i2c_init(void);
#ifdef __cplusplus
}
#endif

#endif  //_MXS_REG_ACCESS_H_
