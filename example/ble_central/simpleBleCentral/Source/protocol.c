#include "protocol.h"
#include "log.h"
#include <stdio.h>
#include <stdint.h>
#include "OSAL.h"
#include "user_app.h"
#include "lcd.h"
#include "storage.h"


#define FRAME_HEADER 0x55aa
#define MAX_FRAME_SIZE 32




#define MAX_DATA_LENGTH 256

// 协议解析状态
typedef enum {
    PARSE_IDLE,
    PARSE_FRAME_HEADER,
    PARSE_VERSION,
    PARSE_COMMAND,
    PARSE_DATA_LENGTH0,
		PARSE_DATA_LENGTH1,
    PARSE_DATA,
    PARSE_CHECKSUM
} ParseState;

// 协议解析结果
typedef struct {
    uint8_t version;
    uint8_t command;
    uint16_t dataLength;
    uint8_t *data;
    uint8_t checksum;
    bool isValid;
} ParseResult;

// 全局变量，用于存储解析结果
ParseResult parseResult;
uint8_t receivedData[MAX_DATA_LENGTH + 5]; // 存储接收到的数据，包括帧头、版本、命令字、数据长度、数据和校验和
uint8_t receivedIndex = 0; // 已接收数据的索引

Timestamp timestamp;
Timestamp temp_timestamp;

// 解析时间戳数据
bool parseTimestamp(uint8_t *data, uint16_t dataLength, Timestamp *ts) {
    if (dataLength < 7) { // 至少需要7字节的数据
        return false;
    }


    ts->hour = data[5];
    ts->minute = data[6];
		ts->second = data[7];
		osal_stop_timerEx(User_App_TaskID,user_notifyenable_event_id);
		osal_start_timerEx(User_App_TaskID, count1s_event_id, 900);
		osal_start_timerEx(User_App_TaskID, user_req_defaultwarmwater_event_id, 200);
    return true;
}


// 校验和计算函数
uint8_t calculateChecksum(uint8_t *data, uint16_t length) {
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < length; ++i) {
        checksum += data[i];
    }
    return checksum % 256;
}

//0x55,0xAA,0x03,0x07,0x00,0x08,0x08,0x02,0x00,0x04,0x00,0x00,0x00,0x35,0x54,
//0x55,0xAA,0x03,0x07,0x00,0x08,0x73,0x02,0x00,0x04,0x00,0x00,0x01,0x7C,0x07
//0x55,0xAA,0x03,0x07,0x00,0x08,0x65,0x02,0x00,0x04,0x00,0x00,0x00,0x35,0xB1,

int extractValue(unsigned char *data, uint16_t dataLength) {
    int len = (data[3] << 8) | data[4];
		if (dataLength !=(len+4)) { // 至少需要7字节的数据
        return false;
    }
    int value = 0;
	
    if (len == 1) {
        value = data[5];
    } else if (len == 2) {
        value = (data[5] << 8) | data[6];
    } else if (len == 4) {
        value = (data[5] << 24) | (data[6] << 16) | (data[7] << 8) | data[8];
    } else {
    }

    return value;
}


