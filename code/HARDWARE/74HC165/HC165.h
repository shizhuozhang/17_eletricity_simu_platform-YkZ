#ifndef __HC165_H
#define __HC165_H	 
#include "sys.h"
#include "delay.h"
#include "usart.h"

#define CLK    PBout(8) //时钟引脚，上升沿有效
#define SHLD   PBout(9) //更新数据允许引脚
#define  QHOUT PBin(10) //串行数据输出引脚，
						//对单片机来说，引脚应配置成输入模式，接受165输出的数据
void Init_HC165(void);
void Read_HC165(void);
void sep01(u8 data, u8 len,u8 *sepData);
u8 Comp_Digital(void);
void Single_bit(void);


#endif

