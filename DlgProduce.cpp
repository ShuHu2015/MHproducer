// Signlogin.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MHproducer.h"
#include "DlgProduce.h"
#include "afxdialogex.h"
#include "AuthLogin.h"
#include "pophttpcomm.h"
#include "mhservice.h"
#include "MHproducerDlg.h"
#include "EnumSerial.h"
#include "utility.h"
#include "hslog.h"

// CSignlogin �Ի���
extern pophttpcomm  g_authpop;
extern mhservice	g_service;
extern CMHproducerDlg* g_mainwin;

pophttpcomm     g_resultcom;

//static BOOL g_allstop = FALSE;

SNThread snthread;
FontThread fontthread;

static int GetBootInfo(const char* filename, ST_HEADER_INFO* info);
static int GetOsInfo(const char* filename, ST_HEADER_INFO* info);
static int GetCFGInfo(const char* filename, ST_HEADER_INFO* info);

IMPLEMENT_DYNAMIC(CDlgProduce, CDialogEx)

CDlgProduce::CDlgProduce(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGPRODUCE, pParent)
{
	m_comname = "";
	m_islogin = FALSE;
	m_isstart = FALSE;
	for (int i = 0; i < 5; i++)
	{
		thread[i] = NULL;
	}
}

CDlgProduce::~CDlgProduce()
{

}

void CDlgProduce::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress1);
	DDX_Control(pDX, IDC_PROGRESS2, m_progress2);
	DDX_Control(pDX, IDC_PROGRESS3, m_progress3);
	DDX_Control(pDX, IDC_PROGRESS4, m_progress4);
	DDX_Control(pDX, IDC_PROGRESS5, m_progress5);
	DDX_Control(pDX, IDC_PROGRESS6, m_progress6);
	DDX_Control(pDX, IDC_EDITCOM1, m_com1);
	DDX_Control(pDX, IDC_EDITCOM2, m_com2);
	DDX_Control(pDX, IDC_EDITCOM3, m_com3);
	DDX_Control(pDX, IDC_EDITCOM4, m_com4);
	DDX_Control(pDX, IDC_EDITCOM5, m_com5);
	DDX_Control(pDX, IDC_BUTTON2, m_start);
	DDX_Control(pDX, IDC_BTNBOOYONLY, m_startboot);
	DDX_Control(pDX, IDC_EDIT3, m_dispatchname);
	DDX_Control(pDX, IDC_BTNSNLOAD, m_snstart);
	DDX_Control(pDX, IDC_SN1START, m_strsn1);
	DDX_Control(pDX, IDC_SN2START, m_strsn2);
	DDX_Control(pDX, IDC_CHECKPROSN1, m_checksn1);
	DDX_Control(pDX, IDC_CHECKPROSN2, m_checksn2);
	DDX_Control(pDX, IDC_BTNFONTLOAD, m_fontstart);
	DDX_Control(pDX, IDC_COMBO1, m_comlist);
	DDX_Control(pDX, IDC_EDITBOOT, m_bootstr);
	DDX_Control(pDX, IDC_EDITOS, m_osstr);
}


BEGIN_MESSAGE_MAP(CDlgProduce, CDialogEx)
	//ON_BN_CLICKED(IDOK, &CDlgProduce::OnBnClickedOk)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, &CDlgProduce::OnNMClickSyslink1)
	ON_MESSAGE(WM_MESSAGEBOOTOSINFO, OnMessageHandler)
	ON_MESSAGE(WM_MESSAGESNINFO, OnSNMessageHandler)
	ON_MESSAGE(WM_MESSAGEINFO, OnFontMessageHandler)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgProduce::OnBnClickedButton2)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTNBOOYONLY, &CDlgProduce::OnBnClickedBtnbooyonly)
	ON_COMMAND(IDOK, &CDlgProduce::OnIdok)
	ON_BN_CLICKED(IDC_BTNSNLOAD, &CDlgProduce::OnBnClickedBtnsnload)
	ON_BN_CLICKED(IDC_BTNFONTLOAD, &CDlgProduce::OnBnClickedBtnfontload)
	ON_BN_CLICKED(IDC_BTNCOMUPDATE, &CDlgProduce::OnBnClickedBtncomupdate)
	ON_BN_CLICKED(IDC_CHECKPROSN2, &CDlgProduce::OnBnClickedCheckprosn2)
	ON_BN_CLICKED(IDC_CHECKPROSN1, &CDlgProduce::OnBnClickedCheckprosn1)
	ON_BN_CLICKED(IDC_BTNSNBIND, &CDlgProduce::OnBnClickedBtnsnbind)
	ON_EN_CHANGE(IDC_SN1START, &CDlgProduce::OnEnChangeSn1start)
END_MESSAGE_MAP()

//�л��˻�
void CDlgProduce::OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CAuthLogin dlg(CAuthLogin::AUTH_LOGIN);
	dlg.DoModal();
	*pResult = 0;
	updateuserinfo();
}

//�����û���Ϣ
void CDlgProduce::updateuserinfo()
{
	CString strtmp = "";

	map <string, int>::iterator iter;
	for (iter = g_authpop.info.licensen.begin(); iter != g_authpop.info.licensen.end(); iter++)
	{
		strtmp += iter->first.c_str();
		char temp[64] = { 0 };
		sprintf_s(temp, ", %d", iter->second);
		strtmp += temp;
		strtmp += "; ";
	}
	GetDlgItem(IDC_EDITPLISCENS)->SetWindowText(strtmp);
}

