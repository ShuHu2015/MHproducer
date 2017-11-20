// DragEdit.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MHproducer.h"
#include "DragEdit.h"


// CDragEdit

IMPLEMENT_DYNAMIC(CDragEdit, CEdit)

CDragEdit::CDragEdit()
{

}

CDragEdit::~CDragEdit()
{
}


BEGIN_MESSAGE_MAP(CDragEdit, CEdit)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()



// CDragEdit ��Ϣ�������

//������ҷ�������ļ�
void CDragEdit::OnDropFiles(HDROP hDropInfo)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	char szFileName[MAX_PATH];
	int iFileNumber;

	// �õ���ק�����е��ļ�����
	iFileNumber = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < iFileNumber; i++)
	{
		// �õ�ÿ���ļ���
		DragQueryFile(hDropInfo, i, szFileName, MAX_PATH);
		if (strlen(szFileName) == 0)
			continue;

		// ���ļ�����ӵ�list��
		this->SetWindowText(szFileName);
	}
}



BOOL CDragEdit::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_ESCAPE: //Esc�����¼�  
			return true;
		case VK_RETURN: //Enter�����¼�  
			return true;
		default:
			break;
			;
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}
