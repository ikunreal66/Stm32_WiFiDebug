#ifndef __WiFi_H__
#define __WiFi_H__  

#include "stm32f10x.h"
#include "Delay.h"
#include "Serial.h"
#include <string.h>



void WIFI_UART_Init(uint32_t baudrate);
void USART3_IRQHandler(void);
uint8_t WiFi_Check_Response(void);
void ToWIFI_SendByte(uint8_t Byte);
void ToWIFI_SendString(char *String);
uint8_t WiFi_Wait_Response(char *Target, uint16_t Timeout_ms);
uint8_t WiFi_AutoConnect(char *SSID, char *PWD, char *IP, char *Port);

#endif
