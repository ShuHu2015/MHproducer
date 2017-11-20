#pragma once
#include "RSA\rsaref.h"
#include "ctb-0.16\serport.h"
#include <queue>
#include <list> 
#include "DlgProduce.h"
using namespace std;


#pragma warning(disable:4200) //zero-sized array in struct/union

#define		MAX_SUPPORT_REG_NUM		32				//MH1901, MH1901B֧��128���Ĵ��������� MH1901Cֻ֧��32��
#define		DIR_UP					0xAA
#define		DIR_DOWN				0x01
#define		MIN_PACKET_LEN			6
#define		MAX_PACKET_LEN			(16 * 1024)
#define		RSA_LEN_2048			(256 + 8)		//MH1901BоƬ����ҪRR, 520
#define		RSA_LEN_4096			1032
#define		ACK						0x28
#define		NACK					0x29

#define		FILE_VALID_FLAG			0x5555AAAAL
#define		FLASH_BASE_ADDR			0x01000000L
#define		FLASH_SIZE				(1 * 1024 * 1024)
#define		FLASH_PROTECT_UNIT		(32 * 1024)
#define		FLASH_SECTOR_SIZE		(4 * 1024)

#define		MAX_FILE_SEG_SIZE		(4 * 256)		//MH1901,MH1901BоƬ֧��8KB�����ݰ���MH1901C���֧��1KB�����ݰ�

//RSA��Կ�ļ��ֶ�����
#define		RSA_FILE_HEAD			"Megahunt SCPU key config file"
#define		RSA_FILE_KEY_LEN		"Key length"
#define		RSA_FILE_PRI_KEY_MOD	"pub_key n"
#define		RSA_FILE_PUB_KEY_EXP	"pub_key e"
#define		RSA_FILE_PRI_KEY_EXP	"pri_key d"
#define		RSA_FILE_PRI_KEY_PRI1	"pri_key p"
#define		RSA_FILE_PRI_KEY_PRI2	"pri_key q"
#define		RSA_FILE_PRI_KEY_PRI_EXP1	"pri_key dp"
#define		RSA_FILE_PRI_KEY_PRI_EXP2	"pri_key dq"
#define		RSA_FILE_PRI_KEY_COEF	"pri_key pq"
#define		RSA_FILE_RR_MOD_M	"rr_mod_m"

//Bin�ļ�ͷ
#define		BIN_HEAD_NOS			"MHNOS_B"	//��ǩ���ļ�
#define		BIN_HEAD_SIG			"MHSIG_B"	//��ǩ���ļ�

#define		SUPER_USER_MASK			0xC000
#define		SUPER_USER_MODE			0x4000

enum HASH_TYPE
{
	HASH_TYPE_NONE = 0,
	HASH_TYPE_256 = 3,
	HASH_TYPE_512 = 5
};

enum SCPU_DL_OPT {
	SCPU_DL_NO_OPERATION = 0,		//���ع����в�д��������
	SCPU_DL_WRITE_PARAM = 0x55		//���ع�����д��׶β����������׶�
};

enum DLStateMsg {
	DL_STATE_C_SN,				//��ȡ���������õ�SN
	DL_STATE_E_SN,				//��ȡ���ͻ����õ�SN
	DL_STATE_MSG_STRING,		//���ַ����ķ�ʽ֪ͨ����
	DL_STATE_WRITE_SN,			//��������SN�󷵻ص�״̬
	DL_STATE_WRITE_FILE_INFO,	//�ͻ�����SN�󷵻ص�״̬
	DL_STATE_ERASE_FLASH,		//����Flash����
	DL_STATE_WRITE_FILE_CONTENT,//д�ļ�����
	DL_STATE_FINISHED			//��̨�߳̽���
};



typedef struct PacketBuf_s
{
	int		len;
	unsigned char buf[16 * 1024];
}PacketBuf;

#pragma pack(push, 1)	//�����ݽṹ��Ϊ���ֽڶ���

typedef struct Reg_s
{
	unsigned int		addr;
	unsigned int		mask;
	unsigned int		pointer;		//Ϊ0ʱ��ʾ��ȡ�����ֵ
	unsigned int		val;
}Reg;

typedef struct SN_s
{
	unsigned char v[16];
}SN;

