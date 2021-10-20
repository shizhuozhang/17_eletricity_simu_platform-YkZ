#include "math.h"

//计算绝对值
u16 abs_u16(u16 a, u16 b)
{
	long k;
	k = a- b;
	if(k > 0)
		return k;
	else
		return (-k);
}

//字符串转换到浮点数
float string_to_float(u8*str) 
{
	u8 i,j,k,negative = 0;
	#define s_temp str
	float result = 0,result_1 = 0;
	for(i = 0;i <10; i++)         
	{
		j = str[i];
		if(j == 0 || ((j<'0'||j>'9')&&(j!='.')&&(j!='-'))) break;  //0（SCLL码为0）为字符串结束标志
	}
	k = j =i; //字符串的个数（数值个数）
	for(i= 0;i <j;i++)    //查找小数点位置，结束后小数点位置为i+1
	{
		if(s_temp[i] == '.')
		break;
	}
	for(j = 0;j<i;j++)   //计算整数部分
	{
		if(s_temp[j] == '-')
		{
			negative = 1;
			continue;
		}
		result = result*10+(s_temp[j]-'0');
	}
	j++;         //j=i+1，即小数点的位置
	i = j;
	for(;j<k;j++)        //计算小数部分
	{
		if(s_temp[j]<'0'||s_temp[j]>'9') break;
		result_1 = result_1*10 + (s_temp[j] - '0');
	}
	for(j=0;j<(k-i);j++) result_1 *= 0.1;
	result += result_1;
	if(negative) result = -result;
	return result;
}

u16 f_to_int(float k)
{
	u16 t;
	t = (u16)k;
	return t;
}

