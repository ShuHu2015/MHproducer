#pragma once


// CAuthLogin 对话框

class CAuthLogin : public CDialogEx
{
	DECLARE_DYNAMIC(CAuthLogin)

public:
	CAuthLogin(CWnd* pParent = NULL);   // 标准构造函数
	CAuthLogin(int logintype, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAuthLogin();

	BOOL islogin() { return m_islogin; }
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGAUTHLOG};
#endif

private:
	BOOL m_islogin;
	int  logandqueryinfo(int type);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CString m_username;
	CString m_password;
	BOOL	m_isremember;
	int		m_logintype;
	enum LOGIN_TYPE
	{
		AUTH_LOGIN,	SIGN_LOGIN
	};

	
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCancel();
};
