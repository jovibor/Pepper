#include "stdafx.h"
#include "PepperListHeader.h"

IMPLEMENT_DYNAMIC(CPepperListHeader, CMFCHeaderCtrl)

BEGIN_MESSAGE_MAP(CPepperListHeader, CMFCHeaderCtrl)
	ON_MESSAGE(HDM_LAYOUT, OnLayout)
END_MESSAGE_MAP()

CPepperListHeader::CPepperListHeader()
{
	LOGFONT lf { };
	lf.lfHeight = 17;
	lf.lfWeight = FW_BOLD;
	StringCchCopyW(lf.lfFaceName, 16, L"Times New Roman");
	m_fontHeader.CreateFontIndirectW(&lf);

	m_hdItem.mask = HDI_TEXT;
	m_hdItem.cchTextMax = MAX_PATH;
	m_hdItem.pszText = m_strHeaderText;
}

CPepperListHeader::~CPepperListHeader()
{
	m_fontHeader.DeleteObject();
}

void CPepperListHeader::OnDrawItem(CDC* pDC, int iItem, CRect rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	pDC->FillSolidRect(&rect, m_colorHeader);
	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->DrawEdge(&rect, EDGE_RAISED, BF_RECT);
	pDC->SelectObject(&m_fontHeader);

	m_strHeaderText[0] = L'\0';
	GetItem(iItem, &m_hdItem);

	if (StrStrW(m_strHeaderText, L"\n"))
		pDC->DrawTextW(m_strHeaderText, &rect, DT_VCENTER | DT_CENTER);
	else
		pDC->DrawTextW(m_strHeaderText, &rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
}

LRESULT  CPepperListHeader::OnLayout(WPARAM wParam, LPARAM lParam)
{
	CHeaderCtrl::DefWindowProcW(HDM_LAYOUT, 0, lParam);

	LPHDLAYOUT pHL = reinterpret_cast<LPHDLAYOUT>(lParam);

	//New header height
	pHL->pwpos->cy += 16;
	//Decreases list height by the new header's height
	pHL->prc->top += 16;

	return 0;
}
