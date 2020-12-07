/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
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
	GetClientRect(&rc);
	CMemDC memDC(dc, rc);
	CDC& rDC = memDC.GetDC();
	rDC.GetClipBox(&rc);
	rDC.FillSolidRect(rc, RGB(255,255,255));

	DefWindowProcW(WM_PAINT, reinterpret_cast<WPARAM>(rDC.m_hDC), static_cast<LPARAM>(0));
}

BOOL CTreeEx::OnEraseBkgnd(CDC* /*pDC*/)
{
	return FALSE;
}

void CTreeEx::OnKillFocus(CWnd* pNewWnd)
{
//	CTreeCtrl::OnKillFocus(pNewWnd);
}

void CTreeEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CTreeCtrl::OnLButtonDown(nFlags, point);
}