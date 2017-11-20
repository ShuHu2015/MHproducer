#include "stdafx.h"
#include "afxmt.h"
#include "define.h"
#include "hex.h"


typedef struct HexRec_s
{
	unsigned int	addr;
	int				len;
	char			buf[16];
	HexRec_s		*next;
}HexRec;


//动作结果宏定义
#define OP_OK	0
#define OP_FAIL 1


//数据类型定义
#define	INT8S	char
#define INT8U   unsigned char
#define INT16S	short
#define INT16U	unsigned short
#define INT32S	int
#define INT32U  unsigned int

//位域结构体定义
typedef struct {
	INT8U l4:4;		//一个字节的低4位
	INT8U h4:4;		//一个字节的高4位		
}BITF;

//联合结构体定义
typedef union {
	INT16U word;	//一个字16位
	struct
	{
		INT8U lb;	//一个字的低8位
		INT8U hb;	//一个字的高8位
	}byte;
}UNWORD;

//联合结构体定义
typedef union {
	BITF  bf;
	INT8U byte;
}BYTEUNION;

//文件结构体定义
typedef struct {
	INT8S  start;	//每一条Hex记录的起始字符“:”
	INT8U  length;	//数据的字节数量
	INT16U address;	//数据存放的地址
	INT8U  type;	//HEX记录的类型
	INT8U  data[40];//一行最多有16个字节的数据
	INT8U  check;	//校验和
	INT16U offset;	//偏移量
	INT8U  format;	//数据行所从属的记录类型
}FILE_STRUCT;


HexRec *GetHexRec(HexRec **p, unsigned int addr, int len, char fillChar)
{
	HexRec *pTmp;

	while (*p != NULL)
	{
		if ((*p)->addr == addr)
		{
			return *p;
		}
		if (addr < (*p)->addr)
		{
			pTmp = *p;
			*p = (HexRec*)malloc(sizeof(HexRec));
			ASSERT(*p != NULL);
			memset(*p, 0, sizeof(HexRec));
			(*p)->addr = addr;
			(*p)->next = pTmp;
			(*p)->len = len;
			memset((*p)->buf, fillChar, sizeof((*p)->buf));
			return *p;
		}
		p = &((*p)->next);
	}
	*p = (HexRec*)malloc(sizeof(HexRec));
	ASSERT(*p != NULL);
	memset(*p, 0, sizeof(HexRec));
	(*p)->addr = addr;
	(*p)->len = len;
	(*p)->next = NULL;
	memset((*p)->buf, fillChar, sizeof((*p)->buf));

	return *p;
}


/*************************************************************************************
*函数名称:AsciiToHex()
*入口参数:ASCII码1、ASCII码2、Hex数据指针
*出口参数:操作结果
*功能描述:将传入的2位ASCII码合并为一个Hex格式的数据
*抛出异常:无
*
**************************************************************************************/
INT8U AsciiToHex(INT8S ascii1,INT8S ascii2,INT8U *p_data)
{
	INT8U status	=OP_OK;
	INT8S ascii;
	BYTEUNION temp;

	temp.byte	=0;
	
	//将传入的ASCII码全部转换为大写
	ascii	=(INT8S)toupper(ascii1);
	
	//判断传入的ascii码是否符合要求
	if( ascii >= '0' && ascii <= '9')
	{
		temp.bf.h4	=ascii-0x30;
	}
	else if( ascii >='A' && ascii <='F')
	{
		temp.bf.h4	=ascii-0x37;
	}
    else //其它字符认为是无效字符
	{
		status	=OP_FAIL;
	}

	//进行低4位转换
	if( OP_OK == status )
	{
		ascii	=(INT8S)toupper(ascii2);

		//判断传入的ascii码是否符合要求
		if( ascii >= '0' && ascii <= '9')
		{
			temp.bf.l4	=ascii-0x30;
		}
		else if( ascii >='A' && ascii <='F')
		{
			temp.bf.l4	=ascii-0x37;
		}
		else //其它字符认为是无效字符
		{
			status	=OP_FAIL;
		}
	}
	
	//获取转换后的Hex数据
	*p_data	=temp.byte;
	
	//返回执行结果
	return status;
}