//������Ϣ����
LRESULT CDlgProduce::OnMessageHandler(WPARAM wparam, LPARAM lparam)
{
	CString tmp = "";
	int response = (int)lparam;
	switch (response)
	{
	case 1:
		settext(wparam, "��Կ����ʧ�� ");
		break;
	case 2:
		settext(wparam, "���ڴ�ʧ�� ");
		break;
	case 3:
		settext(wparam, "BOOT����ʧ�� ");
		break;
	case 4:
		settext(wparam, "����оƬ����ʧ��! ");
		break;
	case 5:
		settext(wparam, "�ļ�д�����ʧ��! ");
		break;
	case 6:
		settext(wparam, "д�ļ����ݳ���! ");
		break;
	case 7:
		settext(wparam, "�ļ�У��ʧ��! ");
		break;
	case 8:
		settext(wparam, "��Ȩʧ��! ");
		break;
	case 9:
		settext(wparam, "OS����ʧ��! ");
		break;
	case 10:
		settext(wparam, "OS����ʧ��! ");
		break;
	case 11:
		setbarcolor(wparam, COLGREEN);
		break;
	case 12:
		setbarcolor(wparam, COLRED);
		break;
	case 13:
		settext(wparam, "�������ʧ��! ");
		break;
	case 14:
		settext(wparam, "�ֿ�����ʧ��! ");
		break;
	case 15:
		settext(wparam, "CFG����ʧ��! ");
		break;
	case 16:
		settext(wparam, "CFG����ʧ��! ");
		break;
	default:
		break;
	}
	
	Sleep(2000);
	return 0;
}

LRESULT CDlgProduce::OnSNMessageHandler(WPARAM wparam, LPARAM lparam)
{
	if (wparam == P_END)
	{
		m_snstart.EnableWindow(TRUE);
		m_start.EnableWindow(TRUE);
		m_fontstart.EnableWindow(TRUE);
		m_strsn1.EnableWindow(TRUE);
		m_strsn2.EnableWindow(TRUE);
		g_service.m_allstop = true;
		m_strsn1.SetSel(0,-1);
		m_strsn1.SetFocus();
	}
	return 0;
}

LRESULT CDlgProduce::OnFontMessageHandler(WPARAM wparam, LPARAM lparam)
{
	CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS6);
	if (ptextcrtl == NULL)
		return 0;
	if (wparam == P_PROGRESS)
	{
		ptextcrtl->SetPos(lparam);
		ptextcrtl->Invalidate();
	}
	if (wparam == P_END)
	{
		m_snstart.EnableWindow(TRUE);
		m_start.EnableWindow(TRUE);
		m_fontstart.EnableWindow(TRUE);
		if (lparam == 1)
		{
			ptextcrtl->SetBarColor(COLGREEN);
			ptextcrtl->Invalidate();
		}
		else if (lparam == 0)
		{
			ptextcrtl->SetBarColor(COLRED);
			ptextcrtl->Invalidate();
		}
		g_service.m_allstop = true;
	}
	if (wparam == P_INFO)
	{
		ptextcrtl->SetWindowText((char* )lparam);
		ptextcrtl->Invalidate();
	}
	return 1;
}

//��ʼ������ť
void CDlgProduce::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString pukpath;
	CFile file;
	ST_HEADER_INFO bootinfo = {0};
	ST_HEADER_INFO osinfo = { 0 };
	ST_HEADER_INFO cfginfo = { 0 };
	CString tmp, tmp1;

	std::string strtmp = "";
	utility::loadconfiginfo("PRODUCT COM", strtmp);
	std::string puk = "";
	utility::loadconfiginfo("PUKFILENAME", puk);

	std::string oem2uni = "";
	std::string uni2oem = "";
	std::string fontlib = "";
	std::string bootpath = "";
	std::string ospath = "";
	std::string cfgpath = "";

	utility::loadconfiginfo("UNI2OEM", uni2oem);
	utility::loadconfiginfo("OEM2UNI", oem2uni);
	utility::loadconfiginfo("FONT", fontlib);
	utility::loadconfiginfo("BOOTPATH", bootpath);
	utility::loadconfiginfo("OSPATH", ospath);
	utility::loadconfiginfo("CFGPATH", cfgpath);

	if (GetBootInfo(bootpath.c_str(), &bootinfo) != 0)
	{
		AfxMessageBox(_T("BOOT�ļ�����"));
		return;
	}
	if (GetOsInfo(ospath.c_str(), &osinfo) != 0)
	{
		AfxMessageBox(_T("OS�ļ�����"));
		return;
	}

	if (cfgpath.size() > 2) 
	{
		if (GetCFGInfo(cfgpath.c_str(), &cfginfo) != 0)
		{
			AfxMessageBox(_T("CFG�ļ�����"));
			return;
		}
	}
	

	tmp.Format("%s   %s   %s ; ", bootinfo.name, bootinfo.PID, bootinfo.version);
	tmp1.Format("%s   %s   %s", osinfo.name, osinfo.PID, osinfo.version);
	m_bootstr.SetWindowText(tmp + tmp1);
	tmp.Format("%s   %s   %s", cfginfo.name, cfginfo.PID, cfginfo.version);
	m_osstr.SetWindowText(tmp);
	
	if (!m_islogin)
	{
		//��ʾ��¼�Ի���
		CAuthLogin dlg(CAuthLogin::AUTH_LOGIN);
		dlg.DoModal();
		if (!dlg.islogin())
		{
			return;
		}
		m_islogin = TRUE;
		updateuserinfo();
	}

	char pFileName[MAX_PATH];
	int nPos = GetCurrentDirectory(MAX_PATH, pFileName);

	pukpath.Format("%s\\%s", pFileName, puk.c_str());
	CFile pukfile;

	if (!m_isstart)
	{
		if (pukpath.GetLength() <= 0) {
			AfxMessageBox(_T("�����ļ�����"));
			return;
		}
		else if (!pukfile.Open(pukpath, CFile::modeRead))
		{
			pukfile.Close();
			AfxMessageBox(_T("��Կ�ļ������ڣ�"));
			return;
		}
		pukfile.Close();
		if (bootpath.size() <= 0) {
			AfxMessageBox(_T("��BOOT�ļ���"));
			return;
		}
		if (ospath.size() <= 0) {
			AfxMessageBox(_T("��OS�ļ���"));
			return;
		}
		CArray<SSerInfo, SSerInfo&> asi;
		EnumSerialPorts(asi, FALSE);
		if (asi.GetSize() <= 0)
		{
			AfxMessageBox(_T("û�п��ô��ڣ�"));
			return;
		}

		g_service.m_allstop = false;
		int index = 0;
		for (int i = 0; i < asi.GetSize(); i++)
		{
			SSerInfo serInfo = asi[i];
			CString strComPort;
			CString strComPortTemp = serInfo.strFriendlyName;
			strComPortTemp.MakeUpper();

			if (index > 4)
				break;

			if (strComPortTemp.Find(strtmp.c_str()) != -1)		//PROLIFIC USB-TO-SERIAL
			{
				int startIndex = strComPortTemp.Find("(COM") + 4;
				int endIndex = strComPortTemp.ReverseFind(')');

				strComPort.Format("COM%s", strComPortTemp.Mid(startIndex, endIndex - startIndex));

				thread[index] = new ProduceThread();
				thread[index]->m_devname = strComPort;

				CEdit* pedit = (CEdit*)GetDlgItem(IDC_EDITCOM1 + index);
				pedit->SetWindowText(strComPort);

				thread[index]->m_index = index;
				thread[index]->pdlg = this;
				thread[index]->m_pukpath = pukpath;
				thread[index]->m_bootpath = bootpath.c_str();
				thread[index]->m_ospath = ospath.c_str();
				thread[index]->m_cfgpath = cfgpath.c_str();

				thread[index]->m_oem2uni = oem2uni.c_str();
				thread[index]->m_uni2oem = uni2oem.c_str();
				thread[index]->m_fontlib = fontlib.c_str();
				thread[index]->m_hwnd = this->m_hWnd;
				thread[index]->Start();
				index++;
			}
		}
		m_isstart = TRUE;
		m_start.SetWindowTextA("ֹͣ");
	}
	else
	{
		m_bootstr.SetWindowText("");
		m_osstr.SetWindowText("");
		g_service.m_allstop = true;
		for (int i = 0; i < 5; i++)
		{
			if (thread[i] != NULL)
			{
				thread[i]->Join();
				delete(thread[i]);
				thread[i] = NULL;
			}
		}
		m_start.SetWindowTextA("��ʼ");
		setdefault();
		m_isstart = FALSE;
	}
}

