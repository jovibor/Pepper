/****************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/						    *
* This software is available under the "MIT License modified with The Commons Clause".  *
* https://github.com/jovibor/Pepper/blob/master/LICENSE                                 *
*                                                                                       *
* This is a SEARCH_HEX control for MFC apps, implemented as CWnd derived class.			    *
* The usage is quite simple:														    *
* 1. Construct CHexCtrl object — HEXControl::CHexCtrl myHex;						    *
* 2. Call myHex.Create member function to create an instance.   					    *
* 3. Call myHex.SetData method to set the data and its size to display as hex.	        *
****************************************************************************************/
#include "stdafx.h"
#include "HexCtrl.h"
#include "strsafe.h"

using namespace HEXControl;

/************************************************************************
* CHexCtrl implementation.												*
************************************************************************/
IMPLEMENT_DYNAMIC(CHexCtrl, CWnd)

BEGIN_MESSAGE_MAP(CHexCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	ON_WM_DESTROY()
	ON_WM_ACTIVATEAPP()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL CHexCtrl::Create(CWnd* pwndParent, UINT uiCtrlId, const CRect* pRect, bool fFloat, const LOGFONT* pLogFont)
{
	if (m_fCreated) //Already created.
		return FALSE;

	m_pLogFontHexView = pLogFont;
	m_dwCtrlId = uiCtrlId;
	m_pwndParentOwner = pwndParent;
	m_fFloat = fFloat;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE;
	if (fFloat) {
		dwStyle &= ~WS_CHILD;
		dwStyle |= WS_OVERLAPPEDWINDOW;
	}

	CRect rc;
	if (pRect)
		rc = *pRect;
	else
	{
		//If input rect==nullptr then place window at screen center.
		int iPosX = GetSystemMetrics(SM_CXSCREEN) / 4;
		int iPosY = GetSystemMetrics(SM_CYSCREEN) / 4;
		int iPosCX = iPosX * 3;
		int iPosCY = iPosY * 3;
		rc.SetRect(iPosX, iPosY, iPosCX, iPosCY);
	}

	if (!CWnd::CreateEx(0, 0, L"HexControl", dwStyle, rc, pwndParent, fFloat ? 0 : uiCtrlId))
		return FALSE;

	CRuntimeClass* pNewViewClass = RUNTIME_CLASS(CHexView);
	CCreateContext context;
	context.m_pNewViewClass = pNewViewClass;
	if (!(m_pHexView = (CHexView*)pNewViewClass->CreateObject()))
		return FALSE;

	GetClientRect(rc);
	if (!m_pHexView->Create(this, rc, 0xFF, &context, m_pLogFontHexView))
		return FALSE;

	m_fCreated = true;

	return TRUE;
}

void CHexCtrl::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	if (m_fCreated)
		GetActiveView()->SetFocus();
}

void CHexCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (m_fCreated)
		GetActiveView()->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CHexCtrl::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CHexCtrl::OnDestroy()
{
	if (m_pwndParentOwner)
	{
		NMHDR nmh { m_hWnd, (UINT)GetDlgCtrlID(), HEXCTRL_MSG_DESTROY };
		m_pwndParentOwner->SendMessageW(WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
		m_pwndParentOwner->SetForegroundWindow();
	}
	m_fCreated = false;
	m_pHexView = nullptr;

	CWnd::OnDestroy();
}

void CHexCtrl::SetData(const PBYTE pData, DWORD_PTR dwCount) const
{
	if (m_fCreated)
		GetActiveView()->SetData(pData, dwCount);
}

void CHexCtrl::ClearData()
{
	if (m_fCreated)
		GetActiveView()->ClearData();
}

void CHexCtrl::SetSelection(DWORD_PTR dwOffset, DWORD dwBytes)
{
	if (m_fCreated)
		GetActiveView()->SetSelection(dwOffset, dwBytes);
}

void CHexCtrl::SetFont(const LOGFONT* pLogFontNew) const
{
	if (m_fCreated)
		return GetActiveView()->SetFont(pLogFontNew);
}

void CHexCtrl::SetFontSize(UINT nSize) const
{
	if (m_fCreated)
		return GetActiveView()->SetFontSize(nSize);
}

void CHexCtrl::SetColor(COLORREF clrTextHex, COLORREF clrTextAscii, COLORREF clrTextCaption,
	COLORREF clrBk, COLORREF clrBkSelected) const
{
	if (m_fCreated)
		GetActiveView()->SetColor(clrTextHex, clrTextAscii, clrTextCaption, clrBk, clrBkSelected);
}

int CHexCtrl::GetDlgCtrlID() const
{
	return m_dwCtrlId;
}

CWnd * CHexCtrl::GetParent() const
{
	return m_pwndParentOwner;
}


/************************************************************************
* CHexView implementation.												*
************************************************************************/

/********************************************************************
* Below is the custom implementation of MFC IMPLEMENT_DYNCREATE()	*
* macro, which doesn't work with nested classes by default.			*
********************************************************************/
CObject* PASCAL CHexView::CreateObject()
{
	return new CHexView;
}

CRuntimeClass* PASCAL CHexView::_GetBaseClass()
{
	return RUNTIME_CLASS(CScrollView);
}

AFX_COMDAT const CRuntimeClass CHexView::classCHexView {
	"CHexView", sizeof(class CHexView), 0xFFFF, CHexView::CreateObject,
	&CHexView::_GetBaseClass, NULL, NULL };

CRuntimeClass* PASCAL CHexView::GetThisClass()
{
	return (CRuntimeClass*)(&CHexView::classCHexView);
}

CRuntimeClass* CHexView::GetRuntimeClass() const
{
	return (CRuntimeClass*)(&CHexView::classCHexView);
}

BEGIN_MESSAGE_MAP(CHexView, CScrollView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

BOOL CHexView::Create(CHexCtrl* pwndParent, const RECT& rc, UINT uiId, CCreateContext* pContext, const LOGFONT* pLogFont)
{
	m_pwndParent = pwndParent;
	m_pwndGrParent = pwndParent->GetParent();

	NONCLIENTMETRICSW ncm { sizeof(NONCLIENTMETRICSW) };
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
	ncm.lfMessageFont.lfHeight = 18; //For some reason above func returns this value as MAX_LONG.

	LOGFONT lf { };
	StringCchCopyW(lf.lfFaceName, 9, L"Consolas");
	lf.lfHeight = 18;

	//In case of inability to create font from LOGFONT*
	//creating default windows font.
	if (pLogFont)
	{
		if (!m_fontHexView.CreateFontIndirectW(pLogFont))
			if (!m_fontHexView.CreateFontIndirectW(&lf))
				m_fontHexView.CreateFontIndirectW(&ncm.lfMessageFont);
	}
	else
		if (!m_fontHexView.CreateFontIndirectW(&lf))
			m_fontHexView.CreateFontIndirectW(&ncm.lfMessageFont);

	lf.lfHeight = 16;
	if (!m_fontBottomRect.CreateFontIndirectW(&lf))
	{
		ncm.lfMessageFont.lfHeight = 16;
		m_fontBottomRect.CreateFontIndirectW(&ncm.lfMessageFont);
	}

	m_menuPopup.CreatePopupMenu();
	m_menuPopup.AppendMenuW(MF_STRING, IDM_POPUP_SEARCH, L"Search...	Ctrl+F");
	m_menuPopup.AppendMenuW(MF_SEPARATOR);
	m_menuPopup.AppendMenuW(MF_STRING, IDM_POPUP_COPYASHEX, L"Copy as Hex...	Ctrl+C");
	m_menuPopup.AppendMenuW(MF_STRING, IDM_POPUP_COPYASHEXFORMATTED, L"Copy as Formatted Hex...");
	m_menuPopup.AppendMenuW(MF_STRING, IDM_POPUP_COPYASASCII, L"Copy as Ascii...");
	m_menuPopup.AppendMenuW(MF_SEPARATOR);
	m_menuPopup.AppendMenuW(MF_STRING, IDM_POPUP_ABOUT, L"About");

	if (!CScrollView::Create(nullptr, nullptr, WS_CHILD | WS_VISIBLE, rc, pwndParent, uiId, pContext))
		return FALSE;

	m_dlgSearch.Create(IDD_HEXCTRL_DIALOG_SEARCH, this);
	Recalc();

	return TRUE;
}

void CHexView::SetData(const unsigned char* pData, DWORD_PTR dwCount)
{
	ClearData();

	m_pData = pData;
	m_dwRawDataCount = dwCount;

	UpdateInfoText();
	Recalc();
}

void CHexView::ClearData()
{
	m_dwRawDataCount = 0;
	m_pData = nullptr;
	m_dwSelectionClick = m_dwSelectionStart = m_dwSelectionEnd = m_dwBytesSelected = 0;
	SetScrollSizes(MM_TEXT, CSize(0, 0));
	m_dlgSearch.ClearAll();
	UpdateInfoText();
}

void CHexView::SetSelection(DWORD_PTR dwOffset, DWORD dwBytes)
{
	SetSelection(dwOffset, dwOffset, dwBytes, true);
}

void CHexView::SetFont(const LOGFONT* pLogFontNew)
{
	if (!pLogFontNew)
		return;

	m_fontHexView.DeleteObject();
	m_fontHexView.CreateFontIndirectW(pLogFontNew);

	Recalc();
}

void CHexView::SetFontSize(UINT uiSize)
{
	//Prevent font size from being too small or too big.
	if (uiSize < 9 || uiSize > 75)
		return;

	LOGFONT lf;
	m_fontHexView.GetLogFont(&lf);
	lf.lfHeight = uiSize;
	m_fontHexView.DeleteObject();
	m_fontHexView.CreateFontIndirectW(&lf);

	Recalc();
}

UINT CHexView::GetFontSize()
{
	LOGFONT lf;
	m_fontHexView.GetLogFont(&lf);

	return lf.lfHeight;
}

void CHexView::SetColor(COLORREF clrTextHex, COLORREF clrTextAscii, COLORREF clrTextCaption,
	COLORREF clrBk, COLORREF clrBkSelected)
{
	m_clrTextHex = clrTextHex;
	m_clrTextAscii = clrTextAscii;
	m_clrTextCaption = clrTextCaption;
	m_clrBk = clrBk;
	m_clrBkSelected = clrBkSelected;

	RedrawWindow();
}

void CHexView::SetCapacity(DWORD dwCapacity)
{
	if (dwCapacity < 1 || dwCapacity > 64)
		return;

	m_dwGridCapacity = dwCapacity;
	m_dwGridBlockSize = m_dwGridCapacity / 2;
	Recalc();
}

void CHexView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_fLMousePressed)
	{
		//If LMouse is pressed but cursor is outside client area.
		//SetCapture() behaviour.

		//Checking for scrollbars existence first.
		BOOL fHorz, fVert;
		CheckScrollBars(fHorz, fVert);
		if (fHorz)
		{
			if (point.x < m_rcClient.left)
			{
				SetScrollPos(SB_HORZ, GetScrollPos(SB_HORZ) - m_sizeLetter.cx);
				point.x = m_iIndentFirstHexChunk;
			}
			else if (point.x >= m_rcClient.right)
			{
				SetScrollPos(SB_HORZ, GetScrollPos(SB_HORZ) + m_sizeLetter.cx);
				point.x = m_iFourthVertLine - 1;
			}
		}
		if (fVert)
		{
			if (point.y < m_iHeightTopRect)
			{
				SetScrollPos(SB_VERT, GetScrollPos(SB_VERT) - m_sizeLetter.cy);
				point.y = m_iHeightTopRect;
			}
			else if (point.y >= m_iHeightWorkArea)
			{
				SetScrollPos(SB_VERT, GetScrollPos(SB_VERT) + m_sizeLetter.cy);
				point.y = m_iHeightWorkArea - 1;
			}
		}
		const int iHit = HitTest(&point);
		if (iHit != -1)
		{
			if (iHit <= (int)m_dwSelectionClick)
			{
				m_dwSelectionStart = iHit;
				m_dwSelectionEnd = m_dwSelectionClick;
			}
			else
			{
				m_dwSelectionStart = m_dwSelectionClick;
				m_dwSelectionEnd = iHit;
			}

			m_dwBytesSelected = m_dwSelectionEnd - m_dwSelectionStart + 1;

			UpdateInfoText();
		}

		RedrawWindow();
	}
}

