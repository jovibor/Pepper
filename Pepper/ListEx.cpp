#include "stdafx.h"
#include "ListEx.h"

constexpr auto TIMER_TOOLTIP = 0x01;

IMPLEMENT_DYNAMIC(CListEx, CMFCListCtrl)

BEGIN_MESSAGE_MAP(CListEx, CMFCListCtrl)
	ON_WM_SETCURSOR()
	ON_WM_KILLFOCUS()
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, &CListEx::OnHdnDividerdblclick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, &CListEx::OnHdnDividerdblclick)
	ON_WM_HSCROLL()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
END_MESSAGE_MAP()

CListEx::CListEx()
{
	LOGFONT lf { };
	lf.lfHeight = 16;
	StringCchCopyW(lf.lfFaceName, 18, L"Consolas");
	if (!m_fontList.CreateFontIndirectW(&lf))
	{
		StringCchCopyW(lf.lfFaceName, 18, L"Times New Roman");
		m_fontList.CreateFontIndirectW(&lf);
	}

	m_hwndTooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	m_stToolInfo.cbSize = TTTOOLINFOW_V1_SIZE;
	m_stToolInfo.uFlags = TTF_TRACK;
	m_stToolInfo.uId = 1;

	::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
	::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)400); //to allow use of newline \n.
}

BOOL CListEx::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
	COLORREF clrText, COLORREF clrBk, COLORREF clrSelectedText, COLORREF clrSelectedBk,
	COLORREF clrTooltipText, COLORREF clrTooltipBk, COLORREF clrSubitemWithTooltipText, COLORREF clrSubitemWithTooltipBk,
	CFont* pFontList, COLORREF clrHdrText, COLORREF clrHdrBk, DWORD dwHdrHeight, CFont* pFontHdr)
{
	CMFCListCtrl::Create(dwStyle | LVS_OWNERDRAWFIXED, rect, pParentWnd, nID);

	m_clrText = clrText;
	m_clrBk = clrBk;
	m_clrTextSelected = clrSelectedText;
	m_clrSelectedBk = clrSelectedBk;
	m_clrTooltipText = clrTooltipText;
	::SendMessage(m_hwndTooltip, TTM_SETTIPTEXTCOLOR, (WPARAM)m_clrTooltipText, 0);
	m_clrTooltipBk = clrTooltipBk;
	::SendMessage(m_hwndTooltip, TTM_SETTIPBKCOLOR, (WPARAM)m_clrTooltipBk, 0);
	m_clrSubitemWithTooltipText = clrSubitemWithTooltipText;
	m_clrSubitemWithTooltipBk = clrSubitemWithTooltipBk;
	if (pFontList)
	{
		m_fontList.DeleteObject();
		LOGFONT lf;
		pFontList->GetLogFont(&lf);
		m_fontList.CreateFontIndirectW(&lf);
	}
	GetHeaderCtrl().SetHdrColor(clrHdrBk, clrHdrText);
	GetHeaderCtrl().SetHeight(dwHdrHeight);
	GetHeaderCtrl().SetFont(pFontHdr);

	return TRUE;
}

void CListEx::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_fTooltipExist)
	{
		LVHITTESTINFO hitInfo { };
		hitInfo.pt = point;
		ListView_SubItemHitTest(m_hWnd, &hitInfo);
		std::wstring  *pStrTipText { }, *pStrTipCaption { };

		bool fHasTooltip = HasTooltip(hitInfo.iItem, hitInfo.iSubItem, &pStrTipText, &pStrTipCaption);

		if (fHasTooltip)
		{
			//Check if cursor is still in the same cell's rect. If so - just leave.
			if (m_stCurrentSubitem.iItem == hitInfo.iItem && m_stCurrentSubitem.iSubItem == hitInfo.iSubItem)
				return;

			m_fTooltipShown = true;

			m_stCurrentSubitem.iItem = hitInfo.iItem;
			m_stCurrentSubitem.iSubItem = hitInfo.iSubItem;
			m_stToolInfo.lpszText = pStrTipText->data();

			ClientToScreen(&point);
			::SendMessage(m_hwndTooltip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(point.x, point.y));
			::SendMessage(m_hwndTooltip, TTM_SETTITLE, (WPARAM)TTI_NONE, (LPARAM)pStrTipCaption->data());
			::SendMessage(m_hwndTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
			::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);

			//Timer to check whether mouse left subitem rect.
			SetTimer(TIMER_TOOLTIP, 200, 0);
		}
		else
		{
			m_stCurrentSubitem.iItem = hitInfo.iItem;
			m_stCurrentSubitem.iSubItem = hitInfo.iSubItem;

			//If there is showed tooltip window.
			if (m_fTooltipShown)
			{
				m_fTooltipShown = false;
				::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
			}
		}
	}
}

