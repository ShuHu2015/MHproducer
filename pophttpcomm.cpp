#include "stdafx.h"

#include <afxinet.h> 
#include "utility.h"
#include "pophttpcomm.h"
#include "json\json.h"
#include "json\value.h"
#include <vector>
#include "hslog.h"

using namespace utilityDES;

pophttpcomm::pophttpcomm()
{
}

pophttpcomm::~pophttpcomm()
{
}

pophttpcomm::pophttpcomm(pophttpcomm &rvlue)
{
	this->accse_token = rvlue.accse_token;
	this->m_ipaddress = rvlue.m_ipaddress;
	this->m_port = rvlue.m_port;
	this->info = rvlue.info;
}

int  pophttpcomm::rasencrypt(char* key, unsigned char* data, int datalen, unsigned char* output, CString& errMessage)
{
	int iret = 0;

	CInternetSession session(NULL);
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	unsigned char oridata[300] = { 0 };
	strcpy((char* )oridata, key);
	strcpy((char*)oridata + strlen((char*)oridata), ".PVK");
	memcpy(oridata + 44, data, 256);
	try
	{
		pServer = session.GetHttpConnection(m_ipaddress.c_str(), (INTERNET_PORT)m_port);

		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/api/Products/RSASignB"));
		CString strtoken;
		strtoken.Format("Authorization: Bearer %s", accse_token.c_str());
		pFile->AddRequestHeaders(strtoken);
		pFile->AddRequestHeaders("Content-Type: application/json; charset=utf-8");
		char base64[1024] = { 0 };
		utility::base64_encode(oridata, base64, 300);

		CString strrequest = "";
		strrequest.Format("\"%s\"", base64);

		pFile->SendRequestEx(strrequest.GetLength());
		pFile->WriteString(strrequest); //重要-->m_Request 中有"name=aaa&name2=BBB&..."  
		pFile->EndRequest();

		char szChars[1024 + 1] = { 0 };
		DWORD dwRet = 0;
		pFile->QueryInfoStatusCode(dwRet);	//查询执行状态  
		printf("HTTP_STATUS_code:%d\n", dwRet);
		if (dwRet != HTTP_STATUS_OK) {		//http请求执行失败  
			memset(szChars, 0, 1024 + 1);
			UINT nReaded = 0;
			CString tmpstr;
			while ((nReaded = pFile->Read((void*)szChars, 1024)) > 0)
			{
				szChars[nReaded] = '\0';
				tmpstr += szChars;
				memset(szChars, 0, 1024 + 1);
			}
			char* err = utility::UTF8ToANSI(tmpstr.GetBuffer());
			errMessage = err;
			if (err != NULL)
			{
				free(err);
				err = NULL;
			}
			iret = -1;
			goto EXIST;
		}

		
		memset(szChars, 0, 1024 + 1);
		int nReaded = 0;
		int ialready = 0;
		while ((nReaded = pFile->Read((void*)szChars, 1024)) > 0)
		{
			memcpy(output + ialready, szChars, nReaded);
			ialready += nReaded;
			if (ialready > 256)
			{
				iret = -1;
				goto EXIST;
			}
		}

	}
	catch (CInternetException *pEx)
	{
		CString tmp = "";
		tmp.Format("rasencrypt CInternetException err %d , %d\r\n", pEx->m_dwError, (pEx->m_dwContext));
		hslog::log(tmp.GetBuffer());;
		iret = -1;
		pEx->Delete();
	}
	catch (...)
	{
		iret = -1;
	}
EXIST:
	if (NULL != pFile)
	{
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}

	if (NULL != pServer)
	{
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	session.Close();
	return iret;
}

int  pophttpcomm::loginandgettoken(string username, string password, string* token)
{
	int iRet = 0;
	
	const BYTE key[] = { 42, 16, 93, 156, 78, 4, 218, 32 };
	const BYTE IV[] = { 55, 103, 24, 179, 36, 99, 167, 3 };
	unsigned char enpassword[1024] = { 0 };
	if (!TDES::RunDES(TDES::ENCRYPT, TDES::CBC, TDES::PAD_PKCS_7, IV, (unsigned char*)password.c_str(), enpassword, password.size(), key, sizeof(key)))
		return -1;
	char strrequest[1024] = { 0 };	
	char base64[1024] = { 0 };
	utility::base64_encode(enpassword, (char*)base64, strlen((char* )enpassword));

	sprintf(strrequest, "grant_type=password&username=%s&password=%s", utility::UrlEncode(username).c_str(), utility::UrlEncode(base64).c_str());

	CInternetSession session(NULL);
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	try
	{
		pServer = session.GetHttpConnection(m_ipaddress.c_str(), (INTERNET_PORT)m_port);

		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/Token"));
		pFile->SendRequestEx(strlen(strrequest));
		pFile->WriteString(strrequest); //重要-->m_Request 中有"name=aaa&name2=BBB&..."  
		pFile->EndRequest();
		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);//查询执行状态  
		if (dwRet != HTTP_STATUS_OK) 
		{//http请求执行失败  
			iRet = -1;
			goto EXIST;
		}

		char szChars[1024 + 1] = { 0 };
		CString strRawResponse = "";
		UINT nReaded = 0;
		while ((nReaded = pFile->Read((void*)szChars, 1024)) > 0)
		{
			szChars[nReaded] = '\0';
			strRawResponse += szChars;
			memset(szChars, 0, 1024 + 1);
		}

		Json::Reader reader;
		Json::Value root;
		Json::Value::Members mem = root.getMemberNames();
		if (reader.parse(strRawResponse.GetBuffer(), root))  // reader将Json字符串解析到root，root将包含Json里所有子元素  
		{
			accse_token = root["access_token"].asString().c_str();  
			*token = accse_token;
		}
		else
		{
			iRet = -1;
			goto EXIST;
		}
		strRawResponse.ReleaseBuffer();
	}
	catch (CInternetException *pEx)
	{
		CString tmp = "";
		tmp.Format("loginandgettoken CInternetException err %d , %d\r\n", pEx->m_dwError, (pEx->m_dwContext));
		hslog::log(tmp.GetBuffer());
		iRet = -1;
		pEx->Delete();
	}
	catch (...)
	{
		CString tmp = "loginandgettoken Exception \r\n";
		hslog::log(tmp.GetBuffer());
		iRet = -1;
	}

EXIST:
	if (NULL != pFile)
	{
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}

	if (NULL != pServer)
	{
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	session.Close();
	return iRet;
}

int  pophttpcomm::rasencrypt(unsigned char* data, int datalen, unsigned char* output)
{
	int iret = 0;

	CInternetSession session(NULL);
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;

	try
	{
		pServer = session.GetHttpConnection(m_ipaddress.c_str(), (INTERNET_PORT)m_port);

		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/api/Products/RSASign"));
		CString strtoken;
		strtoken.Format("Authorization: Bearer %s", accse_token.c_str());
		pFile->AddRequestHeaders(strtoken);
		pFile->AddRequestHeaders("Content-Type: application/json; charset=utf-8");
		char base64[1024] = { 0 };
		utility::base64_encode(data, base64, datalen);

		CString strrequest = "";
		strrequest.Format("\"%s\"", base64);

		pFile->SendRequestEx(strrequest.GetLength());
		pFile->WriteString(strrequest); //重要-->m_Request 中有"name=aaa&name2=BBB&..."  
		pFile->EndRequest();

		DWORD dwRet = 0;
		pFile->QueryInfoStatusCode(dwRet);//查询执行状态  
		printf("HTTP_STATUS_code:%d\n", dwRet);
		if (dwRet != HTTP_STATUS_OK) {//http请求执行失败  
			iret = -1;
			goto EXIST;
		}

		char szChars[1024 + 1] = { 0 };
		memset(szChars, 0, 1024 + 1);
		int nReaded = 0;
		int ialready = 0;
		while ((nReaded = pFile->Read((void*)szChars, 1024)) > 0)
		{
			memcpy(output + ialready, szChars, nReaded);
			ialready += nReaded;
			if (ialready > 256)
			{
				iret = -1;
				goto EXIST;
			}
		}		

	}
	catch (CInternetException *pEx)
	{
		CString tmp = "";
		tmp.Format("rasencrypt CInternetException err %d , %d\r\n", pEx->m_dwError, (pEx->m_dwContext));
		hslog::log(tmp.GetBuffer());;
		iret = -1;
		pEx->Delete();
	}
	catch (...)
	{
		iret = -1;
	}
EXIST:
	if (NULL != pFile)
	{
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}

	if (NULL != pServer)
	{
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	session.Close();
	return iret;
}

int  pophttpcomm::queryuserinfo()
{
	int iret = 0;
	CInternetSession session(NULL);
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	char *pAnsi = NULL;

	info.makedefault();

	///api/Products/PostUserInfo
	CString strrequest = "{\"Name\":null,\"Group\":null,\"DateTime\":null,\"Products\":null,\"Authorize\":null,\"Licenses\":null}";
	try
	{
		pServer = session.GetHttpConnection(m_ipaddress.c_str(), (INTERNET_PORT)m_port);

		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/api/Products/PostUserInfo"));
		CString strtoken;
		strtoken.Format("Authorization: Bearer %s", accse_token.c_str());
		pFile->AddRequestHeaders(strtoken);
		pFile->AddRequestHeaders("Content-Type: application/json;charset=UTF-8");
		pFile->AddRequestHeaders("charset=UTF-8");
		pFile->SendRequestEx(strrequest.GetLength());
		pFile->WriteString(strrequest); //重要-->m_Request 中有"name=aaa&name2=BBB&..."  
		pFile->EndRequest();

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);//查询执行状态  
		printf("HTTP_STATUS_code:%d\n", dwRet);
		if (dwRet != HTTP_STATUS_OK) {//http请求执行失败  
			iret = -1;
			goto EXIST;
		}

		char szChars[1024 + 1] = { 0 };
		CString strRawResponse = "";
		UINT nReaded = 0;
		while ((nReaded = pFile->Read((void*)szChars, 1024)) > 0)
		{
			szChars[nReaded] = '\0';
			strRawResponse += szChars;
			memset(szChars, 0, 1024 + 1);
		}
		
		pAnsi = utility::UTF8ToANSI(strRawResponse.GetBuffer());
		strRawResponse.ReleaseBuffer();

		Json::Reader reader;
		Json::Value root;
		Json::Value::Members mem = root.getMemberNames();
		if (reader.parse(pAnsi, root))  // reader将Json字符串解析到root，root将包含Json里所有子元素  
		{
			if (!root["Name"].isNull())
			{
				info.username = root["Name"].asCString();
			}
			if (!root["Group"].isNull())
			{
				info.group = strstr(root["Group"].asCString(), "|") + 1;
			}
			if (!root["DateTime"].isNull())
			{
				info.datatime = root["DateTime"].asCString();
			}
			if (!root["Products"].isNull())
			{
				int item_size = root["Products"].size();
				for (int i = 0; i < item_size; i++)
				{
					info.strproduct.push_back(root["Products"][i].asCString());
				}
			}
			if (!root["Authorize"].isNull())
			{
				Json::Value element;
				element = root["Authorize"];
				bool isarray = element.isArray();
				std::vector<string> v = root["Authorize"].getMemberNames();
				int item_size = root["Authorize"].size();
				for (int i = 0; i < item_size; i++)
				{
					Json::ValueType type = element[v[i]].type();
					Json::Value subelement = element[v[i]];
					std::vector<int > vitem;
					if (subelement.isArray())
					{
						for (unsigned int j = 0; j < subelement.size(); j++)
						{
							int tmp = subelement[j].asInt();
							vitem.push_back(tmp);
						}
					}

					info.authpermission.insert(map<string, vector<int>>::value_type(v[i], vitem));
				}
			}
			if (!root["Licenses"].isNull())
			{
				Json::Value element;
				element = root["Licenses"];
				std::vector<string> v = root["Licenses"].getMemberNames();
				for (size_t i = 0; i < v.size(); i++)
				{
					int itmp = element[v[i]].asInt();
					info.licensen.insert(map<string, int>::value_type(v[i], itmp));
				}
			}
		}
		else
		{
			iret = -1;
			goto EXIST;
		}
	}
	catch (CInternetException *pEx)
	{
		CString tmp = "";
		tmp.Format("queryuserinfo CInternetException err %d \r\n", pEx->m_dwError);
		hslog::log(tmp.GetBuffer());;
		iret = -1;
		pEx->Delete();
	}
	catch (...)
	{
		CString tmp = "queryuserinfo Exception \r\n";
		hslog::log(tmp.GetBuffer());
		iret = -1;
	}

EXIST:
	if (NULL != pFile)
	{
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}

	if (NULL != pServer)
	{
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}

	if (pAnsi != NULL)
	{
		free(pAnsi);
		pAnsi = NULL;
	}
	session.Close();
	return iret;
}

int  pophttpcomm::authrequest(unsigned char * data, int datalen, unsigned char * output, CString& errMessage)
{
	int iret = 0;

	CInternetSession session(NULL);
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	char szChars[1024 + 1] = { 0 };

	try
	{
		pServer = session.GetHttpConnection(m_ipaddress.c_str(), (INTERNET_PORT)m_port);

		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/api/Products/PostPosInfoT"));
		CString strtoken;
		strtoken.Format("Authorization: Bearer %s", accse_token.c_str());
		pFile->AddRequestHeaders(strtoken);
		pFile->AddRequestHeaders("Content-Type: application/json; charset=utf-8");
		char base64[1024] = { 0 };
		utility::base64_encode(data, base64, datalen);

		CString strrequest = "";
		strrequest.Format("\"%s\"", base64);

		pFile->SendRequestEx(strrequest.GetLength());
		pFile->WriteString(strrequest); //重要-->m_Request 中有"name=aaa&name2=BBB&..."  
		pFile->EndRequest();

		DWORD dwRet = 0;
		pFile->QueryInfoStatusCode(dwRet);		//查询执行状态  
	
		if (dwRet != HTTP_STATUS_OK)			//http请求执行失败
		{
			memset(szChars, 0, 1024 + 1);
			UINT nReaded = 0;
			CString tmpstr;
			while ((nReaded = pFile->Read((void*)szChars, 1024)) > 0)
			{
				szChars[nReaded] = '\0';
				tmpstr += szChars;
				memset(szChars, 0, 1024 + 1);
			}
			char* err = utility::UTF8ToANSI(tmpstr.GetBuffer());
			errMessage = err;
			if (err != NULL)
			{
				free(err);
				err = NULL;
			}
			iret = -1;
			goto EXIST;
		}
	
		memset(szChars, 0, 1024 + 1);
		int nReaded = 0;
		int ialready = 0;
		while ((nReaded = pFile->Read((void*)szChars, 1024)) > 0)
		{
			memcpy(output + ialready, szChars, nReaded);
			ialready += nReaded;
			if (ialready > 256)
			{
				iret = -1;
				goto EXIST;
			}
		}

	}
	catch (CInternetException *pEx)
	{
		CString tmp = "";
		tmp.Format("authrequest CInternetException err %d , %d\r\n", pEx->m_dwError, (pEx->m_dwContext));
		hslog::log(tmp.GetBuffer());
		OutputDebugStringA(tmp);
		iret = -1;
		pEx->Delete();
	}
	catch (...)
	{
		iret = -1;
	}
EXIST:
	if (NULL != pFile)
	{
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}

	if (NULL != pServer)
	{
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	session.Close();
	return iret;
}

void pophttpcomm::initialhttp(string ipaddress, int port)
{
	m_ipaddress = ipaddress;
	m_port = port;

	info.makedefault();
}

int  pophttpcomm::uploadproduceresult(string szresult, CString& errMessage)
{
	int iret = 0;

	CInternetSession session(NULL);
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	char szChars[1024 + 1] = { 0 };
	string sn = "";
	string imei = "";
	int  wholepass = 0;
	int  boilpass = 0;
	string deviceinfo = "";
	string wholetest = "";
	string boiltest = "";
	int npos1 = szresult.find("\n\r", 0);
	int npos2 = szresult.find("\n\r", npos1 + 2);
	int npos3 = szresult.find("\n\r", npos2 + 2);
	deviceinfo = szresult.substr(1, npos1 - 1);	
	wholetest = szresult.substr(npos1 + 2, npos2 - npos1 -2);
	boiltest = szresult.substr(npos2 + 2, szresult.size() - npos2 - 3);
	if (wholetest.find("wholeresult=1", 0) != string::npos)
		wholepass = 1; 
	else
		wholepass = 0;
	if (boiltest.find("boilresult=1", 0) != string::npos)
		boilpass = 1;
	else
		boilpass = 0;

	npos1 = deviceinfo.find("SN=", 0);
	npos2 = deviceinfo.find(";", npos1);
	sn = szresult.substr(npos1 + strlen("SN=") + 1, npos2 - npos1 - strlen("SN="));
	npos1 = deviceinfo.find("IMEI=", 0);
	npos2 = deviceinfo.find(";", npos1);
	imei = szresult.substr(npos1 + strlen("IMEI=") + 1, npos2 - npos1 - strlen("IMEI="));
	try
	{
		pServer = session.GetHttpConnection(m_ipaddress.c_str(), (INTERNET_PORT)m_port);

		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/api/Socket/PostTestInfo"));
		pFile->AddRequestHeaders("Content-Type: application/json; charset=utf-8");

		CString strrequest = "";
		strrequest.Format("{\"SN\":\"%s\",\"IMEI\":\"%s\",\"WholePass\":%d,\"BoilPass\":%d,\"WholeTest\":\"%s\",\"BoilTest\":\"%s\"}", 
			sn.c_str(), imei.c_str(), wholepass, boilpass, wholetest.c_str(), boiltest.c_str());

		pFile->SendRequestEx(strrequest.GetLength());
		pFile->WriteString(strrequest); //重要-->m_Request 中有"name=aaa&name2=BBB&..."  
		pFile->EndRequest();

		DWORD dwRet = 0;
		pFile->QueryInfoStatusCode(dwRet);		//查询执行状态  

		if (dwRet != HTTP_STATUS_OK)			//http请求执行失败
		{
			memset(szChars, 0, 1024 + 1);
			UINT nReaded = 0;
			CString tmpstr;
			while ((nReaded = pFile->Read((void*)szChars, 1024)) > 0)
			{
				szChars[nReaded] = '\0';
				tmpstr += szChars;
				memset(szChars, 0, 1024 + 1);
			}
			char* err = utility::UTF8ToANSI(tmpstr.GetBuffer());
			errMessage = err;
			if (err != NULL)
			{
				free(err);
				err = NULL;
			}
			iret = -1;
			goto EXIST;
		}		
	}
	catch (CInternetException *pEx)
	{
		CString tmp = "";
		tmp.Format("uploadproduceresult CInternetException err %d , %d\r\n", pEx->m_dwError, (pEx->m_dwContext));
		hslog::log(tmp.GetBuffer());;
		iret = -1;
		pEx->Delete();
	}
	catch (...)
	{
		iret = -1;
	}
EXIST:
	if (NULL != pFile)
	{
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}

	if (NULL != pServer)
	{
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	session.Close();
	return iret;
}


userinfo::userinfo()
{
	makedefault();
}

userinfo::~userinfo()
{

}

userinfo::userinfo(userinfo &rvalue)
{
	makedefault();

	this->username = rvalue.username;
	this->group = rvalue.group;
	this->datatime = rvalue.datatime;
	this->strproduct = rvalue.strproduct;
	this->licensen = rvalue.licensen;
	this->authpermission = rvalue.authpermission;
}


