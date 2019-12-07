#include "sys.h"
#include "usart.h"	  
#include "interface.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
UART_STRUCT uart_device;
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
    USART3->DR = (u8) ch;      
	return ch;
}
#endif 

 
#if EN_USART3_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能USART3，GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);
	//USART3_TX   GPIOA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.9

	//USART3_RX	  GPIOB.11初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.10  

	//USART3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口3
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
	USART_Cmd(USART3, ENABLE);                    //使能串口3 

}


// 返回值：1:成功，0:失败-buffer满
uint8_t Uart_Rx_Data_Process(uint8_t buf)
{	
	if (uart_device.addr_wr >= uart_device.addr_rd)  //wr 在后
	{
		if (uart_device.addr_wr == (USART_REC_LEN - 1))//wr在最后一个空位
		{
			if (uart_device.addr_rd == 0)//BUFFER is full!
			{	
				//USART3->CR1 &= ~USART_CR1_RXNEIE;    //关闭接收中断 ？？是否需要禁用接收？
				return ERROR;
			}
			else
			{
                USART_RX_BUF[uart_device.addr_wr] = buf;
                uart_device.addr_wr = 0;
			}
		}
		else
		{
			USART_RX_BUF[uart_device.addr_wr] = buf;
			uart_device.addr_wr++;			
		}
	}
	else   // wr跑了一圈到了rd前面
	{
		if (uart_device.addr_wr + 1 == uart_device.addr_rd)//BUFFER is full! wr在rd前一个空位
		{
			//USART3->CR1 &= ~USART_CR1_RXNEIE;    //关闭接收中断 ？？是否需要禁用接收？
			return ERROR;
		}
		else
		{
			USART_RX_BUF[uart_device.addr_wr] = buf;
			uart_device.addr_wr++;			
		}
	}
  return SUCCESS;
}

// 获取data接收缓冲区长度
uint16_t get_data_rx_rd_length()
{
	
    if(uart_device.addr_rd <= uart_device.addr_wr)
        return (uart_device.addr_wr - uart_device.addr_rd);
    else
        return (USART_REC_LEN -uart_device.addr_rd + uart_device.addr_wr);	//255-RD+1+WR
	
}
// 更新data接收缓冲区地址
void update_data_rx_addr(uint8_t length)
{
	uint16_t temp = 0;
	{	
		temp = USART_REC_LEN - 1 - uart_device.addr_rd;
		if (temp >= length)
			uart_device.addr_rd = uart_device.addr_rd + length;
		else
			uart_device.addr_rd = length - temp - 1;
	}
}

uint8_t get_uart_rx_rd_data(void)
{
    uint8_t temp = USART_RX_BUF[uart_device.addr_rd];
    update_data_rx_addr(1);
    return temp;
}

void uart_send_data(uint16_t length)
{
	uint16_t t;
	for( t = 0 ; t < length ; t++)
	{
		USART_SendData(USART3, SendBuff[t]);//向串口1发送数据
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);//等待发送结束
	}
}

void USART3_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res = USART_ReceiveData(USART3);	//读取接收到的数据
		Uart_Rx_Data_Process(Res);	
        g_flag.uart_rx_int = 1;	 
     } 
} 
#endif	

