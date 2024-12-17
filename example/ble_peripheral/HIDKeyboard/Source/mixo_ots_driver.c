#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "mixo_ots_access.h"
#include "mixo_ots_driver.h"
#include <stdint.h>
#include "types.h"
#include "gpio.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "pwrmgr.h"
//#include <math.h>

#define DIAL_STEP 1000
#define ROUND 14800//360
#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : -(x))
#endif

// macro definitions about registers data
#define REG_PROTECT_ID_HIGH 0x58
#define REG_PROTECT_ID_LOW  0x5B

/**
 * @brief 接收1byte数据
 * @param[in] original_dx   低8bit的dx数据
 * @param[in] original_dy   低8bit的dy数据
 * @param[in] high_bit      高4bit的dx/dy数据
 * @param[out] dx           合成后的dx数据
 * @param[out] dx           合成后的dy数据
 * */
static void data_synthesis(uint8_t original_dx,
                           uint8_t original_dy,
                           uint8_t high_bit,
                           int16_t* dx,
                           int16_t* dy)
{
    // move high 4 bits to bit 11 ~ bit 8
    int16_t dx_high = (high_bit << 4) & 0x0F00;
    int16_t dy_high = (high_bit << 8) & 0x0F00;

    // sign extension
    if (dx_high & 0x0800)
    {
        dx_high |= 0xF000;
    }
    if (dy_high & 0x0800)
    {
        dy_high |= 0xF000;
    }

    // move low 8 bits to bit 7 ~ bit 0
    *dx = dx_high | (int16_t)original_dx;
    *dy = dy_high | (int16_t)original_dy;

    // overflow of 12-bit data
    *dx = (*dx == 2048) ? 0 : *dx;
    *dy = (*dy == 2048) ? 0 : *dy;
}

int mixo_ots_get_mot_status(uint8_t* mot_status)
{
    int ret                = 0;
    uint8_t tmp_mot_status = 0x00;
    ret                    = mixo_ots_reg_read(0x02, &tmp_mot_status);  // get MOTION STATUS
    if (ret != 0) goto Exit;
    *mot_status = tmp_mot_status;

Exit:
    return ret;
}

int mixo_ots_get_raw_data(raw_data_t* raw_data)
{
    int ret = 0;

    uint8_t tmp             = 0x00;
    uint8_t dx_low          = 0x00;
    uint8_t dy_low          = 0x00;
    uint8_t dy_dy_high      = 0x00;

    if (raw_data == NULL)
    {
        LOG("null `raw_data`!");
        ret = -1;
        goto Exit;
    }

    ret = mixo_ots_reg_read(0x12, &dy_dy_high);     // get Delta X and Delta Y high 4-bits
    if (ret != 0) goto Exit;
    ret = mixo_ots_reg_read(0x03, &dx_low);         // get Delta X low 8-bits
    if (ret != 0) goto Exit;
    ret = mixo_ots_reg_read(0x04, &dy_low);         // get Delta Y low 8-bits
    if (ret != 0) goto Exit;

    data_synthesis(
        dx_low, dy_low, dy_dy_high, &(raw_data->dx), &(raw_data->dy));

    // If overflowed, clear out the data
    if ((ABS(raw_data->dx) == 2047) || (ABS(raw_data->dy) == 2047))
    {
        LOG("dy/dx overflowed.");
        mixo_ots_reg_read(0x02, &tmp);
        mixo_ots_reg_read(0x02, &tmp);
			raw_data->dx=0;
			raw_data->dy=0;
    }

Exit:
    return ret;
}

