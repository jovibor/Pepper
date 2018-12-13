/****************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/	                *
* CListEx and CListExHeader class implementation							*
****************************************************************************/
#include "stdafx.h"
#include "ListEx.h"
#include "strsafe.h"

/****************************************************
* CListExHeader class implementation.				*
****************************************************/
BEGIN_MESSAGE_MAP(CListEx::CListExHeader, CMFCHeaderCtrl)
	ON_MESSAGE(HDM_LAYOUT, &CListEx::CListExHeader::OnLayout)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

CListEx::CListExHeader::CListExHeader()
{
	NONCLIENTMETRICSW ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
	m_fontHdr.CreateFontIndirectW(&ncm.lfMessageFont);

	m_hdItem.mask = HDI_TEXT;
	m_hdItem.cchTextMax = MAX_PATH;
	m_hdItem.pszText = m_strHeaderText;
}

void CListEx::CListExHeader::OnDrawItem(CDC* pDC, int iItem, CRect rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	CMemDC memDC(*pDC, rect);
	CDC& rDC = memDC.GetDC();

	rDC.FillSolidRect(&rect, m_clrBk);
	rDC.SetTextColor(m_clrText);
	rDC.DrawEdge(&rect, EDGE_RAISED, BF_RECT);
	rDC.SelectObject(&m_fontHdr);

	//Set item's text buffer first char to zero,
	//then getting item's text and Draw it.
	m_strHeaderText[0] = L'\0';
	GetItem(iItem, &m_hdItem);

	if (StrStrW(m_strHeaderText, L"\n"))
	{	//If it's multiline text, first — calculate rect for the text,
		//with CALC_RECT flag (not drawing anything),
		//and then calculate rect for final vertical text alignment.
		CRect rcText;
		rDC.DrawTextW(m_strHeaderText, &rcText, DT_CENTER | DT_CALCRECT);
		rect.top = rect.bottom / 2 - rcText.bottom / 2;
		rDC.DrawTextW(m_strHeaderText, &rect, DT_CENTER);
	}
	else
		rDC.DrawTextW(m_strHeaderText, &rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
}

LRESULT CListEx::CListExHeader::OnLayout(WPARAM wParam, LPARAM lParam)
{
	CMFCHeaderCtrl::DefWindowProcW(HDM_LAYOUT, 0, lParam);

	LPHDLAYOUT pHL = reinterpret_cast<LPHDLAYOUT>(lParam);

	//New header height.
	pHL->pwpos->cy = m_dwHeaderHeight;
	//Decreasing list's height by the new header's height.
	pHL->prc->top = m_dwHeaderHeight;

	return 0;
}

void CListEx::CListExHeader::SetHeight(DWORD dwHeight)
{
	m_dwHeaderHeight = dwHeight;
}

void CListEx::CListExHeader::SetColor(COLORREF clrText, COLORREF clrBk)
{
	m_clrText = clrText;
	m_clrBk = clrBk;

	Invalidate();
	UpdateWindow();
}

void CListEx::CListExHeader::SetFont(const LOGFONT* pLogFontNew)
{
	if (!pLogFontNew)
		return;

	m_fontHdr.DeleteObject();
	m_fontHdr.CreateFontIndirectW(pLogFontNew);

	TEXTMETRIC tm;
	CDC* pDC = GetDC();
	pDC->SelectObject(&m_fontHdr);
	GetTextMetrics(pDC->m_hDC, &tm);
	ReleaseDC(pDC);
	DWORD iHeight = tm.tmHeight + tm.tmExternalLeading + 1;
	if (iHeight > m_dwHeaderHeight)
		SetHeight(iHeight);
}


/****************************************************
* CListEx class implementation.						*
****************************************************/
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
	ON_WM_MEASUREITEM_REFLECT()
END_MESSAGE_MAP()

BOOL CListEx::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, PLISTEXINFO pInfo)
{
	CMFCListCtrl::Create(dwStyle | LVS_OWNERDRAWFIXED | WS_CLIPSIBLINGS | LVS_REPORT, rect, pParentWnd, nID);

	m_hwndTooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr,
		TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr, nullptr, nullptr, nullptr);

	m_stToolInfo.cbSize = TTTOOLINFOW_V1_SIZE;
	m_stToolInfo.uFlags = TTF_TRACK;
	m_stToolInfo.uId = 0x1;

	::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
	::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)400); //to allow use of newline \n.

	NONCLIENTMETRICSW ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);

	if (pInfo)
	{
		m_clrText = pInfo->clrListText;
		m_clrBk = pInfo->clrListBk;
		m_clrTextSelected = pInfo->clrListTextSelected;
		m_clrBkSelected = pInfo->clrListBkSelected;
		m_clrTextTooltip = pInfo->clrListTextTooltip;
		::SendMessage(m_hwndTooltip, TTM_SETTIPTEXTCOLOR, (WPARAM)m_clrTextTooltip, 0);
		m_clrBkTooltip = pInfo->clrListBkTooltip;
		::SendMessage(m_hwndTooltip, TTM_SETTIPBKCOLOR, (WPARAM)m_clrBkTooltip, 0);
		m_clrTextSubitemTt = pInfo->clrListTextCellTt;
		m_clrBkSubitemTt = pInfo->clrListBkCellTt;
		m_clrGrid = pInfo->clrListGrid;
		m_dwGridWidth = pInfo->dwListGridWidth;

		if (pInfo->pListLogFont)
			m_fontList.CreateFontIndirectW(pInfo->pListLogFont);
		else
			m_fontList.CreateFontIndirectW(&ncm.lfMessageFont);

		GetHeaderCtrl().SetColor(pInfo->clrHeaderText, pInfo->clrHeaderBk);
		GetHeaderCtrl().SetHeight(pInfo->dwHeaderHeight);
		GetHeaderCtrl().SetFont(pInfo->pHeaderLogFont);
	}
	else
		m_fontList.CreateFontIndirectW(&ncm.lfMessageFont);

	m_penGrid.CreatePen(PS_SOLID, m_dwGridWidth, m_clrGrid);

	return TRUE;
}

