/********************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/						*
* Extended CSplitterWnd class with the ability to Hide/Show individual			*
* rows and columns.																*
* Has five additional methods: 1.HideRow() 2. ShowRow()							*
* 3. HideCol(), 4. ShowCol() 5. AddNested().									*
* Public method AddNested() is used when dealing with nested splitters.			*
* It's needed to properly handle Hide/Show row/cols methods on splitters		*
* without generic views, i.e. splitters that is used as host for other, nested	*
* splitters. Below is an example:												*																		
* CSplitterEx mainSpl, leftSpl, rightSpl;										*
* mainSpl.CreateStatic(this, 1, 2);												*
* leftSpl.CreateStatic(&mainSpl, 1, 2, dwStyle, mainSpl.IdFromRowCol(0, 0));	*
* rightSpl.CreateStatic(&mainSpl, 1, 2, dwStyle, mainSpl.IdFromRowCol(0, 1));	*
* mainSpl.AddNested(0, 0, &leftSpl); mainSpl.AddNested(0, 1, &rightSpl);		*
********************************************************************************/
#pragma once

class CSplitterEx : public CSplitterWndEx
{
public:
	DECLARE_DYNAMIC(CSplitterEx)
	CSplitterEx() {}
	virtual ~CSplitterEx() {}
	BOOL CreateStatic(CWnd* pParent, int nRows, int nCols, DWORD dwStyle = WS_CHILD | WS_VISIBLE, UINT nID = AFX_IDW_PANE_FIRST) override;
	BOOL CreateView(int row, int col, CRuntimeClass* pViewClass, SIZE sizeInit, CCreateContext* pContext);
	bool AddNested(int row, int col, CWnd* pNested);
	bool HideRow(UINT nRow);
	bool HideCol(UINT nCol);
	bool ShowRow(UINT nRow);
	bool ShowCol(UINT nCol);
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
	DECLARE_MESSAGE_MAP()
private:
	void RecalcPanes();
	//Vector of tuple of row/col: is visible?
	std::vector<std::tuple<int, bool>> m_vecRows { };
	std::vector<std::tuple<int, bool>> m_vecCols { };
	//row, column ,pPane.
	std::vector<std::tuple<int, int, CWnd*>> m_vecPanes { };
};