typedef struct DLSNParam_s	//��ʼ����ʱ���ݵĲ���
{
	char		stage;
	char		opt;		//SCPU_DL_OPT
	union
	{
		struct {
			SN					sn;		//оƬ���
			unsigned short		time;
			unsigned short		scratch;	//BIT15 ÿ�����س���ʱ��������, BIT14 �Զ�����̼��汾
			unsigned short		regNum;		//������һ����Ϣ���
			unsigned short		info;		//BIT15Ϊ1��ʾRSA-4096, �����ʾRSA-2048
			char				buf[2048];	//RSA��Կ(RR(MH1901), MOD, e)�ͼĴ�����Ϣ, ��һ��Ĵ������õ�addr��FILE_VALID_FLAG, ��һ��Ĵ������õ�val�Ǻ�����ʵ�Ĵ���ֵ��CRC32ֵ
		}param;
		unsigned short ver;
	}content;
}DLSNParam;

typedef struct DLFileParam_s
{
	char				refresh;			//�Ƿ����µ�����
	unsigned char		eraseBitmap[32];	//��Ҫ����������λͼ
	unsigned int		protectFlag;		//Flash �������, ��bit��Ӧ32KB��Flash
	unsigned int		valid;				//�̶�Ϊ0x5555AAAA
	union {
		char v[256];		//MH1901B��Ϊ256B,MH1901Ϊ512B
		struct {
			unsigned int	reserved;			//MH1901C��reserved��len�����Ƶ���startǰ��
			unsigned int	start;				//MH1901B��start��len��ǩ��, MH1901����
			unsigned int	len;
			short			ver;				//��ǰ����汾
			short			opt;				//0��hash, 3 hash256, 5 hash512
			char			hash[64];
			unsigned int	crc;				//��valid��ʼ, ��sig.buf����
		}s;
	}sig;
	//unsigned short		crc;
}DLFileParam;

typedef struct DLFileSeg_s
{
	unsigned int		unlock;
	unsigned int		addr;
	char				buf[0];
	//...
	//unsigned short	crc;
}DLFileSeg;

#pragma pack(pop)

typedef struct Packet_s
{
	char			dir;	//����0xAA, ����0x01
	char			step;
	unsigned short	len;	//From dir to content
	union {
		char buf[1024];
		struct {
			char	stage;
			SN		sn;
			unsigned short	time;
			unsigned short	info;
			unsigned short	ver;
		}chipInfo;
		DLSNParam	snParam;
		DLFileParam	fileParam;
		DLFileSeg	seg;
	}content;

}Packet;

typedef struct DThreadParam_s {
	HANDLE		hThread;
	HANDLE		hMutex;			//�����������̵߳Ļ��������
	HWND		hWnd;			//���ؾ��
	char        szcom[10];		//������
	char		filetype;
	bool		isAbort;		//�Ƿ��Ѿ���ֹ����
	bool		isDownload;		//�����Ϻ��Ƿ��Ѿ���ʼ����
	char		filePath[1024];
	DLSNParam	snParam;
	//BOOL		hasKey;
	R_RSA_PRIVATE_KEY key;
	unsigned char rr_mod_m[512];
}DThreadParam;

typedef struct FileInfo_s
{
	unsigned int startAddr;
	int		fileLen;
	union {
		char	v[1024];
		DLFileParam param;
	}info;
	char	buf[1024 * 1024];
}FileInfo;

//�������
typedef enum 
{
	COMOPENFAIL_ERR = 256,		//�����豸��ʧ��
	SHAKETIMEOUT_ERR,			//���ֳ�ʱ
	SNWRITEFAIL_ERR,			//SN����д��ʧ��
	COMMUFAIL_ERR,				//ͨѶ����
	LRC_ERR,					//lrcУ�����
	LOADFILE_ERR,				//�ļ�����ʧ��
	NOTSUPPORT_ERR,				//��֧��
	PUBRSAFILE_ERR,				//RSA��Կ��ȡʧ��
	DATA_ERR,					//���ݴ���
	PATAM_ERR,					//��������
	USERCANCEL_ERR,				//�û�ȡ��
	FILEREAD_ERR,				//�ļ���ȡʧ��
	FILEOPEN_ERR,				//�ļ���ʧ��
}ERRCODE;

//��Կ����
typedef enum
{
	MFK = 1, MAK, MEKK,
}KEYTYPE;

//�ļ�����
typedef enum
{
	BOOT = 1, LVOS, SYSPARAM, TM, SN1STR, SN2STR, PUK, FONT, APP, SUBFILE ,PACKAGE, BKIMG, CONFIG,
}FILETYPE;

//��������� ����������Ȩ����ǩ������
typedef enum
{
	TASK_POPLOAD = 1, TASK_AUTH, TASK_SIGN
}TASKTYPE;

//��λ��������
typedef enum
{
	CMD_BOOT, CMD_LVOS, CMD_FBOOT, OS_FLASHOPERA = 0x14, OS_FONT, OS_BKIMG,
}CMDH;

