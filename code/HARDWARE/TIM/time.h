#ifndef _TIM_H_
#define _TIM_H_
#include "sys.h"
#include "IO.h"
#include "usart.h"
#include "adc.h"
#include "HC165.h"
#include "tlc5615.h"
#include "HC595.h" 

void TIM7_Init(u16 arr,u16 psc);	
void TIM6_Init(u16 arr,u16 psc);

#endif
