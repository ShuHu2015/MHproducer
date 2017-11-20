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


//��������궨��
#define OP_OK	0
#define OP_FAIL 1


//�������Ͷ���
#define	INT8S	char
#define INT8U   unsigned char
#define INT16S	short
#define INT16U	unsigned short
#define INT32S	int
#define INT32U  unsigned int

//λ��ṹ�嶨��
typedef struct {
	INT8U l4:4;		//һ���ֽڵĵ�4λ
	INT8U h4:4;		//һ���ֽڵĸ�4λ		
}BITF;

//���Ͻṹ�嶨��
typedef union {
	INT16U word;	//һ����16λ
	struct
	{
		INT8U lb;	//һ���ֵĵ�8λ
		INT8U hb;	//һ���ֵĸ�8λ
	}byte;
}UNWORD;

//���Ͻṹ�嶨��
typedef union {
	BITF  bf;
	INT8U byte;
}BYTEUNION;

//�ļ��ṹ�嶨��
typedef struct {
	INT8S  start;	//ÿһ��Hex��¼����ʼ�ַ���:��
	INT8U  length;	//���ݵ��ֽ�����
	INT16U address;	//���ݴ�ŵĵ�ַ
	INT8U  type;	//HEX��¼������
	INT8U  data[40];//һ�������16���ֽڵ�����
	INT8U  check;	//У���
	INT16U offset;	//ƫ����
	INT8U  format;	//�������������ļ�¼����
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
*��������:AsciiToHex()
*��ڲ���:ASCII��1��ASCII��2��Hex����ָ��
*���ڲ���:�������
*��������:�������2λASCII��ϲ�Ϊһ��Hex��ʽ������
*�׳��쳣:��
*
**************************************************************************************/
INT8U AsciiToHex(INT8S ascii1,INT8S ascii2,INT8U *p_data)
{
	INT8U status	=OP_OK;
	INT8S ascii;
	BYTEUNION temp;

	temp.byte	=0;
	
	//�������ASCII��ȫ��ת��Ϊ��д
	ascii	=(INT8S)toupper(ascii1);
	
	//�жϴ����ascii���Ƿ����Ҫ��
	if( ascii >= '0' && ascii <= '9')
	{
		temp.bf.h4	=ascii-0x30;
	}
	else if( ascii >='A' && ascii <='F')
	{
		temp.bf.h4	=ascii-0x37;
	}
    else //�����ַ���Ϊ����Ч�ַ�
	{
		status	=OP_FAIL;
	}

	//���е�4λת��
	if( OP_OK == status )
	{
		ascii	=(INT8S)toupper(ascii2);

		//�жϴ����ascii���Ƿ����Ҫ��
		if( ascii >= '0' && ascii <= '9')
		{
			temp.bf.l4	=ascii-0x30;
		}
		else if( ascii >='A' && ascii <='F')
		{
			temp.bf.l4	=ascii-0x37;
		}
		else //�����ַ���Ϊ����Ч�ַ�
		{
			status	=OP_FAIL;
		}
	}
	
	//��ȡת�����Hex����
	*p_data	=temp.byte;
	
	//����ִ�н��
	return status;
}

/****************************************************************************************
*��������:HexToBin()
*��ڲ���:hex��ʽ�ļ����ơ�bin�ļ����ơ�����ֽڸ������������ָ
*���ڲ���:��
*��������:ʵ�ֽ�hex��ʽ���ļ�����ת��Ϊbin��ʽ��
*�׳��쳣:1.�޷���ָ����hex�ļ���
*****************************************************************************************/
int LoadHexFile(char *path, void *buf, int maxLength, int *pLen, unsigned int *startAddr, const char fillChar)
{
	FILE *f_hex;		//hex�ļ�����ָ��
	INT8S l_buf[100];	//һ��hex��¼�������
	INT8U l_num;		//һ����ʵ�ʶ�ȡ�����ֽ���
	INT8U i,j;			//����
	INT16U l_sum;		//һ�е����ݼ��ܺ�
	INT32U l_addr;		//�ļ�λ��ָ��
	HexRec *pHex = NULL;
	int ret = 0;
	int len = 0;
	unsigned int minAddr = 0xFFFFFFFF;
	HexRec *t = NULL;
	FILE_STRUCT hex;	//hex�ļ��ṹ������

	ASSERT(buf != NULL);
	ASSERT(pLen != NULL);
	ASSERT(path != NULL);
	//////////////////////////////////////////////////////////////////////////
	//��hex��ʽ���ļ�---ֻ��
	if( NULL == (f_hex = fopen(path,"r")))
	{
		return -1;
	}
	
	memset(buf, fillChar, maxLength);
	//////////////////////////////////////////////////////////////////////////
	//��ʼ��hex��¼���ݽṹ��
	hex.start		='~';
	hex.length	=0x00;
	hex.address	=0x0000;
	hex.type		=0xFF;
	hex.check		=0xAA;
	hex.offset	=0x0000;
	hex.format	=0x00;

	//////////////////////////////////////////////////////////////////////////
	//�ļ�ָ���ض���
	rewind(f_hex);
	//��ȡHex�ļ�����
	while(1)
	{
		/////////////////////////////////////////////////////////////////////
		//�����ҵ���ʼ�ַ�
		l_num	=0;
		while( fgetc(f_hex) != ':')
		{	
			if( ++l_num >= 0xFE)	//��254�ַ���û���ҵ���ʼ�ַ���Ϊhex�ļ���ʽ����
			{
				ret = -2;
				goto err;
			}
		}

		/////////////////////////////////////////////////////////////////////
		//���յ��ַ���������
		l_num	=0;	
		//�ҵ���ʼ�ַ���ʼ��������	
		l_buf[l_num++]	=':';
		//��ȡ���е����ݳ���
		l_buf[l_num++]	=(INT8S)fgetc(f_hex);
		l_buf[l_num++]	=(INT8S)fgetc(f_hex);
		if( OP_FAIL == AsciiToHex(l_buf[1],l_buf[2],&hex.length) )
		{
			ret = -3;
			goto err;
		}
		//һ��Ҫ���յ����ݸ���Ϊ11+2*hex.length
		while(l_num < (11 + 2*hex.length) )
		{
			l_buf[l_num++]	=(INT8S)fgetc(f_hex);
		}

		//һ��������Ӧ����: ":"+"ll"+"aaaa"+"tt"+"cc"=11���ַ�
		if( l_num >= 11 )
		{
			///////////////////////////////////////////////////////////////////
			//������ʼ�ַ�			
			hex.start		=l_buf[0];

			///////////////////////////////////////////////////////////////////
			//��ȡ���е���Ч���ݳ���
			if( OP_FAIL == AsciiToHex(l_buf[1],l_buf[2],&hex.length) )
			{
				ret = -4;
				goto err;
			}

			///////////////////////////////////////////////////////////////////
			//��ȡ���е����ݴ�ŵ�ַ
			if( OP_FAIL == AsciiToHex(l_buf[3],l_buf[4],&l_num) )
			{
				ret = -4;
				goto err;
			}
			//��ȡ��ַ�ĸ�8λ
			hex.offset	=(INT16U)l_num;
			hex.offset	<<=8;

			if( OP_FAIL == AsciiToHex(l_buf[5],l_buf[6],&l_num) )
			{
				ret = -4;
				goto err;
			}
			//��ȡ��ַ�ĵ�8λ
			hex.offset	|=(INT16U)l_num;
			
			///////////////////////////////////////////////////////////////////
			//��ȡ���е���������
			if( OP_FAIL == AsciiToHex(l_buf[7],l_buf[8],&hex.type) )
			{
				ret = -4;
				goto err;
			}

			///////////////////////////////////////////////////////////////////
			//�ж����������Ƿ�Ϸ�, δ����05������
			if( 0x00 != hex.type && 0x01 != hex.type && 0x02 != hex.type && 0x04 != hex.type && 0x05 != hex.type)
			{
				ret = -4;
				goto err;
			}
			if (0x05 == hex.type)	//������05���͵�����
			{
				continue;
			}
			
			//////////////////////////////////////////////////////////////////
			//��ȡ����
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
			//��ȡУ���
			if( OP_FAIL == AsciiToHex(l_buf[i],l_buf[i+1],&hex.check) )
			{
				ret = -4;
				goto err;
			}
			
			//////////////////////////////////////////////////////////////////
			//�ж�У����Ƿ���ȷ
			l_sum	=hex.length;
			l_sum	+=(hex.offset>>8);
			l_sum	+=(hex.offset&0xFF);
			l_sum	+=hex.type;
			l_sum	+=hex.check;
			
			for(i=0;i<hex.length;i++)
			{
				l_sum	+=hex.data[i];
			}
			//У��ͱ�Ȼ��256��������,�������������ΪУ���ʧ��
			if( l_sum & 0xFF )
			{				
				ret = -5;
				goto err;
			}
			
			//////////////////////////////////////////////////////////////////
			//�����������;����Ƿ�д���ݵ�bin�ļ���
			i=0;
			l_addr=0;
			switch( hex.type )
			{
				case 0x00:	//���е���������Ϊ�����ݼ�¼��
					//��������������������Ϊ�����ݼ�¼��
					if( 0x00 == hex.format )
					{
						l_addr	=(INT32U)hex.offset;
					}
					//��������������������Ϊ����չ�ε�ַ��¼��(HEX86)--20λ��ַ
					else if( 0x02 == hex.format)
					{
						l_addr	=((INT32U)hex.address<<4)+(INT32U)hex.offset;
					}
					//��������������������Ϊ����չ���Ե�ַ��¼��(HEX386)--32λ��ַ
					else if( 0x04 == hex.format)
					{
						l_addr	=((INT32U)hex.address<<16)+(INT32U)hex.offset;
					}
					//�ļ�����
					else 
					{
						i=1;
						break;
					}
					
					//��¼��ַ�е����ֵ
					if( minAddr > l_addr ) minAddr =l_addr;

					if (hex.length > 0)
					{
						t = GetHexRec(&pHex, l_addr, hex.length, 0xFF);
						ASSERT(t != NULL);
						memcpy(t->buf, hex.data, hex.length);
						len += hex.length;
					}
					break;

				case 0x01:	//���е���������Ϊ���ļ�������¼��
					//�ļ�������¼�����ݸ���һ����0x00
					if( hex.length == 0x00 )	i =1;
					hex.format	=0x01;
					break;

				case 0x02:	//���е���������Ϊ����չ�ε�ַ��¼��
					//��չ�ε�ַ��¼�����ݸ���һ����0x02
					if( hex.length != 0x02 )	i =3;
					//��չ�ε�ַ��¼�ĵ�ַһ����0x0000
					if( hex.offset != 0x0000) i =3;
					//����hex��������������
					hex.format	=0x02;
					//��ȡ�ε�ַ
					hex.address	=(((INT16U)hex.data[0]<<8)|hex.data[1]);
					break;

				case 0x04:	//���е���������Ϊ����չ���Ե�ַ��¼��
					//��չ���Ե�ַ��¼�е����ݸ���һ����0x02
					if( hex.length != 0x02 )	i =4;
					//��չ���Ե�ַ��¼�ĵ�ַһ����0x0000
					if( hex.offset != 0x0000) i =4;
					//����hex��������������
					hex.format	=0x04;	
					//��ȡ��16λ��ַ
					hex.address	=(((INT16U)hex.data[0]<<8)|hex.data[1]);
					break;
			}
			//��������쳣���ļ������˳�ѭ��
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

	//�ر��ļ�
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