#pragma once
#include <string>
#include <vector>
#include <map>
using namespace std;

class userinfo
{
public:
	userinfo();
	~userinfo();

	userinfo(userinfo &rvalue);

	string username;
	string group;
	string datatime;
	vector<string> strproduct;
	map<string, int> licensen;
	map<string, vector<int>> authpermission;

	void makedefault() 
	{
		username = "";
		group = "";
		datatime = "";
		strproduct.clear();
		licensen.clear();
		authpermission.clear();
	}
};

class pophttpcomm
{
public:
	pophttpcomm();
	~pophttpcomm();

	pophttpcomm(pophttpcomm &rvlue);

	string   accse_token;
	string   m_ipaddress;
	int	     m_port;

	//��ǰ��¼���û���Ϣ
	userinfo info;

	void initialhttp(string ipaddress, int port);

	//ֻ����Ӣ�ĺ���ĸ�������û����������У���������û�н���utf-8��ת��
	int  loginandgettoken(string username, string password, string* token);

	//rsa ǩ��ʹ�� datalen ������256���ֽڣ����ص�Ҳ������256���ֽڷ��򱨴�
	int  rasencrypt(unsigned char* data, int datalen, unsigned char* output);

	//rsa ǩ��ʹ�� datalen ������256���ֽڣ����ص�Ҳ������256���ֽڷ��򱨴�,  
	int  rasencrypt(char* key, unsigned char* data, int datalen, unsigned char* output, CString& errMessage);

	//��ȡ�û���Ϣ
	int  queryuserinfo();

	//rsa ��Ȩʹ�� datalen ������256���ֽڣ����ص�Ҳ������256���ֽڷ��򱨴�
	int  authrequest(unsigned char* data, int datalen, unsigned char* output, CString& errMessage);


	int  uploadproduceresult(string szresult, CString& errMessage);
};
