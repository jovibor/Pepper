#pragma once
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "PepperList.h"

class CViewRightBottomRight : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightBottomRight)
protected:
	CViewRightBottomRight();           // protected constructor used by dynamic creation
	virtual ~CViewRightBottomRight();
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	DECLARE_MESSAGE_MAP()
private:
	CWnd* m_pActiveList { };
	Ilibpe* m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CPepperList m_stListTLSCallbacks;
	int CreateListTLSCallbacks();
};


