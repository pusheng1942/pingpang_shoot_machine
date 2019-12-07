#include "motor_drv.h"
#include <stdbool.h>
#include "interface.h"
#include "stm32f10x.h"
#include "key.h"
#include "includes.h"

//pitch
#define   PITCH_MOTOR_A_P PCout(0)  
#define   PITCH_MOTOR_A_N PCout(1)
#define   PITCH_MOTOR_B_P PCout(2)
#define   PITCH_MOTOR_B_N PCout(3) 

//yaw 
#define   YAW_MOTOR_A_P PCout(6)  
#define   YAW_MOTOR_A_N PCout(7)
#define   YAW_MOTOR_B_P PCout(8)
#define   YAW_MOTOR_B_N PCout(9) 

//roll 
#define   ROLL_MOTOR_A_P PBout(6)  
#define   ROLL_MOTOR_A_N PBout(7)
#define   ROLL_MOTOR_B_P PBout(8)
#define   ROLL_MOTOR_B_N PBout(9) 

#define   HIGH  1 
#define   LOW   0
//shoot motor direction
#define   SHOOT_MOTOR1_DIR	PCout(4)
#define   SHOOT_MOTOR2_DIR	PCout(5)

#define   POSITIVE    1 
#define   NEGATIVE    0


uint16_t motor_clockwise[4] ={0x0040,0x0100,0x0080,0x0200};          // D-C-B-A   
uint16_t motor_Counterclockwise[4]={0x0040,0x0080,0x0100,0x0200};    // A-B-C-D.

uint16_t phasecw8[8] ={0x0040,0x0140,0x0100,0x0180,0x0080,0x0280,0x0200,0x0240};// D-C-B-A   

STRUCT_MOTOR_STATUS motor;

#ifdef DEGUG_ON
STRUCT_DC_MOTOR dc_up=
{
	0,
	5,
	80
};
#endif

void motor_run(STRUCT_MOTOR_STATUS motor)
{
	if(g_flag.new_dc_motor_config == true)
	{
		g_flag.new_dc_motor_config = false;
	}
	

	if(g_flag.new_stepper_motor_config == true)
	{
		g_flag.new_stepper_motor_config = false;
	}
	
#ifdef DEGUG_ON	
	up_ball_motor_ctrl(5, &dc_up, 2000);
#endif
}

STRUCT_STEPPER_MOTOR pitch_motor;
STRUCT_STEPPER_MOTOR yaw_motor;
STRUCT_STEPPER_MOTOR roll_motor;

void motor_reset()
{

}

void pitch_motor_reset(void)
{
	pitch_motor_angle_ctrl(100, 20);
	delay_ms(1000);
	pitch_motor_angle_ctrl(-50, 20);
	pitch_motor.location = 0;
}

void yaw_motor_reset(void)
{
	yaw_motor_angle_ctrl(90, 20);
	delay_ms(3000);
	yaw_motor_angle_ctrl(-40, 20);
	yaw_motor.location = 0;
	delay_ms(3000);
}

void roll_motor_reset(void)
{
	roll_motor_angle_ctrl(200, 20);
	delay_ms(1000);
	roll_motor_angle_ctrl(-105, 20);
	roll_motor.location = 0;
	delay_ms(1000);
}

void motor_int_handle(void)
{
	set_motor_up_ball_event();
}

void dc_shoot_motor_ctrl(int motor1_speed, int motor2_speed)
{
	if(motor1_speed>=0)
	{
		SHOOT_MOTOR1_DIR = NEGATIVE;
	}
	else
	{
		SHOOT_MOTOR1_DIR = POSITIVE ;
	}
	
	if(motor2_speed>=0)
	{
		SHOOT_MOTOR2_DIR = NEGATIVE;
	}
	else
	{
		SHOOT_MOTOR2_DIR = POSITIVE;
	}
	
	TIM2->CCR3 = abs(motor1_speed)/100.0 * 5000;
	TIM2->CCR4 = abs(motor2_speed)/100.0 * 5000;
}


