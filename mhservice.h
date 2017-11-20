#pragma once
#include "RSA\rsaref.h"
#include "ctb-0.16\serport.h"
#include <queue>
#include <list> 
#include "DlgProduce.h"
using namespace std;


#pragma warning(disable:4200) //zero-sized array in struct/union

#define		MAX_SUPPORT_REG_NUM		32				//MH1901, MH1901B支持128个寄存器补丁， MH1901C只支持32组
#define		DIR_UP					0xAA
#define		DIR_DOWN				0x01
#define		MIN_PACKET_LEN			6
#define		MAX_PACKET_LEN			(16 * 1024)
#define		RSA_LEN_2048			(256 + 8)		//MH1901B芯片不需要RR, 520
#define		RSA_LEN_4096			1032
#define		ACK						0x28
#define		NACK					0x29

#define		FILE_VALID_FLAG			0x5555AAAAL
#define		FLASH_BASE_ADDR			0x01000000L
#define		FLASH_SIZE				(1 * 1024 * 1024)
#define		FLASH_PROTECT_UNIT		(32 * 1024)
#define		FLASH_SECTOR_SIZE		(4 * 1024)

#define		MAX_FILE_SEG_SIZE		(4 * 256)		//MH1901,MH1901B芯片支持8KB的数据包，MH1901C最大支持1KB的数据包

//RSA密钥文件字段名称
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

//Bin文件头
#define		BIN_HEAD_NOS			"MHNOS_B"	//无签名文件
#define		BIN_HEAD_SIG			"MHSIG_B"	//有签名文件

#define		SUPER_USER_MASK			0xC000
#define		SUPER_USER_MODE			0x4000

enum HASH_TYPE
{
	HASH_TYPE_NONE = 0,
	HASH_TYPE_256 = 3,
	HASH_TYPE_512 = 5
};

enum SCPU_DL_OPT {
	SCPU_DL_NO_OPERATION = 0,		//下载过程中不写其他参数
	SCPU_DL_WRITE_PARAM = 0x55		//下载过程中写入阶段参数并升级阶段
};

enum DLStateMsg {
	DL_STATE_C_SN,				//读取到厂商配置的SN
	DL_STATE_E_SN,				//读取到客户配置的SN
	DL_STATE_MSG_STRING,		//用字符串的方式通知界面
	DL_STATE_WRITE_SN,			//厂商配置SN后返回的状态
	DL_STATE_WRITE_FILE_INFO,	//客户配置SN后返回的状态
	DL_STATE_ERASE_FLASH,		//擦除Flash扇区
	DL_STATE_WRITE_FILE_CONTENT,//写文件内容
	DL_STATE_FINISHED			//后台线程结束
};



typedef struct PacketBuf_s
{
	int		len;
	unsigned char buf[16 * 1024];
}PacketBuf;

#pragma pack(push, 1)	//将数据结构改为单字节对齐

typedef struct Reg_s
{
	unsigned int		addr;
	unsigned int		mask;
	unsigned int		pointer;		//为0时表示读取后面的值
	unsigned int		val;
}Reg;

typedef struct SN_s
{
	unsigned char v[16];
}SN;

typedef struct DLSNParam_s	//开始下载时传递的参数
{
	char		stage;
	char		opt;		//SCPU_DL_OPT
	union
	{
		struct {
			SN					sn;		//芯片序号
			unsigned short		time;
			unsigned short		scratch;	//BIT15 每次下载程序时更新扰码, BIT14 自动管理固件版本
			unsigned short		regNum;		//包含第一组信息标记
			unsigned short		info;		//BIT15为1表示RSA-4096, 否则表示RSA-2048
			char				buf[2048];	//RSA密钥(RR(MH1901), MOD, e)和寄存器信息, 第一组寄存器配置的addr是FILE_VALID_FLAG, 第一组寄存器配置的val是后面真实寄存器值的CRC32值
		}param;
		unsigned short ver;
	}content;
}DLSNParam;

