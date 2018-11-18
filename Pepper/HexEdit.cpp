#include "stdafx.h"
#include "Hexedit.h"

IMPLEMENT_DYNAMIC(CHexEdit, CWnd)

BEGIN_MESSAGE_MAP(CHexEdit, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CHexEdit::Create(CWnd * pParent, const RECT & rect, UINT nID, CFont* pFont)
{
	m_pFontHexEditView = pFont;

	return CWnd::Create(nullptr, nullptr, WS_VISIBLE | WS_CHILD, rect, pParent, nID);
}

int CHexEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRuntimeClass* pNewViewClass = RUNTIME_CLASS(CHexEditView);
	CCreateContext context;
	context.m_pNewViewClass = pNewViewClass;

	m_pHexEditView = (CHexEditView*)pNewViewClass->CreateObject();
	CRect rect;
	GetClientRect(rect);
	m_pHexEditView->Create(this, rect, 0x01, &context, m_pFontHexEditView);
	m_pHexEditView->ShowWindow(SW_SHOW);

	return 0;
}

void CHexEdit::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (m_pHexEditView)
		m_pHexEditView->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CHexEdit::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

BOOL CHexEdit::SetData(const std::vector<std::byte>* pVecData) const
{
	return GetActiveView()->SetData(pVecData);
}

BOOL CHexEdit::SetData(const std::string& strData) const
{
	return GetActiveView()->SetData(strData);
}

int CHexEdit::SetFont(CFont* pFontNew) const
{
	return GetActiveView()->SetFont(pFontNew);
}

void CHexEdit::SetFontSize(UINT nSize) const
{
	return GetActiveView()->SetFontSize(nSize);
}

void CHexEdit::SetFontColor(COLORREF clrTextHex, COLORREF clrTextOffset,
	COLORREF clrTextSelected, COLORREF clrBk, COLORREF clrBkSelected) const
{
	GetActiveView()->SetFontColor(clrTextHex, clrTextOffset, clrTextSelected,
		clrBk, clrBkSelected);
}