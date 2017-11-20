// Authdlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MHproducer.h"
#include "Authdlg.h"
#include "afxdialogex.h"
#include "AuthLogin.h"
#include "pophttpcomm.h"
#include "mhservice.h"
#include "MHproducerDlg.h"

// CAuthdlg 对话框
extern pophttpcomm     g_authpop;
extern mhservice	   g_service;
extern CMHproducerDlg* g_mainwin;

IMPLEMENT_DYNAMIC(CAuthdlg, CDialogEx)

CAuthdlg::CAuthdlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGAUTH, pParent)
{

}

CAuthdlg::~CAuthdlg()
{
}

void CAuthdlg::setipaddress(char * ipaddress)
{
}

void CAuthdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_products);
	DDX_Control(pDX, IDC_EDIT2, m_tips);
}


BEGIN_MESSAGE_MAP(CAuthdlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, &CAuthdlg::OnNMClickSyslink1)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CAuthdlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BTNAUTH1, &CAuthdlg::OnBnClickedBtnauth1)
	ON_BN_CLICKED(IDC_BTNAUTH2, &CAuthdlg::OnBnClickedBtnauth2)
	ON_BN_CLICKED(IDC_BTNAUTH3, &CAuthdlg::OnBnClickedBtnauth3)
	ON_BN_CLICKED(IDC_BTNAUTH4, &CAuthdlg::OnBnClickedBtnauth4)
	ON_BN_CLICKED(IDC_BTNAUTH5, &CAuthdlg::OnBnClickedBtnauth5)
	ON_BN_CLICKED(IDC_BTNAUTH6, &CAuthdlg::OnBnClickedBtnauth6)
	ON_BN_CLICKED(IDC_BTNAUTH7, &CAuthdlg::OnBnClickedBtnauth7)
	ON_BN_CLICKED(IDC_BTNAUTH8, &CAuthdlg::OnBnClickedBtnauth8)
	ON_BN_CLICKED(IDC_BTNAUTH9, &CAuthdlg::OnBnClickedBtnauth9)
	ON_BN_CLICKED(IDC_BTNAUTH10, &CAuthdlg::OnBnClickedBtnauth10)
	ON_BN_CLICKED(IDC_BTNAUTH11, &CAuthdlg::OnBnClickedBtnauth11)
	ON_MESSAGE(WM_MESSAGEINFO, OnMessageHandler)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDOK, &CAuthdlg::OnIdok)
END_MESSAGE_MAP()


// CAuthdlg 消息处理程序
LRESULT CAuthdlg::OnMessageHandler(WPARAM wparam, LPARAM lparam)
{
	if (wparam == P_INFO)
	{
		m_tips.SetWindowTextA((char* )lparam);
	}

	if (wparam == P_END)
	{
		for (int i = 0; i < 10; i++)
		{
			GetDlgItem(IDC_BTNAUTH1 + i)->EnableWindow(TRUE);
		}

		if(lparam != NULL && strlen((char*)lparam) > 0)
			m_tips.SetWindowTextA((char*)lparam);
		g_service.m_allstop = true;
	}
	return 0;
}


//切换账户
void CAuthdlg::OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	CAuthLogin dlg(CAuthLogin::AUTH_LOGIN);
	dlg.DoModal();
	*pResult = 0;
	updateuserinfo();
}


void CAuthdlg::updateuserinfo()
{
	CString strtmp = "";
	GetDlgItem(IDC_EDITGROUP)->SetWindowText(strtmp + g_authpop.info.group.c_str()); //
	strtmp = "";
	GetDlgItem(IDC_EDITUSER)->SetWindowText(strtmp + g_authpop.info.username.c_str());
	m_products.ResetContent();
	for (size_t i = 0; i < g_authpop.info.strproduct.size(); i++)
	{
		m_products.AddString(g_authpop.info.strproduct[i].c_str());
	}
	m_products.SelectString(0, g_authpop.info.strproduct[0].c_str());

	strtmp = "";
	map <string, int>::iterator iter;
	for (iter = g_authpop.info.licensen.begin(); iter != g_authpop.info.licensen.end(); iter++)
	{
		strtmp += iter->first.c_str();
		char temp[64] = { 0 };
		sprintf_s(temp, ", %d", iter->second);
		strtmp += temp;
		strtmp += "; ";
	}
	GetDlgItem(IDC_EDITPLISCENS)->SetWindowText(strtmp);

	updatebuttons(g_authpop.info.strproduct[0].c_str());
}

