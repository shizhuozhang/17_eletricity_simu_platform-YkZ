#include "usart.h"	

/***************REFER TO ALIENTEK*******************/  


//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

 
void uart_init(u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
 	USART_DeInit(USART1);  //��λ����1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure); //��ʼ������
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
	
	MY_NVIC_Init(1,1,USART1_IRQn,2);
}

void Uart1_PutChar(u8 ch)
{
	USART_SendData(USART1, (u8) ch);//�⺯��
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
	
	Model_last=Model;//����ı�ǰ��ֵ�����ڱȽϣ���ֹ����
	
	switch(Position[3])		//ת������ģʽ
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
		default:	Model=0;    break;//��������Ǵ���ģ�Ĭ��Ϊ0
	}	
	if(Model==0)
	{
		if((Model_last==1)|| (Model_last==0))
				Model=0;
		else 
			Model=Model_last;//ģʽ�л������л���ֿ���ȫ���Ͽ�����������д����0ֵ����
	}		
	
	Single_bit();//��ת��������

/*���ַ�����ʽ����*/
	printf("A\r\n");//��ʼУ�飬��֤����ͷ����ȷ 
	
	printf("A\r\n");//����̨�ţ���ͬ����̨�³���ʱҪ�޸�
	
	for(i=0;i<2;i++)	//ǰ16λ������
	{
		for(j=0;j<8;j++)
		{
			printf("%d\r\n",Position_bit[i][j]);
		}
	}
	printf("%d\r\n",Position_bit[2][0]);//��17λ������
	printf("%d\r\n",Model);//��8λ��ʮ���Ʒ���
	
	for(i=0;i<4;i++)
		printf("%d\r\n",Voltage[i].Data16b);//����·ģ������������	
	
	printf("%d\r\n",OK_Flag);//��8λ��ʮ���Ʒ���	
	printf("%d\r\n",Cancle_Flag);//��8λ��ʮ���Ʒ���
	
	printf("*\r\n");//���ڽ�βУ��
	
} 

u8 Uart_OK_Flag=0;
u8 Output_Flag=0;

union FromUp *RX_BUF;//�������ݻ�������ַָ��,����Ϊ������

//u32 *Data_Address;
//u8 Out_Num=0;
u8 USART_RX_STA=0;
u8 CRC_Flag2=0;
u8 Send_ToUp_Flag=0;

extern u8 Malloc_flag;

void USART1_IRQHandler(void)                	//����1�жϷ������
{
	static u8 i=0;
	static u16 j=0;
	u16 Res=0;	
	static u8 CRC_Flag1=0;
	static u8 Send_CRC_Flag=0;
//	static u8 Data_Num=0;
//	static u16 RX_LEN[4]={0};//�����ʾ4��������ÿ��������Ӧһ�����ݳ���
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //��ȡUSARTx->SR�ܱ���Ī������Ĵ���
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		
		if(Res=='^')			//ascII��5E
			Uart_OK_Flag=1;		//ȷ��ͨ�ųɹ�
		
		if(Send_CRC_Flag&&(Res=='M'))	//M��ASCII��
		{
			Send_ToUp_Flag=1;	//���յ�SM��У��ɹ�������һ�����ݿ�ʼ�洢
			Send_CRC_Flag=0;
		}
		
		if(Res=='S')		//�Ƚ���У��,S��ASCII��
			Send_CRC_Flag=1;		

		//����Э�鿪ͷ�����ַ�TD����У�飬��β����0x0a��0x0x0d���ǻس�����,����ͨ�Ž���������������ݳ�ͻ
		if((USART_RX_STA&0x80)==0)//����δ���
		{
			if(USART_RX_STA&0x40)//���յ���0x0d
			{
				if(Res!=0x0a)
					USART_RX_STA=0;//���մ���,���¿�ʼ
				else 					
				{
					USART_RX_STA|=0x80;	//��������� 				
					CRC_Flag1=0;
					CRC_Flag2=0;//�����־λ��������һ�εĽ���
					Malloc_flag=1;//�ڴ������־λ��λ�������´����ݽ���ʱ�����ڴ�
					
					i=0;
					j=0;
				}
			}
			else //��û�յ�0X0D
			{	
				if(Res==0x0d)
					USART_RX_STA|=0x40;
				else			//��û�н��յ�0X0D,0X0A���򱣴水Э�鶨���������
				{
					if(CRC_Flag2)
					{		
						if(Res==0x7C)//����֮����|����,|��ASCII��
						{
							j++;
							i=0;
						}						
						else 
						{  
							if(i==0)//�ڴ��ŵͰ�λ��ǰ���߰�λ�ں�
								(RX_BUF+j)->Data8_bits[1]=Res;//
							else if(i==1)
								(RX_BUF+j)->Data8_bits[0]=Res;//
							
							i++;//i��bit0,bit1�ı�ʶ
						}				
					}
						
					if(CRC_Flag1&&(Res=='D'))	//D��ASCII��
					{
						CRC_Flag2=1;	//���յ�TD��У��ɹ�������һ�����ݿ�ʼ�洢
						//CRC_Flag1=0;
					}
					
					if(Res=='T')		//�Ƚ���У��,T��ASCII��
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

	for(i=0;i<RX_BUF->Data16_bits;i++)//���ڵ���
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
		
	for(i=0;i<RX_BUF->Data16_bits;i++)//���ڵ���
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
		
//	USART_RX_STA=0x80;///���ͺ�,���������ɱ�־�������ٴν���	
}


/*��λ������λ�������������ֽ���ʽ����	
//	Uart1_PutChar(0X41);//A
//	Uart1_PutChar(0X0D);//�س�
//	Uart1_PutChar(0X0A);//����
//	
//	Uart1_PutChar(0X41);//A
//	Uart1_PutChar(0X0D);//�س�
//	Uart1_PutChar(0X0A);//����
//	
//	for(i=0;i<2;i++)	//ǰ16λ������
//	{
//		for(j=0;j<8;j++)
//		{
//			Uart1_PutChar(Position_bit[i][j]);//ǰ16��������
//			Uart1_PutChar(0X0D);//�س�
//			Uart1_PutChar(0X0A);//����
//		}
//	}	
//	
//	Uart1_PutChar(Position_bit[2][0]);//��17λ������
//	Uart1_PutChar(0X0D);//�س�
//	Uart1_PutChar(0X0A);//����
//	
//	Uart1_PutChar(Model);//ģʽ
//	Uart1_PutChar(0X0D);//�س�
//	Uart1_PutChar(0X0A);//����
//	
//	for(i=0;i<4;i++)
//	{
//		Uart1_PutChar(Voltage[i].Data8b[0]);//��λ�ֽ�		
//		Uart1_PutChar(Voltage[i].Data8b[1]);//��λ�ֽ�
//		Uart1_PutChar(0X0D);//�س�
//		Uart1_PutChar(0X0A);//����
//	
//	}

//	Uart1_PutChar(OK_Flag);//��λ����
//	Uart1_PutChar(0X0D);//�س�
//	Uart1_PutChar(0X0A);//����
//	
//	Uart1_PutChar(Cancle_Flag);//��λ����
//	Uart1_PutChar(0X0D);//�س�
//	Uart1_PutChar(0X0A);//����
//	
//	Uart1_PutChar(0X2A);//"*"��βУ��
//	Uart1_PutChar(0X0D);//�س�
//	Uart1_PutChar(0X0A);//����
*/	
