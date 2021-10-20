#include "usart.h"	

/***************REFER TO ALIENTEK*******************/  


//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

 
void uart_init(u32 bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
 	USART_DeInit(USART1);  //复位串口1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART1, ENABLE);                    //使能串口 
	
	MY_NVIC_Init(1,1,USART1_IRQn,2);
}

void Uart1_PutChar(u8 ch)
{
	USART_SendData(USART1, (u8) ch);//库函数
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
} 


extern u8 Position_bit[4][8];
extern u8 Position[4];
extern union Data_Trans Voltage[6];	
extern u8 OK_Flag;
extern u8 Cancle_Flag;

void Print(void)
{
	u8 i,j=0;
	static u8 Model=0;
	static u8 Model_last=0;
	
	Model_last=Model;//保存改变前的值，用于比较，防止出错
	
	switch(Position[3])		//转换工作模式
	{
		case 0:		Model=0;	break;
		case 1 :	Model=1;	break;
		case 2 :	Model=2;	break;
		case 4 :	Model=3;	break;
		case 8 :	Model=4;	break;
		case 16 :	Model=5;	break;
		case 32 :	Model=6;	break;
		case 64 :	Model=7;	break;
		case 128 :	Model=8;	break;
		default:	Model=0;    break;//其他情况是错误的，默认为0
	}	
	if(Model==0)
	{
		if((Model_last==1)|| (Model_last==0))
				Model=0;
		else 
			Model=Model_last;//模式切换过程中会出现开关全部断开的情况，即有错误的0值出现
	}		
	
	Single_bit();//先转换数字量

/*以字符串形式发送*/
	printf("A\r\n");//起始校验，保证数据头部正确 
	
	printf("A\r\n");//试验台号，不同试验台下程序时要修改
	
	for(i=0;i<2;i++)	//前16位数字量
	{
		for(j=0;j<8;j++)
		{
			printf("%d\r\n",Position_bit[i][j]);
		}
	}
	printf("%d\r\n",Position_bit[2][0]);//第17位数字量
	printf("%d\r\n",Model);//后8位以十进制发出
	
	for(i=0;i<4;i++)
		printf("%d\r\n",Voltage[i].Data16b);//发四路模拟量，整型数	
	
	printf("%d\r\n",OK_Flag);//后8位以十进制发出	
	printf("%d\r\n",Cancle_Flag);//后8位以十进制发出
	
	printf("*\r\n");//用于结尾校验
	
} 

u8 Uart_OK_Flag=0;
u8 Output_Flag=0;

union FromUp *RX_BUF;//定义数据缓冲区地址指针,类型为共用体

//u32 *Data_Address;
//u8 Out_Num=0;
u8 USART_RX_STA=0;
u8 CRC_Flag2=0;
u8 Send_ToUp_Flag=0;

extern u8 Malloc_flag;

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	static u8 i=0;
	static u16 j=0;
	u16 Res=0;	
	static u8 CRC_Flag1=0;
	static u8 Send_CRC_Flag=0;
//	static u8 Data_Num=0;
//	static u16 RX_LEN[4]={0};//最多显示4个变量，每个变量对应一个数据长度
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //读取USARTx->SR能避免莫名其妙的错误
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		
		if(Res=='^')			//ascII是5E
			Uart_OK_Flag=1;		//确认通信成功
		
		if(Send_CRC_Flag&&(Res=='M'))	//M的ASCII码
		{
			Send_ToUp_Flag=1;	//接收到SM，校验成功，从下一个数据开始存储
			Send_CRC_Flag=0;
		}
		
		if(Res=='S')		//先进行校验,S的ASCII码
			Send_CRC_Flag=1;		

		//定义协议开头加入字符TD用于校验，结尾加入0x0a和0x0x0d即是回车换行,代表通信结束，避免接受数据冲突
		if((USART_RX_STA&0x80)==0)//接收未完成
		{
			if(USART_RX_STA&0x40)//接收到了0x0d
			{
				if(Res!=0x0a)
					USART_RX_STA=0;//接收错误,重新开始
				else 					
				{
					USART_RX_STA|=0x80;	//接收完成了 				
					CRC_Flag1=0;
					CRC_Flag2=0;//清除标志位，便于下一次的接收
					Malloc_flag=1;//内存申请标志位置位，便于下次数据接收时申请内存
					
					i=0;
					j=0;
				}
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)
					USART_RX_STA|=0x40;
				else			//若没有接收到0X0D,0X0A，则保存按协议定义接收数据
				{
					if(CRC_Flag2)
					{		
						if(Res==0x7C)//数据之间用|隔开,|的ASCII码
						{
							j++;
							i=0;
						}						
						else 
						{  
							if(i==0)//内存存放低八位在前，高八位在后
								(RX_BUF+j)->Data8_bits[1]=Res;//
							else if(i==1)
								(RX_BUF+j)->Data8_bits[0]=Res;//
							
							i++;//i是bit0,bit1的标识
						}				
					}
						
					if(CRC_Flag1&&(Res=='D'))	//D的ASCII码
					{
						CRC_Flag2=1;	//接收到TD，校验成功，从下一个数据开始存储
						//CRC_Flag1=0;
					}
					
					if(Res=='T')		//先进行校验,T的ASCII码
						CRC_Flag1=1;
		
				}		 
			}
		}
	}
} 


