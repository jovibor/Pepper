#include "stdafx.h"
#include "SplitterEx.h"

IMPLEMENT_DYNAMIC(CSplitterEx, CSplitterWndEx)

BEGIN_MESSAGE_MAP(CSplitterEx, CSplitterWndEx)
END_MESSAGE_MAP()

BOOL CSplitterEx::CreateStatic(CWnd * pParent, int nRows, int nCols, DWORD dwStyle, UINT nID)
{
	//If already created.
	if ((!m_vecRows.empty() && !m_vecCols.empty()) || (!nRows | !nCols))
		return FALSE;

	for (int i = 0; i < nRows; i++)
		m_vecRows.emplace_back(i, true);
	for (int i = 0; i < nCols; i++)
		m_vecCols.emplace_back(i, true);

	return CSplitterWndEx::CreateStatic(pParent, nRows, nCols, dwStyle, nID);
}

int CSplitterEx::HideRow(int nRow)
{
	return 0;
}

int CSplitterEx::HideCol(int nCol)
{
	return 0;
}

int CSplitterEx::ShowRow(int nRow)
{
	return 0;
}

int CSplitterEx::ShowCol(int nCol)
{
	return 0;
}

void CSplitterEx::OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect)
{
	CSplitterWndEx::OnDrawSplitter(pDC, nType, rect);
}