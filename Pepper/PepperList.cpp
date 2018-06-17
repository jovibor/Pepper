#include "stdafx.h"
#include "PepperListHeader.h"
#include "PepperList.h"

#define TIMER_TOOLTIP 0x1

IMPLEMENT_DYNAMIC(CPepperList, CMFCListCtrl)

BEGIN_MESSAGE_MAP(CPepperList, CMFCListCtrl)
	ON_WM_SETCURSOR()
	ON_WM_KILLFOCUS()
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, &CPepperList::OnHdnDividerdblclick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, &CPepperList::OnHdnDividerdblclick)
	ON_WM_HSCROLL()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

BOOL CPepperList::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	SetFocus();

	return CMFCListCtrl::OnSetCursor(pWnd, nHitTest, message);
}

void CPepperList::OnKillFocus(CWnd* pNewWnd)
{
}

BOOL CPepperList::OnEraseBkgnd(CDC* pDC)
{
	if (m_fEraseBkgnd) {
		CMFCListCtrl::OnEraseBkgnd(pDC);
		m_fEraseBkgnd = false;
	}

	return TRUE;
}

void CPepperList::OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	*pResult = 0;
}

CPepperList::CPepperList()
{
	LOGFONT lf { };
	lf.lfHeight = 16;
	StringCchCopyW(lf.lfFaceName, 18, L"Consolas");
	if (!m_fontList.CreateFontIndirectW(&lf))
	{
		StringCchCopyW(lf.lfFaceName, 18, L"Times New Roman");
		m_fontList.CreateFontIndirectW(&lf);
	}

	m_hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	m_stToolInfo.cbSize = TTTOOLINFOW_V1_SIZE;
	m_stToolInfo.uFlags = TTF_TRACK;
	m_stToolInfo.uId = 1;

	m_colorListToolTipSubitem = RGB(170, 170, 230);
	m_colorListSelected = RGB(0, 120, 215);

	::SendMessage(m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
	::SendMessage(m_hwndToolTip, TTM_SETTIPBKCOLOR, (WPARAM)RGB(0, 132, 132), 0);
	::SendMessage(m_hwndToolTip, TTM_SETTIPTEXTCOLOR, (WPARAM)RGB(255, 255, 255), 0);
	::SendMessage(m_hwndToolTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)400);//to allow using of newline \n
}

void CPepperList::InitHeader()
{
	m_PepperListHeader.SubclassDlgItem(0, this);
}

void CPepperList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CMFCListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);

	int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
	if (m_stScrollInfo.nPos >= max)
	{
		m_fEraseBkgnd = true;
		Invalidate();
	}
}

void CPepperList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	GetHeaderCtrl().Invalidate();
	GetHeaderCtrl().UpdateWindow();

	CMFCListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CPepperList::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);

	int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
	if (m_stScrollInfo.nPos >= max)
	{
		m_fEraseBkgnd = true;
		Invalidate();
	}

	return CMFCListCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

void CPepperList::DrawItem(LPDRAWITEMSTRUCT pDIS)
{
	if (pDIS->itemID == -1)
		return;

	CDC* pDC = CDC::FromHandle(pDIS->hDC);
	CPen* oldPen = pDC->SelectObject(&m_PenForRect);
	CFont* defFont = pDC->SelectObject(&m_fontList);
	CRect rect;

	switch (pDIS->itemAction)
	{
	case ODA_SELECT:
	case ODA_DRAWENTIRE:
		if (HasToolTip(pDIS->itemID, 0))
			pDC->FillSolidRect(&rect, m_colorListToolTipSubitem);
		else
			pDC->FillSolidRect(&pDIS->rcItem, RGB(255, 255, 255));

		GetItemRect(pDIS->itemID, &rect, LVIR_LABEL);

		if (pDIS->itemState & ODS_SELECTED)
		{
			pDIS->rcItem.left = rect.left;
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->FillSolidRect(&pDIS->rcItem, m_colorListSelected);
		}
		else
			pDC->SetTextColor(RGB(0, 0, 0));

		rect.left += 3;
		pDC->DrawText(GetItemText(pDIS->itemID, 0), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		rect.left -= 3;

		//Drawing rect lines
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
			if (HasToolTip(pDIS->itemID, i))
			{
				pDC->FillSolidRect(&rect, m_colorListToolTipSubitem);
			}
			rect.left += 3;//Drawing text +-3 px from rect bounds
			pDC->DrawText(GetItemText(pDIS->itemID, i), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			rect.left -= 3;

			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right, rect.top);
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.left, rect.bottom);
			pDC->MoveTo(rect.left, rect.bottom);
			pDC->LineTo(rect.right, rect.bottom);
			pDC->MoveTo(rect.right, rect.top);
			pDC->LineTo(rect.right, rect.bottom);
		}

		pDC->SelectObject(oldPen);
		pDC->SelectObject(defFont);
		break;

	case ODA_FOCUS:
		break;
	}
}

