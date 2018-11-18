#include "stdafx.h"
#include "HexEditView.h"

IMPLEMENT_DYNCREATE(CHexEditView, CScrollView)

BEGIN_MESSAGE_MAP(CHexEditView, CScrollView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL CHexEditView::Create(CWnd * pParent, const RECT & rect, UINT nID, CCreateContext* pContext, CFont* pFont)
{
	SetFont(pFont);

	return CScrollView::Create(nullptr, nullptr, WS_VISIBLE | WS_CHILD, rect, pParent, nID, pContext);
}

void CHexEditView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
}

void CHexEditView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
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

void CHexEditView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	GetScrollInfo(SB_HORZ, &m_stScrollInfo, SIF_ALL);

	int pos = m_stScrollInfo.nPos;
	switch (nSBCode)
	{
	case SB_LEFT: pos = m_stScrollInfo.nMin; break;
	case SB_RIGHT: pos = m_stScrollInfo.nMax; break;
	case SB_LINELEFT: pos -= m_sizeLetter.cy; break;
	case SB_LINERIGHT: pos += m_sizeLetter.cy;  break;
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

void CHexEditView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_fLMousePressed)
	{
		const int tmpEnd = HitTest(&point);
		if (tmpEnd != -1) {
			m_dwSelectionEnd = tmpEnd;
			Invalidate();
		}
	}

	CScrollView::OnMouseMove(nFlags, point);
}

BOOL CHexEditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (nFlags == MK_CONTROL)
		SetFontSize(GetFontSize() + zDelta / WHEEL_DELTA * 2);

	Invalidate();

	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CHexEditView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_dwSelectionStart = m_dwSelectionEnd = HitTest(&point);

	if (m_dwSelectionStart != -1)
	{
		m_fLMousePressed = true;
		m_fSelection = true;
		Invalidate();
	}
}

void CHexEditView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_fLMousePressed = false;

	CScrollView::OnLButtonUp(nFlags, point);
}

void CHexEditView::OnRButtonUp(UINT nFlags, CPoint point)
{
	CScrollView::OnRButtonUp(nFlags, point);
}

void CHexEditView::OnMButtonDown(UINT nFlags, CPoint point)
{
}

void CHexEditView::OnSize(UINT nType, int cx, int cy)
{
	GetClientRect(&m_rcClient);

	CScrollView::OnSize(nType, cx, cy);
}

