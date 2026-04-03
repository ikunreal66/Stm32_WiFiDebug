
#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "serial.h"
#include "WiFi.h"

int main(void)
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  OLED_Init();
  OLED_ShowString(1,1,"Hello World!");
  Serial_Init(115200);
  Esp8266_Config();
    
//  uint16_t ret = WIFI_Connect();

  
   uint16_t ret = WIFI_Connect( (uint8_t*) "ikun", 
                                (uint8_t*) "12345678", 
                                (uint8_t*) "192.168.137.1", 
                                (uint8_t*) "8080"
   );

//   uint16_t ret = WIFI_ConnectPC(  "ikunreal", 
//                                 "12345678", 
//                                 "10.37.80.166", 
//                                 "50247"
//   );


  OLED_ShowNum(2,1,ret,1);
  while(1)
  {
    
  }
}

