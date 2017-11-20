#include "stdafx.h"
#include "mhservice.h"
#include "ctb-0.16\win32\serport.h"
#include "hslog.h"
#include "CRC.h"
#include "utility.h"
#include "sha512.h"
#include "newsha256.h"
#include "hex.h"
#include "pophttpcomm.h"



extern pophttpcomm  g_signpop;
extern pophttpcomm  g_authpop;

bool mhservice::m_allstop = true;

int	 GetFileLine(FILE* pfile, char * buf);
int  StrToRSA(char* str, unsigned char* rsa);

mhservice::mhservice()
{
	m_resetcmd = 0;
	m_portconvert = 0;
}

mhservice::~mhservice()
{
	m_allstop = true;
}


bool  PRODUCETASK::operator ==(const PRODUCETASK & rhs)
{
	if (this->m_devname == rhs.m_devname)
		return true;
	return false;
}

int   mhservice::initialservice(int inum)
{
	m_seg = CreateSemaphore(NULL, 0, inum, "");
	if (m_seg == INVALID_HANDLE_VALUE)
		return -1;

	for (int i = 0; i < inum; i++)
	{
		HANDLE hthred = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&_mhthreadproc, this, 0, NULL);
		SetThreadPriority(hthred, THREAD_PRIORITY_HIGHEST);
	}

	return 0;
}

DWORD mhservice::_mhthreadproc(LPVOID lpParam)
{
	mhservice *pThis = (mhservice *)lpParam;
	pThis->mhprocess();
	return 0;
}

bool  mhservice::ishandlingtask(CString devname)
{
	list <CString>::iterator iteror;
	for (iteror = mMidTask.begin(); iteror != mMidTask.end(); iteror++)		//确认是否任务添加成功
	{
		CString tmp = *iteror;
		if (tmp == devname)
		{
			return true;
		}
	}
	return false;
}

int	  mhservice::addhandlingtask(CString param)
{
	m_queuemutex.Lock();
	list <CString>::iterator iteror;
	for(iteror = mMidTask.begin(); iteror != mMidTask.end(); iteror++)		//确认是否任务添加成功
	{
		CString tmp = *iteror;
		if (tmp == param)
		{
			m_queuemutex.Unlock();
			return -1;
		}
	}

	mMidTask.push_back(param);
	m_queuemutex.Unlock();
	return 0;
}

int   mhservice::addtask(TASKDATA param)
{
	m_queuemutex.Lock();
	mTaskQueue.push(param);
	m_queuemutex.Unlock();
	ReleaseSemaphore(m_seg, 1, NULL);
	return 0;
}

int   mhservice::removehandlingtask(CString param)
{
	m_queuemutex.Lock();
	list <CString>::iterator iteror;
	for (iteror = mMidTask.begin(); iteror != mMidTask.end(); iteror++)		//确认是否任务添加成功
	{
		CString tmp = *iteror;
		if (tmp == param)
		{
			mMidTask.erase(iteror);
			m_queuemutex.Unlock();
			return 0;
		}
	}
	m_queuemutex.Unlock();
	return -1;
}

int   mhservice::gettask(TASKDATA* Pparam)
{
	m_queuemutex.Lock();
	if (mTaskQueue.size() <= 0)
	{
		m_queuemutex.Unlock();
		return -1;
	}

	TASKDATA& tmp = mTaskQueue.front();
	memcpy(Pparam, &tmp, sizeof(TASKDATA));
	mTaskQueue.pop();
	m_queuemutex.Unlock();
	return 0;
}     
      
int   mhservice::mhprocess()
{
	while (true)
	{
		WaitForSingleObject(m_seg, INFINITE);

		TASKDATA param = { 0 };

		if (gettask(&param) == 0)
		{
			internalprocess(param);
		}
	}
	return 0;
}

/*
			  任务task
				  |
		|         |        |
   下载task    授权task   签名task    
*/	
int   mhservice::internalprocess(TASKDATA param)
{	
	mhcommethod mhworkhandle;
	
	switch (param.tasktype)
	{
	case TASK_POPLOAD:		//下载任务
	{
		if (m_portconvert)
		{
			throughcom0();
		}
		int iRet = mhworkhandle.open(param.szcom);
		if (iRet != 0)
		{
			SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"OPEN COM ERROR");
			SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, 0);			//tell the main window one task has been handled.
			return 0;
		}
		if (m_resetcmd)
		{
			if (mhworkhandle.pop_osreset() != 0)
			{
				SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"RESTART ERROR");
				SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, 0);			//tell the main window one task has been handled.
				return 0;
			}
		}
		loadtaskhandler(&mhworkhandle, param);
		break;
	}
	case TASK_AUTH:			//授权任务
	{
		if (m_portconvert)
		{
			throughcom0();
		}
		int iRet = mhworkhandle.open(param.szcom);
		if (iRet != 0)
		{
			SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"OPEN COM ERROR");
			SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, 0);			//tell the main window one task has been handled.
			return 0;
		}
		if (m_resetcmd)
		{
			if (mhworkhandle.pop_osreset() != 0)
			{
				SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"RESTART ERROR");
				SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, 0);			//tell the main window one task has been handled.
				return 0;
			}
		}
		authtaskhandler(&mhworkhandle, param);
		break;
	}
	case TASK_SIGN:			//签名任务
	{
		signtaskhandler(&mhworkhandle, param);
		break;
	}
	default:
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"UNKOWN ERROR");
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, 0);			//tell the main window one task has been handled.
		break;
	}
	}
	return 0;
}

int   mhservice::authtaskhandler(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	unsigned char chcode[256] = { 0 };
	unsigned char ucfromserver[256] = { 0 };
	char info[1024*10] = { 0 };
	USER_INFO userinfo = { 0 };
	CString errmessage;

	iRet = mhworkhandle->pop_handshake(param.ack, 30000);
	if (iRet != 0)
	{

		SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, (LPARAM)"HANDLE SHAKE ERROR");
		goto EXIST;
	}	
	
	memcpy(&userinfo, param.szFilePath, sizeof(USER_INFO));

	iRet = mhworkhandle->pop_getchallengecode(userinfo, chcode);
	if (iRet != 0)
	{
		memset(info, 0, sizeof(info));
		sprintf(info, "GET DEVICE INFO ERROR, err=%d ！", iRet);
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, (LPARAM)info);
		goto EXIST;
	}
	else
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"GET INFO，PLEASE WAIT...");
	}

	
	iRet = g_authpop.authrequest(chcode, 256, ucfromserver, errmessage);
	if (iRet != 0)
	{
		memset(info, 0, sizeof(info));
		//sprintf(info, "SERVER RETURN ERROR,%s！", errmessage.GetBuffer());
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, (LPARAM)errmessage.GetBuffer());
		goto EXIST;
	}
	else
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"GET RESPONSE，PLEASE WAIT...");
	}
	
	iRet = mhworkhandle->pop_sendlicsence(ucfromserver, 256, param.id);	
	if (iRet != 0)
	{
		memset(info, 0, sizeof(info));
		sprintf(info, "LICENSE ERROR，err=%d！", iRet);
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, (LPARAM)info);
		goto EXIST;
	}

EXIST:
	if (iRet == 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, (LPARAM)"AUTH SUCCESS！");
	}
	return 0;
}

int   mhservice::signtaskhandler(mhcommethod* mhworkhandle, TASKDATA param)
{
	if (param.cmdl == FILETYPE::BOOT)
	{
		signboot(mhworkhandle, param);
	}
	else if (param.cmdl == FILETYPE::PUK)
	{
		signpuk(mhworkhandle, param);
	}
	else
	{
		signnormalfile(mhworkhandle, param);
	}
	return 0;	
}

//正常的签名步骤
int   mhservice::signnormalfile(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	SIGN_INFO stsigninfo;
	CFile filesource, filesigned;
	CString szfilename, errMsg;
	CString m_filepath = param.szFilePath;
	char *filebuf = NULL;
	int filelen = 0;

	memset(&stsigninfo, 0, sizeof(stsigninfo));
	memcpy(stsigninfo.magic, SIGN_MAGIC, sizeof(stsigninfo.magic));
	stsigninfo.HashAlg = 0x00;
	stsigninfo.SignVer[0] = 0x30;
	stsigninfo.SignVer[1] = 0x31;
	memcpy(stsigninfo.SignCompany, g_signpop.info.group.c_str(), sizeof(stsigninfo.SignCompany) > g_signpop.info.group.size() ? g_signpop.info.group.size() : sizeof(stsigninfo.SignCompany));
	memcpy(stsigninfo.SignUser, g_signpop.info.username.c_str(), sizeof(stsigninfo.SignUser) > g_signpop.info.username.size() ? g_signpop.info.username.size() : sizeof(stsigninfo.SignUser));
	CTime time = CTime::GetCurrentTime();
	int iTmp = time.GetYear();
	iTmp %= 100;
	stsigninfo.SignTime[0] = iTmp / 10 + 0x30;
	stsigninfo.SignTime[1] = iTmp % 10 + 0x30;
	iTmp = time.GetMonth();
	stsigninfo.SignTime[2] = iTmp / 10 + 0x30;
	stsigninfo.SignTime[3] = iTmp % 10 + 0x30;
	iTmp = time.GetDay();
	stsigninfo.SignTime[4] = iTmp / 10 + 0x30;
	stsigninfo.SignTime[5] = iTmp % 10 + 0x30;
	iTmp = time.GetHour();
	stsigninfo.SignTime[6] = iTmp / 10 + 0x30;
	stsigninfo.SignTime[7] = iTmp % 10 + 0x30;
	memcpy(stsigninfo.SignInvalidDate, stsigninfo.SignTime, sizeof(stsigninfo.SignInvalidDate));
	stsigninfo.SignInvalidDate[0] += 2;

	char info[] = "LOADING...";
	SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);

	if (!filesource.Open(m_filepath, CFile::modeRead))
	{
		AfxMessageBox(_T("FILE NOT EXIST ERROR"));
		goto EXIST;
	}

	filelen = (int)filesource.GetLength();
	filebuf = (char*)malloc(filelen);

	iRet = filesource.Read(filebuf, filelen);
	if (iRet != filelen)
	{
		AfxMessageBox(_T("FILE READ ERROR"));
		goto EXIST;
	}
	filesource.Close();

	SHA256Context foo;
	SHA256Init(&foo);
	SHA256Update(&foo, filebuf, filelen);
	SHA256Final(&foo, (unsigned char *)stsigninfo.Hash);

	unsigned char entext[256] = { 0 };

	char info1[] = "ENCRYPT...";
	SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info1);

	iRet = g_signpop.rasencrypt(param.szpukpath, (unsigned char*)&stsigninfo, 256, entext, errMsg);	//加密

	if (iRet != 0) {
		AfxMessageBox(errMsg);
		goto EXIST;
	}

	if (m_filepath.ReverseFind('.') < 0)
	{
		szfilename = m_filepath;
	}
	else
	{
		szfilename = m_filepath.Left(m_filepath.ReverseFind('.'));
	}
	szfilename += _T("_sign.bin");

	if (!filesigned.Open(szfilename, CFile::modeReadWrite | CFile::modeCreate))
	{
		AfxMessageBox(_T("FILE OPEN ERROR"));
		iRet = -1;
		goto EXIST;
	}

	filesigned.Write(filebuf, filelen);
	filesigned.Write(entext, 256);
	filesigned.Close();
	iRet = 0;