// 协议解析函数
void parseProtocol(uint8_t byte) {
    static ParseState state = PARSE_IDLE;
    static uint16_t dataIndex = 0;
    switch (state) {
        case PARSE_IDLE:
            if (byte == 0x55) { // 帧头高字节
                state = PARSE_FRAME_HEADER;
            }
            break;

        case PARSE_FRAME_HEADER:
            if (byte == 0xaa) { // 帧头低字节
                receivedData[0] = 0x55;
                receivedData[1] =0xaa;
                state = PARSE_VERSION;
            } else {
                state = PARSE_IDLE;
            }
            break;

        case PARSE_VERSION:
            receivedData[2] = byte;
            state = PARSE_COMMAND;

            break;

        case PARSE_COMMAND:
            receivedData[3] = byte;
				    parseResult.command=byte;
            state = PARSE_DATA_LENGTH0;
							
            break;

        case PARSE_DATA_LENGTH0:
            receivedData[4] = byte;
            parseResult.dataLength = (uint16_t)byte << 8; // 高字节
            state = PARSE_DATA_LENGTH1;
            break;
				
        case PARSE_DATA_LENGTH1:
            receivedData[5] = byte;
            parseResult.dataLength += (uint16_t)byte; // 高字节
            if(parseResult.dataLength)
						{
							state = PARSE_DATA;
						}
				    else
						{
							state=PARSE_CHECKSUM;
						}
            dataIndex = 0;
            break;
        case PARSE_DATA:
            if (dataIndex < MAX_DATA_LENGTH) {
                receivedData[6 + dataIndex] = byte;
                parseResult.data = &receivedData[5];
                if (++dataIndex == parseResult.dataLength) {
                    state = PARSE_CHECKSUM;
                }
            }
						else
						{
							state = PARSE_IDLE;
						}
            break;

        case PARSE_CHECKSUM:
            parseResult.checksum = byte;
            uint8_t calculatedChecksum = calculateChecksum(receivedData, 6 + parseResult.dataLength);
            if (parseResult.checksum == calculatedChecksum) {
                parseResult.isValid = true;
							   // 根据命令字解析时间戳
								if (parseResult.command == 0x0C || parseResult.command == 0x1C) {
										if (parseTimestamp(parseResult.data, parseResult.dataLength, &timestamp)) {
												LOG("Timestamp: %02d:%02d\n",
															 timestamp.hour, timestamp.minute);
										} else {
												LOG("Invalid timestamp data\n");
										}
								}
								else if(parseResult.command==0x07)
								{
									uint8_t dp_id=parseResult.data[1];
									if(dp_id==0x08)
									{
										if(Dis_DATA.setTemperature==0)
										{
											Dis_DATA.currentTemperature=extractValue(parseResult.data, parseResult.dataLength);
										}
										LOG("currentTemperature=%d C",	Dis_DATA.currentTemperature);
									}
									else if(dp_id==0x73)
									{

										Dis_DATA.currentwaterVolume=Dis_DATA.setwaterVolume-extractValue(parseResult.data, parseResult.dataLength);//剩余出水量
										LOG("currentwaterVolume=%d ml",	Dis_DATA.currentwaterVolume);
									}
									else if(dp_id==0x0c)
									{
										Dis_DATA.isDispensing=parseResult.data[5];
										if(Dis_DATA.isDispensing==0)
										{
											turn_to_standby();
										}
										LOG("isDispensing=%d",	Dis_DATA.isDispensing);
									}
									else if(dp_id==0x65)
									{
										store_setting.defaultwarm_tmp=extractValue(parseResult.data, parseResult.dataLength);
										LOG("defaultwarm_tmp=%d C",	store_setting.defaultwarm_tmp);
									}
								
								}
            } 
						else {
                parseResult.isValid = false;
            }
            state = PARSE_IDLE; // 重置状态，准备解析下一帧
            break;

        default:
            state = PARSE_IDLE;
            break;
    }
    if (state == PARSE_IDLE) {
        receivedIndex = 0; // 重置接收索引
    }
}




uint8_t pack_frame(uint8_t *frame, uint8_t command, uint16_t data_length, uint8_t *data) {
    frame[1] = FRAME_HEADER & 0xFF;
    frame[0] = (FRAME_HEADER >> 8) & 0xFF;
    frame[2] = 0; // Version
    frame[3] = command;
    frame[5] = data_length & 0xFF;
    frame[4] = (data_length >> 8) & 0xFF;
    
    for (int i = 0; i < data_length; i++) {
        frame[6 + i] = data[i];
    }

    // 计算校验和
    uint16_t sum = frame[1]+frame[0] + 0 + command + data_length;
    for (int i = 0; i < data_length; i++) {
        sum += data[i];
    }
    frame[6 + data_length] = sum % 256;

    return data_length + 7;
}


//一键排空
uint8_t send_empty_water(uint8_t *frame, uint8_t status) {
    uint8_t data[5] = {106, 0x01,00,01,status }; // Type: Bool, Value: 0x00 or 0x01
    return pack_frame(frame, 0x06, 5, data); // Command: 0x04, Data length: 4
}



// 无极调温下发函数
uint8_t send_temperature_setting(uint8_t *frame, uint8_t temperature) {
    uint8_t data[8] = {0x07, 0x02,0x00,0x04, 0x00,0x00,0x00,temperature}; // Type: Value, Value: temperature
    return pack_frame(frame, 0x06, 8, data); // Command: 0x03, Data length: 3
}

// 取水开关下发函数
uint8_t send_water_switch(uint8_t *frame, uint8_t status) {
    uint8_t data[5] = {0x0c, 0x01,00,01,status }; // Type: Bool, Value: 0x00 or 0x01
    return pack_frame(frame, 0x06, 5, data); // Command: 0x04, Data length: 4
}

// 取水水量下发函数
uint8_t send_water_amount(uint8_t *frame, uint16_t amount) {
    uint8_t data[8] = {13, 0x02,0x00,0x04, 0x00,0x00,amount>>8,amount&0x00ff}; // Type: Value, Value: temperature
    return pack_frame(frame, 0x06, 8, data); // Command: 0x05, Data length: 3
}



// 故障告警下发函数
uint8_t send_fault_alarm(uint8_t *frame, uint8_t alarm) {
    uint8_t data[2] = {0x04, alarm}; // Type: Enum, Value: alarm
    return pack_frame(frame, 0x06, 2, data); // Command: 0x07, Data length: 2
}



// 温水默认水温下发函数
uint8_t send_warm_water_temperature(uint8_t *frame, uint8_t temperature) {
    uint8_t data[8] = {101, 0x02,0x00,0x04, 0x00,0x00,0x00,temperature}; // Type: Value, Value: temperature
    return pack_frame(frame, 0x06, 8, data); // Command: 0x0A, Data length: 3
}


void SEND_TEST(void)
{
	//uint8_t data[12]={0x55,0xaa,0,6,00,05,0x0c,01,00,01,00,18};
	
}