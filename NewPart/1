#ifndef _WIFI_H_
#define _WIFI_H_
#include "stm32f10x.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"
#include "Serial.h"
typedef struct{
	uint8_t recvbuf[1024];
	uint16_t recvcnt;
}WIFIDATA;

void Esp8266_Config(void);
void U3_SendStr(uint8_t * data);
uint8_t Wifi_Send_Cmd(char * cmd,char * recv,uint32_t timeout);
uint8_t Wifi_ConnectIP(void);
void U3_SendStr(uint8_t * data);
#endif
		
