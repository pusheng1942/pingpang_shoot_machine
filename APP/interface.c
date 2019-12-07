#include "interface.h"
#include "usart.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "motor_drv.h"
#include "auto_run.h"

STRUCT_GLOBAL_FLAG g_flag;
UART_DECODE_STRUCT uart_decode;
SYSTEM_STATUS_STRUCT system_status;
extern STRUCT_MOTOR_STATUS motor;

uint8_t SendBuff[SEND_BUF_SIZE];	//发送数据缓冲区
uint8_t SendBuf[100] = {0x5a , 0xa5};
uint8_t system_error[10] = {0};

static uint8_t get_check_sum(uint8_t * data , uint8_t length)
{
	uint8_t temp = 0 , i;
	if(length > 20)
		return 0xff;
	for(i = 0 ; i < length ; i ++)
	{
		temp += data[i];
	}
	return temp;
}

uint8_t update_uart_pkt_decode_status(void)
{
	uint16_t rx_data_length = 0;
	rx_data_length = get_data_rx_rd_length();
	while(rx_data_length--)
	{
		switch(uart_decode.decode_sta)
		{
			case PKT_DECODE_STA_PRE_SYNC_1:
				if (get_uart_rx_rd_data() == PKT_HEADER_1)
				{
					uart_decode.decode_sta = PKT_DECODE_STA_PRE_SYNC_2;
				}
				else if (get_uart_rx_rd_data() == 'O')
				{
					uart_decode.decode_sta = PKT_DECODE_STA_AT_SYNC_2;
				}
			break;
				
			case PKT_DECODE_STA_AT_SYNC_2:
				if(get_uart_rx_rd_data() == 'K')
				{
					uart_decode.decode_sta = PKT_DECODE_STA_AT_DATA;
					uart_decode.payload_reg[0] = 'O';
					uart_decode.payload_reg[1] = 'K';
					uart_decode.payload_count = 2;
					uart_decode.type_reg = PKT_TYPE_BT_CMD_ACK;
				}
			break;

			case PKT_DECODE_STA_PRE_SYNC_2:
				if (get_uart_rx_rd_data() == PKT_HEADER_2)
				{
					uart_decode.decode_sta = PKT_DECODE_STA_CHECK_TYPE;
				}
				else 
				{
					uart_decode.decode_sta = PKT_DECODE_STA_PRE_SYNC_1;					
				}
			break;

			case PKT_DECODE_STA_CHECK_TYPE://type
				switch(get_uart_rx_rd_data())
				{
					//SYSTEM CMD
                    case PKT_TYPE_CONNECT:
						uart_decode.type_reg = PKT_TYPE_CONNECT;
						uart_decode.decode_sta = PKT_DECODE_STA_CHECE_SUM;
						uart_decode.payload_length = 1;
						uart_decode.payload_count = 0;
					break;
					
					case PKT_TYPE_DISCONNECT:
						uart_decode.type_reg = PKT_TYPE_DISCONNECT;
						uart_decode.decode_sta = PKT_DECODE_STA_CHECE_SUM;
						uart_decode.payload_length = 1;
						uart_decode.payload_count = 0;
					break;
					
					case PKT_TYPE_RESET:
						uart_decode.type_reg = PKT_TYPE_RESET;
						uart_decode.decode_sta = PKT_DECODE_STA_CHECE_SUM;
						uart_decode.payload_length = 4;
						uart_decode.payload_count = 0;
					break;
					
					case PKT_TYPE_BT_CMD:
						uart_decode.type_reg = PKT_TYPE_BT_CMD;
						uart_decode.decode_sta = PKT_DECODE_STA_CHECE_SUM;
						uart_decode.payload_length = 30;
						uart_decode.payload_count = 0;
					break;
					
					//CONTROL CMD
					case PKT_TYPE_SET_DC_MOTOR:
						uart_decode.type_reg = PKT_TYPE_SET_DC_MOTOR;
						uart_decode.decode_sta = PKT_DECODE_STA_CHECE_SUM;
						uart_decode.payload_length = 4;
						uart_decode.payload_count = 0;
					break;
					
					case PKT_TYPE_SET_STEPPER_MOTOR:
						uart_decode.type_reg = PKT_TYPE_SET_STEPPER_MOTOR;
						uart_decode.decode_sta = PKT_DECODE_STA_CHECE_SUM;
						uart_decode.payload_length = 7;
						uart_decode.payload_count = 0;
					break;
					
					//AUTO RUN CMD
					case PKT_TYPE_SET_PROGRAMS:
						uart_decode.type_reg = PKT_TYPE_SET_PROGRAMS;
						uart_decode.decode_sta = PKT_DECODE_STA_CHECE_SUM;
//						uart_decode.payload_length = 301;
					    uart_decode.ball_num =  get_uart_rx_rd_data(); ///////////////////////////
						uart_decode.payload_length = uart_decode.ball_num*6;
						uart_decode.payload_count = 0;
					break;
					
					case PKT_TYPE_AUTO_RUN:
						uart_decode.type_reg = PKT_TYPE_AUTO_RUN;
						uart_decode.decode_sta = PKT_DECODE_STA_CHECE_SUM;
						uart_decode.payload_length = 1;
						uart_decode.payload_count = 0;
					break;
					
					default: 
						uart_decode.type_reg = 0;
						uart_decode.decode_sta = PKT_DECODE_STA_PRE_SYNC_1;
					break;
				}
			break;

			case PKT_DECODE_STA_CHECE_SUM ://0xFF reserved
				uart_decode.check_sum = get_uart_rx_rd_data();
				uart_decode.decode_sta = PKT_DECODE_STA_COUNT_LENGTH;
			break;

			
			case PKT_DECODE_STA_AT_DATA://在此循环，直到接收完成。
				uart_decode.payload_reg[uart_decode.payload_count] = get_uart_rx_rd_data();
				if(uart_decode.payload_reg[uart_decode.payload_count] == 0x0a)
				{
					uart_decode.decode_sta = PKT_DECODE_STA_PKT_CATCHED;
					return PKT_DECODE_STA_PKT_CATCHED;
				}
				else
				{
					uart_decode.payload_count++;
				}
			
			break;
			
			case PKT_DECODE_STA_COUNT_LENGTH://在此循环，直到接收完成。
				uart_decode.payload_reg[uart_decode.payload_count] = get_uart_rx_rd_data();
//				uart_decode.payload_count++;
				if (uart_decode.payload_count == (uart_decode.payload_length - 1))
				{	
					uart_decode.decode_sta = PKT_DECODE_STA_CHECK_END;
				}
				else
				{
					uart_decode.payload_count++;
					break;
				}

			case PKT_DECODE_STA_CHECK_END:
				if(uart_decode.check_sum == get_check_sum(uart_decode.payload_reg,uart_decode.payload_length))
//				if(uart_decode.check_sum == 0xAA)
					uart_decode.decode_sta = PKT_DECODE_STA_PKT_CATCHED;
				else
				{
					if(uart_decode.check_sum == 0xff)
					{
						uart_decode.decode_sta = PKT_DECODE_STA_PKT_CATCHED;
					}
					else
					{
						printf("PKT check sum error\r\n");
						uart_decode.decode_sta = PKT_DECODE_STA_PRE_SYNC_1;
					}
				}
			break;
				
			case PKT_DECODE_STA_PKT_CATCHED:
				return PKT_DECODE_STA_PKT_CATCHED;
			
			default:
				printf("PKT error\r\n");
				uart_decode.decode_sta = PKT_DECODE_STA_PRE_SYNC_1;
				return 0;
			
		}
	}
	return uart_decode.decode_sta;
}

