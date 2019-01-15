/****************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/	                *
* This is an extended and quite featured version of CMFCListCtrl class.		*
* The main difference is in CListEx::Create method, which takes one			*
* additional arg - pointer to LISTEXINFO structure, which fields are		*
* described below.															*
* Also, this class has set of additional public methods to help customize	*
* your control in many different aspects.									*
****************************************************************************/
#include "stdafx.h"
#include "ListEx.h"
#include "strsafe.h"

using namespace LISTEX;
/****************************************************
* CListExHeader class implementation.				*
****************************************************/
BEGIN_MESSAGE_MAP(CListExHeader, CMFCHeaderCtrl)
	ON_MESSAGE(HDM_LAYOUT, &CListExHeader::OnLayout)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

CListExHeader::CListExHeader()
{
	NONCLIENTMETRICSW ncm { };
	ncm.cbSize = sizeof(NONCLIENTMETRICSW);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
	ncm.lfMessageFont.lfHeight = 18; //For some weird reason above func returns this value as MAX_LONG.

	m_fontHdr.CreateFontIndirectW(&ncm.lfMessageFont);

	m_hdItem.mask = HDI_TEXT;
	m_hdItem.cchTextMax = MAX_PATH;
	m_hdItem.pszText = m_wstrHeaderText;
}

void CListExHeader::OnDrawItem(CDC* pDC, int iItem, CRect rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	CMemDC memDC(*pDC, rect);
	CDC& rDC = memDC.GetDC();

	if (iItem < 0) { //Non working area, after last column.
		rDC.FillSolidRect(&rect, m_clrBkNWA);
		return;
	}

	if (m_umapClrColumn.find(iItem) != m_umapClrColumn.end())
		rDC.FillSolidRect(&rect, m_umapClrColumn.at(iItem));
	else
		rDC.FillSolidRect(&rect, m_clrBk);
	rDC.SetTextColor(m_clrText);
	rDC.DrawEdge(&rect, EDGE_RAISED, BF_RECT);
	rDC.SelectObject(&m_fontHdr);

	//Set item's text buffer first char to zero,
	//then getting item's text and Draw it.
	m_wstrHeaderText[0] = L'\0';
	GetItem(iItem, &m_hdItem);

	if (StrStrW(m_wstrHeaderText, L"\n"))
	{	//If it's multiline text, first — calculate rc for the text,
		//with CALC_RECT flag (not drawing anything),
		//and then calculate rc for final vertical text alignment.
		CRect rcText;
		rDC.DrawTextW(m_wstrHeaderText, &rcText, DT_CENTER | DT_CALCRECT);
		rect.top = rect.Height() / 2 - rcText.Height() / 2;
		rDC.DrawTextW(m_wstrHeaderText, &rect, DT_CENTER);
	}
	else
		rDC.DrawTextW(m_wstrHeaderText, &rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
}

LRESULT CListExHeader::OnLayout(WPARAM wParam, LPARAM lParam)
{
	CMFCHeaderCtrl::DefWindowProcW(HDM_LAYOUT, 0, lParam);

	LPHDLAYOUT pHL = reinterpret_cast<LPHDLAYOUT>(lParam);

	//New header height.
	pHL->pwpos->cy = m_dwHeaderHeight;
	//Decreasing list's height by the new header's height.
	pHL->prc->top = m_dwHeaderHeight;

	return 0;
}

void CListExHeader::SetHeight(DWORD dwHeight)
{
	m_dwHeaderHeight = dwHeight;
}

void CListExHeader::SetColor(COLORREF clrText, COLORREF clrBk)
{
	m_clrText = clrText;
	m_clrBk = clrBk;

	RedrawWindow();
}

void CListExHeader::SetColumnColor(DWORD iColumn, COLORREF clr)
{
	m_umapClrColumn[iColumn] = clr;
	RedrawWindow();
}

void CListExHeader::SetFont(const LOGFONT* pLogFontNew)
{
	if (!pLogFontNew)
		return;

	m_fontHdr.DeleteObject();
	m_fontHdr.CreateFontIndirectW(pLogFontNew);

	//If new font's height is higher than current height (m_dwHeaderHeight)
	//we adjust current height as well.
	TEXTMETRIC tm;
	CDC* pDC = GetDC();
	pDC->SelectObject(&m_fontHdr);
	GetTextMetricsW(pDC->m_hDC, &tm);
	ReleaseDC(pDC);
	DWORD dwHeightFont = tm.tmHeight + tm.tmExternalLeading + 1;
	if (dwHeightFont > m_dwHeaderHeight)
		SetHeight(dwHeightFont);
}


/****************************************************
* CListEx class implementation.						*
****************************************************/
IMPLEMENT_DYNAMIC(CListEx, CMFCListCtrl)

BEGIN_MESSAGE_MAP(CListEx, CMFCListCtrl)
	ON_WM_SETCURSOR()
	ON_WM_KILLFOCUS()
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
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, &CListEx::OnHdnDividerdblclick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, &CListEx::OnHdnDividerdblclick)
	ON_NOTIFY(HDN_BEGINTRACKA, 0, &CListEx::OnHdnBegintrack)
	ON_NOTIFY(HDN_BEGINTRACKW, 0, &CListEx::OnHdnBegintrack)
	ON_NOTIFY(HDN_TRACKA, 0, &CListEx::OnHdnTrack)
	ON_NOTIFY(HDN_TRACKW, 0, &CListEx::OnHdnTrack)
