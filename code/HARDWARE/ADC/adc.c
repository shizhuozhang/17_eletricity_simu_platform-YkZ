/**
  * @attention
  *	ADC,ģ�����ɼ���������
  * ʹ��ADC1ͨ��10-15�ɼ���·ģ���ѹ,ͨ��DMAֱ�ӽ����ݴ�����Ӧ�����������У�������ֵ
  * �������������Voltage�У���ֵ��0-4095������ֵ��
  * �ж�����һ����Ƶ�ʼ���ѹ�ı仯����
  * ��ѹ�仯���Ȼ�Ӱ��ADC_flag��ֵ�����������������������������ʾ��ֵ
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
 #include "adc.h"
 	   
/*ADC1ͨ��10��15����ӦIO��PC0,PC1,PC2,PC3,PC4,PC5*/		   
////��ʼ��ADC�������ù����ڶ���ģʽ��ɨ��ģʽ�����趨��˳������ѭ��ת��6��ͨ����																   
void Adc_Init(void)
{
	RCC->APB2ENR|=1<<9;//ʱ��ADC1ʹ��
	RCC->APB2ENR|=1<<4;//ʱ��GPIOCʹ��
	
	GPIOC->CRL&=0xff000000;//PC0-5���ó�ģ������ģʽ
	
	RCC->APB2RSTR|=1<<9;	//adc1�ӿڸ�λ
	RCC->APB2RSTR&=~(1<<9);	//ADC1��λ����
	
	RCC->CFGR&=~(3<<14);//adcʱ��12M
	RCC->CFGR|=2<<14;
	
	ADC1->CR1=0;
	ADC1->CR1|=0<<16;//����ģʽ
	ADC1->CR1|=1<<8;//ɨ��ģʽ
	
	ADC1->CR2=0;
	ADC1->CR2|=1<<20;//ʹ���ⲿ����
	ADC1->CR2|=7<<17;//�������
	ADC1->CR2|=1<<8;//ʹ��DMA
	ADC1->CR2|=1<<1;//����ת��
	ADC1->CR2&=~(1<<11);//�Ҷ���

	ADC1->SMPR2|=0x0003ffff;//adcͨ������ʱ���Ϊ239.5����
				//ÿ��ͨ��ת��ʱ��Ϊ1/12*��1.5+239.5��=20us���㹻���Ĳ���ʱ�䣬�Ա�֤ת������
				
	ADC1->SQR1&=~(0xf<<20);//����ͨ������Ϊ6��
	ADC1->SQR1|=5<<20;

	
	ADC1->SQR3=0;			//ADC1��6��ͨ��
	ADC1->SQR3|=CH10<<0;	//ͨ��10����ת��
	ADC1->SQR3|=CH11<<5;	//ͨ��11���
	ADC1->SQR3|=CH12<<10;
	ADC1->SQR3|=CH13<<15;
	ADC1->SQR3|=CH14<<20;
	ADC1->SQR3|=CH15<<25;	//ͨ��15���ת��
	
	ADC1->CR2|=1<<0;		//����ADת����
	ADC1->CR2|=1<<3;		//adc1��λУ׼
	while(ADC1->CR2&1<<3);	//�ȴ���λ����
	ADC1->CR2|=1<<2;		//����ADУ׼
	while(ADC1->CR2&1<<2);	//�ȴ�У׼����
	
	ADC1->CR2|=1<<22; 		//��ʼת��

}				  

u16 ADC_ori[6]={0};	//16λȫ�ֱ���������ADCת��������������ݣ����ݷ�Χ0��4095
u16 ADC_bac[6]={0}; //16λȫ�ֱ���������ADCת�������ݣ���ֹ���ݶ�ʧ
union Data_Trans Voltage[6]={0};	//16λȫ�ֱ���������ȡ����ֵ���ADC����
u16 Voltage_last[6]={0};//�������֮ǰ��ADC����

//cndtr:����������cpar:�����ַ��cmar:�洢��ַ
void DMA1_ADC_Init(u16 cndtr,u32 cpar,u32 cmar)
{	
	RCC->AHBENR|=1<<0;
	
	DMA1_Channel1->CNDTR=cndtr;	//���ݴ�������
	DMA1_Channel1->CPAR=cpar;	//�����ַ
	DMA1_Channel1->CMAR=cmar;	//�洢����ַ
	
	/*DMA���ƼĴ�������*/
	DMA1_Channel1->CCR=0;
	DMA1_Channel1->CCR&=~(1<<14);//�Ǵ�������������ģʽ
//	DMA1_Channel1->CCR&=~(3<<14);
	DMA1_Channel1->CCR|=0<<12;//ͨ�����ȼ����
	DMA1_Channel1->CCR&=~(3<<10);
	DMA1_Channel1->CCR|=1<<10;//�洢�����ݿ��16
	DMA1_Channel1->CCR&=~(3<<8);
	DMA1_Channel1->CCR|=1<<8;//�������ݿ��16
	DMA1_Channel1->CCR|=2<<6;//�洢����ַ�����������ַ������
	DMA1_Channel1->CCR|=1<<5;//ѭ��ģʽ
	DMA1_Channel1->CCR&=~(1<<4);//�������
	DMA1_Channel1->CCR&=~(7<<1);
	DMA1_Channel1->CCR|=1<<1;//������������жϣ�������������
	DMA1_Channel1->CCR|=1<<0;//ͨ������
	MY_NVIC_Init(1,1,DMA1_Channel1_IRQn,2);
}

//DMA������ɣ���������ȫ��������ɣ��жϷ�����
void  DMA1_Channel1_IRQHandler()
{
	u8 i=0;
	if(DMA1->ISR&(1<<1))//�ж�״̬��־λ
	{	
		for(i=0;i<6;i++)
			ADC_bac[i]=ADC_ori[i];		
	}
	DMA1->IFCR|=1<<1;//�����־λ
}

//��8�β�õ�ģ���ѹ����ֵ�����õ���Ϊ׼ȷ��ģ���ѹֵ
void Get_Voltage(void)
{
	u32 temp_val[6]={0};
	u8 t=0,i=0;
	
	for(i=0;i<6;i++)	//��������ǰ���ȱ����ϴε�ֵ�����ڱȽ�
		Voltage_last[i]=Voltage[i].Data16b;
	
	for(t=0;t<8;t++)	//���㵱ǰ�ĵ�ѹֵ
	{
		for(i=0;i<6;i++)		
			temp_val[i]+=ADC_bac[i];			
		delay_ms(1);
	}
	for(i=0;i<6;i++)
	{
		Voltage[i].Data16b=temp_val[i]/8;//ȡ�˴�ƽ��ֵ	
//		printf("analog  %d \r\n",Voltage[i]);//adc���ܲ������
	}		
//	printf("\r\n");	//adc���ܲ������
	
} 	

u8 Comp_Voltage(u8 Value)//���ж���ʹ�øú�����	
{					//���ж�Ƶ����Ϊ2HZ����Ƚϵ�������ѹֵʱ�������0.5s
	u8 i=0;
	u8 x=0;
	for(i=0;i<6;i++)
	{
		x|=(abs_u16(Voltage_last[i],Voltage[i].Data16b)>Value);//�仯������100����Ϊ1
												//100������ʾ�����ȣ�100/4096=2%
		x=x<<1;		//ִ�����κ�x��������0X3F����Ӧ��·��ѹ���仯����С��0
	}
	if(x)	//ֻҪ��һ���仯��Xֵ�ʹ���1
		return 1;
	else
		return 0;
}







