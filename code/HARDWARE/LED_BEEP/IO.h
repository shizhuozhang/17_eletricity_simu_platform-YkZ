#ifndef __IO_H
#define __IO_H	 
#include "sys.h"
#include "delay.h"

//#define GPIO_Remap_SWJ_JTAGDisable ((uint32_t)0x00300200) /*!< JTAG-DP Disabled and SW-DP Enabled*/ 
#define  LED1    PBout(3)	// PB3
#define  BEEP    PBout(4)	// PB4
#define  LED0    PDout(2)	// PD2	
#define  KEY_OK  PAin(0)	//PA0
#define  KEY_NO  PAin(1)	//PA1

void LED_Init(void);
void KEY_Init(void);
void EXTIx_Init(void);
		 				    
#endif