BOOL CListEx::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	GetHeaderCtrl().Invalidate();
	GetHeaderCtrl().UpdateWindow();

	return CMFCListCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

void CListEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	LVHITTESTINFO hitInfo { };
	hitInfo.pt = point;
	ListView_SubItemHitTest(m_hWnd, &hitInfo);
	if (hitInfo.iSubItem == -1 || hitInfo.iItem == -1)
		return;

	CMFCListCtrl::OnLButtonDown(nFlags, point);
}

void CListEx::OnRButtonDown(UINT nFlags, CPoint point)
{
	LVHITTESTINFO hitInfo { };
	hitInfo.pt = point;
	ListView_SubItemHitTest(m_hWnd, &hitInfo);
	if (hitInfo.iSubItem == -1 || hitInfo.iItem == -1)
		return;

	CMFCListCtrl::OnRButtonDown(nFlags, point);
}

void CListEx::OnTimer(UINT_PTR nIDEvent)
{
	//Checking if mouse left list's subitem rect,
	//if so — hiding tooltip and killing timer.

	LVHITTESTINFO hitInfo { };
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	hitInfo.pt = point;
	ListView_SubItemHitTest(m_hWnd, &hitInfo);

	//If cursor is still hovers subitem then do nothing.
	if (m_stCurrentSubitem.iItem == hitInfo.iItem && m_stCurrentSubitem.iSubItem == hitInfo.iSubItem)
		return;
	else
	{	//If it left.
		m_fTooltipShown = false;
		::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
		KillTimer(TIMER_TOOLTIP);
		m_stCurrentSubitem.iItem = hitInfo.iItem;
		m_stCurrentSubitem.iSubItem = hitInfo.iSubItem;
	}

	CMFCListCtrl::OnTimer(nIDEvent);
}

BOOL CListEx::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	SetFocus();

	return CMFCListCtrl::OnSetCursor(pWnd, nHitTest, message);
}

void CListEx::OnKillFocus(CWnd* pNewWnd)
{
}

BOOL CListEx::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CListEx::OnPaint()
{
	CPaintDC dc(this);

	//To avoid flickering.
	//Drawing to CMemDC, excluding CListHeader area (rect).
	CRect rcClient, rcHdr;
	GetClientRect(&rcClient);
	GetHeaderCtrl().GetClientRect(rcHdr);
	rcClient.top += rcHdr.Height();
	CMemDC memDC(dc, rcClient);
	CDC& rDC = memDC.GetDC();

	CRect clip;
	rDC.GetClipBox(&clip);
	rDC.FillSolidRect(clip, m_clrBk);

	DefWindowProc(WM_PAINT, (WPARAM)rDC.m_hDC, (LPARAM)0);
}

void CListEx::OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	//	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;
}

void CListEx::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CMFCListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CListEx::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	GetHeaderCtrl().Invalidate();
	GetHeaderCtrl().UpdateWindow();

	CMFCListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CListEx::DrawItem(LPDRAWITEMSTRUCT pDIS)
{
	if (pDIS->itemID == -1)
		return;

	CDC* pDC = CDC::FromHandle(pDIS->hDC);
	pDC->SelectObject(&m_penForRect);
	pDC->SelectObject(&m_fontList);
	CRect rect;

	switch (pDIS->itemAction)
	{
	case ODA_SELECT:
	case ODA_DRAWENTIRE:
		if (HasTooltip(pDIS->itemID, 0))
			pDC->FillSolidRect(&rect, m_clrSubitemWithTooltipBk);
		else
			pDC->FillSolidRect(&pDIS->rcItem, m_clrBk);

		GetItemRect(pDIS->itemID, &rect, LVIR_LABEL);

		if (pDIS->itemState & ODS_SELECTED)
		{
			pDIS->rcItem.left = rect.left;
			pDC->SetTextColor(m_clrTextSelected);
			pDC->FillSolidRect(&pDIS->rcItem, m_clrSelectedBk);
		}
		else
			pDC->SetTextColor(m_clrText);

		rect.left += 3;
		pDC->DrawText(GetItemText(pDIS->itemID, 0), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		rect.left -= 3;

		//Drawing Item's rect lines. 
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.right, rect.top);
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.left, rect.bottom);
		pDC->MoveTo(rect.left, rect.bottom);
		pDC->LineTo(rect.right, rect.bottom);
		pDC->MoveTo(rect.right, rect.top);
		pDC->LineTo(rect.right, rect.bottom);

		for (int i = 1; i < GetHeaderCtrl().GetItemCount(); i++)
		{
			GetSubItemRect(pDIS->itemID, i, LVIR_BOUNDS, rect);

			if (HasTooltip(pDIS->itemID, i))
			{
				pDC->SetTextColor(m_clrSubitemWithTooltipText);
				pDC->FillSolidRect(&rect, m_clrSubitemWithTooltipBk);
			}

			CString textItem = GetItemText(pDIS->itemID, i);
			rect.left += 3; //Drawing text +-3 px from rect bounds
			ExtTextOutW(pDC->m_hDC, rect.left, rect.top, ETO_CLIPPED, rect, textItem, textItem.GetLength(), nullptr);
			rect.left -= 3;

			//Drawing Subitem's rect lines. 
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right, rect.top);
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.left, rect.bottom);
			pDC->MoveTo(rect.left, rect.bottom);
			pDC->LineTo(rect.right, rect.bottom);
			pDC->MoveTo(rect.right, rect.top);
			pDC->LineTo(rect.right, rect.bottom);
		}

		break;

	case ODA_FOCUS:
		break;
	}
}