int user_mixo_ots_get_raw_data(short *dx, short *dy)
{
    int ret = 0;

    uint8_t tmp             = 0x00;
    uint8_t dx_low          = 0x00;
    uint8_t dy_low          = 0x00;
    uint8_t dy_dy_high      = 0x00;

		
    ret = mixo_ots_reg_read(0x12, &dy_dy_high);     // get Delta X and Delta Y high 4-bits

    if (ret != 0) goto Exit;
    ret = mixo_ots_reg_read(0x03, &dx_low);         // get Delta X low 8-bits
    if (ret != 0) goto Exit;
    ret = mixo_ots_reg_read(0x04, &dy_low);         // get Delta Y low 8-bits
    if (ret != 0) goto Exit;
		dbg_printf("dy/dx overflowed.=%d,%d\n",dx_low,dy_low);
    data_synthesis(
        dx_low, dy_low, dy_dy_high, (dx), (dy));
		
    // If overflowed, clear out the data
    if ((ABS(*dx) == 2047) || (ABS(*dy) == 2047))
    {
        dbg_printf("dy/dx overflowed.");
        mixo_ots_reg_read(0x02, &tmp);
        mixo_ots_reg_read(0x02, &tmp);
    }

Exit:
    return ret;
}


/* 强制拉高MOT电平 */
void mixo_ots_forced_clear_int(void)
{
    uint8_t tmp = 0x00;

    mixo_ots_reg_read(0x02, &tmp);
    mixo_ots_reg_read(0x02, &tmp);
}

/* 进入/退出Suspend模式 */
void mixo_ots_enter_suspend(uint8_t true_or_false)
{
    uint8_t tmp = 0;

	// 将数据从寄存器中读出后进行位操作，再重新写回
    if (true_or_false)
    {
        // Bit 3 写 1， 进入Suspend Mode
        mixo_ots_reg_read(0x06, &tmp);
        mixo_ots_reg_write(0x06, (tmp | 0x08));
    }
	else
    {
        // Bit 3 写 0， 退出Suspend Mode
        mixo_ots_reg_read(0x06, &tmp);
        mixo_ots_reg_write(0x06, (tmp & 0xF7));
    }
}


int callisto_2x_init(void)
{
    int ret         = 0;
    int read_id_ok  = 0;
    uint8_t id_high = 0x00;
    uint8_t id_low  = 0x00;
    uint8_t tmp_read      = 0x00;
    uint8_t sensor_option = 0x04;  // sensor_option default is 0x04

    LOG("CONFIG_CALLISTO_2X\n");
    mixo_ots_reg_write(0x09, 0x5A);     // write_protect, WO, Disable write protect
    mixo_ots_reg_write(0x3C, 0xAC);     // bank_switch, WO, Banck switch
    mixo_ots_reg_write(0x09, 0xA5);     // write_protect, WO, Disable write protect
    mixo_ots_reg_write(0x6D, 0x39);     // read_protect, WO, Disable read protect
    mixo_ots_reg_read(0x00, &id_high);  // Read chip-id high byte
    mixo_ots_reg_read(0x01, &id_low);   // Read chip-id low byte
    if (id_high == REG_PROTECT_ID_HIGH && id_low == REG_PROTECT_ID_LOW)
    {
        read_id_ok = 1;
    }

    if (read_id_ok)
    {
        mixo_ots_reg_write(0x19,
                           sensor_option);  // sensor_option default is 0x04, WO, Enable 12bit mode; Disable x-y swap
        mixo_ots_reg_write(0x3E, 0x71);  // comm_mode, WO, Switch to LED CC mode, and bypass AVDD
        mixo_ots_reg_write(0x48, 0x0F);  // led_current, WO, Control LDP driven current
        mixo_ots_reg_write(0x1F, 0xFE);  // led_current, WO, Control LDP driven current
        mixo_ots_reg_write(0x73, 0x10);  // 2a3_motion, WO, To be compatible with 2A3 module
		mixo_ots_reg_write(0x69, 0x84);
        mixo_ots_reg_write(0x68, 0x7F);  // col_counter, WO, Adjust the reference voltage for the Bandgap output; Adjust
                                         // the output voltage of the 1.8V in sleep mode
        mixo_ots_reg_write(0x7D, 0x60);  // trb_osch, WO, Adjust the output frequency of OSCH
        mixo_ots_reg_write(0x60, 0x00);  // col_bias_and_output_drive, WO, Adjust the magnitude of the bias current in
                                         // the sensor column; Adjust the sensor output drive strength
        mixo_ots_reg_write(0x6E, 0x17);  //VCM_Voltage_Ctrl
        mixo_ots_reg_write(0x3F, 0x01);  //Switch_EXternal_Reg
        mixo_ots_reg_write(0x52, 0x2E);  //Exposure_sensitivity_And_PGAB_Max_value
        mixo_ots_reg_write(0x3F, 0x00);  //Switch_EXternal_Reg
			
			
			
			
			  mixo_ots_reg_write(0x60, 0x77);  //Switch_EXternal_Reg
        mixo_ots_reg_write(0x7e, 0x0c);  //Exposure_sensitivity_And_PGAB_Max_value
        mixo_ots_reg_write(0x7d, 0x00);  //Switch_EXternal_Reg
        // TO DO, if need calibration
    }
		
    if (read_id_ok)
    {
        mixo_ots_reg_write(0x0D, 0x0c);  // If cpi value is not assigned, use default value
        mixo_ots_reg_write(0x0E, 0x0c);  // If cpi value is not assigned, use default value
//				mixo_ots_reg_write(0x0D, 0x20);  // If cpi value is not assigned, use default value
//        mixo_ots_reg_write(0x0E, 0x20);  // If cpi value is not assigned, use default value
        /**
         * Enable 12bit mode Read the original value
         * If swap_xy is true, swap dx and dy
         * If inv_x is true, invert dx's output
         * If inv_y is true, invert dy's output
         */
        mixo_ots_reg_read(0x19, &tmp_read);
        if (tmp_read != sensor_option)
        {
            LOG("sensor_option init failed!(The expected %02X, and the actual %02X)\r\n", sensor_option, tmp_read);
        }

        ret = 0;
    }
    else
    {
        ret = -1;
        LOG("callisto_2x init failed!\r\n");
    }

    return ret;
}



