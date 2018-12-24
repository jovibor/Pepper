#pragma once
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "HexCtrl.h"

class CViewRightTR : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightTR)
protected:
	CViewRightTR() {}
	virtual ~CViewRightTR() {}
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CWnd* m_pActiveWnd { };
	CHexCtrl m_stHexEdit;
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};