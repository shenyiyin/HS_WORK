#include "ota_task.h"

uint8 ota_TaskID;

#define OTA_DELAY_TIME            3000

/*slb ota*/
static void ppsp_impl_reset_appCb(void);


/*********************************************************************
    @fn      ppsp_impl_reset_appCb

    @brief   slb ota over notify callback and to excute SLB_OTA_ENDED_EVT after OTA_DELAY_TIME

    @param   void

    @return  void
*/
static void ppsp_impl_reset_appCb( void )
{
	osal_start_timerEx(ota_TaskID, SLB_OTA_ENDED_EVT, OTA_DELAY_TIME);
}


/*********************************************************************
    @fn      OTA_Init

    @brief   Initialization function for the OTA App Task.
            This is called during initialization and should contain
            any application specific initialization (ie. hardware
            initialization/setup, table initialization, power up
            notificaiton ... ).

    @param   task_id - the ID assigned by OSAL.  This ID should be
                      used to send messages and set timers.

    @return  none
*/
void OTA_Init( uint8 TaskId )
{
    ota_TaskID = TaskId;
	
	// Set up services slb ota service
	ppsp_serv_add_serv( PPSP_SERV_CFGS_SERV_FEB3_MASK );
	
	/*slb ota*/
	static ppsp_impl_clit_hdlr_t impl_appl_hdlr = {
		.ppsp_impl_appl_rset_hdlr = ppsp_impl_reset_appCb,
	};
	
	// Register for slb service callback.
	ppsp_impl_reg_serv_appl( &impl_appl_hdlr );
	
}



/*********************************************************************
    @fn      OTA_task_ProcessEvent

    @brief   OTA_task Application Task event processor.  This function
            is called to process all events for the task.  Events
            include timers, messages and any other user defined events.

    @param   task_id  - The OSAL assigned task ID.
    @param   events - events to process.  This is a bit map and can
                     contain more than one event.

    @return  events not processed
*/
uint16 OTA_task_ProcessEvent( uint8 task_id, uint16 events )
{
	// ! slb ota over handle events
	if( events & SLB_OTA_ENDED_EVT )
	{
		OTA_LOG("slb ota over to reset\r\n");
		
		hal_system_soft_reset();
		
		return ( events ^ SLB_OTA_ENDED_EVT );
	}
	
	return 0;
}


