/************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/	*
* CSplitterEx class implementation.							*
************************************************************/
#include "stdafx.h"
#include "SplitterEx.h"

IMPLEMENT_DYNAMIC(CSplitterEx, CSplitterWndEx)

BEGIN_MESSAGE_MAP(CSplitterEx, CSplitterWndEx)
END_MESSAGE_MAP()

BOOL CSplitterEx::CreateStatic(CWnd * pParent, int nRows, int nCols, DWORD dwStyle, UINT nID)
{
	//If already created.
	if ((!m_vecRows.empty() && !m_vecCols.empty()) || !(nRows | nCols) || (nRows | nCols) > 16)
		return FALSE;

	for (int i = 0; i < nRows; i++)
		m_vecRows.emplace_back(true);
	for (int i = 0; i < nCols; i++)
		m_vecCols.emplace_back(true);

	return CSplitterWndEx::CreateStatic(pParent, nRows, nCols, dwStyle, nID);
}

BOOL CSplitterEx::CreateView(int row, int col, CRuntimeClass * pViewClass, SIZE sizeInit, CCreateContext * pContext)
{
	if (row >= m_vecRows.size() || col >= m_vecCols.size())
		return FALSE;

	BOOL ret = CSplitterWnd::CreateView(row, col, pViewClass, sizeInit, pContext);

	CWnd* pPane = GetPane(row, col);
	m_vecPanes.emplace_back(row, col, pPane);

	return ret;
}

bool CSplitterEx::AddNested(int row, int col, CWnd* pNested)
{
	for (auto& i : m_vecPanes)
		if (std::get<0>(i) == row && std::get<1>(i) == col)
		{
			i = { row, col, pNested };
			return true;
		}

	m_vecPanes.emplace_back(row, col, pNested);

	return true;
}

bool CSplitterEx::HideRow(UINT nRow)
{
	if (nRow < m_vecRows.size() && m_vecRows.at(nRow))
	{
		m_vecRows.at(nRow) = false;
		m_nRows--;
		RecalcPanes();

		return true;
	}

	return false;
}

bool CSplitterEx::ShowRow(UINT nRow)
{
	if (nRow < m_vecRows.size() && !m_vecRows.at(nRow))
	{
		m_vecRows.at(nRow) = true;
		m_nRows++;
		RecalcPanes();

		return true;
	}

	return false;
}

bool CSplitterEx::HideCol(UINT nCol)
{
	if (nCol < m_vecCols.size() && m_vecCols.at(nCol))
	{
		m_vecCols.at(nCol) = false;
		m_nCols--;
		RecalcPanes();

		return true;
	}

	return false;
}

bool CSplitterEx::ShowCol(UINT nCol)
{
	if (nCol < m_vecCols.size() && !m_vecCols.at(nCol))
	{
		m_vecCols.at(nCol) = true;
		m_nCols++;
		RecalcPanes();

		return true;
	}

	return false;
}

void CSplitterEx::RecalcPanes()
{
	//Populating temp vectors with visible cols/rows first,
	//then adding invisible cols/rows at the end.
	std::vector<int> vecColsOrdered { }, vecRowsOrdered { };
	{
		int iIndex { };
		for (auto i : m_vecCols) {
			if (i)
				vecColsOrdered.emplace_back(iIndex);
			iIndex++;
		}
		iIndex = 0;
		for (auto i : m_vecCols) {
			if (!i)
				vecColsOrdered.emplace_back(iIndex);
			iIndex++;
		}
		iIndex = 0;
		for (auto i : m_vecRows) {
			if (i)
				vecRowsOrdered.emplace_back(iIndex);
			iIndex++;
		}
		iIndex = 0;
		for (auto i : m_vecRows) {
			if (!i)
				vecRowsOrdered.emplace_back(iIndex);
			iIndex++;
		}
	}
	//Recalculating DlgCtrlId based on visible/hidden status.
	for (unsigned iterRow = 0; iterRow < m_vecRows.size(); iterRow++)
		for (unsigned iterCol = 0; iterCol < m_vecCols.size(); iterCol++)
			for (auto& i : m_vecPanes) //Search for exact pane.
				if (std::get<0>(i) == vecRowsOrdered.at(iterRow) && std::get<1>(i) == vecColsOrdered.at(iterCol))
				{
					CWnd *pPane = std::get<2>(i);
					pPane->SetDlgCtrlID(AFX_IDW_PANE_FIRST + iterRow * 16 + iterCol);

					if (!m_vecCols.at(iterCol) || !m_vecRows.at(iterRow))
						pPane->ShowWindow(SW_HIDE);
					else
						pPane->ShowWindow(SW_SHOW);
				}

	RecalcLayout();
}

void CSplitterEx::OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect)
{
	CSplitterWndEx::OnDrawSplitter(pDC, nType, rect);
}