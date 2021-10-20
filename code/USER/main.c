/**
  ******************************************************************************
  * @brief   Main program body
  * @version V1.0
  * @date    15-April-2017
  * @author  ZhangYaokun
  ******************************************************************************
  * @attention
  *	微控制器选用STM32F103RCT6,引脚数64，封装LQPF，开发环境Keil Version5（MDK5.14）
  * 采用中断控制构成程序主要框架：
  * TIM6定时器中断负责主要的信息采集，频率为2～4HZ，优先级2，1
  * TIM7定时器中断负责向外部DAC芯片发送数据，为了使模拟量输出更加连续，频率15KHZ到50KHZ，优先级1，0
  * 看门狗中断，是为了避免外界电磁干扰，保证单片机能够始终正常工作，优先级0，0（最高）
  * 串口接收中断，负责接收上位机传来的数据，处理保存等，优先级1，1
  *	两路外部上升沿电平触发中断，用于采集自复位按键的信息，优先级2，0
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
	static u8 a=1;//仅用一次，用于确认通信正常
	 
	 /******************系统设置*****************/
	MY_NVIC_PriorityGroupConfig(0X02);//设置NVIC分组,组2,2位抢占优先级,2位响应优先级
	JTAG_Set(0X01);//使能SWD禁用JTAG
	 
	/******************外设初始化*****************/	  	 
	delay_init();	    	 //延时函数初始化
    uart_init(9600);     //串口1初始化
	LED_Init();		  	 	//IO口初始化
	Init_595();	 			//数码管初始化
	Init_HC165();			//74HC165初始化
	Adc_Init();				//ADC1初始化
	DMA1_ADC_Init(6,(u32)&ADC1->DR,(u32)ADC_ori); //DMA初始化
	TLC5615_Init();		//DA转换芯片初始化
	EXTIx_Init();	 	//外部中断初始化
	 
	TIM6_Init(9000-1,4000-1); // 2Hz检测频率；
	TIM7_Init(60-1,60-1);	//20KHz中断频率，不小于15K，不大于80K 
	
	WWDG_Init(0X7F,0X4F,WWDG_Prescaler_8); //看门狗初始化 
	
	RX_BUF=(union FromUp *)mymalloc(3072);	//先申请申请3K字节的内存	

	BEEP=1;
	delay_ms(1000);//上电，蜂鸣器长鸣一声，提示系统开始工作
	BEEP=0;
	
//	A.Data8_bits[0]=0X11;//验证内存单元数据存放顺序
//	A.Data8_bits[1]=0X22;

	while(1)
	{
	/************************数码管显示更新函数***********************/
		if(ADC_Flag)
		{
			ADC_Flag=0;
			Update_Tube();	//更新数码管的值
			write_595(20);	//因为每次发送的最后一位是存在于74HC595的移位寄存器中，并没有
		}					//转移到显示寄存器中，故要多发一位，把要显示的最后一位挤出。
		
	/********************通信过程，向上位机发送数据*********************/
		if(Uart_OK_Flag)//确认通信成功
		{			
			if(a)
			{	
			//	Uart1_PutChar(0X5E);//
				printf("^");
				a=0;//只执行一次，用于确认与上位机通信成功
				
//				printf("%X",A.Data16_bits);
				
				BEEP=1;//蜂鸣器间歇鸣叫两声，提示与上位机建立通信成功
				delay_ms(500);
				BEEP=0;
				delay_ms(500);	
				BEEP=1;
				delay_ms(500);	
				BEEP=0;	
			}		
		}
		
		if(Malloc_flag&&CRC_Flag2)//接受该次数据时，再清除上次数据
		{
			myfree(RX_BUF);	//释放内存,清除上次数据
			RX_BUF=(union FromUp *)mymalloc(3072);	//申请3K字节`																			
//				printf("ok");	//测试内存申请成功
			Malloc_flag=0;//清除标志位，避免重复申请内存
		}
			
		if(Send_ToUp_Flag==1)//确认键按下或者数字量变化，则发送实验台数据		
		{			//Analog_Flag|| Digital_Flag || OK_Flag || Cancle_Flag
			Print();//向上位机发送一组数据（实验台号，18位数字量，4位模拟量）	
			Send_ToUp_Flag=0;
//			Analog_Flag=0;
//			Digital_Flag=0;
			OK_Flag=0;
			Cancle_Flag=0;			
		}
		
		if(Send_Test_Flag)//用于测试数据接收成功
		{				
			Update_Dac();
			Display_Flag=1;//在中断中模拟量显示完成后清零
			Send_Test_Flag=0;	
		}
		
//			if(Fail_Flag)	//从上位机接收数据失败
//			{
//				Fail_Flag=0;
//				printf("@");//接收数据错误，要求重新发送
//			}		
	}
 }
