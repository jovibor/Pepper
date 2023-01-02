/****************************************************************************************************
* Copyright © 2018-2023 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#include "stdafx.h"
#include "CSplitterEx.h"
#include <algorithm>

IMPLEMENT_DYNAMIC(CSplitterEx, CSplitterWndEx)

BEGIN_MESSAGE_MAP(CSplitterEx, CSplitterWndEx)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL CSplitterEx::CreateStatic(CWnd* m_pParent, int nRows, int nCols, DWORD dwStyle, UINT nID)
{
	//If already created.
	if ((!m_vecRows.empty() && !m_vecCols.empty()) || !(nRows | nCols) || (nRows | nCols) > 16)
		return FALSE;

	m_vecRows.resize(nRows, true);
	m_vecCols.resize(nCols, true);

	return CSplitterWndEx::CreateStatic(m_pParent, nRows, nCols, dwStyle, nID);
}

BOOL CSplitterEx::CreateView(int row, int col, CRuntimeClass* pViewClass, SIZE sizeInit, CCreateContext* pContext)
{
	if (row >= static_cast<int>(m_vecRows.size()) || col >= static_cast<int>(m_vecCols.size()))
		return FALSE;

	const auto ret = CSplitterWnd::CreateView(row, col, pViewClass, sizeInit, pContext);
	auto* const pPane = GetPane(row, col);
	m_vecPanes.emplace_back(row, col, pPane);

	return ret;
}

bool CSplitterEx::AddNested(int row, int col, CWnd* pNested)
{
	if (row >= static_cast<int>(m_vecRows.size()) || col >= static_cast<int>(m_vecCols.size()))
		return false;

	for (auto& iter : m_vecPanes) {
		if (iter.iRow == row && iter.iCol == col) {
			iter = { row, col, pNested };
			return true;
		}
	}

	m_vecPanes.emplace_back(row, col, pNested);

	return true;
}

bool CSplitterEx::HideRow(UINT nRow)
{
	if (nRow >= m_vecRows.size() || !m_vecRows[nRow])
		return false;

	m_vecRows[nRow] = false;
	--m_nRows;
	RecalcPanes();

	return true;
}

bool CSplitterEx::ShowRow(UINT nRow)
{
	if (nRow >= m_vecRows.size() || m_vecRows[nRow])
		return false;

	m_vecRows[nRow] = true;
	++m_nRows;
	RecalcPanes();

	return true;
}

bool CSplitterEx::HideCol(UINT nCol)
{
	if (nCol >= m_vecCols.size() || !m_vecCols[nCol])
		return false;

	m_vecCols[nCol] = false;
	--m_nCols;
	RecalcPanes();

	return true;
}

bool CSplitterEx::ShowCol(UINT nCol)
{
	if (nCol >= m_vecCols.size() || m_vecCols[nCol])
		return false;

	m_vecCols[nCol] = true;
	++m_nCols;
	RecalcPanes();

	return true;
}

void CSplitterEx::RecalcPanes()
{
	//Populating temp vectors with visible cols/rows first, then adding invisible cols/rows to the end.

	std::vector<int> vecColsOrdered;
	vecColsOrdered.reserve(m_vecCols.size());
	int iIndex { 0 };
	for (const auto iter : m_vecCols) {
		if (iter)
			vecColsOrdered.emplace_back(iIndex);
		++iIndex;
	}
	iIndex = 0;
	for (const auto iter : m_vecCols) {
		if (!iter)
			vecColsOrdered.emplace_back(iIndex);
		++iIndex;
	}

	std::vector<int> vecRowsOrdered;
	vecRowsOrdered.reserve(m_vecRows.size());
	iIndex = 0;
	for (const auto iter : m_vecRows) {
		if (iter)
			vecRowsOrdered.emplace_back(iIndex);
		++iIndex;
	}
	iIndex = 0;
	for (const auto iter : m_vecRows) {
		if (!iter)
			vecRowsOrdered.emplace_back(iIndex);
		++iIndex;
	}

	//Recalculating DlgCtrlId based on visible/hidden status.
	for (auto iterRow = 0U; iterRow < m_vecRows.size(); ++iterRow) {
		for (auto iterCol = 0U; iterCol < m_vecCols.size(); ++iterCol) {
			//Finding an exact pane.
			if (const auto iterPane = std::find_if(m_vecPanes.begin(), m_vecPanes.end(), [=](const SPANES& refData) { return refData.iRow == vecRowsOrdered[iterRow] && refData.iCol == vecColsOrdered[iterCol]; });
				iterPane != m_vecPanes.end()) {
				const auto pPane = iterPane->pPane;
				pPane->SetDlgCtrlID(AFX_IDW_PANE_FIRST + iterRow * 16 + iterCol);
				pPane->ShowWindow((!m_vecRows[iterPane->iRow] || !m_vecCols[iterPane->iCol]) ? SW_HIDE : SW_SHOW);
			}
		}
	}
	RecalcLayout();
}

void CSplitterEx::OnInvertTracker(const CRect& /*rect*/)
{
}

void CSplitterEx::OnMouseMove(UINT nFlags, CPoint pt)
{
	CSplitterWndEx::OnMouseMove(nFlags, pt);

	if (IsTracking()) {
		OnLButtonUp(0, pt);
		OnLButtonDown(0, pt);
	}
}