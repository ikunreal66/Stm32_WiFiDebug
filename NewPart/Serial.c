#include "Serial.h"



// ==================== 电脑串口 (USART1) ====================

void Serial_Init(uint32_t baudrate) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  // TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_InitStructure);

    // 【新增】开启 USART1 接收中断，处理电脑下发的指令
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 优先级略低
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
}


// --- 发送函数 ---
void Serial_SendByte(uint8_t Byte) {
    USART_SendData(USART1, Byte);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void Serial_SendString(char *String) {
    for (uint16_t i = 0; String[i] != '\0'; i++) Serial_SendByte(String[i]);
}


// ==================== 中断处理 (核心) ====================

/**
 * @brief USART1 中断：处理 电脑 -> WiFi
 */
void USART1_IRQHandler(void) {
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

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t Data = USART_ReceiveData(USART1);
        USART3->DR = Data; // 电脑发来什么，立刻塞给WiFi
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

int fputc(int ch, FILE *f) {
    Serial_SendByte(ch);
    return ch;
}
