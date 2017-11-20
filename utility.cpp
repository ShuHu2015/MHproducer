#include "StdAfx.h"
#include <iostream>
#include <fstream>
#include <stdlib.h> 
#include <Wincrypt.h>
#include "utility.h"

using namespace std;

#define CONFIGFILE		"config.data"

namespace utility
{
	const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	char * UnicodeToANSI(const wchar_t* str)
	{
		char* result;
		int textlen;
		textlen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
		result = (char *)malloc((textlen + 1) * sizeof(char));
		memset(result, 0, sizeof(char) * (textlen + 1));
		WideCharToMultiByte(CP_ACP, 0, str, -1, result, textlen, NULL, NULL);
		return result;
	}

	wchar_t * UTF8ToUnicode(const char* str)
	{
		int textlen;
		wchar_t * result;
		textlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
		result = (wchar_t *)malloc((textlen + 1) * sizeof(wchar_t));
		memset(result, 0, (textlen + 1) * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)result, textlen);
		return result;
	}

	char* UTF8ToANSI(const char* str)
	{
		wchar_t* pUnicode = UTF8ToUnicode(str);
		char* pAnsi = UnicodeToANSI(pUnicode);
		if (pUnicode != NULL)
		{
			free(pUnicode);
			pUnicode = NULL;
		}
		return pAnsi;
	}

	char *strtolower(char *s)
	{
		char *p;
		p = s;
		while (*p)
		{
			if (*p >= 'A' && *p <= 'Z')
			{
				*p += 0x20;
			}
			p++;
		}
		return s;
	}

	int GetExtFileName(const char *path, char *extName)
	{
		char buf[10] = { 0 };
		int ret = 0;
		int pos;

		for (pos = strlen(path) - 1; pos > 0; pos--)
		{
			if (path[pos] == '.')
			{
				strcpy(extName, path + pos + 1);
				return 0;
			}
		}

		return -1;
	}

	unsigned char ToHex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	unsigned char FromHex(unsigned char x)
	{
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		return y;
	}

	std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum((unsigned char)str[i]) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else
			{
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4);
				strTemp += ToHex((unsigned char)str[i] % 16);
			}
		}
		return strTemp;
	}

	std::string UrlDecode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') strTemp += ' ';
			else if (str[i] == '%')
			{
				unsigned char high = FromHex((unsigned char)str[++i]);
				unsigned char low = FromHex((unsigned char)str[++i]);
				strTemp += high * 16 + low;
			}
			else strTemp += str[i];
		}
		return strTemp;
	}

	char * base64_encode(unsigned char * bindata, char * base64, int binlength)
	{
		int i, j;
		unsigned char current;

		for (i = 0, j = 0; i < binlength; i += 3)
		{
			current = (bindata[i] >> 2);
			current &= (unsigned char)0x3F;
			base64[j++] = base64char[(int)current];

			current = ((unsigned char)(bindata[i] << 4)) & ((unsigned char)0x30);
			if (i + 1 >= binlength)
			{
				base64[j++] = base64char[(int)current];
				base64[j++] = '=';
				base64[j++] = '=';
				break;
			}
			current |= ((unsigned char)(bindata[i + 1] >> 4)) & ((unsigned char)0x0F);
			base64[j++] = base64char[(int)current];

			current = ((unsigned char)(bindata[i + 1] << 2)) & ((unsigned char)0x3C);
			if (i + 2 >= binlength)
			{
				base64[j++] = base64char[(int)current];
				base64[j++] = '=';
				break;
			}
			current |= ((unsigned char)(bindata[i + 2] >> 6)) & ((unsigned char)0x03);
			base64[j++] = base64char[(int)current];

			current = ((unsigned char)bindata[i + 2]) & ((unsigned char)0x3F);
			base64[j++] = base64char[(int)current];
		}
		base64[j] = '\0';
		return base64;
	}

	int base64_decode(const char * base64, unsigned char * bindata)
	{
		int i, j;
		unsigned char k;
		unsigned char temp[4];
		for (i = 0, j = 0; base64[i] != '\0'; i += 4)
		{
			memset(temp, 0xFF, sizeof(temp));
			for (k = 0; k < 64; k++)
			{
				if (base64char[k] == base64[i])
					temp[0] = k;
			}
			for (k = 0; k < 64; k++)
			{
				if (base64char[k] == base64[i + 1])
					temp[1] = k;
			}
			for (k = 0; k < 64; k++)
			{
				if (base64char[k] == base64[i + 2])
					temp[2] = k;
			}
			for (k = 0; k < 64; k++)
			{
				if (base64char[k] == base64[i + 3])
					temp[3] = k;
			}

			bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2)) & 0xFC)) |
				((unsigned char)((unsigned char)(temp[1] >> 4) & 0x03));
			if (base64[i + 2] == '=')
				break;

			bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0)) |
				((unsigned char)((unsigned char)(temp[2] >> 2) & 0x0F));
			if (base64[i + 3] == '=')
				break;

			bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0)) |
				((unsigned char)(temp[3] & 0x3F));
		}
		return j;
	}

	int loadconfiginfo(std::string keyname, std::string & keyvalue)
	{
		ifstream ifs(CONFIGFILE, ios::in);
		if (!ifs)
			return -1;

		string strline;
		string tmpname;
		while (getline(ifs, strline))
		{
			if (strline.find(keyname) != string::npos)
			{
				size_t pos = strline.find('=');
				tmpname = strline.substr(0, pos);
				if (tmpname != keyname)
					continue;
				keyvalue = strline.substr(pos + 1);

				ifs.close();
				return 0;
			}
		}
		ifs.close();
		return -1;
	}

	int  setconfiginfo(std::string keyname, std::string keyvalue)
	{
		ifstream ifs(CONFIGFILE, ios::in | ios::out);
		
		if (!ifs)
			return -1;	

		string strline = "";
		string strreplace = "";
		while (getline(ifs, strline))
		{		
			if (strline.find(keyname) != string::npos)
			{	
				strreplace += (keyname + "=" + keyvalue);
				strreplace += "\n";				
			}
			else
			{
				strreplace += strline;
				strreplace += "\n";
			}
		}
		ifs.close();


		ofstream ofs(CONFIGFILE, ios::out);
		if (!ofs)
			return -1;
		ofs.flush();
		ofs << strreplace;
		return 0;
	}
};

