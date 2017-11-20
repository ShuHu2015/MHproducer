#pragma once


// CMyTabCtrl

class CMyTabCtrl : public CMFCTabCtrl
{
	DECLARE_DYNAMIC(CMyTabCtrl)

public:
	CMyTabCtrl();
	virtual ~CMyTabCtrl();
	
	virtual void SetTabsHeight();
	virtual BOOL SetActiveTab(int iTab);

	BOOL m_isauthed;	//是否登录了授权服务器
	BOOL m_issigned;	//是否登录了签名服务器
protected:
	DECLARE_MESSAGE_MAP()
};


