#pragma once
#include <afxinet.h>

class httpfunc
{
public:
	httpfunc(void);
	~httpfunc(void);
	httpfunc(CString url, int port);

	int logon(CString username, CString password);

private:
	BOOL m_bconnect;
	CString m_baseurl;
	int m_port;

	CString access_token;

private:
	int settoken();
	int postrequest(CString url, CString strRequest);
};

