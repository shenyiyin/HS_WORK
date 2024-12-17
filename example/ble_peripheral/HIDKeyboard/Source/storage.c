#include "storage.h"


/**************************************************************************************************

    Phyplus Microelectronics Limited confidential and proprietary.
    All rights reserved.

    IMPORTANT: All rights of this software belong to Phyplus Microelectronics
    Limited ("Phyplus"). Your use of this Software is limited to those
    specific rights granted under  the terms of the business contract, the
    confidential agreement, the non-disclosure agreement and any other forms
    of agreements as a customer or a partner of Phyplus. You may not use this
    Software unless you agree to abide by the terms of these agreements.
    You acknowledge that the Software may not be modified, copied,
    distributed or disclosed unless embedded on a Phyplus Bluetooth Low Energy
    (BLE) integrated circuit, either as a product or is integrated into your
    products.  Other than for the aforementioned purposes, you may not use,
    reproduce, copy, prepare derivative works of, modify, distribute, perform,
    display or sell this Software and/or its documentation for any purposes.

    YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
    PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
    INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
    NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
    PHYPLUS OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
    NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
    LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
    OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
    OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

**************************************************************************************************/

/**************************************************************************************************
    Filename:       fs_demo.c
    Revised:        $Date $
    Revision:       $Revision $


**************************************************************************************************/

/*********************************************************************
    INCLUDES
*/

#include "storage.h"
#include "OSAL.h"
#include "gpio.h"
#include "clock.h"
#include "timer.h"
//#include "fs_test.h"

#include "fs.h"
#include "log.h"

static uint8 fs_TaskID;


Store_Setting store_setting;

/*********************************************************************
    @fn      AP_TIMER_Demo_Init

    @brief

    @param

    @return
*/


void app_fst_init(void)
{
    if(hal_fs_initialized() == FALSE)
    {
        uint8_t err=hal_fs_init(0x1103C000,4);
				dbg_printf("test1=%d\n",err);
    }
		dbg_printf("test2\n");
}

void write_data1(uint8_t* buf,uint16_t len)
{
	hal_fs_item_write(1, buf, len);
}

void read_data(uint8_t* buf,uint16_t len,uint16_t *get_len)
{
	hal_fs_item_read(1, buf, len,get_len);
}


void save_setting(Store_Setting *ss)
{
	hal_fs_item_write(1, (uint8_t*)ss, sizeof(Store_Setting));
}


void load_setting(Store_Setting *ss)
{
	uint16_t len;
	hal_fs_item_read(1, (uint8_t*)ss, sizeof(Store_Setting),&len);
}


void fs_Init( uint8 task_id )
{
    fs_TaskID = task_id;
    app_fst_init();
}
uint16 fs_ProcessEvent( uint8 task_id, uint16 events )
{
    if ( events & SYS_EVENT_MSG )
    {
       
    }

    return 0;
}
