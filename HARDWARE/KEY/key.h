#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define BALL_TRIGGER_STATE  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)

void ball_check_config(void); //IO初始化
				    
#endif