void CListEx::SetFont(const LOGFONT* pLogFontNew)
{
	if (!pLogFontNew)
		return;

	m_fontList.DeleteObject();
	m_fontList.CreateFontIndirectW(pLogFontNew);

	//To get WM_MEASUREITEM msg after changing font.
	CRect rect;
	GetWindowRect(&rect);
	WINDOWPOS wp;
	wp.hwnd = m_hWnd;
	wp.cx = rect.Width();
	wp.cy = rect.Height();
	wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
	SendMessageW(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);

	Update(0);
}

void CListEx::SetItemTooltip(int nItem, int nSubitem, const std::wstring& strTooltip, const std::wstring& strCaption)
{
	auto it = m_umapTooltip.find(nItem);

	//If there is no tooltip for such item/subitem we just set it.
	if (it == m_umapTooltip.end())
	{
		//Initializing inner map.
		std::unordered_map<int, std::tuple< std::wstring, std::wstring>> umapInner {
			{ nSubitem, { strTooltip, strCaption } } };
		m_umapTooltip.insert({ nItem, std::move(umapInner) });
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

void CListEx::SetTooltipColor(COLORREF clrTooltipText, COLORREF clrTooltipBk, COLORREF clrTextCellTt, COLORREF clrBkCellTt)
{
	m_clrTextTooltip = clrTooltipText;
	m_clrBkTooltip = clrTooltipBk;
	m_clrTextSubitemTt = clrTextCellTt;
	m_clrBkSubitemTt = clrBkCellTt;
	::SendMessage(m_hwndTooltip, TTM_SETTIPTEXTCOLOR, (WPARAM)m_clrTextTooltip, 0);
	::SendMessage(m_hwndTooltip, TTM_SETTIPBKCOLOR, (WPARAM)m_clrBkTooltip, 0);

	Invalidate();
	UpdateWindow();
}

void CListEx::SetHeaderColor(COLORREF clrHdrText, COLORREF clrHdrBk)
{
	GetHeaderCtrl().SetColor(clrHdrText, clrHdrBk);
}

void CListEx::SetHeaderHeight(DWORD dwHeight)
{
	GetHeaderCtrl().SetHeight(dwHeight);
	Update(0);
	GetHeaderCtrl().Invalidate();
	GetHeaderCtrl().UpdateWindow();
}

void CListEx::SetHeaderFont(const LOGFONT* pLogFontNew)
{
	GetHeaderCtrl().SetFont(pLogFontNew);
	Update(0);
	GetHeaderCtrl().Invalidate();
	GetHeaderCtrl().UpdateWindow();
}

void CListEx::InitHeader()
{
	m_stListHeader.SubclassDlgItem(0, this);
}

void CListEx::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	//Set row height according to current font's height.
	TEXTMETRIC tm;
	CDC* pDC = GetDC();
	pDC->SelectObject(&m_fontList);
	GetTextMetrics(pDC->m_hDC, &tm);
	lpMIS->itemHeight = tm.tmHeight + tm.tmExternalLeading + 1;
	ReleaseDC(pDC);
}

void CListEx::DrawItem(LPDRAWITEMSTRUCT pDIS)
{
	if (pDIS->itemID == -1)
		return;

	CDC* pDC = CDC::FromHandle(pDIS->hDC);
	pDC->SelectObject(&m_penGrid);
	pDC->SelectObject(&m_fontList);
	COLORREF clrText, clrBk;

	if (HasTooltip(pDIS->itemID, 0))
	{
		clrText = m_clrTextSubitemTt;
		clrBk = m_clrBkSubitemTt;
	}
	else
	{
		clrText = m_clrText;
		clrBk = m_clrBk;
	}

	CRect rect;
	switch (pDIS->itemAction)
	{
	case ODA_SELECT:
	case ODA_DRAWENTIRE:
		GetItemRect(pDIS->itemID, &rect, LVIR_LABEL);

		if (pDIS->itemState & ODS_SELECTED)
		{
			pDIS->rcItem.left = rect.left;
			clrText = m_clrTextSelected;
			clrBk = m_clrBkSelected;
		}
		pDC->SetTextColor(clrText);
		pDC->FillSolidRect(&pDIS->rcItem, clrBk);

		rect.left += 3;
		pDC->DrawTextW(GetItemText(pDIS->itemID, 0), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
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

			//Here comes Subitems draw routine.
			//Assigning colors depending on whether subitem has tooltip,
			//and whether it's selected or not.
			if (HasTooltip(pDIS->itemID, i))
			{
				clrText = m_clrTextSubitemTt;
				clrBk = m_clrBkSubitemTt;
			}
			else
			{
				clrText = m_clrText;
				clrBk = m_clrBk;
			}
			if (pDIS->itemState & ODS_SELECTED)
			{
				clrText = m_clrTextSelected;
				clrBk = m_clrBkSelected;
			}
			pDC->SetTextColor(clrText);
			pDC->FillSolidRect(&rect, clrBk);

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
			SetTimer(ID_TIMER_TOOLTIP, 200, 0);
		}
		else
		{
			m_stCurrentSubitem.iItem = hitInfo.iItem;
			m_stCurrentSubitem.iSubItem = hitInfo.iSubItem;

			//If there is shown tooltip window.
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
	if (nIDEvent == ID_TIMER_TOOLTIP)
	{
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
			KillTimer(ID_TIMER_TOOLTIP);
			m_stCurrentSubitem.iItem = hitInfo.iItem;
			m_stCurrentSubitem.iSubItem = hitInfo.iSubItem;
		}
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
	//Drawing to CMemDC, excluding list header area (rect).
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

	CMFCListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

bool CListEx::HasTooltip(int nItem, int nSubitem, std::wstring** ppStrTipText, std::wstring** ppStrTipCaption)
{
	//Can return true/false indicating if subitem has tooltip,
	//or can return pointers to tooltip text as well, if poiters are not nullptr.
	auto it = m_umapTooltip.find(nItem);
	if (it != m_umapTooltip.end())
	{
		auto itInner = it->second.find(nSubitem);

		//If subitem id found and its text is not empty.
		if (itInner != it->second.end() && !std::get<0>(itInner->second).empty())
		{
			//If pointer for text is nullptr we just return true.
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