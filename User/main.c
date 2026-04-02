#include "stm32f10x.h"
#include "Serial.h"
#include "OLED.h"
#include "Delay.h"

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(1, 1, "Full-Duplex Brdg");

    Serial_Init(115200);   
    WIFI_UART_Init(115200); 
    
    Delay_ms(500);
    Serial_SendString("\r\nSystem Interrupt Bridge Ready.\r\n");

    while (1)
    {
        // 这里的 while(1) 现在是空的！
        // 或者是放一些不影响通信的 UI 代码
        if (WiFi_Check_Response()) {
            OLED_ShowString(2, 1, "Last Msg: OK   ");
        }
    }
}