BOOL CHexView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (nFlags == MK_CONTROL)
	{
		SetFontSize(GetFontSize() + zDelta / WHEEL_DELTA * 2);
		return TRUE;
	}
	else if (nFlags & (MK_CONTROL | MK_SHIFT))
	{
		SetCapacity(m_dwGridCapacity + zDelta / WHEEL_DELTA);
		return TRUE;
	}
	Invalidate();

	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CHexView::OnLButtonDown(UINT nFlags, CPoint point)
{
	const int iHit = HitTest(&point);
	if (iHit != -1)
	{
		SetCapture();
		if (m_dwBytesSelected && (nFlags & MK_SHIFT))
		{
			if (iHit <= (int)m_dwSelectionClick)
			{
				m_dwSelectionStart = iHit;
				m_dwSelectionEnd = m_dwSelectionClick;
			}
			else
			{
				m_dwSelectionStart = m_dwSelectionClick;
				m_dwSelectionEnd = iHit;
			}

			m_dwBytesSelected = m_dwSelectionEnd - m_dwSelectionStart + 1;
		}
		else
		{
			m_dwSelectionClick = m_dwSelectionStart = m_dwSelectionEnd = iHit;
			m_dwBytesSelected = 1;
		}

		m_fLMousePressed = true;
		UpdateInfoText();
	}
}

void CHexView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_fLMousePressed = false;
	ReleaseCapture();

	CScrollView::OnLButtonUp(nFlags, point);
}

void CHexView::OnMButtonDown(UINT nFlags, CPoint point)
{
}

BOOL CHexView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT uiId = LOWORD(wParam);

	switch (uiId)
	{
	case IDM_POPUP_SEARCH:
		m_dlgSearch.ShowWindow(SW_SHOW);
		break;
	case IDM_POPUP_COPYASHEX:
		CopyToClipboard(CLIPBOARD_COPY_AS_HEX);
		break;
	case IDM_POPUP_COPYASHEXFORMATTED:
		CopyToClipboard(CLIPBOARD_COPY_AS_HEX_FORMATTED);
		break;
	case IDM_POPUP_COPYASASCII:
		CopyToClipboard(CLIPBOARD_COPY_AS_ASCII);
		break;
	case IDM_POPUP_ABOUT:
		m_dlgAbout.DoModal();
		break;
	}

	return CScrollView::OnCommand(wParam, lParam);
}

void CHexView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	m_menuPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
}