void CAuthdlg::updatebuttons(CString strproduct)
{
	std::map<string, vector<int >>::iterator it;
	it = g_authpop.info.authpermission.find(strproduct.GetBuffer());
	if (it == g_authpop.info.authpermission.end())
		return;

	for (int i = 0; i < 10; i++)
	{
		GetDlgItem(IDC_BTNAUTH1 + i)->ShowWindow(SW_HIDE);
	}

	vector<int > vec(it->second);
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (vec[i] == 2)
		{
			GetDlgItem(IDC_BTNAUTH1)->ShowWindow(SW_SHOW);
		}
		if (vec[i] == 3)
		{
			GetDlgItem(IDC_BTNAUTH1 + 1)->ShowWindow(SW_SHOW);
		}
		if (vec[i] == 4)
		{
			GetDlgItem(IDC_BTNAUTH1 + 2)->ShowWindow(SW_SHOW);
		}
		if (vec[i] == 5)
		{
			GetDlgItem(IDC_BTNAUTH1 + 3)->ShowWindow(SW_SHOW);
		}
		if (vec[i] == 6)
		{
			GetDlgItem(IDC_BTNAUTH1 + 4)->ShowWindow(SW_SHOW);
		}
		if (vec[i] == 7)
		{
			GetDlgItem(IDC_BTNAUTH1 + 5)->ShowWindow(SW_SHOW);
		}
		if (vec[i] == 8)
		{
			GetDlgItem(IDC_BTNAUTH1 + 6)->ShowWindow(SW_SHOW);
		}
		if (vec[i] == 9)
		{
			GetDlgItem(IDC_BTNAUTH1 + 7)->ShowWindow(SW_SHOW);
		}
		if (vec[i] == 10)
		{
			GetDlgItem(IDC_BTNAUTH1 + 8)->ShowWindow(SW_SHOW);
		}
		if (vec[i] == 11)
		{
			GetDlgItem(IDC_BTNAUTH1 + 9)->ShowWindow(SW_SHOW);
		}
	}
}

//
void CAuthdlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString selectstring;
	m_products.GetLBText(m_products.GetCurSel(), selectstring);
	updatebuttons(selectstring);
}

//生产授权
void CAuthdlg::OnBnClickedBtnauth1()
{
	// TODO: 在此添加控件通知处理程序代码
	addauthorizetask(0x31);
	
}

//固件调试
void CAuthdlg::OnBnClickedBtnauth2()
{
	// TODO: 在此添加控件通知处理程序代码
	addauthorizetask(0x32);

}

//固件调试清除
void CAuthdlg::OnBnClickedBtnauth3()
{
	// TODO: 在此添加控件通知处理程序代码
	addauthorizetask(0x33);
}

//软件调试
void CAuthdlg::OnBnClickedBtnauth4()
{
	// TODO: 在此添加控件通知处理程序代码
	addauthorizetask(0x34);
}

//软件调试清除
void CAuthdlg::OnBnClickedBtnauth5()
{
	// TODO: 在此添加控件通知处理程序代码
	addauthorizetask(0x35);
}

//修改SN1
void CAuthdlg::OnBnClickedBtnauth6()
{
	// TODO: 在此添加控件通知处理程序代码
	addauthorizetask(0x38);
}

//修改SN2
void CAuthdlg::OnBnClickedBtnauth7()
{
	// TODO: 在此添加控件通知处理程序代码
	addauthorizetask(0x39);
}

//清除触发
void CAuthdlg::OnBnClickedBtnauth8()
{
	// TODO: 在此添加控件通知处理程序代码
	addauthorizetask(0x37);
}

//恢复出厂设置
void CAuthdlg::OnBnClickedBtnauth9()
{
	// TODO: 在此添加控件通知处理程序代码
	addauthorizetask(0x36);
}

//修改配置文件
void CAuthdlg::OnBnClickedBtnauth10()
{
	// TODO: 在此添加控件通知处理程序代码	
	addauthorizetask(0x3a);
}

//停止
void CAuthdlg::OnBnClickedBtnauth11()
{
	// TODO: 在此添加控件通知处理程序代码
	g_service.stopalltask();
}

int  CAuthdlg::addauthorizetask(int authtype)
{
	TASKDATA data = { 0 };
	data.hwnd = this->m_hWnd;
	data.id = authtype;
	data.ack = BOOTHANDSHAKE;
	CString portname = g_mainwin->getworkportname();
	strcpy(data.szcom, portname);
	data.cmdh = CMDH::CMD_BOOT;
	data.tasktype = TASKTYPE::TASK_AUTH;

	USER_INFO info = { 0 };
	GetYYMMDDHH(info.time);
	info.type[0] = GetAuthNo(authtype);
	info.type[1] = 0x33;

	strcpy(info.userinfo, g_authpop.info.username.c_str());
	memcpy(data.szFilePath, &info, sizeof(USER_INFO));
	g_service.m_allstop = false;
	g_service.addtask(data);
	for (int i = 0; i < 10; i++)
	{
		GetDlgItem(IDC_BTNAUTH1 + i)->EnableWindow(FALSE);
	}
	m_tips.SetWindowTextA("Please wait...");
	return 0;
}


void CAuthdlg::GetYYMMDDHH(char* time)
{
	SYSTEMTIME stLocal;
	GetLocalTime(&stLocal);
	char tt[256] = { 0 };
	//unsigned char* t = (unsigned char*)&stLocal.wYear;

	sprintf_s(tt, 256, "%02d%02d%02d%02d", stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour);
	memcpy(time, tt + 2, 8);
}

BOOL CAuthdlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect clientRect;
	GetClientRect(&clientRect);
	pDC->FillRect(&clientRect, &m_bkgbrush);
	return TRUE;
	//return CDialogEx::OnEraseBkgnd(pDC);
}


BOOL CAuthdlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_bkgbrush.CreateSolidBrush(RGB(192, 192, 192));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


HBRUSH CAuthdlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
		return m_bkgbrush;
		break;
	default:
		break;
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

int  CAuthdlg::GetAuthNo(int type)
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

void CAuthdlg::OnIdok()
{
	// TODO: 在此添加命令处理程序代码
	return;
}
