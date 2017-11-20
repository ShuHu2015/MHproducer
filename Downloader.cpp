// Downloader.cpp : 实现文件
//

#include "stdafx.h"
#include "MHproducer.h"
#include "Downloader.h"
#include "afxdialogex.h"
#include "mhservice.h"
#include "EnumSerial.h"
#include "hslog.h"
#include "utility.h"

// CDownloader 对话框

extern mhservice	g_service;

IMPLEMENT_DYNAMIC(CDownloader, CDialogEx)

CDownloader::CDownloader(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGDOWNLOAD, pParent)
	, m_sn1str(_T(""))
	, m_sn2str(_T(""))
{

}

CDownloader::~CDownloader()
{
}

void CDownloader::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_LIST2, m_filelist);
	DDX_Control(pDX, IDC_EDIT2, m_tips);
	DDX_Control(pDX, IDC_BTNDOWN, m_btndownload);
	DDX_Control(pDX, IDC_COMBO1, m_validports);
	DDX_Control(pDX, IDC_BTNQUERY, m_devinfo);
	DDX_Text(pDX, IDC_EDIT4, m_sn2str);
	DDX_Text(pDX, IDC_EDIT3, m_sn1str);
	DDX_Control(pDX, IDC_CHECKSN1, m_sn1check);
	DDX_Control(pDX, IDC_CHECKSN2, m_sn2check);
	DDX_Control(pDX, IDC_CHECKFILE, m_checkfile);
	DDX_Control(pDX, IDC_BTNSEIALCHG, m_transfer);
	DDX_Control(pDX, IDC_CHECKRESTARTSP, m_restartsp);
	DDX_Control(pDX, IDC_CHECKRESTARTSP2, m_checkcvtport);
	DDX_Control(pDX, IDC_EDIT1, m_deviceinfo);
	DDX_Control(pDX, IDC_BTNFONT, m_downfont);
	DDX_Control(pDX, IDC_BTNBKIMG, m_downbkimg);
	DDX_Control(pDX, IDC_BTNSTOP, m_stop);
}


BEGIN_MESSAGE_MAP(CDownloader, CDialogEx)
	ON_MESSAGE(WM_MESSAGEINFO, OnMessageHandler)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTNADD, &CDownloader::OnBnClickedBtnadd)
	ON_BN_CLICKED(IDC_BTNDEC, &CDownloader::OnBnClickedBtndec)
	ON_BN_CLICKED(IDC_BTNDOWN, &CDownloader::OnBnClickedBtndown)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BTNQUERY, &CDownloader::OnBnClickedBtnquery)
	ON_BN_CLICKED(IDC_BTNSEIALCHG, &CDownloader::OnBnClickedBtnseialchg)
	ON_BN_CLICKED(IDC_CHECKRESTARTSP, &CDownloader::OnBnClickedCheckrestartsp)
	ON_BN_CLICKED(IDC_CHECKRESTARTSP2, &CDownloader::OnBnClickedCheckrestartsp2)
	ON_COMMAND(IDOK, &CDownloader::OnIdok)
	ON_BN_CLICKED(IDC_BTNFONT, &CDownloader::OnBnClickedBtnfont)
	ON_BN_CLICKED(IDC_BTNSTOP, &CDownloader::OnBnClickedBtnstop)
	ON_BN_CLICKED(IDC_BTNBKIMG, &CDownloader::OnBnClickedBtnbkimg)
END_MESSAGE_MAP()


// CDownloader 消息处理程序


