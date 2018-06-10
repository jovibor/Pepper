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
END_MESSAGE_MAP()

CHexEditView::~CHexEditView()
{
	delete m_pFontHexViewDefault;
}

BOOL CHexEditView::CreateView(CWnd * pParent, const RECT & rect, UINT nID, CCreateContext* pContext, CFont* pFont)
{
	SetFont(pFont);
	Create(0, 0, WS_VISIBLE | WS_CHILD /*| WS_VSCROLL | WS_HSCROLL*/, CRect(0, 0, 0, 0), pParent, nID, pContext);

	return TRUE;
}

void CHexEditView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
}

BOOL CHexEditView::OnEraseBkgnd(CDC* pDC)
{
	if (m_fEraseBkgnd) {
		CScrollView::OnEraseBkgnd(pDC);
		m_fEraseBkgnd = false;
	}

	return TRUE;// CScrollView::OnEraseBkgnd(pDC);
}

BOOL CHexEditView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	return CScrollView::OnScrollBy(sizeScroll, bDoScroll);
}

void CHexEditView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);

	int pos = m_stScrollInfo.nPos;
	switch (nSBCode)
	{
	case SB_LEFT: pos = m_stScrollInfo.nMin; break;
	case SB_RIGHT: pos = m_stScrollInfo.nMax; break;
	case SB_LINEUP: pos -= m_sizeText.cy; break;
	case SB_LINEDOWN: pos += m_sizeText.cy;  break;
	case SB_PAGEUP: pos -= m_stScrollInfo.nPage; break;
	case SB_PAGEDOWN: pos += m_stScrollInfo.nPage; break;
	case SB_THUMBPOSITION: pos = m_stScrollInfo.nTrackPos; break;
	case SB_THUMBTRACK: pos = m_stScrollInfo.nTrackPos; break;
	}

	//make sure the new position is within range
	if (pos < m_stScrollInfo.nMin) pos = m_stScrollInfo.nMin;
	int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
	if (pos > max) pos = max;

	m_stScrollInfo.nPos = pos;
	SetScrollInfo(SB_VERT, &m_stScrollInfo);

	Invalidate();
}

void CHexEditView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);

	GetScrollInfo(SB_HORZ, &m_stScrollInfo, SIF_ALL);

	int pos = m_stScrollInfo.nPos;
	switch (nSBCode)
	{
	case SB_LEFT: pos = m_stScrollInfo.nMin; break;
	case SB_RIGHT: pos = m_stScrollInfo.nMax; break;
	case SB_LINELEFT: pos -= m_sizeText.cy; break;
	case SB_LINERIGHT: pos += m_sizeText.cy;  break;
	case SB_PAGELEFT: pos -= m_stScrollInfo.nPage; break;
	case SB_PAGERIGHT: pos += m_stScrollInfo.nPage; break;
	case SB_THUMBPOSITION: pos = m_stScrollInfo.nTrackPos; break;
	case SB_THUMBTRACK: pos = m_stScrollInfo.nTrackPos; break;
	}

	//make sure the new position is within range
	if (pos < m_stScrollInfo.nMin) pos = m_stScrollInfo.nMin;
	int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
	if (pos > max) pos = max;

	m_stScrollInfo.nPos = pos;
	SetScrollInfo(SB_HORZ, &m_stScrollInfo);
	m_fEraseBkgnd = true;

	Invalidate();
}

BOOL CHexEditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (nFlags == MK_CONTROL)
		SetFontSize(GetFontSize() + zDelta / WHEEL_DELTA * 2);

	Invalidate();

	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CHexEditView::OnMButtonDown(UINT nFlags, CPoint point)
{
}

void CHexEditView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);
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

	if (!pFont)
	{
		if (!m_pFontHexViewDefault)
			m_pFontHexViewDefault = new CFont();

		LOGFONT lf { };
		StringCchCopyW(lf.lfFaceName, 9, L"Consolas");
		lf.lfHeight = 18;
		if (!m_pFontHexViewDefault->CreateFontIndirectW(&lf))
		{
			StringCchCopyW(lf.lfFaceName, 16, L"Times New Roman");
			m_pFontHexViewDefault->CreateFontIndirectW(&lf);
		}
		m_pFontHexView = m_pFontHexViewDefault;
	}
	else
		m_pFontHexView = pFont;

	Recalc();

	return pOldFont;
}

void CHexEditView::SetFontSize(UINT nSize)
{
	if (nSize < 7 || nSize>80 || !m_pFontHexView)
		return;

	LOGFONT lf { };
	m_pFontHexView->GetLogFont(&lf);
	lf.lfHeight = nSize;

	m_pFontHexView->DeleteObject();
	m_pFontHexView->CreateFontIndirectW(&lf);

	Recalc();
}

UINT CHexEditView::GetFontSize()
{
	if (!m_pFontHexView)
		return 0;

	LOGFONT logFont { };
	m_pFontHexView->GetLogFont(&logFont);

	return logFont.lfHeight;
}

