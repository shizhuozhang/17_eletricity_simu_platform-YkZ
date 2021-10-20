#ifndef __TLC5615__H
#define __TLC5615__H
#include "sys.h"
#include "delay.h"
#include "usart.h"

#define CS1  PAout(2)
#define CS2 PAout(3)
#define CS3  PCout(8)
#define CS4  PCout(9)

void TLC5615_Init(void);
void Write_5615ZERO(u16 i);
void Write_5615YI(u16 i);
void Write_5615ER(u16 i);
void Write_5615SAN(u16 i);
void Write_5615SI(u16 i);

#endif