int mixo_ots_init(void)
{
    int rc             = 0;
    uint8_t HardWareId = 0;
		WaitMs(50);
		pat1_i2c_init();
    WaitMs(50);
		mixo_ots_enter_suspend(0);
    mixo_ots_reg_read(0x1B, &HardWareId);
		dbg_printf("Address: %x, Data: %x\n", 0x1B, HardWareId);
		//rc = callisto_2x_init();
    switch (HardWareId)
    {
        case 0xA1:
            rc = callisto_2x_init();
            break;
        case 0xB1:
            rc = callisto_2x_init();
            break;
        case 0xC1:
            rc = callisto_2x_init();
            break;
        default:
            dbg_printf("OTS Sensor Not Support!\n");
            break;
    }
    return rc;
}

int32_t dx_sum     = 0;
int16_t dy_sum     = 0;

void clear_dial_rawdata(void)
{
	 dx_sum =0;
}

short get_motion_angle(void)
{
  uint8_t mot_status = 0x00;
  mixo_ots_get_mot_status(&mot_status);
  if ((mot_status & 0x80) == 0x80)
  {
		if((dx_sum>=DIAL_STEP)||(dx_sum<=-DIAL_STEP))
		{
			dx_sum=dx_sum%DIAL_STEP;
		}
		
    raw_data_t raw_data;
    mixo_ots_get_raw_data(&raw_data);
//		mixo_ots_reg_read(0x17, &mot_status);
//		dbg_printf("=%d,%d\n", raw_data.dx,mot_status);
//		mixo_ots_reg_read(0x22, &mot_status);
//		dbg_printf("=%d\n",mot_status);
//		mixo_ots_reg_read(0x23, &mot_status);
//		dbg_printf("=%d\n",mot_status);
//		mixo_ots_reg_read(0x02, &mot_status);
//		dbg_printf("=%d\n",mot_status);


    dx_sum += raw_data.dx;
    dy_sum += raw_data.dy;
		if((dx_sum<DIAL_STEP)&&(dx_sum>-DIAL_STEP))
		{
			return 0xFFF;
		}
		return -dx_sum/DIAL_STEP;
  }
	else                                    // 中断+轮询模式下，mot_status的最高位为0时，累加no_motion_count计数
	{
			{

					mixo_ots_forced_clear_int();    // 拉高MOT脚电平
			}
	}
	return 0xFFF;
}