void CHexEditView::OnDraw(CDC* pDC)
{
	GetClientRect(&m_rectClient);

	pDC->SelectObject(&m_penLines);
	pDC->SelectObject(m_pFontHexView);

	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_POS);

	//find the nLineStart and nLineEnd posion, print the visible portion
	UINT nLineStart = m_stScrollInfo.nPos / m_sizeText.cy;
	UINT nLineEnd = nLineStart + (m_rectClient.Height() - m_nTopHeaderWidth - m_nBottomRectWidth) / m_sizeText.cy;
	if (m_dwRawDataCount == 0)
		nLineEnd = 0;
	else if (nLineEnd > m_dwRawDataCount / 16 + 1)
		nLineEnd = m_dwRawDataCount / 16 + 1;

	//Horizontal lines coords depending on scroll position
	m_nFirstHorizLine = m_stScrollInfo.nPos;
	m_nSecondHorizLine = m_nTopHeaderWidth - 1 + m_stScrollInfo.nPos;
	m_nThirdHorizLine = m_rectClient.Height() + m_stScrollInfo.nPos - m_nBottomRectWidth;
	m_nFourthHorizLine = m_rectClient.Height() + m_stScrollInfo.nPos - 3;

	//First horizontal line
	pDC->MoveTo(0, m_nFirstHorizLine);
	pDC->LineTo(m_nFourthVertLine, m_nFirstHorizLine);

	//Second horizontal line
	pDC->MoveTo(0, m_nSecondHorizLine);
	pDC->LineTo(m_nFourthVertLine, m_nSecondHorizLine);

	//Third horizontal line
	pDC->MoveTo(0, m_nThirdHorizLine);
	pDC->LineTo(m_nFourthVertLine, m_nThirdHorizLine);

	//Fourth horizontal line
	pDC->MoveTo(0, m_nFourthHorizLine);
	pDC->LineTo(m_nFourthVertLine, m_nFourthHorizLine);

	pDC->SetTextColor(m_colorTextOffset);
	//"Offset" text
	ExtTextOutW(pDC->m_hDC, m_sizeText.cx / 6, m_stScrollInfo.nPos + (m_sizeText.cy / 6), NULL, nullptr, L"Offset", 6, 0);

	//"Ascii" text
	ExtTextOutW(pDC->m_hDC, m_nOffsetAscii + m_nIndentBetweenAscii * 6, m_stScrollInfo.nPos + (m_sizeText.cy / 6), NULL, nullptr, L"Ascii", 5, 0);

	for (unsigned i = 0; i < 16; i++)
		if (i > 7)//Top Offset text
			ExtTextOutW(pDC->m_hDC, m_nFirstHexChunkIndent + m_nLetterWidth + (m_nIndentBetweenHex*i) + m_nIndentBetween78, m_stScrollInfo.nPos + (m_sizeText.cy / 6),
				NULL, nullptr, &m_strHexMap[i], 1, nullptr);
		else//Top Offset text, part after 7
			ExtTextOutW(pDC->m_hDC, m_nFirstHexChunkIndent + m_nLetterWidth + (m_nIndentBetweenHex*i), m_stScrollInfo.nPos + (m_sizeText.cy / 6),
				NULL, nullptr, &m_strHexMap[i], 1, nullptr);

	//First Vertical line
	pDC->MoveTo(m_nFirstVertLine, m_stScrollInfo.nPos);
	pDC->LineTo(m_nFirstVertLine, m_nFourthHorizLine);

	//Second Vertical line
	pDC->MoveTo(m_nSecondVertLine, m_stScrollInfo.nPos);
	pDC->LineTo(m_nSecondVertLine, m_nThirdHorizLine);

	//Third Vertical line
	pDC->MoveTo(m_nThirdVertLine, m_stScrollInfo.nPos);
	pDC->LineTo(m_nThirdVertLine, m_nThirdHorizLine);

	//Fourth Vertical line
	pDC->MoveTo(m_nFourthVertLine, m_stScrollInfo.nPos);
	pDC->LineTo(m_nFourthVertLine, m_nFourthHorizLine);

	int nIndentHexX = 0, nIndentAsciiX = 0, nLine = 0, nIndent78 = 0;
	UINT nFirstHexPosToPrintX { }, nFirstHexPosToPrintY { };
	//	UINT nSecondHexPosToPrintX { }, nSecondHexPosToPrintY { };
	UINT nAsciiPosToPrintX { }, nAsciiPosToPrintY { };

	//Index of byte in m_pRawData to print
	size_t nIndexDataToPrint { };
	
	//Ascii to print
	char chAsciiToPrint { };
	//HEX chunk to print
	wchar_t strHexToPrint[2] { };

	for (unsigned int i = nLineStart; i < nLineEnd; i++)
	{
		swprintf_s(m_strOffset, 9, L"%08X", i * 16);
		pDC->SetTextColor(m_colorTextOffset);
		
		//Left column offset Print (00000001...000000A1...)
		ExtTextOutW(pDC->m_hDC, m_nLetterWidth, m_nTopHeaderWidth + (m_sizeText.cy*nLine + m_stScrollInfo.nPos), NULL, nullptr, m_strOffset, 8, nullptr);
		pDC->SetTextColor(m_colorTextHex);

		nIndentHexX = 0;
		nIndentAsciiX = 0;
		nIndent78 = 0;
		
		//Main loop to print Hex chunks and Ascii chars (right column)
		for (int j = 0; j < 16; j++)
		{
			if (j > 7)
				nIndent78 = m_nIndentBetween78;

			nFirstHexPosToPrintX = m_nFirstHexChunkIndent + nIndentHexX + nIndent78;
			nFirstHexPosToPrintY = m_nTopHeaderWidth + m_sizeText.cy*nLine + m_stScrollInfo.nPos;
			//	nSecondHexPosToPrintX = m_nSecondHex + nIndentHexX + nIndent78;
			//	nSecondHexPosToPrintY = m_nTopHeaderWidth + m_sizeText.cy*nLine + m_stScrollInfo.nPos;
			nAsciiPosToPrintX = m_nOffsetAscii + nIndentAsciiX;
			nAsciiPosToPrintY = m_nTopHeaderWidth + m_sizeText.cy*nLine + m_stScrollInfo.nPos;

			//Index of next char (in m_pRawData) to print.
			nIndexDataToPrint = i * 16 + j;

			if (nIndexDataToPrint < m_dwRawDataCount)
			{
				strHexToPrint[0] = m_strHexMap[((const unsigned char)m_pRawData[nIndexDataToPrint] & 0xF0) >> 4];
				strHexToPrint[1] = m_strHexMap[((const unsigned char)m_pRawData[nIndexDataToPrint] & 0x0F)];

				//HEX chunk print.
				ExtTextOutW(pDC->m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, &strHexToPrint[0], 2, nullptr);

				chAsciiToPrint = m_pRawData[nIndexDataToPrint];
				//for non printable Ascii
				if (chAsciiToPrint < 32 || chAsciiToPrint == 127)
					chAsciiToPrint = '.';
				
				//Ascii Print
				ExtTextOutA(pDC->m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, &chAsciiToPrint, 1, nullptr);
			}
			else
			{
				ExtTextOutW(pDC->m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, L" ", 2, nullptr);
				ExtTextOutA(pDC->m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, "", 1, nullptr);
			}
			//Increasing indents for nex print for both - Hex and Ascii
			nIndentHexX += m_nIndentBetweenHex;
			nIndentAsciiX += m_nIndentBetweenAscii;
		}
		nLine++;
	}
}

