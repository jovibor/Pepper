#include "stdafx.h"
#include "TreeEx.h"

IMPLEMENT_DYNAMIC(CTreeEx, CTreeCtrl)

BEGIN_MESSAGE_MAP(CTreeEx, CTreeCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CTreeEx::OnPaint()
{
	//To avoid flickering.
	CPaintDC dc(this);

	CRect rc;
	dc.GetClipBox(&rc);
	CMemDC memDC(dc, rc);
	CDC& rDC = memDC.GetDC();
	rDC.GetClipBox(&rc);
	rDC.FillSolidRect(rc, RGB(255,255,255));

	DefWindowProcW(WM_PAINT, (WPARAM)rDC.m_hDC, (LPARAM)0);
}

BOOL CTreeEx::OnEraseBkgnd(CDC * pDC)
{
	return FALSE;
}

void CTreeEx::OnKillFocus(CWnd* pNewWnd)
{
	//CTreeCtrl::OnKillFocus(pNewWnd);
}

void CTreeEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CTreeCtrl::OnLButtonDown(nFlags, point);
}