END_MESSAGE_MAP()

BOOL CListEx::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, PLISTEXINFO pListExInfo)
{
	BOOL fRet = CMFCListCtrl::Create(dwStyle | LVS_OWNERDRAWFIXED | LVS_REPORT, rect, pParentWnd, nID);

	m_hwndTt = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr,
		TTS_BALLOON | TTS_NOANIMATE | TTS_NOFADE | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr, nullptr, nullptr, nullptr);
	SetWindowTheme(m_hwndTt, nullptr, L""); //To prevent Windows from changing theme of Balloon window.

	m_stToolInfo.cbSize = TTTOOLINFOW_V1_SIZE;
	m_stToolInfo.uFlags = TTF_TRACK;
	m_stToolInfo.uId = 0x1;
	::SendMessageW(m_hwndTt, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
	::SendMessageW(m_hwndTt, TTM_SETMAXTIPWIDTH, 0, (LPARAM)400); //to allow use of newline \n.

	NONCLIENTMETRICSW ncm { };
	ncm.cbSize = sizeof(NONCLIENTMETRICSW);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
	ncm.lfMessageFont.lfHeight = 18; //For some weird reason above func returns this value as MAX_LONG.

	if (pListExInfo)
	{
		m_clrText = pListExInfo->clrListText;
		m_clrBkRow1 = pListExInfo->clrListBkRow1;
		m_clrBkRow2 = pListExInfo->clrListBkRow2;
		m_clrTextSelected = pListExInfo->clrListTextSelected;
		m_clrBkSelected = pListExInfo->clrListBkSelected;
		m_clrTooltipText = pListExInfo->clrTooltipText;
		::SendMessageW(m_hwndTt, TTM_SETTIPTEXTCOLOR, (WPARAM)m_clrTooltipText, 0);
		m_clrTooltipBk = pListExInfo->clrTooltipBk;
		::SendMessageW(m_hwndTt, TTM_SETTIPBKCOLOR, (WPARAM)m_clrTooltipBk, 0);
		m_clrTextCellTt = pListExInfo->clrListTextCellTt;
		m_clrBkCellTt = pListExInfo->clrListBkCellTt;
		m_clrGrid = pListExInfo->clrListGrid;
		m_dwGridWidth = pListExInfo->dwListGridWidth;

		if (pListExInfo->pListLogFont)
			m_fontList.CreateFontIndirectW(pListExInfo->pListLogFont);
		else
			m_fontList.CreateFontIndirectW(&ncm.lfMessageFont);

		GetHeaderCtrl().SetColor(pListExInfo->clrHeaderText, pListExInfo->clrHeaderBk);
		GetHeaderCtrl().SetHeight(pListExInfo->dwHeaderHeight);
		GetHeaderCtrl().SetFont(pListExInfo->pHeaderLogFont);
	}
	else
		m_fontList.CreateFontIndirectW(&ncm.lfMessageFont);

	m_penGrid.CreatePen(PS_SOLID, m_dwGridWidth, m_clrGrid);

	return fRet;
}

