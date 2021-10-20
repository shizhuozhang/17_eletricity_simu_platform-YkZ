/**
  * @attention
  *	�����������������
  * ����ܲ��ù��������ģ���飬4λһ�飬��4�飬֧�־�̬��ʾ���������ݿ���ˢ����ʾ��
  * 4������ܵ���ʾ˳����Ƿ���ʾ�����޸ġ�
  * ����ܵĸ�����ͨ��ADC�ɼ�����ģ�����ı仯���ȿ��Ƶġ�����ʵ�ֹ�������TIM6�ж���
  * ���ɼ����ĵ�ѹ�ı仯��������Ԥ����������Ķ�ӦADC_flag�������������ж�flag��ֵ
  * ����������ܵ���ʾ���ݡ�
  ******************************************************************************
**/ 

/* Includes ------------------------------------------------------------------*/
#include "HC595.h" 

// ���������,ǰʮ���ǲ���С����0~9�ı��룬����ʮ���Ǵ�С����0~9�ı���,���һ���ǲ���
//��Tab[0]~Tab[9]��ʾ0��9��Tab[10]~Tab[19]��ʾ0.��9.��Tab[20]�رղ���ʾ
char Tab[] = {0xc0,0xcf,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
			  0x40,0x4f,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,0xff};

			  
	 /****************IO���ų�ʼ��****************/
void Init_595(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	GPIO_ResetBits(GPIOB,GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7);						 	
}
 
/****************һλ�����д�ַ�����****************/
//��ڲ���ΪҪ��ʾ��ʮ��������0-9��ʾ0-9��10-19��ʾ0.-9.��20����
void write_595(u8 a)
{
	u8 out_data,i,temp;
	
	out_data = Tab[a];
	
	LOAD = 1;            //����Ĵ���   
	delay_us(1);
	LOAD = 0; 	
	delay_us(1);
	
	for(i=0;i<8;i++)	//�������룬ÿ��ʱ����������һ��bit
	{
		SCL= 0;
		temp = out_data&0x80;//�Ӹߵ���ȡ��ÿһλbit
		if(temp == 0x80)
			SDK = 1;
		else   SDK = 0;
		out_data = out_data<<1;
		SCL= 1;
	}
}

/*�������ã�����λ�����а�λ�ֿ���Ȼ��浽�����б��ڿ��������ÿһλ����ʾ
���ڸ���������봮�����ݣ���ʮ�������ĸ�λ�Ǻ���ʾ�ģ�Ӧ��������ĺ���*/
u8 *Float_sepret(float data)
{
	static u8 ShowNum[4] = {0,0,0,0};
	int zhengshu;
	float xiaoshu;
	int num;
	
	zhengshu = (int)data;
	xiaoshu = data-zhengshu;
	
	if(zhengshu>=0 && zhengshu<10)           num = 1;
	if(zhengshu>=10 && zhengshu<100)         num = 2;
	if(zhengshu>=100 && zhengshu<1000)       num = 3;
	if(zhengshu>=1000 && zhengshu<10000)     num = 4;
	
	if(num == 1)    
	{
		ShowNum[0] = (int)(xiaoshu*1000)%10;	
		ShowNum[1] = (int)(xiaoshu*100)%10;
		ShowNum[2] = (int)(xiaoshu*10);      //��С���������λ
		ShowNum[3]   = 10+zhengshu;      //����С�����num
	}		
	if(num == 2)
	{
		ShowNum[0] = (int)(xiaoshu*100)%10;
		ShowNum[1] = (int)(xiaoshu*10);
		ShowNum[2]   = 10+zhengshu%10;         //����С����ĸ�λ
		ShowNum[3] = zhengshu/10;         //������С�����ʮλ	
	}
	if(num == 3)
	{
		ShowNum[0] = (int)(xiaoshu*10);
		ShowNum[1]   = 10+zhengshu%10;         //����С����ĸ�λ
		ShowNum[2] = (zhengshu/10)%10;         //������С�����ʮλ
		ShowNum[3] = zhengshu/100;         //������С����İ�λ
	}
	if(num == 4)
	{
		ShowNum[0]   = zhengshu%10;         //����λ,��ʱ�Ͳ���Ҫ��С������
		ShowNum[1] = (zhengshu/10)%10;         //������С�����ʮλ
		ShowNum[2] = (zhengshu/100)%10;         //������С����İ�λ
		ShowNum[3] = zhengshu/1000;         //������С�����ǧλ
	}
	return ShowNum;
}

/**** ����ܸ�����ʾ���ݺ���  ****/
/* ��Ҫ����ʵ�飬�е�ʵ��ʹ��ǰ��������ܣ��е�ʹ�ú��������еĶ�ʹ��,
��Ҫ����һ�������������ļ����������Ҫ��ʾ��*/
extern u16 Voltage[6];	

void Update_Tube(void)
{		
	u8 t=0,k=0;
	float temp;		//�������ʾ���м������ʱֵ
	u8 TubeNum[4][4];  //�������ʾ��ֵ 4������ܡ�ÿ���������ʾ4����
	u8 *TubeNumTemp;  //�洢��ʱ�������ֵ
	
	u8 TubeUseOrder[4] ={3,2,1,0};//����������Ǵ�������ģ��������ʾ�������һ�������
									//�������4 3 2 1�ǵ�1��4������ܵ���ʾ˳��
/**********************************��Ҫ�޸ĵĲ���*****************************************/
	float BaseRange[4] = {0.059,0.5,3,10};//�趨�ĵ�λ����Χ
	float BaseMin[4] = {1.001,0.8,0,5};//�趨�ĵ�λ����Сֵ		
	u8 TubeUseTrue[4] = {1,1,1,1};//������Ƿ�ʹ�ñ�־��1Ϊʹ��
	
	for(t=0;t<4;t++)	//ADC���Բɼ�6·������ֻ�õ�ǰ4·
	{					//tΪ��������ţ�kΪÿ���������ĵ�N��0��3������
		temp = Voltage[t]*BaseRange[t]/4096+BaseMin[t];//������������ʾ��ֵ
		TubeNumTemp = Float_sepret(temp);	//Ҫ��ʾ��ֵ�ֳ�һλһλ��
		for(k=0;k<4;k++)
		{
			TubeNum[t][k] = TubeNumTemp[k];	//����ʾ���鸳ֵ
		}
	}
	
	for(t=0;t<4;t++)	 	//�����������ʾ
	{
		k = TubeUseOrder[t];	//�ȶ�ȡ4���������ʾ��˳��
		if(TubeUseTrue[k] == 1)	//ȷ���Ƿ���Ҫ�����������ʾ
		{
			write_595(TubeNum[k][0]);
			write_595(TubeNum[k][1]);
			write_595(TubeNum[k][2]);
			write_595(TubeNum[k][3]);
		}
		else  //����������ܲ��ã���رղ���ʾ����д��ı����20
		{
			write_595(20);
			write_595(20);
			write_595(20);
			write_595(20);
		}
	}
}

