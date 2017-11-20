
// MHproducerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MHproducer.h"
#include "MHproducerDlg.h"
#include "afxdialogex.h"
#include "utility.h"
#include "pophttpcomm.h"
#include <Dbt.h>
#include "mhservice.h"
#include "EnumSerial.h"
#include "hslog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define		TOOLNAME			"MHproduct V1010"


pophttpcomm g_authpop;
pophttpcomm g_signpop;
mhservice	g_service;
CMHproducerDlg* g_mainwin;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMHproducerDlg 对话框

CMHproducerDlg::CMHproducerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMHproducerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMHproducerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMHproducerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_WM_CLOSE()
	ON_WM_COPYDATA()
END_MESSAGE_MAP()


// CMHproducerDlg 消息处理程序

void CMHproducerDlg::maindefault()
{
	CRect rc;
	GetClientRect(rc);
	m_tab.EnableActiveTabCloseButton(FALSE);
	m_tab.Create(CMFCTabCtrl::STYLE_3D_ONENOTE, rc, this, 1, CMFCTabCtrl::LOCATION_TOP);
	m_tab.EnableTabSwap(FALSE);
	m_DlgTab1.Create(IDD_DLGDOWNLOAD, &m_tab);
	m_DlgTab2.Create(IDD_DLGAUTH, &m_tab);
	m_DlgTab3.Create(IDD_DLGSIGN, &m_tab);
	m_DlgTab4.Create(IDD_DLGPRODUCE, &m_tab);
	//初始化tab控件
	HINSTANCE hMod = ::GetModuleHandle(NULL);

	m_tab.AddTab(&m_DlgTab1, _T("  Downloader  "), -1, true);
	m_tab.AddTab(&m_DlgTab2, _T("  Authorize   "), -1, true);
	m_tab.AddTab(&m_DlgTab3, _T("  Signature   "), -1, true);

	std::string strtmp;
	utility::loadconfiginfo("PRODUCT", strtmp);
	if(atoi(strtmp.c_str()) > 0)
		m_tab.AddTab(&m_DlgTab4, _T("Factory Produce"), -1, true);

	m_tab.SetActiveTabBoldFont(TRUE);
	m_tab.SetActiveTab(0);

}

BOOL CMHproducerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	g_mainwin = this;
	this->SetWindowText(TOOLNAME);

	std::string strtmp = "";
	utility::loadconfiginfo("DEBUG", strtmp);
	hslog::isprintlog(atoi(strtmp.c_str()));

	maindefault();
	BOOL iii = DoRegisterDeviceInterfaceToHwnd();
	g_service.initialservice(1);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMHproducerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMHproducerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMHproducerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//生产的任务产生器
BOOL CMHproducerDlg::OnDeviceChange(UINT nEventType, DWORD dwData)
{

	DEV_BROADCAST_HDR* dhr = (DEV_BROADCAST_HDR *)dwData;

	switch (nEventType)
	{
	case DBT_DEVICEARRIVAL:
	{
		if (m_tab.GetActiveTab() != 3 || m_DlgTab4.m_isstart != TRUE)	//如果不是生产模式更新一下可用串口
		{
			m_DlgTab1.updateports();
		}	
		return TRUE;
	}
	case DBT_DEVICEREMOVECOMPLETE:
	{
		if (m_tab.GetActiveTab() != 3 || m_DlgTab4.m_isstart != TRUE)	//如果不是生产模式更新一下可用串口
		{
			m_DlgTab1.updateports();
		}
		return TRUE;
	}
	default:
		return TRUE;
	}
}

BOOL CMHproducerDlg::DoRegisterDeviceInterfaceToHwnd()
{
	DEV_BROADCAST_DEVICEINTERFACE dbd = {0};
	dbd.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	dbd.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);

	return (RegisterDeviceNotification(this->m_hWnd, &dbd, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES) == NULL) ? 0 : 1;
}

CString CMHproducerDlg::getworkportname()
{
	return m_DlgTab1.getworkportname();
}

void CMHproducerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!g_service.m_allstop)
	{
		AfxMessageBox("Please Stop First ！");
		return;
	}
	CDialogEx::OnClose();
}

BOOL CMHproducerDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	char* info = NULL;
	info = (char*)malloc(pCopyDataStruct->cbData + 1);
	if (info == NULL)
		return FALSE;
	memset(info, 0, pCopyDataStruct->cbData + 1);	
	memcpy(info, pCopyDataStruct->lpData, pCopyDataStruct->cbData);

	m_DlgTab4.m_strsn1.SetWindowTextA(info);
	
	free(info);
	return TRUE;
	//return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
}
