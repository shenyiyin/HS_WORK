#ifndef STORAGE_H
#define STORAGE_H

#include "types.h"

void fs_Init( uint8 task_id );
void write_file(void);
void read_data(uint8_t* buf,uint16_t len,uint16_t *get_len);
void write_data1(uint8_t* buf,uint16_t len);
#endif