namespace utilityDES
{
	typedef struct
	{
		BLOBHEADER header;
		DWORD cbKeySize;
		BYTE rgbKeyData[8];
	}KeyBlob;

	//感觉不好，最好别用这两个个函数
	DWORD DESEncrypt(CString data, CString password, BYTE* buffer, DWORD bufferLength)
	{
		CT2CA passwd(password, CP_UTF8);
		CT2CA secret(data, CP_UTF8);
		CString tmpstr = secret;
		DWORD dataLength = strlen(secret);
		int pad = 8 - dataLength % 8;
		dataLength += pad;
		if (pad != 8)
		{
			for (int i = 0; i < pad; i++)
			{
				tmpstr += (char)0x07;
			}
		}
		//if (buffer == NULL )//|| bufferLength < dataLength + 8 - (dataLength % 8) || password.GetLength() < 8) 
		//	return 0;
		memcpy(buffer, tmpstr, dataLength);

		HCRYPTPROV hProv = NULL;
		HCRYPTKEY hSessionKey = NULL;
		BOOL bResult = TRUE;
		KeyBlob blob;
		blob.header.bType = PLAINTEXTKEYBLOB;
		blob.header.bVersion = CUR_BLOB_VERSION;
		blob.header.reserved = 0;
		blob.header.aiKeyAlg = CALG_DES;
		blob.cbKeySize = 8;
		memcpy(blob.rgbKeyData, passwd, 8);
		bResult &= CryptAcquireContextA(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0);
		bResult &= CryptImportKey(hProv, (BYTE*)&blob, sizeof(blob), 0, 0, &hSessionKey);
		bResult &= CryptSetKeyParam(hSessionKey, KP_IV, (BYTE*)IV, 0);
		DWORD tmplen = 16;
		//bResult &= CryptEncrypt(hSessionKey, NULL, FALSE, 0, (BYTE*)buffer, &tmplen, bufferLength);
		bResult &= CryptEncrypt(hSessionKey, NULL, TRUE, 0, (BYTE*)buffer, &tmplen, bufferLength + 8);
		int iii = GetLastError();
		bResult &= CryptDestroyKey(hSessionKey);
		bResult &= CryptReleaseContext(hProv, 0);
		return bResult ? dataLength : 0;
	}