//BOOT��λ��������,�����ʱ����
typedef enum
{
	BOOT_START,			BOOT_ING,			BOOT_END,
	LVOS_START,			LVOS_ING,			LVOS_END,
	APP_START,			APP_ING,			APP_END,
	CMD_SN1,			CMD_SN2,			CMD_PUK,	CMD_CONFIG,
	AUTH_CHCODE = 0x30, AUTH_PRODUCE,		AUTH_FIRMDEBUG,
	AUTH_FIRMCLEAR,		AUTH_SOFTDEBUG,		AUTH_SOFTCLEAR,
	AUTH_RECOVER,		AUTH_CLEARMESH,		AUTH_SN1,
	AUTH_SN2,
}CMDL;

//OS��λ��������,�����ʱ����
typedef enum
{
	OS_GETTIME,		OS_SETTIME,			OS_GETRANDOM,	    OS_GETMESHSTUTAS,	 OS_RESET,		     OS_GETBOOTVER = 0xe0,
	OS_GETOSVER,	OS_GETPID,			OS_GETPUKINFO,      OS_GETSTUTAS = 0xe5, OS_GETFDEBUGNUM,   OS_GETADEBUGNUM,
	OS_GETPUKNUM,	OS_GETPUKLEVEL,		OS_GETOSSIGNINFO,	OS_GETSN1,		     OS_GETSN2,			OS_GETPUK,		
	OS_ALL = 0xFF
}OSCMDL;

//P_INIT ��ʼ���� P_INFO ��ʾ��ʾ��Ϣ���ѣ� P_PROGRESS��ʾ���ȣ�P_RESULT��ʾ��������
typedef enum
{
	P_INIT, P_INFO, P_PROGRESS, P_QUERY,  P_RESULT, P_END
}MESSAGETYPE;

//��������
typedef enum
{
	D_STR, D_INT, D_OBJ,
}DATATYPE;

//�ձ����أ���Ȩ��ǩ������
typedef struct _TASKDATA
{	
	char        szcom[10];				//������ 
	int			id;						//��������Ȩ������CMDL��Ҳ������ǩ����Կ����KEYTYPE
	BYTE		tasktype;
	BYTE		cmdh;
	BYTE	    cmdl;
	char		ack;
	HWND		hwnd;	
	TCHAR		szpukpath[MAX_PATH];	//puk �ļ�·��������Կ����
	TCHAR		szFilePath[MAX_PATH];
}TASKDATA;

//�ļ�ͷ��Ϣ
typedef struct
{
	unsigned int stack;
	void(*entry)(void *);
	unsigned int  text_len;    // �̼�����
	char name[32];    // �̼�����
	char PID[16];   // ��Ʒ�ͺ�
	char version[8]; // �汾��
	char reserve[64]; // Ԥ��  
}ST_HEADER_INFO;

#define SIGN_MAGIC "RSASIGN"
typedef struct
{
	unsigned char magic[7];
	unsigned char SignVer[2];
	unsigned char HashAlg;
	unsigned char Hash[32];
	unsigned char SignCompany[16];
	unsigned char SignUser[10];
	unsigned char SignTime[8];
	unsigned char SignInvalidDate[6];
	unsigned char reserve[174];
}SIGN_INFO;

//�������������
class PRODUCETASK
{
public:
	PRODUCETASK() { m_hwnd = NULL;  m_devname = ""; }
	PRODUCETASK(HWND hwnd, CString devname) { m_hwnd = hwnd; m_devname = devname; }
	~PRODUCETASK() {}

	bool operator == (const PRODUCETASK& rhs) ;
	HWND m_hwnd;
	CString m_devname;
	CString m_devpath;
	CString m_pukpath;
	CString m_bootpath;
	CString m_ospath;
};

//���͸����ڵİ�
typedef struct _RESPPACK
{
	union
	{
		char buffer[1024];
		struct {
			char devname[24];
			char reserve[1000];			
		};
	}buf;
}RESPPACK;

//û���ظ�Ԫ�صĶ���
template<class T>
class QueueNoDup : public queue<T>
{
public:
	void push(const value_type& x)
	{
		if (!isduplicate(x))
		{
			//û�ҵ���Ԫ�������
			__super::push(x);
		}
	}

	bool isduplicate(const value_type& x)
	{
		if (find(c.begin(), c.end(), x) == c.end())
		{
			return false;
		}
		return true;
	}
};


class mhcommethod;
class mhservice
{
public:
	mhservice();
	~mhservice();
	