void CHexEditView::OnDraw(CDC* pDC)
{
	//Drawing through CMemDC, to avoid
	//any sort of flickering.
	CMemDC memDC(*pDC, this);
	CDC& rDC = memDC.GetDC();

	CRect clip;
	rDC.GetClipBox(&clip);
	rDC.FillSolidRect(clip, m_clrTextBk);

	rDC.SelectObject(&m_penLines);
	rDC.SelectObject(&m_fontHexView);

	//Need to properly Draw content.
	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_POS);

	//Find the nLineStart and nLineEnd posion, draw the visible portion.
	const UINT nLineStart = m_stScrollInfo.nPos / m_sizeLetter.cy;
	UINT nLineEnd = m_dwRawDataCount ? (nLineStart + (m_rcClient.Height() - m_nHeaderRectWidth - m_nBottomRectWidth) / m_sizeLetter.cy) : 0;
	if (nLineEnd > (m_dwRawDataCount / 16))
		nLineEnd = m_dwRawDataCount % 16 ? m_dwRawDataCount / 16 + 1 : m_dwRawDataCount / 16;

	//Horizontal lines coords depending on scroll position.
	m_nFirstHorizLine = m_stScrollInfo.nPos;
	m_nSecondHorizLine = m_nHeaderRectWidth - 1 + m_stScrollInfo.nPos;
	m_nThirdHorizLine = m_rcClient.Height() + m_stScrollInfo.nPos - m_nBottomRectWidth;
	m_nFourthHorizLine = m_rcClient.Height() + m_stScrollInfo.nPos - 3;

	//First horizontal line.
	rDC.MoveTo(0, m_nFirstHorizLine);
	rDC.LineTo(m_nFourthVertLine, m_nFirstHorizLine);

	//Second horizontal line.
	rDC.MoveTo(0, m_nSecondHorizLine);
	rDC.LineTo(m_nFourthVertLine, m_nSecondHorizLine);

	//Third horizontal line.
	rDC.MoveTo(0, m_nThirdHorizLine);
	rDC.LineTo(m_nFourthVertLine, m_nThirdHorizLine);

	//Fourth horizontal line.
	rDC.MoveTo(0, m_nFourthHorizLine);
	rDC.LineTo(m_nFourthVertLine, m_nFourthHorizLine);

	//"Offset" text.
	rDC.SetTextColor(m_clrTextOffset);
	RECT rect;
	rect.left = m_nFirstVertLine; rect.top = m_nFirstHorizLine;
	rect.right = m_nSecondVertLine; rect.bottom = m_nSecondHorizLine;
	DrawTextW(rDC.m_hDC, L"Offset", 6, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	for (unsigned i = 0; i < 16; i++)
		if (i <= 7) //Top offset numbers (0 1 2 3 4 5 6 7)
			ExtTextOutW(rDC.m_hDC, m_nIndentFirstHexChunk + m_sizeLetter.cx + (m_nIndentBetweenHexChunks*i),
				m_stScrollInfo.nPos + (m_sizeLetter.cy / 6), NULL, nullptr, &m_strHexMap[i], 1, nullptr);
		else //Top offset numbers, part after 7 (8 9 A B C D E F)
			ExtTextOutW(rDC.m_hDC, m_nIndentFirstHexChunk + m_sizeLetter.cx + (m_nIndentBetweenHexChunks*i) + m_nIndentBetween78,
				m_stScrollInfo.nPos + (m_sizeLetter.cy / 6), NULL, nullptr, &m_strHexMap[i], 1, nullptr);

	//"Ascii" text.
	rect.left = m_nThirdVertLine; rect.top = m_nFirstHorizLine;
	rect.right = m_nFourthVertLine; rect.bottom = m_nSecondHorizLine;
	DrawTextW(rDC.m_hDC, L"Ascii", 5, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	//First Vertical line.
	rDC.MoveTo(m_nFirstVertLine, m_stScrollInfo.nPos);
	rDC.LineTo(m_nFirstVertLine, m_nFourthHorizLine);

	//Second Vertical line.
	rDC.MoveTo(m_nSecondVertLine, m_stScrollInfo.nPos);
	rDC.LineTo(m_nSecondVertLine, m_nThirdHorizLine);

	//Third Vertical line.
	rDC.MoveTo(m_nThirdVertLine, m_stScrollInfo.nPos);
	rDC.LineTo(m_nThirdVertLine, m_nThirdHorizLine);

	//Fourth Vertical line.
	rDC.MoveTo(m_nFourthVertLine, m_stScrollInfo.nPos);
	rDC.LineTo(m_nFourthVertLine, m_nFourthHorizLine);

	int nLine { };

	//Ascii to print.
	char chAsciiToPrint { };
	//HEX chunk to print.
	wchar_t strHexToPrint[2] { };

	for (unsigned iterLines = nLineStart; iterLines < nLineEnd; iterLines++)
	{
		swprintf_s(m_strOffset, 9, L"%08X", iterLines * 16);
		rDC.SetTextColor(m_clrTextOffset);

		//Left column offset Print (00000001...000000A1...)
		ExtTextOutW(rDC.m_hDC, m_sizeLetter.cx, m_nHeaderRectWidth + (m_sizeLetter.cy * nLine + m_stScrollInfo.nPos),
			NULL, nullptr, m_strOffset, 8, nullptr);
		rDC.SetTextColor(m_clrTextHex);

		int nIndentHexX = 0;
		int nIndentAsciiX = 0;
		int nIndent78 = 0;

		//Main loop for printing Hex chunks and ASCII chars (right column).
		for (int iterChunks = 0; iterChunks < 16; iterChunks++)
		{
			if (iterChunks > 7)
				nIndent78 = m_nIndentBetween78;

			const UINT nFirstHexPosToPrintX = m_nIndentFirstHexChunk + nIndentHexX + nIndent78;
			const UINT nFirstHexPosToPrintY = m_nHeaderRectWidth + m_sizeLetter.cy * nLine + m_stScrollInfo.nPos;
			//	nSecondHexPosToPrintX = m_nSecondHex + nIndentHexX + nIndent78;
			//	nSecondHexPosToPrintY = m_nHeaderRectWidth + m_sizeLetter.cy*nLine + m_stScrollInfo.nPos;
			const UINT nAsciiPosToPrintX = m_nIndentAscii + nIndentAsciiX;
			const UINT nAsciiPosToPrintY = m_nHeaderRectWidth + m_sizeLetter.cy * nLine + m_stScrollInfo.nPos;

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
				strHexToPrint[0] = m_strHexMap[((const unsigned char)m_pRawData[nIndexDataToPrint] & 0xF0) >> 4];
				strHexToPrint[1] = m_strHexMap[((const unsigned char)m_pRawData[nIndexDataToPrint] & 0x0F)];

				//Selection draw with different BK color.
				if (m_fSelection && nIndexDataToPrint >= min(m_dwSelectionStart, m_dwSelectionEnd) && nIndexDataToPrint <= max(m_dwSelectionStart, m_dwSelectionEnd))
				{
					rDC.SetBkColor(m_clrTextBkSelected);

					//To prevent change bk color after last selected HEX chunk.
					if (nIndexDataToPrint != max(m_dwSelectionStart, m_dwSelectionEnd))
						FillRect(rDC.m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_brTextBkSelection.m_hObject);
					else
						FillRect(rDC.m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_brTextBkDefault.m_hObject);
				}
				else {
					rDC.SetBkColor(m_clrTextBk);
					FillRect(rDC.m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_brTextBkDefault.m_hObject);
				}

				//HEX chunk draw.
				ExtTextOutW(rDC.m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, &strHexToPrint[0], 2, nullptr);

				chAsciiToPrint = m_pRawData[nIndexDataToPrint];
				//For non printable ASCII, just print a dot.
				if (chAsciiToPrint < 32 || chAsciiToPrint == 127)
					chAsciiToPrint = '.';

				//ASCII draw.
				ExtTextOutA(rDC.m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, &chAsciiToPrint, 1, nullptr);
			}
			else
			{	//Fill remaining chunks with blank spaces.
				rDC.SetBkColor(m_clrTextBk);
				ExtTextOutW(rDC.m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, L" ", 2, nullptr);
				ExtTextOutA(rDC.m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, "", 1, nullptr);
			}
			//Increasing indents for next print, for both - Hex and ASCII
			nIndentHexX += m_nIndentBetweenHexChunks;
			nIndentAsciiX += m_nIndentBetweenAscii;
		}
		nLine++;
	}
}

