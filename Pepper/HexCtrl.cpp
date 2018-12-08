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

/************************************************************************
* CHexView implementation.												*
************************************************************************/

/********************************************************************
* Below is the custom implementation of MFC IMPLEMENT_DYNCREATE()	* 
* macro, which doesn't work with nested classes by default.			*
********************************************************************/
CObject* PASCAL CHexCtrl::CHexView::CreateObject()
{
	return new CHexCtrl::CHexView;
}

CRuntimeClass* PASCAL CHexCtrl::CHexView::_GetBaseClass()
{
	return RUNTIME_CLASS(CScrollView);
}

AFX_COMDAT const CRuntimeClass CHexCtrl::CHexView::classCHexView = {
	"CHexView", sizeof(class CHexCtrl::CHexView), 0xFFFF, CHexCtrl::CHexView::CreateObject,
	&CHexCtrl::CHexView::_GetBaseClass, NULL, NULL };

CRuntimeClass* PASCAL CHexCtrl::CHexView::GetThisClass()
{
	return (CRuntimeClass*)(&CHexCtrl::CHexView::classCHexView);
}

CRuntimeClass* CHexCtrl::CHexView::GetRuntimeClass() const
{
	return (CRuntimeClass*)(&CHexCtrl::CHexView::classCHexView);
}
/********************************************************************
* End of custom IMPLEMENT_DYNCREATE()								*
********************************************************************/

BEGIN_MESSAGE_MAP(CHexCtrl::CHexView, CScrollView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_COMMAND_RANGE(IDC_MENU_POPUP_COPY_AS_HEX, IDC_MENU_POPUP_COPY_AS_ASCII, &CHexCtrl::CHexView::OnMenuRange)
END_MESSAGE_MAP()

BOOL CHexCtrl::CHexView::Create(CWnd * pParent, const RECT & rect, UINT nID, CCreateContext* pContext, CFont* pFont)
{
	SetFont(pFont);

	m_menuPopup.CreatePopupMenu();
	m_menuPopup.AppendMenuW(MF_STRING, IDC_MENU_POPUP_COPY_AS_HEX, L"Copy as Hex...");
	m_menuPopup.AppendMenuW(MF_STRING, IDC_MENU_POPUP_COPY_AS_HEX_FORMATTED, L"Copy as Formatted Hex...		Ctrl+C");
	m_menuPopup.AppendMenuW(MF_STRING, IDC_MENU_POPUP_COPY_AS_ASCII, L"Copy as Ascii...");

	return CScrollView::Create(nullptr, nullptr, WS_VISIBLE | WS_CHILD, rect, pParent, nID, pContext);
}

void CHexCtrl::CHexView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
}

void CHexCtrl::CHexView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//If scrollbar doesn't exist on the screen - do nothing.
	GetScrollBarInfo(OBJID_VSCROLL, &m_stSBI);
	if (m_stSBI.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return;

	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);

	int pos = m_stScrollInfo.nPos;
	switch (nSBCode)
	{
	case SB_TOP: pos = m_stScrollInfo.nMin; break;
	case SB_BOTTOM: pos = m_stScrollInfo.nMax; break;
	case SB_LINEUP: pos -= m_sizeLetter.cy; break;
	case SB_LINEDOWN: pos += m_sizeLetter.cy;  break;
	case SB_PAGEUP: pos -= m_sizeLetter.cy * 16; break;
	case SB_PAGEDOWN: pos += m_sizeLetter.cy * 16; break;
	case SB_THUMBPOSITION: pos = m_stScrollInfo.nTrackPos; break;
	case SB_THUMBTRACK: pos = m_stScrollInfo.nTrackPos; break;
	}

	//Make sure the new position is within range.
	if (pos < m_stScrollInfo.nMin)
		pos = m_stScrollInfo.nMin;
	int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
	if (pos > max)
		pos = max;

	m_stScrollInfo.nPos = pos;
	SetScrollInfo(SB_VERT, &m_stScrollInfo);

	Invalidate();
}

