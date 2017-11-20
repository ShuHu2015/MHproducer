
// MHproducerDlg.h : ͷ�ļ�
//

#pragma once
#include   "resource.h"
#include "MyTabCtrl.h"
#include "Downloader.h"
#include "Authdlg.h"
#include "Signdlg.h"
#include "DlgProduce.h"

// CMHproducerDlg �Ի���
class CMHproducerDlg : public CDialogEx
{
// ����
public:
	CMHproducerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MHPRODUCER_DIALOG};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
// ʵ��
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
	

	// ���ɵ���Ϣӳ�亯��
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