EXIST:
	if (filebuf)
	{
		free(filebuf);
		filebuf = NULL;
	}
	if (iRet == 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_RESULT, 1);
	}
	else
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_RESULT, 0);
	}
	return iRet;

}

//针对boot不同的签名方式
int   mhservice::signboot(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	CFile filesigned;
	CString szfilename, errMsg;
	CString m_filepath = param.szFilePath;
	FileInfo *pInfo = (FileInfo *)malloc(sizeof(FileInfo));
	memset(pInfo, 0, sizeof(FileInfo));

	pInfo->info.param.refresh = 0;			//不更新扰码
	pInfo->info.param.valid = FILE_VALID_FLAG;						//固定为0x5555AAAA
	char info[] = "LODING...";
	SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);

	iRet = mhworkhandle->ConvertFile(pInfo, param.szFilePath, 0x1001000, HASH_TYPE_256, (short)0xFFFF);	//设置需要加密部分数据

	if (iRet != 0) {
		AfxMessageBox(_T("ConvertFile ERROR"));
		goto EXIST;
	}

	unsigned char entext[256] = { 0 };

	char info1[] = "ENCRYPT...";
	SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info1);

	iRet = g_signpop.rasencrypt(param.szpukpath, (unsigned char*)&pInfo->info.param.sig.s.reserved, 256, entext, errMsg);	//加密

	if (iRet != 0) {
		AfxMessageBox(errMsg);
		goto EXIST;
	}

	memcpy((unsigned char*)&pInfo->info.param.sig.s.reserved, entext, sizeof(entext));


	if (m_filepath.ReverseFind('.') < 0)
	{
		szfilename = m_filepath;
	}
	else
	{
		szfilename = m_filepath.Left(m_filepath.ReverseFind('.'));
	}
	szfilename += _T("_sign.bin");

	if (!filesigned.Open(szfilename, CFile::modeReadWrite | CFile::modeCreate))
	{
		AfxMessageBox(_T("FILE OPEN ERROR"));
		iRet = -1;
		goto EXIST;
	}

	filesigned.Write("MHSIG_B", 7);
	filesigned.Write(&(pInfo->startAddr), 8);
	filesigned.Write(pInfo->info.v, sizeof(pInfo->info.v));
	filesigned.Write(pInfo->buf, pInfo->fileLen);

	iRet = 0;
EXIST:
	if (NULL != pInfo)
	{
		free(pInfo);
		pInfo = NULL;
	}

	if (iRet == 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_RESULT, 1);
	}
	else
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_RESULT, 0);
	}
	return iRet;
}

//针对PUK不同的签名方式
int   mhservice::signpuk(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	SIGN_INFO stsigninfo;
	CFile filesource, filesigned;
	CString szfilename, errMsg;
	CString m_filepath = param.szFilePath;
	char *filebuf = NULL;
	int filelen = 0;

	memset(&stsigninfo, 0, sizeof(stsigninfo));
	memcpy(stsigninfo.magic, SIGN_MAGIC, sizeof(stsigninfo.magic));
	stsigninfo.HashAlg = 0x00;
	stsigninfo.SignVer[0] = 0x30;
	stsigninfo.SignVer[1] = 0x31;
	memcpy(stsigninfo.SignCompany, g_signpop.info.group.c_str(), sizeof(stsigninfo.SignCompany) > g_signpop.info.group.size() ? g_signpop.info.group.size() : sizeof(stsigninfo.SignCompany));
	memcpy(stsigninfo.SignUser, g_signpop.info.username.c_str(), sizeof(stsigninfo.SignUser) > g_signpop.info.username.size() ? g_signpop.info.username.size() : sizeof(stsigninfo.SignUser));
	CTime time = CTime::GetCurrentTime();
	int iTmp = time.GetYear();
	iTmp %= 100;
	stsigninfo.SignTime[0] = iTmp / 10 + 0x30;
	stsigninfo.SignTime[1] = iTmp % 10 + 0x30;
	iTmp = time.GetMonth();
	stsigninfo.SignTime[2] = iTmp / 10 + 0x30;
	stsigninfo.SignTime[3] = iTmp % 10 + 0x30;
	iTmp = time.GetDay();
	stsigninfo.SignTime[4] = iTmp / 10 + 0x30;
	stsigninfo.SignTime[5] = iTmp % 10 + 0x30;
	iTmp = time.GetHour();
	stsigninfo.SignTime[6] = iTmp / 10 + 0x30;
	stsigninfo.SignTime[7] = iTmp % 10 + 0x30;
	memcpy(stsigninfo.SignInvalidDate, stsigninfo.SignTime, sizeof(stsigninfo.SignInvalidDate));
	stsigninfo.SignInvalidDate[0] += 2;

	char info[] = "LOADING...";
	SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);

	if (!filesource.Open(m_filepath, CFile::modeRead))
	{
		AfxMessageBox(_T("FILE NOT EXIST ERROR"));
		goto EXIST;
	}

	filelen = (int)filesource.GetLength();
	filebuf = (char*)malloc(filelen);

	iRet = filesource.Read(filebuf, filelen);
	if (iRet != filelen || filelen < sizeof(ST_HEADER_INFO))
	{
		AfxMessageBox(_T("FILE READ ERROR"));
		goto EXIST;
	}
	filesource.Close();

	SHA256Context foo;
	SHA256Init(&foo);
	SHA256Update(&foo, filebuf + sizeof(ST_HEADER_INFO), filelen - sizeof(ST_HEADER_INFO));
	SHA256Final(&foo, (unsigned char *)stsigninfo.Hash);

	unsigned char entext[256] = { 0 };

	char info1[] = "ENCRYPT...";
	SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info1);

	iRet = g_signpop.rasencrypt(param.szpukpath, (unsigned char*)&stsigninfo, 256, entext, errMsg);	//加密

	if (iRet != 0) {
		AfxMessageBox(errMsg);
		goto EXIST;
	}

	if (m_filepath.ReverseFind('.') < 0)
	{
		szfilename = m_filepath;
	}
	else
	{
		szfilename = m_filepath.Left(m_filepath.ReverseFind('.'));
	}
	szfilename += _T("_sign.bin");

	if (!filesigned.Open(szfilename, CFile::modeReadWrite | CFile::modeCreate))
	{
		AfxMessageBox(_T("FILE OPEN ERROR"));
		iRet = -1;
		goto EXIST;
	}

	filesigned.Write(filebuf, filelen);
	filesigned.Write(entext, 256);
	filesigned.Close();
	iRet = 0;
EXIST:
	if (filebuf)
	{
		free(filebuf);
		filebuf = NULL;
	}
	if (iRet == 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_RESULT, 1);
	}
	else
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_RESULT, 0);
	}
	return iRet;
}

int   mhservice::loadtaskhandler(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	char info[1024] = { 0 };
	

	iRet = mhworkhandle->pop_handshake(param.ack, POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"HAND SHAKE ERROR");
		goto EXIST;
	}

	switch (param.cmdh)
	{
	case CMD_BOOT:
	{
		iRet = bootcmdhandler(mhworkhandle, param);
		break;
	}
	case CMD_LVOS:
	{
		iRet = lvoscmdhandler(mhworkhandle, param);
		break;
	}
	case OS_FLASHOPERA:
	{
		iRet = flashutf8table(mhworkhandle, param);
		break;
	}
	case OS_FONT:
	{
		iRet = flashfonttable(mhworkhandle, param);
		break;
	}
	case OS_BKIMG:
	{
		iRet = flashbkimg(mhworkhandle, param);
		break;
	}
	default:
		iRet = NOTSUPPORT_ERR;
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"NOT SUPPORT");			//tell some infomation, and end this task
		goto EXIST;
	}

EXIST:

	SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, 0);			//tell the main window one task has been handled.
	return iRet;
}

int   mhservice::bootcmdhandler(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	RESPPACK pack = { 0 };
	
	switch (param.cmdl)
	{
	case BOOT:
	{
		iRet = mhworkhandle->pop_bootdownload(param);
		break;
	}
	case LVOS:
	{
		iRet = mhworkhandle->pop_osdownload(param);
		break;
	}
	case SN1STR:
	{
		iRet = mhworkhandle->pop_sn1downlaod(param);
		if(iRet == 0)
			SendMessage(param.hwnd, WM_MESSAGEINFO, P_PROGRESS, 100);
		break;
	}
	case SN2STR:
	{
		iRet = mhworkhandle->pop_sn2downlaod(param);
		if (iRet == 0)
			SendMessage(param.hwnd, WM_MESSAGEINFO, P_PROGRESS, 100);
		break;
	}
	case PUK:
	{
		iRet = mhworkhandle->pop_pukdownlaod(param);
		break;
	}	
	case CONFIG:
	{
		iRet = mhworkhandle->pop_configdownlaod(param.szFilePath);
		if (iRet == 0)
			SendMessage(param.hwnd, WM_MESSAGEINFO, P_PROGRESS, 100);
		break;
	}
	default:
		iRet = NOTSUPPORT_ERR;
		goto EXIST;
	}

	GetFileTypeFromId(param.cmdl, pack.buf.devname);
	if (iRet == 0)											   //tell the result of a handling task  
	{
		sprintf(pack.buf.reserve, "SUCESS");
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_RESULT, (LPARAM)&pack);
	}
	else
	{
		sprintf(pack.buf.reserve, "FAIL %d", iRet);
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_RESULT, (LPARAM)&pack);
	}

EXIST:
	return iRet;
}

int   mhservice::lvoscmdhandler(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	CString result;
	iRet = mhworkhandle->pop_getdeviceinfo(result);

	SendMessage(param.hwnd, WM_MESSAGEINFO, P_QUERY, (LPARAM)result.GetBuffer());
	SendMessage(param.hwnd, WM_MESSAGEINFO, P_END, 0);			//tell the main window one task has been handled.
EXIST:
	return iRet;
}

int   mhservice::flashutf8table(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	unsigned char* oembuf = NULL;
	unsigned char* utfbuf = NULL;
	CFile utffile;
	CFile oemfile;
	int filelen = 0;
	char info[256] = { 0 };

	iRet  = mhworkhandle->eraseblockflash(0x00000, 192 * 1024);
	if (iRet != 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"FLASH ERASE FAIL");
		goto EXIST;
	}
	iRet = mhworkhandle->pop_handshake(param.ack, POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"HAND SHAKE ERROR");
		goto EXIST;
	}

	if (!oemfile.Open(param.szpukpath, CFile::modeRead))
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"OEM2UNI NOT EXIST");
		goto EXIST;
	}
	filelen = (int)oemfile.GetLength();
	oemfile.SeekToBegin();
	oembuf = (unsigned char*)malloc(filelen);
	if (oemfile.Read(oembuf, filelen) != filelen)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"FILE READ ERROR");
		goto EXIST;
	}
	iRet = mhworkhandle->writeflash(param.hwnd, 0x00000, oembuf, filelen);
	if (iRet != 0)
	{
		sprintf(info, "WRITE OEM2UNI FAIL %d", iRet);
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);
		goto EXIST;
	}

	iRet = mhworkhandle->pop_handshake(param.ack, POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"HAND SHAKE ERROR");
		goto EXIST;
	}

	filelen = 0;
	
	if (!utffile.Open(param.szFilePath, CFile::modeRead))
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"UNI2OEM NOT EXIST");
		goto EXIST;
	}
	filelen = (int)utffile.GetLength();
	utffile.SeekToBegin();
	utfbuf = (unsigned char*)malloc(filelen);
	if (utffile.Read(utfbuf, filelen) != filelen)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"File Read Error");
		goto EXIST;
	}
	iRet = mhworkhandle->writeflash(param.hwnd, 0x18000, utfbuf, filelen);
	if (iRet != 0)
	{
		sprintf(info, "写入UNI2OEM码表失败 %d", iRet);
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);
		goto EXIST;
	}

	SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"UNI2OEM SUCCESS");
