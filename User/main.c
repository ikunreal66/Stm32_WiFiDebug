#include "stm32f10x.h"
#include "serial.h"
#include "stdio.h"
#include "delay.h"
#include "string.h"
#include "wifi.h"
uint8_t Send_wifidata[102];

int main()
{
	NVIC_SetPriorityGrouping(5);//两位抢占两位次级
    Serial_Init(115200); 
	Esp8266_Config();
    strcpy((char*)Send_wifidata, "hello world");
	Wifi_ConnectIP();
    U3_SendStr(Send_wifidata);
    while(1)
    {	
    }
		return 0;
}

