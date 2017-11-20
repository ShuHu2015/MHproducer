#pragma once


// CAuthLogin �Ի���

class CAuthLogin : public CDialogEx
{
	DECLARE_DYNAMIC(CAuthLogin)

public:
	CAuthLogin(CWnd* pParent = NULL);   // ��׼���캯��
	CAuthLogin(int logintype, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAuthLogin();

	BOOL islogin() { return m_islogin; }
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGAUTHLOG};
#endif

private:
	BOOL m_islogin;
	int  logandqueryinfo(int type);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