void CHexCtrl::CHexView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//If scrollbar doesn't exist on the screen - do nothing.
	GetScrollBarInfo(OBJID_HSCROLL, &m_stSBI);
	if (m_stSBI.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return;

	GetScrollInfo(SB_HORZ, &m_stScrollInfo, SIF_ALL);

	int pos = m_stScrollInfo.nPos;
	switch (nSBCode)
	{
	case SB_LEFT: pos = m_stScrollInfo.nMin; break;
	case SB_RIGHT: pos = m_stScrollInfo.nMax; break;
	case SB_LINELEFT: pos -= m_sizeLetter.cx; break;
	case SB_LINERIGHT: pos += m_sizeLetter.cx;  break;
	case SB_PAGELEFT: pos -= m_stScrollInfo.nPage; break;
	case SB_PAGERIGHT: pos += m_stScrollInfo.nPage; break;
	case SB_THUMBPOSITION: pos = m_stScrollInfo.nTrackPos; break;
	case SB_THUMBTRACK: pos = m_stScrollInfo.nTrackPos; break;
	}

	//Make sure the new position is within range.
	if (pos < m_stScrollInfo.nMin)
		pos = m_stScrollInfo.nMin;
	int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
	if (pos > max)
		pos = max;

	m_stScrollInfo.nPos = pos;
	SetScrollInfo(SB_HORZ, &m_stScrollInfo);

	Invalidate();
}

void CHexCtrl::CHexView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_fLMousePressed)
	{
		//If LMouse is pressed but cursor is outside client area.
		//SetCapture() behaviour.
		if (point.x < m_iIndentFirstHexChunk)
		{
			OnHScroll(SB_LINELEFT, m_stScrollInfo.nPos, nullptr);
			point.x = m_iIndentFirstHexChunk;
		}
		else if (point.x >= m_iThirdVertLine)
		{
			OnHScroll(SB_LINERIGHT, m_stScrollInfo.nPos, nullptr);
			point.x = m_iThirdVertLine - 1;
		}
		if (point.y < m_iHeightHeaderRect)
		{
			OnVScroll(SB_LINEUP, m_stScrollInfo.nPos, nullptr);
			point.y = m_iHeightHeaderRect;
		}
		else if (point.y >= m_iHeightWorkArea)
		{
			OnVScroll(SB_LINEDOWN, m_stScrollInfo.nPos, nullptr);
			point.y = m_iHeightWorkArea - 1;
		}

		const int tmpEnd = HitTest(&point);
		if (tmpEnd != -1)
			m_dwSelectionEnd = tmpEnd;
		Invalidate();
	}
}

BOOL CHexCtrl::CHexView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (nFlags == MK_CONTROL)
	{
		SetFontSize(GetFontSize() + zDelta / WHEEL_DELTA * 2);
		return TRUE;
	}

	Invalidate();

	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CHexCtrl::CHexView::OnLButtonDown(UINT nFlags, CPoint point)
{
	const int hit = HitTest(&point);
	if (hit != -1)
	{
		SetCapture();
		if (m_fSelected && (nFlags & MK_SHIFT))
			m_dwSelectionEnd = hit;
		else
			m_dwSelectionStart = m_dwSelectionEnd = hit;
		m_fLMousePressed = true;
		m_fSelected = true;
		Invalidate();
	}
}

void CHexCtrl::CHexView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_fLMousePressed = false;
	ReleaseCapture();

	CScrollView::OnLButtonUp(nFlags, point);
}

void CHexCtrl::CHexView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	m_menuPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, this);

	CScrollView::OnRButtonUp(nFlags, point);
}

void CHexCtrl::CHexView::OnMButtonDown(UINT nFlags, CPoint point)
{
}

void CHexCtrl::CHexView::OnSize(UINT nType, int cx, int cy)
{
	Recalc();

	CScrollView::OnSize(nType, cx, cy);
}

void CHexCtrl::CHexView::OnMenuRange(UINT nID)
{
	switch (nID)
	{
	case IDC_MENU_POPUP_COPY_AS_HEX:
		CopyToClipboard(CLIPBOARD_COPY_AS_HEX);
		break;
	case IDC_MENU_POPUP_COPY_AS_HEX_FORMATTED:
		CopyToClipboard(CLIPBOARD_COPY_AS_HEX_FORMATTED);
		break;
	case IDC_MENU_POPUP_COPY_AS_ASCII:
		CopyToClipboard(CLIPBOARD_COPY_AS_ASCII);
		break;
	}
}

