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

#ifndef _MXS_DRIVER_H_
#define _MXS_DRIVER_H_

#include <stdint.h>

typedef struct
{
    int16_t dx;
    int16_t dy;
} raw_data_t;

int mixo_ots_init(void);
int mixo_ots_get_mot_status(uint8_t* mot_status);
int mixo_ots_get_raw_data(raw_data_t* raw_data);
void mixo_ots_forced_clear_int(void);
void mixo_ots_enter_suspend(uint8_t true_or_false);

int user_mixo_ots_get_raw_data(short *dx, short *dy);

short get_motion_angle(void);
void clear_dial_rawdata(void);

#endif  //_MXS_DRIVER_H_
