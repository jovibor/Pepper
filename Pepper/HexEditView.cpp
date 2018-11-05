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

CHexEditView::~CHexEditView()
{
	delete m_pFontDefaultHexView;
}

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
	CRect clip;
	memDC.GetDC().GetClipBox(&clip);
	memDC.GetDC().FillSolidRect(clip, GetSysColor(COLOR_WINDOW));

	memDC.GetDC().SelectObject(&m_penLines);
	memDC.GetDC().SelectObject(m_pFontHexView);

	//Need to properly Draw content.
	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_POS);

	//Find the nStartLine and nLineEnd posion, draw the visible portion.
	const UINT nLineStart = m_stScrollInfo.nPos / m_sizeLetter.cy;
	UINT nLineEnd = nLineStart + (m_rcClient.Height() - m_nTopHeaderWidth - m_nBottomRectWidth) / m_sizeLetter.cy;
	if (m_dwRawDataCount == 0)
		nLineEnd = 0;
	else if (nLineEnd > (m_dwRawDataCount / 16 + 1))
		nLineEnd = m_dwRawDataCount / 16 + 1;

	//Horizontal lines coords depending on scroll position.
	m_nFirstHorizLine = m_stScrollInfo.nPos;
	m_nSecondHorizLine = m_nTopHeaderWidth - 1 + m_stScrollInfo.nPos;
	m_nThirdHorizLine = m_rcClient.Height() + m_stScrollInfo.nPos - m_nBottomRectWidth;
	m_nFourthHorizLine = m_rcClient.Height() + m_stScrollInfo.nPos - 3;

	//First horizontal line.
	memDC.GetDC().MoveTo(0, m_nFirstHorizLine);
	memDC.GetDC().LineTo(m_nFourthVertLine, m_nFirstHorizLine);

	//Second horizontal line.
	memDC.GetDC().MoveTo(0, m_nSecondHorizLine);
	memDC.GetDC().LineTo(m_nFourthVertLine, m_nSecondHorizLine);

	//Third horizontal line.
	memDC.GetDC().MoveTo(0, m_nThirdHorizLine);
	memDC.GetDC().LineTo(m_nFourthVertLine, m_nThirdHorizLine);

	//Fourth horizontal line.
	memDC.GetDC().MoveTo(0, m_nFourthHorizLine);
	memDC.GetDC().LineTo(m_nFourthVertLine, m_nFourthHorizLine);

	//"Offset" text.
	memDC.GetDC().SetTextColor(m_clrTextOffset);
	RECT rect;
	rect.left = m_nFirstVertLine; rect.top = m_nFirstHorizLine;
	rect.right = m_nSecondVertLine; rect.bottom = m_nSecondHorizLine;
	DrawTextW(memDC.GetDC().m_hDC, L"Offset", 6, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	//"ASCII" text.
	rect.left = m_nThirdVertLine; rect.top = m_nFirstHorizLine;
	rect.right = m_nFourthVertLine; rect.bottom = m_nSecondHorizLine;
	DrawTextW(memDC.GetDC().m_hDC, L"ASCII", 5, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	for (unsigned i = 0; i < 16; i++)
		if (i <= 7) //Top offset text (0 1 2 3 4 5 6 7)
			ExtTextOutW(memDC.GetDC().m_hDC, m_nIndentFirstHexChunk + m_sizeLetter.cx + (m_nIndentBetweenHexChunks*i),
				m_stScrollInfo.nPos + (m_sizeLetter.cy / 6), NULL, nullptr, &m_strHexMap[i], 1, nullptr);
		else //Top Offset text, part after 7 (8 9 A B C D E F)
			ExtTextOutW(memDC.GetDC().m_hDC, m_nIndentFirstHexChunk + m_sizeLetter.cx + (m_nIndentBetweenHexChunks*i) + m_nIndentBetween78,
				m_stScrollInfo.nPos + (m_sizeLetter.cy / 6), NULL, nullptr, &m_strHexMap[i], 1, nullptr);

	//First Vertical line.
	memDC.GetDC().MoveTo(m_nFirstVertLine, m_stScrollInfo.nPos);
	memDC.GetDC().LineTo(m_nFirstVertLine, m_nFourthHorizLine);

	//Second Vertical line.
	memDC.GetDC().MoveTo(m_nSecondVertLine, m_stScrollInfo.nPos);
	memDC.GetDC().LineTo(m_nSecondVertLine, m_nThirdHorizLine);

	//Third Vertical line.
	memDC.GetDC().MoveTo(m_nThirdVertLine, m_stScrollInfo.nPos);
	memDC.GetDC().LineTo(m_nThirdVertLine, m_nThirdHorizLine);

	//Fourth Vertical line.
	memDC.GetDC().MoveTo(m_nFourthVertLine, m_stScrollInfo.nPos);
	memDC.GetDC().LineTo(m_nFourthVertLine, m_nFourthHorizLine);

	int nLine { };

	//Ascii to print.
	char chAsciiToPrint { };
	//HEX chunk to print.
	wchar_t strHexToPrint[2] { };

	for (unsigned iterLines = nLineStart; iterLines < nLineEnd; iterLines++)
	{
		swprintf_s(m_strOffset, 9, L"%08X", iterLines * 16);
		memDC.GetDC().SetTextColor(m_clrTextOffset);

		//Left column offset Print (00000001...000000A1...)
		ExtTextOutW(memDC.GetDC().m_hDC, m_sizeLetter.cx, m_nTopHeaderWidth + (m_sizeLetter.cy * nLine + m_stScrollInfo.nPos), NULL, nullptr, m_strOffset, 8, nullptr);
		memDC.GetDC().SetTextColor(m_clrTextHex);

		int nIndentHexX = 0;
		int nIndentAsciiX = 0;
		int nIndent78 = 0;

		//Main loop for printing Hex chunks and ASCII chars (right column).
		for (int iterChunks = 0; iterChunks < 16; iterChunks++)
		{
			if (iterChunks > 7)
				nIndent78 = m_nIndentBetween78;

			const UINT nFirstHexPosToPrintX = m_nIndentFirstHexChunk + nIndentHexX + nIndent78;
			const UINT nFirstHexPosToPrintY = m_nTopHeaderWidth + m_sizeLetter.cy * nLine + m_stScrollInfo.nPos;
			//	nSecondHexPosToPrintX = m_nSecondHex + nIndentHexX + nIndent78;
			//	nSecondHexPosToPrintY = m_nTopHeaderWidth + m_sizeLetter.cy*nLine + m_stScrollInfo.nPos;
			const UINT nAsciiPosToPrintX = m_nIndentAscii + nIndentAsciiX;
			const UINT nAsciiPosToPrintY = m_nTopHeaderWidth + m_sizeLetter.cy * nLine + m_stScrollInfo.nPos;

			//Index of next char (in m_pRawData) to draw.
			const size_t nIndexDataToPrint = iterLines * 16 + iterChunks;

			//Rect of the Space between HEX chunks, for proper selection drawing.
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
					memDC.GetDC().SetBkColor(m_clrTextBkSelection);

					//To prevent change bk color after last selected HEX chunk.
					if (nIndexDataToPrint != max(m_dwSelectionStart, m_dwSelectionEnd))
						FillRect(memDC.GetDC().m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_brTextBkSelection.m_hObject);
					else
						FillRect(memDC.GetDC().m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_brTextBkDefault.m_hObject);
				}
				else {
					memDC.GetDC().SetBkColor(m_clrTextBkDefault);
					FillRect(memDC.GetDC().m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_brTextBkDefault.m_hObject);
				}

				//HEX chunk draw.
				ExtTextOutW(memDC.GetDC().m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, &strHexToPrint[0], 2, nullptr);

				chAsciiToPrint = m_pRawData[nIndexDataToPrint];
				//For non printable ASCII, just print a dot.
				if (chAsciiToPrint < 32 || chAsciiToPrint == 127)
					chAsciiToPrint = '.';

				//ASCII draw.
				ExtTextOutA(memDC.GetDC().m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, &chAsciiToPrint, 1, nullptr);
			}
			else
			{	//Fill remaining chunks with blank spaces.
				memDC.GetDC().SetBkColor(m_clrTextBkDefault);
				ExtTextOutW(memDC.GetDC().m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, L" ", 2, nullptr);
				ExtTextOutA(memDC.GetDC().m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, "", 1, nullptr);
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
	if ((pPoint->x >= m_nIndentFirstHexChunk) && (pPoint->x < m_nThirdVertLine) && (pPoint->y >= m_nTopHeaderWidth))
	{
		int tmp78;
		if (pPoint->x > m_nIndentFirstHexChunk + (m_nIndentBetweenHexChunks * 8))
			tmp78 = m_nIndentBetween78;
		else
			tmp78 = 0;

		nHexChunk = ((pPoint->x - m_nIndentFirstHexChunk - tmp78) / (m_sizeLetter.cx * 3)) +
			((pPoint->y - m_nTopHeaderWidth) / m_sizeLetter.cy) * 16 + ((m_stScrollInfo.nPos / m_sizeLetter.cy) * 16);
	}
	else if ((pPoint->x >= m_nIndentAscii) && (pPoint->x < m_nIndentAscii + m_nIndentBetweenAscii * 16) && (pPoint->y >= m_nTopHeaderWidth))
	{
		nHexChunk = ((pPoint->x - m_nIndentAscii) / (m_nIndentBetweenAscii)) +
			((pPoint->y - m_nTopHeaderWidth) / m_sizeLetter.cy) * 16 + ((m_stScrollInfo.nPos / m_sizeLetter.cy) * 16);
	}
	else
		nHexChunk = -1;

	//If cursor is out of end-bound of HEX chunks or ASCII chars.
	if (nHexChunk >= m_dwRawDataCount)
		nHexChunk = -1;

	return nHexChunk;
}

BOOL CHexEditView::SetData(const std::vector<std::byte> *vecData)
{
	if (!vecData)
		return FALSE;

	m_pRawData = (const unsigned char*)vecData->data();
	m_dwRawDataCount = vecData->size();
	Recalc();

	return TRUE;
}

BOOL CHexEditView::SetData(const std::string_view strData, UINT nCount)
{
	m_pRawData = (const unsigned char*)strData.data();

	if (nCount > 0)
		m_dwRawDataCount = nCount;
	else
		m_dwRawDataCount = strData.length();

	Recalc();

	return 0;
}

CFont* CHexEditView::SetFont(CFont *pFont)
{
	CFont* pOldFont = m_pFontHexView;

	//If pFont is nullptr then assigning default font.
	if (!pFont)
	{
		if (!m_pFontDefaultHexView)
		{
			m_pFontDefaultHexView = new CFont();
			LOGFONT lf { };
			StringCchCopyW(lf.lfFaceName, 9, L"Consolas");
			lf.lfHeight = 18;
			if (!m_pFontDefaultHexView->CreateFontIndirectW(&lf))
			{
				StringCchCopyW(lf.lfFaceName, 16, L"Times New Roman");
				m_pFontDefaultHexView->CreateFontIndirectW(&lf);
			}
		}
		m_pFontHexView = m_pFontDefaultHexView;
	}
	else
		m_pFontHexView = pFont;

	Recalc();

	return pOldFont;
}

void CHexEditView::SetFontSize(UINT nSize)
{
	//Prevent font size being too small or too big.
	if (nSize < 7 || nSize > 80 || !m_pFontHexView)
		return;

	LOGFONT lf { };
	m_pFontHexView->GetLogFont(&lf);
	lf.lfHeight = nSize;

	m_pFontHexView->DeleteObject();
	m_pFontHexView->CreateFontIndirectW(&lf);

	Recalc();
}

void CHexEditView::SetFontColor(COLORREF clrHex, COLORREF clrOffset)
{
	m_clrTextHex = clrHex;

	if (clrOffset)/*if zero use default*/
		m_clrTextOffset = clrOffset;
}

UINT CHexEditView::GetFontSize() const
{
	if (!m_pFontHexView)
		return 0;

	LOGFONT logFont { };
	m_pFontHexView->GetLogFont(&logFont);

	return logFont.lfHeight;
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
	CFont* oldFont = pDC->SelectObject(m_pFontHexView);
	GetTextExtentPoint32W(pDC->m_hDC, L"0", 1, &m_sizeLetter);
	pDC->SelectObject(oldFont);
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

	m_nTopHeaderWidth = int(m_sizeLetter.cy*1.5);

	//Scroll sizes according to current font size.
	SetScrollSizes(MM_TEXT,
		CSize(m_nFourthVertLine + 1, m_nTopHeaderWidth + m_nBottomRectWidth + (m_sizeLetter.cy * ((m_dwRawDataCount / 16) + 2))));

	if (m_fSecondLaunch) //First launch? Do we need to ajust scroll bars?
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