#ifndef STORAGE_H
#define STORAGE_H

#include "types.h"


typedef struct STORE_Setting
{
	  uint8_t pair_addr[6];
    uint8_t water_switch;
	  uint8_t defaultnormal_tmp;
		uint8_t defaultwarm_tmp;
	  uint16_t deault_mount;
		uint16_t warm_vol;
		uint16_t normal_vol;
	  uint8_t  NC1;
} Store_Setting;

extern Store_Setting store_setting;

void fs_Init( uint8 task_id );
void write_file(void);
void read_data(uint8_t* buf,uint16_t len,uint16_t *get_len);
void write_data1(uint8_t* buf,uint16_t len);
void load_setting(Store_Setting *ss);
void save_setting(Store_Setting *ss);
	


#endif