void CDlgProduce::updatetable(CString strcom)
{
	for (int i = 0; i < 5; i++)
	{
		CEdit* pedit = (CEdit*)GetDlgItem(IDC_EDITCOM1 + i);
		CString tmp;
		pedit->GetWindowText(tmp);
		if (tmp == "") {
			pedit->SetWindowText(strcom);
			return;
		}
	}
}

void CDlgProduce::setdefault()
{
	for (int i = 0; i < 5; i++)
	{
		CEdit* pedit = (CEdit*)GetDlgItem(IDC_EDITCOM1 + i);
		CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS1 + i);
		pedit->SetWindowText("");
		ptextcrtl->SetWindowText(_T(""));
		ptextcrtl->SetPos(0);
		ptextcrtl->SetBarColor(CLR_DEFAULT);
		pedit->Invalidate();
	}
}

void CDlgProduce::setprogress(CString strcom, int pos)
{
	for (int i = 0; i < 5; i++)
	{
		CEdit* pedit = (CEdit*)GetDlgItem(IDC_EDITCOM1 + i);
		CString tmp;
		pedit->GetWindowText(tmp);
		if (tmp == strcom) {
			CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS1 + i);
			ptextcrtl->SetPos(pos);
			ptextcrtl->Invalidate();
			return;
		}
	}
}

void CDlgProduce::settext(CString strcom, CString text)
{
	for (int i = 0; i < 5; i++)
	{
		CEdit* pedit = (CEdit*)GetDlgItem(IDC_EDITCOM1 + i);
		CString tmp;
		pedit->GetWindowText(tmp);
		if (tmp == strcom) {
			CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS1 + i);
			ptextcrtl->SetWindowText(text);
			ptextcrtl->Invalidate();
			return;
		}
	}
}

void CDlgProduce::settext(CString strcom, CString text, COLORREF barcolor)
{
	for (int i = 0; i < 5; i++)
	{
		CEdit* pedit = (CEdit*)GetDlgItem(IDC_EDITCOM1 + i);
		CString tmp;
		pedit->GetWindowText(tmp);
		if (tmp == strcom) {
			CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS1 + i);
			ptextcrtl->SetWindowText(text);
			ptextcrtl->SetBarColor(barcolor);
			ptextcrtl->Invalidate();
			return;
		}
	}
}

void CDlgProduce::setprogress(CString strcom, CString text, int pos)
{
	for (int i = 0; i < 5; i++)
	{
		CEdit* pedit = (CEdit*)GetDlgItem(IDC_EDITCOM1 + i);
		CString tmp;
		pedit->GetWindowText(tmp);
		if (tmp == strcom) {
			CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS1 + i);
			ptextcrtl->SetPos(pos);
			ptextcrtl->SetWindowText(text);
			ptextcrtl->Invalidate();
			return;
		}
	}
}

void CDlgProduce::setprogress(int index, int pos)
{
	CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS1 + index);
	ptextcrtl->SetPos(pos);
	ptextcrtl->Invalidate();
	return;
}

void CDlgProduce::settext(int index,CString text)
{
	CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS1 + index);
	ptextcrtl->SetWindowText(text);
	ptextcrtl->Invalidate();
	return;
}

void CDlgProduce::setbarcolor(int index, COLORREF barcolor)
{
	CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS1 + index);
	ptextcrtl->SetBarColor(barcolor);
	ptextcrtl->Invalidate();
	return;
}

void CDlgProduce::setprogress(int index, CString text, int pos)
{
	CTextProgressCtrl* ptextcrtl = (CTextProgressCtrl*)GetDlgItem(IDC_PROGRESS1 + index);
	ptextcrtl->SetPos(pos);
	ptextcrtl->SetBarColor(COLBLUE);
	ptextcrtl->SetWindowText(text);
	ptextcrtl->Invalidate();
	return;
}

BOOL CDlgProduce::OnEraseBkgnd(CDC* pDC)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CRect clientRect;
	GetClientRect(&clientRect);
	pDC->FillRect(&clientRect, &m_bkgbrush);
	return TRUE;
	//return CDialogEx::OnEraseBkgnd(pDC);
}

