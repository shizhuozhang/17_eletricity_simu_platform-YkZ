#ifndef __HC595_H
#define __HC595_H

#include "sys.h"
#include "adc.h"

//��ֻ����ܲ��ô������ӣ�ֻ��Ҫһ�����ݴ�������
#define LOAD  PBout(5) //������������ 
#define SCL   PBout(6) //���ݴ���ʱ������   
#define SDK   PBout(7) //��������

void Init_595(void);
void write_595(u8 a);
u8 *Float_sepret(float data);
void Update_Tube(void);

#endif
