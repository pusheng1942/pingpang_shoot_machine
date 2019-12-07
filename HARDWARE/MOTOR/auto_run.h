#ifndef __AUTO_RUN_H
#define __AUTO_RUN_H	 
#include "sys.h"


//直流电机
typedef struct
{
	int16_t	pitch_motor;	//步进电机1位置
	int16_t	yaw_motor;		//步进电机2位置
	int16_t	roll_motor;		//步进电机3位置
	int8_t	speed1;			//直流电机1速度
	int8_t	speed2;			//直流电机2速度
	int8_t	delay_s;		//延迟时间（S）
}STRUCT_AUTO_ARRAY;

//步进电机
typedef struct
{
	STRUCT_AUTO_ARRAY array[50];
}STRUCT_AUTO_PROGRAMS;

extern STRUCT_AUTO_PROGRAMS auto_programs;
void auto_run_init(void);//初始化
void auto_run(void);
void auto_run_reset(void);
		 				    
#endif
