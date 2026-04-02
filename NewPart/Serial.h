#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"
#include <stdio.h>

/* 保持不变：电脑串口 (USART1) */
void Serial_Init(uint32_t baudrate);
void Serial_SendByte(uint8_t Byte);
void Serial_SendString(char *String);

/* 重新命名：WiFi 模块串口 (USART3) */
void WIFI_UART_Init(uint32_t baudrate);
void ToWIFI_SendByte(uint8_t Byte);
void ToWIFI_SendString(char *String);
uint8_t WiFi_Check_Response(void);


#endif
