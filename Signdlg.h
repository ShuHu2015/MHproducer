#pragma once
#include "afxwin.h"
#include "DragEdit.h"


// CSigndlg 对话框

class CSigndlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSigndlg)

public:
	CSigndlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSigndlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGSIGN };
#endif

	enum { FILE_BOOT, FILE_OS, } FILETYPE;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg LRESULT OnMessageHandler(WPARAM wparam, LPARAM lparam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	CString m_filepath;
	CString m_filetype;
	CMFCStatusBar m_StatusBar;

public:

	void updateuserinfo();
	void settips(CString text);
	void addtips(CString text);

	CComboBox m_keys;
	virtual BOOL OnInitDialog();
	CEdit m_tips;
	CButton m_btnsign;
	CBrush m_bkgbrush;
	CDragEdit m_signpath;
	
	
	afx_msg void OnIdok();
};
