#include "wifi.h"

WIFIDATA wifidata={0};

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
		USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);//USART1->CR1 |= 0x1<<5;//使能串口1的接收非空中断
		NVIC_SetPriority(USART3_IRQn,7);//设置优先级0~15
		NVIC_EnableIRQ(USART3_IRQn);//使能中断通道
		GPIO_SetBits(GPIOE,GPIO_Pin_6);
	    Delay_ms(500);
}

void USART3_IRQHandler(void)
{
	
	uint8_t data=0;
	if((USART3->SR&0x1<<5)!=0)
	{//执行该中断函数的原因有很多，所以判断一下是不是接收导致的
		data = USART_ReceiveData(USART3);//读操作，同时也是清空中断标志位
		wifidata.recvbuf[wifidata.recvcnt] = data;
		wifidata.recvcnt++;
		wifidata.recvcnt%=1024;
		USART_SendData(USART1, data); 
	}
}

//串口5发送单字节函数
void Usart3Senddata(uint8_t data)
{
	//等待发送完成
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==0);
	//如果上次发送完成，就发送
	USART_SendData(USART3,data);
}

//串口5发送数组函数
void U3_Sendarr(uint8_t * data,uint32_t len)
{
	uint32_t i=0;
	for(i=0;i<len;i++){
		Usart3Senddata(*data);
		data++;
	}
}

void U3_SendStr(uint8_t * data)
{	
	while(*data!='\0')
	{
		Usart3Senddata(*data);
		data++;
	}
}

uint8_t Wifi_Send_Cmd(char * cmd,char * recv,uint32_t timeout)
{
	uint32_t timecnt=0;
	memset(&wifidata,0,sizeof(wifidata));
	U3_SendStr((uint8_t *)cmd);
	while(strstr((char *)wifidata.recvbuf,recv)==NULL){
	timecnt++;
	Delay_ms(1);
		if(timecnt>=timeout){
		printf("发送超时失败%s",cmd);
		return 1;
	 }
	}
	printf(" 发送成功 ");
	return 0;
}

uint8_t Wifi_ConnectIP(void)
{
	if(Wifi_Send_Cmd("AT\r\n","OK",1000) != 0){//测试
		return 1;
	}
	if(Wifi_Send_Cmd("AT+CWMODE=1\r\n","OK",2000) != 0){//设置为STA
		return 1;
	}
	if(Wifi_Send_Cmd("AT+CWJAP=\"ikun\",\"12345678\"\r\n","OK",10000)!= 0){//连接热点
		return 1;
	}
	if(Wifi_Send_Cmd("AT+CIPSTART=\"TCP\",\"114.213.213.14\",8080\r\n","OK",10000)!= 0){//连接服务器
		return 1;
	}
	if(Wifi_Send_Cmd("AT+CIPMODE=1\r\n","OK",1000)!= 0){//开启透传
		return 1;
	}
	if(Wifi_Send_Cmd("AT+CIPSEND\r\n","OK",1000)!= 0){//启动发送功能
		return 1;
	}
	return 0;	
}

