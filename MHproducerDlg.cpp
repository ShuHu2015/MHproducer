
// MHproducerDlg.cpp : ʵ���ļ�
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CMHproducerDlg �Ի���

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


// CMHproducerDlg ��Ϣ�������

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
	//��ʼ��tab�ؼ�
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

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	g_mainwin = this;
	this->SetWindowText(TOOLNAME);

	std::string strtmp = "";
	utility::loadconfiginfo("DEBUG", strtmp);
	hslog::isprintlog(atoi(strtmp.c_str()));

	maindefault();
	BOOL iii = DoRegisterDeviceInterfaceToHwnd();
	g_service.initialservice(1);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMHproducerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMHproducerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//���������������
BOOL CMHproducerDlg::OnDeviceChange(UINT nEventType, DWORD dwData)
{

	DEV_BROADCAST_HDR* dhr = (DEV_BROADCAST_HDR *)dwData;

	switch (nEventType)
	{
	case DBT_DEVICEARRIVAL:
	{
		if (m_tab.GetActiveTab() != 3 || m_DlgTab4.m_isstart != TRUE)	//�����������ģʽ����һ�¿��ô���
		{
			m_DlgTab1.updateports();
		}	
		return TRUE;
	}
	case DBT_DEVICEREMOVECOMPLETE:
	{
		if (m_tab.GetActiveTab() != 3 || m_DlgTab4.m_isstart != TRUE)	//�����������ģʽ����һ�¿��ô���
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (!g_service.m_allstop)
	{
		AfxMessageBox("Please Stop First ��");
		return;
	}
	CDialogEx::OnClose();
}

BOOL CMHproducerDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