void CHexEditView::Recalc()
{
	UINT nLineStart { };
	if (m_fSecondLaunch)
	{
		GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);
		nLineStart = m_stScrollInfo.nPos / m_sizeText.cy;
	}

	CDC* pDC = GetDC();
	CFont* oldFont = pDC->SelectObject(m_pFontHexView);
	GetTextExtentPoint32W(pDC->m_hDC, L"00000000", 8, &m_sizeText);
	pDC->SelectObject(oldFont);
	ReleaseDC(pDC);

	m_nLetterWidth = m_sizeText.cx / 8;
	m_nFirstVertLine = 0;
	m_nSecondVertLine = m_sizeText.cx + m_sizeText.cx / 4;
	m_nIndentBetweenHex = m_sizeText.cx / 2.5;
	m_nIndentBetween78 = m_sizeText.cx / 4;
	m_nThirdVertLine = m_nSecondVertLine + (m_nIndentBetweenHex * 16) + m_nIndentBetween78 + m_nLetterWidth;

	m_nOffsetAscii = m_nThirdVertLine + m_nLetterWidth;
	m_nIndentBetweenAscii = m_nLetterWidth + 1;
	m_nFourthVertLine = m_nOffsetAscii + (m_nIndentBetweenAscii * 16) + m_nLetterWidth;

	m_nFirstHexChunkIndent = m_nSecondVertLine + m_nLetterWidth;
	//	m_nSecondHex = m_nFirstHexChunkIndent + m_nLetterWidth;

	m_nTopHeaderWidth = m_sizeText.cy*1.5;

	//Scroll sizes according to current font size
	CSize size(m_sizeText.cx, m_sizeText.cy);
	SetScrollSizes(MM_TEXT, CSize(m_nFourthVertLine + 1, m_nTopHeaderWidth + m_nBottomRectWidth + (m_sizeText.cy * ((m_dwRawDataCount / 16) + 2))),
		size, size);

	if (m_fSecondLaunch)//First launch? Do we need to ajust scroll bars?
	{
		GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);
		m_stScrollInfo.nPos = m_sizeText.cy * nLineStart;
		int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
		if (m_stScrollInfo.nPos > max)
			m_stScrollInfo.nPos = max;

		SetScrollInfo(SB_VERT, &m_stScrollInfo, TRUE);
	}
	else
		m_fSecondLaunch = true;

	m_fEraseBkgnd = true;

	Invalidate();
}