void CPepperList::OnRButtonDown(UINT nFlags, CPoint point)
{
	//	CMFCListCtrl::OnRButtonDown(nFlags, point);
}

void CPepperList::SetItemToolTip(int nItem, int nSubitem, const std::wstring& strTipText, const std::wstring& strTipCaption)
{
	UINT iter { };

	for (auto& i : m_vecToolTips)
	{
		if (std::get<0>(i) == nItem && std::get<1>(i) == nSubitem)
		{
			if (strTipText.empty())
				//delete subitem tooltip
				m_vecToolTips.erase(m_vecToolTips.begin() + iter);
			else
				i = { nItem, nSubitem, strTipText, strTipCaption };

			return;
		}
		iter++;
	}
	if (strTipText.empty())
		return;

	m_vecToolTips.push_back({ nItem, nSubitem, strTipText, strTipCaption });
	m_fToolTip = true;
}

void CPepperList::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_fToolTip)
	{
		LVHITTESTINFO hitInfo { };
		hitInfo.pt = point;
		ListView_SubItemHitTest(m_hWnd, &hitInfo);

		std::wstring _tipText { }, _tipCaption { };
		bool fTip = HasToolTip(hitInfo.iItem, hitInfo.iSubItem, _tipText, _tipCaption);
		if (fTip)
		{	//Check if cursor is still in cell's rect
			if (m_stCurrentSubItem.iItem == hitInfo.iItem && m_stCurrentSubItem.iSubItem == hitInfo.iSubItem)
				return;

			m_stCurrentSubItem.iItem = hitInfo.iItem;
			m_stCurrentSubItem.iSubItem = hitInfo.iSubItem;
			m_stToolInfo.lpszText = const_cast<LPWSTR>(_tipText.c_str());

			ClientToScreen(&point);
			::SendMessage(m_hwndToolTip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(point.x, point.y));
			::SendMessage(m_hwndToolTip, TTM_SETTITLE, (WPARAM)TTI_NONE, (LPARAM)_tipCaption.c_str());
			::SendMessage(m_hwndToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
			::SendMessage(m_hwndToolTip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
			SetTimer(TIMER_TOOLTIP, 200, 0);//timer to check whether mouse pointer left subitem rect.
		}
		else
		{
			m_stCurrentSubItem.iItem = hitInfo.iItem;
			m_stCurrentSubItem.iSubItem = hitInfo.iSubItem;
			::SendMessage(m_hwndToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
		}
	}
}

bool CPepperList::HasToolTip(int iItem, int iSubItem, std::wstring& strTipText, std::wstring& strTipCaption)
{
	for (auto& i : m_vecToolTips)
		if (std::get<0>(i) == iItem && std::get<1>(i) == iSubItem)
		{
			strTipText = std::get<2>(i);
			strTipCaption = std::get<3>(i);
			return true;
		}

	return false;
}

bool CPepperList::HasToolTip(int iItem, int iSubItem)
{
	for (auto& i : m_vecToolTips)
		if (std::get<0>(i) == iItem && std::get<1>(i) == iSubItem)
			return true;

	return false;
}

void CPepperList::OnTimer(UINT_PTR nIDEvent)
{	
	//Checking if mouse pointer left list subitem rect,
	//if so —> hiding tooltip and killing timer
	LVHITTESTINFO hitInfo { };
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	hitInfo.pt = point;
	ListView_SubItemHitTest(m_hWnd, &hitInfo);

	//if cursor is still hovers subitem then do nothing
	if (m_stCurrentSubItem.iItem == hitInfo.iItem && m_stCurrentSubItem.iSubItem == hitInfo.iSubItem)
		return;
	else 
	{	//if it left —>
		::SendMessage(m_hwndToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
		KillTimer(TIMER_TOOLTIP);
		m_stCurrentSubItem.iItem = hitInfo.iItem;
		m_stCurrentSubItem.iSubItem = hitInfo.iSubItem;
	}

	CMFCListCtrl::OnTimer(nIDEvent);
}