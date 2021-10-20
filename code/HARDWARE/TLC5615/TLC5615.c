/**
  * @attention
  *	��ģת��оƬTLC5615��������
  * ʹ��4ƬTLC5615ͨ��Ƭѡ��ͬʱ��ʵ�����м�С��ʱ�����4·��ֵΪ0-5V��ģ���ź�
  * ���ģ���ѹ�����ֵ��2���Ĳο���ѹ����Ӳ����ѡ��MC1403�ṩ׼ȷ��2.5V��ѹ
  * ʹ��STM32���ڲ���SPIͨ��������TLC5615���������źţ�����Ƶ��263KHz��������1MHz��
  *
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
#include "tlc5615.h"	

/*SPI1��ʼ����*/
void TLC5615_Init(void)			 
{	
	RCC->APB2ENR|=1<<2;       //PORTAʱ��ʹ�� 
	RCC->APB2ENR|=1<<4;		//ʱ��GPIOCʹ��	
	RCC->APB2ENR|=1<<12;      //SPI1ʱ��ʹ�� 
	
	GPIOA->CRL&=0;
	GPIOC->CRH&=0;	
	GPIOC->CRH|=(0X33<<0);//PC8.9 �������������ٶ�50M
	GPIOA->CRL|=0XBBB33300;//PA2.3 �������������ٶ�50M��PA4.5.6.7���� �������������ٶ�50M	 
	GPIOA->ODR|=3<<2;    //PA2.3�����
	GPIOC->ODR|=3<<8;      //PC8.9�����	
	GPIOA->BSRR|=0X7<<21;  //PA5.6.7�����		

	/*����TLC5615��DATASHEET�е���Ϣ����SPIģʽ,��ʵ��˫��ͨ��*/	
	SPI1->CR1=0;
	SPI1->CR1|=0<<0; //���ݲ����ӵڶ���ʱ����ؿ�ʼ,CPHA=0
	SPI1->CR1|=0<<1; //����ģʽ��SCKΪ0, CPOL=0	 	
	SPI1->CR1|=1<<2; //SPI����	
	SPI1->CR1|=7<<3; //Fsck=Fcpu/256=263KHz		,6<<3,��526K
	SPI1->CR1|=0<<7; //MSBfirst 
	
	SPI1->CR1|=1<<8;  // ����NSSΪ�ߵ�ƽ	
	SPI1->CR1|=1<<9; //�������	NSS	
	SPI1->CR1|=0<<10;//ȫ˫��ģʽ	
	SPI1->CR1|=3<<14;//����˫��ֻ��ģʽ
	SPI1->CR1|=1<<11;//16bit���ݸ�ʽ
	
	
	SPI1->CR1|=1<<6; //����SPI�豸	
	
//	delay_us(5);
//	Write_5615ZERO(0x0ffc);
}

void Write_5615ZERO(u16 i)
{
	u16 retry=0;	
	/***********��һ·**********/
	while((SPI1->SR&1<<1)==0)//�ȴ���������	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}	
	SPI1->DR=i;	//����һ��byte 
	delay_us(1);	
}
void Write_5615YI(u16 i)
{
	u16 retry=0;	
	/***********��һ·**********/	
	CS1=0;
	while((SPI1->SR&1<<1)==0)//�ȴ���������	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}			  
	SPI1->DR=i;	//����һ��byte 
	delay_us(1);
	CS1=1;
}	
void Write_5615ER(u16 i)
{
	u16 retry=0;	
	/***********�ڶ�·**********/
	CS2=0;
	while((SPI1->SR&1<<1)==0)//�ȴ���������	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}			  
	SPI1->DR=i;	//����һ��byte 
	delay_us(1);
	CS2=1;				
}	

void Write_5615SAN(u16 i)
{
	u16 retry=0;
	/***********����·**********/
	CS3=0;
	while((SPI1->SR&1<<1)==0)//�ȴ���������	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}			  
	SPI1->DR=i;	//����һ��byte 
	delay_us(1);
	CS3=1;			
}	
void Write_5615SI(u16 i)
{
	u16 retry=0;
	/***********����·**********/
	CS4=0;
	while((SPI1->SR&1<<1)==0)//�ȴ���������	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}			  
	SPI1->DR=i;	//����һ��byte 
	delay_us(1);
	CS4=1;	
	///
//	while((SPI1->SR&1<<1)==0)//�ȴ���������	
//	{
//		retry++;
//		if(retry>0XFFFE)
//			break;
//	}			
//	while((SPI1->SR&1<<7)==1)//�ȴ�æ��־����	
//	{
//		retry++;
//		if(retry>0XFFFE)
//			break;
//	}		
	
}


	



