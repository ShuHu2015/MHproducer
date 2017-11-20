#include "StdAfx.h"
#include "HttpFunc.h"


httpfunc::httpfunc(void)
{
	
}

httpfunc::httpfunc(CString baseurl, int port)
{
	
}

httpfunc::~httpfunc(void)
{
}


int httpfunc::postrequest(CString url, CString strRequest)
{
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	CInternetSession session(NULL);
	try
	{
		pServer = session.GetHttpConnection(m_baseurl, (INTERNET_PORT)m_port);

		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, url);

		pFile->SendRequestEx(strRequest.GetLength());
		pFile->WriteString(strRequest); 
		pFile->EndRequest();


	}
	catch(...)
	{
	}
	return 0;
}