void CHexView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case 'F':
		if (GetKeyState(VK_CONTROL) < 0)
			m_dlgSearch.ShowWindow(SW_SHOW);
		break;
	case 'C':
		if (GetKeyState(VK_CONTROL) < 0)
			CopyToClipboard(CLIPBOARD_COPY_AS_HEX);
		break;
	case VK_RIGHT:
		if (m_dwBytesSelected && (GetAsyncKeyState(VK_SHIFT) < 0))
		{
			if (m_dwSelectionStart == m_dwSelectionClick)
				SetSelection(m_dwSelectionClick, m_dwSelectionClick, m_dwBytesSelected + 1);
			else
				SetSelection(m_dwSelectionClick, m_dwSelectionStart + 1, m_dwBytesSelected - 1);
		}
		else
			SetSelection(m_dwSelectionClick + 1, m_dwSelectionClick + 1, 1);
		break;
	case VK_LEFT:
		if (m_dwBytesSelected && (GetAsyncKeyState(VK_SHIFT) < 0))
		{
			if (m_dwSelectionStart == m_dwSelectionClick && m_dwBytesSelected > 1)
				SetSelection(m_dwSelectionClick, m_dwSelectionClick, m_dwBytesSelected - 1);
			else
				SetSelection(m_dwSelectionClick, m_dwSelectionStart - 1, m_dwBytesSelected + 1);
		}
		else
			SetSelection(m_dwSelectionClick - 1, m_dwSelectionClick - 1, 1);
		break;
	case VK_DOWN:
		if (m_dwBytesSelected && (GetAsyncKeyState(VK_SHIFT) < 0))
		{
			if (m_dwSelectionStart == m_dwSelectionClick)
				SetSelection(m_dwSelectionClick, m_dwSelectionClick, m_dwBytesSelected + m_dwGridCapacity);
			else if (m_dwSelectionStart < m_dwSelectionClick)
			{
				DWORD dwStartAt = m_dwBytesSelected > m_dwGridCapacity ? m_dwSelectionStart + m_dwGridCapacity : m_dwSelectionClick;
				DWORD dwBytes = m_dwBytesSelected >= m_dwGridCapacity ? m_dwBytesSelected - m_dwGridCapacity : m_dwGridCapacity;
				SetSelection(m_dwSelectionClick, dwStartAt, dwBytes ? dwBytes : 1);
			}
		}
		else
			SetSelection(m_dwSelectionClick + m_dwGridCapacity, m_dwSelectionClick + m_dwGridCapacity, 1);
		break;
	case VK_UP:
		if (m_dwBytesSelected && (GetAsyncKeyState(VK_SHIFT) < 0))
		{
			if (m_dwSelectionStart == 0)
				return;

			if (m_dwSelectionStart < m_dwSelectionClick)
			{
				DWORD dwStartAt;
				DWORD dwBytes;
				if (m_dwSelectionStart < m_dwGridCapacity)
				{
					dwStartAt = 0;
					dwBytes = m_dwBytesSelected + m_dwSelectionStart;
				}
				else
				{
					dwStartAt = m_dwSelectionStart - m_dwGridCapacity;
					dwBytes = m_dwBytesSelected + m_dwGridCapacity;
				}
				SetSelection(m_dwSelectionClick, dwStartAt, dwBytes);
			}
			else
			{
				DWORD dwStartAt = m_dwBytesSelected >= m_dwGridCapacity ? m_dwSelectionClick : m_dwSelectionClick - m_dwGridCapacity + 1;
				DWORD dwBytes = m_dwBytesSelected >= m_dwGridCapacity ? m_dwBytesSelected - m_dwGridCapacity : m_dwGridCapacity;
				SetSelection(m_dwSelectionClick, dwStartAt, dwBytes ? dwBytes : 1);
			}
		}
		else
			SetSelection(m_dwSelectionClick - m_dwGridCapacity, m_dwSelectionClick - m_dwGridCapacity, 1);
		break;
	case VK_PRIOR: //Page-Up
		SetScrollPos(SB_VERT, GetScrollPos(SB_VERT) - m_sizeLetter.cy * 16);
		break;
	case VK_NEXT:  //Page-Down
		SetScrollPos(SB_VERT, GetScrollPos(SB_VERT) + m_sizeLetter.cy * 16);
		break;
	case VK_HOME:
	{
		SCROLLINFO si { sizeof(SCROLLINFO), SIF_ALL };
		GetScrollInfo(SB_VERT, &si, SIF_ALL);
		SetScrollPos(SB_VERT, si.nMin);
	}
	break;
	case VK_END:
	{
		SCROLLINFO si { sizeof(SCROLLINFO), SIF_ALL };
		GetScrollInfo(SB_VERT, &si, SIF_ALL);
		SetScrollPos(SB_VERT, si.nMax);
	}
	break;
	}
	RedrawWindow();

	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CHexView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si { sizeof(SCROLLINFO), SIF_ALL };
	GetScrollInfo(SB_VERT, &si, SIF_ALL);

	int iPos = si.nPos;
	switch (nSBCode)
	{
	case SB_TOP: iPos = si.nMin; break;
	case SB_BOTTOM: iPos = si.nMax; break;
	case SB_LINEUP: iPos -= m_sizeLetter.cy; break;
	case SB_LINEDOWN: iPos += m_sizeLetter.cy;  break;
	case SB_PAGEUP: iPos -= m_sizeLetter.cy * 16; break;
	case SB_PAGEDOWN: iPos += m_sizeLetter.cy * 16; break;
	case SB_THUMBPOSITION: iPos = si.nTrackPos; break;
	case SB_THUMBTRACK: iPos = si.nTrackPos; break;
	}

	//Make sure the new position is within range.
	if (iPos < si.nMin)
		iPos = si.nMin;
	int max = si.nMax - si.nPage + 1;
	if (iPos > max)
		iPos = max;

	si.nPos = iPos;
	SetScrollInfo(SB_VERT, &si);
	RedrawWindow();

	NMLVSCROLL nmlv { { m_pwndParent->m_hWnd, (UINT)m_pwndParent->GetDlgCtrlID(), HEXCTRL_MSG_SCROLLING }, -1, iPos };
	if (m_pwndGrParent)
		m_pwndGrParent->SendMessageW(WM_NOTIFY, nmlv.hdr.idFrom, (LPARAM)&nmlv);
}

void CHexView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si { sizeof(SCROLLINFO), SIF_ALL };
	GetScrollInfo(SB_HORZ, &si, SIF_ALL);

	int iPos = si.nPos;
	switch (nSBCode)
	{
	case SB_LEFT: iPos = si.nMin; break;
	case SB_RIGHT: iPos = si.nMax; break;
	case SB_LINELEFT: iPos -= m_sizeLetter.cx; break;
	case SB_LINERIGHT: iPos += m_sizeLetter.cx;  break;
	case SB_PAGELEFT: iPos -= si.nPage; break;
	case SB_PAGERIGHT: iPos += si.nPage; break;
	case SB_THUMBPOSITION: iPos = si.nTrackPos; break;
	case SB_THUMBTRACK: iPos = si.nTrackPos; break;
	}

	//Make sure the new position is within range.
	if (iPos < si.nMin)
		iPos = si.nMin;
	int max = si.nMax - si.nPage + 1;
	if (iPos > max)
		iPos = max;

	si.nPos = iPos;
	SetScrollInfo(SB_HORZ, &si);
	RedrawWindow();

	NMLVSCROLL nmlv { { m_pwndParent->m_hWnd, (UINT)m_pwndParent->GetDlgCtrlID(), HEXCTRL_MSG_SCROLLING }, iPos, -1 };
	if (m_pwndGrParent)
		m_pwndGrParent->SendMessageW(WM_NOTIFY, nmlv.hdr.idFrom, (LPARAM)&nmlv);
}

void CHexView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);
	Recalc();
}

