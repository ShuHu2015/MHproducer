
// MHproducerDlg.h : 头文件
//

#pragma once
#include   "resource.h"
#include "MyTabCtrl.h"
#include "Downloader.h"
#include "Authdlg.h"
#include "Signdlg.h"
#include "DlgProduce.h"

// CMHproducerDlg 对话框
class CMHproducerDlg : public CDialogEx
{
// 构造
public:
	CMHproducerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MHPRODUCER_DIALOG};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
// 实现
protected:
	HICON m_hIcon;

	CMyTabCtrl  m_tab;

public:
	CDownloader m_DlgTab1;
	CAuthdlg    m_DlgTab2;
	CSigndlg    m_DlgTab3;
	CDlgProduce m_DlgTab4;

private:
	BOOL DoRegisterDeviceInterfaceToHwnd();
	void maindefault();
	

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD dwData);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	DECLARE_MESSAGE_MAP()

public:
	CString getworkportname();
	afx_msg void OnClose();
};
