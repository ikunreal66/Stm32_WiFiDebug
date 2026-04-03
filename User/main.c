#include "stm32f10x.h"
#include "Serial.h"
#include "OLED.h"
#include "Delay.h"
#include "WiFi.h"

int main(void) {
    // 1. 硬件初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    OLED_Init();
    Serial_Init(115200);   // 电脑调试口
    WIFI_UART_Init(115200); // WiFi 串口
    
    OLED_ShowString(1, 1, "System Boot...");
    Delay_ms(1000);

    // 2. 尝试联网 (根据你之前的截图填入参数)
    // 如果热点没开，这个函数跑完 8 秒就会返回 0，程序继续往下走
    uint8_t wifi_ready = WiFi_AutoConnect("ikunreal", "12345678", "10.37.80.177", "49381");

    if (wifi_ready) {
        OLED_Clear();
        OLED_ShowString(1, 1, "WiFi: Online");
    } else {
        OLED_Clear();
        OLED_ShowString(1, 1, "WiFi: Skip/Fail");
    }

    while (1) {
        // 3. 业务代码
        if (wifi_ready) {
            // 如果联网成功，每秒给手机发个“56”
            ToWIFI_SendString("56\r\n");
            OLED_ShowString(2, 1, "Sending: 56");
        } else {
            // 如果联网失败，就在本地 OLED 显示数据，不发 WiFi
            OLED_ShowString(2, 1, "Local Mode ");
        }
        Delay_ms(1000);
    }
}