void CHexView::OnDraw(CDC* pDC)
{
	int iScrollV = GetScrollPos(SB_VERT);
	int iScrollH = GetScrollPos(SB_HORZ);
	RECT rc; //Used for all local rc related drawing.

	rc = m_rcClient;
	rc.top += iScrollV;
	rc.bottom += iScrollV;
	rc.left += iScrollH;
	rc.right += iScrollH;

	//Drawing through CMemDC to avoid any flickering.
	CMemDC memDC(*pDC, rc);
	CDC& rDC = memDC.GetDC();

	rDC.GetClipBox(&rc);
	rDC.FillSolidRect(&rc, m_clrBk);
	rDC.SelectObject(&m_penLines);
	rDC.SelectObject(&m_fontHexView);

	//Find the iLineStart and iLineEnd position, draw the visible portion.
	const UINT iLineStart = iScrollV / m_sizeLetter.cy;
	UINT iLineEnd = m_dwRawDataCount ?
		(iLineStart + (m_rcClient.Height() - m_iHeightTopRect - m_iHeightBottomOffArea) / m_sizeLetter.cy) : 0;
	//If m_dwRawDataCount is really small we adjust iLineEnd to not be bigger than maximum allowed.
	if (iLineEnd > (m_dwRawDataCount / m_dwGridCapacity))
		iLineEnd = m_dwRawDataCount % m_dwGridCapacity ? m_dwRawDataCount / m_dwGridCapacity + 1 : m_dwRawDataCount / m_dwGridCapacity;

	//Horizontal lines depending on scroll.
	const int iFirstHorizLine = iScrollV;
	const int iSecondHorizLine = iFirstHorizLine + m_iHeightTopRect - 1;
	const int iThirdHorizLine = iFirstHorizLine + m_rcClient.Height() - m_iHeightBottomOffArea;
	const int iFourthHorizLine = iThirdHorizLine + m_iHeightBottomRect;

	//First horizontal line.
	rDC.MoveTo(0, iFirstHorizLine);
	rDC.LineTo(m_iFourthVertLine, iFirstHorizLine);

	//Second horizontal line.
	rDC.MoveTo(0, iSecondHorizLine);
	rDC.LineTo(m_iFourthVertLine, iSecondHorizLine);

	//Third horizontal line.
	rDC.MoveTo(0, iThirdHorizLine);
	rDC.LineTo(m_iFourthVertLine, iThirdHorizLine);

	//Fourth horizontal line.
	rDC.MoveTo(0, iFourthHorizLine);
	rDC.LineTo(m_iFourthVertLine, iFourthHorizLine);

	//«Offset» text.
	rc.left = m_iFirstVertLine; rc.top = iFirstHorizLine;
	rc.right = m_iSecondVertLine; rc.bottom = iSecondHorizLine;
	rDC.SetTextColor(m_clrTextCaption);
	DrawTextW(rDC.m_hDC, L"Offset", 6, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	//«Bytes total:» text.
	rc.left = m_iFirstVertLine; rc.top = iThirdHorizLine + 1;
	rc.right = m_rcClient.right > m_iFourthVertLine ? m_rcClient.right : m_iFourthVertLine;
	rc.bottom = iFourthHorizLine;
	rDC.FillSolidRect(&rc, m_clrBkBottomRect);
	rDC.SetTextColor(m_clrTextBottomRect);
	rDC.SelectObject(&m_fontBottomRect);
	rc.left = m_iFirstVertLine + 5;
	DrawTextW(rDC.m_hDC, m_wstrBottomText.data(), m_wstrBottomText.size(), &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	rDC.SelectObject(&m_fontHexView);
	rDC.SetTextColor(m_clrTextCaption);
	rDC.SetBkColor(m_clrBk);

	for (unsigned iterCapacity = 0; iterCapacity < m_dwGridCapacity; iterCapacity++)
	{
		WCHAR wstrCapacity[4];
		swprintf_s(&wstrCapacity[0], 3, L"%X", iterCapacity);

		int x, c;
		if (iterCapacity < m_dwGridBlockSize) //Top capacity numbers (0 1 2 3 4 5 6 7...)
			x = m_iIndentFirstHexChunk + m_sizeLetter.cx + (m_iSpaceBetweenHexChunks*iterCapacity);
		else //Top capacity numbers, second block (8 9 A B C D E F...).
			x = m_iIndentFirstHexChunk + m_sizeLetter.cx + (m_iSpaceBetweenHexChunks*iterCapacity) + m_iSpaceBetweenBlocks;

		//If iterCapacity >= 16 two chars needed (10, 11..., 1F) to print as capacity.
		if (iterCapacity < 16)
			c = 1;
		else {
			c = 2;
			x -= m_sizeLetter.cx;
		}
		ExtTextOutW(rDC.m_hDC, x, iFirstHorizLine + m_iIndentTextCapacityY, NULL, nullptr, &wstrCapacity[0], c, nullptr);
	}

	//"Ascii" text.
	rc.left = m_iThirdVertLine; rc.top = iFirstHorizLine;
	rc.right = m_iFourthVertLine; rc.bottom = iSecondHorizLine;
	DrawTextW(rDC.m_hDC, L"Ascii", 5, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	//First Vertical line.
	rDC.MoveTo(m_iFirstVertLine, iScrollV);
	rDC.LineTo(m_iFirstVertLine, iFourthHorizLine);

	//Second Vertical line.
	rDC.MoveTo(m_iSecondVertLine, iScrollV);
	rDC.LineTo(m_iSecondVertLine, iThirdHorizLine);

	//Third Vertical line.
	rDC.MoveTo(m_iThirdVertLine, iScrollV);
	rDC.LineTo(m_iThirdVertLine, iThirdHorizLine);

	//Fourth Vertical line.
	rDC.MoveTo(m_iFourthVertLine, iScrollV);
	rDC.LineTo(m_iFourthVertLine, iFourthHorizLine);

	int iLine { };
	for (unsigned iterLines = iLineStart; iterLines < iLineEnd; iterLines++)
	{
		WCHAR wstrOffset[9];
		swprintf_s(wstrOffset, 9, L"%08X", iterLines * m_dwGridCapacity);

		//Drawing m_strOffset with bk color depending on selection range.
		if (m_dwBytesSelected && (iterLines * m_dwGridCapacity + m_dwGridCapacity) > m_dwSelectionStart &&
			(iterLines * m_dwGridCapacity) <= m_dwSelectionEnd)
			rDC.SetBkColor(m_clrBkSelected);
		else
			rDC.SetBkColor(m_clrBk);

		//Left column offset print (00000001...0000FFFF...).
		rDC.SetTextColor(m_clrTextCaption);
		ExtTextOutW(rDC.m_hDC, m_sizeLetter.cx, m_iHeightTopRect + (m_sizeLetter.cy * iLine + iScrollV),
			NULL, nullptr, wstrOffset, 8, nullptr);

		int iIndentHexX { };
		int iIndentAsciiX { };
		int iIndentBetweenBlocks { };

		//Main loop for printing Hex chunks and Ascii chars (right column).
		for (int iterChunks = 0; iterChunks < (int)m_dwGridCapacity; iterChunks++)
		{
			if (iterChunks >= (int)m_dwGridBlockSize)
				iIndentBetweenBlocks = m_iSpaceBetweenBlocks;

			const UINT iHexPosToPrintX = m_iIndentFirstHexChunk + iIndentHexX + iIndentBetweenBlocks;
			const UINT iHexPosToPrintY = m_iHeightTopRect + m_sizeLetter.cy * iLine + iScrollV;
			const UINT iAsciiPosToPrintX = m_iIndentAscii + iIndentAsciiX;
			const UINT iAsciiPosToPrintY = m_iHeightTopRect + m_sizeLetter.cy * iLine + iScrollV;

			//Index of the next char (in m_pData) to draw.
			const size_t iIndexDataToPrint = iterLines * m_dwGridCapacity + iterChunks;

			if (iIndexDataToPrint < m_dwRawDataCount) //Draw until reaching the end of m_dwRawDataCount.
			{
				//Rect of the space between Hex chunks, needed for proper selection drawing.
				rc.left = iHexPosToPrintX + m_sizeLetter.cx * 2;
				rc.top = iHexPosToPrintY;
				if (iterChunks == m_dwGridBlockSize - 1) //Space between capacity halves.
					rc.right = iHexPosToPrintX + m_sizeLetter.cx * 5;
				else
					rc.right = iHexPosToPrintX + m_sizeLetter.cx * 3;
				rc.bottom = iHexPosToPrintY + m_sizeLetter.cy;

				//Hex chunk to print.
				WCHAR wstrHexToPrint[2];
				wstrHexToPrint[0] = m_pwszHexMap[((unsigned char)m_pData[iIndexDataToPrint] & 0xF0) >> 4];
				wstrHexToPrint[1] = m_pwszHexMap[((unsigned char)m_pData[iIndexDataToPrint] & 0x0F)];

				//Selection draw with different BK color.
				if (m_dwBytesSelected && iIndexDataToPrint >= m_dwSelectionStart && iIndexDataToPrint <= m_dwSelectionEnd)
				{
					rDC.SetBkColor(m_clrBkSelected);

					//To prevent change bk color after last selected Hex in a row, and very last Hex.
					if (iIndexDataToPrint != m_dwSelectionEnd && (iIndexDataToPrint + 1) % m_dwGridCapacity)
						FillRect(rDC.m_hDC, &rc, (HBRUSH)m_stBrushBkSelected.m_hObject);
					else
						FillRect(rDC.m_hDC, &rc, (HBRUSH)m_stBrushBk.m_hObject);
				}
				else
				{
					rDC.SetBkColor(m_clrBk);
					FillRect(rDC.m_hDC, &rc, (HBRUSH)m_stBrushBk.m_hObject);
				}

				//Hex chunk printing.
				rDC.SetTextColor(m_clrTextHex);
				ExtTextOutW(rDC.m_hDC, iHexPosToPrintX, iHexPosToPrintY, 0, nullptr, &wstrHexToPrint[0], 2, nullptr);

				//Ascii to print.
				char chAsciiToPrint = m_pData[iIndexDataToPrint];
				//For non printable SEARCH_ASCII, just print a dot.
				if (chAsciiToPrint < 32 || chAsciiToPrint == 127)
					chAsciiToPrint = '.';

				//Ascii printing.
				rDC.SetTextColor(m_clrTextAscii);
				ExtTextOutA(rDC.m_hDC, iAsciiPosToPrintX, iAsciiPosToPrintY, 0, nullptr, &chAsciiToPrint, 1, nullptr);
			}
			else
			{	//Fill remaining chunks with blank spaces.
				rDC.SetBkColor(m_clrBk);
				ExtTextOutW(rDC.m_hDC, iHexPosToPrintX, iHexPosToPrintY, 0, nullptr, L" ", 2, nullptr);
				ExtTextOutA(rDC.m_hDC, iAsciiPosToPrintX, iAsciiPosToPrintY, 0, nullptr, "", 1, nullptr);
			}
			//Increasing indents for next print, for both - Hex and Ascii
			iIndentHexX += m_iSpaceBetweenHexChunks;
			iIndentAsciiX += m_iSpaceBetweenAscii;
		}
		iLine++;
	}
}

BOOL CHexView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

int CHexView::HitTest(LPPOINT pPoint)
{
	DWORD dwHexChunk;
	int iScrollV = GetScrollPos(SB_VERT);
	int iScrollH = GetScrollPos(SB_HORZ);

	//To compensate horizontal scroll.
	pPoint->x += iScrollH;

	//Checking if cursor is within SEARCH_HEX chunks area.
	if ((pPoint->x >= m_iIndentFirstHexChunk) && (pPoint->x < m_iThirdVertLine)
		&& (pPoint->y >= m_iHeightTopRect) && pPoint->y <= m_iHeightWorkArea)
	{
		int tmpBetweenBlocks;
		if (pPoint->x > m_iIndentFirstHexChunk + (m_iSpaceBetweenHexChunks * (int)m_dwGridBlockSize))
			tmpBetweenBlocks = m_iSpaceBetweenBlocks;
		else
			tmpBetweenBlocks = 0;

		//Calculate iHit SEARCH_HEX chunk, taking into account scroll position and letter sizes.
		dwHexChunk = ((pPoint->x - m_iIndentFirstHexChunk - tmpBetweenBlocks) / (m_sizeLetter.cx * 3)) +
			((pPoint->y - m_iHeightTopRect) / m_sizeLetter.cy) * m_dwGridCapacity +
			((iScrollV / m_sizeLetter.cy) * m_dwGridCapacity);
	}
	else if ((pPoint->x >= m_iIndentAscii) && (pPoint->x < (m_iIndentAscii + m_iSpaceBetweenAscii * (int)m_dwGridCapacity))
		&& (pPoint->y >= m_iHeightTopRect) && pPoint->y <= m_iHeightWorkArea)
	{
		//Calculate iHit Ascii symbol.
		dwHexChunk = ((pPoint->x - m_iIndentAscii) / (m_iSpaceBetweenAscii)) +
			((pPoint->y - m_iHeightTopRect) / m_sizeLetter.cy) * m_dwGridCapacity +
			((iScrollV / m_sizeLetter.cy) * m_dwGridCapacity);
	}
	else
		dwHexChunk = -1;

	//If cursor is out of end-bound of SEARCH_HEX chunks or Ascii chars.
	if (dwHexChunk >= m_dwRawDataCount)
		dwHexChunk = -1;

	return dwHexChunk;
}

void CHexView::CopyToClipboard(UINT nType)
{
	if (!m_dwBytesSelected)
		return;

	const char* const strHexMap = "0123456789ABCDEF";
	char chHexToCopy[2];
	std::string strToClipboard { };

	switch (nType)
	{
	case CLIPBOARD_COPY_AS_HEX:
	{
		for (unsigned i = 0; i < m_dwBytesSelected; i++)
		{
			chHexToCopy[0] = strHexMap[((unsigned char)m_pData[m_dwSelectionStart + i] & 0xF0) >> 4];
			chHexToCopy[1] = strHexMap[((unsigned char)m_pData[m_dwSelectionStart + i] & 0x0F)];
			strToClipboard += chHexToCopy[0];
			strToClipboard += chHexToCopy[1];
		}
		break;
	}
	case CLIPBOARD_COPY_AS_HEX_FORMATTED:
	{
		//How many spaces are needed to be inserted at the beginnig.
		DWORD dwModStart = m_dwSelectionStart % m_dwGridCapacity;
		//When to insert first "\r\n".
		DWORD dwTail = m_dwGridCapacity - dwModStart;
		DWORD dwNextBlock = m_dwGridCapacity % 2 ? m_dwGridBlockSize + 2 : m_dwGridBlockSize + 1;

		//If at least two rows are selected.
		if (dwModStart + m_dwBytesSelected > m_dwGridCapacity)
		{
			strToClipboard.insert(0, dwModStart * 3, ' ');
			if (dwTail <= m_dwGridBlockSize)
				strToClipboard.insert(0, 2, ' ');
		}

		for (unsigned i = 0; i < m_dwBytesSelected; i++)
		{
			chHexToCopy[0] = strHexMap[((unsigned char)m_pData[m_dwSelectionStart + i] & 0xF0) >> 4];
			chHexToCopy[1] = strHexMap[((unsigned char)m_pData[m_dwSelectionStart + i] & 0x0F)];
			strToClipboard += chHexToCopy[0];
			strToClipboard += chHexToCopy[1];

			if (i < (m_dwBytesSelected - 1) && (dwTail - 1) != 0)
				if (dwTail == dwNextBlock) //Space between blocks.
					strToClipboard += "   ";
				else
					strToClipboard += " ";
			if (--dwTail == 0 && i < (m_dwBytesSelected - 1)) //Next string.
			{
				strToClipboard += "\r\n";
				dwTail = m_dwGridCapacity;
			}
		}
		break;
	}
	case CLIPBOARD_COPY_AS_ASCII:
	{
		char ch;
		for (unsigned i = 0; i < m_dwBytesSelected; i++)
		{
			ch = m_pData[m_dwSelectionStart + i];
			//If next byte is zero —> substitute it with space.
			if (ch == 0)
				ch = ' ';
			strToClipboard += ch;
		}
		break;
	}
	}

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strToClipboard.length() + 1);
	if (!hMem)
		return;
	LPVOID hMemLock = GlobalLock(hMem);
	if (!hMemLock)
		return;

	memcpy(hMemLock, strToClipboard.data(), strToClipboard.length() + 1);
	GlobalUnlock(hMem);
	OpenClipboard();
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
}

void CHexView::UpdateInfoText()
{
	if (!m_dwRawDataCount)
		m_wstrBottomText.clear();
	else
	{
		m_wstrBottomText.resize(128);
		if (m_dwBytesSelected)
			m_wstrBottomText.resize(swprintf_s(m_wstrBottomText.data(), 128, L"Bytes selected: 0x%X(%u); Offset: 0x%X(%u) - 0x%X(%u)",
				m_dwBytesSelected, m_dwBytesSelected, m_dwSelectionStart, m_dwSelectionStart, m_dwSelectionEnd, m_dwSelectionEnd));
		else
			m_wstrBottomText.resize(swprintf_s(m_wstrBottomText.data(), 128, L"Bytes total: 0x%X(%u)", m_dwRawDataCount, m_dwRawDataCount));
	}
	RedrawWindow();
}

void CHexView::Recalc()
{
	GetClientRect(&m_rcClient);

	UINT iStartLine { };
	if (m_fSecondLaunch)
		iStartLine = GetScrollPos(SB_VERT) / m_sizeLetter.cy;

	HDC hDC = ::GetDC(m_hWnd);
	SelectObject(hDC, m_fontHexView.m_hObject);
	TEXTMETRICW tm { };
	GetTextMetricsW(hDC, &tm);
	m_sizeLetter.cx = tm.tmAveCharWidth;
	m_sizeLetter.cy = tm.tmHeight + tm.tmExternalLeading;
	::ReleaseDC(m_hWnd, hDC);

	m_iFirstVertLine = 0;
	m_iSecondVertLine = m_sizeLetter.cx * 10;
	m_iSpaceBetweenHexChunks = m_sizeLetter.cx * 3;
	m_iSpaceBetweenBlocks = m_sizeLetter.cx * 2;
	m_iThirdVertLine = m_iSecondVertLine + (m_iSpaceBetweenHexChunks * m_dwGridCapacity) + m_iSpaceBetweenBlocks + m_sizeLetter.cx;
	m_iIndentAscii = m_iThirdVertLine + m_sizeLetter.cx;
	m_iSpaceBetweenAscii = m_sizeLetter.cx + 1;
	m_iFourthVertLine = m_iIndentAscii + (m_iSpaceBetweenAscii * m_dwGridCapacity) + m_sizeLetter.cx;
	m_iIndentFirstHexChunk = m_iSecondVertLine + m_sizeLetter.cx;
	m_iHeightTopRect = int(m_sizeLetter.cy * 1.5);
	m_iHeightWorkArea = m_rcClient.Height() - m_iHeightBottomOffArea -
		((m_rcClient.Height() - m_iHeightTopRect - m_iHeightBottomOffArea) % m_sizeLetter.cy);
	m_iIndentTextCapacityY = (m_iHeightTopRect / 2) - (m_sizeLetter.cy / 2);

	//Scroll sizes according to current font size.
	SetScrollSizes(MM_TEXT, CSize(m_iFourthVertLine + 1,
		m_iHeightTopRect + m_iHeightBottomOffArea + (m_sizeLetter.cy * (m_dwRawDataCount / m_dwGridCapacity + 3))));

	//This m_fSecondLaunch shows that Recalc() was invoked at least once before,
	//and ScrollSizes have already been set, so we can adjust them.
	if (m_fSecondLaunch)
		SetScrollPos(SB_VERT, m_sizeLetter.cy * iStartLine);
	else
		m_fSecondLaunch = true;

	RedrawWindow();
}

void CHexView::Search(HEXSEARCH& rSearch)
{
	rSearch.fFound = false;
	DWORD dwStartAt = rSearch.dwStartAt;
	DWORD dwSizeBytes;
	DWORD dwUntil;
	std::string strSearch { };

	if (rSearch.wstrSearch.empty() || m_dwRawDataCount == 0 || rSearch.dwStartAt > (m_dwRawDataCount - 1))
		return m_dlgSearch.SearchCallback();

	int iSizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &rSearch.wstrSearch[0], (int)rSearch.wstrSearch.size(), nullptr, 0, nullptr, nullptr);
	std::string strSearchAscii(iSizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, &rSearch.wstrSearch[0], (int)rSearch.wstrSearch.size(), &strSearchAscii[0], iSizeNeeded, nullptr, nullptr);

	switch (rSearch.dwSearchType)
	{
	case SEARCH_HEX:
	{
		DWORD dwIterations = strSearchAscii.size() % 2 ? strSearchAscii.size() / 2 + 1 : strSearchAscii.size() / 2;
		int iNextChar = 0;
		std::string strToUL;
		char* pEndPtr { };

		for (DWORD i = 0; i < dwIterations; i++)
		{
			if (strSearchAscii.size() >= i + 2)
				strToUL = strSearchAscii.substr(iNextChar, 2);
			else
				strToUL = strSearchAscii.substr(iNextChar, 1);

			unsigned long ulNumber = strtoul(strToUL.data(), &pEndPtr, 16);
			if (ulNumber == 0 && (pEndPtr == strToUL.data() || *pEndPtr != '\0'))
			{
				rSearch.fFound = false;
				m_dlgSearch.SearchCallback();
				return;
			}

			strSearch += (unsigned char)ulNumber;
			iNextChar += 2;
		}

		dwSizeBytes = strSearch.size();
		if (dwSizeBytes > m_dwRawDataCount)
			goto End;

		break;
	}
	case SEARCH_ASCII:
	{
		dwSizeBytes = strSearchAscii.size();
		if (dwSizeBytes > m_dwRawDataCount)
			goto End;

		strSearch = std::move(strSearchAscii);
		break;
	}
	case SEARCH_UNICODE:
	{
		dwSizeBytes = rSearch.wstrSearch.length() * sizeof(wchar_t);
		if (dwSizeBytes > m_dwRawDataCount)
			goto End;

		break;
	}
	}

	///////////////Actual Search:////////////////////////////////////////////
	switch (rSearch.dwSearchType) {
	case SEARCH_HEX:
	case SEARCH_ASCII:
	{
		if (rSearch.iDirection == SEARCH_FORWARD)
		{
			dwUntil = m_dwRawDataCount - strSearch.size();
			dwStartAt = rSearch.fSecondMatch ? rSearch.dwStartAt + 1 : 0;

			for (DWORD i = dwStartAt; i <= dwUntil; i++)
			{
				if (memcmp(m_pData + i, strSearch.data(), strSearch.size()) == 0)
				{
					rSearch.fFound = true;
					rSearch.dwStartAt = i;
					rSearch.fWrap = false;
					goto End;
				}
			}

			dwStartAt = 0;
			for (DWORD i = dwStartAt; i <= dwUntil; i++)
			{
				if (memcmp(m_pData + i, strSearch.data(), strSearch.size()) == 0)
				{
					rSearch.fFound = true;
					rSearch.dwStartAt = i;
					rSearch.fWrap = true;
					rSearch.iWrap = SEARCH_END;
					rSearch.fCount = true;
					goto End;
				}
			}
		}
		if (rSearch.iDirection == SEARCH_BACKWARD)
		{
			if (rSearch.fSecondMatch && dwStartAt > 0)
			{
				dwStartAt--;
				for (int i = (int)dwStartAt; i >= 0; i--)
				{
					if (memcmp(m_pData + i, strSearch.data(), strSearch.size()) == 0)
					{
						rSearch.fFound = true;
						rSearch.dwStartAt = i;
						rSearch.fWrap = false;
						goto End;
					}
				}
			}

			dwStartAt = m_dwRawDataCount - strSearch.size();
			for (int i = (int)dwStartAt; i >= 0; i--)
			{
				if (memcmp(m_pData + i, strSearch.data(), strSearch.size()) == 0)
				{
					rSearch.fFound = true;
					rSearch.dwStartAt = i;
					rSearch.fWrap = true;
					rSearch.iWrap = SEARCH_BEGINNING;
					rSearch.fCount = false;
					goto End;
				}
			}
		}
		break;
	}
	case SEARCH_UNICODE:
	{
		if (rSearch.iDirection == SEARCH_FORWARD)
		{
			dwUntil = m_dwRawDataCount - dwSizeBytes;
			dwStartAt = rSearch.fSecondMatch ? rSearch.dwStartAt + 1 : 0;

			for (DWORD i = dwStartAt; i <= dwUntil; i++)
			{
				if (wmemcmp((const wchar_t*)(m_pData + i), rSearch.wstrSearch.data(), rSearch.wstrSearch.length()) == 0)
				{
					rSearch.fFound = true;
					rSearch.dwStartAt = i;
					rSearch.fWrap = false;
					goto End;
				}
			}

			dwStartAt = 0;
			for (DWORD i = dwStartAt; i <= dwUntil; i++)
			{
				if (wmemcmp((const wchar_t*)(m_pData + i), rSearch.wstrSearch.data(), rSearch.wstrSearch.length()) == 0)
				{
					rSearch.fFound = true;
					rSearch.dwStartAt = i;
					rSearch.iWrap = SEARCH_END;
					rSearch.fWrap = true;
					rSearch.fCount = true;
					goto End;
				}
			}
		}
		else if (rSearch.iDirection == SEARCH_BACKWARD)
		{
			if (rSearch.fSecondMatch && dwStartAt > 0)
			{
				dwStartAt--;
				for (int i = (int)dwStartAt; i >= 0; i--)
				{
					if (wmemcmp((const wchar_t*)(m_pData + i), rSearch.wstrSearch.data(), rSearch.wstrSearch.length()) == 0)
					{
						rSearch.fFound = true;
						rSearch.dwStartAt = i;
						rSearch.fWrap = false;
						goto End;
					}
				}
			}

			dwStartAt = m_dwRawDataCount - dwSizeBytes;
			for (int i = (int)dwStartAt; i >= 0; i--)
			{
				if (wmemcmp((const wchar_t*)(m_pData + i), rSearch.wstrSearch.data(), rSearch.wstrSearch.length()) == 0)
				{
					rSearch.fFound = true;
					rSearch.dwStartAt = i;
					rSearch.fWrap = true;
					rSearch.iWrap = SEARCH_BEGINNING;
					rSearch.fCount = false;
					goto End;
				}
			}
		}
		break;
	}
	}

End:
	{
		m_dlgSearch.SearchCallback();
		if (rSearch.fFound)
			SetSelection(rSearch.dwStartAt, rSearch.dwStartAt, dwSizeBytes, true);
	}
}

void CHexView::SetSelection(DWORD_PTR dwClick, DWORD_PTR dwStart, DWORD dwBytes, bool fHighlight)
{
	if (dwClick >= m_dwRawDataCount || dwStart >= m_dwRawDataCount || !dwBytes)
		return;
	if ((dwStart + dwBytes) > m_dwRawDataCount)
		dwBytes = m_dwRawDataCount - dwStart;

	//New scroll depending on selection direction: top <-> bottom.
	//fHighlight means centralize scroll position on the screen (used in Search()).
	DWORD dwEnd = dwStart + dwBytes - 1;
	int iMaxV = GetScrollPos(SB_VERT) + m_rcClient.Height() - m_iHeightBottomOffArea - m_iHeightTopRect -
		((m_rcClient.Height() - m_iHeightTopRect - m_iHeightBottomOffArea) % m_sizeLetter.cy);
	int iNewEndV = dwEnd / m_dwGridCapacity * m_sizeLetter.cy;
	int iNewStartV = dwStart / m_dwGridCapacity * m_sizeLetter.cy;

	int iNewScrollV { };
	if (fHighlight)
		iNewScrollV = iNewStartV - (m_iHeightWorkArea / 2);
	else
	{
		if (dwStart == dwClick)
		{
			if (iNewEndV >= iMaxV)
				iNewScrollV = GetScrollPos(SB_VERT) + m_sizeLetter.cy;
			else
			{
				if (iNewEndV >= GetScrollPos(SB_VERT))
					iNewScrollV = GetScrollPos(SB_VERT);
				else if (iNewStartV <= GetScrollPos(SB_VERT))
					iNewScrollV = GetScrollPos(SB_VERT) - m_sizeLetter.cy;
			}
		}
		else
		{
			if (iNewStartV < GetScrollPos(SB_VERT))
				iNewScrollV = GetScrollPos(SB_VERT) - m_sizeLetter.cy;
			else
			{
				if (iNewStartV < iMaxV)
					iNewScrollV = GetScrollPos(SB_VERT);
				else
					iNewScrollV = GetScrollPos(SB_VERT) + m_sizeLetter.cy;
			}
		}
	}
	iNewScrollV -= iNewScrollV % m_sizeLetter.cy;
	m_dwSelectionClick = dwClick;
	m_dwSelectionStart = dwStart;
	m_dwSelectionEnd = dwEnd;
	m_dwBytesSelected = dwEnd - dwStart + 1;

	BOOL fHorz, fVert;
	CheckScrollBars(fHorz, fVert);
	if (fVert)
		SetScrollPos(SB_VERT, iNewScrollV);
	if (fHorz)
		SetScrollPos(SB_HORZ, (m_dwSelectionStart % m_dwGridCapacity) * m_iSpaceBetweenHexChunks);

	UpdateInfoText();
}


/****************************************************
* CHexDlgSearch class implementation.				*
****************************************************/
BEGIN_MESSAGE_MAP(CHexDlgSearch, CDialogEx)
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_F, &CHexDlgSearch::OnButtonSearchF)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_B, &CHexDlgSearch::OnButtonSearchB)
	ON_COMMAND_RANGE(IDC_RADIO_HEX, IDC_RADIO_UNICODE, &CHexDlgSearch::OnRadioBnRange)
