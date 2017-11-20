#include "stdafx.h"
#include "ShThread.h"

ShThread::ShThread()
{
	m_hThread = NULL;
	m_pParam = NULL;
	m_nRet = 0;
	m_bStart = false;
}

DWORD ShThread::Run()
{
	return 0;
}

bool ShThread::Start()
{
	m_bStart = true;
	m_hThread = CreateThread(NULL, 0, &RunThread, this, 0, NULL);

	if (!m_hThread)
	{
		m_bStart = false;
		return false;
	}
	SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);
	return true;
}

bool ShThread::Join(DWORD nMillSec)
{
	while (m_bStart)
		Sleep(nMillSec);

	if (FALSE == GetExitCodeThread(m_hThread, &m_nRet))
		return false;
	else
	{
		CloseHandle(m_hThread);
		return true;
	}
}

DWORD WINAPI ShThread::RunThread(LPVOID pParam)
{
	ShThread* pThis = (ShThread*)pParam;
	DWORD nRet = pThis->Run();
	pThis->m_bStart = false;
	return nRet;
}