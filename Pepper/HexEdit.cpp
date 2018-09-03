#include "stdafx.h"
#include "HexEditView.h"
#include "HexEdit.h"

IMPLEMENT_DYNAMIC(CHexEdit, CWnd)

BEGIN_MESSAGE_MAP(CHexEdit, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CHexEdit::OnEraseBkgnd(CDC* pDC)
{
	return CWnd::OnEraseBkgnd(pDC);
}

void CHexEdit::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (m_pHexEditView)
		m_pHexEditView->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

int CHexEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRuntimeClass* pNewViewClass = RUNTIME_CLASS(CHexEditView);
	CCreateContext context;
	context.m_pNewViewClass = pNewViewClass;

	m_pHexEditView = (CHexEditView*)pNewViewClass->CreateObject();
	m_pHexEditView->Create(this, CRect(0, 0, 0, 0), 0x01, &context, m_pFontHexEditView);
	m_pHexEditView->ShowWindow(SW_SHOW);

	return 0;
}

BOOL CHexEdit::Create(CWnd * pParent, const RECT & rect, UINT nID, CFont* pFont)
{
	m_pFontHexEditView = pFont;

	return CWnd::Create(0, 0, WS_VISIBLE | WS_CHILD, rect, pParent, nID);
}

BOOL CHexEdit::SetData(const std::vector<std::byte>* vecData)
{
	return GetActiveView()->SetData(vecData);
}

BOOL CHexEdit::SetData(const std::string_view strData, UINT nCount)
{
	return GetActiveView()->SetData(strData, nCount);
}

CFont* CHexEdit::SetFont(CFont* pFont)
{
	return GetActiveView()->SetFont(pFont);
}

void CHexEdit::SetFontSize(UINT nSize)
{
	return GetActiveView()->SetFontSize(nSize);
}

void CHexEdit::SetFontColor(COLORREF clrHex, COLORREF clrOffset)
{
	GetActiveView()->SetFontColor(clrHex, clrOffset);
}