//��ʼ������
BOOL CDlgProduce::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	std::string strtmp = "";
	utility::loadconfiginfo("UPLOADIPADDR", strtmp);
	size_t pos = strtmp.find(':');
	g_resultcom.initialhttp(strtmp.substr(0, pos), atoi(strtmp.substr(pos + 1).c_str()));

	m_dispatchname.SetWindowTextA("�����봰�ڱ���");
	strtmp = "";
	utility::loadconfiginfo("NEEDUPLOADRESULT", strtmp);
	if (atoi(strtmp.c_str()))
	{
		m_dispatchname.ShowWindow(SW_SHOW);
	}
	
	strtmp = "";
	utility::loadconfiginfo("UPLOADOLDWAY", strtmp);
	if (atoi(strtmp.c_str()))
	{
		m_startboot.ShowWindow(SW_SHOW);
	}
	updatecomlist();
	m_checksn1.SetCheck(1);
	//m_checksn2.SetCheck(1);
	m_bkgbrush.CreateSolidBrush(RGB(192, 192, 192));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

HBRUSH CDlgProduce::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

//�ɼ�������Ϣ
void CDlgProduce::OnBnClickedBtnbooyonly()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int comnum = 0;
	CString strComPort;
	CArray<SSerInfo, SSerInfo&> asi;
	EnumSerialPorts(asi, FALSE);
	if (asi.GetSize() <= 0)
	{
		AfxMessageBox(_T("û�п��ô��ڣ�"));
		return;
	}
	
	std::string snresultcom = "";
	utility::loadconfiginfo("SNRESULTCOM", snresultcom);			//���ڹؼ�������
	CString strtmp = snresultcom.c_str();

	for (int i = 0; i < (asi.GetSize() > 5 ? 5 : asi.GetSize()); i++)
	{
		SSerInfo serInfo = asi[i];
		
		CString strComPortTemp = serInfo.strFriendlyName;
		strComPortTemp.MakeUpper();

		if (strComPortTemp.Find(strtmp) != -1)		//PROLIFIC USB-TO-SERIAL
		{
			int startIndex = strComPortTemp.Find("(COM") + 4;
			int endIndex = strComPortTemp.ReverseFind(')');

			strComPort.Format("COM%s", strComPortTemp.Mid(startIndex, endIndex - startIndex));		
			comnum++;
		}
	}
	if (comnum > 1)
	{
		AfxMessageBox(_T("ֻ����һ���豸���ӣ�"));
		return;
	}

	mhcommethod commethod;
	if (commethod.open(strComPort.GetBuffer()) != 0)
	{
		AfxMessageBox(_T("���ڴ�ʧ�ܣ�"));
		return;
	}
	char szResult[1024] = { 0 };
	if (commethod.GetProduceResult(szResult) != 0)
	{
		AfxMessageBox(_T("ͨѶʧ�ܣ�"));
		return;
	}

	//string szResult = "aewojfe|awfiojaewofijawoifjw";
	CString msg;
	if (g_resultcom.uploadproduceresult(szResult, msg) != 0)
	{
		AfxMessageBox(_T("�ϴ�ʧ�ܣ�") + msg);
		return;
	}
	AfxMessageBox(_T("�ϴ��ɹ���"));
}

//��¼SN��
void CDlgProduce::OnBnClickedBtnsnload()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	sndownload();
}

void CDlgProduce::sndownload()
{
	if (!m_isstart)
	{
		CString sntmp;
		snthread.pdlg = this;
		snthread.m_hwnd = this->m_hWnd;
		if (m_checksn1.GetCheck() == 0 && m_checksn2.GetCheck() == 0)
		{
			AfxMessageBox("�빴ѡҪ��¼��SN");
			return;
		}
		std::string strtmp = "";
		utility::loadconfiginfo("NEEDUPLOADRESULT", strtmp);	//�Ƿ���Ҫ�ϴ�������
		snthread.m_needupload = atoi(strtmp.c_str());

		strtmp = "";
		utility::loadconfiginfo("SNCOMCOVERT", strtmp);			//�Ƿ�ת��Ϊ͸������
		snthread.m_comconvert = atoi(strtmp.c_str());

		strtmp = "";
		utility::loadconfiginfo("SNRESTARTSP", strtmp);			//�Ƿ�����SP
		snthread.m_restartsp = atoi(strtmp.c_str());

		std::string snresultcom = "";
		utility::loadconfiginfo("SNRESULTCOM", snresultcom);			//���ڹؼ�������


		snthread.m_sn1checked = m_checksn1.GetCheck();
		snthread.m_sn2checked = m_checksn2.GetCheck();
		sntmp = "";
		m_strsn1.GetWindowTextA(sntmp);
		//snthread.m_sn1 = GetRightSN(sntmp);
		snthread.m_sn1 = sntmp;
		sntmp = "";
		m_strsn2.GetWindowTextA(sntmp);
		//snthread.m_sn2 = GetRightSN(sntmp);
		snthread.m_sn2 = sntmp;
		m_dispatchname.GetWindowTextA(snthread.m_winname);

		if (snthread.m_winname.GetLength() < 1)
		{
			AfxMessageBox("���ڱ��ⲻ��Ϊ�գ�");
			return;
		}

		CArray<SSerInfo, SSerInfo&> asi;
		EnumSerialPorts(asi, FALSE);
		if (asi.GetSize() <= 0)
		{
			AfxMessageBox(_T("û�п��ô��ڣ�"));
			return;
		}

		int index = 0;

		for (int i = 0; i < asi.GetSize(); i++)
		{
			SSerInfo serInfo = asi[i];
			CString strComPort;
			CString strComPortTemp = serInfo.strFriendlyName;
			strComPortTemp.MakeUpper();

			if (index > 1)
			{
				AfxMessageBox(_T("ͬ���͵Ĵ���ֻ����һ����"));
				return;
			}

			if (strComPortTemp.Find(snresultcom.c_str()) != -1)		//PROLIFIC USB-TO-SERIAL
			{
				int startIndex = strComPortTemp.Find("(COM") + 4;
				int endIndex = strComPortTemp.ReverseFind(')');

				strComPort.Format("COM%s", strComPortTemp.Mid(startIndex, endIndex - startIndex));
				snthread.m_resultcom = strComPort;
				index++;
			}
		}

		if (index <= 0)
		{
			AfxMessageBox(_T("û�п��õĻ�ȡ���Խ�����ڣ�"));
			return;
		}
		g_service.m_allstop = false;
		snthread.Start();
		m_snstart.EnableWindow(FALSE);
		m_start.EnableWindow(FALSE);
		m_fontstart.EnableWindow(FALSE);
		m_strsn1.EnableWindow(FALSE);
		m_strsn2.EnableWindow(FALSE);
	}
	else
	{
		AfxMessageBox(_T("��ֹͣ��"));
		return;
	}
}