void Update_Dac(void)
{
	u8 i;	
	u16 datavalue[4]={0};	
	
	RX_BUF->Data16_bits=200;

	for(i=0;i<RX_BUF->Data16_bits;i++)//用于调试
	{
		datavalue[0]+=5;
		(RX_BUF+1+i)->Data16_bits=datavalue[0];
	}	
	for(i=0;i<RX_BUF->Data16_bits;i++)
	{
		datavalue[1]+=5;
		(RX_BUF+1+RX_BUF->Data16_bits+i)->Data16_bits=datavalue[1];
	}	
	for(i=0;i<RX_BUF->Data16_bits;i++)
	{
		datavalue[2]+=5;
		(RX_BUF+1+2*(RX_BUF->Data16_bits)+i)->Data16_bits=datavalue[2];
	}		
	for(i=0;i<RX_BUF->Data16_bits;i++)	
	{
		datavalue[3]+=5;
		(RX_BUF+1+3*(RX_BUF->Data16_bits)+i)->Data16_bits=datavalue[3];
	}
		
	for(i=0;i<RX_BUF->Data16_bits;i++)//用于调试
		printf("one:%x\r\n",(RX_BUF+4+i)->Data16_bits);
//	
//	for(i=0;i<(RX_BUF+1)->Data16_bits;i++)
//		printf("two:%x\r\n",(RX_BUF+4+RX_BUF->Data16_bits+i)->Data16_bits);
//	
//	for(i=0;i<(RX_BUF+2)->Data16_bits;i++)
//		printf("thr:%x\r\n",(RX_BUF+4+RX_BUF->Data16_bits+(RX_BUF+1)->Data16_bits+i)->Data16_bits);	
//		
//	for(i=0;i<(RX_BUF+3)->Data16_bits;i++)	
//		printf("for:%x\r\n",(RX_BUF+4+RX_BUF->Data16_bits+(RX_BUF+1)->Data16_bits+(RX_BUF+2)->Data16_bits+i)->Data16_bits);
		
//	USART_RX_STA=0x80;///发送后,清除接收完成标志，便于再次接受	
}


/*下位机向上位机发送数据以字节形式发送	
//	Uart1_PutChar(0X41);//A
//	Uart1_PutChar(0X0D);//回车
//	Uart1_PutChar(0X0A);//换行
//	
//	Uart1_PutChar(0X41);//A
//	Uart1_PutChar(0X0D);//回车
//	Uart1_PutChar(0X0A);//换行
//	
//	for(i=0;i<2;i++)	//前16位数字量
//	{
//		for(j=0;j<8;j++)
//		{
//			Uart1_PutChar(Position_bit[i][j]);//前16个数字量
//			Uart1_PutChar(0X0D);//回车
//			Uart1_PutChar(0X0A);//换行
//		}
//	}	
//	
//	Uart1_PutChar(Position_bit[2][0]);//第17位数字量
//	Uart1_PutChar(0X0D);//回车
//	Uart1_PutChar(0X0A);//换行
//	
//	Uart1_PutChar(Model);//模式
//	Uart1_PutChar(0X0D);//回车
//	Uart1_PutChar(0X0A);//换行
//	
//	for(i=0;i<4;i++)
//	{
//		Uart1_PutChar(Voltage[i].Data8b[0]);//低位字节		
//		Uart1_PutChar(Voltage[i].Data8b[1]);//高位字节
//		Uart1_PutChar(0X0D);//回车
//		Uart1_PutChar(0X0A);//换行
//	
//	}

//	Uart1_PutChar(OK_Flag);//两位变量
//	Uart1_PutChar(0X0D);//回车
//	Uart1_PutChar(0X0A);//换行
//	
//	Uart1_PutChar(Cancle_Flag);//两位变量
//	Uart1_PutChar(0X0D);//回车
//	Uart1_PutChar(0X0A);//换行
//	
//	Uart1_PutChar(0X2A);//"*"结尾校验
//	Uart1_PutChar(0X0D);//回车
//	Uart1_PutChar(0X0A);//换行
*/	
