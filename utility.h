#pragma once

#include <string>

namespace utility
{
	char* UTF8ToANSI(const char* str);
	char * UnicodeToANSI(const wchar_t* str);
	wchar_t * UTF8ToUnicode(const char* str);
	char *strtolower(char *s);
	int GetExtFileName(const char *path, char *extName);

	unsigned char ToHex(unsigned char x);
	unsigned char FromHex(unsigned char x);
	std::string UrlEncode(const std::string& str);
	std::string UrlDecode(const std::string& str);
	char * base64_encode(unsigned char * bindata, char * base64, int binlength);
	int base64_decode(const char * base64, unsigned char * bindata);

	int loadconfiginfo(std::string keyname, std::string& keyvalue);

	int setconfiginfo(std::string keyname, std::string keyvalue);
};


namespace utilityDES
{

	const BYTE IV[] = { 55, 103, 24, 179, 36, 99, 167, 3 };
	DWORD DESEncrypt(CString data, CString password, BYTE * buffer, DWORD bufferLength);
	DWORD DESDecrypt(BYTE* buffer, DWORD bufferLength, CString password);


	/* TDES��˵��
	*
	* ������DES��3DES�㷨��
	*
	*/
	class TDES
	{
	public:
		TDES();
		virtual ~TDES();

		//���ܽ���  
		enum
		{
			ENCRYPT = 0,    // ����  
			DECRYPT,        // ����  
		};

		//DES�㷨��ģʽ  
		enum
		{
			ECB = 0,    // ECBģʽ  
			CBC             // CBCģʽ  
		};

		typedef bool(*PSubKey)[16][48];

		//Pad����ģʽ  
		enum
		{
			PAD_ISO_1 = 0,  // ISO_1��䣺���ݳ��Ȳ���8���صı�������0x00���㣬���Ϊ8���صı�������8��0x00  
			PAD_ISO_2,      // ISO_2��䣺���ݳ��Ȳ���8���صı�������0x80,0x00..���㣬���Ϊ8���صı�������0x80,0x00..0x00  
			PAD_PKCS_7      // PKCS7��䣺���ݳ��ȳ�8����Ϊn,��(8-n)����Ϊ8�ı��������Ϊ8���صı�������8��0x08  
		};

		/* ִ��DES�㷨���ı��ӽ���
		*
		* Description    : ִ��DES�㷨���ı��ӽ���
		* @param bType   : ���ͣ�����ENCRYPT������DECRYPT
		* @param bMode   : ģʽ��ECB,CBC
		* @param In      : �����ܴ�ָ��
		* @param Out     : �������ָ��
		* @param datalen : �����ܴ��ĳ��ȣ�ͬʱOut�Ļ�������СӦ���ڻ��ߵ���datalen
		* @param Key     : ��Կ(��Ϊ8λ,16λ,24λ)֧��3��Կ
		* @param keylen  : ��Կ���ȣ����24λ���ֽ����Զ��ü�
		* @return true--�ɹ���false--ʧ�ܣ�
		*/
		static bool RunDES(bool bType, bool bMode, int PaddingMode, const unsigned char* IV, const unsigned char* In,
			unsigned char* Out, unsigned datalen, const unsigned char* Key, unsigned keylen);

	protected:
		//���㲢�������Կ��SubKey������  
		static void SetSubKey(PSubKey pSubKey, const unsigned char Key[8]);

		//DES��Ԫ����  
		static void DES(unsigned char Out[8], const unsigned char In[8], const PSubKey pSubKey, bool Type);

		/* ����8λ����
		*
		* Description    : ����Э��Լ���ǰ�����ݽ������
		* @param nType   : ���ͣ�PAD����
		* @param In      : ���ݴ�ָ��
		* @param Out     : ��������ָ��
		* @param datalen : ���ݵĳ���
		* @param padlen  : (in,out)���buffer�ĳ��ȣ�����ĳ���
		* @return true--�ɹ���false--ʧ�ܣ�
		*/
		static bool RunPad(bool bType, int nType, const unsigned char* In,
			unsigned datalen, unsigned char* Out, unsigned& padlen);
	};
	//---------------------------------------------------------------------------  

	/* TBase64��˵��
	*
	* ������Base64������
	*
	*/
	class TBase64
	{
	public:
		static char* Base64_Encode(const char* src);
		static char* Base64_Decode(const char* src);
	protected:
		static void Base64_Encode(unsigned char* src, unsigned char* dest, int srclen);
		static void Base64_Decode(unsigned char* src, unsigned char* dest, int srclen);
		static int GetLenEncode(const char* src);
		static int GetLenDecode(const char* src);
	};
};