void CDlgProduce::OnIdok()
{
	// TODO: �ڴ���������������
	return;
}

DWORD ProduceThread::Run()
{
	int iRet = 0;

	BOOL bflag = FALSE;		//������¼������״̬���ɹ���ʧ��
	mhcommethod mhworkhandle;
	DThreadParam threadparam = { 0 };
	R_RSA_PUBLIC_KEY puk = { 0 };
	RESPPACK pack = { 0 };
	int pos = 0;
	FileInfo *pFileInfo = (FileInfo *)malloc(sizeof(FileInfo));

	strcpy(threadparam.filePath, m_bootpath.GetBuffer());
	m_bootpath.ReleaseBuffer();
	strcpy(threadparam.szcom, m_devname.GetBuffer());
	m_devname.ReleaseBuffer();	

	char rsapath[260] = { 0 };
	strcpy(rsapath, m_pukpath.GetBuffer());
	m_pukpath.ReleaseBuffer();

	iRet = mhworkhandle.LoadRSAPublicKeyFile(rsapath, &puk);
	if (iRet != 0) {
		pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 1);
		goto EXIST;
	}

	memcpy(threadparam.snParam.content.param.buf, puk.modulus, puk.bits / 8);
	memcpy(threadparam.snParam.content.param.buf + puk.bits / 8, puk.exponent + (puk.bits / 8) - 8, 8);

	if (mhworkhandle.open(threadparam.szcom) != 0) {
		pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 2);
		goto EXIST;
	}

	if (mhworkhandle.loadfile(pFileInfo, threadparam.filePath) != 0) {
		pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 3);
		goto EXIST;
	}

	while (!g_service.m_allstop)
	{
		threadparam.snParam.stage = 0;
		threadparam.snParam.opt = SCPU_DL_WRITE_PARAM;	//����Ϊд�����
		threadparam.snParam.content.param.info = 0;		//��Կ����
		memset(threadparam.snParam.content.param.sn.v, 0x30, sizeof(SN) - 1);	//Ĭ�϶��ǡ�0��
		threadparam.snParam.content.param.scratch = 0;	//���������룬���Զ�����汾
		threadparam.snParam.content.param.time = 50;	//�ȴ�ʱ��
		threadparam.snParam.content.param.regNum = 0;	//����Ҫ���üĴ���

		pdlg->setprogress(m_index, "", 0);

		iRet = mhworkhandle.stepconnecthandler(threadparam, PACKETTIMEOUT * 2);		//���� 10%
		if (iRet != 0)
		{
			goto CHECK_NEXT;
		}
		pdlg->setprogress(m_index, "", 5);
		bflag = FALSE;															//���¿�ʼ��һ̨����¼
		iRet = mhworkhandle.stepwritesnhandler(threadparam, PACKETTIMEOUT * 2);		//���� 10%		
		if (iRet != 0)
		{
			pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 4);
			goto CHECK_NEXT;
		}
		pos = 10;
		pdlg->setprogress(m_index, pos);

		iRet = mhworkhandle.stepwritefilehandler(threadparam, pFileInfo, PACKETTIMEOUT * 2);		
		if (iRet != 0)
		{
			pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 5);
			goto CHECK_NEXT;
		}
		pos = 30;																//���� 30%
		pdlg->setprogress(m_index, pos);

		iRet = mhworkhandle.stepwritefiledatahandler(pFileInfo, PACKETTIMEOUT * 8);				
		if (iRet != 0)
		{
			pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 6);
			goto CHECK_NEXT;
		}

		pos = 40;														//���� 40%
		pdlg->setprogress(m_index, pos);
		iRet = mhworkhandle.stepverifydatahandler(PACKETTIMEOUT * 2);				
		if (iRet != 0)
		{
			pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 7);
			goto CHECK_NEXT;
		}
		pos = 50;																	//���� 50%					
		pdlg->setprogress(m_index, pos);


		//��ʱֻ��SL51 ��Ҫ���أ�����Ҫ��ʱ�������ļ�ȥ��CFGPATH��ֵ
		if (m_cfgpath.GetLength() > 2)
		{
			iRet = mhworkhandle.pop_handshake('B', POP_SHAKETIMEOUT);
			if (iRet != 0)
			{
				pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 15);
				goto CHECK_NEXT;
			}

			iRet = mhworkhandle.pop_configdownlaod(m_cfgpath);
			if (iRet != 0 && iRet != 3)
			{
				pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 16);
				goto CHECK_NEXT;
			}
		}

		pos = 60;
		pdlg->setprogress(m_index, pos);											//���� 60%

		iRet = mhworkhandle.pop_handshake('B', POP_SHAKETIMEOUT);
		if (iRet != 0)
		{
			pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 9);
			goto CHECK_NEXT;
		}
		iRet = mhworkhandle.pop_osdownload(m_ospath, m_hwnd);
		if (iRet != 0)
		{
			pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 10);
			goto CHECK_NEXT;
		}

		pos = 80;
		pdlg->setprogress(m_index, pos);											//���� 80%

		int iii = 0;
		for (iii = 0; iii < 3; iii++)
		{
			if (mhworkhandle.pop_productauthorize() == 0)
				break;

			Sleep(1000);
		}

		if(iii == 3)
		{
			pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 8);
			goto CHECK_NEXT;
		}
	
		pos = 100;
		pdlg->setprogress(m_index, pos);
		bflag = TRUE;											//������¼�ɹ�
	CHECK_NEXT:
		if (g_service.m_allstop)								//����û�ֹͣ���˳��߳�
		{
			break;
		}
		else
		{
			if (bflag)
			{
				int issucc = 0;
				while (!g_service.m_allstop)
				{
					if (mhworkhandle.pop_handshake('B', 3000) != 0)	//5s �����ֲ��ɹ�
					{
						break;
					}
					else
					{
						issucc++;
						if (issucc == 1)												//��һ�����ֳɹ�������ɹ�����
							pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 11);		//������¼�ɹ� ����������

						Sleep(100);
					}
				}

				if (g_service.m_allstop)	//����û�ֹͣ���˳��߳�
				{
					goto EXIST;
				}
			}
			else
			{
				pdlg->PostMessageA(WM_MESSAGEBOOTOSINFO, m_index, 12);		//������¼ʧ�� ���������
				Sleep(3000);
			}
			continue;
		}
	}

