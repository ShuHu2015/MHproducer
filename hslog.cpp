#include "StdAfx.h"
#include "hslog.h"

bool hslog::m_islog = true;
HANDLE hslog::mutex = CreateMutex(NULL, FALSE, "LOG");
char hslog::szlogfile[256] = { 0 };

hslog::hslog(void)
{
}

hslog::~hslog(void)
{
}

void hslog::isprintlog(int islog)
{
	if (islog == 1)
		m_islog = true;
	else
		m_islog = false;
}

void hslog::log(char* str)
{
	if (!m_islog)
		return;
	WaitForSingleObject(mutex, INFINITE);
	SYSTEMTIME st;  
	GetLocalTime(&st);
	char filename[256] = {0};
	sprintf(filename, "log_%d%d%d.txt", st.wYear, st.wMonth, st.wDay);
	FILE *fp;  
	fp=fopen(filename,"at");  
	fprintf(fp,"log info: %d:%d:%d:%d ",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);  
	fprintf(fp,str);  
	fclose(fp);  
	OutputDebugStringA(str); 
	ReleaseMutex(mutex);
}

void hslog::loghex(unsigned char* pbuf, int len)
{
	if (!m_islog)
		return;
	WaitForSingleObject(mutex, INFINITE);
	SYSTEMTIME st;
	GetLocalTime(&st);
	char filename[256] = { 0 };
	sprintf(filename, "log_%d%d%d.txt", st.wYear, st.wMonth, st.wDay);
	FILE *fp;
	fp = fopen(filename, "at");

	if (fp != NULL)
	{
		fprintf(fp, "%s:", "hex:");
		while (len--)
		{
			fprintf(fp, "%02X ", *pbuf++);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
	ReleaseMutex(mutex);
}

void hslog::loghandler(void* args)
{
		
}

void hslog::startlog()
{
	m_islog = true;
}

void hslog::stoplog()
{
	m_islog = false;
}