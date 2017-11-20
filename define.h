#ifndef DETECT_DEFINE
#define DETECT_DEFINE


// CEMHDetectDlg ¶Ô»°¿ò
#define		MAX_EMH_FILE_SIZE		48640		//47.5KB
#define		DETECT_COLUMN_ITEM_NAME				0
#define		DETECT_COLUMN_STATE					1
#define		DETECT_COLUMN_FAILURE_REASON		2


typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;


#define	MAX_FILE_LENGTH		(48 * 1024)
#define	FILE_TOO_BIG		(-1)
#define	FILE_ERROR_FORMAT	(-2)
#define	NOT_ENOUGH_MEMORY	(-3)
#define SUCCESS				0


#define		WM_DOWNLOAD_MESSAGE		(WM_USER + 100)
#define		WM_DAPARAM_MESSAGE		(WM_USER + 101)

#endif