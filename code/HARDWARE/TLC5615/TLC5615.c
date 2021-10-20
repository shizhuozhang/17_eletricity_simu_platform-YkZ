/**
  * @attention
  *	数模转换芯片TLC5615驱动程序
  * 使用4片TLC5615通过片选可同时（实际上有极小的时间差）输出4路幅值为0-5V的模拟信号
  * 输出模拟电压的最大值是2倍的参考电压，故硬件上选用MC1403提供准确的2.5V电压
  * 使用STM32中内部的SPI通信外设向TLC5615发送数字信号，发送频率263KHz（最大可至1MHz）
  *
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
#include "tlc5615.h"	

/*SPI1初始化，*/
void TLC5615_Init(void)			 
{	
	RCC->APB2ENR|=1<<2;       //PORTA时钟使能 
	RCC->APB2ENR|=1<<4;		//时钟GPIOC使能	
	RCC->APB2ENR|=1<<12;      //SPI1时钟使能 
	
	GPIOA->CRL&=0;
	GPIOC->CRH&=0;	
	GPIOC->CRH|=(0X33<<0);//PC8.9 推挽输出，最大速度50M
	GPIOA->CRL|=0XBBB33300;//PA2.3 推挽输出，最大速度50M；PA4.5.6.7复用 推挽输出，最大速度50M	 
	GPIOA->ODR|=3<<2;    //PA2.3输出高
	GPIOC->ODR|=3<<8;      //PC8.9输出高	
	GPIOA->BSRR|=0X7<<21;  //PA5.6.7输出低		

	/*按照TLC5615的DATASHEET中的信息配置SPI模式,以实现双方通信*/	
	SPI1->CR1=0;
	SPI1->CR1|=0<<0; //数据采样从第二个时间边沿开始,CPHA=0
	SPI1->CR1|=0<<1; //空闲模式下SCK为0, CPOL=0	 	
	SPI1->CR1|=1<<2; //SPI主机	
	SPI1->CR1|=7<<3; //Fsck=Fcpu/256=263KHz		,6<<3,是526K
	SPI1->CR1|=0<<7; //MSBfirst 
	
	SPI1->CR1|=1<<8;  // 设置NSS为高电平	
	SPI1->CR1|=1<<9; //软件管理	NSS	
	SPI1->CR1|=0<<10;//全双工模式	
	SPI1->CR1|=3<<14;//单线双向只发模式
	SPI1->CR1|=1<<11;//16bit数据格式
	
	
	SPI1->CR1|=1<<6; //开启SPI设备	
	
//	delay_us(5);
//	Write_5615ZERO(0x0ffc);
}

void Write_5615ZERO(u16 i)
{
	u16 retry=0;	
	/***********第一路**********/
	while((SPI1->SR&1<<1)==0)//等待发送区空	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}	
	SPI1->DR=i;	//发送一个byte 
	delay_us(1);	
}
void Write_5615YI(u16 i)
{
	u16 retry=0;	
	/***********第一路**********/	
	CS1=0;
	while((SPI1->SR&1<<1)==0)//等待发送区空	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}			  
	SPI1->DR=i;	//发送一个byte 
	delay_us(1);
	CS1=1;
}	
void Write_5615ER(u16 i)
{
	u16 retry=0;	
	/***********第二路**********/
	CS2=0;
	while((SPI1->SR&1<<1)==0)//等待发送区空	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}			  
	SPI1->DR=i;	//发送一个byte 
	delay_us(1);
	CS2=1;				
}	

void Write_5615SAN(u16 i)
{
	u16 retry=0;
	/***********第三路**********/
	CS3=0;
	while((SPI1->SR&1<<1)==0)//等待发送区空	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}			  
	SPI1->DR=i;	//发送一个byte 
	delay_us(1);
	CS3=1;			
}	
void Write_5615SI(u16 i)
{
	u16 retry=0;
	/***********第四路**********/
	CS4=0;
	while((SPI1->SR&1<<1)==0)//等待发送区空	
	{
		retry++;
		if(retry>0XFFFE)
			break;
	}			  
	SPI1->DR=i;	//发送一个byte 
	delay_us(1);
	CS4=1;	
	///
//	while((SPI1->SR&1<<1)==0)//等待发送区空	
//	{
//		retry++;
//		if(retry>0XFFFE)
//			break;
//	}			
//	while((SPI1->SR&1<<7)==1)//等待忙标志清零	
//	{
//		retry++;
//		if(retry>0XFFFE)
//			break;
//	}		
	
}


	