void CHexCtrl::CHexView::OnDraw(CDC* pDC)
{
	GetScrollInfo(SB_HORZ, &m_stScrollInfo, SIF_ALL);
	int nScrollHorz = m_stScrollInfo.nPos;
	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);
	int nScrollVert = m_stScrollInfo.nPos;

	CRect rcClientVisible = m_rcClient;
	rcClientVisible.top += nScrollVert;
	rcClientVisible.bottom += nScrollVert;
	rcClientVisible.left += nScrollHorz;
	rcClientVisible.right += nScrollHorz;

	//Drawing through CMemDC, to avoid any flickering.
	CMemDC memDC(*pDC, rcClientVisible);
	CDC& rDC = memDC.GetDC();

	CRect clip;
	rDC.GetClipBox(&clip);
	rDC.FillSolidRect(clip, m_clrTextBk);
	rDC.SelectObject(&m_penLines);
	rDC.SelectObject(&m_fontHexView);

	//Find the nLineStart and nLineEnd position, draw the visible portion.
	const UINT nLineStart = nScrollVert / m_sizeLetter.cy;
	UINT nLineEnd = m_dwRawDataCount ? (nLineStart + (m_rcClient.Height() - m_iHeightHeaderRect - m_iHeightBottomRect) / m_sizeLetter.cy) : 0;
	//If m_dwRawDataCount is really small
	//we adjust nLineEnd to not to be bigger than maximum allowed.
	if (nLineEnd > (m_dwRawDataCount / 16))
		nLineEnd = m_dwRawDataCount % 16 ? m_dwRawDataCount / 16 + 1 : m_dwRawDataCount / 16;

	//Horizontal lines coords depending on scroll position.
	m_iFirstHorizLine = nScrollVert;
	m_iSecondHorizLine = m_iHeightHeaderRect - 1 + nScrollVert;
	m_iThirdHorizLine = m_rcClient.Height() + nScrollVert - m_iHeightBottomRect;
	m_iFourthHorizLine = m_rcClient.Height() + nScrollVert - 3;

	//First horizontal line.
	rDC.MoveTo(0, m_iFirstHorizLine);
	rDC.LineTo(m_iFourthVertLine, m_iFirstHorizLine);

	//Second horizontal line.
	rDC.MoveTo(0, m_iSecondHorizLine);
	rDC.LineTo(m_iFourthVertLine, m_iSecondHorizLine);

	//Third horizontal line.
	rDC.MoveTo(0, m_iThirdHorizLine);
	rDC.LineTo(m_iFourthVertLine, m_iThirdHorizLine);

	//Fourth horizontal line.
	rDC.MoveTo(0, m_iFourthHorizLine);
	rDC.LineTo(m_iFourthVertLine, m_iFourthHorizLine);

	//"Offset" text.
	rDC.SetTextColor(m_clrTextOffset);
	RECT rect;
	rect.left = m_iFirstVertLine; rect.top = m_iFirstHorizLine;
	rect.right = m_iSecondVertLine; rect.bottom = m_iSecondHorizLine;
	DrawTextW(rDC.m_hDC, L"Offset", 6, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	for (unsigned i = 0; i < 16; i++)
		if (i <= 7) //Top offset numbers (0 1 2 3 4 5 6 7)
			ExtTextOutW(rDC.m_hDC, m_iIndentFirstHexChunk + m_sizeLetter.cx + (m_iIndentBetweenHexChunks*i),
				nScrollVert + (m_sizeLetter.cy / 6), NULL, nullptr, &m_strHexMap[i], 1, nullptr);
		else //Top offset numbers, part after 7 (8 9 A B C D E F)
			ExtTextOutW(rDC.m_hDC, m_iIndentFirstHexChunk + m_sizeLetter.cx + (m_iIndentBetweenHexChunks*i) + m_iIndentBetween78,
				nScrollVert + (m_sizeLetter.cy / 6), NULL, nullptr, &m_strHexMap[i], 1, nullptr);

	//"Ascii" text.
	rect.left = m_iThirdVertLine; rect.top = m_iFirstHorizLine;
	rect.right = m_iFourthVertLine; rect.bottom = m_iSecondHorizLine;
	DrawTextW(rDC.m_hDC, L"Ascii", 5, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	//First Vertical line.
	rDC.MoveTo(m_iFirstVertLine, nScrollVert);
	rDC.LineTo(m_iFirstVertLine, m_iFourthHorizLine);

	//Second Vertical line.
	rDC.MoveTo(m_iSecondVertLine, nScrollVert);
	rDC.LineTo(m_iSecondVertLine, m_iThirdHorizLine);

	//Third Vertical line.
	rDC.MoveTo(m_iThirdVertLine, nScrollVert);
	rDC.LineTo(m_iThirdVertLine, m_iThirdHorizLine);

	//Fourth Vertical line.
	rDC.MoveTo(m_iFourthVertLine, nScrollVert);
	rDC.LineTo(m_iFourthVertLine, m_iFourthHorizLine);

	int nLine { };

	for (unsigned iterLines = nLineStart; iterLines < nLineEnd; iterLines++)
	{
		swprintf_s(m_strOffset, 9, L"%08X", iterLines * 16);
		rDC.SetTextColor(m_clrTextOffset);

		//Drawing m_strOffset with bk color depending on selection range.
		if (m_fSelected && (iterLines * 16 + 16) > min(m_dwSelectionStart, m_dwSelectionEnd) &&
			(iterLines * 16) <= max(m_dwSelectionStart, m_dwSelectionEnd))
			rDC.SetBkColor(m_clrTextBkSelected);
		else
			rDC.SetBkColor(m_clrTextBk);

		//Left column offset print (00000001...0000FFFF...).
		ExtTextOutW(rDC.m_hDC, m_sizeLetter.cx, m_iHeightHeaderRect + (m_sizeLetter.cy * nLine + nScrollVert),
			NULL, nullptr, m_strOffset, 8, nullptr);
		rDC.SetTextColor(m_clrTextHex);

		int nIndentHexX { };
		int nIndentAsciiX { };
		int nIndent78 { };

		//Main loop for printing Hex chunks and Ascii chars (right column).
		for (int iterChunks = 0; iterChunks < 16; iterChunks++)
		{
			if (iterChunks > 7)
				nIndent78 = m_iIndentBetween78;

			const UINT nFirstHexPosToPrintX = m_iIndentFirstHexChunk + nIndentHexX + nIndent78;
			const UINT nFirstHexPosToPrintY = m_iHeightHeaderRect + m_sizeLetter.cy * nLine + nScrollVert;
			const UINT nAsciiPosToPrintX = m_iIndentAscii + nIndentAsciiX;
			const UINT nAsciiPosToPrintY = m_iHeightHeaderRect + m_sizeLetter.cy * nLine + nScrollVert;

			//Index of next char (in m_pRawData) to draw.
			const size_t nIndexDataToPrint = iterLines * 16 + iterChunks;

			//Rect of the space between HEX chunks, for proper selection drawing.
			m_rcSpaceBetweenHex.left = nFirstHexPosToPrintX + m_sizeLetter.cx * 2;
			m_rcSpaceBetweenHex.top = nFirstHexPosToPrintY;
			if (iterChunks == 7)
				m_rcSpaceBetweenHex.right = nFirstHexPosToPrintX + m_sizeLetter.cx * 5;
			else
				m_rcSpaceBetweenHex.right = nFirstHexPosToPrintX + m_sizeLetter.cx * 3;

			m_rcSpaceBetweenHex.bottom = nFirstHexPosToPrintY + m_sizeLetter.cy;

			if (nIndexDataToPrint < m_dwRawDataCount) //Draw until reaching the end of m_dwRawDataCount.
			{
				//HEX chunk to print.
				wchar_t strHexToPrint[2];

				strHexToPrint[0] = m_strHexMap[((const unsigned char)m_pRawData[nIndexDataToPrint] & 0xF0) >> 4];
				strHexToPrint[1] = m_strHexMap[((const unsigned char)m_pRawData[nIndexDataToPrint] & 0x0F)];

				//Selection draw with different BK color.
				if (m_fSelected && nIndexDataToPrint >= min(m_dwSelectionStart, m_dwSelectionEnd)
					&& nIndexDataToPrint <= max(m_dwSelectionStart, m_dwSelectionEnd))
				{
					rDC.SetBkColor(m_clrTextBkSelected);

					//To prevent change bk color after last selected HEX in a row, and very last HEX.
					if (nIndexDataToPrint != max(m_dwSelectionStart, m_dwSelectionEnd) && (nIndexDataToPrint + 1) % 16)
						FillRect(rDC.m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_brTextBkSelection.m_hObject);
					else
						FillRect(rDC.m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_brTextBk.m_hObject);
				}
				else
				{
					rDC.SetBkColor(m_clrTextBk);
					FillRect(rDC.m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_brTextBk.m_hObject);
				}

				//HEX chunk print.
				ExtTextOutW(rDC.m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, &strHexToPrint[0], 2, nullptr);

				//Ascii to print.
				char chAsciiToPrint = m_pRawData[nIndexDataToPrint];
				//For non printable ASCII, just print a dot.
				if (chAsciiToPrint < 32 || chAsciiToPrint == 127)
					chAsciiToPrint = '.';

				//Ascii print..
				ExtTextOutA(rDC.m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, &chAsciiToPrint, 1, nullptr);
			}
			else
			{	//Fill remaining chunks with blank spaces.
				rDC.SetBkColor(m_clrTextBk);
				ExtTextOutW(rDC.m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, L" ", 2, nullptr);
				ExtTextOutA(rDC.m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, "", 1, nullptr);
			}
			//Increasing indents for next print, for both - Hex and ASCII
			nIndentHexX += m_iIndentBetweenHexChunks;
			nIndentAsciiX += m_iIndentBetweenAscii;
		}
		nLine++;
	}
}

BOOL CHexCtrl::CHexView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

BOOL CHexCtrl::CHexView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == 'C' && GetKeyState(VK_CONTROL) < 0)
		CopyToClipboard(CLIPBOARD_COPY_AS_HEX_FORMATTED);

	return CScrollView::PreTranslateMessage(pMsg);
}

