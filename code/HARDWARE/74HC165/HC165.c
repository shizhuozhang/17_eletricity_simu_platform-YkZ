/**
  * @attention
  *	8位并行转串行数据移位寄存器74HC165芯片底层驱动
  * 使用4片HC165级联可同时采集32位数字信号
  * 
  ******************************************************************************
**/  

/* Includes ------------------------------------------------------------------*/
#include "HC165.h"

void Init_HC165(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 |GPIO_Pin_9;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);						 
	GPIO_SetBits(GPIOB,GPIO_Pin_9);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //下拉输入	
	GPIO_Init(GPIOB, &GPIO_InitStructure);		
}

u8 Position[4]={0};	//4组*8=32个开关量数据数组
u8 Position_last[4]={0};//保存更新之前的开关变量

void Read_HC165(void)	//执行一次该程序大约需要35ms，故应注意读数的频率不能超过28HZ
{
	u8 j=0,i=0;
	
	for(i=0;i<4;i++)
		Position_last[i]=Position[i];//先保存上次的值，用于比较
	
/**********************下面的驱动程序参考74HC165芯片手册中的工作时序图*******************/		
	SHLD=1;	//初始化74HC165芯片
	CLK=0;
	delay_ms(1);
	
	SHLD=0;			//将并行数据送入芯片
	delay_ms(1);	
	SHLD=1;			//锁存数据
	delay_ms(1);	
	
	for(j=0;j<4;j++)	//j值为要读取的HC165的级联片数
	{
		/*单片HC165读数程序*/
		for(i=0;i<8;i++)
		{			//数据格式HGFEDCBA，其中H为最高位，A为最低位，串行移位时，先移出H然后是G～A
			Position[j]<<=1;//先出的位向高位移动
			if(QHOUT==1)
			{	
				Position[j]|= 0x0001; //QHOUT  8次后即得到了存储的数据 例如10101011
			}
			CLK=0;			//每一个上升沿传输一个数据
			delay_us(500);	
			CLK=1;
			delay_us(500);		//单个信号的采集频率为1KHz,
		}
		CLK=0;	
//		printf("\r\n");				//165测试语句
//		printf("digial  %x\r\n",Position[j]);//165测试程序语句	
	}	
		
}

u8 Comp_Digital(void)//更新频率2HZ
{
	u8 i=0;
	u8 y=0;
	for(i=0;i<4;i++)
	{
		if(Position_last[i]!=Position[i])//变化量大于50，则为1（50决定显示灵敏度）
			y+=1;		//执行四次后，y最大可能是4，即四组开关量全改变；最小是0
	}
	if(y)	//只要有一个开关量变化，y值就大于1
		return 1;
	else
		return 0;
}

/*在输出开关量时，需要把数据转换成0/1的数组形式供上位机使用*/
u8 Position_bit[4][8]={0};	

/************************取bit位函数************************/
 //data 要分开的数据，默认u8类型
//len为要分成几位,最大默认8位
//将分成的bit位存到数组SepData中
void sep01(u8 data, u8 len,u8 *sepData)
{
	u8 t;
	for(t=0;t<(len);t++)
	{
		sepData[t] = data&0x01;      
		data = data>>1;
	}
}
void Single_bit(void)
{
	u8 i=0;
	for(i=0;i<3;i++)
	{		
		sep01(Position[i], 8, Position_bit[i]);
	}
}


