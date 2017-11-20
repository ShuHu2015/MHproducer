#pragma once
#include "afxcmn.h"
#include "TextProgressCtrl.h"
#include "afxwin.h"

// CDownloader 对话框

class CDownloader : public CDialogEx
{
	DECLARE_DYNAMIC(CDownloader)

public:
	CDownloader(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDownloader();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGDOWNLOAD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnMessageHandler(WPARAM wparam, LPARAM lparam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnadd();
	afx_msg void OnBnClickedBtndec();
	afx_msg void OnBnClickedBtndown();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBnClickedBtnquery();
	afx_msg void OnBnClickedBtnseialchg();
	afx_msg void OnBnClickedCheckrestartsp();
	afx_msg void OnBnClickedCheckrestartsp2();
	afx_msg void OnIdok();
	afx_msg void OnBnClickedBtnfont();
	afx_msg void OnBnClickedBtnstop();
	afx_msg void OnBnClickedBtnbkimg();

public:
	CTextProgressCtrl m_progress;
	CBrush			  m_bkgbrush;
	CEdit			  m_tips;
	CListBox		  m_filelist;
	int				  m_tasknumadd;
	int				  m_tasknumhandle;
	CString			  m_resultmessage;
	CButton			  m_btndownload;

public:
	int		GetFileType(CString filepath);
	void    updateports();
	void	loadhistory();
	void	savehistory();
	CString getworkportname();
	CString GetRightSN(CString str);


	CComboBox m_validports;
	CButton m_devinfo;
	CString m_sn1str;
	CString m_sn2str;
	CButton m_sn1check;
	CButton m_sn2check;
	CButton m_checkfile;	
	CButton m_transfer;
	CButton m_restartsp;
	CButton m_checkcvtport;
	CEdit m_deviceinfo;	
	CButton m_downfont;
	CButton m_downbkimg;
	CButton m_stop;


	
};