	//��ʼ��Ϊ�Զ����os cos sbi�ȵĸ��·���
	int		initialservice(int inum);

	void	stopalltask() { m_allstop = true; }

	//�Ƿ���������
	int		m_resetcmd;

	//�Ƿ�ת������
	int     m_portconvert;

	//������л�����
	CMutex  m_queuemutex;
	
	HANDLE m_seg;

	//�������
	QueueNoDup <PRODUCETASK > mWorkQueue;

	//�����е��������
	list <CString > mMidTask;

	//��������������
	std::queue <CString > mEndQueue;


	std::queue <TASKDATA> mTaskQueue;

	bool	ishandlingtask(CString devname);
	int		addtask(TASKDATA param);
	int		addhandlingtask(CString param);
	int     removehandlingtask(CString param);
	int		gettask(TASKDATA* Pparam);

	//��produce���ǹ�������ʹ�õ�
	//�����ź���
	//HANDLE  m_produceseg;

	static  DWORD _mhthreadproc(LPVOID lpParam);
	int		mhprocess();
	int		internalprocess(TASKDATA param);
	void	GetFileTypeFromId(BYTE type, char* devname)
	{
		switch (type)
		{
		case FILETYPE::BOOT:
			strcpy(devname,"BOOT");
			break;
		case FILETYPE::LVOS:
			strcpy(devname, "OS");
			break;
		case FILETYPE::SN1STR:
			strcpy(devname, "SN1");
			break;
		case FILETYPE::SN2STR:
			strcpy(devname, "SN2");
			break;
		case FILETYPE::PUK:
			strcpy(devname, "PUK");
			break;
		case FILETYPE::CONFIG:
			strcpy(devname, "CFG");
			break;
		default:
			return;
		}
		return;
	}
	void    throughcom0();

	int     authtaskhandler(mhcommethod* mhworkhandle, TASKDATA param);
	int     signtaskhandler(mhcommethod* mhworkhandle, TASKDATA param);
	int     signboot(mhcommethod* mhworkhandle, TASKDATA param);
	int     signpuk(mhcommethod* mhworkhandle, TASKDATA param);
	int     signnormalfile(mhcommethod* mhworkhandle, TASKDATA param);
	int     loadtaskhandler(mhcommethod* mhworkhandle, TASKDATA param);
	int     bootcmdhandler(mhcommethod* mhworkhandle,  TASKDATA param);
	int     lvoscmdhandler(mhcommethod* mhworkhandle,  TASKDATA param);
	int     flashutf8table(mhcommethod* mhworkhandle, TASKDATA param);
	int     flashfonttable(mhcommethod* mhworkhandle, TASKDATA param);
	int     flashbkimg(mhcommethod* mhworkhandle, TASKDATA param);

	static  bool m_allstop;
	
};

#define		BUFMAX_LEN				1024 * 3 //8192
#define		POPPACKET_LEN			BUFMAX_LEN + 8
#define		SOH						0x05
#define		MIN_POPPACKETLEN		7
#define		BOOTHANDSHAKE			'B'
#define		OSHANDSHAKE				'O'

#define		POP_SHAKETIMEOUT		10000
#define		WM_MESSAGEINFO			WM_USER + 1000
#define		WM_MESSAGESNINFO		WM_USER + 1001
#define		WM_MESSAGEFONTINFO		WM_USER + 1002		
#define		WM_MESSAGEBOOTOSINFO	WM_USER + 1004		//������������BOOT OS

//���ṹ
typedef struct OBEX_S
{
	unsigned char stx;
	unsigned char cmdh;
	unsigned char cmdl;
	unsigned char serialno;
	unsigned char len[2];		//���ݲ��ֵĳ��ȣ���� -- ��λ��ǰ
	unsigned char data[1];		//���ݲ���
}OBEX;

//��¼���û���Ϣ
typedef struct _USER_INFO
{
	char time[8]; /*YYMMDDHH*/
	char type[2]; /*auth type*/
	char userinfo[32];
}USER_INFO;

#define		offset(s, f) ((int)&(((s*)NULL)->f))
#define		PACKETSENDTIMEOUT	300
#define		PACKETTIMEOUT		4000
#define		BLOCKSIZE			4096		//�����Ĵ�С
#define     POP_FLASHSECTIONSIZE	4 * 1024	
#define     POP_FLASHBLOCKSIZE		64 * 1024

//typedef void(*ProgressHandler)(ProduceThread* , int);

class mhcommethod
{
public:
	mhcommethod();
	~mhcommethod();
	mhcommethod(const mhcommethod& rhs);

	int open(char* szportname);
	int ReceivePacket();