void step_motor_ctrl(STRUCT_STEPPER_MOTOR pitch_step, STRUCT_STEPPER_MOTOR yaw_step, STRUCT_STEPPER_MOTOR roll_step)
{

}

char state_record_flag = 1;
char delay_flag = 0;
static u16 total_shooted_ball_num = 0;
static u16 current_shooted_ball_num = 0;


void up_ball_motor_ctrl(int target_ball_num, STRUCT_DC_MOTOR* dc_up, int delay_time)
{
	u16 current_shooted_ball_num = 0;
	u32 time_error_count;
	for(current_shooted_ball_num = 0; current_shooted_ball_num < target_ball_num;)
	{
		if(BALL_TRIGGER_STATE == 0 && state_record_flag == 0)
		{   
//			delay_ms(50); 
			up_ball_motor_speed_ctrl(0);
			dc_up->status = MOTOR_STOPPED;
			current_shooted_ball_num++;
			state_record_flag = 1;
			
			motor.target_ball_num++;
			delay_ms(delay_time);
		}
		else if(BALL_TRIGGER_STATE == 1 && state_record_flag == 1)
		{
			state_record_flag = 0;
			dc_up->status = MOTOR_RUNNING;
			up_ball_motor_speed_ctrl(dc_up->speed);
		}
		
		if(BALL_TRIGGER_STATE == 0 && state_record_flag == 1)
		{
			dc_up->status = MOTOR_RUNNING;
			up_ball_motor_speed_ctrl(dc_up->speed);
		}
		delay_ms(20);
	}
	up_ball_motor_speed_ctrl(0);
}

void up_ball_motor_speed_ctrl(u16 speed)
{
	TIM3->CCR2 = speed/100.0 * 50000; 
}

extern STRUCT_STEPPER_MOTOR pitch_motor;
extern STRUCT_STEPPER_MOTOR yaw_motor;
extern STRUCT_STEPPER_MOTOR roll_motor;

void static pitch_motor_clockwise(int speed){
	uint8_t i;
	
    for(i=0;i<4;i++)  
    {  
		switch(i)
		{
			case 0:
			{
				PITCH_MOTOR_A_P = HIGH;
				PITCH_MOTOR_A_N = LOW;
				PITCH_MOTOR_B_P = LOW;
				PITCH_MOTOR_B_N = LOW;
				break;
			}
			
			case 1:
			{
				PITCH_MOTOR_A_P = LOW;
				PITCH_MOTOR_A_N = LOW;
				PITCH_MOTOR_B_P = HIGH;
				PITCH_MOTOR_B_N = LOW;
				break;
			}
			
			case 2:
			{
				PITCH_MOTOR_A_P = LOW;
				PITCH_MOTOR_A_N = HIGH;
				PITCH_MOTOR_B_P = LOW;
				PITCH_MOTOR_B_N = LOW;
				break;
			}
			
			case 3:
			{
				PITCH_MOTOR_A_P = LOW;
				PITCH_MOTOR_A_N = LOW;
				PITCH_MOTOR_B_P = LOW;
				PITCH_MOTOR_B_N = HIGH;
				break;
			}
		}
		delay_ms(speed);
	}		
}

void static pitch_motor_counterclockwise(int speed)
{
	uint8_t i;
	
    for(i=0;i<4;i++)  
    {  
		switch(i)
		{
			case 0:
			{
				PITCH_MOTOR_A_P = LOW;
				PITCH_MOTOR_A_N = LOW;
				PITCH_MOTOR_B_P = LOW;
				PITCH_MOTOR_B_N = HIGH;
				break;
			}
			
			case 1:
			{
				PITCH_MOTOR_A_P = LOW;
				PITCH_MOTOR_A_N = HIGH;
				PITCH_MOTOR_B_P = LOW;
				PITCH_MOTOR_B_N = LOW;
				break;
			}
			
			case 2:
			{
				PITCH_MOTOR_A_P = LOW;
				PITCH_MOTOR_A_N = LOW;
				PITCH_MOTOR_B_P = HIGH;
				PITCH_MOTOR_B_N = LOW;
				break;
			}
			
			case 3:
			{
				PITCH_MOTOR_A_P = HIGH;
				PITCH_MOTOR_A_N = LOW;
				PITCH_MOTOR_B_P = LOW;
				PITCH_MOTOR_B_N = LOW;
				break;
			}
		}
		delay_ms(speed);
	}		
}