EXIST:
	if (pFileInfo)
	{
		free(pFileInfo);
		pFileInfo = NULL;
	}
	return iRet;
}

DWORD SNThread::Run()
{
	int iRet = 0;	
	mhcommethod mhresulthandle;
	char info[1024] = { 0 };
	char sninfo[1024] = { 0 };
	char result[1024] = { 0 };	
	int  resultlen = 0;
	int noduplicate = 0;
	CString imei,sn;
	char* ps = NULL, *pe = NULL;
	HWND hwDispatch = NULL;

	std::string sntype = "";
	utility::loadconfiginfo("SNTYPE", sntype);

	if (m_comconvert)
	{
		mhresulthandle.throughcom0();
	}
	
	if (mhresulthandle.open(m_resultcom.GetBuffer()) != 0)
	{
		AfxMessageBox("���ڴ�ʧ��");
		goto Exist;
	}
	m_resultcom.ReleaseBuffer();

	if (m_needupload)
	{		
		hwDispatch = FindWindowA(NULL, m_winname);
		if (hwDispatch == NULL)
		{
			AfxMessageBox("�Ҳ������ڣ�");
			goto Exist;
		}

		iRet = mhresulthandle.GetProduceResult(result, &resultlen);
		if (iRet != 0)
		{
			iRet = mhresulthandle.GetProduceResult(result, &resultlen);
			if (iRet != 0)
			{
				pdlg->m_strsn1.SetWindowTextA("��ȡ�豸��Ϣʧ��");
				goto Exist;
			}
		}

		if (strstr(result, "fail") > 0 || strstr(result, "FAIL") > 0)
		{
			memset(info, 0, sizeof(info));
			sprintf(info, "����δͨ�� %s", result);
			pdlg->m_strsn1.SetWindowTextA(info);
			goto Exist;
		}

		if ((ps = strstr(result, "IMEI:\"")) > 0)
		{
			ps += strlen("IMEI:\"");
			pe = strstr(ps, "\"");
			memset(info, 0, sizeof(info));
			memcpy(info, ps, pe - ps);
			imei = info;
		}

		if ((ps = strstr(result, "SN:\"")) > 0)
		{
			ps += strlen("SN:\"");
			pe = strstr(ps, "\"");
			memset(info, 0, sizeof(info));
			memcpy(info, ps, pe - ps);
			sn = info;
		}

		if (imei.GetLength() <= 1)
		{
			pdlg->m_strsn1.SetWindowTextA("û�յ�IMEI��");
			goto Exist;
		}
		Sleep(2000);
	}

	if (m_sn1checked)
	{			
		if (sntype == "SL13")
		{
			if (m_restartsp)
			{
				iRet = mhresulthandle.pop_osreset();
				if (iRet != 0)
				{
					memset(info, 0, sizeof(info));
					sprintf(info, "����SN1����ʧ��, error %d", iRet);
					pdlg->m_strsn1.SetWindowTextA(info);
					goto Exist;
				}
			}

			iRet = mhresulthandle.pop_handshake('B', 5000);
			if (iRet != 0)
			{
				memset(info, 0, sizeof(info));
				sprintf(info, "SN1 ����ʧ��, error %d", iRet);
				pdlg->m_strsn1.SetWindowTextA(info);
				goto Exist;
			}
			iRet = mhresulthandle.pop_sn1downlaod(m_sn1);
			if (iRet != 0)
			{
				if (iRet == 2)
				{
					memset(sninfo, 0, sizeof(sninfo));
					iRet = mhresulthandle.pop_osgetsn1(sninfo);
					if (iRet != 0)
					{
						strcpy(sninfo, " ERROR");
					}				

					memset(info, 0, sizeof(info));
					sprintf(info, "����SN1=%s", sninfo);
					pdlg->m_strsn1.SetWindowTextA(info);
					goto Exist;
				}
				memset(info, 0, sizeof(info));
				sprintf(info, "����SN1ʧ��, error %d", iRet);
				pdlg->m_strsn1.SetWindowTextA(info);
				goto Exist;
			}
		}
		else if (sntype == "SL58")
		{
			iRet = mhresulthandle.produceSN(0x0B, m_sn1.GetBuffer());
			m_sn1.ReleaseBuffer();
			if (iRet != 0 && sn.GetLength() <= 0)
			{
				memset(info, 0, sizeof(info));
				sprintf(info, "����SN1ʧ��, error %d", iRet);
				pdlg->m_strsn1.SetWindowTextA(info);
				goto Exist;
			}
		}
		else
		{
			memset(info, 0, sizeof(info));
			sprintf(info, "����SN1 ����δ����");
			pdlg->m_strsn1.SetWindowTextA(info);
			goto Exist;
		}
	}

	Sleep(2000);
	if (m_sn2checked)
	{
		if (sntype == "SL13")
		{
			if (m_restartsp)
			{
				iRet = mhresulthandle.pop_osreset();
				if (iRet != 0)
				{
					memset(info, 0, sizeof(info));
					sprintf(info, "����SN2����ʧ��, error %d", iRet);
					pdlg->m_strsn2.SetWindowTextA(info);
					goto Exist;
				}
			}

			iRet = mhresulthandle.pop_handshake('B', 5000);
			if (iRet != 0)
			{
				memset(info, 0, sizeof(info));
				sprintf(info, "SN2 ����ʧ��, error %d", iRet);
				pdlg->m_strsn2.SetWindowTextA(info);
				goto Exist;
			}
			iRet = mhresulthandle.pop_sn2downlaod(m_sn2);
			if (iRet != 0)
			{
				if (iRet == 2)
				{
					memset(sninfo, 0, sizeof(sninfo));
					iRet = mhresulthandle.pop_osgetsn2(sninfo);
					if (iRet != 0)
					{
						strcpy(sninfo, " ERROR");
					}

					memset(info, 0, sizeof(info));
					sprintf(info, "����SN2=%s", sninfo);
					pdlg->m_strsn2.SetWindowTextA(info);
					goto Exist;
				}
				memset(info, 0, sizeof(info));
				sprintf(info, "����SN2ʧ��, error %d", iRet);
				pdlg->m_strsn2.SetWindowTextA(info);
				goto Exist;
			}
		}
		else if (sntype == "SL58")
		{
			iRet = mhresulthandle.produceSN(0x0C, m_sn2.GetBuffer());
			m_sn1.ReleaseBuffer();
			if (iRet != 0 && sn.GetLength() <= 0)
			{
				memset(info, 0, sizeof(info));
				sprintf(info, "����SN2ʧ��, error %d", iRet);
				pdlg->m_strsn2.SetWindowTextA(info);
				goto Exist;
			}
		}
		else
		{
			memset(info, 0, sizeof(info));
			sprintf(info, "����SN2 ����δ����");
			pdlg->m_strsn2.SetWindowTextA(info);
			goto Exist;
		}
	}

	if (m_needupload)
	{
		COPYDATASTRUCT MyCDS;
		
		MyCDS.dwData = 0x01;          // function identifier
		sprintf(info, "popsecu||SN=%s||IMEI=%s", m_sn1.GetBuffer(), imei.GetBuffer());
		MyCDS.cbData = strlen(info) + 1;  // size of data
		MyCDS.lpData = info;           // data structure

		iRet = SendMessage(hwDispatch, WM_COPYDATA, (WPARAM)(HWND)m_hwnd, (LPARAM)(LPVOID)&MyCDS);		
	}
	else
	{
		if (m_sn1checked)
			pdlg->m_strsn1.SetWindowTextA("SN1 PASS");
		if (m_sn2checked)
			pdlg->m_strsn2.SetWindowTextA("SN2 PASS");
	}
Exist:
	::SendMessageA(m_hwnd, WM_MESSAGESNINFO, P_END, 0);
	return iRet;
}

