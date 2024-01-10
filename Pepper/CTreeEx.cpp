/****************************************************************************************************
* Copyright © 2018-2024 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#include "stdafx.h"
#include "CTreeEx.h"

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
	GetClientRect(&rc);
	CMemDC memDC(dc, rc);
	CDC& rDC = memDC.GetDC();
	rDC.GetClipBox(rc);
	rDC.FillSolidRect(rc, RGB(255, 255, 255));

	DefWindowProcW(WM_PAINT, reinterpret_cast<WPARAM>(rDC.m_hDC), static_cast<LPARAM>(0));
}

BOOL CTreeEx::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTreeEx::OnKillFocus(CWnd* /*pNewWnd*/)
{
}

void CTreeEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CTreeCtrl::OnLButtonDown(nFlags, point);
}