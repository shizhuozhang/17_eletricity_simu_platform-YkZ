/**
  * @attention
  *	�ڲ�������ʱ��TIM6��TIM7�����ļ�
  * TIM6��Ҫ���ڸ��¿�����״̬�Ͳɼ�����ģ�����ı仯���������Ƶ��2Hz��
  * TIM7��Ҫ������DACоƬ����������ʵ��ģ�������������������Ƶ��30KHz
  * �����ʾ���������������ε�����
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
#include "time.h"
/* ---------------------------------------------------------------------------*/

extern u16 Voltage[6];

/********������ʱ��7��ʼ��********/
void TIM7_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<5;//ʹ�ܶ�ʱ��ʱ��
	TIM7->ARR=arr;
	TIM7->PSC=psc;
	TIM7->DIER|=1<<0;//ʹ���ж�
	TIM7->CR1|=1<<7;//arpeʹ��
	TIM7->CR1|=1<<0;//ʹ�ܼ�����
	MY_NVIC_Init(1,0,TIM7_IRQn,2);//
}

/**********������ʱ��6��ʼ��**********/
void TIM6_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<4;//ʹ�ܶ�ʱ��ʱ��
	TIM6->ARR=arr;
	TIM6->PSC=psc;
	TIM6->DIER|=1<<0;//ʹ���ж�
	TIM6->CR1|=1<<7;//arpeʹ��
	TIM6->CR1|=1<<0;//ʹ�ܼ�����
	MY_NVIC_Init(2,1,TIM6_IRQn,2);//
}
/**********������ʱ���жϷ�����**********/

extern union FromUp *RX_BUF;	//�������ݻ�������ַָ��
extern u16 RX_LEN[4];//�����ʾ4��������ÿ��������Ӧһ�����ݳ���
extern u16 USART_RX_STA;
extern u8 Display_Flag;

#define Lim_Num  200		//�궨��������ݷ�������ȷ��ÿ�����ε���ʾ������һ����

/***��DACоƬ���ʹ���λ�����ܵ����ݣ�������ʾʵʱ����
����ʱ���ж�Ƶ��Ϊ20KHz,�����ͼ����0.05ms,���ͺͽ������ݵ�ʱ����²�����0.213ms(���ɺ���)��������Ϊ100��
ʱ��������ʾ�Ĳ��ε����ڴ�Լ��26.3ms;������Ϊ200��ʱ��������ʾ�Ĳ��ε����ڴ�Լ��53ms��
���͹����ǣ�ÿ�δ���λ���������ݣ������ڴ棬����������һ�Σ���Ҫ�ٴ��������IN2��ť
****/	
void TIM7_IRQHandler(void) 
{	
	static u8 Num=0;
	static u8 Num_L=0;
		
	if(TIM7->SR&0x0001)//�ж��Ƿ�����ж��¼����±�־
	{
		if((USART_RX_STA&0x80)||Display_Flag)//�ж������Ƿ�������,����ɲ���DAC���
		{
			Write_5615ZERO((RX_BUF+ 1 +Num)->Data16_bits);	//ÿ�θ�������֮ǰ����һ�¼Ĵ����������������	
			Write_5615YI((RX_BUF+ 1 +Num)->Data16_bits);
			Write_5615ER((RX_BUF+ 1 + 1 * (RX_BUF->Data16_bits)+Num)->Data16_bits);
			Write_5615SAN((RX_BUF+ 1 + 2 * (RX_BUF->Data16_bits)+Num)->Data16_bits);
			Write_5615SI((RX_BUF+ 1 + 3 * (RX_BUF->Data16_bits)+Num)->Data16_bits);
			
			Num++;
			Num_L++;
									
			if(Num>=RX_BUF->Data16_bits)	//Num++�൱�����ݵ�ַ����ƫ��
				Num=RX_BUF->Data16_bits - 1;//����������ݷ����꣬��һֱ�������һ������
			
			if(Num_L>Lim_Num)	//Num_L���ڼ�¼���͹���������
			{					//ÿ����Lim_Num�����ݣ������һ�β�����ʾ
				USART_RX_STA=0;	//��ʱ��������ɱ�־���㣬�����ٴν�������
				Num=0;	//���ݼ�¼ֵȫ������
				Num_L=0;
				Display_Flag=0;
			}	
								
		}
	}	
	TIM7->SR&=~(1<<0);//����жϱ�־λ
}
/***************���´������ڲ���DAC������������·������ξ�Ϊ���������ǲ�**************/	
//	static u8 flag1=0;
//	static u8 flag2=0;
//	if(TIM7->SR&0x0001)//�ж��Ƿ�����ж��¼����±�־
//	{
//		if(flag1)
//			datavalue1+=7;
//		else 
//			datavalue1-=7;
//		if(datavalue1<=15)
//		{ 
//			flag1=1;
//		}
//		if(datavalue1>=900)
//		{
//			flag1=0;
//		}
//		
//		if(flag2)
//			datavalue2+=4;
//		else 
//			datavalue2-=4;
//		if(datavalue2<=15)
//		{ 
//			flag2=1;
//		}
//		if(datavalue2>=930)
//		{
//			flag2=0;
//		}
//		Write_5615ZERO(datavalue1);	//ÿ�θ�������֮ǰ����һ�¼Ĵ����������������	
//		Write_5615YI(datavalue1);
//		Write_5615ER(datavalue2);
//		Write_5615SAN(datavalue1);
//		Write_5615SI(datavalue2);		
//	}
//	TIM7->SR&=~(1<<0);//����жϱ�־λ 
//}


u8 ADC_Flag=0;
u8 Digital_Flag=0;
u8 Analog_Flag=0;

void TIM6_IRQHandler(void) //500ms�ж�һ��
{
	if(TIM6->SR&0x0001)//�ж��Ƿ�����ж��¼����±�־
	{
		LED1=~LED1;
		Get_Voltage();	//�ɼ�ģ����
		ADC_Flag=Comp_Voltage(50);	//ģ�����ı仯��,���ڸ����������ʾ��50Ϊ1%�ľ���
		Analog_Flag=Comp_Voltage(50);	//ģ�����ı仯�������ڸ�������λ�����ݵķ���
		
		Read_HC165();//�ɼ���������
		Digital_Flag=Comp_Digital();//�������ı仯���		
	}
	TIM6->SR&=~(1<<0);//����жϱ�־λ 
}