int CHexCtrl::CHexView::HitTest(LPPOINT pPoint)
{
	DWORD dwHexChunk;
	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_POS);
	SCROLLINFO stScrollX;
	GetScrollInfo(SB_HORZ, &stScrollX, SIF_POS);
	//To compensate horizontal scroll.
	pPoint->x += stScrollX.nPos;

	//Checking if cursor is within HEX chunks area.
	if ((pPoint->x >= m_iIndentFirstHexChunk) && (pPoint->x < m_iThirdVertLine)
		&& (pPoint->y >= m_iHeightHeaderRect) && pPoint->y <= m_iHeightWorkArea)
	{
		int tmp78;
		if (pPoint->x > m_iIndentFirstHexChunk + (m_iIndentBetweenHexChunks * 8))
			tmp78 = m_iIndentBetween78;
		else
			tmp78 = 0;

		//Calculate hit HEX chunk, taking into account scroll position and letter sizes.
		dwHexChunk = ((pPoint->x - m_iIndentFirstHexChunk - tmp78) / (m_sizeLetter.cx * 3)) +
			((pPoint->y - m_iHeightHeaderRect) / m_sizeLetter.cy) * 16 + ((m_stScrollInfo.nPos / m_sizeLetter.cy) * 16);
	}
	else if ((pPoint->x >= m_iIndentAscii) && (pPoint->x < m_iIndentAscii + m_iIndentBetweenAscii * 16) && (pPoint->y >= m_iHeightHeaderRect))
	{
		//Calculate hit Ascii symbol.
		dwHexChunk = ((pPoint->x - m_iIndentAscii) / (m_iIndentBetweenAscii)) +
			((pPoint->y - m_iHeightHeaderRect) / m_sizeLetter.cy) * 16 + ((m_stScrollInfo.nPos / m_sizeLetter.cy) * 16);
	}
	else
		dwHexChunk = -1;

	//If cursor is out of end-bound of HEX chunks or Ascii chars.
	if (dwHexChunk >= m_dwRawDataCount)
		dwHexChunk = -1;

	return dwHexChunk;
}

