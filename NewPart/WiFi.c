#include "WiFi.h"

WIFI wifi = {0};

//配置串口3  8数据位，0校验位，1停止位，波特率115200
//PB10(TX) PB11(RX)
void Esp8266_Config()
{
	 //开时钟：GPIOB，USART3
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	
	  //配置对应的IO口 PB10（tx）:复用推挽 PB11（RX）：浮空输入
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStruct);
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
		GPIO_Init(GPIOB,&GPIO_InitStruct);
		//PE6
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE,&GPIO_InitStruct);
		
		
	  //配置串口3  8数据位，0校验位，1停止位，波特率115200
		USART_InitTypeDef USART_InitStruct = {0};//可以通过结构体类型跳转
		USART_InitStruct.USART_BaudRate = 115200;//波特率
		USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件控制流不开
		USART_InitStruct.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;//打开发送和接收
		USART_InitStruct.USART_Parity = USART_Parity_No;
		USART_InitStruct.USART_StopBits = USART_StopBits_1;
		USART_InitStruct.USART_WordLength = USART_WordLength_8b;
		USART_Init(USART3,&USART_InitStruct);
		USART_Cmd(USART3,ENABLE);
    //配置串口3的中断
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // 开启接收中断
		USART_ITConfig(USART3,USART_IT_IDLE,ENABLE);//USARTCR |=x1<<5;//使能串口1的非空中断
		NVIC_SetPriority(USART3_IRQn,7);//设置优先级0~15
		NVIC_EnableIRQ(USART3_IRQn);//使能中断通道
		GPIO_SetBits(GPIOE,GPIO_Pin_6);
	    Delay_ms(500);
}

/**
  * @brief  串口2发送字节 -- 发送的最基本的函数 -->其它发送函数都是基于它
  * @param  
  * @retval 
  */
void USART3_SendByte(uint8_t Byte)
{
  USART_SendData(USART3,Byte);
  while(USART_GetFlagStatus(USART3,USART_FLAG_TXE) == RESET );
}
/**
  * @brief  串口中断函数
            中断函数里面可以放你想要实现的功能函数
  * @param  
  * @retval 
  */
void USART3_IRQHandler(void)
{
  uint8_t data = 0;
  if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
  {
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    
    data = USART_ReceiveData(USART3);//ESP8266 发送给 STM32的数据
    
    wifi.rxbuff[wifi.rxcount++] = data;//将 ESP8266 发送给单片机的数据 转存到rxbuff里面
    
    USART_SendData(USART1,data); //可通过 串口助手在电脑屏幕上 显示
  }
  if(USART_GetITStatus(USART3, USART_IT_IDLE) == 1) //串口空闲中断
  {
      data = USART3->SR;
    
      data = USART3->DR;
    
      wifi.rxover = 1;
  }
}
/**
  * @brief  发送字符串
  * @param  string 字符串
  * @retval 
  */
void USART3_SendString(uint8_t *string)
{
  for(uint8_t i = 0;string[i] != '\0';i++)
  {
    USART3_SendByte(string[i]);
  }
}
/**
  * @brief  发送字符串 + 带长度
  * @param  
  * @retval 
  */
void Esp8266_SendString(uint8_t *string,uint16_t len)//串口3 发送字符串
{
  for(uint16_t i=0;i<len;i++)
  USART3_SendByte(string[i]);
}

/**
  * @brief  发送AT指令，检查AT指令的返回，判断AT指令是否发送成功
  * @param  cmd:发送的AT指令
  * @param  Rcmd:AT指令对应的返回值
  * @param  outtime:ESP8266超时回复判断
  * @retval 
  */
uint8_t EspSendCmdAndCheckRecvData(uint8_t *cmd,uint8_t *Rcmd,uint16_t outtime)
{
  uint8_t data = 0;
  
  //发送AT指令之前要清空
  
  memset(wifi.rxbuff,0,1024);
  
  wifi.rxover = 0;
  
  wifi.rxcount = 0;
  
  USART3_SendString(cmd);//发送AT指令 -- > 才会进入串口2中断服务
  
  while(outtime) 
  {
    if(wifi.rxover == 1)
    {
      if(strstr((char *)wifi.rxbuff,(char *)Rcmd) != NULL)
      {
        data = 1;
        break;
      }
    }
    outtime--;
    Delay_ms(1);
  }
  return data;
}


/**
  * @brief  连接电脑 TCP Server (调试版)
  * @retval 1: 成功
  * 10: AT握手失败
  * 20: 设置模式失败
  * 30: 连接热点失败 (SSID/密码错或信号差)
  * 40: 连接服务器失败 (IP/端口错或防火墙)
  * 50: 开启透传失败
  * 60: 进入透传发送失败
  */
uint8_t WIFI_ConnectPC(void)
{
	// 1. 基础握手
    if(EspSendCmdAndCheckRecvData("AT\r\n", "OK", 1000) != 1) return 0;

    // 2. 设置模式
    if(EspSendCmdAndCheckRecvData("AT+CWMODE=1\r\n", "OK", 1000) != 1) return 1;
    
    // 3. 连接热点
    if(EspSendCmdAndCheckRecvData("AT+CWJAP=\"ikunreal\",\"12345678\"\r\n", "OK", 15000) != 1) return 2;

    // 4. 连接服务器
    if(EspSendCmdAndCheckRecvData("AT+CIPSTART=\"TCP\",\"10.37.80.166\",8080\r\n", "OK", 5000) != 1) return 3;

    // 5. 开启透传
    if(EspSendCmdAndCheckRecvData("AT+CIPMODE=1\r\n", "OK", 1000) != 1) return 4;

    // 6. 进入透传状态
    if(EspSendCmdAndCheckRecvData("AT+CIPSEND\r\n", ">", 1000) != 1) return 5;

    // 成功
    USART3_SendString("STM32 Link Start!\r\n");
    return 6;
}