/****************************************************************************************
*函数名称:HexToBin()
*入口参数:hex格式文件名称、bin文件名称、填充字节个数、填充数据指
*出口参数:无
*功能描述:实现将hex格式的文件类型转换为bin格式。
*抛出异常:1.无法打开指定的hex文件。
*****************************************************************************************/
int LoadHexFile(char *path, void *buf, int maxLength, int *pLen, unsigned int *startAddr, const char fillChar)
{
	FILE *f_hex;		//hex文件类型指针
	INT8S l_buf[100];	//一行hex记录存放数据
	INT8U l_num;		//一行中实际读取到的字节数
	INT8U i,j;			//索引
	INT16U l_sum;		//一行的数据加总和
	INT32U l_addr;		//文件位置指针
	HexRec *pHex = NULL;
	int ret = 0;
	int len = 0;
	unsigned int minAddr = 0xFFFFFFFF;
	HexRec *t = NULL;
	FILE_STRUCT hex;	//hex文件结构体声明

	ASSERT(buf != NULL);
	ASSERT(pLen != NULL);
	ASSERT(path != NULL);
	//////////////////////////////////////////////////////////////////////////
	//打开hex格式的文件---只读
	if( NULL == (f_hex = fopen(path,"r")))
	{
		return -1;
	}
	
	memset(buf, fillChar, maxLength);
	//////////////////////////////////////////////////////////////////////////
	//初始化hex记录数据结构体
	hex.start		='~';
	hex.length	=0x00;
	hex.address	=0x0000;
	hex.type		=0xFF;
	hex.check		=0xAA;
	hex.offset	=0x0000;
	hex.format	=0x00;

	//////////////////////////////////////////////////////////////////////////
	//文件指针重定向
	rewind(f_hex);
	//读取Hex文件内容
	while(1)
	{
		/////////////////////////////////////////////////////////////////////
		//首先找到起始字符
		l_num	=0;
		while( fgetc(f_hex) != ':')
		{	
			if( ++l_num >= 0xFE)	//在254字符内没有找到起始字符认为hex文件格式错误
			{
				ret = -2;
				goto err;
			}
		}

		/////////////////////////////////////////////////////////////////////
		//接收的字符个数清零
		l_num	=0;	
		//找到起始字符开始接收数据	
		l_buf[l_num++]	=':';
		//获取本行的数据长度
		l_buf[l_num++]	=(INT8S)fgetc(f_hex);
		l_buf[l_num++]	=(INT8S)fgetc(f_hex);
		if( OP_FAIL == AsciiToHex(l_buf[1],l_buf[2],&hex.length) )
		{
			ret = -3;
			goto err;
		}
		//一共要接收的数据个数为11+2*hex.length
		while(l_num < (11 + 2*hex.length) )
		{
			l_buf[l_num++]	=(INT8S)fgetc(f_hex);
		}

		//一行中至少应包含: ":"+"ll"+"aaaa"+"tt"+"cc"=11个字符
		if( l_num >= 11 )
		{
			///////////////////////////////////////////////////////////////////
			//保存起始字符			
			hex.start		=l_buf[0];

			///////////////////////////////////////////////////////////////////
			//获取本行的有效数据长度
			if( OP_FAIL == AsciiToHex(l_buf[1],l_buf[2],&hex.length) )
			{
				ret = -4;
				goto err;
			}

			///////////////////////////////////////////////////////////////////
			//获取本行的数据存放地址
			if( OP_FAIL == AsciiToHex(l_buf[3],l_buf[4],&l_num) )
			{
				ret = -4;
				goto err;
			}
			//获取地址的高8位
			hex.offset	=(INT16U)l_num;
			hex.offset	<<=8;

			if( OP_FAIL == AsciiToHex(l_buf[5],l_buf[6],&l_num) )
			{
				ret = -4;
				goto err;
			}
			//获取地址的低8位
			hex.offset	|=(INT16U)l_num;
			
			///////////////////////////////////////////////////////////////////
			//获取本行的数据类型
			if( OP_FAIL == AsciiToHex(l_buf[7],l_buf[8],&hex.type) )
			{
				ret = -4;
				goto err;
			}

			///////////////////////////////////////////////////////////////////
			//判断数据类型是否合法, 未处理05的数据
			if( 0x00 != hex.type && 0x01 != hex.type && 0x02 != hex.type && 0x04 != hex.type && 0x05 != hex.type)
			{
				ret = -4;
				goto err;
			}
			if (0x05 == hex.type)	//不处理05类型的数据
			{
				continue;
			}
			
			//////////////////////////////////////////////////////////////////
			//获取数据
			if( hex.length > 0 )
			{
				for(j=0;j<hex.length;j++)
				{
					if( OP_FAIL == AsciiToHex(l_buf[2*j+9],l_buf[2*j+10],&hex.data[j]) )
					{
						ret = -4;
						goto err;
					}
				}
			}

			i=9+2*hex.length;
			//////////////////////////////////////////////////////////////////
			//获取校验和
			if( OP_FAIL == AsciiToHex(l_buf[i],l_buf[i+1],&hex.check) )
			{
				ret = -4;
				goto err;
			}
			
			//////////////////////////////////////////////////////////////////
			//判断校验和是否正确
			l_sum	=hex.length;
			l_sum	+=(hex.offset>>8);
			l_sum	+=(hex.offset&0xFF);
			l_sum	+=hex.type;
			l_sum	+=hex.check;
			
			for(i=0;i<hex.length;i++)
			{
				l_sum	+=hex.data[i];
			}
			//校验和必然是256的整数倍,如果有御书则认为校验和失败
			if( l_sum & 0xFF )
			{				
				ret = -5;
				goto err;
			}
			
			//////////////////////////////////////////////////////////////////
			//根据数据类型决定是否写数据到bin文件中
			i=0;
			l_addr=0;
			switch( hex.type )
			{
				case 0x00:	//本行的数据类型为“数据记录”
					//本行所从属的数据类型为“数据记录”
					if( 0x00 == hex.format )
					{
						l_addr	=(INT32U)hex.offset;
					}
					//本行所从属的数据类型为“扩展段地址记录”(HEX86)--20位地址
					else if( 0x02 == hex.format)
					{
						l_addr	=((INT32U)hex.address<<4)+(INT32U)hex.offset;
					}
					//本行所从属的数据类型为“扩展线性地址记录”(HEX386)--32位地址
					else if( 0x04 == hex.format)
					{
						l_addr	=((INT32U)hex.address<<16)+(INT32U)hex.offset;
					}
					//文件结束
					else 
					{
						i=1;
						break;
					}
					
					//记录地址中的最大值
					if( minAddr > l_addr ) minAddr =l_addr;

					if (hex.length > 0)
					{
						t = GetHexRec(&pHex, l_addr, hex.length, 0xFF);
						ASSERT(t != NULL);
						memcpy(t->buf, hex.data, hex.length);
						len += hex.length;
					}
					break;

				case 0x01:	//本行的数据类型为“文件结束记录”
					//文件结束记录的数据个数一定是0x00
					if( hex.length == 0x00 )	i =1;
					hex.format	=0x01;
					break;

				case 0x02:	//本行的数据类型为“扩展段地址记录”
					//扩展段地址记录的数据个数一定是0x02
					if( hex.length != 0x02 )	i =3;
					//扩展段地址记录的地址一定是0x0000
					if( hex.offset != 0x0000) i =3;
					//更改hex从属的数据类型
					hex.format	=0x02;
					//获取段地址
					hex.address	=(((INT16U)hex.data[0]<<8)|hex.data[1]);
					break;

				case 0x04:	//本行的数据类型为“扩展线性地址记录”
					//扩展线性地址记录中的数据个数一定是0x02
					if( hex.length != 0x02 )	i =4;
					//扩展线性地址记录的地址一定是0x0000
					if( hex.offset != 0x0000) i =4;
					//更改hex从属的数据类型
					hex.format	=0x04;	
					//获取高16位地址
					hex.address	=(((INT16U)hex.data[0]<<8)|hex.data[1]);
					break;
			}
			//如果出现异常或文件结束退出循环
			if (i == 1)
			{
				break;
			}
			if( i > 0 )
			{
				ret = -6;
				goto err;
			}
		}
	}
	
	len = 0;
	int minLen = 0;
	int offset = 0;
	while (pHex != NULL)
	{
		offset = pHex->addr - minAddr;
		if (offset + pHex->len > maxLength)
		{
			ret = -7;
			goto err;
		}
		memcpy((char*)buf + offset, pHex->buf, pHex->len);
		if (minLen < offset + pHex->len)
		{
			minLen = offset + pHex->len;
		}
		len += pHex->len;
		t = pHex;
		pHex = pHex->next;
		free(t);
	}
	if (len < minLen)
	{
		len = minLen;
	}
	*pLen = len;

	if (startAddr)
	{
		*startAddr = minAddr;
	}

	//关闭文件
	fclose(f_hex);
	return 0;

err:
	while (pHex != NULL)
	{
		t = pHex;
		pHex = pHex->next;
		free(t);
	}
	fclose(f_hex);
	return ret;
}