void CListEx::InitHeader()
{
	m_stListHeader.SubclassDlgItem(0, this);
}

int CListEx::SetFont(CFont* pFontNew)
{
	if (!pFontNew)
		return -1;

	LOGFONT lf;
	pFontNew->GetLogFont(&lf);
	m_fontList.DeleteObject();
	m_fontList.CreateFontIndirectW(&lf);

	Update(0);

	return 0;
}

void CListEx::SetItemTooltip(int nItem, int nSubitem, const std::wstring& strTooltip, const std::wstring& strCaption)
{
	auto it = m_umapTooltip.find(nItem);

	//If there is no tooltip for such item/subitem we just set it.
	if (it == m_umapTooltip.end())
	{
		//Initializing inner map.
		std::unordered_map<int, std::tuple< std::wstring, std::wstring>> umapInner = {
			{ nSubitem, { strTooltip, strCaption } } };
		m_umapTooltip.insert({ nItem, umapInner });
	}
	else
	{
		auto itInner = it->second.find(nSubitem);

		//If there is Item's tooltip but no Subitem's tooltip
		//inserting new Subitem into inner map.
		if (itInner == it->second.end())
			it->second.insert({ nSubitem, { strTooltip, strCaption } });
		else //If there is already such an Item-Subitem's tooltip, just change it.
			itInner->second = { strTooltip, strCaption };
	}

	m_fTooltipExist = true;
}

void CListEx::SetTooltipColor(COLORREF clrTooltipBk, COLORREF clrTooltipText, COLORREF clrSubitemWithTooltip)
{
	m_clrSubitemWithTooltipBk = clrSubitemWithTooltip;
	m_clrTooltipBk = clrTooltipBk;
	m_clrTooltipText = clrTooltipText;
	::SendMessage(m_hwndTooltip, TTM_SETTIPBKCOLOR, (WPARAM)m_clrTooltipBk, 0);
	::SendMessage(m_hwndTooltip, TTM_SETTIPTEXTCOLOR, (WPARAM)m_clrTooltipText, 0);

	Invalidate();
	UpdateWindow();
}

void CListEx::SetHeaderColor(COLORREF clrHdrBk, COLORREF clrHdrText)
{
	GetHeaderCtrl().SetHdrColor(clrHdrBk, clrHdrText);
}

void CListEx::SetHeaderHeight(DWORD dwHeight)
{
	GetHeaderCtrl().SetHeight(dwHeight);
	Update(0);
	GetHeaderCtrl().Invalidate();
	GetHeaderCtrl().UpdateWindow();
}

int CListEx::SetHeaderFont(CFont* pFontNew)
{
	return GetHeaderCtrl().SetFont(pFontNew);
}

bool CListEx::HasTooltip(int nItem, int nSubitem, std::wstring** ppStrTipText, std::wstring** ppStrTipCaption)
{
	//Can return true/false indicating if subitem has tooltip,
	//or can return pointers to tooltip text as well, if poiters are not nullptr.
	auto it = m_umapTooltip.find(nItem);
	if (it != m_umapTooltip.end())
	{
		auto itInner = it->second.find(nSubitem);
		if (itInner != it->second.end())
		{
			//If pointers are nullptr we just return true.
			if (ppStrTipText)
			{
				*ppStrTipText = &std::get<0>(itInner->second);
				if (ppStrTipCaption)
					*ppStrTipCaption = &std::get<1>(itInner->second);
			}
			return true;
		}
	}

	return false;
}