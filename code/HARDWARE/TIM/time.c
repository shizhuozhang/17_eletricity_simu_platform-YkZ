/**
  * @attention
  *	内部基本定时器TIM6和TIM7配置文件
  * TIM6主要用于更新开关量状态和采集到的模拟量的变化情况，更新频率2Hz；
  * TIM7主要用于向DAC芯片发送数据以实现模拟量的连续输出，发送频率30KHz
  * 解决用示波器测量连续波形的问题
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
#include "time.h"
/* ---------------------------------------------------------------------------*/

extern u16 Voltage[6];

/********基本定时器7初始化********/
void TIM7_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<5;//使能定时器时钟
	TIM7->ARR=arr;
	TIM7->PSC=psc;
	TIM7->DIER|=1<<0;//使能中断
	TIM7->CR1|=1<<7;//arpe使能
	TIM7->CR1|=1<<0;//使能计数器
	MY_NVIC_Init(1,0,TIM7_IRQn,2);//
}

/**********基本定时器6初始化**********/
void TIM6_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<4;//使能定时器时钟
	TIM6->ARR=arr;
	TIM6->PSC=psc;
	TIM6->DIER|=1<<0;//使能中断
	TIM6->CR1|=1<<7;//arpe使能
	TIM6->CR1|=1<<0;//使能计数器
	MY_NVIC_Init(2,1,TIM6_IRQn,2);//
}
/**********基本定时器中断服务函数**********/

extern union FromUp *RX_BUF;	//定义数据缓冲区地址指针
extern u16 RX_LEN[4];//最多显示4个变量，每个变量对应一个数据长度
extern u16 USART_RX_STA;
extern u8 Display_Flag;

#define Lim_Num  200		//宏定义最大数据发送量，确保每个波形的显示周期是一样的

/***向DAC芯片发送从上位机接受的数据，用于显示实时波形
若定时器中断频率为20KHz,即发送间隔是0.05ms,发送和接收数据的时间大致测量是0.213ms(不可忽略)，数据量为100个
时，则所显示的波形的周期大约是26.3ms;数据量为200个时，则所显示的波形的周期大约是53ms。
发送规则是：每次从上位机接收数据，存入内存，接受完成输出一次，若要再次输出可摁IN2按钮
****/	
void TIM7_IRQHandler(void) 
{	
	static u8 Num=0;
	static u8 Num_L=0;
		
	if(TIM7->SR&0x0001)//判断是否产生中断事件更新标志
	{
		if((USART_RX_STA&0x80)||Display_Flag)//判断数据是否接受完成,若完成才能DAC输出
		{
			Write_5615ZERO((RX_BUF+ 1 +Num)->Data16_bits);	//每次更新数据之前先清一下寄存器，否则会有问题	
			Write_5615YI((RX_BUF+ 1 +Num)->Data16_bits);
			Write_5615ER((RX_BUF+ 1 + 1 * (RX_BUF->Data16_bits)+Num)->Data16_bits);
			Write_5615SAN((RX_BUF+ 1 + 2 * (RX_BUF->Data16_bits)+Num)->Data16_bits);
			Write_5615SI((RX_BUF+ 1 + 3 * (RX_BUF->Data16_bits)+Num)->Data16_bits);
			
			Num++;
			Num_L++;
									
			if(Num>=RX_BUF->Data16_bits)	//Num++相当于数据地址依次偏移
				Num=RX_BUF->Data16_bits - 1;//如果该组数据发送完，则一直发送最后一个数据
			
			if(Num_L>Lim_Num)	//Num_L用于记录发送过的数据量
			{					//每发送Lim_Num个数据，即完成一次波形显示
				USART_RX_STA=0;	//此时将接收完成标志清零，便于再次接收数据
				Num=0;	//数据记录值全部清零
				Num_L=0;
				Display_Flag=0;
			}	
								
		}
	}	
	TIM7->SR&=~(1<<0);//清除中断标志位
}
/***************以下代码用于测试DAC工作正常，四路输出波形均为周期性三角波**************/	
//	static u8 flag1=0;
//	static u8 flag2=0;
//	if(TIM7->SR&0x0001)//判断是否产生中断事件更新标志
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
//		Write_5615ZERO(datavalue1);	//每次更新数据之前先清一下寄存器，否则会有问题	
//		Write_5615YI(datavalue1);
//		Write_5615ER(datavalue2);
//		Write_5615SAN(datavalue1);
//		Write_5615SI(datavalue2);		
//	}
//	TIM7->SR&=~(1<<0);//清除中断标志位 
//}


u8 ADC_Flag=0;
u8 Digital_Flag=0;
u8 Analog_Flag=0;

void TIM6_IRQHandler(void) //500ms中断一次
{
	if(TIM6->SR&0x0001)//判断是否产生中断事件更新标志
	{
		LED1=~LED1;
		Get_Voltage();	//采集模拟量
		ADC_Flag=Comp_Voltage(50);	//模拟量的变化量,用于更新数码管显示，50为1%的精度
		Analog_Flag=Comp_Voltage(50);	//模拟量的变化量，用于更新向上位机数据的发送
		
		Read_HC165();//采集数字量，
		Digital_Flag=Comp_Digital();//数字量的变化情况		
	}
	TIM6->SR&=~(1<<0);//清除中断标志位 
}