void static pitch_motor_stop(void)
{ 
    PITCH_MOTOR_A_P = LOW;
	PITCH_MOTOR_A_N = LOW;
	PITCH_MOTOR_B_P = LOW;
	PITCH_MOTOR_B_N = HIGH;	
}

void pitch_motor_angle_ctrl(int angle,uint32_t speed)
{
	int i,j;
	char dir = 0;
	
//	TIM2->CCR3 = 1200/speed;
//	TIM2->CCR4 = 1200/speed;
	
	if(angle >= 0)
	{
		dir = 1;
	}
	else
	{
		dir =0;
	}
	
	j=(int)(abs(angle/7.2));
	for(i=0;i<j;i++)
	{
		if(dir)
		{
			pitch_motor_clockwise(speed);
		}
		else
		{
			pitch_motor_counterclockwise(speed);
		}
	}
	pitch_motor_stop();
}

//YAW
void static yaw_motor_clockwise(int speed){
	uint8_t i;
	
    for(i=0;i<4;i++)  
    {  
		switch(i)
		{
			case 0:
			{
				YAW_MOTOR_A_P = HIGH;
				YAW_MOTOR_A_N = LOW;
				YAW_MOTOR_B_P = LOW;
				YAW_MOTOR_B_N = LOW;
				break;
			}
			
			case 1:
			{
				YAW_MOTOR_A_P = LOW;
				YAW_MOTOR_A_N = LOW;
				YAW_MOTOR_B_P = HIGH;
				YAW_MOTOR_B_N = LOW;
				break;
			}
			
			case 2:
			{
				YAW_MOTOR_A_P = LOW;
				YAW_MOTOR_A_N = HIGH;
				YAW_MOTOR_B_P = LOW;
				YAW_MOTOR_B_N = LOW;
				break;
			}
			
			case 3:
			{
				YAW_MOTOR_A_P = LOW;
				YAW_MOTOR_A_N = LOW;
				YAW_MOTOR_B_P = LOW;
				YAW_MOTOR_B_N = HIGH;
				break;
			}
		}
		delay_ms(speed);
	}		
}

void static yaw_motor_counterclockwise(int speed)
{
	uint8_t i;
	
    for(i=0;i<4;i++)  
    {  
		switch(i)
		{
			case 0:
			{
				YAW_MOTOR_A_P = LOW;
				YAW_MOTOR_A_N = LOW;
				YAW_MOTOR_B_P = LOW;
				YAW_MOTOR_B_N = HIGH;
				break;
			}
			
			case 1:
			{
				YAW_MOTOR_A_P = LOW;
				YAW_MOTOR_A_N = HIGH;
				YAW_MOTOR_B_P = LOW;
				YAW_MOTOR_B_N = LOW;
				break;
			}
			
			case 2:
			{
				YAW_MOTOR_A_P = LOW;
				YAW_MOTOR_A_N = LOW;
				YAW_MOTOR_B_P = HIGH;
				YAW_MOTOR_B_N = LOW;
				break;
			}
			
			case 3:
			{
				YAW_MOTOR_A_P = HIGH;
				YAW_MOTOR_A_N = LOW;
				YAW_MOTOR_B_P = LOW;
				YAW_MOTOR_B_N = LOW;
				break;
			}
		}
		delay_ms(speed);
	}		
}


void static yaw_motor_stop(void)
{ 
	YAW_MOTOR_A_P = LOW;
	YAW_MOTOR_A_N = LOW;
	YAW_MOTOR_B_P = LOW;
	YAW_MOTOR_B_N = LOW;	
}


