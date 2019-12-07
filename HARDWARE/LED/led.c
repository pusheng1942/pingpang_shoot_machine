#include "led.h"

//LED IO初始化
void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PC端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;				 //LED0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC.14
	GPIO_SetBits(GPIOC,GPIO_Pin_14);						 //PC.14 输出高
	LED0 = 1;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //LED
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC.14
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //发射电机速度使能引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC.14
	GPIO_ResetBits(GPIOC,GPIO_Pin_4);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //发射电机速度使能引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC.14
	GPIO_ResetBits(GPIOC,GPIO_Pin_5);	
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
//	GPIO_Init(GPIOB, &GPIO_InitStructure);	
//	GPIO_SetBits(GPIOB,GPIO_Pin_0);						 //PC.14 输出高

	//根据设定参数初始化GPIOC.14
////	GPIO_ResetBits(GPIOB,GPIO_Pin_0);	
}
 
