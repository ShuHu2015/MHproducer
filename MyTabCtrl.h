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

	BOOL m_isauthed;	//�Ƿ��¼����Ȩ������
	BOOL m_issigned;	//�Ƿ��¼��ǩ��������
protected:
	DECLARE_MESSAGE_MAP()
};


