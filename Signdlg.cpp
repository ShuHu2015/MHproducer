// Signdlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MHproducer.h"
#include "Signdlg.h"
#include "afxdialogex.h"
#include "pophttpcomm.h"
#include "mhservice.h"
#include "MHproducerDlg.h"

#define		BLOCKSIZE		4096		//�����Ĵ�С



// CSigndlg �Ի���
extern pophttpcomm  g_signpop;
extern mhservice	g_service;
extern CMHproducerDlg* g_mainwin;

IMPLEMENT_DYNAMIC(CSigndlg, CDialogEx)

CSigndlg::CSigndlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGSIGN, pParent)
	, m_filepath(_T(""))
{

}

CSigndlg::~CSigndlg()
{
}

void CSigndlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_filepath);
	DDX_Control(pDX, IDC_COMBO1, m_keys);
	DDX_Control(pDX, IDC_EDIT2, m_tips);
	DDX_Control(pDX, IDC_BUTTON3, m_btnsign);
	DDX_Control(pDX, IDC_EDIT1, m_signpath);
}


BEGIN_MESSAGE_MAP(CSigndlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CSigndlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CSigndlg::OnBnClickedButton3)
	ON_MESSAGE(WM_MESSAGEINFO, OnMessageHandler)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDOK, &CSigndlg::OnIdok)
END_MESSAGE_MAP()


// CSigndlg ��Ϣ�������

//������Ϣ����
LRESULT CSigndlg::OnMessageHandler(WPARAM wparam, LPARAM lparam)
{
	
	if (wparam == P_RESULT)
	{
		CString tmp;
		if (lparam == 1)
		{
			settips("SIGN SUCCESS��");
		}
		else if (lparam == 0)
		{
			settips("SIGN FAIL��");
		}
		m_btnsign.EnableWindow(TRUE);
	}
	if (wparam == P_INFO)
	{
		settips((char* )lparam);
	}
	return 0;
}

void CSigndlg::updateuserinfo()
{
	CString strtmp = "";
	GetDlgItem(IDC_EDITGROUP)->SetWindowText(strtmp + g_signpop.info.group.c_str()); //
	strtmp = "";
	GetDlgItem(IDC_EDITUSER)->SetWindowText(strtmp + g_signpop.info.username.c_str());
	strtmp = "";

	m_keys.ResetContent();
	for (size_t i = 0; i < g_signpop.info.strproduct.size(); i++)
	{
		m_keys.AddString(g_signpop.info.strproduct[i].c_str());	
	}	
	m_keys.SetCurSel(0);
}

//ѡ���ļ����ж��ļ�����
void CSigndlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CFile file;
	CFileDialog filedlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||");

	if (filedlg.DoModal() == IDOK)
	{
		m_filepath = filedlg.GetPathName();
		UpdateData(FALSE);
	}
}

//ǩ��
void CSigndlg::OnBnClickedButton3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CFile file;
	CFile filesigned;
	CString szfilename;
	CString keyname;
	TASKDATA data = { 0 };
	UpdateData(TRUE);
	if (!file.Open(m_filepath, CFile::modeRead))
	{
		AfxMessageBox(_T("FILE OPEN ERROR"));
		return;
	}

	ST_HEADER_INFO headerinfo = { 0 };
	file.Read(&headerinfo, sizeof(ST_HEADER_INFO));
	file.Close();

	CString filetype = headerinfo.name;
	if (filetype == _T("BOOT"))
	{
		settips("Start Sign BOOT...");

		data.hwnd = this->m_hWnd;
		data.tasktype = TASKTYPE::TASK_SIGN;
		data.cmdl = FILETYPE::BOOT;
		m_keys.GetWindowText(keyname);
		strcpy(data.szpukpath, keyname);
		strcpy(data.szFilePath, m_filepath.GetBuffer());
		m_filepath.ReleaseBuffer();

		g_service.addtask(data);
		m_btnsign.EnableWindow(FALSE);
	}
	else if (filetype == _T("OS"))
	{
		settips("Start Sign OS...");

		data.hwnd = this->m_hWnd;
		data.tasktype = TASKTYPE::TASK_SIGN;
		data.cmdl = FILETYPE::LVOS;
		m_keys.GetWindowText(keyname);
		strcpy(data.szpukpath, keyname);			//��Կ����
		strcpy(data.szFilePath, m_filepath.GetBuffer());
		m_filepath.ReleaseBuffer();

		g_service.addtask(data);
		m_btnsign.EnableWindow(FALSE);
	}
	else if (filetype == _T("PUK"))
	{
		settips("Start Sign PUK...");

		data.hwnd = this->m_hWnd;
		data.tasktype = TASKTYPE::TASK_SIGN;
		data.cmdl = FILETYPE::PUK;
		m_keys.GetWindowText(keyname);
		strcpy(data.szpukpath, keyname);			//��Կ����
		strcpy(data.szFilePath, m_filepath.GetBuffer());
		m_filepath.ReleaseBuffer();

		g_service.addtask(data);
		m_btnsign.EnableWindow(FALSE);
	}
	else
	{
		settips("Start Sign unknow file...");

		data.hwnd = this->m_hWnd;
		data.tasktype = TASKTYPE::TASK_SIGN;
		data.cmdl = FILETYPE::LVOS;
		m_keys.GetWindowText(keyname);
		strcpy(data.szpukpath, keyname);			//��Կ����
		strcpy(data.szFilePath, m_filepath.GetBuffer());
		m_filepath.ReleaseBuffer();

		g_service.addtask(data);
		m_btnsign.EnableWindow(FALSE);
	}

}


BOOL CSigndlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	m_bkgbrush.CreateSolidBrush(RGB(192, 192, 192));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

void CSigndlg::settips(CString text)
{
	m_tips.SetWindowTextA(text);
}

void CSigndlg::addtips(CString text)
{
	CString tmp;
	m_tips.GetWindowTextA(tmp);

	tmp += text;
	m_tips.SetWindowTextA(tmp);
}

BOOL CSigndlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CRect clientRect;
	GetClientRect(&clientRect);
	pDC->FillRect(&clientRect, &m_bkgbrush);
	return TRUE;
	//return CDialogEx::OnEraseBkgnd(pDC);
}


HBRUSH CSigndlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
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
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}


void CSigndlg::OnIdok()
{
	// TODO: �ڴ���������������
	return;
}
