// AuthLogin.cpp : 实现文件
//

#include "stdafx.h"
#include "MHproducer.h"
#include "AuthLogin.h"
#include "afxdialogex.h"
#include "pophttpcomm.h"
#include "utility.h"
#include <sstream>

using namespace utilityDES;

extern pophttpcomm g_authpop;
extern pophttpcomm g_signpop;
// CAuthLogin 对话框
const BYTE g_key[] = { 42, 16, 93, 156, 78, 4, 218, 32 };
const BYTE g_IV[] = { 55, 103, 24, 179, 36, 99, 167, 3 };

IMPLEMENT_DYNAMIC(CAuthLogin, CDialogEx)

CAuthLogin::CAuthLogin(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGAUTHLOG, pParent)
	, m_username(_T(""))
	, m_password(_T(""))
	, m_isremember(FALSE)
{
	m_islogin = FALSE;
}

CAuthLogin::CAuthLogin(int logintype, CWnd * pParent)
	: CDialogEx(IDD_DLGAUTHLOG, pParent)
	, m_username(_T(""))
	, m_password(_T(""))
	, m_isremember(FALSE)
{
	m_islogin = FALSE;
	m_logintype = logintype;
}

CAuthLogin::~CAuthLogin()
{
	
}

int CAuthLogin::logandqueryinfo(int type)
{
	int iret = 0;

	switch (type)
	{
	case AUTH_LOGIN:
	{
		string token = "";
		std::string strtmp = "";
		utility::loadconfiginfo("AUTH IP", strtmp);
		size_t pos = strtmp.find(':');
		g_authpop.initialhttp(strtmp.substr(0, pos), atoi(strtmp.substr(pos + 1).c_str()));
		if (g_authpop.loginandgettoken(string(m_username), string(m_password), &token) == 0)
		{
			strtmp = "";
			stringstream ss;
			ss << m_isremember;
			ss >> strtmp;
			utility::setconfiginfo("AUTH REMEMBER", strtmp);
			utility::setconfiginfo("AUTH USERNAME", string(m_username));
			if (m_isremember)
			{
				unsigned char enpassword[1024] = { 0 };
				if (!TDES::RunDES(TDES::ENCRYPT, TDES::CBC, TDES::PAD_PKCS_7, g_IV, (unsigned char*)m_password.GetBuffer(), enpassword, m_password.GetLength(), g_key, sizeof(g_key)))
				{
					AfxMessageBox(_T("ENCRYPT ERROE"));
					return -1;
				}
				char base64[1024] = { 0 };
				utility::base64_encode(enpassword, (char*)base64, strlen((char*)enpassword));
				utility::setconfiginfo("AUTH PWD", base64);
			}
			else
			{
				utility::setconfiginfo("AUTH PWD", "");
			}

			if (g_authpop.queryuserinfo() != 0)
				return -1;
			m_islogin = TRUE;		
		}
		else
		{
			iret = -1;
			AfxMessageBox(_T("LOGIN FAILS"));
		}
		break;
	}
	case SIGN_LOGIN:
	{
		string token = "";
		std::string strtmp = "";
		utility::loadconfiginfo("SIGN IP", strtmp);
		size_t pos = strtmp.find(':');
		g_signpop.initialhttp(strtmp.substr(0, pos), atoi(strtmp.substr(pos + 1).c_str()));
		if (g_signpop.loginandgettoken(string(m_username), string(m_password), &token) == 0)
		{
			strtmp = "";
			stringstream ss;
			ss << m_isremember;
			ss >> strtmp;
			utility::setconfiginfo("SIGN REMEMBER", strtmp);
			utility::setconfiginfo("SIGN USERNAME", string(m_username));
			if (m_isremember)
			{
				unsigned char enpassword[1024] = { 0 };
				if (!TDES::RunDES(TDES::ENCRYPT, TDES::CBC, TDES::PAD_PKCS_7, g_IV, (unsigned char*)m_password.GetBuffer(), enpassword, m_password.GetLength(), g_key, sizeof(g_key)))
				{
					AfxMessageBox(_T("ENCRYPT ERROE"));
					return -1;
				}
				char base64[1024] = { 0 };
				utility::base64_encode(enpassword, (char*)base64, strlen((char*)enpassword));
				utility::setconfiginfo("SIGN PWD", base64);
			}
			else
			{
				utility::setconfiginfo("SIGN PWD", "");
			}

			if (g_signpop.queryuserinfo() != 0)
				return -1;
			m_islogin = TRUE;
		}
		else
		{
			iret = -1;
			AfxMessageBox(_T("LOGIN FAILS"));
		}
		break;
	}
	default:
	{
		iret = -1;
		break;
	}
	}
	return iret;
}

void CAuthLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_username);
	DDX_Text(pDX, IDC_EDIT2, m_password);
	DDX_Check(pDX, IDC_CHECK1, m_isremember);
}


BEGIN_MESSAGE_MAP(CAuthLogin, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAuthLogin::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK1, &CAuthLogin::OnBnClickedCheck1)
	ON_BN_CLICKED(IDCANCEL, &CAuthLogin::OnBnClickedCancel)
END_MESSAGE_MAP()


// CAuthLogin 消息处理程序

//登录
void CAuthLogin::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	AfxGetApp()->BeginWaitCursor();
	if (logandqueryinfo(m_logintype) == 0)
	{
		CDialogEx::OnOK();
	}
	AfxGetApp()->EndWaitCursor();
}


BOOL CAuthLogin::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	switch (m_logintype)
	{
	case AUTH_LOGIN:
	{
		this->SetWindowText(_T("AUTH LOGIN"));
		std::string strtmp = "";
		utility::loadconfiginfo("AUTH REMEMBER", strtmp);
		if (strtmp != "")
		{
			m_isremember = atoi(strtmp.c_str());
		}

		utility::loadconfiginfo("AUTH USERNAME", strtmp);
		m_username = strtmp.c_str();

		if (m_isremember)	//base64解密，再用des解密
		{		
			utility::loadconfiginfo("AUTH PWD", strtmp);
			unsigned char bindata[1024] = { 0 };
			int binlen = utility::base64_decode(strtmp.c_str(), bindata);
			unsigned char enpassword[1024] = { 0 };
			if (!TDES::RunDES(TDES::DECRYPT, TDES::CBC, TDES::PAD_PKCS_7, g_IV, bindata, enpassword, binlen, g_key, sizeof(g_key)))
			{
				AfxMessageBox(_T("DECRYPT ERROR, Please input password again"));
				m_password = "";
			}
			else
				m_password = (char*)enpassword;
		}		
		break;
	}
	case SIGN_LOGIN:
	{
		this->SetWindowText(_T("SIGN LOGIN"));
		std::string strtmp = "";
		utility::loadconfiginfo("SIGN REMEMBER", strtmp);
		if (strtmp != "")
		{
			m_isremember = atoi(strtmp.c_str());
		}
		utility::loadconfiginfo("SIGN USERNAME", strtmp);
		m_username = strtmp.c_str();

		if (m_isremember)
		{
			utility::loadconfiginfo("SIGN PWD", strtmp);
			unsigned char bindata[1024] = { 0 };
			int binlen = utility::base64_decode(strtmp.c_str(), bindata);
			unsigned char enpassword[1024] = { 0 };
			if (!TDES::RunDES(TDES::DECRYPT, TDES::CBC, TDES::PAD_PKCS_7, g_IV, bindata, enpassword, binlen, g_key, sizeof(g_key)))
			{
				AfxMessageBox(_T("DECRYPT ERROR, Please input password again"));
				m_password = "";
			}
			else
				m_password = (char*)enpassword;
		}
		break;
	}
	default:
		AfxMessageBox(_T("LOGIN TYPE ERROR"));
		break;
	}
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CAuthLogin::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
}


void CAuthLogin::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
