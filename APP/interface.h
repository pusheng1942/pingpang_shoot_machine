#ifndef __INTERFACE_H
#define __INTERFACE_H
#include "stdio.h"	
#include "sys.h" 

#define SEND_BUF_SIZE 1024	//发送数据长度,最好等于sizeof(TEXT_TO_SEND)+2的整数倍.

/*************************
*********** Pkt **********
*************************/
#define PKT_DECODE_STA_PRE_SYNC_1   0x11
#define PKT_DECODE_STA_PRE_SYNC_2   0x12
#define PKT_DECODE_STA_CHECK_TYPE   0x21
#define PKT_DECODE_STA_CHECE_SUM    0x22
#define PKT_DECODE_STA_COUNT_LENGTH 0x31
#define PKT_DECODE_STA_CHECK_END    0x41
#define PKT_DECODE_STA_RESERVED_2   0x43
#define PKT_DECODE_STA_PKT_CATCHED  0x51
#define PKT_DECODE_STA_PAYLOAD      0x61

#define PKT_DECODE_STA_AT_SYNC_1 	0x71
#define PKT_DECODE_STA_AT_SYNC_2 	0x72
#define PKT_DECODE_STA_AT_DATA	 	0x73

#define PKT_HEADER_1 0x5a
#define PKT_HEADER_2 0xa5
#define PKT_RESERVED 0xFF//header == 0x5aa5; lower byte first. but for communicate with app, keep [0] == AA; [1] == 55

//SYSTEM CMD
#define PKT_TYPE_CONNECT				0x41
#define PKT_TYPE_CONNECT_ACK			0x51
#define PKT_TYPE_DISCONNECT				0x42
#define PKT_TYPE_DISCONNECT_ACK			0x52
#define PKT_TYPE_RESET					0x43
#define PKT_TYPE_RESET_ACK				0x53
#define PKT_TYPE_SYSTEM_ERROR			0x54
#define PKT_TYPE_BT_CMD					0x45
#define PKT_TYPE_BT_CMD_ACK				0x55


//CONTROL CMD
#define PKT_TYPE_SET_DC_MOTOR			0x21
#define PKT_TYPE_SET_DC_MOTOR_ACK		0x31
#define PKT_TYPE_UP_BALL_NUMBER			0x32
#define PKT_TYPE_SET_STEPPER_MOTOR		0x23
#define PKT_TYPE_STEPPER_MOTOR_ACK		0x33

//AUTO RUN CMD
#define PKT_TYPE_SET_PROGRAMS			0x61
#define PKT_TYPE_SET_PROGRAMS_ACK		0x71
#define PKT_TYPE_AUTO_RUN				0x62
#define PKT_TYPE_AUTO_RUN_ACK			0x72


//verify_code
#define RESET_CODE 0x04030201
typedef struct
{
	uint8_t		uart_rx_int;//串口1有新中断
	uint8_t		uart_new_byte;//uart1接收到新字节
	uint8_t		uart_idel;
	uint8_t		uart_data;
	uint32_t	motor_event;
	uint8_t 	new_dc_motor_config;
	uint8_t 	new_stepper_motor_config;
}STRUCT_GLOBAL_FLAG;

typedef struct
{
    uint8_t type_reg;
    uint8_t check_sum;
    uint8_t payload_reg[500];
    uint8_t payload_count;
    uint16_t payload_length;
    uint8_t decode_sta;
	uint8_t ball_num;
}UART_DECODE_STRUCT;


typedef struct
{
    uint8_t last_device;

}SYSTEM_STATUS_STRUCT;

extern uint8_t SendBuff[SEND_BUF_SIZE];
extern STRUCT_GLOBAL_FLAG g_flag;
extern SYSTEM_STATUS_STRUCT system_status;
uint8_t uart_rx_process(void);
void motor_event_send(uint32_t event);
void set_motor_up_ball_event(void);
void set_sys_error(uint8_t type , uint8_t data);
void sys_error_clear(void);

#endif
