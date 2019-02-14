/****************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/						    *
* This software is available under the "MIT License modified with The Commons Clause".  *
* https://github.com/jovibor/Pepper/blob/master/LICENSE                                 *
* This is a Hex control for MFC apps, implemented as CWnd derived class.			    *
* The usage is quite simple:														    *
* 1. Construct CHexCtrl object — HEXCTRL::CHexCtrl myHex;								*
* 2. Call myHex.Create member function to create an instance.   					    *
* 3. Call myHex.SetData method to set the data and its size to display as hex.	        *
****************************************************************************************/
#include "stdafx.h"
#include "HexCtrl.h"
#include "strsafe.h"

using namespace HEXCTRL;

/************************************************************************
* CHexCtrl implementation.												*
************************************************************************/
BEGIN_MESSAGE_MAP(CHexCtrl, CWnd)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_NCACTIVATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_ACTIVATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

BOOL CHexCtrl::Create(CWnd* pwndParent, UINT uiCtrlId, const CRect* pRect, bool fFloat, const LOGFONT* pLogFont)
{
	if (m_fCreated) //Already created.
		return FALSE;

	m_dwCtrlId = uiCtrlId;
	m_fFloat = fFloat;
	m_pwndParentOwner = pwndParent;

	DWORD dwStyle;
	if (fFloat)
		dwStyle = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX;
	else
		dwStyle = WS_VISIBLE | WS_CHILD;

	CRect rc;
	if (pRect)
		rc = *pRect;
	else if (fFloat)
	{	//If pRect == nullptr and it's a float window then place it at screen center.

		int iPosX = GetSystemMetrics(SM_CXSCREEN) / 4;
		int iPosY = GetSystemMetrics(SM_CYSCREEN) / 4;
		int iPosCX = iPosX * 3;
		int iPosCY = iPosY * 3;
		rc.SetRect(iPosX, iPosY, iPosCX, iPosCY);
	}

	HCURSOR hCur;
	if (!(hCur = (HCURSOR)LoadImageW(0, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED)))
		return FALSE;
	if (!CWnd::CreateEx(0, AfxRegisterWndClass(0, hCur), L"HexControl", dwStyle, rc, pwndParent, fFloat ? 0 : uiCtrlId))
		return FALSE;

	//Removing window's border frame.
	MARGINS marg { 0, 0, 0, 1 };
	DwmExtendFrameIntoClientArea(m_hWnd, &marg);
	SetWindowPos(nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

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

	m_stScrollV.Create(this, SB_VERT, 0, 0, 0); //Actual sizes are set in Recalc().
	m_stScrollH.Create(this, SB_HORZ, 0, 0, 0);

	m_dlgSearch.Create(IDD_HEXCTRL_SEARCH, this);

	Recalc();
	m_fCreated = true;

	return TRUE;
}

void CHexCtrl::SetData(const unsigned char* pData, ULONGLONG ullSize, bool fVirtual, ULONGLONG ullGotoOffset)
{
	ClearData();

	//Virtual mode is possible only when there is a parent window
	//to which data requests will be sent.
	if (fVirtual && m_pwndParentOwner == nullptr)
		return;

	m_pData = pData;
	m_ullDataCount = ullSize;
	m_fVirtual = fVirtual;

	if (ullGotoOffset)
		SetSelection(ullGotoOffset);
	UpdateInfoText();
	Recalc();
}

void CHexCtrl::ClearData()
{
	m_ullDataCount = 0;
	m_pData = nullptr;
	m_ullSelectionClick = m_ullSelectionStart = m_ullSelectionEnd = m_ullBytesSelected = 0;

	m_stScrollV.SetScrollPos(0);
	m_stScrollV.SetScrollSizes(0, 0, 0);
	m_stScrollH.SetScrollPos(0);
	UpdateInfoText();
}

void CHexCtrl::SetSelection(ULONGLONG ullOffset, ULONGLONG ullSize)
{
	SetSelection(ullOffset, ullOffset, ullSize, true);
}

void CHexCtrl::SetFont(const LOGFONT* pLogFontNew)
{
	if (!pLogFontNew)
		return;

	m_fontHexView.DeleteObject();
	m_fontHexView.CreateFontIndirectW(pLogFontNew);

	Recalc();
}

void CHexCtrl::SetFontSize(UINT uiSize)
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

UINT CHexCtrl::GetFontSize()
{
	LOGFONT lf;
	m_fontHexView.GetLogFont(&lf);

	return lf.lfHeight;
}

void CHexCtrl::SetColor(COLORREF clrTextHex, COLORREF clrTextAscii, COLORREF clrTextCaption,
	COLORREF clrBk, COLORREF clrBkSelected)
{
	m_clrTextHex = clrTextHex;
	m_clrTextAscii = clrTextAscii;
	m_clrTextCaption = clrTextCaption;
	m_clrBk = clrBk;
	m_clrBkSelected = clrBkSelected;

	RedrawWindow();
}

void CHexCtrl::SetCapacity(DWORD dwCapacity)
{
	if (dwCapacity < 1 || dwCapacity > 64)
		return;

	m_dwGridCapacity = dwCapacity;
	m_dwGridBlockSize = m_dwGridCapacity / 2;
	Recalc();
}

int CHexCtrl::GetDlgCtrlID() const
{
	return m_dwCtrlId;
}

CWnd * CHexCtrl::GetParent() const
{
	return m_pwndParentOwner;
}

void CHexCtrl::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	SetFocus();

	CWnd::OnActivate(nState, pWndOther, bMinimized);
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
	ClearData();

	CWnd::OnDestroy();
}

void CHexCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_fLMousePressed)
	{
		//If LMouse is pressed but cursor is outside client area.
		//SetCapture() behaviour.

		CRect rcClient;
		GetClientRect(&rcClient);
		//Checking for scrollbars existence first.
		if (m_stScrollH.IsVisible())
		{
			if (point.x < rcClient.left)
			{
				m_stScrollH.ScrollLineLeft();
				point.x = m_iIndentFirstHexChunk;
			}
			else if (point.x >= rcClient.right)
			{
				m_stScrollH.ScrollLineRight();
				point.x = m_iFourthVertLine - 1;
			}
		}
		if (m_stScrollV.IsVisible())
		{
			if (point.y < m_iHeightTopRect)
			{
				m_stScrollV.ScrollLineUp();
				point.y = m_iHeightTopRect;
			}
			else if (point.y >= m_iHeightWorkArea)
			{
				m_stScrollV.ScrollLineDown();
				point.y = m_iHeightWorkArea - 1;
			}
		}
		const ULONGLONG ullHit = HitTest(&point);
		if (ullHit != -1)
		{
			if (ullHit <= m_ullSelectionClick)
			{
				m_ullSelectionStart = ullHit;
				m_ullSelectionEnd = m_ullSelectionClick;
			}
			else
			{
				m_ullSelectionStart = m_ullSelectionClick;
				m_ullSelectionEnd = ullHit;
			}

			m_ullBytesSelected = m_ullSelectionEnd - m_ullSelectionStart + 1;

			UpdateInfoText();
		}

		RedrawWindow();
	}
	else
	{
		m_stScrollV.OnMouseMove(nFlags, point);
		m_stScrollH.OnMouseMove(nFlags, point);
	}
}

BOOL CHexCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
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

	ULONGLONG ullCurPos = m_stScrollV.GetScrollPos();
	ULONGLONG ullPage = m_stScrollV.GetScrollPageSize();
	ULONGLONG ullNewPos;
	if (zDelta > 0) //Scrolling Up.
		ullNewPos = ullCurPos < ullPage ? 0 : ullCurPos - ullPage;
	else
		ullNewPos = ullCurPos + ullPage;

	m_stScrollV.SetScrollPos(ullNewPos);

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CHexCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	const ULONGLONG ullHit = HitTest(&point);
	if (ullHit != -1)
	{
		SetCapture();
		if (m_ullBytesSelected && (nFlags & MK_SHIFT))
		{
			if (ullHit <= (int)m_ullSelectionClick)
			{
				m_ullSelectionStart = ullHit;
				m_ullSelectionEnd = m_ullSelectionClick;
			}
			else
			{
				m_ullSelectionStart = m_ullSelectionClick;
				m_ullSelectionEnd = ullHit;
			}

			m_ullBytesSelected = m_ullSelectionEnd - m_ullSelectionStart + 1;
		}
		else
		{
			m_ullSelectionClick = m_ullSelectionStart = m_ullSelectionEnd = ullHit;
			m_ullBytesSelected = 1;
		}

		m_fLMousePressed = true;
		UpdateInfoText();
	}
}

void CHexCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_fLMousePressed = false;
	ReleaseCapture();

	m_stScrollV.OnLButtonUp(nFlags, point);
	m_stScrollH.OnLButtonUp(nFlags, point);

	CWnd::OnLButtonUp(nFlags, point);
}

void CHexCtrl::OnMButtonDown(UINT nFlags, CPoint point)
{
}

BOOL CHexCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT uiId = LOWORD(wParam);

	switch (uiId)
	{
	case IDM_POPUP_SEARCH:
		if (m_fVirtual)
			MessageBoxW(L"This function isn't supported in Virtual mode", L"Error", MB_ICONEXCLAMATION);
		else
			m_dlgSearch.ShowWindow(SW_SHOW);
		break;
	case IDM_POPUP_COPYASHEX:
		if (m_fVirtual)
			MessageBoxW(L"This function isn't supported in Virtual mode", L"Error", MB_ICONEXCLAMATION);
		else
			CopyToClipboard(COPY_AS_HEX);
		break;
	case IDM_POPUP_COPYASHEXFORMATTED:
		if (m_fVirtual)
			MessageBoxW(L"This function isn't supported in Virtual mode", L"Error", MB_ICONEXCLAMATION);
		else
			CopyToClipboard(COPY_AS_HEX_FORMATTED);
		break;
	case IDM_POPUP_COPYASASCII:
		if (m_fVirtual)
			MessageBoxW(L"This function isn't supported in Virtual mode", L"Error", MB_ICONEXCLAMATION);
		else
			CopyToClipboard(COPY_AS_ASCII);
		break;
	case IDM_POPUP_ABOUT:
		m_dlgAbout.DoModal();
		break;
	}

	return CWnd::OnCommand(wParam, lParam);
}

void CHexCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	m_menuPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
}

void CHexCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case 'F':
		if (GetKeyState(VK_CONTROL) < 0)
			m_dlgSearch.ShowWindow(SW_SHOW);
		break;
	case 'C':
		if (GetKeyState(VK_CONTROL) < 0)
			CopyToClipboard(COPY_AS_HEX);
		break;
	case VK_RIGHT:
		if (m_ullBytesSelected && (GetAsyncKeyState(VK_SHIFT) < 0))
		{
			if (m_ullSelectionStart == m_ullSelectionClick)
				SetSelection(m_ullSelectionClick, m_ullSelectionClick, m_ullBytesSelected + 1);
			else
				SetSelection(m_ullSelectionClick, m_ullSelectionStart + 1, m_ullBytesSelected - 1);
		}
		else
			SetSelection(m_ullSelectionClick + 1, m_ullSelectionClick + 1, 1);
		break;
	case VK_LEFT:
		if (m_ullBytesSelected && (GetAsyncKeyState(VK_SHIFT) < 0))
		{
			if (m_ullSelectionStart == m_ullSelectionClick && m_ullBytesSelected > 1)
				SetSelection(m_ullSelectionClick, m_ullSelectionClick, m_ullBytesSelected - 1);
			else
				SetSelection(m_ullSelectionClick, m_ullSelectionStart - 1, m_ullBytesSelected + 1);
		}
		else
			SetSelection(m_ullSelectionClick - 1, m_ullSelectionClick - 1, 1);
		break;
	case VK_DOWN:
		if (m_ullBytesSelected && (GetAsyncKeyState(VK_SHIFT) < 0))
		{
			if (m_ullSelectionStart == m_ullSelectionClick)
				SetSelection(m_ullSelectionClick, m_ullSelectionClick, m_ullBytesSelected + m_dwGridCapacity);
			else if (m_ullSelectionStart < m_ullSelectionClick)
			{
				ULONGLONG dwStartAt = m_ullBytesSelected > m_dwGridCapacity ? m_ullSelectionStart + m_dwGridCapacity : m_ullSelectionClick;
				ULONGLONG dwBytes = m_ullBytesSelected >= m_dwGridCapacity ? m_ullBytesSelected - m_dwGridCapacity : m_dwGridCapacity;
				SetSelection(m_ullSelectionClick, dwStartAt, dwBytes ? dwBytes : 1);
			}
		}
		else
			SetSelection(m_ullSelectionClick + m_dwGridCapacity, m_ullSelectionClick + m_dwGridCapacity, 1);
		break;
	case VK_UP:
		if (m_ullBytesSelected && (GetAsyncKeyState(VK_SHIFT) < 0))
		{
			if (m_ullSelectionStart == 0)
				return;

			if (m_ullSelectionStart < m_ullSelectionClick)
			{
				ULONGLONG dwStartAt;
				ULONGLONG dwBytes;
				if (m_ullSelectionStart < m_dwGridCapacity)
				{
					dwStartAt = 0;
					dwBytes = m_ullBytesSelected + m_ullSelectionStart;
				}
				else
				{
					dwStartAt = m_ullSelectionStart - m_dwGridCapacity;
					dwBytes = m_ullBytesSelected + m_dwGridCapacity;
				}
				SetSelection(m_ullSelectionClick, dwStartAt, dwBytes);
			}
			else
			{
				ULONGLONG dwStartAt = m_ullBytesSelected >= m_dwGridCapacity ? m_ullSelectionClick : m_ullSelectionClick - m_dwGridCapacity + 1;
				ULONGLONG dwBytes = m_ullBytesSelected >= m_dwGridCapacity ? m_ullBytesSelected - m_dwGridCapacity : m_dwGridCapacity;
				SetSelection(m_ullSelectionClick, dwStartAt, dwBytes ? dwBytes : 1);
			}
		}
		else
			SetSelection(m_ullSelectionClick - m_dwGridCapacity, m_ullSelectionClick - m_dwGridCapacity, 1);
		break;
	case VK_PRIOR: //Page-Up
		m_stScrollV.ScrollPageUp();
		break;
	case VK_NEXT:  //Page-Down
		m_stScrollV.ScrollPageDown();
		break;
	case VK_HOME:
		m_stScrollV.ScrollHome();
		break;
	case VK_END:
		m_stScrollV.ScrollEnd();
		break;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CHexCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	RedrawWindow();
}

void CHexCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	RedrawWindow();
}

