/**
  * @attention
  *	ADC,模拟量采集函数配置
  * 使用ADC1通道10-15采集六路模拟电压,通过DMA直接将数据传到对应的整型数组中，经过均值
  * 保处理存在数组Voltage中，其值是0-4095的整数值。
  * 中断中以一定的频率监测电压的变化幅度
  * 电压变化幅度会影响ADC_flag的值，用于主函数中启动更新数码管显示数值
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
 #include "adc.h"
 	   
/*ADC1通道10到15，对应IO口PC0,PC1,PC2,PC3,PC4,PC5*/		   
////初始化ADC，并设置工作在独立模式，扫描模式（按设定的顺序依次循环转换6个通道）																   
void Adc_Init(void)
{
	RCC->APB2ENR|=1<<9;//时钟ADC1使能
	RCC->APB2ENR|=1<<4;//时钟GPIOC使能
	
	GPIOC->CRL&=0xff000000;//PC0-5配置成模拟输入模式
	
	RCC->APB2RSTR|=1<<9;	//adc1接口复位
	RCC->APB2RSTR&=~(1<<9);	//ADC1复位结束
	
	RCC->CFGR&=~(3<<14);//adc时钟12M
	RCC->CFGR|=2<<14;
	
	ADC1->CR1=0;
	ADC1->CR1|=0<<16;//独立模式
	ADC1->CR1|=1<<8;//扫描模式
	
	ADC1->CR2=0;
	ADC1->CR2|=1<<20;//使用外部触发
	ADC1->CR2|=7<<17;//软件触发
	ADC1->CR2|=1<<8;//使用DMA
	ADC1->CR2|=1<<1;//连续转换
	ADC1->CR2&=~(1<<11);//右对齐

	ADC1->SMPR2|=0x0003ffff;//adc通道采样时间均为239.5周期
				//每个通道转换时间为1/12*（1.5+239.5）=20us，足够长的采样时间，以保证转换精度
				
	ADC1->SQR1&=~(0xf<<20);//规则通道长度为6个
	ADC1->SQR1|=5<<20;

	
	ADC1->SQR3=0;			//ADC1后6个通道
	ADC1->SQR3|=CH10<<0;	//通道10最先转换
	ADC1->SQR3|=CH11<<5;	//通道11其次
	ADC1->SQR3|=CH12<<10;
	ADC1->SQR3|=CH13<<15;
	ADC1->SQR3|=CH14<<20;
	ADC1->SQR3|=CH15<<25;	//通道15最后转换
	
	ADC1->CR2|=1<<0;		//唤醒AD转换器
	ADC1->CR2|=1<<3;		//adc1复位校准
	while(ADC1->CR2&1<<3);	//等待复位结束
	ADC1->CR2|=1<<2;		//开启AD校准
	while(ADC1->CR2&1<<2);	//等待校准结束
	
	ADC1->CR2|=1<<22; 		//开始转换

}				  

u16 ADC_ori[6]={0};	//16位全局变量，保存ADC转换的最初整型数据，数据范围0～4095
u16 ADC_bac[6]={0}; //16位全局变量，备份ADC转换的数据，防止数据丢失
union Data_Trans Voltage[6]={0};	//16位全局变量，保存取过均值后的ADC数据
u16 Voltage_last[6]={0};//保存更新之前的ADC数据

//cndtr:数据数量；cpar:外设地址；cmar:存储地址
void DMA1_ADC_Init(u16 cndtr,u32 cpar,u32 cmar)
{	
	RCC->AHBENR|=1<<0;
	
	DMA1_Channel1->CNDTR=cndtr;	//数据传输数量
	DMA1_Channel1->CPAR=cpar;	//外设地址
	DMA1_Channel1->CMAR=cmar;	//存储器地址
	
	/*DMA控制寄存器配置*/
	DMA1_Channel1->CCR=0;
	DMA1_Channel1->CCR&=~(1<<14);//非储存器到储存器模式
//	DMA1_Channel1->CCR&=~(3<<14);
	DMA1_Channel1->CCR|=0<<12;//通道优先级最低
	DMA1_Channel1->CCR&=~(3<<10);
	DMA1_Channel1->CCR|=1<<10;//存储器数据宽度16
	DMA1_Channel1->CCR&=~(3<<8);
	DMA1_Channel1->CCR|=1<<8;//外设数据宽度16
	DMA1_Channel1->CCR|=2<<6;//存储器地址增量，外设地址非增量
	DMA1_Channel1->CCR|=1<<5;//循环模式
	DMA1_Channel1->CCR&=~(1<<4);//从外设读
	DMA1_Channel1->CCR&=~(7<<1);
	DMA1_Channel1->CCR|=1<<1;//开启传输完成中断，用来备份数据
	DMA1_Channel1->CCR|=1<<0;//通道开启
	MY_NVIC_Init(1,1,DMA1_Channel1_IRQn,2);
}

//DMA传输完成（整个数组全部传输完成）中断服务函数
void  DMA1_Channel1_IRQHandler()
{
	u8 i=0;
	if(DMA1->ISR&(1<<1))//判断状态标志位
	{	
		for(i=0;i<6;i++)
			ADC_bac[i]=ADC_ori[i];		
	}
	DMA1->IFCR|=1<<1;//清除标志位
}

//对8次测得的模拟电压做均值处理，得到更为准确的模拟电压值
void Get_Voltage(void)
{
	u32 temp_val[6]={0};
	u8 t=0,i=0;
	
	for(i=0;i<6;i++)	//更新数据前，先保存上次的值，用于比较
		Voltage_last[i]=Voltage[i].Data16b;
	
	for(t=0;t<8;t++)	//计算当前的电压值
	{
		for(i=0;i<6;i++)		
			temp_val[i]+=ADC_bac[i];			
		delay_ms(1);
	}
	for(i=0;i<6;i++)
	{
		Voltage[i].Data16b=temp_val[i]/8;//取八次平均值	
//		printf("analog  %d \r\n",Voltage[i]);//adc功能测试语句
	}		
//	printf("\r\n");	//adc功能测试语句
	
} 	

u8 Comp_Voltage(u8 Value)//在中断中使用该函数，	
{					//若中断频率设为2HZ，则比较的两个电压值时间上相错0.5s
	u8 i=0;
	u8 x=0;
	for(i=0;i<6;i++)
	{
		x|=(abs_u16(Voltage_last[i],Voltage[i].Data16b)>Value);//变化量大于100，则为1
												//100决定显示灵敏度，100/4096=2%
		x=x<<1;		//执行六次后，x最大可能是0X3F，对应六路电压均变化；最小是0
	}
	if(x)	//只要有一个变化，X值就大于1
		return 1;
	else
		return 0;
}