BOOL CDownloader::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_progress.SetShowPercent(FALSE);
	m_bkgbrush.CreateSolidBrush(RGB(192, 192, 192));

	updateports();
	loadhistory();

	std::string strtmp = "";
	utility::loadconfiginfo("DOWNUTFTABLE", strtmp);
	if (atoi(strtmp.c_str()) == 1)
	{
		m_transfer.ShowWindow(SW_SHOW);
	}
	strtmp = "";
	utility::loadconfiginfo("DOWNFONTTABLE", strtmp);
	if (atoi(strtmp.c_str()) == 1)
	{
		m_downfont.ShowWindow(SW_SHOW);
	}
	strtmp = "";
	utility::loadconfiginfo("DOWNBKIMG", strtmp);
	if (atoi(strtmp.c_str()) == 1)
	{
		m_downbkimg.ShowWindow(SW_SHOW);
	}
	strtmp = "";
	utility::loadconfiginfo("NEEDRESTART", strtmp);
	if (atoi(strtmp.c_str()) == 1)
	{
		m_restartsp.ShowWindow(SW_SHOW);
	}
	strtmp = "";
	utility::loadconfiginfo("NEEDTRANSFER", strtmp);
	if (atoi(strtmp.c_str()) == 1)
	{
		m_checkcvtport.ShowWindow(SW_SHOW);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

//接受和处理服务返回的信息
LRESULT CDownloader::OnMessageHandler(WPARAM wparam, LPARAM lparam)
{
	char info[1024] = { 0 };
	if (wparam == P_INFO)						//show a message from the service
	{
		m_tips.SetWindowTextA((char*)lparam);
		return 0;
	}

	if (wparam == P_RESULT)						//get a result of the handling task
	{
		RESPPACK *p = (RESPPACK *)lparam;
		sprintf(info, "%s %s; ", p->buf.devname, p->buf.reserve);
		m_tips.SetWindowTextA(info);
		m_resultmessage += info;
	}

	if (wparam == P_QUERY)
	{
		m_deviceinfo.SetWindowTextA((char*)lparam);
		m_tips.SetWindowTextA("SUCCESS");
		return 0;
	}

	if (wparam == P_PROGRESS)					//show the progress of the handling task
	{
		m_progress.SetPos(lparam);
		m_progress.Invalidate();
		return 0;
	}

	if (wparam == P_END)						// if there is a result,then show it, and we must enable the buttons.
	{
		m_tasknumhandle++;
		if (m_tasknumhandle == m_tasknumadd)
		{
			m_downbkimg.EnableWindow(TRUE);
			m_devinfo.EnableWindow(TRUE);
			m_btndownload.EnableWindow(TRUE);
			m_transfer.EnableWindow(TRUE);
			m_downfont.EnableWindow(TRUE);
			if(m_resultmessage.GetLength() != 0)		
				m_tips.SetWindowTextA(m_resultmessage);

			m_sn1str = "";
			m_sn2str = "";
			GetDlgItem(IDC_EDIT3)->SetFocus();
			UpdateData(FALSE);
			g_service.m_allstop = true;
		}
	}
	return 0;
}


BOOL CDownloader::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect clientRect;
	GetClientRect(&clientRect);
	pDC->FillRect(&clientRect, &m_bkgbrush);
	return TRUE;
	//return CDialogEx::OnEraseBkgnd(pDC);
}


HBRUSH CDownloader::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
	case CTLCOLOR_EDIT:
	case CTLCOLOR_BTN:
		if (pDC)
			pDC->SetBkColor(RGB(192, 192, 192));
		//return (HBRUSH)GetStockObject(NULL_BRUSH);
		return m_bkgbrush;
		break;
	default:
		break;
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

//添加文件
void CDownloader::OnBnClickedBtnadd()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog filedlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "All Files (*.*)|*.*||");
	if (filedlg.DoModal() == IDOK) 
	{
		if (m_filelist.FindString(0, filedlg.GetPathName()) != LB_ERR)
			return;

		m_filelist.AddString(filedlg.GetPathName());
	}
}

//移除选中的文件
void CDownloader::OnBnClickedBtndec()
{
	// TODO: 在此添加控件通知处理程序代码
	int sel = m_filelist.GetCurSel();
	if (sel < 0)
	{
		int itemp = m_filelist.GetCount();
		m_filelist.DeleteString(itemp - 1);
	}
	else
		m_filelist.DeleteString(sel);

}

