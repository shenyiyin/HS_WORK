#ifndef __OTA_TASK_H__
#define __OTA_TASK_H__

#include "bcomdef.h"
#include "ppsp_serv.h"
#include "ppsp_impl.h"
#include "clock.h"
#include "OSAL_Timers.h"

#define OTA_DEBUG_LOG
#ifdef OTA_DEBUG_LOG
#define OTA_LOG(...)	LOG(__VA_ARGS__)
#else
#define OTA_LOG(...)
#endif

#define SLB_OTA_ENDED_EVT            0x0001

void OTA_Init( uint8 TaskId );
uint16 OTA_task_ProcessEvent( uint8 task_id, uint16 events );


#endif