void CListEx::SetFont(const LOGFONT* pLogFontNew)
{
	if (!pLogFontNew)
		return;

	m_fontList.DeleteObject();
	m_fontList.CreateFontIndirectW(pLogFontNew);

	//To get WM_MEASUREITEM msg after changing font.
	CRect rc;
	GetWindowRect(&rc);
	WINDOWPOS wp;
	wp.hwnd = m_hWnd;
	wp.cx = rc.Width();
	wp.cy = rc.Height();
	wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
	SendMessageW(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);

	Update(0);
}

void CListEx::SetFontSize(UINT uiSize)
{
	//Prevent size from being too small or too big.
	if (uiSize < 9 || uiSize > 75)
		return;

	LOGFONT lf;
	m_fontList.GetLogFont(&lf);
	lf.lfHeight = uiSize;
	m_fontList.DeleteObject();
	m_fontList.CreateFontIndirectW(&lf);

	//To get WM_MEASUREITEM msg after changing font.
	CRect rc;
	GetWindowRect(&rc);
	WINDOWPOS wp;
	wp.hwnd = m_hWnd;
	wp.cx = rc.Width();
	wp.cy = rc.Height();
	wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
	SendMessageW(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);

	Update(0);
}

UINT CListEx::GetFontSize()
{
	LOGFONT lf;
	m_fontList.GetLogFont(&lf);

	return lf.lfHeight;
}

void CListEx::SetCellTooltip(int iItem, int iSubitem, const std::wstring& wstrTt, const std::wstring& wstrCaption)
{
	auto it = m_umapTooltip.find(iItem);

	//If there is no tooltip for such item/subitem we just set it.
	if (it == m_umapTooltip.end() && (!wstrTt.empty() || !wstrCaption.empty()))
	{
		//Initializing inner map.
		std::unordered_map<int, std::tuple< std::wstring, std::wstring>> umapInner {
			{ iSubitem, { wstrTt, wstrCaption } } };
		m_umapTooltip.insert({ iItem, std::move(umapInner) });
	}
	else
	{
		auto itInner = it->second.find(iSubitem);

		//If there is Item's tooltip but no Subitem's tooltip
		//inserting new Subitem into inner map.
		if (itInner == it->second.end())
			it->second.insert({ iSubitem, { wstrTt, wstrCaption } });
		else //If there is already exist this Item-Subitem's tooltip:
			 //change or erase it, depending on wstrTt emptiness.
			if (!wstrTt.empty())
				itInner->second = { wstrTt, wstrCaption };
			else
				it->second.erase(itInner);
	}
}

void CListEx::SetTooltipColor(COLORREF clrTooltipText, COLORREF clrTooltipBk, COLORREF clrTextCellTt, COLORREF clrBkCellTt)
{
	m_clrTooltipText = clrTooltipText;
	m_clrTooltipBk = clrTooltipBk;
	m_clrTextCellTt = clrTextCellTt;
	m_clrBkCellTt = clrBkCellTt;
	::SendMessage(m_hwndTt, TTM_SETTIPTEXTCOLOR, (WPARAM)m_clrTooltipText, 0);
	::SendMessage(m_hwndTt, TTM_SETTIPBKCOLOR, (WPARAM)m_clrTooltipBk, 0);

	RedrawWindow();
}

void CListEx::SetHeaderColor(COLORREF clrHdrText, COLORREF clrHdrBk)
{
	GetHeaderCtrl().SetColor(clrHdrText, clrHdrBk);
}

void CListEx::SetHeaderHeight(DWORD dwHeight)
{
	GetHeaderCtrl().SetHeight(dwHeight);
	Update(0);
	GetHeaderCtrl().RedrawWindow();
}

void CListEx::SetHeaderFont(const LOGFONT* pLogFontNew)
{
	GetHeaderCtrl().SetFont(pLogFontNew);
	Update(0);
	GetHeaderCtrl().RedrawWindow();
}

