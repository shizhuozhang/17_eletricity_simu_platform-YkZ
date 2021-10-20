/**
  * @attention
  *	IO口引脚配置文件
  * 一方面作为输出口，用于LED和蜂鸣器的驱动，一方面作为输入口，接收外部中断。
  * LED1作为系统运行指示灯，LED0为状态指示灯，BEEP为状态提醒
  * 
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
#include "IO.h"
/* ---------------------------------------------------------------------------*/
	    
//普通IO初始化，初始化PD2和PB3,4,5为输出口.并使能这两个口的时钟	
void LED_Init(void)
{ 
     GPIO_InitTypeDef  GPIO_InitStructure;
        
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB,PD端口时钟
   
//	 GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); /*使能SWD 禁用JTAG*/	

	/******PD2,PB3,4用于LED和BEEP******/	
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 |GPIO_Pin_4;//PB3,4 端口配置
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
     GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB3,4,5
     GPIO_SetBits(GPIOB,GPIO_Pin_3 |GPIO_Pin_4);	 //PB3,4 输出高
            
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	    		 //PD.2 端口配置, 推挽输出
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		
     GPIO_Init(GPIOD, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
     GPIO_SetBits(GPIOD,GPIO_Pin_2);   						//PD2  输出高
					
}
 
//IO作为外部中断初始化，初始化PA0,PA1作为输入口.并使能这两个口的时钟
void KEY_Init(void)
{
     GPIO_InitTypeDef  GPIO_InitStructure;
        
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PA端口时钟	

	/******PA0,PA1用于按键控制******/	
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1;//PA0,A1 端口配置
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //按键一端接VCC，故设置成下拉输入，以接收上升沿中断   
     GPIO_Init(GPIOA, &GPIO_InitStructure);			//根据设定参数初始化GPIOA0       					
}

//外部中断配置
void EXTIx_Init(void)		//直接调用sys.c对外部中断的配置
{
	KEY_Init();
	Ex_NVIC_Config(GPIO_A,0,RTIR);//PA0，外部上升沿触发
	Ex_NVIC_Config(GPIO_A,1,RTIR);//PA1，外部上升沿触发
	MY_NVIC_Init(2,0,EXTI0_IRQn,2);//中断优先级设为低
	MY_NVIC_Init(2,0,EXTI1_IRQn,2);//中断优先级设为低
}

u8 OK_Flag=0;
u8 Cancle_Flag=0;
u8 Send_Test_Flag=0;

//外部中断0服务程序
void EXTI0_IRQHandler(void)
{
	delay_ms(8);	//消抖
	if(KEY_OK==1)	//对应确认按键 
	{
		OK_Flag=1;
		Cancle_Flag=1;
	}		 
	EXTI->PR=1<<0;  //清除LINE0上的中断标志位  
}
//外部中断1服务程序
void EXTI1_IRQHandler(void)
{			
	delay_ms(8);   //消抖			 
    if(KEY_NO==1)	//对应取消按键
	{
		Send_Test_Flag=1;
	}
 	EXTI->PR=1<<1;     //清除LINE1上的中断标志位  
}