int CHexCtrl::CHexView::CopyToClipboard(UINT nType)
{
	if (m_fSelected)
	{
		const char* const strHexMap = "0123456789ABCDEF";
		char chHexToCopy[2];
		std::string strToClipboard { };
		const size_t dwSelectionLen = (max(m_dwSelectionEnd, m_dwSelectionStart) - min(m_dwSelectionEnd, m_dwSelectionStart)) + 1;
		DWORD dwSelectionStart = min(m_dwSelectionEnd, m_dwSelectionStart);
		DWORD dwSelectionEnd = max(m_dwSelectionEnd, m_dwSelectionStart);

		if (nType == CLIPBOARD_COPY_AS_HEX)
		{
			for (unsigned i = 0; i < dwSelectionLen; i++)
			{
				chHexToCopy[0] = strHexMap[((const unsigned char)m_pRawData[dwSelectionStart + i] & 0xF0) >> 4];
				chHexToCopy[1] = strHexMap[((const unsigned char)m_pRawData[dwSelectionStart + i] & 0x0F)];
				strToClipboard += chHexToCopy[0];
				strToClipboard += chHexToCopy[1];
			}
		}
		else if (nType == CLIPBOARD_COPY_AS_HEX_FORMATTED)
		{
			//How many spaces need to be inserted at the beginnig.
			DWORD dwModStart = dwSelectionStart % 16;
			//When to insert first \r\n.
			DWORD dwTail = 16 - dwModStart;

			//If at least two rows are selected.
			if (dwModStart + dwSelectionLen > 16)
			{
				strToClipboard.insert(0, dwModStart * 3, ' ');
				if (dwTail < 9)
					strToClipboard.insert(0, 2, ' ');
			}

			for (unsigned i = 0; i < dwSelectionLen; i++)
			{
				chHexToCopy[0] = strHexMap[((const unsigned char)m_pRawData[dwSelectionStart + i] & 0xF0) >> 4];
				chHexToCopy[1] = strHexMap[((const unsigned char)m_pRawData[dwSelectionStart + i] & 0x0F)];
				strToClipboard += chHexToCopy[0];
				strToClipboard += chHexToCopy[1];

				if (i < (dwSelectionLen - 1) && (dwTail - 1) != 0)
					if (dwTail == 9)
						strToClipboard += "   ";
					else
						strToClipboard += " ";
				if (--dwTail == 0 && i < (dwSelectionLen - 1))
				{
					strToClipboard += "\r\n";
					dwTail = 16;
				}
			}
		}
		else if (nType == CLIPBOARD_COPY_AS_ASCII)
		{
			char ch;
			for (unsigned i = 0; i < dwSelectionLen; i++)
			{
				ch = m_pRawData[dwSelectionStart + i];
				//If next byte is zero —> substitute it with space.
				if (ch == 0)
					ch = ' ';
				strToClipboard += ch;
			}
		}

		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strToClipboard.length() + 1);
		if (!hMem)
			return -1;
		LPVOID hMemLock = GlobalLock(hMem);
		if (!hMemLock)
			return -1;

		memcpy(hMemLock, strToClipboard.data(), strToClipboard.length() + 1);
		GlobalUnlock(hMem);
		OpenClipboard();
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
	return 1;
}

