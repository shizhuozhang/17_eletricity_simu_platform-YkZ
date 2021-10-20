/**
  * @attention
  *	级联数码管驱动程序。
  * 数码管采用共阳数码管模块组，4位一组，共4组，支持静态显示，更新数据可以刷新显示。
  * 4组数码管的显示顺序和是否显示可以修改。
  * 数码管的更新是通过ADC采集到的模拟量的变化幅度控制的。具体实现过程是在TIM6中断里
  * 检测采集到的电压的变化，若超出预定幅度则更改对应ADC_flag，在主函数里判断flag的值
  * 来更新数码管的显示数据。
  ******************************************************************************
**/ 

/* Includes ------------------------------------------------------------------*/
#include "HC595.h" 

// 共阳数码管,前十个是不带小数点0~9的编码，接着十个是带小数点0~9的编码,最后一个是不亮
//即Tab[0]~Tab[9]显示0～9；Tab[10]~Tab[19]显示0.～9.；Tab[20]关闭不显示
char Tab[] = {0xc0,0xcf,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
			  0x40,0x4f,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,0xff};

			  
	 /****************IO引脚初始化****************/
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
 
/****************一位数码管写字符函数****************/
//入口参数为要显示的十进制数，0-9显示0-9；10-19显示0.-9.；20不亮
void write_595(u8 a)
{
	u8 out_data,i,temp;
	
	out_data = Tab[a];
	
	LOAD = 1;            //先清寄存器   
	delay_us(1);
	LOAD = 0; 	
	delay_us(1);
	
	for(i=0;i<8;i++)	//串行输入，每个时钟周期输入一个bit
	{
		SCL= 0;
		temp = out_data&0x80;//从高到低取出每一位bit
		if(temp == 0x80)
			SDK = 1;
		else   SDK = 0;
		out_data = out_data<<1;
		SCL= 1;
	}
}

/*函数作用：将四位数进行按位分开，然后存到数组中便于控制数码管每一位的显示
由于给数码管送入串行数据，故十进制数的高位是后显示的，应放在数组的后面*/
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
		ShowNum[2] = (int)(xiaoshu*10);      //即小数部分最高位
		ShowNum[3]   = 10+zhengshu;      //即带小数点的num
	}		
	if(num == 2)
	{
		ShowNum[0] = (int)(xiaoshu*100)%10;
		ShowNum[1] = (int)(xiaoshu*10);
		ShowNum[2]   = 10+zhengshu%10;         //即带小数点的个位
		ShowNum[3] = zhengshu/10;         //即不带小数点的十位	
	}
	if(num == 3)
	{
		ShowNum[0] = (int)(xiaoshu*10);
		ShowNum[1]   = 10+zhengshu%10;         //即带小数点的个位
		ShowNum[2] = (zhengshu/10)%10;         //即不带小数点的十位
		ShowNum[3] = zhengshu/100;         //即不带小数点的百位
	}
	if(num == 4)
	{
		ShowNum[0]   = zhengshu%10;         //即个位,此时就不需要带小数点了
		ShowNum[1] = (zhengshu/10)%10;         //即不带小数点的十位
		ShowNum[2] = (zhengshu/100)%10;         //即不带小数点的百位
		ShowNum[3] = zhengshu/1000;         //即不带小数点的千位
	}
	return ShowNum;
}

/**** 数码管更新显示数据函数  ****/
/* 需要区分实验，有的实验使用前两个数码管，有的使用后两个，有的都使用,
故要定义一个变量来管理哪几个数码管是要显示的*/
extern u16 Voltage[6];	

void Update_Tube(void)
{		
	u8 t=0,k=0;
	float temp;		//数码管显示的中间计算临时值
	u8 TubeNum[4][4];  //数码管显示的值 4组数码管×每组数码管显示4个数
	u8 *TubeNumTemp;  //存储临时的数码管值
	
	u8 TubeUseOrder[4] ={3,2,1,0};//由于数码管是串行输入的，因此先显示的是最后一个数码管
									//即这里的4 3 2 1是第1到4个数码管的显示顺序
/**********************************需要修改的参数*****************************************/
	float BaseRange[4] = {0.059,0.5,3,10};//设定的电位器范围
	float BaseMin[4] = {1.001,0.8,0,5};//设定的电位器最小值		
	u8 TubeUseTrue[4] = {1,1,1,1};//数码管是否使用标志，1为使用
	
	for(t=0;t<4;t++)	//ADC可以采集6路，这里只用到前4路
	{					//t为数码管组标号，k为每组数码管里的第N（0～3）个数
		temp = Voltage[t]*BaseRange[t]/4096+BaseMin[t];//按比例计算显示的值
		TubeNumTemp = Float_sepret(temp);	//要显示的值分成一位一位的
		for(k=0;k<4;k++)
		{
			TubeNum[t][k] = TubeNumTemp[k];	//给显示数组赋值
		}
	}
	
	for(t=0;t<4;t++)	 	//驱动数码管显示
	{
		k = TubeUseOrder[t];	//先读取4组数码管显示的顺序
		if(TubeUseTrue[k] == 1)	//确定是否需要该组数码管显示
		{
			write_595(TubeNum[k][0]);
			write_595(TubeNum[k][1]);
			write_595(TubeNum[k][2]);
			write_595(TubeNum[k][3]);
		}
		else  //如果这个数码管不用，则关闭不显示，即写入的编号是20
		{
			write_595(20);
			write_595(20);
			write_595(20);
			write_595(20);
		}
	}
}

