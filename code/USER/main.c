/**
  ******************************************************************************
  * @brief   Main program body
  * @version V1.0
  * @date    15-April-2017
  * @author  ZhangYaokun
  ******************************************************************************
  * @attention
  *	΢������ѡ��STM32F103RCT6,������64����װLQPF����������Keil Version5��MDK5.14��
  * �����жϿ��ƹ��ɳ�����Ҫ��ܣ�
  * TIM6��ʱ���жϸ�����Ҫ����Ϣ�ɼ���Ƶ��Ϊ2��4HZ�����ȼ�2��1
  * TIM7��ʱ���жϸ������ⲿDACоƬ�������ݣ�Ϊ��ʹģ�����������������Ƶ��15KHZ��50KHZ�����ȼ�1��0
  * ���Ź��жϣ���Ϊ�˱�������Ÿ��ţ���֤��Ƭ���ܹ�ʼ���������������ȼ�0��0����ߣ�
  * ���ڽ����жϣ����������λ�����������ݣ�������ȣ����ȼ�1��1
  *	��·�ⲿ�����ص�ƽ�����жϣ����ڲɼ��Ը�λ��������Ϣ�����ȼ�2��0
  ******************************************************************************
  */  
  
/* Includes ------------------------------------------------------------------*/
#include "delay.h"
#include "time.h"
#include "IO.h"
#include "adc.h"
#include "usart.h"
#include "HC165.h"
#include "tlc5615.h"
#include "HC595.h"
#include "malloc.h"	
#include "wwdg.h"	
/* ---------------------------------------------------------------------------*/
extern u8 Send_ToUp_Flag;
extern u16 ADC_ori[6];
extern u8 ADC_Flag;
extern u8 Send_Test_Flag;
extern u8 OK_Flag;
extern u8 Cancle_Flag;
extern u8 Uart_OK_Flag;
extern u8 CRC_Flag2;
extern union FromUp *RX_BUF;	

u8 Malloc_flag=1;
u8 Display_Flag=0;

//union FromUp A;
//extern u8 Digital_Flag;
//extern u8 Analog_Flag;
 int main(void)
 {	
	static u8 a=1;//����һ�Σ�����ȷ��ͨ������
	 
	 /******************ϵͳ����*****************/
	MY_NVIC_PriorityGroupConfig(0X02);//����NVIC����,��2,2λ��ռ���ȼ�,2λ��Ӧ���ȼ�
	JTAG_Set(0X01);//ʹ��SWD����JTAG
	 
	/******************�����ʼ��*****************/	  	 
	delay_init();	    	 //��ʱ������ʼ��
    uart_init(9600);     //����1��ʼ��
	LED_Init();		  	 	//IO�ڳ�ʼ��
	Init_595();	 			//����ܳ�ʼ��
	Init_HC165();			//74HC165��ʼ��
	Adc_Init();				//ADC1��ʼ��
	DMA1_ADC_Init(6,(u32)&ADC1->DR,(u32)ADC_ori); //DMA��ʼ��
	TLC5615_Init();		//DAת��оƬ��ʼ��
	EXTIx_Init();	 	//�ⲿ�жϳ�ʼ��
	 
	TIM6_Init(9000-1,4000-1); // 2Hz���Ƶ�ʣ�
	TIM7_Init(60-1,60-1);	//20KHz�ж�Ƶ�ʣ���С��15K��������80K 
	
	WWDG_Init(0X7F,0X4F,WWDG_Prescaler_8); //���Ź���ʼ�� 
	
	RX_BUF=(union FromUp *)mymalloc(3072);	//����������3K�ֽڵ��ڴ�	

	BEEP=1;
	delay_ms(1000);//�ϵ磬����������һ������ʾϵͳ��ʼ����
	BEEP=0;
	
//	A.Data8_bits[0]=0X11;//��֤�ڴ浥Ԫ���ݴ��˳��
//	A.Data8_bits[1]=0X22;

	while(1)
	{
	/************************�������ʾ���º���***********************/
		if(ADC_Flag)
		{
			ADC_Flag=0;
			Update_Tube();	//��������ܵ�ֵ
			write_595(20);	//��Ϊÿ�η��͵����һλ�Ǵ�����74HC595����λ�Ĵ����У���û��
		}					//ת�Ƶ���ʾ�Ĵ����У���Ҫ�෢һλ����Ҫ��ʾ�����һλ������
		
	/********************ͨ�Ź��̣�����λ����������*********************/
		if(Uart_OK_Flag)//ȷ��ͨ�ųɹ�
		{			
			if(a)
			{	
			//	Uart1_PutChar(0X5E);//
				printf("^");
				a=0;//ִֻ��һ�Σ�����ȷ������λ��ͨ�ųɹ�
				
//				printf("%X",A.Data16_bits);
				
				BEEP=1;//��������Ъ������������ʾ����λ������ͨ�ųɹ�
				delay_ms(500);
				BEEP=0;
				delay_ms(500);	
				BEEP=1;
				delay_ms(500);	
				BEEP=0;	
			}		
		}
		
		if(Malloc_flag&&CRC_Flag2)//���ܸô�����ʱ��������ϴ�����
		{
			myfree(RX_BUF);	//�ͷ��ڴ�,����ϴ�����
			RX_BUF=(union FromUp *)mymalloc(3072);	//����3K�ֽ�`																			
//				printf("ok");	//�����ڴ�����ɹ�
			Malloc_flag=0;//�����־λ�������ظ������ڴ�
		}
			
		if(Send_ToUp_Flag==1)//ȷ�ϼ����»����������仯������ʵ��̨����		
		{			//Analog_Flag|| Digital_Flag || OK_Flag || Cancle_Flag
			Print();//����λ������һ�����ݣ�ʵ��̨�ţ�18λ��������4λģ������	
			Send_ToUp_Flag=0;
//			Analog_Flag=0;
//			Digital_Flag=0;
			OK_Flag=0;
			Cancle_Flag=0;			
		}
		
		if(Send_Test_Flag)//���ڲ������ݽ��ճɹ�
		{				
			Update_Dac();
			Display_Flag=1;//���ж���ģ������ʾ��ɺ�����
			Send_Test_Flag=0;	
		}
		
//			if(Fail_Flag)	//����λ����������ʧ��
//			{
//				Fail_Flag=0;
//				printf("@");//�������ݴ���Ҫ�����·���
//			}		
	}
 }