void CHexCtrl::CHexView::SetData(const std::vector<std::byte>& vecData)
{
	m_pRawData = (const unsigned char*)vecData.data();
	m_dwRawDataCount = vecData.size();
	m_fSelected = false;
	m_stScrollInfo.nPos = 0;
	SetScrollInfo(SB_VERT, &m_stScrollInfo);
	SetScrollInfo(SB_HORZ, &m_stScrollInfo);
	Recalc();
}

void CHexCtrl::CHexView::SetData(const std::string& strData)
{
	m_pRawData = (const unsigned char*)strData.data();
	m_dwRawDataCount = strData.length();
	m_fSelected = false;
	m_stScrollInfo.nPos = 0;
	SetScrollInfo(SB_VERT, &m_stScrollInfo);
	SetScrollInfo(SB_HORZ, &m_stScrollInfo);
	Recalc();
}

void CHexCtrl::CHexView::SetData(const PBYTE pData, DWORD_PTR dwCount)
{
	m_pRawData = pData;
	m_dwRawDataCount = dwCount;
	m_fSelected = false;
	m_stScrollInfo.nPos = 0;
	SetScrollInfo(SB_VERT, &m_stScrollInfo);
	SetScrollInfo(SB_HORZ, &m_stScrollInfo);
	Recalc();
}

void CHexCtrl::CHexView::ClearData()
{
	m_pRawData = nullptr;
	m_dwRawDataCount = 0;
	m_fSelected = false;
	m_stScrollInfo.nPos = 0;
	SetScrollInfo(SB_VERT, &m_stScrollInfo);
	SetScrollInfo(SB_HORZ, &m_stScrollInfo);
}