typedef struct DLFileParam_s
{
	char				refresh;			//是否用新的扰码
	unsigned char		eraseBitmap[32];	//需要擦除的扇区位图
	unsigned int		protectFlag;		//Flash 保护标记, 第bit对应32KB的Flash
	unsigned int		valid;				//固定为0x5555AAAA
	union {
		char v[256];		//MH1901B改为256B,MH1901为512B
		struct {
			unsigned int	reserved;			//MH1901C将reserved从len后面移到了start前面
			unsigned int	start;				//MH1901B对start和len做签名, MH1901不用
			unsigned int	len;
			short			ver;				//当前程序版本
			short			opt;				//0无hash, 3 hash256, 5 hash512
			char			hash[64];
			unsigned int	crc;				//从valid开始, 到sig.buf结束
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
	char			dir;	//上行0xAA, 下行0x01
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
	HANDLE		hMutex;			//窗口与下载线程的互斥锁句柄
	HWND		hWnd;			//下载句柄
	char        szcom[10];		//串口名
	char		filetype;
	bool		isAbort;		//是否已经终止下载
	bool		isDownload;		//连接上后是否已经开始下载
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

//错误代码
typedef enum 
{
	COMOPENFAIL_ERR = 256,		//串口设备打开失败
	SHAKETIMEOUT_ERR,			//握手超时
	SNWRITEFAIL_ERR,			//SN参数写入失败
	COMMUFAIL_ERR,				//通讯错误
	LRC_ERR,					//lrc校验错误
	LOADFILE_ERR,				//文件加载失败
	NOTSUPPORT_ERR,				//不支持
	PUBRSAFILE_ERR,				//RSA公钥获取失败
	DATA_ERR,					//数据错误
	PATAM_ERR,					//参数错误
	USERCANCEL_ERR,				//用户取消
	FILEREAD_ERR,				//文件读取失败
	FILEOPEN_ERR,				//文件打开失败
}ERRCODE;

//密钥类型
typedef enum
{
	MFK = 1, MAK, MEKK,
}KEYTYPE;

//文件类型
typedef enum
{
	BOOT = 1, LVOS, SYSPARAM, TM, SN1STR, SN2STR, PUK, FONT, APP, SUBFILE ,PACKAGE, BKIMG, CONFIG,
}FILETYPE;

//任务的类型 下载任务，授权任务，签名任务
typedef enum
{
	TASK_POPLOAD = 1, TASK_AUTH, TASK_SIGN
}TASKTYPE;

//高位命令类型
typedef enum
{
	CMD_BOOT, CMD_LVOS, CMD_FBOOT, OS_FLASHOPERA = 0x14, OS_FONT, OS_BKIMG,
}CMDH;

//BOOT低位命令类型,组包的时候用
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

//OS低位命令类型,组包的时候用
typedef enum
{
	OS_GETTIME,		OS_SETTIME,			OS_GETRANDOM,	    OS_GETMESHSTUTAS,	 OS_RESET,		     OS_GETBOOTVER = 0xe0,
	OS_GETOSVER,	OS_GETPID,			OS_GETPUKINFO,      OS_GETSTUTAS = 0xe5, OS_GETFDEBUGNUM,   OS_GETADEBUGNUM,
	OS_GETPUKNUM,	OS_GETPUKLEVEL,		OS_GETOSSIGNINFO,	OS_GETSN1,		     OS_GETSN2,			OS_GETPUK,		
	OS_ALL = 0xFF
}OSCMDL;

//P_INIT 初始化， P_INFO 显示提示信息而已， P_PROGRESS显示进度，P_RESULT显示任务处理结果
typedef enum
{
	P_INIT, P_INFO, P_PROGRESS, P_QUERY,  P_RESULT, P_END
}MESSAGETYPE;

//数据类型
typedef enum
{
	D_STR, D_INT, D_OBJ,
}DATATYPE;

//普遍下载，授权，签名任务
typedef struct _TASKDATA
{	
	char        szcom[10];				//串口名 
	int			id;						//可以是授权子命令CMDL，也可以是签名密钥类型KEYTYPE
	BYTE		tasktype;
	BYTE		cmdh;
	BYTE	    cmdl;
	char		ack;
	HWND		hwnd;	
	TCHAR		szpukpath[MAX_PATH];	//puk 文件路径或者密钥名称
	TCHAR		szFilePath[MAX_PATH];
}TASKDATA;

//文件头信息
typedef struct
{
	unsigned int stack;
	void(*entry)(void *);
	unsigned int  text_len;    // 固件长度
	char name[32];    // 固件名称
	char PID[16];   // 产品型号
	char version[8]; // 版本号
	char reserve[64]; // 预留  
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

//生产服务的任务
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

//发送给窗口的包
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

//没有重复元素的队列
template<class T>
class QueueNoDup : public queue<T>
{
public:
	void push(const value_type& x)
	{
		if (!isduplicate(x))
		{
			//没找到此元素则添加
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
	
	//初始化为自定义的os cos sbi等的更新服务
	int		initialservice(int inum);

	void	stopalltask() { m_allstop = true; }

	//是否发重启命令
	int		m_resetcmd;

	//是否转换串口
	int     m_portconvert;

	//任务队列互斥量
	CMutex  m_queuemutex;
	
	HANDLE m_seg;

	//任务队列
	QueueNoDup <PRODUCETASK > mWorkQueue;

	//处理中的任务队列
	list <CString > mMidTask;

	//处理完的任务队列
	std::queue <CString > mEndQueue;


	std::queue <TASKDATA> mTaskQueue;

	bool	ishandlingtask(CString devname);
	int		addtask(TASKDATA param);
	int		addhandlingtask(CString param);
	int     removehandlingtask(CString param);
	int		gettask(TASKDATA* Pparam);

	//带produce的是工厂生产使用的
	//任务信号量
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
#define		WM_MESSAGEBOOTOSINFO	WM_USER + 1004		//用于生产下载BOOT OS

//包结构
typedef struct OBEX_S
{
	unsigned char stx;
	unsigned char cmdh;
	unsigned char cmdl;
	unsigned char serialno;
	unsigned char len[2];		//数据部分的长度，大端 -- 高位在前
	unsigned char data[1];		//数据部分
}OBEX;

//登录的用户信息
typedef struct _USER_INFO
{
	char time[8]; /*YYMMDDHH*/
	char type[2]; /*auth type*/
	char userinfo[32];
}USER_INFO;

#define		offset(s, f) ((int)&(((s*)NULL)->f))
#define		PACKETSENDTIMEOUT	300
#define		PACKETTIMEOUT		4000
#define		BLOCKSIZE			4096		//扇区的大小
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

	//公司自定义的包格式
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
	//计算lrc
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