	//��˾�Զ���İ���ʽ
	void pop_buildpacket(void* packet, unsigned char cmdh, unsigned char cmdl, unsigned char serialno, void* data, int datalen);
	int  pop_receivepacket(DWORD timeout);
	void pop_sendpacket(void *packet, int len);
	int  pop_sendrecvpacket(void *packet, int len);
	void SendPacket(char step, void *buf, int len);
	void SendPacketTimeout(char step, void *buf, int len, int timeout);
	int  pop_handshake(char ack);
	int  pop_handshake(char ack, int timeout);
	void pop_clearbaddata();
	int  pop_filehandler(HWND hwnd, unsigned char start, char* startbuf, int startlen, char* buf, int len, bool isreport);
	int  pop_getchallengecode(USER_INFO info, unsigned char* chcode);
	int  pop_sendlicsence(unsigned char* data, int datalen, int type);

	int  s_FrameRx(unsigned char* recvbuf, int* recvlen, int timeout);
	int  s_FrameTx(unsigned char soh, unsigned char* sendbuf, int sendlen);
	
	int  produceSN(unsigned char soh, char* sn);
	int  pop_bootdownload(TASKDATA param);
	int  pop_osdownload(TASKDATA param);
	int  pop_osdownload(CString ospath, HWND hwnd);
	int  pop_sn1downlaod(TASKDATA param);
	int  pop_sn1downlaod(CString sn1);
	int  pop_sn2downlaod(TASKDATA param);
	int  pop_sn2downlaod(CString sn2);
	int  pop_pukdownlaod(TASKDATA param);
	int  pop_configdownlaod(CString filepath);
	int  pop_appdownlaod(TASKDATA param);
	int  pop_productauthorize();

	int  pop_ossettime(char* time);
	int  pop_osgettime(char* time);
	int  pop_osgetrandom(int randomlen, byte* random);
	int  pop_osgetmeshstatus();
	int  pop_osreset();
	int  pop_osgetbootver(char* ver);
	int  pop_osgetosver(char* ver);
	int  pop_osgetpid(char* pid);
	int  pop_osgetpuksigndata(byte* signdata);
	int  pop_osgetdevstatus(byte* firmstatus, byte* appstatus);
	int  pop_osgetfdebugnum(byte* num);
	int  pop_osgetadebugnum(byte* num);
	int  pop_osgetclearpuk(byte* num);
	int  pop_osgetpuklevel(byte* level);
	int  pop_osgetossigndata(byte* signdata);
	int  pop_osgetsn1(char* sn1str);
	int  pop_osgetsn2(char* sn2str);
	int  pop_osgetpuk(byte* pukdata);
	int  pop_getdeviceinfo(CString& result);

	//flash operation
	int  eraseblockflash(unsigned int startaddr, int isize);
	int  writeflash(HWND hwnd, unsigned int startaddr, unsigned char* buf, int isize);
	int  writeflash(unsigned int startaddr, unsigned char* buf, int isize);
	//int  writeflash(unsigned int startaddr, unsigned char* buf, int isize, int index, ProgressHandler handle);

	DWORD GetDiffTime(DWORD t);
	int   GetAuthNo(int type);
	int   GetProduceResult(char* szResult);
	int   GetProduceResult(char* szResult, int* len);
	//����lrc
	unsigned char lrc(unsigned char* buf, int len);

	int  loadfile(FileInfo *pInfo, char* path);
	int  ConvertFile(FileInfo *pInfo, char *path, const unsigned int startAddr, const unsigned short hashType, const short ver);
		 
	int  stepconnecthandler(DThreadParam& param, int timeout);
	int  stepwritesnhandler(DThreadParam& param, int timeout);
	int  stepwritefilehandler(DThreadParam& param, FileInfo *pInfo, int timeout);
	int  stepwritefiledatahandler(FileInfo *pFileInfo, int timeout);
	int  stepverifydatahandler(int timeout);

	int  LoadRSAPublicKeyFile(char *path, R_RSA_PUBLIC_KEY *pkey);
	int  LoadRSAFile(char *path, R_RSA_PRIVATE_KEY *pkey, unsigned char *rr);
	void GetErasePages(unsigned char* pPages, int blocknum);

	void sendmessage(HWND hwnd, HANDLE mutex, WPARAM wparam, LPARAM lparam);
	void throughcom0();
	void gettime(char* time);

public:
	ctb::SerialPort m_Port;
	PacketBuf_s*	m_packet;
	byte			m_serialno;			//when i need to use os command, i use m_serialno to control the serial number
										//but when i use boot command, i usuall use handshake first and initial m_serialno
};

