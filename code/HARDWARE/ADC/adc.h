#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "math.h"
 
/*宏定义通道数用于设定各通道的转换顺序*/
#define CH10 10  //ADC1通道10
#define CH11 11  //ADC1通道11
#define CH12 12  //ADC1通道12
#define CH13 13  //ADC1通道13
#define CH14 14  //ADC1通道14
#define CH15 15  //ADC1通道15

__align(1) union Data_Trans
{
	u16 Data16b;
	u8  Data8b[2];
};

void Adc_Init(void);
void DMA1_ADC_Init(u16 cndtr,u32 cpar,u32 cmar);
void Get_Voltage(void);
u8 Comp_Voltage(u8 Value);

#endif 