//下载按钮
void CDownloader::OnBnClickedBtndown()
{
	// TODO: 在此添加控件通知处理程序代码
	
	UpdateData(TRUE);
	m_tasknumadd = 0;						//记录添加任务的个数
	m_tasknumhandle = 0;					//记录完成的任务的个数
	m_resultmessage = "";					//记录任务完成结果的字符串
	char snstr[256] = { 0 };
	m_progress.SetPos(0);
	m_progress.Invalidate();
	savehistory();

	if (m_sn1check.GetCheck())
	{
		TASKDATA data = { 0 };
		data.ack = BOOTHANDSHAKE;
		data.hwnd = this->m_hWnd;
		strcpy(data.szcom, getworkportname());
		memset(snstr, 0, sizeof(snstr));
		
		strcpy(data.szFilePath, m_sn1str.GetBuffer());
		m_sn1str.ReleaseBuffer();
		data.tasktype = TASKTYPE::TASK_POPLOAD;
		data.cmdh = CMDH::CMD_BOOT;
		data.cmdl = SN1STR;
		g_service.m_allstop = false;
		g_service.addtask(data);
		m_tasknumadd++;
	}
	if (m_sn2check.GetCheck())
	{
		TASKDATA data = { 0 };
		data.ack = BOOTHANDSHAKE;
		data.hwnd = this->m_hWnd;
		strcpy(data.szcom, getworkportname());
		strcpy(data.szFilePath, m_sn2str.GetBuffer());
		m_sn2str.ReleaseBuffer();
		data.tasktype = TASKTYPE::TASK_POPLOAD;
		data.cmdh = CMDH::CMD_BOOT;
		data.cmdl = SN2STR;
		g_service.m_allstop = false;
		g_service.addtask(data);
		m_tasknumadd++;

	}
	
	if (m_checkfile.GetCheck())
	{
		int filecount = m_filelist.GetCount();
		for (int i = 0; i < filecount; i++)
		{
			TASKDATA data = { 0 };
			data.ack = BOOTHANDSHAKE;
			data.hwnd = this->m_hWnd;
			strcpy(data.szcom, getworkportname());
			CString strtmp = "";
			m_filelist.GetText(i, strtmp);
			strcpy(data.szFilePath, strtmp);

			data.tasktype = TASKTYPE::TASK_POPLOAD;
			data.cmdh = CMDH::CMD_BOOT;
			data.cmdl = (FILETYPE)GetFileType(strtmp);
			if (GetFileType(strtmp) <= 0)
			{
				strtmp.Format("%s File Error", strtmp);
				AfxMessageBox(strtmp);
				return;
			}
			g_service.m_allstop = false;
			g_service.addtask(data);
			m_tasknumadd++;
		}
	}

	if (m_tasknumadd == 0)		//没添加任何任务
	{
		AfxMessageBox(_T("Please Select An Item"));
		return;
	}

	g_service.m_allstop = false;
	m_btndownload.EnableWindow(FALSE);
	m_devinfo.EnableWindow(FALSE);
	m_tips.SetWindowTextA("Please wait...");
}

//处理拖曳过来的文件
void CDownloader::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	char szFileName[MAX_PATH];
	int iFileNumber;

	// 得到拖拽操作中的文件个数
	iFileNumber = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < iFileNumber; i++)
	{
		// 得到每个文件名
		DragQueryFile(hDropInfo, i, szFileName, MAX_PATH);
		if (strlen(szFileName) == 0)
			continue;
		if (m_filelist.FindString(0, szFileName) != LB_ERR)
			continue;

		// 把文件名添加到list中
		m_filelist.AddString(szFileName);
	}
	
	CDialogEx::OnDropFiles(hDropInfo);
}

//获取某文件的类型
int CDownloader::GetFileType(CString filepath)
{
	CFile file;
	int filelen = 0;
	ST_HEADER_INFO headerinfo = { 0 };

	if (!file.Open(filepath, CFile::modeRead))
		return -1;

	filelen = (int)file.GetLength();
	if (filelen < sizeof(ST_HEADER_INFO))
		return -2;

	if (filelen > 1039 + sizeof(ST_HEADER_INFO))
	{
		char tmp[10] = { 0 };
		file.Read(tmp, 7);
		if (strcmp(tmp, "MHNOS_B") == 0)
		{
			return FILETYPE::BOOT;
		}
		else
		{
			file.SeekToBegin();
			file.Read(&headerinfo, sizeof(ST_HEADER_INFO));
			if (strcmp(headerinfo.name, "OS") == 0)
				return FILETYPE::LVOS;

			if (strcmp(headerinfo.name, "CONFIG") == 0)
				return FILETYPE::CONFIG;
		}
	}
	else
	{
		file.Read(&headerinfo, sizeof(ST_HEADER_INFO));
		if (strcmp(headerinfo.name, "PUK") == 0)
			return FILETYPE::PUK;
	}
	
	return -1;
}