void CListEx::SetHeaderColumnColor(DWORD nColumn, COLORREF clr)
{
	GetHeaderCtrl().SetColumnColor(nColumn, clr);
	Update(0);
	GetHeaderCtrl().RedrawWindow();
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
	COLORREF clrBkCurrRow = pDIS->itemID % 2 ? m_clrBkRow2 : m_clrBkRow1;

	if (HasTooltip(pDIS->itemID, 0))
	{
		clrText = m_clrTextCellTt;
		clrBk = m_clrBkCellTt;
	}
	else
	{
		clrText = m_clrText;
		clrBk = clrBkCurrRow;
	}

	switch (pDIS->itemAction)
	{
	case ODA_SELECT:
	case ODA_DRAWENTIRE:
	{
		CRect rc;
		GetItemRect(pDIS->itemID, &rc, LVIR_LABEL);

		if (pDIS->itemState & ODS_SELECTED)
		{
			pDIS->rcItem.left = rc.left;
			clrText = m_clrTextSelected;
			clrBk = m_clrBkSelected;
		}
		pDC->SetTextColor(clrText);
		pDC->FillSolidRect(&pDIS->rcItem, clrBk);

		rc.left += 3;
		pDC->DrawTextW(GetItemText(pDIS->itemID, 0), &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		rc.left -= 3;

		//Drawing Item's rc lines. 
		pDC->MoveTo(rc.left, rc.top);
		pDC->LineTo(rc.right, rc.top);
		pDC->MoveTo(rc.left, rc.top);
		pDC->LineTo(rc.left, rc.bottom);
		pDC->MoveTo(rc.left, rc.bottom);
		pDC->LineTo(rc.right, rc.bottom);
		pDC->MoveTo(rc.right, rc.top);
		pDC->LineTo(rc.right, rc.bottom);

		for (int i = 1; i < GetHeaderCtrl().GetItemCount(); i++)
		{
			GetSubItemRect(pDIS->itemID, i, LVIR_BOUNDS, rc);

			//Here comes Subitems draw routine.
			//Assigning colors depending on whether subitem has tooltip,
			//and whether it's selected or not.
			if (HasTooltip(pDIS->itemID, i))
			{
				clrText = m_clrTextCellTt;
				clrBk = m_clrBkCellTt;
			}
			else
			{
				clrText = m_clrText;
				clrBk = clrBkCurrRow;
			}
			if (pDIS->itemState & ODS_SELECTED)
			{
				clrText = m_clrTextSelected;
				clrBk = m_clrBkSelected;
			}
			pDC->SetTextColor(clrText);
			pDC->FillSolidRect(&rc, clrBk);

			CString strItem = GetItemText(pDIS->itemID, i);
			rc.left += 3; //Drawing text +-3 px from rc bounds
			ExtTextOutW(pDC->m_hDC, rc.left, rc.top, ETO_CLIPPED, rc, strItem, strItem.GetLength(), nullptr);
			rc.left -= 3;

			//Drawing Subitem's rc lines. 
			pDC->MoveTo(rc.left, rc.top);
			pDC->LineTo(rc.right, rc.top);
			pDC->MoveTo(rc.left, rc.top);
			pDC->LineTo(rc.left, rc.bottom);
			pDC->MoveTo(rc.left, rc.bottom);
			pDC->LineTo(rc.right, rc.bottom);
			pDC->MoveTo(rc.right, rc.top);
			pDC->LineTo(rc.right, rc.bottom);
		}
	}
	break;
	case ODA_FOCUS:
		break;
	}
}

void CListEx::OnMouseMove(UINT nFlags, CPoint pt)
{
	LVHITTESTINFO hi { };
	hi.pt = pt;
	ListView_SubItemHitTest(m_hWnd, &hi);
	std::wstring  *pwstrTt { }, *pwstrCaption { };

	bool fHasTooltip = HasTooltip(hi.iItem, hi.iSubItem, &pwstrTt, &pwstrCaption);

	if (fHasTooltip)
	{
		//Check if cursor is still in the same cell's rc. If so - just leave.
		if (m_stCurrCell.iItem == hi.iItem && m_stCurrCell.iSubItem == hi.iSubItem)
			return;

		m_fTtShown = true;
		m_stCurrCell.iItem = hi.iItem;
		m_stCurrCell.iSubItem = hi.iSubItem;
		m_stToolInfo.lpszText = const_cast<LPWSTR>(pwstrTt->data());

		ClientToScreen(&pt);
		::SendMessage(m_hwndTt, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x, pt.y));
		::SendMessage(m_hwndTt, TTM_SETTITLE, (WPARAM)TTI_NONE, (LPARAM)pwstrCaption->data());
		::SendMessage(m_hwndTt, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
		::SendMessage(m_hwndTt, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);

		//Timer to check whether mouse left subitem rc.
		SetTimer(ID_TIMER_TOOLTIP, 200, 0);
	}
	else
	{
		m_stCurrCell.iItem = hi.iItem;
		m_stCurrCell.iSubItem = hi.iSubItem;

		//If there is shown tooltip window.
		if (m_fTtShown)
		{
			m_fTtShown = false;
			::SendMessage(m_hwndTt, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
		}
	}
}

BOOL CListEx::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (nFlags == MK_CONTROL)
	{
		SetFontSize(GetFontSize() + zDelta / WHEEL_DELTA * 2);
		return TRUE;
	}
	GetHeaderCtrl().RedrawWindow();

	return CMFCListCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

void CListEx::OnLButtonDown(UINT nFlags, CPoint pt)
{
	LVHITTESTINFO hi { };
	hi.pt = pt;
	ListView_SubItemHitTest(m_hWnd, &hi);
	if (hi.iSubItem == -1 || hi.iItem == -1)
		return;

	CMFCListCtrl::OnLButtonDown(nFlags, pt);
}

void CListEx::OnRButtonDown(UINT nFlags, CPoint pt)
{
	LVHITTESTINFO hi { };
	hi.pt = pt;
	ListView_SubItemHitTest(m_hWnd, &hi);
	if (hi.iSubItem == -1 || hi.iItem == -1)
		return;

	CMFCListCtrl::OnRButtonDown(nFlags, pt);
}

void CListEx::OnTimer(UINT_PTR nIDEvent)
{
	//Checking if mouse left list's subitem rc,
	//if so — hiding tooltip and killing timer.
	if (nIDEvent == ID_TIMER_TOOLTIP)
	{
		LVHITTESTINFO hitInfo { };
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		hitInfo.pt = pt;
		ListView_SubItemHitTest(m_hWnd, &hitInfo);

		//If cursor is still hovers subitem then do nothing.
		if (m_stCurrCell.iItem == hitInfo.iItem && m_stCurrCell.iSubItem == hitInfo.iSubItem)
			return;
		else
		{	//If it left.
			m_fTtShown = false;
			::SendMessage(m_hwndTt, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
			KillTimer(ID_TIMER_TOOLTIP);
			m_stCurrCell.iItem = hitInfo.iItem;
			m_stCurrCell.iSubItem = hitInfo.iSubItem;
		}
	}

	CMFCListCtrl::OnTimer(nIDEvent);
}

BOOL CListEx::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
//	SetFocus();
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
	//To avoid flickering.
	//Drawing to CMemDC, excluding list header area (rc).
	CRect rc, rcHdr;
	GetClientRect(&rc);
	GetHeaderCtrl().GetClientRect(rcHdr);
	rc.top += rcHdr.Height();
	
	CPaintDC dc(this);
	CMemDC memDC(dc, rc);
	CDC& rDC = memDC.GetDC();
	rDC.GetClipBox(&rc);
	rDC.FillSolidRect(rc, m_clrBkNWA);

	DefWindowProcW(WM_PAINT, (WPARAM)rDC.m_hDC, (LPARAM)0);
}

void CListEx::OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	//*pResult = 0;
}

void CListEx::OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	//*pResult = 0;
}

void CListEx::OnHdnTrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	//*pResult = 0;
}

void CListEx::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CMFCListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CListEx::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	GetHeaderCtrl().RedrawWindow();

	CMFCListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

bool CListEx::HasTooltip(int nItem, int nSubitem, std::wstring** ppwstrText, std::wstring** ppwstrCaption)
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
			if (ppwstrText)
			{
				*ppwstrText = &std::get<0>(itInner->second);
				if (ppwstrCaption)
					*ppwstrCaption = &std::get<1>(itInner->second);
			}
			return true;
		}
	}

	return false;
}