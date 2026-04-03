#include "WiFi.h"

uint8_t WiFi_RxBuffer[64]; 
uint8_t WiFi_RxFlag;   
// ==================== WiFi 模块串口 (USART3) ====================

void WIFI_UART_Init(uint32_t baudrate) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; // RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); 

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 模块优先级最高
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART3, ENABLE);
}


void ToWIFI_SendByte(uint8_t Byte) {
    USART_SendData(USART3, Byte);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void ToWIFI_SendString(char *String) {
    for (uint16_t i = 0; String[i] != '\0'; i++) ToWIFI_SendByte(String[i]);
}


/**
 * @brief 借鉴 DMA 方案的强化版 USART3 中断
 */
void USART3_IRQHandler(void)
{
    // 获取当前状态寄存器和数据寄存器（读操作本身就能触发某些标志位的清除）
    uint16_t stat = USART3->SR;
    uint16_t data = USART3->DR; 

    /* 1. 异常处理：照搬 DMA 方案的强力清除逻辑 */
    // 检查是否有 溢出(ORE)、噪声(NE)、帧错误(FE) 或 校验错误(PE)
    if (stat & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE))
    {
        // 这里的读操作（SR 然后 DR）是芯片手册规定的清除 ORE 的标准序列
        (void)USART3->SR;
        (void)USART3->DR;
        
        // 如果你之前用了 DMA，这里要重启 DMA。
        // 但既然现在是纯串口转发，我们只需确保标志位清空，直接退出防止跑飞
        return; 
    }

    /* 2. 正常接收处理 (RXNE) */
    if (stat & USART_SR_RXNE) 
    {
        // 将 WiFi 的数据转发给电脑
        Serial_SendByte((uint8_t)data);

        // 存入缓存做逻辑判断（用于 OLED 显示 OK）
        static uint8_t Index = 0;
        if (Index < 63) {
            WiFi_RxBuffer[Index++] = (uint8_t)data;
            if (data == '\n') {
                WiFi_RxFlag = 1;
                WiFi_RxBuffer[Index] = '\0';
                Index = 0;
            }
        } else {
            Index = 0; // 缓冲区保护
        }
    }
}


uint8_t WiFi_Check_Response(void) {
    if (WiFi_RxFlag == 1) {
        WiFi_RxFlag = 0;
        if (strstr((char *)WiFi_RxBuffer, "OK") != NULL) return 1;
    }
    return 0;
}

//=================Auto connect WiFi================================


/**
 * @brief 等待特定回复的工具函数
 * @param Target 期待的字符串 (如 "OK" 或 "WIFI GOT IP")
 * @param Timeout_ms 最大等待时间 (ms)
 * @retval 1: 成功匹配, 0: 超时失败
 */
uint8_t WiFi_Wait_Response(char *Target, uint16_t Timeout_ms) {
    uint16_t time = 0;
    while (time < Timeout_ms) {
        if (WiFi_RxFlag == 1) { // 中断里收到了一行
            WiFi_RxFlag = 0;
            if (strstr((char *)WiFi_RxBuffer, Target) != NULL) {
                memset(WiFi_RxBuffer, 0, sizeof(WiFi_RxBuffer)); // 清除缓存供下次使用
                return 1; // 成功！
            }
            memset(WiFi_RxBuffer, 0, sizeof(WiFi_RxBuffer)); // 不是想要的，也清了
        }
        Delay_ms(1);
        time++;
    }
    return 0; // 闹钟响了，还没等到，返回失败
}

/**
 * @brief 自动化连接流程
 * @retval 1: 全链路通畅(进入透传), 0: 某一步失败(已跳过)
 */
uint8_t WiFi_AutoConnect(char *SSID, char *PWD, char *IP, char *Port) {
    char cmd[128];

    // 1. 测试通信
    ToWIFI_SendString("AT\r\n");
    if (!WiFi_Wait_Response("OK", 500)) return 0;

    // 2. 模式设置 (Station)
    ToWIFI_SendString("AT+CWMODE=1\r\n");
    if (!WiFi_Wait_Response("OK", 500)) return 0;

    // 3. 连接热点 (这是最容易失败的一步，给 8 秒宽限)
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PWD);
    ToWIFI_SendString(cmd);
    if (!WiFi_Wait_Response("WIFI GOT IP", 8000)) {
        // 如果热点没开，代码运行到这里就结束了，返回 0
        return 0; 
    }

    // 4. 连接 TCP 服务器 (手机 IP 和 端口)
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", IP, Port);
    ToWIFI_SendString(cmd);
    if (!WiFi_Wait_Response("CONNECT", 2000)) return 0;

    // 5. 开启透传模式
    ToWIFI_SendString("AT+CIPMODE=1\r\n");
    if (!WiFi_Wait_Response("OK", 500)) return 0;

    // 6. 进入透传发送状态
    ToWIFI_SendString("AT+CIPSEND\r\n");
    if (!WiFi_Wait_Response(">", 500)) return 0;

    return 1; // 走到这一步，说明已经可以像发串口一样直接发数据到手机了
}