END_MESSAGE_MAP()

BOOL CHexDlgSearch::Create(UINT nIDTemplate, CHexView* pwndParent)
{
	m_pParent = pwndParent;

	return CDialog::Create(nIDTemplate, m_pParent);
}

CHexView* CHexDlgSearch::GetParent() const
{
	return m_pParent;
}

BOOL CHexDlgSearch::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_iRadioCurrent = IDC_RADIO_HEX;
	CheckRadioButton(IDC_RADIO_HEX, IDC_RADIO_UNICODE, m_iRadioCurrent);
	m_stBrushDefault.CreateSolidBrush(m_clrMenu);

	return TRUE;
}

void CHexDlgSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CHexDlgSearch::SearchCallback()
{
	WCHAR wstrSearch[128];

	if (m_stSearch.fFound)
	{
		if (m_stSearch.fCount)
		{
			if (!m_stSearch.fWrap)
			{
				if (m_stSearch.iDirection == CHexView::HEXCTRL_SEARCH::SEARCH_FORWARD)
					m_dwnOccurrence++;
				else if (m_stSearch.iDirection == CHexView::HEXCTRL_SEARCH::SEARCH_BACKWARD)
					m_dwnOccurrence--;
			}
			else
				m_dwnOccurrence = 1;

			swprintf_s(wstrSearch, 127, L"Found occurrence \u2116 %u from the beginning.", m_dwnOccurrence);
		}
		else
			swprintf_s(wstrSearch, 127, L"Search found occurrence.");

		m_stSearch.fSecondMatch = true;
		GetDlgItem(IDC_STATIC_BOTTOM_TEXT)->SetWindowTextW(wstrSearch);
	}
	else
	{
		ClearAll();

		if (m_stSearch.iWrap == 1)
			swprintf_s(wstrSearch, 127, L"Didn't find any occurrence. The end is reached.");
		else
			swprintf_s(wstrSearch, 127, L"Didn't find any occurrence. The begining is reached.");

		GetDlgItem(IDC_STATIC_BOTTOM_TEXT)->SetWindowTextW(wstrSearch);
	}
}

