/**
  * @attention
  *	8λ����ת����������λ�Ĵ���74HC165оƬ�ײ�����
  * ʹ��4ƬHC165������ͬʱ�ɼ�32λ�����ź�
  * 
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
#include "HC165.h"

void Init_HC165(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 |GPIO_Pin_9;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);						 
	GPIO_SetBits(GPIOB,GPIO_Pin_9);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //��������	
	GPIO_Init(GPIOB, &GPIO_InitStructure);		
}

u8 Position[4]={0};	//4��*8=32����������������
u8 Position_last[4]={0};//�������֮ǰ�Ŀ��ر���

void Read_HC165(void)	//ִ��һ�θó����Լ��Ҫ35ms����Ӧע�������Ƶ�ʲ��ܳ���28HZ
{
	u8 j=0,i=0;
	
	for(i=0;i<4;i++)
		Position_last[i]=Position[i];//�ȱ����ϴε�ֵ�����ڱȽ�
	
/**********************�������������ο�74HC165оƬ�ֲ��еĹ���ʱ��ͼ*******************/		
	SHLD=1;	//��ʼ��74HC165оƬ
	CLK=0;
	delay_ms(1);
	
	SHLD=0;			//��������������оƬ
	delay_ms(1);	
	SHLD=1;			//��������
	delay_ms(1);	
	
	for(j=0;j<4;j++)	//jֵΪҪ��ȡ��HC165�ļ���Ƭ��
	{
		/*��ƬHC165��������*/
		for(i=0;i<8;i++)
		{			//���ݸ�ʽHGFEDCBA������HΪ���λ��AΪ���λ��������λʱ�����Ƴ�HȻ����G��A
			Position[j]<<=1;//�ȳ���λ���λ�ƶ�
			if(QHOUT==1)
			{	
				Position[j]|= 0x0001; //QHOUT  8�κ󼴵õ��˴洢������ ����10101011
			}
			CLK=0;			//ÿһ�������ش���һ������
			delay_us(500);	
			CLK=1;
			delay_us(500);		//�����źŵĲɼ�Ƶ��Ϊ1KHz,
		}
		CLK=0;	
//		printf("\r\n");				//165�������
//		printf("digial  %x\r\n",Position[j]);//165���Գ������	
	}	
		
}

u8 Comp_Digital(void)//����Ƶ��2HZ
{
	u8 i=0;
	u8 y=0;
	for(i=0;i<4;i++)
	{
		if(Position_last[i]!=Position[i])//�仯������50����Ϊ1��50������ʾ�����ȣ�
			y+=1;		//ִ���Ĵκ�y��������4�������鿪����ȫ�ı䣻��С��0
	}
	if(y)	//ֻҪ��һ���������仯��yֵ�ʹ���1
		return 1;
	else
		return 0;
}

/*�����������ʱ����Ҫ������ת����0/1��������ʽ����λ��ʹ��*/
u8 Position_bit[4][8]={0};	

/************************ȡbitλ����************************/
 //data Ҫ�ֿ������ݣ�Ĭ��u8����
//lenΪҪ�ֳɼ�λ,���Ĭ��8λ
//���ֳɵ�bitλ�浽����SepData��
void sep01(u8 data, u8 len,u8 *sepData)
{
	u8 t;
	for(t=0;t<(len);t++)
	{
		sepData[t] = data&0x01;      
		data = data>>1;
	}
}
void Single_bit(void)
{
	u8 i=0;
	for(i=0;i<3;i++)
	{		
		sep01(Position[i], 8, Position_bit[i]);
	}
}


