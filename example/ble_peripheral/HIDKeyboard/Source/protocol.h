#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


// ����ʱ������ݸ�ʽΪ����(2�ֽ�)����(1�ֽ�)����(1�ֽ�)��ʱ(1�ֽ�)����(1�ֽ�)����(1�ֽ�)
typedef struct {
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} Timestamp;

extern Timestamp temp_timestamp;
extern Timestamp timestamp;

void parseProtocol(uint8_t byte) ;
uint8_t send_water_switch(uint8_t *frame, uint8_t status);
uint8_t send_temperature_setting(uint8_t *frame, uint8_t temperature);
uint8_t send_water_amount(uint8_t *frame, uint16_t amount);
#endif