//��¼�ֿ����ť
void CDlgProduce::OnBnClickedBtnfontload()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (!m_isstart)
	{
		std::string oem2uni = "";
		std::string uni2oem = "";
		std::string fontfile = "";

		fontthread.pdlg = this;
		fontthread.m_hwnd = this->m_hWnd;

		m_progress6.SetWindowText(" ");
		m_progress6.SetPos(0);
		m_progress6.SetBarColor(CLR_DEFAULT);
		m_progress6.Invalidate();
		std::string strtmp = "";
		utility::loadconfiginfo("FONTCOMCOVERT", strtmp);			//�Ƿ�ת��Ϊ͸������
		fontthread.m_comconvert = atoi(strtmp.c_str());

		utility::loadconfiginfo("UNI2OEM", uni2oem);
		utility::loadconfiginfo("OEM2UNI", oem2uni);
		utility::loadconfiginfo("FONT", fontfile);

		fontthread.m_uni2oem = uni2oem.c_str();
		fontthread.m_oem2uni = oem2uni.c_str();
		fontthread.m_fontlib = fontfile.c_str();

		if (fontthread.m_uni2oem.GetLength() <= 1 || fontthread.m_oem2uni.GetLength() <= 1)
		{
			AfxMessageBox("���ȱʧ! ");
			return;
		}
		if (fontthread.m_fontlib.GetLength() <= 1 )
		{
			AfxMessageBox("�ֿ�ȱʧ!");
			return;
		}

		m_comlist.GetWindowText(fontthread.m_devname);
		if (fontthread.m_devname.GetLength() <= 1)
		{
			AfxMessageBox("�޿��ô���!");
			return;
		}

		g_service.m_allstop = false;
		fontthread.Start();
		m_snstart.EnableWindow(FALSE);
		m_start.EnableWindow(FALSE);
		m_fontstart.EnableWindow(FALSE);
	}
	else
	{
		AfxMessageBox(_T("��ֹͣ��"));
		return;
	}
}


DWORD FontThread::Run()
{
	mhcommethod mhhandle;

	int iRet = 0;
	unsigned char* oembuf = NULL;
	unsigned char* utfbuf = NULL;
	unsigned char* fontbuf = NULL;
	CFile utffile;
	CFile oemfile;
	CFile fontfile;
	
	int filelen = 0;
	char info[256] = { 0 };

	if (m_comconvert)
	{
		mhhandle.throughcom0();
	}

	iRet = mhhandle.open(m_devname.GetBuffer());
	if (iRet != 0)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"���ڴ�ʧ�� ");
		goto EXIST;
	}

	iRet = mhhandle.pop_handshake('O', POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"����ʧ�� ");
		goto EXIST;
	}

	iRet = mhhandle.eraseblockflash(0x00000, 192 * 1024);
	if (iRet != 0)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"FLASH����ʧ�� ");
		goto EXIST;
	}
	iRet = mhhandle.pop_handshake('O', POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"����ʧ�� ");
		goto EXIST;
	}

	if (!oemfile.Open(m_oem2uni, CFile::modeRead))
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"OEM2UNI������� ");
		goto EXIST;
	}
	filelen = (int)oemfile.GetLength();
	oemfile.SeekToBegin();
	oembuf = (unsigned char*)malloc(filelen);
	if (oemfile.Read(oembuf, filelen) != filelen)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"�ļ���ȡʧ�� ");
		goto EXIST;
	}
	iRet = mhhandle.writeflash(m_hwnd, 0x00000, oembuf, filelen);
	if (iRet != 0)
	{
		sprintf(info, "д��OEM2UNI���ʧ�� %d ", iRet);
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);
		goto EXIST;
	}

	iRet = mhhandle.pop_handshake('O', POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"����ʧ�� ");
		goto EXIST;
	}

	filelen = 0;

	if (!utffile.Open(m_uni2oem, CFile::modeRead))
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"UNI2OEM������� ");
		goto EXIST;
	}
	filelen = (int)utffile.GetLength();
	utffile.SeekToBegin();
	utfbuf = (unsigned char*)malloc(filelen);
	if (utffile.Read(utfbuf, filelen) != filelen)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"�ļ���ȡʧ�� ");
		goto EXIST;
	}
	iRet = mhhandle.writeflash(m_hwnd, 0x18000, utfbuf, filelen);
	if (iRet != 0)
	{
		sprintf(info, "д��UNI2OEM���ʧ�� %d ", iRet);
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);
		goto EXIST;
	}

	// download font
	iRet = mhhandle.pop_handshake('O', POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"����ʧ�� ");
		goto EXIST;
	}

	iRet = mhhandle.eraseblockflash(0x30000, 576 * 1024);
	if (iRet != 0)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"FLASH����ʧ�� ");
		goto EXIST;
	}

	iRet = mhhandle.pop_handshake('O', POP_SHAKETIMEOUT);
	if (iRet != 0)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"����ʧ�� ");
		goto EXIST;
	}

	filelen = 0;

	if (!fontfile.Open(m_fontlib, CFile::modeRead))
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"�ֿⲻ���� ");
		goto EXIST;
	}

	filelen = (int)fontfile.GetLength();
	fontfile.SeekToBegin();
	fontbuf = (unsigned char*)malloc(filelen);
	if (fontbuf == NULL)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"�ڴ����ʧ�� ");
		goto EXIST;
	}

	if (fontfile.Read(fontbuf, filelen) != filelen)
	{
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)"�ļ���ȡʧ�� ");
		goto EXIST;
	}

	iRet = mhhandle.writeflash(m_hwnd, 0x30000, fontbuf, filelen);
	if (iRet != 0)
	{
		sprintf(info, "д���ֿ�ʧ�� %d ", iRet);
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_INFO, (LPARAM)info);
		goto EXIST;
	}