void CHexDlgSearch::OnButtonSearchF()
{
	CString strSearchText;
	GetDlgItemTextW(IDC_EDIT_SEARCH, strSearchText);
	if (strSearchText.IsEmpty())
		return;
	if (strSearchText.Compare(m_stSearch.wstrSearch.data()) != 0)
	{
		ClearAll();
		m_stSearch.wstrSearch = strSearchText;
	}

	switch (GetCheckedRadioButton(IDC_RADIO_HEX, IDC_RADIO_UNICODE))
	{
	case IDC_RADIO_HEX:
		m_stSearch.dwSearchType = CHexView::HEXCTRL_SEARCH::SEARCH_HEX;
		break;
	case IDC_RADIO_ASCII:
		m_stSearch.dwSearchType = CHexView::HEXCTRL_SEARCH::SEARCH_ASCII;
		break;
	case IDC_RADIO_UNICODE:
		m_stSearch.dwSearchType = CHexView::HEXCTRL_SEARCH::SEARCH_UNICODE;
		break;
	}
	m_stSearch.iDirection = CHexView::HEXCTRL_SEARCH::SEARCH_FORWARD;

	GetDlgItem(IDC_EDIT_SEARCH)->SetFocus();
	GetParent()->Search(m_stSearch);
}

void CHexDlgSearch::OnButtonSearchB()
{
	CString strSearchText;
	GetDlgItemTextW(IDC_EDIT_SEARCH, strSearchText);
	if (strSearchText.IsEmpty())
		return;
	if (strSearchText.Compare(m_stSearch.wstrSearch.data()) != 0)
	{
		ClearAll();
		m_stSearch.wstrSearch = strSearchText;
	}

	switch (GetCheckedRadioButton(IDC_RADIO_HEX, IDC_RADIO_UNICODE))
	{
	case IDC_RADIO_HEX:
		m_stSearch.dwSearchType = CHexView::HEXCTRL_SEARCH::SEARCH_HEX;
		break;
	case IDC_RADIO_ASCII:
		m_stSearch.dwSearchType = CHexView::HEXCTRL_SEARCH::SEARCH_ASCII;
		break;
	case IDC_RADIO_UNICODE:
		m_stSearch.dwSearchType = CHexView::HEXCTRL_SEARCH::SEARCH_UNICODE;
		break;
	}
	m_stSearch.iDirection = CHexView::HEXCTRL_SEARCH::SEARCH_BACKWARD;

	GetDlgItem(IDC_EDIT_SEARCH)->SetFocus();
	GetParent()->Search(m_stSearch);
}