BOOL CHexEditView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

int CHexEditView::HitTest(LPPOINT pPoint)
{
	DWORD nHexChunk;
	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_POS);

	//Checkig if cursor is within HEX chunks area.
	if ((pPoint->x >= m_nIndentFirstHexChunk) && (pPoint->x < m_nThirdVertLine) && (pPoint->y >= m_nHeaderRectWidth))
	{
		int tmp78;
		if (pPoint->x > m_nIndentFirstHexChunk + (m_nIndentBetweenHexChunks * 8))
			tmp78 = m_nIndentBetween78;
		else
			tmp78 = 0;

		nHexChunk = ((pPoint->x - m_nIndentFirstHexChunk - tmp78) / (m_sizeLetter.cx * 3)) +
			((pPoint->y - m_nHeaderRectWidth) / m_sizeLetter.cy) * 16 + ((m_stScrollInfo.nPos / m_sizeLetter.cy) * 16);
	}
	else if ((pPoint->x >= m_nIndentAscii) && (pPoint->x < m_nIndentAscii + m_nIndentBetweenAscii * 16) && (pPoint->y >= m_nHeaderRectWidth))
	{
		nHexChunk = ((pPoint->x - m_nIndentAscii) / (m_nIndentBetweenAscii)) +
			((pPoint->y - m_nHeaderRectWidth) / m_sizeLetter.cy) * 16 + ((m_stScrollInfo.nPos / m_sizeLetter.cy) * 16);
	}
	else
		nHexChunk = -1;

	//If cursor is out of end-bound of HEX chunks or ASCII chars.
	if (nHexChunk >= m_dwRawDataCount)
		nHexChunk = -1;

	return nHexChunk;
}