uint8_t deal_with_uart_data(void)
{
	uint8_t uart_ack[50]={PKT_HEADER_1 , PKT_HEADER_2};
	uint16_t i;
	switch(uart_decode.type_reg)
	{
		//SYSTEM CMD
        case PKT_TYPE_CONNECT:
        {
            uart_ack[2] = PKT_TYPE_CONNECT_ACK;
			uart_ack[4] = system_status.last_device;
            uart_ack[3] = get_check_sum(&uart_ack[4] , 1);
            memcpy(SendBuff , uart_ack , 5);
            uart_send_data(5);
			
			//
			system_status.last_device = uart_decode.payload_reg[0];
        }
		break;
		
		case PKT_TYPE_DISCONNECT:
        {
            uart_ack[2] = PKT_TYPE_DISCONNECT_ACK;
			uart_ack[4] = 0;
            uart_ack[3] = get_check_sum(&uart_ack[4] , 1);
            memcpy(SendBuff , uart_ack , 5);
            uart_send_data(5);
			
			//
			system_status.last_device = 0;
        }
		break;
		
		case PKT_TYPE_RESET:
        {
			//
			uint32_t verify_code = 0;
			memcpy(&verify_code , &uart_decode.payload_reg[0] , 4);
			if(verify_code == RESET_CODE)
			{
				uart_ack[4] = 1;
				motor_reset();
			}
			else
			{
				uart_ack[4] = 0;
			}
				
			//
            uart_ack[2] = PKT_TYPE_RESET_ACK;
            uart_ack[3] = get_check_sum(&uart_ack[4] , 1);
            memcpy(SendBuff , uart_ack , 5);
            uart_send_data(5);
        }
		break;
		
		case PKT_TYPE_BT_CMD:
        {
			//
			uint8_t length = uart_decode.payload_reg[0];
            memcpy(SendBuff , &uart_decode.payload_reg[1] , length);
            uart_send_data(length);
        }
		break;
		
		case PKT_TYPE_BT_CMD_ACK:
        {
			uint8_t length = strlen((const char*)&uart_decode.payload_reg[0]);
			if(length < 30)
			{
				uart_ack[4] = length;
				memcpy(&uart_ack[5] , &uart_decode.payload_reg[0] , length);
			}
			else
			{
				uart_ack[4] = 29;
				memcpy(&uart_ack[5] , &uart_decode.payload_reg[0] , 29);
			}
			uart_ack[2] = PKT_TYPE_BT_CMD_ACK;
            uart_ack[3] = get_check_sum(&uart_ack[4] , 30);
            memcpy(SendBuff , uart_ack , 34);
            uart_send_data(34);
        }
		break;
		
		//CONTROL CMD
		case PKT_TYPE_SET_DC_MOTOR:
        {
			if(uart_decode.payload_reg[0] & 0x04) //高位有效
			{
				g_flag.new_dc_motor_config = true;
				if(abs((int8_t)uart_decode.payload_reg[1]) <= 100)
				{
					motor.dc1.new_speed = (int8_t)uart_decode.payload_reg[1];
				}
				else
				{
					if(uart_decode.payload_reg[1] > 0)
						motor.dc1.new_speed = 100;
					else
						motor.dc1.new_speed = -100;
				}
			}
			
			if(uart_decode.payload_reg[0] & 0x02) //中位有效
			{
				g_flag.new_dc_motor_config = true;
				if(abs((int8_t)uart_decode.payload_reg[2]) <= 100)
				{
					motor.dc2.new_speed = (int8_t)uart_decode.payload_reg[2];
				}
				else
				{
					if(uart_decode.payload_reg[2] > 0)
						motor.dc2.new_speed = 100;
					else
						motor.dc2.new_speed = -100;
				}
			}
			
			if(uart_decode.payload_reg[0] & 0x01) //低位有效
			{
				g_flag.new_dc_motor_config = true;
				if(abs((int8_t)uart_decode.payload_reg[3]) <= 100)
				{
					motor.dc_up.new_speed = (int8_t)uart_decode.payload_reg[3];
				}
				else
				{
					if(uart_decode.payload_reg[3] > 0)
						motor.dc_up.new_speed = 100;
					else
						motor.dc_up.new_speed = -100;
				}
			}
			//
			uart_ack[4] = motor.dc1.status<<2 || motor.dc2.status<<1 || motor.dc_up.status;
			uart_ack[5] = motor.dc1.speed;
			uart_ack[6] = motor.dc2.speed;
			uart_ack[7] = motor.dc_up.speed;
			
            uart_ack[2] = PKT_TYPE_SET_DC_MOTOR_ACK;
            uart_ack[3] = get_check_sum(&uart_ack[4] , 4);
            memcpy(SendBuff , uart_ack , 8);
            uart_send_data(8);
			
			//
			system_status.last_device = 0;
        }
		break;
		
		case PKT_TYPE_SET_STEPPER_MOTOR:
        {
			if(uart_decode.payload_reg[0] & 0x04) //高位有效
			{
				g_flag.new_stepper_motor_config = true;
				motor.step1.new_location = uart_decode.payload_reg[1];
				motor.step1.new_speed = uart_decode.payload_reg[2];
			}
			
			if(uart_decode.payload_reg[0] & 0x02) //中位有效
			{
				g_flag.new_stepper_motor_config = true;
				motor.step2.new_location = uart_decode.payload_reg[3];
				motor.step2.new_speed = uart_decode.payload_reg[4];
			}
			
			if(uart_decode.payload_reg[0] & 0x01) //低位有效
			{
				g_flag.new_stepper_motor_config = true;
				motor.step3.new_location = uart_decode.payload_reg[5];
				motor.step3.new_speed = uart_decode.payload_reg[6];
			}
			//
			uart_ack[4] = motor.step1.status<<2 || motor.step2.status<<1 || motor.step3.status;
			uart_ack[5] = motor.step1.location;
			uart_ack[6] = motor.step1.speed;
			uart_ack[7] = motor.step2.location;
			uart_ack[8] = motor.step2.speed;
			uart_ack[9] = motor.step3.location;
			uart_ack[10] = motor.step3.speed;
			
            uart_ack[2] = PKT_TYPE_SET_DC_MOTOR_ACK;
            uart_ack[3] = get_check_sum(&uart_ack[4] , 7);
            memcpy(SendBuff , uart_ack , 11);
            uart_send_data(11);
			
			//
			system_status.last_device = 0;
        }
		break;
		//AUTO RUN CMD
		case PKT_TYPE_SET_PROGRAMS:
        {
//			uint8_t length = uart_decode.payload_reg[0];
			uint8_t length = uart_decode.ball_num;
			if(length <= 50 && length > 0)
			{
				for(i = 0 ; i < length ; i++)
				{
					memcpy(&auto_programs.array[i] , &uart_decode.payload_reg[sizeof(STRUCT_AUTO_ARRAY)*i] , sizeof(STRUCT_AUTO_ARRAY));//per ball's para
					if((auto_programs.array[i].pitch_motor > 180) || 
						(auto_programs.array[i].yaw_motor > 180) ||     //最后电机的输入参数从该结构体中获取
						(auto_programs.array[i].roll_motor > 180) || 
						(abs(auto_programs.array[i].speed1 > 100)) || 
						(abs(auto_programs.array[i].speed2 > 100)) )
					{
						uart_ack[4] = 2;
						break;
					}
				}
			}
			else
			{
				uart_ack[4] = 2;
			}
			uart_ack[2] = PKT_TYPE_SET_PROGRAMS_ACK;
            uart_ack[3] = get_check_sum(&uart_ack[4] , 1);
            memcpy(SendBuff , uart_ack , 5);
            uart_send_data(5);
        }
		
		//todo
		case PKT_TYPE_AUTO_RUN:
        {
			uint8_t new_status = uart_decode.payload_reg[0];
			if(motor.run_status == MOTOR_STATUS_STOP)
			{
				if(new_status == 1)
				{
					auto_run_init();
					uart_ack[4] = motor.up_auto_number;
				}
			}else if(motor.run_status == MOTOR_STATUS_BASE)
			{
				
			}else if(motor.run_status == AUTO_RUN_STATUS_RUN)
			{
				
			}else if(motor.run_status == AUTO_RUN_STATUS_PAUSE)
			{
				
			}else if(motor.run_status == AUTO_RUN_STATUS_FINISH)
			{
				
			}
			uart_ack[2] = PKT_TYPE_AUTO_RUN_ACK;
            uart_ack[3] = get_check_sum(&uart_ack[4] , 1);
            memcpy(SendBuff , uart_ack , 5);
            uart_send_data(5);
        }
		break;
            
		default: 
		break;
	}
	return true;
}

