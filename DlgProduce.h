#pragma once
#include "afxcmn.h"
#include "TextProgressCtrl.h"
#include "afxwin.h"
#include "DragEdit.h"
#include "ShThread.h"
#include "mhservice.h"


// CSignlogin 对话框
#define COLRED			0x0000FF
#define COLGREEN		0x00FF00
#define COLBLUE			CLR_DEFAULT

class SThread;
class ProduceThread;
class SNThread;

class CDlgProduce : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgProduce)

public:
	CDlgProduce(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgProduce();
	BOOL islogin() { return m_islogin; }

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGPRODUCE };
#endif

public:
	void updateuserinfo();

private:
	BOOL m_islogin;
	enum { COL_RED, COL_GREEN };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnBnClickedOk();
	afx_msg void OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnMessageHandler(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnSNMessageHandler(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnFontMessageHandler(WPARAM wparam, LPARAM lparam);
	afx_msg void OnBnClickedButton2();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnbooyonly();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnIdok();
	afx_msg void OnBnClickedBtnsnload();
	afx_msg void OnBnClickedBtnfontload();


	CTextProgressCtrl m_progress1;
	CTextProgressCtrl m_progress2;
	CTextProgressCtrl m_progress3;
	CTextProgressCtrl m_progress4;
	CTextProgressCtrl m_progress5;
	CTextProgressCtrl m_progress6;
	CEdit m_com1;
	CEdit m_com2;
	CEdit m_com3;
	CEdit m_com4;
	CEdit m_com5;

	CEdit m_bootstr;
	CEdit m_osstr;

	BOOL     m_isstart;
	CBrush	 m_bkgbrush;
	CString  m_comname;

	void updatetable(CString strcom);
	void setdefault();
	void settext(CString strcom, CString text);
	void settext(CString strcom, CString text, COLORREF bkgcolor);
	void setprogress(CString strcom, int pos);
	void setprogress(CString strcom, CString text, int pos);
	void settext(int index, CString text);
	void setbarcolor(int index, COLORREF bkgcolor);
	void setprogress(int index, int pos);
	void setprogress(int index, CString text, int pos);
	void updatecomlist();
	void sndownload();
	CString GetRightSN(CString str);

	virtual BOOL OnInitDialog();

	CButton	  m_start;
	CButton   m_fontstart;
	CButton   m_startboot;
	CEdit	  m_dispatchname;
	CButton	  m_snstart;
	ProduceThread* thread[5];
	

	CEdit	m_strsn1;
	CEdit	m_strsn2;
	CButton m_checksn1;
	CButton m_checksn2;
	CComboBox m_comlist;
	
	afx_msg void OnBnClickedBtncomupdate();
	afx_msg void OnBnClickedCheckprosn2();
	afx_msg void OnBnClickedCheckprosn1();
	afx_msg void OnBnClickedBtnsnbind();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnChangeSn1start();
};

class mhcommethod;

class ProduceThread : public ShThread
{
public:
	DWORD Run();
	HWND m_hwnd;
	int  m_index;
	CString m_devname;
	CString m_pukpath;
	CString m_bootpath;
	CString m_ospath;
	CString m_oem2uni;
	CString m_uni2oem;
	CString m_fontlib;
	CString m_cfgpath;
	int     m_comconvert;
	CDlgProduce* pdlg;
};

class FontThread : public ShThread
{
public:
	DWORD Run();
	HWND m_hwnd;
	int  m_index;
	CString m_devname;
	CString m_oem2uni;
	CString m_uni2oem;
	CString m_fontlib;
	int     m_comconvert;
	CDlgProduce* pdlg;
};


class SNThread : public ShThread
{
public:
	DWORD	Run();
	HWND	m_hwnd;
	int		m_index;
	CString m_resultcom;
	CString m_devname;
	CString m_sn1;
	CString m_sn2;
	CString m_winname;
	int		m_sn1checked;
	int		m_sn2checked;
	int     m_needupload;
	int     m_comconvert;
	int     m_restartsp;
	CDlgProduce* pdlg;
};
