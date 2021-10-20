#ifndef __HC595_H
#define __HC595_H

#include "sys.h"
#include "adc.h"

//四只数码管采用串级连接，只需要一组数据传输引脚
#define LOAD  PBout(5) //数据锁存引脚 
#define SCL   PBout(6) //数据传输时钟引脚   
#define SDK   PBout(7) //数据引脚

void Init_595(void);
void write_595(u8 a);
u8 *Float_sepret(float data);
void Update_Tube(void);

#endif