EXIST:
	if (oembuf != NULL)
	{
		free(oembuf);
		oembuf = NULL;
	}
	if (utfbuf != NULL)
	{
		free(utfbuf);
		utfbuf = NULL;
	}
	if (fontbuf != NULL)
	{
		free(fontbuf);
		fontbuf = NULL;
	}
	if(iRet == 0)
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_END, 1);
	else
		SendMessage(m_hwnd, WM_MESSAGEINFO, P_END, 0);
	return iRet;
}

//���´���
void CDlgProduce::OnBnClickedBtncomupdate()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	updatecomlist();
}

void CDlgProduce::updatecomlist()
{
	std::string fontcom = "";

	m_comlist.ResetContent();
	utility::loadconfiginfo("FONTCOM", fontcom);			//���ڹؼ�������

	CArray<SSerInfo, SSerInfo&> asi;
	EnumSerialPorts(asi, FALSE);
	if (asi.GetSize() <= 0)
	{
		return;
	}

	for (int i = 0; i < asi.GetSize(); i++)
	{
		SSerInfo serInfo = asi[i];
		CString strComPort;
		CString strComPortTemp = serInfo.strFriendlyName;
		strComPortTemp.MakeUpper();

		if (strComPortTemp.Find(fontcom.c_str()) != -1)		//PROLIFIC USB-TO-SERIAL
		{
			int startIndex = strComPortTemp.Find("(COM") + 4;
			int endIndex = strComPortTemp.ReverseFind(')');

			strComPort.Format("COM%s", strComPortTemp.Mid(startIndex, endIndex - startIndex));
			m_comlist.AddString(strComPort);
		}
	}
	m_comlist.SetCurSel(0);
}

void CDlgProduce::OnBnClickedCheckprosn2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}

void CDlgProduce::OnBnClickedCheckprosn1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}

void CDlgProduce::OnBnClickedBtnsnbind()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
}

int GetBootInfo(const char* filename, ST_HEADER_INFO* info)
{
	CFile file;
	int iread = sizeof(ST_HEADER_INFO);
	if (!file.Open(filename, CFile::modeRead | CFile::shareDenyNone))
		return -1;

	file.Seek(1039, CFile::begin);

	if (iread != file.Read(info, sizeof(ST_HEADER_INFO)))
	{
		file.Close();
		return -2;
	}

	file.Close();
	return 0;
}

int GetOsInfo(const char* filename, ST_HEADER_INFO* info)
{
	CFile file;
	int iread = sizeof(ST_HEADER_INFO);
	if (!file.Open(filename, CFile::modeRead | CFile::shareDenyNone))
		return -1;

	file.SeekToBegin();

	if (iread != file.Read(info, sizeof(ST_HEADER_INFO)))
	{
		file.Close();
		return -2;
	}

	file.Close();
	return 0;
}

int GetCFGInfo(const char* filename, ST_HEADER_INFO* info)
{
	CFile file;
	int iread = sizeof(ST_HEADER_INFO);
	if (!file.Open(filename, CFile::modeRead | CFile::shareDenyNone))
		return -1;

	file.SeekToBegin();

	if (file.GetLength() != 3 * 1024)
	{
		file.Close();
		return -3;
	}

	if (iread != file.Read(info, sizeof(ST_HEADER_INFO)))
	{
		file.Close();
		return -2;
	}

	file.Close();
	return 0;
}

BOOL CDlgProduce::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (pMsg->hwnd == m_strsn1.m_hWnd || pMsg->hwnd == m_strsn2.m_hWnd)
		{
			std::string tmp = "";
			utility::loadconfiginfo("SNHANDLEENTER", tmp);
			if (atoi(tmp.c_str()))
			{
				sndownload();
				return TRUE;
			}
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

CString CDlgProduce::GetRightSN(CString str)
{
	CString strret = "";
	for (int i = 0; i < str.GetLength(); i++)
	{
		if (isalpha(str.GetAt(i)) || isalnum(str.GetAt(i)))
		{
			strret += str.GetAt(i);
		}
	}

	return strret;
}

void CDlgProduce::OnEnChangeSn1start()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�
	CString tmp = "";
	m_strsn1.GetWindowTextA(tmp);

	std::string strtmp = "";
	utility::loadconfiginfo("SNSUIXINGFU", strtmp);
	if (atoi(strtmp.c_str()) == 1 && tmp.GetLength() == 8)
	{
		tmp = "58" + tmp;
		m_strsn1.SetWindowText(tmp);
	}

	if(m_checksn2.GetCheck() && g_service.m_allstop == true)
		m_strsn2.SetWindowTextA(tmp);
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}
