/**
  * @attention
  *	IO�����������ļ�
  * һ������Ϊ����ڣ�����LED�ͷ�������������һ������Ϊ����ڣ������ⲿ�жϡ�
  * LED1��Ϊϵͳ����ָʾ�ƣ�LED0Ϊ״ָ̬ʾ�ƣ�BEEPΪ״̬����
  * 
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
#include "IO.h"
/* ---------------------------------------------------------------------------*/
	    
//��ͨIO��ʼ������ʼ��PD2��PB3,4,5Ϊ�����.��ʹ���������ڵ�ʱ��	
void LED_Init(void)
{ 
     GPIO_InitTypeDef  GPIO_InitStructure;
        
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB,PD�˿�ʱ��
   
//	 GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); /*ʹ��SWD ����JTAG*/	

	/******PD2,PB3,4����LED��BEEP******/	
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 |GPIO_Pin_4;//PB3,4 �˿�����
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
     GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB3,4,5
     GPIO_SetBits(GPIOB,GPIO_Pin_3 |GPIO_Pin_4);	 //PB3,4 �����
            
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	    		 //PD.2 �˿�����, �������
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		
     GPIO_Init(GPIOD, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
     GPIO_SetBits(GPIOD,GPIO_Pin_2);   						//PD2  �����
					
}
 
//IO��Ϊ�ⲿ�жϳ�ʼ������ʼ��PA0,PA1��Ϊ�����.��ʹ���������ڵ�ʱ��
void KEY_Init(void)
{
     GPIO_InitTypeDef  GPIO_InitStructure;
        
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PA�˿�ʱ��	

	/******PA0,PA1���ڰ�������******/	
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1;//PA0,A1 �˿�����
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //����һ�˽�VCC�������ó��������룬�Խ����������ж�   
     GPIO_Init(GPIOA, &GPIO_InitStructure);			//�����趨������ʼ��GPIOA0       					
}

//�ⲿ�ж�����
void EXTIx_Init(void)		//ֱ�ӵ���sys.c���ⲿ�жϵ�����
{
	KEY_Init();
	Ex_NVIC_Config(GPIO_A,0,RTIR);//PA0���ⲿ�����ش���
	Ex_NVIC_Config(GPIO_A,1,RTIR);//PA1���ⲿ�����ش���
	MY_NVIC_Init(2,0,EXTI0_IRQn,2);//�ж����ȼ���Ϊ��
	MY_NVIC_Init(2,0,EXTI1_IRQn,2);//�ж����ȼ���Ϊ��
}

u8 OK_Flag=0;
u8 Cancle_Flag=0;
u8 Send_Test_Flag=0;

//�ⲿ�ж�0�������
void EXTI0_IRQHandler(void)
{
	delay_ms(8);	//����
	if(KEY_OK==1)	//��Ӧȷ�ϰ��� 
	{
		OK_Flag=1;
		Cancle_Flag=1;
	}		 
	EXTI->PR=1<<0;  //���LINE0�ϵ��жϱ�־λ  
}
//�ⲿ�ж�1�������
void EXTI1_IRQHandler(void)
{			
	delay_ms(8);   //����			 
    if(KEY_NO==1)	//��Ӧȡ������
	{
		Send_Test_Flag=1;
	}
 	EXTI->PR=1<<1;     //���LINE1�ϵ��жϱ�־λ  
}