int CHexCtrl::CHexView::SetFont(CFont *pFontNew)
{
	//If pFontNew is nullptr then assigning default font.
	if (!pFontNew)
	{
		LOGFONT lf { };
		StringCchCopyW(lf.lfFaceName, 9, L"Consolas");
		lf.lfHeight = 18;
		m_fontHexView.DeleteObject();
		if (!m_fontHexView.CreateFontIndirectW(&lf))
		{
			StringCchCopyW(lf.lfFaceName, 16, L"Times New Roman");
			m_fontHexView.CreateFontIndirectW(&lf);
		}
	}
	else
	{
		LOGFONT lf { };
		pFontNew->GetLogFont(&lf);
		m_fontHexView.DeleteObject();
		m_fontHexView.CreateFontIndirectW(&lf);
	}
	Recalc();

	return 0;
}

void CHexCtrl::CHexView::SetFontSize(UINT nSize)
{
	//Prevent font size from being too small or too big.
	if (nSize < 7 || nSize > 75)
		return;

	LOGFONT lf;
	m_fontHexView.GetLogFont(&lf);
	lf.lfHeight = nSize;

	m_fontHexView.DeleteObject();
	m_fontHexView.CreateFontIndirectW(&lf);

	Recalc();
}

void CHexCtrl::CHexView::SetFontColor(COLORREF clrTextHex, COLORREF clrTextOffset,
	COLORREF clrTextSelected, COLORREF clrTextBk, COLORREF clrTextBkSelected)
{
	m_clrTextHex = clrTextHex;
	m_clrTextOffset = clrTextOffset;
	m_clrTextSelected = clrTextSelected;
	m_clrTextBk = clrTextBk;
	m_clrTextBkSelected;

	Invalidate();
	UpdateWindow();
}

UINT CHexCtrl::CHexView::GetFontSize()
{
	LOGFONT lf;
	m_fontHexView.GetLogFont(&lf);

	return lf.lfHeight;
}

void CHexCtrl::CHexView::Recalc()
{
	GetClientRect(&m_rcClient);

	UINT nStartLine { };
	if (m_fSecondLaunch)
	{
		GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);
		nStartLine = m_stScrollInfo.nPos / m_sizeLetter.cy;
	}

	CDC* pDC = GetDC();
	pDC->SelectObject(&m_fontHexView);
	GetTextExtentPoint32W(pDC->m_hDC, L"0", 1, &m_sizeLetter);
	ReleaseDC(pDC);

	m_iFirstVertLine = 0;
	m_iSecondVertLine = m_sizeLetter.cx * 10;
	m_iIndentBetweenHexChunks = m_sizeLetter.cx * 3;
	m_iIndentBetween78 = m_sizeLetter.cx * 2;
	m_iThirdVertLine = m_iSecondVertLine + (m_iIndentBetweenHexChunks * 16) + m_iIndentBetween78 + m_sizeLetter.cx;
	m_iIndentAscii = m_iThirdVertLine + m_sizeLetter.cx;
	m_iIndentBetweenAscii = m_sizeLetter.cx + 1;
	m_iFourthVertLine = m_iIndentAscii + (m_iIndentBetweenAscii * 16) + m_sizeLetter.cx;
	m_iIndentFirstHexChunk = m_iSecondVertLine + m_sizeLetter.cx;
	m_iHeightHeaderRect = int(m_sizeLetter.cy*1.5);
	m_iHeightWorkArea = m_rcClient.Height() - m_iHeightBottomRect - ((m_rcClient.Height() - m_iHeightHeaderRect - m_iHeightBottomRect) % m_sizeLetter.cy);

	//Scroll sizes according to current font size.
	SetScrollSizes(MM_TEXT, CSize(m_iFourthVertLine + 1,
		m_iHeightHeaderRect + m_iHeightBottomRect + (m_sizeLetter.cy * (m_dwRawDataCount / 16 + 3))));

	//This flag shows that Recalc() was invoked at least once before,
	//and ScrollSizes have already been set, so we can adjust them.
	if (m_fSecondLaunch)
	{
		GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);
		m_stScrollInfo.nPos = m_sizeLetter.cy * nStartLine;

		int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
		if (m_stScrollInfo.nPos > max)
			m_stScrollInfo.nPos = max;

		SetScrollInfo(SB_VERT, &m_stScrollInfo);
	}
	else
		m_fSecondLaunch = true;

	Invalidate();
}

/************************************************************************
* CHexCtrl implementation.												*
************************************************************************/
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

	CRuntimeClass* pNewViewClass = RUNTIME_CLASS(CHexCtrl::CHexView);
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