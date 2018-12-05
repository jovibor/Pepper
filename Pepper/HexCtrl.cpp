/****************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/	                *
* This is a HEX control class for MFC based apps, derived from CWnd.		*
* The usage is quite simple:												*
* 1. Construct CHexCtrl object.												*
* 2. Call Create(...) member function to create an instance.				*
* 3. Use one of SetData(...) methods to set actual data to display as hex.	*
* 4. Set window position, if needed, with hexCtrl.SetWindowPos(...).		*
*****************************************************************************/
#include "stdafx.h"
#include "HexCtrl.h"

IMPLEMENT_DYNAMIC(CHexCtrl, CWnd)

BEGIN_MESSAGE_MAP(CHexCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CHexCtrl::Create(CWnd * pParent, const RECT & rect, UINT nID, CFont* pFont)
{
	m_pFontHexView = pFont;

	return CWnd::Create(nullptr, nullptr, WS_VISIBLE | WS_CHILD, rect, pParent, nID);
}

int CHexCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRuntimeClass* pNewViewClass = RUNTIME_CLASS(CHexView);
	CCreateContext context;
	context.m_pNewViewClass = pNewViewClass;

	m_pHexView = (CHexView*)pNewViewClass->CreateObject();
	CRect rect;
	GetClientRect(rect);
	m_pHexView->Create(this, rect, 0x01, &context, m_pFontHexView);
	m_pHexView->ShowWindow(SW_SHOW);

	return 0;
}

void CHexCtrl::OnSize(UINT nType, int cx, int cy)
{
	if (m_pHexView)
		m_pHexView->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);

	CWnd::OnSize(nType, cx, cy);
}

BOOL CHexCtrl::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CHexCtrl::SetData(const std::vector<std::byte>& vecData) const
{
	if (GetActiveView())
		GetActiveView()->SetData(vecData);
}

void CHexCtrl::SetData(const std::string& strData) const
{
	if (GetActiveView())
		GetActiveView()->SetData(strData);
}

void CHexCtrl::SetData(const PBYTE pData, DWORD_PTR dwCount) const
{
	if (GetActiveView())
		GetActiveView()->SetData(pData, dwCount);
}

void CHexCtrl::ClearData()
{
	if (GetActiveView())
		GetActiveView()->ClearData();
}

int CHexCtrl::SetFont(CFont* pFontNew) const
{
	if (GetActiveView())
		return GetActiveView()->SetFont(pFontNew);
}

void CHexCtrl::SetFontSize(UINT nSize) const
{
	if (GetActiveView())
		return GetActiveView()->SetFontSize(nSize);
}

void CHexCtrl::SetFontColor(COLORREF clrTextHex, COLORREF clrTextOffset,
	COLORREF clrTextSelected, COLORREF clrBk, COLORREF clrBkSelected) const
{
	if (GetActiveView())
		GetActiveView()->SetFontColor(clrTextHex, clrTextOffset, clrTextSelected,
			clrBk, clrBkSelected);
}