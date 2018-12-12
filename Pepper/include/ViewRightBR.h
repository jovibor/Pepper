#pragma once
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "ListEx.h"

class CViewRightBR : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightBR)
protected:
	CViewRightBR() {}
	virtual ~CViewRightBR() {}
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	DECLARE_MESSAGE_MAP()
private:
	CWnd* m_pActiveList { };
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEXINFO m_stListInfo;
	CListEx m_stListTLSCallbacks;
	int CreateListTLSCallbacks();
};