//更新可用串口
void CDownloader::updateports()
{
	CArray<SSerInfo, SSerInfo&> asi;

	EnumSerialPorts(asi, FALSE);
	CString strComPort,strsel;
	int portCount = 0;
	m_validports.GetWindowTextA(strsel);
	m_validports.ResetContent();

	std::string strtmp = "";
	utility::loadconfiginfo("NORMAL COM", strtmp);

	for (int ii = 0; ii<asi.GetSize(); ii++)
	{
		// 匹配到串口 XP和2K的大小写可能不同，要统一一下。
		CString strHardwareID = asi[ii].strHardwareID;
		strHardwareID.MakeUpper();

		SSerInfo serInfo = asi[ii];
		CString strComPortTemp = serInfo.strFriendlyName;
		strComPortTemp.MakeUpper();

		//PROLIFIC USB-TO-SERIAL
		if (strComPortTemp.Find(strtmp.c_str()) != -1)
		{
			int startIndex = strComPortTemp.Find("(COM") + 4;
			int endIndex = strComPortTemp.ReverseFind(')');

			strComPort.Format("COM%s", strComPortTemp.Mid(startIndex, endIndex - startIndex));
			portCount++;
			m_validports.AddString(strComPort);
		}
	}
	if(m_validports.SelectString(-1, strsel) < 0)
		m_validports.SetCurSel(0);
}

//获取当前选择的串口
CString CDownloader::getworkportname()
{
	CString tmp;
	m_validports.GetWindowTextA(tmp);	
	return tmp;
}

//查询OS信息
void CDownloader::OnBnClickedBtnquery()
{
	// TODO: 在此添加控件通知处理程序代码
	m_tasknumadd = 0;						//记录添加任务的个数
	m_tasknumhandle = 0;					//记录完成的任务的个数
	m_resultmessage = "";					//记录任务完成结果的字符串
	m_deviceinfo.SetWindowTextA("");

	TASKDATA data = { 0 };
	data.ack = BOOTHANDSHAKE;
	data.tasktype = TASKTYPE::TASK_POPLOAD;
	data.cmdh = CMDH::CMD_LVOS;
	data.cmdl = OSCMDL::OS_ALL;
	strcpy(data.szcom, getworkportname());
	data.hwnd = this->m_hWnd;

	g_service.m_allstop = false;
	g_service.addtask(data);
	m_tasknumadd++;

	m_devinfo.EnableWindow(FALSE);
	m_btndownload.EnableWindow(FALSE);
	m_tips.SetWindowTextA("Please wait...");
}

//加载历史记录
void CDownloader::loadhistory()
{
	string tmp = "";
	utility::loadconfiginfo("Firmpath", tmp);

	string::size_type pos;
	string pattern = ";";
	//tmp += pattern;			//扩展字符串以方便操作
	int size = tmp.size();

	for (int i = 0; i<size; i++)
	{
		pos = tmp.find(pattern, i);
		if (pos < size)
		{
			std::string s = tmp.substr(i, pos - i);
			if(s.size() > 0)
				m_filelist.AddString(s.c_str());
			i = pos + pattern.size() - 1;
		}
	}

	UpdateData(FALSE);
}

//保存历史记录
void CDownloader::savehistory()
{
	UpdateData(TRUE);

	string allfile = "";
	int size = m_filelist.GetCount();
	for (int i = 0; i < size; i++)
	{
		CString strtmp = "";
		m_filelist.GetText(i, strtmp);
		allfile += strtmp;
		allfile += ";";
	}

	utility::setconfiginfo("Firmpath", allfile);
}

