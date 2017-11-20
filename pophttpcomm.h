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

	//当前登录的用户信息
	userinfo info;

	void initialhttp(string ipaddress, int port);

	//只允许英文和字母出现在用户名和密码中！函数里面没有进行utf-8的转码
	int  loginandgettoken(string username, string password, string* token);

	//rsa 签名使用 datalen 必须是256个字节，返回的也必须是256个字节否则报错
	int  rasencrypt(unsigned char* data, int datalen, unsigned char* output);

	//rsa 签名使用 datalen 必须是256个字节，返回的也必须是256个字节否则报错,  
	int  rasencrypt(char* key, unsigned char* data, int datalen, unsigned char* output, CString& errMessage);

	//获取用户信息
	int  queryuserinfo();

	//rsa 授权使用 datalen 必须是256个字节，返回的也必须是256个字节否则报错
	int  authrequest(unsigned char* data, int datalen, unsigned char* output, CString& errMessage);


	int  uploadproduceresult(string szresult, CString& errMessage);
};