void CHexDlgSearch::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (nState == WA_INACTIVE)
		SetLayeredWindowAttributes(0, 150, LWA_ALPHA);
	else
	{
		SetLayeredWindowAttributes(0, 255, LWA_ALPHA);
		GetDlgItem(IDC_EDIT_SEARCH)->SetFocus();
	}

	CDialogEx::OnActivate(nState, pWndOther, bMinimized);
}

BOOL CHexDlgSearch::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		OnButtonSearchF();
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CHexDlgSearch::OnClose()
{
	ClearAll();
	CDialogEx::OnClose();
}

HBRUSH CHexDlgSearch::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_BOTTOM_TEXT)
	{
		pDC->SetBkColor(m_clrMenu);
		pDC->SetTextColor(m_stSearch.fFound ? m_clrSearchFound : m_clrSearchFailed);
		return m_stBrushDefault;
	}

	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CHexDlgSearch::OnRadioBnRange(UINT nID)
{
	if (nID != m_iRadioCurrent)
		ClearAll();
	m_iRadioCurrent = nID;
}

void CHexDlgSearch::ClearAll()
{
	m_dwnOccurrence = 0;
	m_stSearch.dwStartAt = 0;
	m_stSearch.fSecondMatch = false;
	m_stSearch.wstrSearch = { };
	m_stSearch.fWrap = false;
	m_stSearch.fCount = true;

	GetDlgItem(IDC_STATIC_BOTTOM_TEXT)->SetWindowTextW(L"");
}



