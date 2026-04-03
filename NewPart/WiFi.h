#ifndef __WiFi_H__
#define __WiFi_H__  

#include "stm32f10x.h"

uint8_t WiFi_RxBuffer[64]; 
uint8_t WiFi_RxFlag = 0;   



void WIFI_UART_Init(uint32_t baudrate);
void USART3_IRQHandler(void);
uint8_t WiFi_Check_Response(void);
void ToWIFI_SendByte(uint8_t Byte);
void ToWIFI_SendString(char *String);

#endif
