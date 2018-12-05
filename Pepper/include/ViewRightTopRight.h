#pragma once
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "HexCtrl.h"

class CViewRightTopRight : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightTopRight)
protected:
	CViewRightTopRight() {}
	virtual ~CViewRightTopRight() {}
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CWnd* m_pActiveList { };
	CHexCtrl m_stHexEdit;
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};