/****************************************************
* CHexDlgAbout class implementation.				*
****************************************************/

BEGIN_MESSAGE_MAP(CHexDlgAbout, CDialogEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CHexDlgAbout::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//To prevent cursor from blinking
	SetClassLongPtr(m_hWnd, GCL_HCURSOR, 0);

	m_fontDefault = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

	LOGFONT lf { };
	GetObject(m_fontDefault, sizeof(lf), &lf);
	lf.lfUnderline = TRUE;

	m_fontUnderline = CreateFontIndirect(&lf);

	m_stBrushDefault.CreateSolidBrush(m_clrMenu);

	m_curHand = LoadCursor(nullptr, IDC_HAND);
	m_curArrow = LoadCursor(nullptr, IDC_ARROW);

	return TRUE;
}

void CHexDlgAbout::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd* pWnd = ChildWindowFromPoint(point);

	if (!pWnd)
		return;

	if (m_fGithubLink == (pWnd->GetDlgCtrlID() == IDC_STATIC_HTTP_GITHUB))
	{
		m_fGithubLink = !m_fGithubLink;
		GetDlgItem(IDC_STATIC_HTTP_GITHUB)->RedrawWindow();
		SetCursor(m_fGithubLink ? m_curArrow : m_curHand);
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CHexDlgAbout::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd* pWnd = ChildWindowFromPoint(point);

	if (!pWnd)
		return;

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_HTTP_GITHUB)
		ShellExecute(nullptr, L"open", L"https://github.com/jovibor/Pepper", nullptr, nullptr, NULL);

	CDialogEx::OnLButtonDown(nFlags, point);
}

HBRUSH CHexDlgAbout::OnCtlColor(CDC * pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_HTTP_GITHUB)
	{
		pDC->SetBkColor(m_clrMenu);
		pDC->SetTextColor(RGB(0, 0, 210));
		pDC->SelectObject(m_fGithubLink ? m_fontDefault : m_fontUnderline);
		return m_stBrushDefault;
	}

	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}
