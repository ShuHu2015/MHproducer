#pragma once
#include "afxwin.h"


// CAuthdlg �Ի���

class CAuthdlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAuthdlg)

public:
	CAuthdlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAuthdlg();

	void setipaddress(char* ipaddress);

	//�����û���Ϣ
	void updateuserinfo();
	void updatebuttons(CString strproduct);
	int  addauthorizetask(int authtype);
	void GetYYMMDDHH(char* time);

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGAUTH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_products;
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedBtnauth1();
	afx_msg void OnBnClickedBtnauth2();
	afx_msg void OnBnClickedBtnauth3();
	afx_msg void OnBnClickedBtnauth4();
	afx_msg void OnBnClickedBtnauth5();
	afx_msg void OnBnClickedBtnauth6();
	afx_msg void OnBnClickedBtnauth7();
	afx_msg void OnBnClickedBtnauth8();
	afx_msg void OnBnClickedBtnauth9();
	afx_msg void OnBnClickedBtnauth10();
	afx_msg void OnBnClickedBtnauth11();
	afx_msg LRESULT OnMessageHandler(WPARAM wparam, LPARAM lparam);
	CEdit m_tips;
	CBrush m_bkgbrush;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);


public:
	int GetAuthNo(int type);
	afx_msg void OnIdok();
};