BOOL CHexEditView::SetData(const std::vector<std::byte> *pVecData)
{
	if (!pVecData)
	{
		m_dwRawDataCount = 0;
		m_pRawData = nullptr;
		return FALSE;
	}

	m_pRawData = (const unsigned char*)pVecData->data();
	m_dwRawDataCount = pVecData->size();
	Recalc();

	return TRUE;
}

BOOL CHexEditView::SetData(const std::string& strData)
{
	m_pRawData = (const unsigned char*)strData.data();
	m_dwRawDataCount = strData.length();
	Recalc();

	return TRUE;
}

int CHexEditView::SetFont(CFont *pFontNew)
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

void CHexEditView::SetFontSize(UINT nSize)
{
	//Prevent font size from being too small or too big.
	if (nSize < 7 || nSize > 80)
		return;

	LOGFONT lf { };
	m_fontHexView.GetLogFont(&lf);
	lf.lfHeight = nSize;

	m_fontHexView.DeleteObject();
	m_fontHexView.CreateFontIndirectW(&lf);

	Recalc();
}

void CHexEditView::SetFontColor(COLORREF clrTextHex, COLORREF clrTextOffset,
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

UINT CHexEditView::GetFontSize()
{
	LOGFONT lf;
	m_fontHexView.GetLogFont(&lf);

	return lf.lfHeight;
}

void CHexEditView::Recalc()
{
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

	m_nFirstVertLine = 0;
	m_nSecondVertLine = m_sizeLetter.cx * 10;
	m_nIndentBetweenHexChunks = m_sizeLetter.cx * 3;
	m_nIndentBetween78 = m_sizeLetter.cx * 2;
	m_nThirdVertLine = m_nSecondVertLine + (m_nIndentBetweenHexChunks * 16) + m_nIndentBetween78 + m_sizeLetter.cx;
	m_nIndentAscii = m_nThirdVertLine + m_sizeLetter.cx;
	m_nIndentBetweenAscii = m_sizeLetter.cx + 1;
	m_nFourthVertLine = m_nIndentAscii + (m_nIndentBetweenAscii * 16) + m_sizeLetter.cx;

	m_nIndentFirstHexChunk = m_nSecondVertLine + m_sizeLetter.cx;
	//	m_nSecondHex = m_nIndentFirstHexChunk + m_sizeLetter.cx;

	m_nHeaderRectWidth = int(m_sizeLetter.cy*1.5);

	//Scroll sizes according to current font size 
	//and whether m_dwRawDataCount/16 has a remainder (mod) or we have fullfilled row.
	SetScrollSizes(MM_TEXT, CSize(m_nFourthVertLine + 1,
		m_nHeaderRectWidth + m_nBottomRectWidth + (m_sizeLetter.cy * (m_dwRawDataCount % 16 ? m_dwRawDataCount / 16 + 2 : m_dwRawDataCount / 16 + 1))));

	//This flag shows that Recalc() was invoked at least second time,
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