void CHexCtrl::OnPaint()
{
	CPaintDC dc(this);

	ULONGLONG iScrollV = m_stScrollV.GetScrollPos();
	int iScrollH = (int)m_stScrollH.GetScrollPos();

	CRect rcClient;
	GetClientRect(rcClient);
	//Drawing through CMemDC to avoid flickering.
	CMemDC memDC(dc, rcClient);
	CDC& rDC = memDC.GetDC();

	RECT rc; //Used for all local rc related drawing.
	rDC.GetClipBox(&rc);
	rDC.FillSolidRect(&rc, m_clrBk);
	rDC.SelectObject(&m_penLines);
	rDC.SelectObject(&m_fontHexView);

	//To prevent drawing in too small window (can cause hangs).
	if (rcClient.Height() < m_iHeightTopRect + m_iHeightBottomOffArea)
		return;

	//Find the ullLineStart and ullLineEnd position, draw the visible portion.
	const ULONGLONG ullLineStart = iScrollV / GetPixelsLineScrollV();
	ULONGLONG ullLineEndtmp { };
	if (m_ullDataCount)
	{
		ullLineEndtmp = ullLineStart + (rcClient.Height() - m_iHeightTopRect - m_iHeightBottomOffArea) / m_sizeLetter.cy;

		//If m_dwDataCount is really small we adjust dwLineEnd to be not bigger than maximum allowed.
		if (ullLineEndtmp > (m_ullDataCount / m_dwGridCapacity))
			ullLineEndtmp = m_ullDataCount % m_dwGridCapacity ? m_ullDataCount / m_dwGridCapacity + 1 : m_ullDataCount / m_dwGridCapacity;
	}
	const ULONGLONG ullLineEnd = ullLineEndtmp;

	const auto iFirstHorizLine = 0;
	const auto iSecondHorizLine = iFirstHorizLine + m_iHeightTopRect - 1;
	const auto iThirdHorizLine = iFirstHorizLine + rcClient.Height() - m_iHeightBottomOffArea;
	const auto iFourthHorizLine = iThirdHorizLine + m_iHeightBottomRect;

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
	rc.left = m_iFirstVertLine - iScrollH; rc.top = iFirstHorizLine;
	rc.right = m_iSecondVertLine - iScrollH; rc.bottom = iSecondHorizLine;
	rDC.SetTextColor(m_clrTextCaption);
	DrawTextW(rDC.m_hDC, L"Offset", 6, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	//«Bytes total:» text.
	rc.left = m_iFirstVertLine + 5;	rc.top = iThirdHorizLine + 1;
	rc.right = rcClient.right > m_iFourthVertLine ? rcClient.right : m_iFourthVertLine;
	rc.bottom = iFourthHorizLine;
	rDC.FillSolidRect(&rc, m_clrBkBottomRect);
	rDC.SetTextColor(m_clrTextBottomRect);
	rDC.SelectObject(&m_fontBottomRect);
	DrawTextW(rDC.m_hDC, m_wstrBottomText.data(), m_wstrBottomText.size(), &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	rDC.SelectObject(&m_fontHexView);
	rDC.SetTextColor(m_clrTextCaption);
	rDC.SetBkColor(m_clrBk);

	//Loop for printing top Capacity numbers.
	for (unsigned iterCapacity = 0; iterCapacity < m_dwGridCapacity; iterCapacity++)
	{
		WCHAR wstrCapacity[4];
		swprintf_s(&wstrCapacity[0], 3, L"%X", iterCapacity);

		int x, c;
		if (iterCapacity < m_dwGridBlockSize) //Top capacity numbers (0 1 2 3 4 5 6 7...)
			x = m_iIndentFirstHexChunk + m_sizeLetter.cx + (m_iDistanceBetweenHexChunks*iterCapacity);
		else //Top capacity numbers, second block (8 9 A B C D E F...).
			x = m_iIndentFirstHexChunk + m_sizeLetter.cx + (m_iDistanceBetweenHexChunks*iterCapacity) + m_iSpaceBetweenBlocks;

		//If iterCapacity >= 16 two chars needed (10, 11,... 1F) to print as capacity.
		if (iterCapacity < 16)
			c = 1;
		else {
			c = 2;
			x -= m_sizeLetter.cx;
		}
		ExtTextOutW(rDC.m_hDC, x - iScrollH, iFirstHorizLine + m_iIndentTextCapacityY, NULL, nullptr, &wstrCapacity[0], c, nullptr);
	}

	//"Ascii" text.
	rc.left = m_iThirdVertLine - iScrollH; rc.top = iFirstHorizLine;
	rc.right = m_iFourthVertLine - iScrollH; rc.bottom = iSecondHorizLine;
	DrawTextW(rDC.m_hDC, L"Ascii", 5, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	//First Vertical line.
	rDC.MoveTo(m_iFirstVertLine - iScrollH, 0);
	rDC.LineTo(m_iFirstVertLine - iScrollH, iFourthHorizLine);

	//Second Vertical line.
	rDC.MoveTo(m_iSecondVertLine - iScrollH, 0);
	rDC.LineTo(m_iSecondVertLine - iScrollH, iThirdHorizLine);

	//Third Vertical line.
	rDC.MoveTo(m_iThirdVertLine - iScrollH, 0);
	rDC.LineTo(m_iThirdVertLine - iScrollH, iThirdHorizLine);

	//Fourth Vertical line.
	rDC.MoveTo(m_iFourthVertLine - iScrollH, 0);
	rDC.LineTo(m_iFourthVertLine - iScrollH, iFourthHorizLine);

	//Current line to print.
	int iLine { };
	//Loop for printing hex and Ascii line by line.

	for (ULONGLONG iterLines = ullLineStart; iterLines < ullLineEnd; iterLines++)
	{
		WCHAR wstrOffset[9];
		swprintf_s(wstrOffset, 9, L"%08llX", iterLines * m_dwGridCapacity);

		//Drawing m_strOffset with bk color depending on selection range.
		if (m_ullBytesSelected && (iterLines * m_dwGridCapacity + m_dwGridCapacity) > m_ullSelectionStart &&
			(iterLines * m_dwGridCapacity) <= m_ullSelectionEnd)
			rDC.SetBkColor(m_clrBkSelected);
		else
			rDC.SetBkColor(m_clrBk);

		//Left column offset print (00000001...0000FFFF...).
		rDC.SetTextColor(m_clrTextCaption);
		ExtTextOutW(rDC.m_hDC, m_sizeLetter.cx - iScrollH, m_iHeightTopRect + (m_sizeLetter.cy * iLine),
			NULL, nullptr, wstrOffset, 8, nullptr);

		int iIndentHexX { };
		int iIndentAsciiX { };
		int iIndentBetweenBlocks { };

		//Main loop for printing Hex chunks and Ascii chars.
		for (unsigned iterChunks = 0; iterChunks < m_dwGridCapacity; iterChunks++)
		{
			if (iterChunks >= m_dwGridBlockSize)
				iIndentBetweenBlocks = m_iSpaceBetweenBlocks;

			const UINT iHexPosToPrintX = m_iIndentFirstHexChunk + iIndentHexX + iIndentBetweenBlocks - iScrollH;
			const UINT iHexPosToPrintY = m_iHeightTopRect + m_sizeLetter.cy * iLine;
			const UINT iAsciiPosToPrintX = m_iIndentAscii + iIndentAsciiX - iScrollH;
			const UINT iAsciiPosToPrintY = m_iHeightTopRect + m_sizeLetter.cy * iLine;

			//Index of the next char (in m_pData) to draw.
			const ULONGLONG ullIndexDataToPrint = iterLines * m_dwGridCapacity + iterChunks;

			if (ullIndexDataToPrint < m_ullDataCount) //Draw until reaching the end of m_dwDataCount.
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
				//If it's virtual data control, we aquire next byte to print from parent window.
				WCHAR wstrHexToPrint[2];
				unsigned char chByteToPrint { };
				if (m_fVirtual && m_pwndParentOwner)
				{
					HEXNOTIFY hexntfy { { m_hWnd, (UINT)GetDlgCtrlID(), HEXCTRL_MSG_GETDISPINFO },
						ullIndexDataToPrint, 0 };
					m_pwndParentOwner->SendMessageW(WM_NOTIFY, hexntfy.hdr.idFrom, (LPARAM)&hexntfy);
					chByteToPrint = hexntfy.chByte;
				}
				else
					chByteToPrint = m_pData[ullIndexDataToPrint];

				wstrHexToPrint[0] = m_pwszHexMap[(chByteToPrint & 0xF0) >> 4];
				wstrHexToPrint[1] = m_pwszHexMap[(chByteToPrint & 0x0F)];

				//Selection draw with different BK color.
				if (m_ullBytesSelected && ullIndexDataToPrint >= m_ullSelectionStart && ullIndexDataToPrint <= m_ullSelectionEnd)
				{
					rDC.SetBkColor(m_clrBkSelected);

					//To prevent change bk color after last selected Hex in a row, and very last Hex.
					if (ullIndexDataToPrint != m_ullSelectionEnd && (ullIndexDataToPrint + 1) % m_dwGridCapacity)
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
				char chAsciiToPrint = chByteToPrint;
				//For non printable Ascii just print a dot.
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
			iIndentHexX += m_iDistanceBetweenHexChunks;
			iIndentAsciiX += m_iSpaceBetweenAscii;
		}
		iLine++;
	}
}

void CHexCtrl::OnDraw(CDC * pDC)
{
}

BOOL CHexCtrl::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

BOOL CHexCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	m_stScrollV.OnSetCursor(pWnd, nHitTest, message);
	m_stScrollH.OnSetCursor(pWnd, nHitTest, message);

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CHexCtrl::OnNcActivate(BOOL bActive)
{
	m_stScrollV.OnNcActivate(bActive);
	m_stScrollH.OnNcActivate(bActive);

	return CWnd::OnNcActivate(bActive);
}

void CHexCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	//Sequince is important - H->V.
	m_stScrollH.OnNcCalcSize(bCalcValidRects, lpncsp);
	m_stScrollV.OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CHexCtrl::OnNcPaint()
{
	Default();

	m_stScrollV.OnNcPaint();
	m_stScrollH.OnNcPaint();
}

ULONGLONG CHexCtrl::HitTest(LPPOINT pPoint)
{
	ULONGLONG dwHexChunk;
	ULONGLONG iScrollV = m_stScrollV.GetScrollPos();
	int iScrollH = (int)m_stScrollH.GetScrollPos();

	//To compensate horizontal scroll.
	pPoint->x += iScrollH;

	//Checking if cursor is within hex chunks area.
	if ((pPoint->x >= m_iIndentFirstHexChunk) && (pPoint->x < m_iThirdVertLine)
		&& (pPoint->y >= m_iHeightTopRect) && pPoint->y <= m_iHeightWorkArea)
	{
		int itmpBetweenBlocks;
		if (pPoint->x > m_iIndentFirstHexChunk + (m_iDistanceBetweenHexChunks * (int)m_dwGridBlockSize))
			itmpBetweenBlocks = m_iSpaceBetweenBlocks;
		else
			itmpBetweenBlocks = 0;

		//Calculate ullHit hex chunk, taking into account scroll position and letter sizes.
		dwHexChunk = ((pPoint->x - m_iIndentFirstHexChunk - itmpBetweenBlocks) / (m_sizeLetter.cx * 3)) +
			((pPoint->y - m_iHeightTopRect) / m_sizeLetter.cy) * m_dwGridCapacity +
			((iScrollV / GetPixelsLineScrollV()) * m_dwGridCapacity);
	}
	else if ((pPoint->x >= m_iIndentAscii) && (pPoint->x < (m_iIndentAscii + m_iSpaceBetweenAscii * (int)m_dwGridCapacity))
		&& (pPoint->y >= m_iHeightTopRect) && pPoint->y <= m_iHeightWorkArea)
	{
		//Calculate ullHit Ascii symbol.
		dwHexChunk = ((pPoint->x - m_iIndentAscii) / (m_iSpaceBetweenAscii)) +
			((pPoint->y - m_iHeightTopRect) / m_sizeLetter.cy) * m_dwGridCapacity +
			((iScrollV / GetPixelsLineScrollV()) * m_dwGridCapacity);
	}
	else
		dwHexChunk = -1;

	//If cursor is out of end-bound of hex chunks or Ascii chars.
	if (dwHexChunk >= m_ullDataCount)
		dwHexChunk = -1;

	return dwHexChunk;
}

void CHexCtrl::HexPoint(ULONGLONG ullChunk, ULONGLONG& ullCx, ULONGLONG& ullCy)
{
	int itmpBetweenBlocks;
	if (ullChunk % m_dwGridCapacity > m_dwGridBlockSize)
		itmpBetweenBlocks = m_iSpaceBetweenBlocks;
	else
		itmpBetweenBlocks = 0;

	ullCx = m_iIndentFirstHexChunk + ((ullChunk % m_dwGridCapacity)*m_iDistanceBetweenHexChunks) + itmpBetweenBlocks;

	if (ullChunk % m_dwGridCapacity)
		ullCy = (ullChunk / m_dwGridCapacity) * m_sizeLetter.cy + m_sizeLetter.cy;
	else
		ullCy = (ullChunk / m_dwGridCapacity) * m_sizeLetter.cy;
}

void CHexCtrl::CopyToClipboard(UINT nType)
{
	if (!m_ullBytesSelected)
		return;

	const char* const strHexMap = "0123456789ABCDEF";
	char chHexToCopy[2];
	std::string strToClipboard { };

	switch (nType)
	{
	case COPY_AS_HEX:
	{
		for (unsigned i = 0; i < m_ullBytesSelected; i++)
		{
			chHexToCopy[0] = strHexMap[((unsigned char)m_pData[m_ullSelectionStart + i] & 0xF0) >> 4];
			chHexToCopy[1] = strHexMap[((unsigned char)m_pData[m_ullSelectionStart + i] & 0x0F)];
			strToClipboard += chHexToCopy[0];
			strToClipboard += chHexToCopy[1];
		}
		break;
	}
	case COPY_AS_HEX_FORMATTED:
	{
		//How many spaces are needed to be inserted at the beginnig.
		DWORD dwModStart = m_ullSelectionStart % m_dwGridCapacity;
		//When to insert first "\r\n".
		DWORD dwTail = m_dwGridCapacity - dwModStart;
		DWORD dwNextBlock = m_dwGridCapacity % 2 ? m_dwGridBlockSize + 2 : m_dwGridBlockSize + 1;

		//If at least two rows are selected.
		if (dwModStart + m_ullBytesSelected > m_dwGridCapacity)
		{
			strToClipboard.insert(0, dwModStart * 3, ' ');
			if (dwTail <= m_dwGridBlockSize)
				strToClipboard.insert(0, 2, ' ');
		}

		for (unsigned i = 0; i < m_ullBytesSelected; i++)
		{
			chHexToCopy[0] = strHexMap[((unsigned char)m_pData[m_ullSelectionStart + i] & 0xF0) >> 4];
			chHexToCopy[1] = strHexMap[((unsigned char)m_pData[m_ullSelectionStart + i] & 0x0F)];
			strToClipboard += chHexToCopy[0];
			strToClipboard += chHexToCopy[1];

			if (i < (m_ullBytesSelected - 1) && (dwTail - 1) != 0)
				if (dwTail == dwNextBlock) //Space between blocks.
					strToClipboard += "   ";
				else
					strToClipboard += " ";
			if (--dwTail == 0 && i < (m_ullBytesSelected - 1)) //Next string.
			{
				strToClipboard += "\r\n";
				dwTail = m_dwGridCapacity;
			}
		}
		break;
	}
	case COPY_AS_ASCII:
	{
		char ch;
		for (unsigned i = 0; i < m_ullBytesSelected; i++)
		{
			ch = m_pData[m_ullSelectionStart + i];
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

void CHexCtrl::UpdateInfoText()
{
	if (!m_ullDataCount)
		m_wstrBottomText.clear();
	else
	{
		m_wstrBottomText.resize(128);
		if (m_ullBytesSelected)
			m_wstrBottomText.resize(swprintf_s(&m_wstrBottomText[0], 128, L"Bytes selected: 0x%llX(%llu); Offset: 0x%llX(%llu) - 0x%llX(%llu)",
				m_ullBytesSelected, m_ullBytesSelected, m_ullSelectionStart, m_ullSelectionStart, m_ullSelectionEnd, m_ullSelectionEnd));
		else
			m_wstrBottomText.resize(swprintf_s(&m_wstrBottomText[0], 128, L"Bytes total: 0x%llX(%llu)", m_ullDataCount, m_ullDataCount));
	}
	RedrawWindow();
}

void CHexCtrl::Recalc()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	ULONGLONG ullStartLineV = m_stScrollV.GetScrollPos() / GetPixelsLineScrollV();
	HDC hDC = ::GetDC(m_hWnd);
	SelectObject(hDC, m_fontHexView.m_hObject);
	TEXTMETRICW tm { };
	GetTextMetricsW(hDC, &tm);
	m_sizeLetter.cx = tm.tmAveCharWidth;
	m_sizeLetter.cy = tm.tmHeight + tm.tmExternalLeading;
	::ReleaseDC(m_hWnd, hDC);

	m_iFirstVertLine = 0;
	m_iSecondVertLine = m_sizeLetter.cx * 10;
	m_iDistanceBetweenHexChunks = m_sizeLetter.cx * 3;
	m_iSpaceBetweenBlocks = m_sizeLetter.cx * 2;
	m_iThirdVertLine = m_iSecondVertLine + (m_iDistanceBetweenHexChunks * m_dwGridCapacity) + m_iSpaceBetweenBlocks + m_sizeLetter.cx;
	m_iIndentAscii = m_iThirdVertLine + m_sizeLetter.cx;
	m_iSpaceBetweenAscii = m_sizeLetter.cx + 1;
	m_iFourthVertLine = m_iIndentAscii + (m_iSpaceBetweenAscii * m_dwGridCapacity) + m_sizeLetter.cx;
	m_iIndentFirstHexChunk = m_iSecondVertLine + m_sizeLetter.cx;
	m_iHeightTopRect = int(m_sizeLetter.cy * 1.5);
	m_iHeightWorkArea = rcClient.Height() - m_iHeightBottomOffArea -
		((rcClient.Height() - m_iHeightTopRect - m_iHeightBottomOffArea) % m_sizeLetter.cy);
	m_iIndentTextCapacityY = (m_iHeightTopRect / 2) - (m_sizeLetter.cy / 2);

	//Scroll sizes according to current font size.
	UINT uiPage = m_iHeightWorkArea - m_iHeightTopRect;
	m_stScrollV.SetScrollSizes(GetPixelsLineScrollV(), uiPage,
		m_iHeightTopRect + m_iHeightBottomOffArea + (GetPixelsLineScrollV() * (m_ullDataCount / m_dwGridCapacity + 2)));
	m_stScrollV.SetScrollPos(ullStartLineV * GetPixelsLineScrollV());
	m_stScrollH.SetScrollSizes(m_sizeLetter.cx, rcClient.Width(), m_iFourthVertLine + 1);

	RedrawWindow();
}

void CHexCtrl::Search(HEXSEARCH& rSearch)
{
	rSearch.fFound = false;
	ULONGLONG dwStartAt = rSearch.ullStartAt;
	ULONGLONG dwSizeBytes;
	ULONGLONG dwUntil;
	std::string strSearch { };

	if (rSearch.wstrSearch.empty() || m_ullDataCount == 0 || rSearch.ullStartAt > (m_ullDataCount - 1))
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
		if (dwSizeBytes > m_ullDataCount)
			goto End;

		break;
	}
	case SEARCH_ASCII:
	{
		dwSizeBytes = strSearchAscii.size();
		if (dwSizeBytes > m_ullDataCount)
			goto End;

		strSearch = std::move(strSearchAscii);
		break;
	}
	case SEARCH_UNICODE:
	{
		dwSizeBytes = rSearch.wstrSearch.length() * sizeof(wchar_t);
		if (dwSizeBytes > m_ullDataCount)
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
			dwUntil = m_ullDataCount - strSearch.size();
			dwStartAt = rSearch.fSecondMatch ? rSearch.ullStartAt + 1 : 0;

			for (ULONGLONG i = dwStartAt; i <= dwUntil; i++)
			{
				if (memcmp(m_pData + i, strSearch.data(), strSearch.size()) == 0)
				{
					rSearch.fFound = true;
					rSearch.ullStartAt = i;
					rSearch.fWrap = false;
					goto End;
				}
			}

			dwStartAt = 0;
			for (ULONGLONG i = dwStartAt; i <= dwUntil; i++)
			{
				if (memcmp(m_pData + i, strSearch.data(), strSearch.size()) == 0)
				{
					rSearch.fFound = true;
					rSearch.ullStartAt = i;
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
						rSearch.ullStartAt = i;
						rSearch.fWrap = false;
						goto End;
					}
				}
			}

			dwStartAt = m_ullDataCount - strSearch.size();
			for (int i = (int)dwStartAt; i >= 0; i--)
			{
				if (memcmp(m_pData + i, strSearch.data(), strSearch.size()) == 0)
				{
					rSearch.fFound = true;
					rSearch.ullStartAt = i;
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
			dwUntil = m_ullDataCount - dwSizeBytes;
			dwStartAt = rSearch.fSecondMatch ? rSearch.ullStartAt + 1 : 0;

			for (ULONGLONG i = dwStartAt; i <= dwUntil; i++)
			{
				if (wmemcmp((const wchar_t*)(m_pData + i), rSearch.wstrSearch.data(), rSearch.wstrSearch.length()) == 0)
				{
					rSearch.fFound = true;
					rSearch.ullStartAt = i;
					rSearch.fWrap = false;
					goto End;
				}
			}

			dwStartAt = 0;
			for (ULONGLONG i = dwStartAt; i <= dwUntil; i++)
			{
				if (wmemcmp((const wchar_t*)(m_pData + i), rSearch.wstrSearch.data(), rSearch.wstrSearch.length()) == 0)
				{
					rSearch.fFound = true;
					rSearch.ullStartAt = i;
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
						rSearch.ullStartAt = i;
						rSearch.fWrap = false;
						goto End;
					}
				}
			}

			dwStartAt = m_ullDataCount - dwSizeBytes;
			for (int i = (int)dwStartAt; i >= 0; i--)
			{
				if (wmemcmp((const wchar_t*)(m_pData + i), rSearch.wstrSearch.data(), rSearch.wstrSearch.length()) == 0)
				{
					rSearch.fFound = true;
					rSearch.ullStartAt = i;
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
			SetSelection(rSearch.ullStartAt, rSearch.ullStartAt, dwSizeBytes, true);
	}
}

void CHexCtrl::SetSelection(ULONGLONG ullClick, ULONGLONG ullStart, ULONGLONG ullSize, bool fHighlight)
{
	if (ullClick >= m_ullDataCount || ullStart >= m_ullDataCount || !ullSize)
		return;
	if ((ullStart + ullSize) > m_ullDataCount)
		ullSize = m_ullDataCount - ullStart;

	ULONGLONG ullCurrScrollV = m_stScrollV.GetScrollPos();
	ULONGLONG ullCurrScrollH = m_stScrollH.GetScrollPos();
	ULONGLONG ullCx, ullCy;
	HexPoint(ullStart, ullCx, ullCy);
	CRect rcClient;
	GetClientRect(&rcClient);

	//New scroll depending on selection direction: top <-> bottom.
	//fHighlight means centralize scroll position on the screen (used in Search()).
	ULONGLONG ullEnd = ullStart + ullSize - 1;
	ULONGLONG ullPixelsLineScrollV = GetPixelsLineScrollV();
	ULONGLONG ullMaxV = (ullCurrScrollV / ullPixelsLineScrollV * m_sizeLetter.cy) + rcClient.Height() - m_iHeightBottomOffArea - m_iHeightTopRect -
		((rcClient.Height() - m_iHeightTopRect - m_iHeightBottomOffArea) % m_sizeLetter.cy);
	ULONGLONG ullNewStartV = ullStart / m_dwGridCapacity * ullPixelsLineScrollV;
	ULONGLONG ullNewEndV = ullEnd / m_dwGridCapacity * ullPixelsLineScrollV;

	ULONGLONG iNewScrollV { }, iNewScrollH { };
	if (fHighlight)
	{
		iNewScrollV = ullNewStartV - ((m_iHeightWorkArea / 2) / m_sizeLetter.cy * ullPixelsLineScrollV);
		iNewScrollH = (ullStart % m_dwGridCapacity) * m_iDistanceBetweenHexChunks;
	}
	else
	{
		if (ullStart == ullClick)
		{
			if (ullNewEndV >= ullMaxV)
				iNewScrollV = ullCurrScrollV + ullPixelsLineScrollV;
			else
			{
				if (ullNewEndV >= ullCurrScrollV)
					iNewScrollV = ullCurrScrollV;
				else if (ullNewStartV <= ullCurrScrollV)
					iNewScrollV = ullCurrScrollV - ullPixelsLineScrollV;
			}
		}
		else
		{
			if (ullNewStartV < ullCurrScrollV)
				iNewScrollV = ullCurrScrollV - ullPixelsLineScrollV;
			else
			{
				if (ullNewStartV < ullMaxV)
					iNewScrollV = ullCurrScrollV;
				else
					iNewScrollV = ullCurrScrollV + ullPixelsLineScrollV;
			}
		}

		if (ullCx >= (ullCurrScrollH + rcClient.Width()))
			iNewScrollH = int(ullCurrScrollH + ((ullCx - ullCurrScrollH) / m_sizeLetter.cx));
		else if (ullCx < ullCurrScrollH)
			iNewScrollH = int(ullCx);
		else
			iNewScrollH = ullCurrScrollH;
	}
	iNewScrollV -= iNewScrollV % ullPixelsLineScrollV;
	iNewScrollH -= iNewScrollH % m_sizeLetter.cx;

	m_ullSelectionClick = ullClick;
	m_ullSelectionStart = ullStart;
	m_ullSelectionEnd = ullEnd;
	m_ullBytesSelected = ullEnd - ullStart + 1;

	m_stScrollV.SetScrollPos(iNewScrollV);
	m_stScrollH.SetScrollPos(iNewScrollH);

	UpdateInfoText();
}

int CHexCtrl::GetPixelsLineScrollV()
{
	return m_sizeLetter.cy;
}

void CHexCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	Invalidate();
}

void CHexCtrl::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CWnd::OnGetMinMaxInfo(lpMMI);
}


/************************************************************
* CHexDlgSearch class implementation.						*
* This class implements search routines within HexControl.	*
************************************************************/
BEGIN_MESSAGE_MAP(CHexDlgSearch, CDialogEx)
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_F, &CHexDlgSearch::OnButtonSearchF)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_B, &CHexDlgSearch::OnButtonSearchB)
	ON_COMMAND_RANGE(IDC_RADIO_HEX, IDC_RADIO_UNICODE, &CHexDlgSearch::OnRadioBnRange)
END_MESSAGE_MAP()

BOOL CHexDlgSearch::Create(UINT nIDTemplate, CHexCtrl* pwndParent)
{
	m_pParent = pwndParent;

	return CDialog::Create(nIDTemplate, m_pParent);
}

CHexCtrl* CHexDlgSearch::GetParent() const
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
				if (m_stSearch.iDirection == CHexCtrl::HEXCTRL_SEARCH::SEARCH_FORWARD)
					m_dwnOccurrence++;
				else if (m_stSearch.iDirection == CHexCtrl::HEXCTRL_SEARCH::SEARCH_BACKWARD)
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
		m_stSearch.dwSearchType = CHexCtrl::HEXCTRL_SEARCH::SEARCH_HEX;
		break;
	case IDC_RADIO_ASCII:
		m_stSearch.dwSearchType = CHexCtrl::HEXCTRL_SEARCH::SEARCH_ASCII;
		break;
	case IDC_RADIO_UNICODE:
		m_stSearch.dwSearchType = CHexCtrl::HEXCTRL_SEARCH::SEARCH_UNICODE;
		break;
	}
	m_stSearch.iDirection = CHexCtrl::HEXCTRL_SEARCH::SEARCH_FORWARD;

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
		m_stSearch.dwSearchType = CHexCtrl::HEXCTRL_SEARCH::SEARCH_HEX;
		break;
	case IDC_RADIO_ASCII:
		m_stSearch.dwSearchType = CHexCtrl::HEXCTRL_SEARCH::SEARCH_ASCII;
		break;
	case IDC_RADIO_UNICODE:
		m_stSearch.dwSearchType = CHexCtrl::HEXCTRL_SEARCH::SEARCH_UNICODE;
		break;
	}
	m_stSearch.iDirection = CHexCtrl::HEXCTRL_SEARCH::SEARCH_BACKWARD;

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
	m_stSearch.ullStartAt = 0;
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