EXIST:
	if (oembuf != NULL)
	{
		free(oembuf);
		oembuf = NULL;
	}
	if (utfbuf != NULL)
	{
		free(utfbuf);
		utfbuf = NULL;
	}
	return iRet;
}

int   mhservice::flashfonttable(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	CFile utffile;
	unsigned char* utfbuf = NULL;
	int filelen = 0;
	char info[256] = { 0 };

	iRet = mhworkhandle->eraseblockflash(0x30000, 576 * 1024);
	if (iRet != 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"FLASH ERASE FAIL");
		goto EXIST;
	}

	iRet = mhworkhandle->pop_handshake(param.ack, POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"HAND SHAKE ERROR");
		goto EXIST;
	}

	filelen = 0;

	if (!utffile.Open(param.szFilePath, CFile::modeRead))
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"FONTLIB NOT EXIST");
		goto EXIST;
	}
	filelen = (int)utffile.GetLength();
	utffile.SeekToBegin();
	utfbuf = (unsigned char*)malloc(filelen);
	if (utffile.Read(utfbuf, filelen) != filelen)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"File Read Error");
		goto EXIST;
	}
	iRet = mhworkhandle->writeflash(param.hwnd, 0x30000, utfbuf, filelen);
	if (iRet != 0)
	{
		sprintf(info, "写入字库失败 %d", iRet);
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);
		goto EXIST;
	}

	SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"FONTLIB SUCCESS");
EXIST:

	if (utfbuf != NULL)
	{
		free(utfbuf);
		utfbuf = NULL;
	}
	return iRet;
}

int   mhservice::flashbkimg(mhcommethod* mhworkhandle, TASKDATA param)
{
	int iRet = 0;
	CFile utffile;
	unsigned char* utfbuf = NULL;
	int filelen = 0;
	char info[256] = { 0 };

	iRet = mhworkhandle->eraseblockflash(0x1D0000, 192 * 1024);
	if (iRet != 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"FLASH ERASE ERROR");
		goto EXIST;
	}

	iRet = mhworkhandle->pop_handshake(param.ack, POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"HAND SHAKE ERROR");
		goto EXIST;
	}

	filelen = 0;

	if (!utffile.Open(param.szFilePath, CFile::modeRead))
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"background img not exist");
		goto EXIST;
	}
	filelen = (int)utffile.GetLength();
	utffile.SeekToBegin();
	utfbuf = (unsigned char*)malloc(filelen);
	if (utffile.Read(utfbuf, filelen) != filelen)
	{
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"file read error");
		goto EXIST;
	}
	iRet = mhworkhandle->writeflash(param.hwnd, 0x1D0000, utfbuf, filelen);
	if (iRet != 0)
	{
		sprintf(info, "write flash error %d", iRet);
		SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);
		goto EXIST;
	}

	SendMessage(param.hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"background img success");
EXIST:

	if (utfbuf != NULL)
	{
		free(utfbuf);
		utfbuf = NULL;
	}
	return iRet;
}

//将SPRD AT串口转换为SP通讯串口
void  mhservice::throughcom0()
{
	DWORD iexit = 0;
	USES_CONVERSION;

	CString batpath = "";
	GetModuleFileName(NULL, batpath.GetBuffer(MAX_PATH), MAX_PATH);
	batpath.ReleaseBuffer();
	int pos = batpath.ReverseFind(_T('\\'));
	batpath = batpath.Left(pos);

	SHELLEXECUTEINFO   ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = "TransferAPSP.bat";
	ShExecInfo.lpParameters = NULL;
	ShExecInfo.lpDirectory = batpath;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);

	// 等待脚本返回  
	WaitForSingleObject(ShExecInfo.hProcess, 3000);

	GetExitCodeProcess(ShExecInfo.hProcess, &iexit);
	if (iexit == STILL_ACTIVE)
	{
		HWND hWnd = ::FindWindow(_T("ConsoleWindowClass"), NULL);
		if (hWnd)
		{
			::PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
	}
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	Sleep(100);
}

///////////////////////////   mhcommethod defines   ////////////////////////////////////////////////////////

mhcommethod::mhcommethod()
{
	m_serialno = 0;
	m_packet = (PacketBuf*)malloc(sizeof(PacketBuf));
	memset(m_packet, 0, sizeof(PacketBuf));
}

mhcommethod::mhcommethod(const mhcommethod& rhs)
{
	m_packet = (PacketBuf*)malloc(sizeof(PacketBuf));
	memset(m_packet, 0, sizeof(PacketBuf));

	m_packet->len = rhs.m_packet->len;
	memcpy(m_packet->buf, rhs.m_packet->buf, m_packet->len);
}

mhcommethod::~mhcommethod()
{
	if (m_packet)
	{
		free(m_packet);
		m_packet = NULL;
	}

	if (m_Port.IsOpen())
	{
		m_Port.Close();
	}
}

int  mhcommethod::open(char* szportname)
{
	std::string strtmp = "";
	int baudrate = 115200;
	if (utility::loadconfiginfo("COMBAUDRATE", strtmp) == 0)
	{
		baudrate = atoi(strtmp.c_str());
	}

	if (m_Port.Open(szportname, baudrate, "8N1", ctb::SerialPort::NoFlowControl) != 0)
	{
		Sleep(2000);
		return m_Port.Open(szportname, baudrate, "8N1", ctb::SerialPort::NoFlowControl);
	}

	return 0;
}

