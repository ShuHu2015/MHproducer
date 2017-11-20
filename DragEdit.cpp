// DragEdit.cpp : 实现文件
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



// CDragEdit 消息处理程序

//处理拖曳过来的文件
void CDragEdit::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	char szFileName[MAX_PATH];
	int iFileNumber;

	// 得到拖拽操作中的文件个数
	iFileNumber = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < iFileNumber; i++)
	{
		// 得到每个文件名
		DragQueryFile(hDropInfo, i, szFileName, MAX_PATH);
		if (strlen(szFileName) == 0)
			continue;

		// 把文件名添加到list中
		this->SetWindowText(szFileName);
	}
}



BOOL CDragEdit::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_ESCAPE: //Esc按键事件  
			return true;
		case VK_RETURN: //Enter按键事件  
			return true;
		default:
			break;
			;
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}
