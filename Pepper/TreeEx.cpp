#include "stdafx.h"
#include "TreeEx.h"

IMPLEMENT_DYNAMIC(CTreeEx, CTreeCtrl)

BEGIN_MESSAGE_MAP(CTreeEx, CTreeCtrl)
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

void CTreeEx::OnKillFocus(CWnd* pNewWnd)
{
	//	CTreeCtrl::OnKillFocus(pNewWnd);
}

void CTreeEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CTreeCtrl::OnLButtonDown(nFlags, point);
}