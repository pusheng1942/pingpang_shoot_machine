#include "delay.h"
#include "sys.h"
#include "includes.h" 

/////////////////////////UCOSII任务设置///////////////////////////////////
#define START_TASK_PRIO      			              12 //开始任务的优先级设置为最低
#define START_STK_SIZE  				              64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
void start_task(void *pdata);	

//ball shooting
#define BALL_SHOOTING_TASK_PRIO       			       10
#define BALL_SHOOTING_STK_SIZE  		    		   1024
OS_STK  BALL_SHOOTING_TASK_STK[BALL_SHOOTING_STK_SIZE];
void ball_shooting_task(void *pdata);

//pitch
#define PITCH_STEP_MOTOR_TASK_PRIO       			           6
#define PITCH_STEP_MOTOR_STK_SIZE  					           1024
OS_STK  PITCH_STEP_MOTOR_TASK_STK[PITCH_STEP_MOTOR_STK_SIZE];
void pitch_step_motor_task(void *pdata);

//yaw
#define YAW_STEP_MOTOR_TASK_PRIO       			               7 
#define YAW_STEP_MOTOR_STK_SIZE  					           1024
OS_STK  YAW_STEP_MOTOR_TASK_STK[YAW_STEP_MOTOR_STK_SIZE];
void yaw_step_motor_task(void *pdata);

//roll
#define ROLL_STEP_MOTOR_TASK_PRIO       			           8 
#define ROLL_STEP_MOTOR_STK_SIZE  					           1024
OS_STK  ROLL_STEP_MOTOR_TASK_STK[YAW_STEP_MOTOR_STK_SIZE];
void roll_step_motor_task(void *pdata);
extern UART_DECODE_STRUCT uart_decode;

extern STRUCT_MOTOR_STATUS motor;
extern STRUCT_STEPPER_MOTOR pitch_motor;
extern STRUCT_STEPPER_MOTOR yaw_motor;
extern STRUCT_STEPPER_MOTOR roll_motor;

#define RESET_START 0
#define RESET_DONE  1

int main(void)
{	
	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(115200);	 
	LED_Init();		  	//初始化与LED连接的硬件接口
	ball_check_config();
	Motor_Init();	
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	
}

	  
// create and start task
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	
  	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
	
// 	OSTaskCreate(ball_shooting_task,(void *)0,(OS_STK*)&BALL_SHOOTING_TASK_STK[BALL_SHOOTING_STK_SIZE-1],BALL_SHOOTING_TASK_PRIO);						   
// 	OSTaskCreate(pitch_step_motor_task,(void *)0,(OS_STK*)&PITCH_STEP_MOTOR_TASK_STK[PITCH_STEP_MOTOR_STK_SIZE-1],PITCH_STEP_MOTOR_TASK_PRIO);	
// 	OSTaskCreate(yaw_step_motor_task,(void *)0,(OS_STK*)&YAW_STEP_MOTOR_TASK_STK[YAW_STEP_MOTOR_STK_SIZE-1],YAW_STEP_MOTOR_TASK_PRIO);	 				   
 	OSTaskCreate(roll_step_motor_task,(void *)0,(OS_STK*)&ROLL_STEP_MOTOR_TASK_STK[ROLL_STEP_MOTOR_STK_SIZE-1],ROLL_STEP_MOTOR_TASK_PRIO);	 				   
	
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

//ball shooting task
void ball_shooting_task(void *pdata)
{	 
	while(1)
	{
		dc_shoot_motor_ctrl(20, 20);
		motor_run(motor);
		
		if(g_flag.motor_event)
		{
			motor_event_send(g_flag.motor_event);
			g_flag.motor_event = 0;
		}
	};
}

//pitch_step_motor
void pitch_step_motor_task(void *pdata)
{	  
	int speed = 30; //2 到20
	pitch_motor.status = RESET_START; 
	while(1)
	{
		if(pitch_motor.status == RESET_START)
		{
			pitch_motor_reset();
			pitch_motor.status = RESET_DONE;
		}
		else
		{
			if(g_flag.uart_rx_int)
			{
				uart_rx_process();
			}
			pitch_motor_angle_ctrl(-15, speed); //2 to 10
			delay_ms(5000);
			LED0 =1;
			pitch_motor_angle_ctrl(15, speed); //2 to 10
			delay_ms(5000);
			LED0 =0;
		}
	};
}

//yaw_step_motor
void yaw_step_motor_task(void *pdata)
{	 
	int speed = 30; //2 到20
	yaw_motor.status = RESET_START;
	while(1)
	{
		if(yaw_motor.status == RESET_START)
		{
			yaw_motor_reset();
			yaw_motor.status = RESET_DONE;
		}
		else
		{
			yaw_motor_location_set(-40, speed);
			delay_ms(1000);
			yaw_motor_location_set(0, speed);
			delay_ms(1000);
			yaw_motor_location_set(40, speed); //2 to 10
			delay_ms(1000);

		};
	}
}

//roll_step_motor
void roll_step_motor_task(void *pdata)
{	  
	int speed = 26; //2 到20
	
	roll_motor.status = RESET_START;
	while(1)
	{
		if(roll_motor.status == RESET_START)
		{
			roll_motor_reset();
			roll_motor.status = RESET_DONE;
		}
		else
		{
			roll_motor_location_set(50, speed);
			delay_ms(1000);
			roll_motor_location_set(1, speed);// 0 bug ...
			delay_ms(1000);
			roll_motor_location_set(-50, speed);	
			delay_ms(1000);
		}

	};
}


