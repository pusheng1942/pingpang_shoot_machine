#ifndef __MOTOR_DRV_H
#define __MOTOR_DRV_H	
#include "sys.h"

#define MOTOR_EVENT_UP_BALL			1<<0	//????
#define MOTOR_EVENT_RUN_FINISH		1<<1	//??????
#define MOTOR_EVENT_UP_FINISH		1<<2	//??????

#define MOTOR_STATUS_STOP		0
#define MOTOR_STATUS_BASE		1
#define AUTO_RUN_STATUS_RUN		2
#define AUTO_RUN_STATUS_PAUSE	3
#define AUTO_RUN_STATUS_FINISH	4

typedef struct
{
	uint8_t	status;		//???? 1 ?? 0 ?? 2-255??
	int8_t	speed;		//???? ±100
	int8_t	new_speed;	//????? ±100
}STRUCT_DC_MOTOR;


#define MOTOR_RUNNING  1
#define MOTOR_STOPPED  0

typedef struct
{
	uint8_t	status;		//???? 1 ?? 0 ?? 2-255?? 
	int8_t	speed;		//???? ±100
	int8_t	new_speed;	//????? ±100
	int16_t location;	//????  0-180 
	int16_t new_location;//????	0-180
}STRUCT_STEPPER_MOTOR;


typedef struct
{
	uint8_t run_status; //0-?? 1-???? 2-?????? 3-?????? 4-??????
	STRUCT_DC_MOTOR dc1;
	STRUCT_DC_MOTOR dc2;
	STRUCT_DC_MOTOR dc_up;
	STRUCT_STEPPER_MOTOR step1;
	STRUCT_STEPPER_MOTOR step2;
	STRUCT_STEPPER_MOTOR step3;
	uint8_t target_ball_num;    //???????????,GP add
	uint8_t up_status;
	uint8_t up_total_number;
	uint8_t up_auto_number;
}STRUCT_MOTOR_STATUS;

#define PITCH_START_LOCATION    100
#define YAW_START_LOCATION      100
#define ROLL_START_LOCATION     100
#define DEGUG_ON

extern STRUCT_MOTOR_STATUS motor;
void motor_run(STRUCT_MOTOR_STATUS motor);
void motor_reset(void);
void up_ball_motor_ctrl(int target_ball_num, STRUCT_DC_MOTOR* dc_up, int delay_time);
void up_ball_motor_speed_ctrl(u16 speed);
void dc_shoot_motor_ctrl(int motor1_speed, int motor2_speed);
void pitch_motor_angle_ctrl(int angle,uint32_t speed);
void yaw_motor_angle_ctrl(int angle,uint32_t speed);
void roll_motor_angle_ctrl(int angle,uint32_t speed);
void pitch_motor_reset(void);
void yaw_motor_reset(void);
void roll_motor_reset(void);
void roll_motor_location_set(int new_location, uint32_t speed);
void yaw_motor_location_set(int new_location, uint32_t speed);
#endif