	DWORD DESDecrypt(BYTE* buffer, DWORD bufferLength, CString password)
	{
		CT2CA passwd(password, CP_UTF8);
		DWORD dataLength = bufferLength;
		if (buffer == NULL || password.GetLength() < 8) return 0;
		HCRYPTPROV hProv = NULL;
		HCRYPTKEY hSessionKey = NULL;
		BOOL bResult = TRUE;
		KeyBlob blob;
		blob.header.bType = PLAINTEXTKEYBLOB;
		blob.header.bVersion = CUR_BLOB_VERSION;
		blob.header.reserved = 0;
		blob.header.aiKeyAlg = CALG_DES;
		blob.cbKeySize = 8;
		memcpy(blob.rgbKeyData, passwd, 8);
		bResult &= CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0);
		bResult &= CryptImportKey(hProv, (BYTE*)&blob, sizeof(blob), 0, 0, &hSessionKey);
		bResult &= CryptSetKeyParam(hSessionKey, KP_IV, (BYTE*)IV, 0);
		bResult &= CryptDecrypt(hSessionKey, NULL, TRUE, 0, buffer, &dataLength);
		bResult &= CryptDestroyKey(hSessionKey);
		bResult &= CryptReleaseContext(hProv, 0);
		return bResult ? dataLength : 0;
	}

	// initial permutation IP  
	const char IP_Table[64] = {
		58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
		62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
		57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
		61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7
	};
	// final permutation IP^-1   
	const char IPR_Table[64] = {
		40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
		38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
		36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
		34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25
	};
	// expansion operation matrix  
	const char E_Table[48] = {
		32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
		8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
		16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
		24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
	};
	// 32-bit permutation function P used on the output of the S-boxes   
	const char P_Table[32] = {
		16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
		2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25
	};
	// permuted choice table (key)   
	const char PC1_Table[56] = {
		57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
		10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
		63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
		14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
	};
	// permuted choice key (table)   
	const char PC2_Table[48] = {
		14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
		23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
		41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
		44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
	};
	// number left rotations of pc1   
	const char LOOP_Table[16] = {
		1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
	};
	// The (in)famous S-boxes   
	const char S_Box[8][4][16] = {
		// S1   
		14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
		0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
		4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
		15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
		// S2   
		15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
		3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
		0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
		13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
		// S3   
		10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
		13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
		13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
		1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
		// S4   
		7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
		13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
		10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
		3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
		// S5   
		2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
		14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
		4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
		11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
		// S6   
		12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
		10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
		9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
		4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
		// S7   
		4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
		13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
		1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
		6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
		// S8   
		13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
		1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
		7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
		2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
	};


	TDES::TDES()
	{
	}

	TDES::~TDES()
	{
	}

	/*******************************************************************/
	/*
	函 数 名 称:  ByteToBit
	功 能 描 述：  把BYTE转化为Bit流
	参 数 说 明：  Out:    输出的Bit流[in][out]
	In:     输入的BYTE流[in]
	bits:   Bit流的长度[in]

	返回值 说明：   void
	作       者:    邹德强
	更 新 日 期：  2003.12.19
	/*******************************************************************/
	static void ByteToBit(bool *Out, const unsigned char *In, int bits)
	{
		for (int i = 0; i<bits; ++i)
			Out[i] = (In[i >> 3] >> (7 - i & 7)) & 1;
	}

	/*******************************************************************/
	/*
	函 数 名 称:  BitToByte
	功 能 描 述：  把Bit转化为Byte流
	参 数 说 明：  Out:    输出的BYTE流[in][out]
	In:     输入的Bit流[in]
	bits:   Bit流的长度[in]

	返回值 说明：   void
	作       者:    邹德强
	更 新 日 期：  2003.12.19
	/*******************************************************************/
	static void BitToByte(unsigned char *Out, const bool *In, int bits)
	{
		memset(Out, 0, bits >> 3);
		for (int i = 0; i<bits; ++i)
			Out[i >> 3] |= In[i] << (7 - i & 7);
	}
	/*******************************************************************/
	/*
	函 数 名 称:  RotateL
	功 能 描 述：  把BIT流按位向左迭代
	参 数 说 明：  In:     输入的Bit流[in]
	len:    Bit流的长度[in]
	loop:   向左迭代的长度

	返回值 说明：   void
	作       者:    邹德强
	更 新 日 期：  2003.12.19
	/*******************************************************************/
	static void RotateL(bool *In, int len, int loop)
	{
		bool Tmp[256];

		memcpy(Tmp, In, loop);
		memcpy(In, In + loop, len - loop);
		memcpy(In + len - loop, Tmp, loop);
	}

	/*******************************************************************/
	/*
	函 数 名 称:  Xor
	功 能 描 述：  把两个Bit流进行异或
	参 数 说 明：  InA:    输入的Bit流[in][out]
	InB:    输入的Bit流[in]
	loop:   Bit流的长度

	返回值 说明：   void
	作       者:    邹德强
	更 新 日 期：  2003.12.19
	/*******************************************************************/
	static void Xor(bool *InA, const bool *InB, int len)
	{
		for (int i = 0; i<len; ++i)
			InA[i] ^= InB[i];
	}

	/*******************************************************************/
	/*
	函 数 名 称:  Transform
	功 能 描 述：  把两个Bit流按表进行位转化
	参 数 说 明：  Out:    输出的Bit流[out]
	In:     输入的Bit流[in]
	Table:  转化需要的表指针
	len:    转化表的长度

	返回值 说明：   void
	作       者:    邹德强
	更 新 日 期：  2003.12.19
	/*******************************************************************/
	static void Transform(bool *Out, bool *In, const char *Table, int len)
	{
		bool Tmp[256];

		for (int i = 0; i<len; ++i)
			Tmp[i] = In[Table[i] - 1];
		memcpy(Out, Tmp, len);
	}

	/*******************************************************************/
	/*
	函 数 名 称:  S_func
	功 能 描 述：  实现数据加密S BOX模块
	参 数 说 明：  Out:    输出的32Bit[out]
	In:     输入的48Bit[in]

	返回值 说明：   void
	作       者:    邹德强
	更 新 日 期：  2003.12.19
	/*******************************************************************/
	static void S_func(bool Out[32], const bool In[48])
	{
		for (char i = 0, j, k; i<8; ++i, In += 6, Out += 4)
		{
			j = (In[0] << 1) + In[5];
			k = (In[1] << 3) + (In[2] << 2) + (In[3] << 1) + In[4]; //组织SID下标  

			for (int l = 0; l<4; ++l)                               //把相应4bit赋值  
				Out[l] = (S_Box[i][j][k] >> (3 - l)) & 1;
		}
	}

	/*******************************************************************/
	/*
	函 数 名 称:  F_func
	功 能 描 述：  实现数据加密到输出P
	参 数 说 明：  Out:    输出的32Bit[out]
	In:     输入的48Bit[in]

	返回值 说明：   void
	作       者:    邹德强
	更 新 日 期：  2003.12.19
	/*******************************************************************/
	static void F_func(bool In[32], const bool Ki[48])
	{
		bool MR[48];
		Transform(MR, In, E_Table, 48);
		Xor(MR, Ki, 48);
		S_func(In, MR);
		Transform(In, In, P_Table, 32);
	}

	//计算并填充子密钥到SubKey数据中  
	void TDES::SetSubKey(PSubKey pSubKey, const unsigned char Key[8])
	{
		bool K[64], *KL = &K[0], *KR = &K[28];
		ByteToBit(K, Key, 64);
		Transform(K, K, PC1_Table, 56);
		for (int i = 0; i<16; ++i) {
			RotateL(KL, 28, LOOP_Table[i]);
			RotateL(KR, 28, LOOP_Table[i]);
			Transform((*pSubKey)[i], K, PC2_Table, 48);
		}
	}

	//DES单元运算  
	void TDES::DES(unsigned char Out[8], const unsigned char In[8], const PSubKey pSubKey, bool Type)
	{
		bool M[64], tmp[32], *Li = &M[0], *Ri = &M[32];
		ByteToBit(M, In, 64);
		Transform(M, M, IP_Table, 64);
		if (Type == ENCRYPT)
		{
			for (int i = 0; i<16; ++i)
			{
				memcpy(tmp, Ri, 32);        //Ri[i-1] 保存  
				F_func(Ri, (*pSubKey)[i]);  //Ri[i-1]经过转化和SBox输出为P  
				Xor(Ri, Li, 32);            //Ri[i] = P XOR Li[i-1]  
				memcpy(Li, tmp, 32);        //Li[i] = Ri[i-1]  
			}
		}
		else
		{
			for (int i = 15; i >= 0; --i)
			{
				memcpy(tmp, Ri, 32);        //Ri[i-1] 保存  
				F_func(Ri, (*pSubKey)[i]);  //Ri[i-1]经过转化和SBox输出为P  
				Xor(Ri, Li, 32);            //Ri[i] = P XOR Li[i-1]  
				memcpy(Li, tmp, 32);        //Li[i] = Ri[i-1]  
			}
		}
		RotateL(M, 64, 32);                   //Ri与Li换位重组M  
		Transform(M, M, IPR_Table, 64);     //最后结果进行转化  
		BitToByte(Out, M, 64);              //组织成字符  
	}

	bool TDES::RunDES(bool bType, bool bMode, int PaddingMode, const unsigned char* Iv, const unsigned char* In,
		unsigned char* Out, unsigned datalen, const unsigned char* Key, unsigned keylen)
	{
		memset(Out, 0x00, strlen((char*)Out));
		unsigned char* outbuf = Out;
		//判断输入合法性  
		if (!(/*In && */outbuf && Key && /*datalen &&*/ keylen >= 8)) // 空字符串加密的时候In和datalen都为0,应该去掉此判断  
			return false;

		unsigned char* inbuf = new unsigned char[datalen + 8];
		memset(inbuf, 0x00, datalen + 8);
		memcpy(inbuf, In, datalen);
		unsigned padlen = datalen;
		// 根据填充模式填充  
		if (!RunPad(bType, PaddingMode, In, datalen, inbuf, padlen))
		{
			delete[]inbuf; inbuf = NULL;
			return false;
		}
		unsigned char* tempBuf = inbuf;

		bool m_SubKey[3][16][48];        //密钥  
										 //构造并生成SubKeys  
		unsigned char nKey = (keylen >> 3) >= 3 ? 3 : (keylen >> 3);
		for (int i = 0; i<nKey; i++)
		{
			SetSubKey(&m_SubKey[i], &Key[i << 3]);
		}

		if (bMode == ECB)    //ECB模式  
		{
			if (nKey == 1)  //单Key  
			{
				int j = padlen >> 3;
				for (int i = 0, j = padlen >> 3; i<j; ++i, outbuf += 8, tempBuf += 8)
				{
					DES(outbuf, tempBuf, &m_SubKey[0], bType);
				}
			}
			else
				if (nKey == 2)   //3DES 2Key  
				{
					for (int i = 0, j = padlen >> 3; i<j; ++i, outbuf += 8, tempBuf += 8)
					{
						DES(outbuf, tempBuf, &m_SubKey[0], bType);
						DES(outbuf, outbuf, &m_SubKey[1], !bType);
						DES(outbuf, outbuf, &m_SubKey[0], bType);
					}
				}
				else            //3DES 3Key  
				{
					for (int i = 0, j = padlen >> 3; i<j; ++i, outbuf += 8, tempBuf += 8)
					{
						DES(outbuf, tempBuf, &m_SubKey[bType ? 2 : 0], bType);
						DES(outbuf, outbuf, &m_SubKey[1], !bType);
						DES(outbuf, outbuf, &m_SubKey[bType ? 0 : 2], bType);
					}
				}
		}
		else                //CBC模式  
		{
			unsigned char   cvec[8] = ""; // 扭转向量  
			unsigned char   cvin[8] = ""; // 中间变量  
			memcpy(cvec, Iv, 8);

			if (nKey == 1)   //单Key  
			{
				for (int i = 0, j = padlen >> 3; i<j; ++i, outbuf += 8, tempBuf += 8)
				{
					if (bType == TDES::ENCRYPT)
					{
						for (int j = 0; j<8; ++j)     //将输入与扭转变量异或  
						{
							cvin[j] = tempBuf[j] ^ cvec[j];
						}
					}
					else
					{
						memcpy(cvin, tempBuf, 8);
					}

					DES(outbuf, cvin, &m_SubKey[0], bType);

					if (bType == TDES::ENCRYPT)
					{
						memcpy(cvec, outbuf, 8);         //将输出设定为扭转变量  
					}
					else
					{
						for (int j = 0; j<8; ++j)     //将输出与扭转变量异或  
						{
							outbuf[j] = outbuf[j] ^ cvec[j];
						}
						memcpy(cvec, cvin, 8);            //将输入设定为扭转变量  
					}
				}
			}
			else
				if (nKey == 2)   //3DES CBC 2Key  
				{
					for (int i = 0, j = padlen >> 3; i<j; ++i, outbuf += 8, tempBuf += 8)
					{
						if (bType == TDES::ENCRYPT)
						{
							for (int j = 0; j<8; ++j)     //将输入与扭转变量异或  
							{
								cvin[j] = tempBuf[j] ^ cvec[j];
							}
						}
						else
						{
							memcpy(cvin, tempBuf, 8);
						}

						DES(outbuf, cvin, &m_SubKey[0], bType);
						DES(outbuf, outbuf, &m_SubKey[1], !bType);
						DES(outbuf, outbuf, &m_SubKey[0], bType);

						if (bType == TDES::ENCRYPT)
						{
							memcpy(cvec, outbuf, 8);         //将输出设定为扭转变量  
						}
						else
						{
							for (int j = 0; j<8; ++j)     //将输出与扭转变量异或  
							{
								outbuf[j] = outbuf[j] ^ cvec[j];
							}
							memcpy(cvec, cvin, 8);            //将输入设定为扭转变量  
						}
					}
				}
				else            //3DES CBC 3Key  
				{
					for (int i = 0, j = padlen >> 3; i<j; ++i, outbuf += 8, tempBuf += 8)
					{
						if (bType == TDES::ENCRYPT)
						{
							for (int j = 0; j<8; ++j)     //将输入与扭转变量异或  
							{
								cvin[j] = tempBuf[j] ^ cvec[j];
							}
						}
						else
						{
							memcpy(cvin, tempBuf, 8);
						}

						DES(outbuf, cvin, &m_SubKey[bType ? 2 : 0], bType);
						DES(outbuf, outbuf, &m_SubKey[1], !bType);
						DES(outbuf, outbuf, &m_SubKey[bType ? 0 : 2], bType);

						if (bType == TDES::ENCRYPT)
						{
							memcpy(cvec, outbuf, 8);         //将输出设定为扭转变量  
						}
						else
						{
							for (int j = 0; j<8; ++j)     //将输出与扭转变量异或  
							{
								outbuf[j] = outbuf[j] ^ cvec[j];
							}
							memcpy(cvec, cvin, 8);            //将输入设定为扭转变量  
						}
					}
				}
		}
		if (inbuf)
		{
			delete[]inbuf;
			inbuf = NULL;
		}
		if (bType == TDES::DECRYPT)
		{
			if (PaddingMode == PAD_ISO_1)
			{
				//待补充  
			}
			else
				if (PaddingMode == PAD_ISO_2)
				{
					//待补充  
				}
				else
					if (PaddingMode == PAD_PKCS_7)
					{
						unsigned int l_Out = strlen((char*)Out);
						unsigned int l_num = Out[l_Out - 1];
						if (l_num <= 8) // 非法密文會造成此處出問題，加以保護  
							memset(Out + l_Out - l_num, 0x00, l_num);
					}
		}

		return true;
	}

	/*******************************************************************/
	/*
	函 数 名 称:  RunPad
	功 能 描 述： 根据协议对加密前的数据进行填充
	参 数 说 明： bType   :类型：PAD类型
	In      :数据串指针
	Out     :填充输出串指针
	datalen :数据的长度
	padlen  :(in,out)输出buffer的长度，填充后的长度

	返回值 说明： bool    :是否填充成功
	修 改 历 史：
	2014.09.10 修改PKCS7填充算法；修改明文是8byte的整倍数时加密解密算法；修改解密算法
	更 新 日 期：  2014.09.10
	*/
	/*******************************************************************/
	bool TDES::RunPad(bool bType, int nType, const unsigned char* In,
		unsigned datalen, unsigned char* Out, unsigned& padlen)
	{
		if (nType < PAD_ISO_1 || nType > PAD_PKCS_7)
			return false;

		if (In == NULL || datalen < 0 || Out == NULL)
			return false;
		int res = (datalen & 0x07);

		if (bType == TDES::DECRYPT)
		{
			padlen = datalen;
			memcpy(Out, In, datalen);
			return true;
		}

		padlen = (datalen + 8 - res);
		memcpy(Out, In, datalen);

		if (nType == PAD_ISO_1)
		{
			memset(Out + datalen, 0x00, 8 - res);
		}
		else
			if (nType == PAD_ISO_2)
			{
				memset(Out + datalen, 0x80, 1);
				memset(Out + datalen, 0x00, 7 - res);
			}
			else
				if (nType == PAD_PKCS_7)
				{
					memset(Out + datalen, 8 - res, 8 - res);
				}
				else
				{
					// 其他填充模式尚待补充  
					return false;
				}

		return true;
	}

	//转换前 aaaaaabb ccccdddd eeffffff  
	//转换后 00aaaaaa 00bbcccc 00ddddee 00ffffff  
	void TBase64::Base64_Encode(unsigned char* src, unsigned char* dest, int srclen)
	{
		//编码函数  
		unsigned char EncodeIndex[] =
		{
			//编码索引表  
			'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
			'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
			'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
			'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/','='
		};

		int sign = 0;
		for (int i = 0; i != srclen; i++, src++, dest++)
		{
			switch (sign)
			{
			case 0://编码第1字节  
				*(dest) = EncodeIndex[*src >> 2];
				break;
			case 1://编码第2字节  
				*dest = EncodeIndex[((*(src - 1) & 0x03) << 4) | (((*src) & 0xF0) >> 4)];
				break;
			case 2://编码第3字节  
				*dest = EncodeIndex[((*(src - 1) & 0x0F) << 2) | ((*(src) & 0xC0) >> 6)];
				*(++dest) = EncodeIndex[(*(src) & 0x3F)];//编码第4字节  
				break;
			}
			(sign == 2) ? (sign = 0) : (sign++);
		}

		switch (sign)
		{
			//3的余数字节，后补=处理  
		case 0:
			break;
		case 1:
			// *(dest++) = EncodeIndex[((*(src-1)  & 0x03) << 4) | (((*src) & 0xF0) >> 4)];  
			*(dest++) = EncodeIndex[((*(src - 1) & 0x03) << 4)];
			*(dest++) = '=';
			*(dest++) = '=';
			break;
		case 2:
			// *(dest++) = EncodeIndex[((*(src-1) &0x0F) << 2) | ((*(src) & 0xC0) >> 6)];  
			*(dest++) = EncodeIndex[((*(src - 1) & 0x0F) << 2)];
			*(dest++) = '=';
			break;
		default:
			break;
		}
	}
	//---------------------------------------------------------------------------  

	void TBase64::Base64_Decode(unsigned char* src, unsigned char* dest, int srclen)
	{
		unsigned char DecodeIndex[] =
		{
			//解码索引表  
			0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,//0  00-15  
			0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,//1  16-31  
			0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x3E,0x40,0x40,0x40,0x3F,//2  32-47    43[+](0x38)  47[/](0x39)  
			0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x40,0x40,0x40,0x40,0x40,0x40,//3  48-63    48[0](0x34)- 57[9](0x3D)  61[=](0x40)  
			0x40,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,//4  64-79    65[A](0x00)- 79[O](0x0E)  
			0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x40,0x40,0x40,0x40,0x40,//5  80-95    80[P](0x0F)- 90[Z](0x19)  
			0x40,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,//6  96-111   97[a](0x1A)-111[o](0x28)  
			0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x40,0x40,0x40,0x40,0x40 //7 112-127  122[p](0x29)-122[z](0x33)  
		};

		//解码处理函数 //len%4 == 0总为true;  
		for (int i = 0; i != srclen / 4; i++)//对于不足4个的不作计算  
		{
			//每个字符,通过数组直接得到其值,比较快  
			*dest = (DecodeIndex[*src] << 2) | ((DecodeIndex[*(src + 1)] & 0x30) >> 4);
			*(dest + 1) = (DecodeIndex[*(src + 1)] << 4) | ((DecodeIndex[*(src + 2)] & 0x3C) >> 2);
			*(dest + 2) = ((DecodeIndex[*(src + 2)] & 0x03) << 6) | (DecodeIndex[*(src + 3)] & 0x3F);
			src += 4;
			dest += 3;
		}
	}
	//---------------------------------------------------------------------------  
	//*/  
	int TBase64::GetLenEncode(const char* src)
	{
		//求编码后的长度  
		int len = strlen((char*)src);
		return (len + (len % 3 == 0 ? 0 : (3 - len % 3))) / 3 * 4 + 1;
	}
	//---------------------------------------------------------------------------  
	int TBase64::GetLenDecode(const char* src)
	{
		//求解码后的长度  
		int len = strlen(src);
		return len / 4 * 3 + 1;
	}
	//---------------------------------------------------------------------------  
	char* TBase64::Base64_Encode(const char* src)
	{
		int src_len = strlen(src);
		int lenEncode = GetLenEncode(src);
		unsigned char* Base64Out = new unsigned char[lenEncode];
		memset(Base64Out, 0x00, lenEncode);
		Base64_Encode((unsigned char *)src, (unsigned char *)Base64Out, src_len);//原字符长度  
		return (char*)Base64Out;
	}
	//---------------------------------------------------------------------------  
	char* TBase64::Base64_Decode(const char* src)
	{
		int lenEncode = strlen(src);
		int lenDecode = GetLenDecode((const char *)src);//获得编码后字符串的再解码的长度  
		unsigned char* pDecodeStr = new unsigned char[lenDecode];
		memset(pDecodeStr, 0x00, lenDecode);
		Base64_Decode((unsigned char *)src, pDecodeStr, lenEncode);//编码后的字符长度  
		return (char*)pDecodeStr;
	}
};