// uart接收缓冲区处理
uint8_t uart_rx_process(void)
{
	g_flag.uart_rx_int = false;
	//由uart接收中断触发
	if (update_uart_pkt_decode_status() == PKT_DECODE_STA_PKT_CATCHED)
	{	
		deal_with_uart_data();
		uart_decode.decode_sta = PKT_DECODE_STA_PRE_SYNC_1;
	}
	else
		;
	return true;
}

void motor_event_send(uint32_t event)
{
	uint8_t uart_ack[20]={PKT_HEADER_1 , PKT_HEADER_2};
	if(event & MOTOR_EVENT_UP_BALL)
	{
		uart_ack[2] = PKT_TYPE_UP_BALL_NUMBER; // 表示本次开机累计发球次数，主动发送。
		memcpy(&uart_ack[4] , &motor.up_total_number , 2);
		uart_ack[3] = get_check_sum(&uart_ack[4] , 2);
		memcpy(SendBuff , uart_ack , 6);
		uart_send_data(6);
	}
	
	if(event & MOTOR_EVENT_RUN_FINISH)
	{
		uart_ack[2] = PKT_TYPE_AUTO_RUN_ACK;
		uart_ack[4] = AUTO_RUN_STATUS_FINISH;
		uart_ack[3] = get_check_sum(&uart_ack[4] , 1);
		memcpy(SendBuff , uart_ack , 5);
		uart_send_data(5);
	}
	//TO DO
	if(event & MOTOR_EVENT_UP_FINISH)
	{
		
	}
}

void set_motor_up_ball_event(void)
{
	g_flag.motor_event |= MOTOR_EVENT_UP_BALL;
	motor.up_status = true;
	motor.up_total_number ++;
}

void send_system_error(void)
{
	uint8_t uart_ack[20]={PKT_HEADER_1 , PKT_HEADER_2};
	
	memcpy(&uart_ack[4] , system_error , 10);
	
	uart_ack[2] = PKT_TYPE_SYSTEM_ERROR;
	uart_ack[3] = get_check_sum(&uart_ack[4] , 10);
	memcpy(SendBuff , uart_ack , 14);
	uart_send_data(14);
}

void set_sys_error(uint8_t type , uint8_t data)
{
	system_error[type] = data;
	send_system_error();
}

void sys_error_clear(void)
{
	memset(system_error , 0 , 10);
}
