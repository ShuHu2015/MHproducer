// MyTabCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "MyTabCtrl.h"
#include "AuthLogin.h"
#include "MHproducerDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// CMyTabCtrl

IMPLEMENT_DYNAMIC(CMyTabCtrl, CMFCTabCtrl)

CMyTabCtrl::CMyTabCtrl()
{
	m_isauthed = FALSE;
	m_issigned = FALSE;
}

CMyTabCtrl::~CMyTabCtrl()
{
}




BEGIN_MESSAGE_MAP(CMyTabCtrl, CMFCTabCtrl)
	
END_MESSAGE_MAP()



// CMyTabCtrl 消息处理程序
void CMyTabCtrl::SetTabsHeight()
{
	const int nImageHeight = m_sizeImage.cy <= 0 ? 0 : m_sizeImage.cy + 7;
	m_nTabsHeight = (max(nImageHeight, GetGlobalData()->GetTextHeight() + 12));
}

BOOL CMyTabCtrl::SetActiveTab(int iTab)
{	
	if (iTab == 1 )
	{
		if (!m_isauthed)
		{			
			//显示登录对话框
			CAuthLogin dlg(CAuthLogin::AUTH_LOGIN);
			dlg.DoModal();
			if (!dlg.islogin())
			{
				return FALSE;
			}
			m_isauthed = TRUE;
		}
		((CMHproducerDlg*)this->GetParent())->m_DlgTab2.updateuserinfo();
	}
	else if (iTab == 2)
	{
		if (!m_issigned)
		{
			CAuthLogin dlg(CAuthLogin::SIGN_LOGIN);
			dlg.DoModal();
			if (!dlg.islogin())
			{
				return FALSE;
			}			
			m_issigned = TRUE;
		}
		((CMHproducerDlg*)this->GetParent())->m_DlgTab3.updateuserinfo();
	}
	//else if (iTab == 3)
	//{
	//	if (!m_isauthed)
	//	{
	//		//显示登录对话框
	//		CAuthLogin dlg(CAuthLogin::AUTH_LOGIN);
	//		dlg.DoModal();
	//		if (!dlg.islogin())
	//		{
	//			return FALSE;
	//		}
	//		m_isauthed = TRUE;

	//	}
	//	((CMHproducerDlg*)this->GetParent())->m_DlgTab4.updateuserinfo();
	//}
	CMFCTabCtrl::SetActiveTab(iTab);
	return TRUE;
}