int  mhcommethod::ReceivePacket()
{
	int i;
	int len = 0;

	Packet *pPkt = (Packet *)m_packet->buf;
	//m_packet->len = 0;

	while (!mhservice::m_allstop && m_Port.Readv((char*)m_packet->buf + m_packet->len, 1, 1) > 0)
	{
		if (m_packet->len == 0)
		{
			if (m_packet->buf[0] == DIR_UP)
			{
				m_packet->len++;
			}
		}
		else
		{
			++m_packet->len;
			while (m_packet->len >= MIN_PACKET_LEN)
			{
				if (pPkt->len < (MIN_PACKET_LEN - 2) ||
					pPkt->len >MAX_PACKET_LEN ||
					pPkt->len + 2 < m_packet->len)
				{
					for (i = 0; i < m_packet->len; i++)
					{
						if (m_packet->buf[i] == DIR_UP)
						{
							memmove(m_packet->buf, m_packet->buf + i, m_packet->len - i);
							break;
						}
					}
					m_packet->len -= i;
				}
				else if (pPkt->len + 2 == m_packet->len)
				{
					if (CRC::crc16((unsigned char *)m_packet->buf, pPkt->len, 0xFFFF) == *(unsigned short*)(m_packet->buf + pPkt->len))
					{
						hslog::loghex((unsigned char *)m_packet->buf, m_packet->len);
						m_packet->len = 0;						
						return 0;
					}
					else
					{
						for (i = 0; i < m_packet->len; i++)
						{
							if (m_packet->buf[i] == DIR_UP)
							{
								memmove(m_packet->buf, m_packet->buf + i, m_packet->len - i);
								break;
							}
						}
						m_packet->len -= i;
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	return -1;
}

void mhcommethod::pop_buildpacket(void* packet, unsigned char cmdh, unsigned char cmdl, unsigned char serialno, void* data, int datalen)
{
	OBEX* pobex = (OBEX*)packet;
	pobex->stx = SOH;
	pobex->cmdh = cmdh;
	pobex->cmdl = cmdl;
	pobex->serialno = serialno;
	pobex->len[0] = datalen >> 8 & 0xFF;
	pobex->len[1] = datalen & 0xFF;
	memcpy(pobex->data, data, datalen);

	pobex->data[datalen] = lrc((unsigned char*)packet + 1, datalen + 5);

}

void mhcommethod::pop_sendpacket(void *packet, int len)
{
	m_Port.Writev((char* )packet, len, PACKETSENDTIMEOUT);
	hslog::loghex((unsigned char *)packet, len);
}

int  mhcommethod::pop_receivepacket(DWORD timeout)
{
	OBEX* recvbuf = (OBEX*)m_packet->buf;
	m_packet->len = 0;
	//memset(m_packet, 0, sizeof(PacketBuf));		//每次都清空，或者直接 m_packet->len = 0;
	
	int tick = GetCurrentTime();
	while (!mhservice::m_allstop )
	{
		if (GetDiffTime(tick) > PACKETTIMEOUT)
		{
			return COMMUFAIL_ERR;
		}

		if (m_Port.Readv((char*)recvbuf, 1, timeout) != 1 || recvbuf->stx != SOH)
			continue;
		m_packet->len++;

		if (m_Port.Readv((char*)recvbuf + m_packet->len, 5, timeout) != 5)
			return COMMUFAIL_ERR;

		m_packet->len += 5;

		int datalen = (recvbuf->len[1] | (recvbuf->len[0] << 8));

		if (m_Port.Readv((char*)recvbuf + m_packet->len, datalen + 1, timeout) != datalen + 1)
			return COMMUFAIL_ERR;
		
		m_packet->len += (datalen + 1);
		if (lrc((unsigned char* )recvbuf + 1, m_packet->len - 2) != *((unsigned char*)recvbuf + m_packet->len - 1))
			return LRC_ERR;

		hslog::loghex(m_packet->buf, m_packet->len);
		return 0;
	}

	if (mhservice::m_allstop)
		return USERCANCEL_ERR;

	return 0;
}

int  mhcommethod::pop_sendrecvpacket(void *packet, int len)
{
	int iRet = 0;
	OBEX* recvbuf = (OBEX*)m_packet->buf;
	for (int i = 0; i < 3; i++)
	{
		if (mhservice::m_allstop)
			return USERCANCEL_ERR;

		pop_sendpacket(packet, len);

		iRet = pop_receivepacket(PACKETTIMEOUT);
		if (iRet != 0)
			continue;

		if (recvbuf->cmdl != 0x00)
		{
			iRet = recvbuf->cmdl;
			return iRet;
		}
		break;
	}

	return iRet;
}

void mhcommethod::SendPacket(char step, void *buf, int len)
{
	char *pBuf = (char *)malloc(16 * 1024);
	int i = 0;
	ASSERT(pBuf != NULL);

	pBuf[i++] = DIR_DOWN;
	pBuf[i++] = step;
	*(unsigned short*)(pBuf + i) = len + 4;	//4为包头信息长度
	i += 2;
	memcpy(pBuf + i, buf, len);
	i += len;
	*(unsigned short*)(pBuf + i) = CRC::crc16((unsigned char *)pBuf, i, 0xFFFF);
	i += 2;
	m_Port.Write(pBuf, i);
	hslog::loghex((unsigned char *)pBuf, i);
	free(pBuf);
}

void mhcommethod::SendPacketTimeout(char step, void *buf, int len, int timeout)
{
	char *pBuf = (char *)malloc(16 * 1024);
	int i = 0;
	int j = 0, n;
	ASSERT(pBuf != NULL);

	pBuf[i++] = DIR_DOWN;
	pBuf[i++] = step;
	*(unsigned short*)(pBuf + i) = len + 4;	//4为包头信息长度
	i += 2;
	memcpy(pBuf + i, buf, len);
	i += len;
	*(unsigned short*)(pBuf + i) = CRC::crc16((unsigned char *)pBuf, i, 0xFFFF);
	i += 2;
	for (j = 0; j < i; j += 32)
	{
		if (j + 32 > i)
		{
			n = i - j;
		}
		else
		{
			n = 32;
		}
		m_Port.Write(pBuf + j, n);
		Sleep(timeout);
	}
	hslog::loghex((unsigned char *)pBuf, i);
	free(pBuf);
}

//step 1 , 2	握手
int  mhcommethod::stepconnecthandler(DThreadParam& param, int timeout)
{
	int iRet = 0;
	Packet *pPacket = (Packet *)m_packet->buf;
	char buf[128] = { 0 };
	int step = 0;
	DWORD tick = 0;
	m_packet->len = 0;
	while (!mhservice::m_allstop)
	{
		if (step == 0)
		{
			memset(buf, 0x7F, 10);
			m_Port.Write(buf, 1);
			if (ReceivePacket() == 0)
			{
				if (pPacket->step == '1')
				{
					step++;
					memset(buf, 0x7C, 10);
					m_Port.Write(buf, 10);
					hslog::loghex((unsigned char *)buf, 10);
					tick = GetCurrentTime();
					param.snParam.stage = pPacket->content.chipInfo.stage;
					if (pPacket->content.chipInfo.stage >= 4)		//读取到了E_SN
					{
						param.snParam.opt = SCPU_DL_NO_OPERATION;
					}
				}
			}
		}
		else if (step == 1)
		{
			if (ReceivePacket() == 0)
			{
				if (pPacket->step == '2')
				{
					if (pPacket->content.chipInfo.stage >= 4)		//读取到了C_SN
					{
						param.snParam.opt = SCPU_DL_NO_OPERATION;
					}
					return 0;
				}
			}
			else if (timeout == -1)
			{
				continue;
			}
			else if ((int)GetDiffTime(tick) > timeout)
			{
				iRet = SHAKETIMEOUT_ERR;
				goto EXIST;
			}
		}
	}
	if (mhservice::m_allstop)
		iRet = USERCANCEL_ERR;
EXIST:
	return iRet;
}

//step 3	配置参数
int  mhcommethod::stepwritesnhandler(DThreadParam& param, int timeout)
{
	int iRet = 0;
	int len;
	int keyLen = RSA_LEN_2048;
	DLSNParam *p = &param.snParam;
	Packet *pPacket = (Packet *)m_packet->buf;

	if (p->opt != SCPU_DL_WRITE_PARAM)
	{
		len = 2;
	}
	else
	{
		if (p->content.param.info & 0x8000)
		{
			keyLen = RSA_LEN_4096;
		}
		len = p->content.param.regNum * sizeof(Reg) + keyLen + offset(DLSNParam, content.param.buf);
	}
	SendPacket('3', &(p->stage), len);

	int tick = GetCurrentTime();

	while (!mhservice::m_allstop)
	{
		if (ReceivePacket() == 0)
		{
			if (pPacket->step != ACK)
			{
				iRet = htonl(*(int *)pPacket->content.buf);
				goto EXIST;
			}
			else if (pPacket->step == ACK)
			{				
				return 0;
			}
		}
		else if ((int)GetDiffTime(tick) > timeout)
		{
			iRet = SHAKETIMEOUT_ERR;
			goto EXIST;
		}
	}
	if (mhservice::m_allstop)
		iRet = USERCANCEL_ERR;
EXIST:
	return iRet;
}

//step 4	写文件参数
int  mhcommethod::stepwritefilehandler(DThreadParam& param, FileInfo *pFileInfo, int timeout)
{
	int iRet = 0;
	char buf[2048];
	Packet *pPacket = (Packet *)m_packet->buf;
	DLFileParam *p = (DLFileParam *)buf;	
	memset(buf, 0, sizeof(buf));

	memcpy(p, pFileInfo->info.v, sizeof(pFileInfo->info));
	int ttt = sizeof(DLFileParam);
	SendPacket('4', buf, sizeof(DLFileParam));

	sprintf(buf, "ERASE Flash...");
	//SendDownLoadMsg(DL_STATE_MSG_STRING, buf, strlen(buf) + 1, GetWndHandle());

	int nSector = 0;
	int i, j;

	for (i = 0; i < sizeof(pFileInfo->info.param.eraseBitmap); i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (pFileInfo->info.param.eraseBitmap[i] & (1 << j))
			{
				nSector++;
			}
		}
	}
	if (nSector == i * j)	//全擦
	{
		nSector = 1;
	}
	i = j = 0;

	int tick = GetCurrentTime();
	//SendDownLoadMsg(DL_STATE_ERASE_FLASH, &j, sizeof(j), GetWndHandle());
	while (!mhservice::m_allstop)
	{
		if (ReceivePacket() == 0)
		{
			j = i * 100 / nSector;
			//这里是否需要进度条
			//SendDownLoadMsg(DL_STATE_ERASE_FLASH, &j, sizeof(j), GetWndHandle());
			if (pPacket->step != ACK)
			{
				sprintf(buf, "文件写入参数失败, 返回错误代码为%04X!", (pPacket->content.buf[0] << 8) | pPacket->content.buf[1]);
				iRet = (pPacket->content.buf[0] << 8) | pPacket->content.buf[1];
				return iRet;
			}
			else if (pPacket->step == ACK)
			{
				if (++i == nSector)
				{
					return 0;
				}
			}
		}
		else if ((int)GetDiffTime(tick) > timeout)
		{
			iRet = SHAKETIMEOUT_ERR;
			goto EXIST;
		}
	}
	if (mhservice::m_allstop)
		iRet = USERCANCEL_ERR;
EXIST:
	return iRet;
}

//step 5	传输文件内容
int  mhcommethod::stepwritefiledatahandler(FileInfo *pFileInfo, int timeout)
{
	char fileSeg[16 * 1024];
	char buf[1024];
	DLFileSeg *p = (DLFileSeg *)fileSeg;
	int i = 0;
	int ret = 0;
	int len = 0;
	Packet *pPacket = (Packet *)m_packet->buf;
	int percent = 0;
	RESPPACK pack = { 0 };

	//Flash基址:0x01000000
	if (pFileInfo->startAddr + pFileInfo->fileLen + FLASH_SECTOR_SIZE > FLASH_BASE_ADDR + FLASH_SIZE &&
		pFileInfo->startAddr < FLASH_BASE_ADDR + FLASH_SIZE)
	{
		sprintf(buf, "文件跨越了Flash边界或文件太大!");
		ret = -4;
		goto quit;
	}
	while (!mhservice::m_allstop && i < pFileInfo->fileLen)
	{
		if (pFileInfo->startAddr < FLASH_BASE_ADDR ||
			pFileInfo->startAddr > FLASH_BASE_ADDR + FLASH_SIZE)
		{
			p->unlock = 0;
		}
		else
		{
			p->unlock = 1 << ((pFileInfo->startAddr + i) / FLASH_PROTECT_UNIT);
		}
		len = pFileInfo->fileLen - i;
		if (len > MAX_FILE_SEG_SIZE)
		{
			len = MAX_FILE_SEG_SIZE;
		}
		p->addr = i + pFileInfo->startAddr;
		memcpy(p->buf, pFileInfo->buf + i, len);
		i += len;
		SendPacketTimeout('5', fileSeg, sizeof(DLFileSeg) + len, 5);
		int tick = GetCurrentTime();
		while (!mhservice::m_allstop)
		{
			if (ReceivePacket() == 0)
			{
				if (pPacket->step != ACK)
				{
					sprintf(buf, "写文件内容出错, 返回错误代码为%08X!", htonl(*(int *)pPacket->content.buf));
					ret = -3;
					goto quit;
				}
				else if (pPacket->step == ACK)
				{
					break;
				}
			}
			else if ((int)GetDiffTime(tick) > timeout)
			{
				return SHAKETIMEOUT_ERR;
			}
		}

		//计算文件下载进度
		/*int pos = 30;
		pos += (i * 20 / pFileInfo->fileLen);
		strcpy(pack.buf.devname, task.m_devname.GetBuffer());
		task.m_devname.ReleaseBuffer();
		memcpy(pack.buf.reserve, &pos, sizeof(pos));
		SendMessage(task.m_hwnd, WM_MESSAGEINFO, P_PROGRESS, (LPARAM)&pack);*/
	}
	if (mhservice::m_allstop)
		ret = USERCANCEL_ERR;

quit:
	//SendDownLoadMsg(DL_STATE_MSG_STRING, buf, strlen(buf) + 1, GetWndHandle());
	return ret;
}

//step 6	取收验证内容
int  mhcommethod::stepverifydatahandler(int timeout)
{
	Packet *pPacket = (Packet *)m_packet->buf;
	int tick = GetCurrentTime();
	while (!mhservice::m_allstop)
	{
		if (ReceivePacket() == 0)
		{
			if (pPacket->step != ACK)
			{			
				return htonl(*(int *)pPacket->content.buf);
			}
			else if (pPacket->step == ACK)
			{
				return 0;
			}
		}
		else if ((int)GetDiffTime(tick) > timeout)
		{
			return SHAKETIMEOUT_ERR;
		}
	}
	if (mhservice::m_allstop)
		return USERCANCEL_ERR;
	return 0;
}

DWORD mhcommethod::GetDiffTime(DWORD t)
{
	DWORD n = ::GetCurrentTime();
	if (n < t) {
		return (n + (~t) + 1);
	}
	else {
		return (n - t);
	}
}

unsigned char mhcommethod::lrc(unsigned char* buf, int len)
{
	unsigned char lrc = 0;

	for (int i = 0; i < len; i++)
	{
		lrc ^= buf[i];
	}
	return lrc;
}

int  mhcommethod::loadfile(FileInfo *pInfo, char* path)
{
	char buf[10] = { 0 };
	FILE *fp;
	int ret = 0;
	int pos;

	memset(pInfo, 0, sizeof(FileInfo));
	utility::strtolower(path);
	for (pos = strlen(path) - 1; pos > 0; pos--)
	{
		if (path[pos] == '.' && strcmp(path + pos + 1, "hex") == 0)
		{
			break;
		}
	}
	if (pos > 0)	//读取并转换hex文件
	{
		pInfo->info.param.refresh = 0;
		memset(pInfo->info.param.eraseBitmap, 0xFF, sizeof(pInfo->info.param.eraseBitmap));
		if (ConvertFile(pInfo, path, 0, HASH_TYPE_256, (short)0xFFFF))
		{
			return -4;
		}
	}
	else
	{
		fp = fopen(path, "rb");
		if (fp == NULL)
		{
			return -1;
		}
		if (fread(buf, 7, 1, fp) > 0)
		{
			if (strcmp(buf, BIN_HEAD_NOS) != 0 && strcmp(buf, BIN_HEAD_SIG) != 0)
			{
				pInfo->info.param.refresh = 0;
				memset(pInfo->info.param.eraseBitmap, 0xFF, sizeof(pInfo->info.param.eraseBitmap));
				if (ConvertFile(pInfo, path, 0x1001000, HASH_TYPE_256, (short)0xFFFF))
				{
					return -4;
				}
			}
			else
			{
				fread(&(pInfo->startAddr), 8, 1, fp);	//startAddr and fileLen
				fread(pInfo->info.v, 1024, 1, fp);
				fread(pInfo->buf, pInfo->fileLen, 1, fp);
			}
		}
		else
		{
			ret = -3;
			goto err;
		}

	err:
		fclose(fp);
	}
	return ret;
}

/*
功    能: 转换指定的文件并生成文件头信息
参数说明：
pInfo		文件数据缓存
path		转换文件路径
startAddr	bin文件起始地址，仅限bin文件时有效
hashType	HASH_TYPE_256 或 HASH_TYPE_512
ver			固件版本，数值越小，版本越高
返回值：
-1			不支持的文件类型(文件扩展名错误)
-2			hex文件格式错误
-3			文件无法正确打开
-4			文件太大
-5			hashType参数错误
*/
int  mhcommethod::ConvertFile(FileInfo *pInfo, char *path, const unsigned int startAddr, const unsigned short hashType, const short ver)
{
	int ret;
	SHA256Context foo;
	char extName[1024];

	if (utility::GetExtFileName(path, extName) != 0)
	{
		return -1;
	}

	utility::strtolower(extName);
	if (strcmp(extName, "hex") == 0)   
	{
		ret = LoadHexFile(path, pInfo->buf, sizeof(pInfo->buf), &(pInfo->fileLen), &(pInfo->startAddr), 0x00);
		if (ret != 0)
		{
			return -2;
		}
	}
	else if (strcmp(extName, "bin") == 0)
	{
		FILE *fp = fopen(path, "rb");

		if (fp == NULL)
		{
			return -3;
		}
		fseek(fp, 0, SEEK_END);
		if (ftell(fp) >= sizeof(pInfo->buf))
		{
			return -4;
		}
		pInfo->fileLen = ftell(fp);
		pInfo->startAddr = startAddr;
		fseek(fp, 0, SEEK_SET);
		fread(pInfo->buf, pInfo->fileLen, 1, fp);

		if (fp != NULL)
		{
			fclose(fp);
		}
	}
	else
	{
		return -1;
	}

	int blocknum = 0;
	if (pInfo->fileLen % BLOCKSIZE == 0)
	{
		blocknum = pInfo->fileLen / BLOCKSIZE + 1;
	}
	else
	{
		blocknum = pInfo->fileLen / BLOCKSIZE + 2;
	}
	int i, nItem = 0;

	for (i = nItem = 0; nItem < blocknum; nItem++, i++)
	{
		pInfo->info.param.eraseBitmap[i >> 3] |= 1 << (i & 0x07);
	}

	pInfo->info.param.sig.s.len = pInfo->fileLen;
	pInfo->info.param.sig.s.start = pInfo->startAddr;
	pInfo->info.param.sig.s.opt = hashType;
	pInfo->info.param.sig.s.reserved = 0;	//固定为0
	pInfo->info.param.sig.s.ver = ver;
	pInfo->info.param.valid = FILE_VALID_FLAG;

	//签名——算Hash
	if (hashType == HASH_TYPE_256)
	{
		SHA256Init(&foo);
		SHA256Update(&foo, pInfo->buf, pInfo->fileLen);
		SHA256Final(&foo, (unsigned char *)pInfo->info.param.sig.s.hash);
		CRC::MakeCrc32Table();
		pInfo->info.param.sig.s.crc = CRC::crc32((char *)&pInfo->info.param.valid, 84, 0xFFFFFFFFL);
	}
	else if (hashType == HASH_TYPE_512)
	{
		SHA512((const unsigned char *)pInfo->buf, pInfo->fileLen, (unsigned char *)pInfo->info.param.sig.s.hash);
		CRC::MakeCrc32Table();
		pInfo->info.param.sig.s.crc = CRC::crc32((char *)&pInfo->info.param.valid, 84, 0xFFFFFFFFL);
	}

	
	return 0;
}


int  mhcommethod::pop_filehandler(HWND hwnd, unsigned char start, char* startbuf, int startlen, char* buf, int len, bool isreport)
{
	int iRet = 0;
	int serialno = 0;
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);

	pop_buildpacket(pobex, 0x00, start, serialno, startbuf, startlen);
	
	iRet = pop_sendrecvpacket(pobex, startlen + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;
	serialno++;

	int ineedsend = len;
	int ialready = 0;
	while (ineedsend != 0)
	{
		int isendlen = 0;
		if (ineedsend > BUFMAX_LEN)
		{
			isendlen = BUFMAX_LEN;
		}
		else if(ineedsend > 0)
		{
			isendlen = ineedsend;
		}

		pop_buildpacket(pobex, 0x00, start + 1, serialno, buf + ialready, isendlen);

		iRet = pop_sendrecvpacket(pobex, isendlen + MIN_POPPACKETLEN);
		if (iRet != 0)
			goto EXIST;
		serialno++;
		ineedsend -= isendlen;
		ialready += isendlen;
		//这里需要返回进度
		if(isreport)
			SendMessage(hwnd, WM_MESSAGEINFO, P_PROGRESS, ialready * 100 / len );
	}

	pop_buildpacket(pobex, 0x00, start + 2, serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;
EXIST:

	if (pobex != NULL)
	{
		free(pobex);
		pobex = NULL;
	}

	return iRet;
}

int  mhcommethod::pop_handshake(char ack)
{
	char buf[10] = { 0 };
	char recvbuf[10] = { 0 };
	buf[0] = ack;

	pop_clearbaddata();

	while (!mhservice::m_allstop)
	{
		pop_sendpacket(buf, 1);

		if (m_Port.Readv((char*)recvbuf, 1, PACKETSENDTIMEOUT) == 1 && recvbuf[0] == 'A')
			break;
	}

	if (mhservice::m_allstop)
		return USERCANCEL_ERR;

	m_serialno = 0x00;

	return 0;
}

int  mhcommethod::pop_handshake(char ack, int timeout)
{
	char buf[10] = { 0 };
	char recvbuf[10] = { 0 };
	buf[0] = ack;

	pop_clearbaddata();
	int tick = GetCurrentTime();

	while (!mhservice::m_allstop)
	{
		if (GetDiffTime(tick) > (DWORD)timeout)
		{
			return SHAKETIMEOUT_ERR;
		}

		pop_sendpacket(buf, 1);

		if (m_Port.Readv((char*)recvbuf, 1, PACKETSENDTIMEOUT) == 1 && recvbuf[0] == 'A')
		{
				break;
		}
	}

	if (mhservice::m_allstop)
		return USERCANCEL_ERR;

	m_serialno = 0x00;

	return 0;
}

void mhcommethod::pop_clearbaddata()
{
	char buf[10] = { 0 };
	while (!mhservice::m_allstop)
	{		
		if (m_Port.Readv(buf, 1, 300) < 1)
			break;
	}
}

void mhcommethod::sendmessage(HWND hwnd, HANDLE mutex, WPARAM wparam, LPARAM lparam)
{
	WaitForSingleObject(mutex, INFINITE);
	SendMessage(hwnd, WM_USER, wparam, lparam);
	ReleaseMutex(mutex);
}

int  mhcommethod::LoadRSAPublicKeyFile(char *path, R_RSA_PUBLIC_KEY *pkey)
{
	char buf[10 * 1024];
	char field[200];
	char *p = NULL;
	int i;
	FILE *pfile = NULL;
	//R_RSA_PRIVATE_KEY key;
	int bits = 0;
	int n;
	int ret = -3;


	if ((pfile = fopen(path, "rb")) == NULL)
	{
		return -1;
	}
	i = 0;
	if (GetFileLine(pfile, field) <= 0 || strcmp(field, RSA_FILE_HEAD) != 0)
	{
		fclose(pfile);
		return -2;
	}
	while (1)
	{
		if (GetFileLine(pfile, buf) > 0)
		{
			for (i = 0; buf[i]; i++)
			{
				if (buf[i] == ':')
				{
					buf[i] = 0;
					strcpy(field, buf);
					p = buf + i + 1;
					i++;
					break;
				}
			}
			if (buf[i] == 0)
			{
				goto fmterr;
			}
			if (strcmp(field, RSA_FILE_KEY_LEN) == 0)
			{
				pkey->bits = strtol(p, NULL, 10);
				bits |= 0x01;
			}
			else if (strcmp(field, RSA_FILE_PRI_KEY_MOD) == 0)
			{
				n = StrToRSA(p, pkey->modulus);
				if (n != pkey->bits / 8)
				{
					goto fmterr;
				}
				bits |= 0x02;
			}
			else if (strcmp(field, RSA_FILE_PUB_KEY_EXP) == 0)
			{
				n = StrToRSA(p, pkey->exponent);
				if (n <= 8)
				{
					memmove(pkey->exponent + pkey->bits / 8 - n, pkey->exponent, n);
					memset(pkey->exponent, 0, pkey->bits / 8 - n);
				}
				else if (n > 8)
				{
					goto fmterr;
				}
				bits |= 0x04;
			}
		}
		else
		{
			break;
		}
	}

	if (bits != 0x07)
	{
		ret = -4;
		goto fmterr;
	}

	fclose(pfile);
	return 0;

fmterr:
	fclose(pfile);
	return ret;
}

int  mhcommethod::LoadRSAFile(char *path, R_RSA_PRIVATE_KEY *pkey, unsigned char *rr)
{
	char buf[10 * 1024];
	char field[200];
	char *p = NULL;
	int i;
	FILE *pfile = NULL;
	//R_RSA_PRIVATE_KEY key;
	int bits = 0;
	int n;
	int ret = -3;


	if ((pfile = fopen(path, "rb")) == NULL)
	{
		return -1;
	}
	i = 0;
	if (GetFileLine(pfile, field) <= 0 || strcmp(field, RSA_FILE_HEAD) != 0)
	{
		fclose(pfile);
		return -2;
	}
	while (1)
	{
		if (GetFileLine(pfile, buf) > 0)
		{
			for (i = 0; buf[i]; i++)
			{
				if (buf[i] == ':')
				{
					buf[i] = 0;
					strcpy(field, buf);
					p = buf + i + 1;
					i++;
					break;
				}
			}
			if (buf[i] == 0)
			{
				goto fmterr;
			}
			if (strcmp(field, RSA_FILE_KEY_LEN) == 0)
			{
				pkey->bits = strtol(p, NULL, 10);
				bits |= 0x01;
			}
			else if (strcmp(field, RSA_FILE_PRI_KEY_MOD) == 0)
			{
				n = StrToRSA(p, pkey->modulus);
				if (n != pkey->bits / 8)
				{
					goto fmterr;
				}
				bits |= 0x02;
			}
			else if (strcmp(field, RSA_FILE_PUB_KEY_EXP) == 0)
			{
				n = StrToRSA(p, pkey->publicExponent);
				if (n <= 8)
				{
					memmove(pkey->publicExponent + pkey->bits / 8 - n, pkey->publicExponent, n);
					memset(pkey->publicExponent, 0, pkey->bits / 8 - n);
				}
				else if (n > 8)
				{
					goto fmterr;
				}
				bits |= 0x04;
			}
			else if (strcmp(field, RSA_FILE_PRI_KEY_EXP) == 0)
			{
				n = StrToRSA(p, pkey->exponent);
				if (n != pkey->bits / 8)
				{
					goto fmterr;
				}
				bits |= 0x08;
			}
			else if (strcmp(field, RSA_FILE_PRI_KEY_PRI1) == 0)
			{
				n = StrToRSA(p, pkey->prime[0]);
				if (n != pkey->bits / 8 / 2)
				{
					goto fmterr;
				}
				bits |= 0x10;
			}
			else if (strcmp(field, RSA_FILE_PRI_KEY_PRI2) == 0)
			{
				n = StrToRSA(p, pkey->prime[1]);
				if (n != pkey->bits / 8 / 2)
				{
					goto fmterr;
				}
				bits |= 0x20;
			}
			else if (strcmp(field, RSA_FILE_PRI_KEY_PRI_EXP1) == 0)
			{
				n = StrToRSA(p, pkey->primeExponent[0]);
				if (n != pkey->bits / 8 / 2)
				{
					goto fmterr;
				}
				bits |= 0x40;
			}
			else if (strcmp(field, RSA_FILE_PRI_KEY_PRI_EXP2) == 0)
			{
				n = StrToRSA(p, pkey->primeExponent[1]);
				if (n != pkey->bits / 8 / 2)
				{
					goto fmterr;
				}
				bits |= 0x80;
			}
			else if (strcmp(field, RSA_FILE_PRI_KEY_COEF) == 0)
			{
				n = StrToRSA(p, pkey->coefficient);
				if (n != pkey->bits / 8 / 2)
				{
					goto fmterr;
				}
				bits |= 0x100;
			}
			else if (strcmp(field, RSA_FILE_RR_MOD_M) == 0)
			{
				n = StrToRSA(p, rr);
				if (n != pkey->bits / 8)
				{
					goto fmterr;
				}
				bits |= 0x200;
			}
			else
			{
				goto fmterr;
			}
		}
		else
		{
			break;
		}
	}

	if (bits != 0x3FF && bits != 0x1FF)
	{
		ret = -4;
		goto fmterr;
	}

	fclose(pfile);
	return 0;

fmterr:
	fclose(pfile);
	return ret;
}

void mhcommethod::GetErasePages(unsigned char* pPages, int blocknum)
{
	int i, nItem;

	for (i = nItem = 0; nItem < blocknum; nItem++, i++)
	{
		pPages[i >> 3] |= 1 << (i & 0x07);
	}
}

int  mhcommethod::pop_getchallengecode(USER_INFO info, unsigned char* chcode)
{
	int iRet = 0;
	int serialno = 0x01;
	OBEX *pobex = (OBEX*)malloc(MAX_PACKET_LEN);
	unsigned char sendbuf[42] = { 0 };

	memcpy(sendbuf, &info, sizeof(info));

	pop_buildpacket(pobex, CMDH::CMD_BOOT, CMDL::AUTH_CHCODE, serialno, sendbuf, sizeof(sendbuf));
	iRet = pop_sendrecvpacket(pobex, sizeof(sendbuf) + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < 256 + MIN_POPPACKETLEN)
	{
		iRet = DATA_ERR;
		goto EXIST;
	}
	memcpy(chcode, m_packet->buf + 6, 256);
EXIST:
	if (pobex != NULL)
	{
		free(pobex);
		pobex = NULL;
	}

	return iRet;
}

int  mhcommethod::pop_sendlicsence(unsigned char* data, int datalen, int type)
{
	int iRet = 0;
	int serialno = 0x02;
	OBEX *pobex = (OBEX*)malloc(MAX_PACKET_LEN);
	char sendbuf[256] = { 0 };

	if (datalen != 256)
	{
		iRet = PATAM_ERR;
		goto EXIST;
	}

	//sendbuf[0] = type;
	memcpy(sendbuf, data, datalen);

	pop_buildpacket(pobex, CMDH::CMD_BOOT, type, serialno, sendbuf, 256);
	iRet = pop_sendrecvpacket(pobex, 256 + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

EXIST:
	if (pobex != NULL)
	{
		free(pobex);
		pobex = NULL;
	}

	return iRet;
}

int  mhcommethod::pop_productauthorize()
{

	USER_INFO info = { 0 };
	SYSTEMTIME stLocal;
	unsigned char chcode[256] = { 0 };
	CString errmessage = "";
	unsigned char ucfromserver[256] = { 0 };
	pophttpcomm authpop = g_authpop;

	GetLocalTime(&stLocal);
	char tt[256] = { 0 };
	sprintf_s(tt, 256, "%02d%02d%02d%02d", stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour);
	memcpy(info.time, tt + 2, 8);

	info.type[0] = GetAuthNo(AUTH_PRODUCE);
	info.type[1] = 0x33;

	strcpy(info.userinfo, authpop.info.username.c_str());

	if (pop_handshake('B', 15000) != 0)
		return 1;

	if (pop_getchallengecode(info, chcode) != 0)
		return 2;

	if (authpop.authrequest(chcode, 256, ucfromserver, errmessage) != 0)
	{
		hslog::log(errmessage.GetBuffer());
		return 3;
	}

	int iret = pop_sendlicsence(ucfromserver, 256, AUTH_PRODUCE);
	if(iret == 3)
		return 0;
	if (iret != 0)
		return 4;

	return 0;
}

int  mhcommethod::pop_bootdownload(TASKDATA param)
{
	int iRet = 0;
	unsigned char startbuf[100] = { 0 };
	int startlen = 0;
	unsigned char* filebuf = NULL;
	int filelen = 0;
	CFile file;
	ST_HEADER_INFO headinfo = { 0 };

	if (!file.Open(param.szFilePath, CFile::modeRead))
	{
		iRet = FILEOPEN_ERR;
		goto EXIST;
	}
	filelen = (int)file.GetLength();
	file.SeekToBegin();
	if (file.Read(&headinfo, sizeof(headinfo)) != sizeof(headinfo))
	{
		iRet = FILEREAD_ERR;
		goto EXIST;
	}
	memcpy(startbuf, headinfo.PID, sizeof(headinfo.PID));
	memcpy(startbuf + sizeof(headinfo.PID), headinfo.version, sizeof(headinfo.version));
	startlen = sizeof(headinfo.PID) + sizeof(headinfo.version);
	

	filebuf = (unsigned char*)malloc(filelen);
	file.SeekToBegin();
	
	if (file.Read(filebuf, filelen) != filelen)
	{
		iRet = FILEREAD_ERR;
		goto EXIST;
	}
	iRet = pop_filehandler(param.hwnd, 0x00, (char*)startbuf, startlen, (char*)filebuf, filelen, true);

EXIST:
	if (filebuf)
	{
		free(filebuf);
		filebuf = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osdownload(TASKDATA param)
{
	int iRet = 0;
	unsigned char startbuf[100] = { 0 };
	int startlen = 0;
	unsigned char* filebuf = NULL;
	int filelen = 0;
	CFile file;
	ST_HEADER_INFO headinfo = { 0 };

	if (!file.Open(param.szFilePath, CFile::modeRead))
	{
		iRet = FILEOPEN_ERR;
		goto EXIST;
	}
	filelen = (int)file.GetLength();
	file.SeekToBegin();
	if (file.Read(&headinfo, sizeof(headinfo)) != sizeof(headinfo))
	{
		iRet = FILEREAD_ERR;
		goto EXIST;
	}
	memcpy(startbuf, headinfo.PID, sizeof(headinfo.PID));
	memcpy(startbuf + sizeof(headinfo.PID), headinfo.version, sizeof(headinfo.version));
	startlen = sizeof(headinfo.PID) + sizeof(headinfo.version);


	filebuf = (unsigned char*)malloc(filelen);
	file.SeekToBegin();

	if (file.Read(filebuf, filelen) != filelen)
	{
		iRet = FILEREAD_ERR;
		goto EXIST;
	}
	iRet = pop_filehandler(param.hwnd, 0x03, (char*)startbuf, startlen, (char*)filebuf, filelen,true);

EXIST:
	if (filebuf)
	{
		free(filebuf);
		filebuf = NULL;
	}
	return iRet;
}

//生产用
int  mhcommethod::pop_osdownload(CString ospath ,HWND hwnd)
{
	int iRet = 0;
	unsigned char startbuf[100] = { 0 };
	int startlen = 0;
	unsigned char* filebuf = NULL;
	int filelen = 0;
	CFile file;
	ST_HEADER_INFO headinfo = { 0 };

	if (!file.Open(ospath, CFile::modeRead | CFile::shareDenyNone))
	{
		iRet = FILEOPEN_ERR;
		goto EXIST;
	}
	filelen = (int)file.GetLength();
	file.SeekToBegin();
	if (file.Read(&headinfo, sizeof(headinfo)) != sizeof(headinfo))
	{
		iRet = FILEREAD_ERR;
		goto EXIST;
	}
	memcpy(startbuf, headinfo.PID, sizeof(headinfo.PID));
	memcpy(startbuf + sizeof(headinfo.PID), headinfo.version, sizeof(headinfo.version));
	startlen = sizeof(headinfo.PID) + sizeof(headinfo.version);


	filebuf = (unsigned char*)malloc(filelen);
	file.SeekToBegin();

	if (file.Read(filebuf, filelen) != filelen)
	{
		iRet = FILEREAD_ERR;
		goto EXIST;
	}

	file.Close();
	iRet = pop_filehandler(hwnd, 0x03, (char*)startbuf, startlen, (char*)filebuf, filelen, false);

EXIST:
	if (filebuf)
	{
		free(filebuf);
		filebuf = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_sn1downlaod(TASKDATA param)
{
	int iRet = 0;
	unsigned char startbuf[100] = { 0 };
	int startlen = 0;
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);
	memset(pobex, 0, POPPACKET_LEN);
	strcpy((char*)startbuf, param.szFilePath);
	startlen = 24;

	pop_buildpacket(pobex, CMD_BOOT, CMD_SN1, 0x01, startbuf, startlen);

	iRet = pop_sendrecvpacket(pobex, startlen + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

EXIST:
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_sn1downlaod(CString sn1)
{
	int iRet = 0;
	unsigned char startbuf[100] = { 0 };
	int startlen = 0;
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);

	memset(pobex, 0, POPPACKET_LEN);
	strcpy((char*)startbuf, sn1.GetBuffer());
	startlen = 24;

	pop_buildpacket(pobex, CMD_BOOT, CMD_SN1, 0x01, startbuf, startlen);

	iRet = pop_sendrecvpacket(pobex, startlen + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

EXIST:
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_sn2downlaod(TASKDATA param)
{
	int iRet = 0;
	unsigned char startbuf[100] = { 0 };
	int startlen = 0;
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);
	memset(pobex, 0, POPPACKET_LEN);
	strcpy((char*)startbuf, param.szFilePath);
	startlen = 24;

	pop_buildpacket(pobex, CMD_BOOT, CMD_SN2, 0x01, startbuf, startlen);

	iRet = pop_sendrecvpacket(pobex, startlen + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	EXIST:
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_configdownlaod(CString filepath)
{
	int iRet = 0;
	unsigned char* filebuf = NULL;
	int filelen = 0;
	CFile file;
	ST_HEADER_INFO headinfo = { 0 };
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);
	memset(pobex, 0, POPPACKET_LEN);

	if (!file.Open(filepath, CFile::modeRead | CFile::shareDenyNone))
	{
		iRet = FILEOPEN_ERR;
		goto EXIST;
	}
	filelen = (int)file.GetLength();
	file.SeekToBegin();

	if (filelen != 3 * 1024)
	{
		iRet = FILEREAD_ERR;
		goto EXIST;
	}
	filebuf = (unsigned char*)malloc(filelen);

	if (file.Read(filebuf, filelen) != filelen)
	{
		iRet = FILEREAD_ERR;
		goto EXIST;
	}

	file.Close();

	pop_buildpacket(pobex, CMD_BOOT, CMD_CONFIG, 0x01, filebuf, filelen);

	iRet = pop_sendrecvpacket(pobex, filelen + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

EXIST:
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	if (filebuf)
	{
		free(filebuf);
		filebuf = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_sn2downlaod(CString sn2)
{
	int iRet = 0;
	unsigned char startbuf[100] = { 0 };
	int startlen = 0;
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);
	memset(pobex, 0, POPPACKET_LEN);
	strcpy((char*)startbuf, sn2.GetBuffer());
	startlen = 24;

	pop_buildpacket(pobex, CMD_BOOT, CMD_SN2, 0x01, startbuf, startlen);

	iRet = pop_sendrecvpacket(pobex, startlen + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

EXIST:
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_pukdownlaod(TASKDATA param)
{
	int iRet = 0;
	unsigned char startbuf[100] = { 0 };
	int startlen = 0;
	unsigned char* filebuf = NULL;
	int filelen = 0;
	CFile file;
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);

	if (!file.Open(param.szFilePath, CFile::modeRead))
	{
		iRet = FILEOPEN_ERR;
		goto EXIST;
	}

	startlen = (int)file.GetLength() - sizeof(ST_HEADER_INFO);
	if (startlen != 512)
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	file.Seek(sizeof(ST_HEADER_INFO), CFile::begin);
	if (file.Read(startbuf, startlen) != 512)
	{
		iRet = DATA_ERR;
		goto EXIST;
	}	

	pop_buildpacket(pobex, CMD_BOOT, CMD_PUK, 0x01, startbuf, startlen);

	iRet = pop_sendrecvpacket(pobex, startlen + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	SendMessage(param.hwnd, WM_MESSAGEINFO, P_PROGRESS, 100);
EXIST:
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_appdownlaod(TASKDATA param)
{
	//todo:
	return 0;
}



int  mhcommethod::pop_ossettime(char* time)
{
	int iRet = 0;	
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_SETTIME, m_serialno, time, strlen(time));

	iRet = pop_sendrecvpacket(pobex, strlen(time) + MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgettime(char* time)
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETTIME, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (12 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(time, m_packet->buf + 6, 12);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetrandom(int randomlen, byte* random) 
{
	int iRet = 0;
	byte buffer[2] = { 0 };
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);

	if (randomlen > 1024)
	{
		iRet = PATAM_ERR;
		goto EXIST;
	}
	
	buffer[0] = (randomlen >> 8 & 0xFF);
	buffer[1] = randomlen & 0xFF;
	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETRANDOM, m_serialno, buffer, 2);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (randomlen + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(random, m_packet->buf + 6, randomlen);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetmeshstatus()
{
	//TODO::
	return 0;
}

int  mhcommethod::pop_osreset() 
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_RESET, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetbootver(char* ver) 
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETBOOTVER, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (8 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(ver, m_packet->buf + 6, 8);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetosver(char* ver) 
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETOSVER, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (8 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(ver, m_packet->buf + 6, 8);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetpid(char* pid) 
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETPID, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (16 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(pid, m_packet->buf + 6, 16);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetpuksigndata(byte* signdata) 
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETPUKINFO, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (256 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(signdata, m_packet->buf + 6, 256);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetdevstatus(byte* firmstatus, byte* appstatus) 
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETSTUTAS, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (2 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	*firmstatus = m_packet->buf[6];
	*appstatus = m_packet->buf[7];
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetfdebugnum(byte* num)
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETFDEBUGNUM, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (1 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	*num = m_packet->buf[6];
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetadebugnum(byte* num) 
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETADEBUGNUM, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (1 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	*num = m_packet->buf[6];
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetclearpuk(byte* num)
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETPUKNUM, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (1 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	*num = m_packet->buf[6];
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetpuklevel(byte* level)
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETPUKLEVEL, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (1 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	*level = m_packet->buf[6];
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetossigndata(byte* signdata)
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETOSSIGNINFO, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (256 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(signdata, m_packet->buf + 6, 256);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetsn1(char* sn1str)
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETSN1, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (24 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(sn1str, m_packet->buf + 6, 24);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;
	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetsn2(char* sn2str)
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETSN2, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (24 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(sn2str, m_packet->buf + 6, 24);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;

	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_osgetpuk(byte* pukdata)
{
	int iRet = 0;
	OBEX *pobex = (OBEX*)malloc(MIN_POPPACKETLEN);

	pop_buildpacket(pobex, CMD_LVOS, OSCMDL::OS_GETPUK, m_serialno, NULL, 0);

	iRet = pop_sendrecvpacket(pobex, MIN_POPPACKETLEN);
	if (iRet != 0)
		goto EXIST;

	if (m_packet->len < (256 + MIN_POPPACKETLEN))
	{
		iRet = DATA_ERR;
		goto EXIST;
	}

	memcpy(pukdata, m_packet->buf + 6, 256);
EXIST:
	m_serialno++;
	if (m_serialno == 128)
		m_serialno = 0;

	if (pobex)
	{
		free(pobex);
		pobex = NULL;
	}
	return iRet;
}

int  mhcommethod::pop_getdeviceinfo(CString& result)
{
	int iRet = 0;
	char data[260] = { 0 };

	result += " BOOT VERSION: ";
	memset(data, 0, sizeof(data));
	iRet = pop_osgetbootver(data);
	if (iRet != 0)
	{
		result += "ERROR";
	}
	else
	{
		result += data;
	}
	result += "\r\n";

	memset(data, 0, sizeof(data));
	result += " OS VERSION: ";
	iRet = pop_osgetosver(data);
	if (iRet != 0)
	{
		result += "ERROR";
	}
	else
	{
		result += data;
	}
	result += "\r\n";

	memset(data, 0, sizeof(data));
	result += " PID: ";
	iRet = pop_osgetpid(data);
	if (iRet != 0)
	{
		result += "ERROR";
	}
	else
	{
		result += data;
	}
	result += "\r\n";

	byte fstatus = 0x00;
	byte astatus = 0x00;
	result += " DEVICE STATUS: ";
	iRet = pop_osgetdevstatus(&fstatus, &astatus);
	if (iRet != 0)
	{
		result += "ERROR";
	}
	else
	{
		if (fstatus){
			result += " FIRM RELEASE";
		}
		else {
			result += " FIRM DEBUG";
		}
		if (astatus) {
			result += " APP RELEASE";
		}
		else {
			result += " APP DEBUG";
		}
	}
	result += "\r\n";

	
	byte fdebugno = 0xff;
	iRet = pop_osgetfdebugnum(&fdebugno);	
	byte adebugno = 0xff;
	pop_osgetadebugnum(&adebugno);
	byte pukleve = 0xff;
	pop_osgetpuklevel(&pukleve);

	memset(data, 0, sizeof(data));
	sprintf(data, " FIRM DEBUG LEFT: %d APP DEBUG LEFT: %d PUKLEVEL: %d ", fdebugno, adebugno, pukleve);
	result += data;
	result += "\r\n";

	memset(data, 0, sizeof(data));
	iRet = pop_osgetsn1(data);
	if (iRet != 0)
	{
		result += " SN1: ERROR";
	}
	else
	{
		result += " SN1: ";
		if (iRet == 1)
			result += "NONE";
		else
			result += data;
	}
	result += "\r\n";
	memset(data, 0, sizeof(data));
	iRet = pop_osgetsn2(data);
	if (iRet != 0)
	{
		result += " SN2: ERROR";
	}
	else
	{
		result += " SN2: ";
		if (iRet == 1)		
			result += "NONE";
		else	
			result += data;
	}
	result += "\r\n";
	return 0;
}

int  GetFileLine(FILE* pfile, char * buf)
{
	int c;
	int n = 0;

	while ((c = fgetc(pfile)) >= 0)
	{
		if (c != '\r' && c != '\n')
		{
			break;
		}
	}
	if (c >= 0)
	{
		n++;
		*buf++ = c;
		do {
			c = fgetc(pfile);
			if (c < 0 || c == '\r' || c == '\n')
			{
				*buf++ = 0;
				return n;
			}
			*buf++ = c;
			n++;
		} while (1);
	}
	else
	{
		return -1;
	}
}

int  StrToRSA(char* str, unsigned char* rsa)
{
	int i = 0, j = 0, k = 0;
	unsigned char c;
	//int len = strlen(str);
	char buf[1024] = { 0 };

	i = strlen(str) - 1;
	while (i >= 0 && str[i])
	{
		c = 0;
		k = 0;
		if (i >= 0 && str[i] != 0 && (str[i] == '\r' || str[i] == '\n' || str[i] == ' ' || str[i] == '\t'))
		{
			i--;
			continue;
		}
		while (str[i] != ' ' && str[i] != '\t' && i >= 0 && str[i] != 0 && k < 2)
		{
			if (str[i] >= '0' && str[i] <= '9')
			{
				c = (c >> 4) | ((str[i] - '0') << 4);
			}
			else if (str[i] >= 'A' && str[i] <= 'F')
			{
				c = (c >> 4) | ((str[i] - 'A' + 10) << 4);
			}
			else if (str[i] >= 'a' && str[i] <= 'f')
			{
				c = (c >> 4) | ((str[i] - 'a' + 10) << 4);
			}
			i--;
			k++;
		}
		if (k != 2)
		{
			c >>= 4;
		}
		buf[j++] = c;
	}

	for (i = 0; i < j; i++)
	{
		rsa[i] = buf[j - 1 - i];
	}

	return j;
}

int  mhcommethod::GetAuthNo(int type)
{
	int iret = 0;
	switch (type)
	{
	case 0x31:
		iret = 0x02;
		break;
	case 0x32:
		iret = 0x03;
		break;
	case 0x33:
		iret = 0x04;
		break;
	case 0x34:
		iret = 0x05;
		break;
	case 0x35:
		iret = 0x06;
		break;
	case 0x38:
		iret = 0x07;
		break;
	case 0x39:
		iret = 0x08;
		break;
	case 0x37:
		iret = 0x09;
		break;
	case 0x36:
		iret = 0x0a;
		break;
	case 0x3a:
		iret = 0x0b;
		break;
	default:
		iret = -1;
		break;
	}
	return iret;
}

int  mhcommethod::GetProduceResult(char* szResult)
{
	int iexpect = GetTickCount() + 7 * 1000;
	m_packet->len = 0;
	char recvbuf[1024] = { 0 };
	char* p = recvbuf;
	int isstart = 0;
	while (1)
	{
		int curtime = GetTickCount() - iexpect;

		if (curtime > 0)
		{
			return -1;
		}

		if (isstart == 0)
		{
			if (m_Port.Readv(p, 1, 1000) == 1)
			{
				if (*p == '#')
				{
					p++;
					isstart = 1;
				}
			}
		}

		if (isstart == 1)
		{
			if (m_Port.Readv(p, 1, 1000) == 1)
			{
				if (*p == '!')
				{			
					strcpy(szResult, recvbuf);
					break;
				}
				p++;
			}
		}

	}
	return 0;
}

//块区擦除
int  mhcommethod::eraseblockflash(unsigned int startaddr, int isize)
{
	int iRet = 0;
	int serialno = 0;
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN);
	unsigned int start = startaddr;
	unsigned char startbuf[4] = { 0 };
	int startlen = 0;

	while (start < (startaddr + isize))
	{
		memset(startbuf, 0, sizeof(startbuf));
		//设置擦除的首地址
		startbuf[0] = start >> 24 & 0xFF;
		startbuf[1] = start >> 16 & 0xFF;
		startbuf[2] = start >> 8 & 0xFF;
		startbuf[3] = start & 0xFF;

		pop_buildpacket(pobex, OS_FLASHOPERA, 0x03, serialno, startbuf, sizeof(startbuf));

		iRet = pop_sendrecvpacket(pobex, sizeof(startbuf) + MIN_POPPACKETLEN);
		if (iRet != 0)
			goto EXIST;

		start += POP_FLASHBLOCKSIZE;
		serialno++;
	}
	
EXIST:

	if (pobex != NULL)
	{
		free(pobex);
		pobex = NULL;
	}

	return iRet = 0;
}

//flash 写入
int  mhcommethod::writeflash(HWND hwnd, unsigned int startaddr, unsigned char* buf, int isize)
{
	int iRet = 0;
	int serialno = 0;
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN + 6); //
	unsigned int start = startaddr;
	unsigned char startbuf[BUFMAX_LEN + 6] = { 0 };
	int startlen = 0;
	int ialready = 0;
	int ineedsend = isize;
	int isendlen = 0;

	while (ineedsend > 0)
	{
		isendlen = 0;

		memset(startbuf, 0, sizeof(startbuf));
		//设置擦除的首地址
		startbuf[0] = start >> 24 & 0xFF;
		startbuf[1] = start >> 16 & 0xFF;
		startbuf[2] = start >> 8 & 0xFF;
		startbuf[3] = start & 0xFF;

		
		if (ineedsend > BUFMAX_LEN)
		{
			isendlen = BUFMAX_LEN;
		}
		else if (ineedsend > 0)
		{
			isendlen = ineedsend;
		}
		startbuf[4] = isendlen >> 8 & 0xFF;
		startbuf[5] = isendlen & 0xFF;
		memcpy(startbuf + 6, buf + ialready, isendlen);
		
		pop_buildpacket(pobex, OS_FLASHOPERA, 0x01, serialno, startbuf, isendlen + 6);

		iRet = pop_sendrecvpacket(pobex, isendlen + 6 + MIN_POPPACKETLEN);
		if (iRet != 0)
			goto EXIST;

		start += BUFMAX_LEN;
		serialno++;
		ineedsend -= isendlen;
		ialready += isendlen;

		SendMessage(hwnd, WM_MESSAGEINFO, P_PROGRESS, ialready * 100 / isize);
	}

EXIST:

	if (pobex != NULL)
	{
		free(pobex);
		pobex = NULL;
	}

	return iRet;
}

//flash 写入
int  mhcommethod::writeflash(unsigned int startaddr, unsigned char* buf, int isize)
{
	int iRet = 0;
	int serialno = 0;
	OBEX *pobex = (OBEX*)malloc(POPPACKET_LEN + 6); //
	unsigned int start = startaddr;
	unsigned char startbuf[BUFMAX_LEN + 6] = { 0 };
	int startlen = 0;
	int ialready = 0;
	int ineedsend = isize;
	int isendlen = 0;

	while (ineedsend > 0)
	{
		isendlen = 0;

		memset(startbuf, 0, sizeof(startbuf));
		//设置擦除的首地址
		startbuf[0] = start >> 24 & 0xFF;
		startbuf[1] = start >> 16 & 0xFF;
		startbuf[2] = start >> 8 & 0xFF;
		startbuf[3] = start & 0xFF;


		if (ineedsend > BUFMAX_LEN)
		{
			isendlen = BUFMAX_LEN;
		}
		else if (ineedsend > 0)
		{
			isendlen = ineedsend;
		}
		startbuf[4] = isendlen >> 8 & 0xFF;
		startbuf[5] = isendlen & 0xFF;
		memcpy(startbuf + 6, buf + ialready, isendlen);

		pop_buildpacket(pobex, OS_FLASHOPERA, 0x01, serialno, startbuf, isendlen + 6);

		iRet = pop_sendrecvpacket(pobex, isendlen + 6 + MIN_POPPACKETLEN);
		if (iRet != 0)
			goto EXIST;

		start += BUFMAX_LEN;
		serialno++;
		ineedsend -= isendlen;
		ialready += isendlen;

	}

EXIST:

	if (pobex != NULL)
	{
		free(pobex);
		pobex = NULL;
	}

	return iRet;
}


int  mhcommethod::GetProduceResult(char* szResult, int* len)
{
	int recvlen = 0;
	char recvbuf[1024] = { 0 };
	int iRet = 0;

	char sendbuf[] = "1";
	pop_clearbaddata();
	iRet = s_FrameTx(0x0A, (unsigned char* )sendbuf, strlen(sendbuf));
	if (iRet != 0)
		return iRet;

	Sleep(1000);

	iRet = s_FrameRx((unsigned char* )recvbuf, &recvlen, 6 * 1000);
	if (iRet != 0)
		return iRet;
	
	memcpy(szResult, recvbuf + 1, recvlen - 3);
	*len = recvlen - 3;
	return 0;
}

int  mhcommethod::s_FrameRx(unsigned char* recvbuf, int* recvlen, int timeout)
{
	//注意接收的数据不要大于1024*2
	unsigned char getData[1024 * 2] = { 0 };
	unsigned int start, end, index = 0, lrc = 0;
	unsigned char*p;
	int i, iRet;
	int iloop = 0;

	start = GetTickCount();
	p = getData;

	while (1)
	{
		end = GetTickCount();
		if ((int)(end - start) > timeout)
			return -1001;

		if (iloop == 0)
		{
			iRet = m_Port.Readv((char*)p + index, 1, 1000);
			if (iRet != 1)
				continue;

			if (getData[0] != 0x0A && getData[0] != 0x0B && getData[0] != 0x0C)
				continue;

			index++;
			iloop = 1;
		}

		iRet = m_Port.Readv((char*)p + index, 1, 1000);
		if (iRet != 1)
			continue;
		
		index++;	

		if (index > 2 && getData[index - 1] == 0x03)
		{
			for (i = 1; i<(int)index - 2; i++)
			{
				lrc ^= getData[i];
			}

			if (lrc == getData[index - 2])
				break;
		}
	}

	memcpy(recvbuf, getData, index);
	*recvlen = index;
	return 0;
}

int  mhcommethod::s_FrameTx(unsigned char soh, unsigned char* sendbuf, int sendlen)
{
	char tmpsendBuf[2 * 1024];
	int iRet, lrc = 0;
	int i;

	tmpsendBuf[0] = soh;
	memcpy(tmpsendBuf + 1, sendbuf, sendlen);
	for (i = 0; i<sendlen; i++)
	{
		lrc ^= sendbuf[i];
	}

	tmpsendBuf[sendlen + 1] = lrc;
	tmpsendBuf[sendlen + 2] = 0x03;

	iRet = m_Port.Writev((char* )tmpsendBuf, sendlen + 3, 2000);  // send head
	if (iRet != sendlen + 3)
		return -1;

	return 0;
}

int  mhcommethod::produceSN(unsigned char soh, char* sn)
{
	int iRet, errcode = 0;
	int recvlen = 0;
	char recvbuf[1024] = { 0 };
	char time[100] = { 0 };
	unsigned char senddata[24+14] = { 0 };

	if (strlen(sn) > 24)
		return -100;

	pop_clearbaddata();
	gettime(time);
	memcpy(senddata, time, 14);

	memcpy(senddata+14, sn, strlen(sn));
	iRet = s_FrameTx(soh, senddata, sizeof(senddata));
	if (iRet != 0)
		return -102;

	Sleep(2000);
	iRet = s_FrameRx((unsigned char*)recvbuf, &recvlen, 10*1000);
	hslog::loghex((unsigned char*)recvbuf, recvlen);
	if (iRet != 0 || recvlen != 7)
		return -103;

	memcpy(&errcode, recvbuf + 1, 4);
	return errcode;
}

void  mhcommethod::throughcom0()
{
	DWORD iexit = 0;
	USES_CONVERSION;

	CString batpath = "";
	GetModuleFileName(NULL, batpath.GetBuffer(MAX_PATH), MAX_PATH);
	batpath.ReleaseBuffer();
	int pos = batpath.ReverseFind(_T('\\'));
	batpath = batpath.Left(pos);

	SHELLEXECUTEINFO   ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = "TransferAPSP.bat";
	ShExecInfo.lpParameters = NULL;
	ShExecInfo.lpDirectory = batpath;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);

	// 等待脚本返回  
	WaitForSingleObject(ShExecInfo.hProcess, 3000);

	GetExitCodeProcess(ShExecInfo.hProcess, &iexit);
	if (iexit == STILL_ACTIVE)
	{
		HWND hWnd = ::FindWindow(_T("ConsoleWindowClass"), NULL);
		if (hWnd)
		{
			::PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
	}
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	Sleep(1000);
}

void mhcommethod::gettime(char* time)
{
	CTime Time;
	Time = CTime::GetCurrentTime();
	sprintf(time, "%04d%02d%02d%02d%02d%02d", Time.GetYear(), Time.GetMonth(), Time.GetDay(), Time.GetHour(), Time.GetMinute(), Time.GetSecond());
}