void yaw_motor_location_set(int new_location, uint32_t speed)
{
	int i, j, angle;
	char dir = 0;
	angle = new_location - yaw_motor.location;
	yaw_motor.location = new_location;
	yaw_motor_angle_ctrl(angle, speed);
}
void yaw_motor_angle_ctrl(int angle, uint32_t speed)
{
	int i, j;
	char dir = 0;
	if(angle >= 0)
	{
		dir = 1;
	}
	else
	{
		dir =0;
	}
	
	j=(int)(abs(angle/7.2));
	for(i=0;i<j;i++)
	{
		if(dir)
		{
			yaw_motor_clockwise(speed);
		}
		else
		{
			yaw_motor_counterclockwise(speed);
		}
	}
	yaw_motor_stop();
}


//YAW
void static roll_motor_clockwise(int speed){
	uint8_t i;
	
    for(i=0;i<4;i++)  
    {  
		switch(i)
		{
			case 0:
			{
				ROLL_MOTOR_A_P = HIGH;
				ROLL_MOTOR_A_N = LOW;
				ROLL_MOTOR_B_P = LOW;
				ROLL_MOTOR_B_N = LOW;
				break;
			}
			
			case 1:
			{
				ROLL_MOTOR_A_P = LOW;
				ROLL_MOTOR_A_N = LOW;
				ROLL_MOTOR_B_P = HIGH;
				ROLL_MOTOR_B_N = LOW;
				break;
			}
			
			case 2:
			{
				ROLL_MOTOR_A_P = LOW;
				ROLL_MOTOR_A_N = HIGH;
				ROLL_MOTOR_B_P = LOW;
				ROLL_MOTOR_B_N = LOW;
				break;
			}
			
			case 3:
			{
				ROLL_MOTOR_A_P = LOW;
				ROLL_MOTOR_A_N = LOW;
				ROLL_MOTOR_B_P = LOW;
				ROLL_MOTOR_B_N = HIGH;
				break;
			}
		}
		delay_ms(speed);
	}		
}

void static roll_motor_counterclockwise(int speed)
{
	uint8_t i;
	
    for(i=0;i<4;i++)  
    {  
		switch(i)
		{
			case 0:
			{
				ROLL_MOTOR_A_P = LOW;
				ROLL_MOTOR_A_N = LOW;
				ROLL_MOTOR_B_P = LOW;
				ROLL_MOTOR_B_N = HIGH;
				break;
			}
			
			case 1:
			{
				ROLL_MOTOR_A_P = LOW;
				ROLL_MOTOR_A_N = HIGH;
				ROLL_MOTOR_B_P = LOW;
				ROLL_MOTOR_B_N = LOW;
				break;
			}
			
			case 2:
			{
				ROLL_MOTOR_A_P = LOW;
				ROLL_MOTOR_A_N = LOW;
				ROLL_MOTOR_B_P = HIGH;
				ROLL_MOTOR_B_N = LOW;
				break;
			}
			
			case 3:
			{
				ROLL_MOTOR_A_P = HIGH;
				ROLL_MOTOR_A_N = LOW;
				ROLL_MOTOR_B_P = LOW;
				ROLL_MOTOR_B_N = LOW;
				break;
			}
		}
		delay_ms(speed);
	}		
}


void static roll_motor_stop(void)
{ 
	ROLL_MOTOR_A_P = LOW;
	ROLL_MOTOR_A_N = LOW;
	ROLL_MOTOR_B_P = LOW;
	ROLL_MOTOR_B_N = LOW;	
}


void roll_motor_location_set(int new_location, uint32_t speed)
{
	int i, j, angle;
	char dir = 0;
	angle = new_location - roll_motor.location;
	roll_motor.location = new_location;
	roll_motor_angle_ctrl(angle, speed);
}
void roll_motor_angle_ctrl(int angle, uint32_t speed)
{
	int i, j;
	char dir = 0;
	
	if(angle >= 0)
	{
		dir = 1;
	}
	else
	{
		dir =0;
	}
	
	j=(int)(abs(angle/7.2));
	for(i=0;i<j;i++)
	{
		if(dir)
		{
			roll_motor_clockwise(speed);
		}
		else
		{
			roll_motor_counterclockwise(speed);
		}
	}
	roll_motor_stop();
}





