#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h"
#include "HC165.h"
#include "IO.h"
#include "malloc.h"	
#include "adc.h"

/***************REFER TO ALIENTEK*******************/

////////////////////////////////////////////////////////////////////////////////// 	
//#define USART_REC_LEN  			200  	//�����������ֽ��� 200
//#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
//extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
//extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��

__align(1) union FromUp
{
	u16 Data16_bits;
	u8  Data8_bits[2];
};


void uart_init(u32 bound);
void Print(void);
void Update_Dac(void);
void Uart1_PutChar(u8 ch);
#endif


