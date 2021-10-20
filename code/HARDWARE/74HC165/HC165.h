#ifndef __HC165_H
#define __HC165_H	 
#include "sys.h"
#include "delay.h"
#include "usart.h"

#define CLK    PBout(8) //ʱ�����ţ���������Ч
#define SHLD   PBout(9) //����������������
#define  QHOUT PBin(10) //��������������ţ�
						//�Ե�Ƭ����˵������Ӧ���ó�����ģʽ������165���������
void Init_HC165(void);
void Read_HC165(void);
void sep01(u8 data, u8 len,u8 *sepData);
u8 Comp_Digital(void);
void Single_bit(void);


#endif

