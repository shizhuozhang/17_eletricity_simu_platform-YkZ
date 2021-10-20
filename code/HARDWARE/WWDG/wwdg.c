#include "wwdg.h"
#include "IO.h"


//����WWDG������������ֵ,Ĭ��Ϊ���.
u8 WWDG_CNT=0x7f;

void WWDG_Init(u8 tr,u8 wr,u32 fprer)
{ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);  //   WWDGʱ��ʹ��

	WWDG_SetPrescaler(fprer);////����IWDGԤ��Ƶֵ

	WWDG_SetWindowValue(wr);//���ô���ֵ

	WWDG_Enable(tr);	 //ʹ�ܿ��Ź� ,	���� counter .                  

	WWDG_ClearFlag();

	WWDG_NVIC_Init();//��ʼ�����ڿ��Ź� NVIC

	WWDG_EnableIT(); //�������ڿ��Ź��ж�
} 

//������WWDG��������ֵ
void WWDG_Set_Counter(u8 cnt)
{
    WWDG_Enable(cnt);	 
}



//���ڿ��Ź��жϷ������
void WWDG_NVIC_Init()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;    //WWDG�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //��ռ0�������ȼ�0����2	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	 //��ռ0�������ȼ�0����2	
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE; 
	NVIC_Init(&NVIC_InitStructure);//NVIC��ʼ��
}

void WWDG_IRQHandler(void)
	{
	// Update WWDG counter
	WWDG_SetCounter(0x7F);	  //�������˾��,���ڿ��Ź���������λ
	// Clear EWI flag */
	WWDG_ClearFlag();	  //�����ǰ�����жϱ�־λ
		
	LED0=!LED0;
	}