//码表烧录
void CDownloader::OnBnClickedBtnseialchg()
{
	// TODO: 在此添加控件通知处理程序代码
	m_tasknumadd = 0;						//记录添加任务的个数
	m_tasknumhandle = 0;					//记录完成的任务的个数
	m_resultmessage = "";					//记录任务完成结果的字符串

	TASKDATA data = { 0 };
	data.ack = OSHANDSHAKE;
	data.tasktype = TASKTYPE::TASK_POPLOAD;
	data.cmdh = CMDH::OS_FLASHOPERA;
	//data.cmdl = OSCMDL::OS_ALL;
	strcpy(data.szcom, getworkportname());
	data.hwnd = this->m_hWnd;

	std::string strtmp = "";
	utility::loadconfiginfo("UNI2OEM", strtmp);
	strcpy(data.szFilePath, strtmp.c_str());
	strtmp = "";
	utility::loadconfiginfo("OEM2UNI", strtmp);
	strcpy(data.szpukpath, strtmp.c_str());

	g_service.m_allstop = false;
	g_service.addtask(data);
	m_tasknumadd++;

	m_devinfo.EnableWindow(FALSE);
	m_btndownload.EnableWindow(FALSE);
	m_transfer.EnableWindow(FALSE);
	m_tips.SetWindowTextA("Please wait...");
}

//是否需要重启SP
void CDownloader::OnBnClickedCheckrestartsp()
{
	// TODO: 在此添加控件通知处理程序代码
	g_service.m_resetcmd = m_restartsp.GetCheck();
}

//是否需要转换为透传串口
void CDownloader::OnBnClickedCheckrestartsp2()
{
	// TODO: 在此添加控件通知处理程序代码
	g_service.m_portconvert = m_checkcvtport.GetCheck();
}


void CDownloader::OnIdok()
{
	// TODO: 在此添加命令处理程序代码
	return;
}


void CDownloader::OnBnClickedBtnfont()
{
	// TODO: 在此添加控件通知处理程序代码
	m_tasknumadd = 0;						//记录添加任务的个数
	m_tasknumhandle = 0;					//记录完成的任务的个数
	m_resultmessage = "";					//记录任务完成结果的字符串

	TASKDATA data = { 0 };
	data.ack = OSHANDSHAKE;
	data.tasktype = TASKTYPE::TASK_POPLOAD;
	data.cmdh = CMDH::OS_FONT;
	//data.cmdl = OSCMDL::OS_ALL;
	strcpy(data.szcom, getworkportname());
	data.hwnd = this->m_hWnd;

	std::string strtmp = "";
	utility::loadconfiginfo("FONT", strtmp);
	strcpy(data.szFilePath, strtmp.c_str());	

	g_service.m_allstop = false;
	g_service.addtask(data);
	m_tasknumadd++;

	m_devinfo.EnableWindow(FALSE);
	m_btndownload.EnableWindow(FALSE);
	m_transfer.EnableWindow(FALSE);
	m_downfont.EnableWindow(FALSE);
	m_tips.SetWindowTextA("Please wait...");
}


void CDownloader::OnBnClickedBtnstop()
{
	// TODO: 在此添加控件通知处理程序代码
	g_service.m_allstop = true;
}

//烧录背景图片
void CDownloader::OnBnClickedBtnbkimg()
{
	// TODO: 在此添加控件通知处理程序代码
	m_tasknumadd = 0;						//记录添加任务的个数
	m_tasknumhandle = 0;					//记录完成的任务的个数
	m_resultmessage = "";					//记录任务完成结果的字符串

	TASKDATA data = { 0 };
	data.ack = OSHANDSHAKE;
	data.tasktype = TASKTYPE::TASK_POPLOAD;
	data.cmdh = CMDH::OS_BKIMG;
	strcpy(data.szcom, getworkportname());
	data.hwnd = this->m_hWnd;

	std::string strtmp = "";
	utility::loadconfiginfo("BKIMG", strtmp);
	strcpy(data.szFilePath, strtmp.c_str());

	g_service.m_allstop = false;
	g_service.addtask(data);
	m_tasknumadd++;

	m_downbkimg.EnableWindow(FALSE);
	m_devinfo.EnableWindow(FALSE);
	m_btndownload.EnableWindow(FALSE);
	m_transfer.EnableWindow(FALSE);
	m_tips.SetWindowTextA("Please wait...");
}

CString CDownloader::GetRightSN(CString str)
{
	CString strret = "";
	for (int i = 0; i < str.GetLength(); i++)
	{
		if (isalpha(str.GetAt(i)) || isalnum(str.GetAt(i)))
		{
			strret += str.GetAt(i);
		}
	}

	return strret;
}
