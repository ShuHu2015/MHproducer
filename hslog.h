#pragma once
class hslog
{
public:
	hslog(void);
	~hslog(void);

public:
	static HANDLE mutex;
	static bool m_islog;
	static char szlogfile[256];
	static void isprintlog(int islog);
	static void log(char* str);
	static void startlog();
	static void loghex(unsigned char* pbuf, int len);
	static void loghandler(void* args);
	static void stoplog();
};

