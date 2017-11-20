#pragma once


class ShThread
{
public:
	ShThread();
	virtual DWORD Run();
	bool Start();
	bool Join(DWORD nMillSec = 200);

private:
	static DWORD WINAPI RunThread(LPVOID pParam);
	HANDLE m_hThread;
	LPVOID m_pParam;
	DWORD  m_nRet;
	bool   m_bStart;
};



