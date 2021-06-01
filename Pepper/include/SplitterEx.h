/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
/**************************************************************************************
* Extended CSplitterWndEx class with the ability to Hide/Show individual              *
* rows and columns.	It has five additional methods:                                   *
* 1. HideRow() 2. ShowRow() 3. HideCol(), 4. ShowCol() 5. AddNested().                *
* The method AddNested() is used when dealing with nested splitters.		          *
* It's needed to properly handle Hide/Show rows/cols methods on splitters	          *
* without generic views, i.e. splitters that is used as a hosts for other,            *
* nested, splitters.														          *
* Example of creating four views splitted horizontally and vertically:                *
*	CSplitterEx horzSplit, topSplit, botSplit;									      *
*	horzSplit.CreateStatic(this, 2, 1); //Two rows, one column.                       *
*	topSplit.CreateStatic(&horzSplit, 1, 2, dwStyle, horzSplit.IdFromRowCol(0, 0));   *
*	topSplit.CreateView(0, 0, ...) //Create top-left view.                            *
*	topSplit.CreateView(0, 1, ...) //Create top-right view.                           *
*	horzSplit.AddNested(0, 0, &topSplit); //Adding topSplit as nested to horz(0, 0).  *
*   botSplit.CreateStatic(&horzSplit, 1, 2, dwStyle, horzSplit.IdFromRowCol(1, 0));   *
*	botSplit.CreateView(0, 0, ...) //Create bottom-left view.                         *
*	botSplit.CreateView(0, 1, ...) //Create bottom-right view.                        *
*	horzSplit.AddNested(1, 0, &botSplit); //Adding botpSplit as nested to horz(1, 0). *
**************************************************************************************/
#pragma once
#include <vector>

class CSplitterEx : public CSplitterWndEx
{
public:
	DECLARE_DYNAMIC(CSplitterEx)
	BOOL CreateStatic(CWnd* m_pParent, int nRows, int nCols, DWORD dwStyle = WS_CHILD | WS_VISIBLE, UINT nID = AFX_IDW_PANE_FIRST)override;
	BOOL CreateView(int row, int col, CRuntimeClass* pViewClass, SIZE sizeInit, CCreateContext* pContext);
	bool AddNested(int row, int col, CWnd* pNested);
	bool HideRow(UINT nRow);
	bool ShowRow(UINT nRow);
	bool HideCol(UINT nCol);
	bool ShowCol(UINT nCol);
	DECLARE_MESSAGE_MAP()
protected:
	void RecalcPanes();
	afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
	void OnInvertTracker(const CRect& rect)override;
private:
	struct SPANES
	{
		int iRow { };    //Row of the pane.
		int iCol { };    //Column of the pane.
		CWnd* pPane { }; //Pane pointer.
	};

	//Vectors of row/col visibility flags.
	std::vector<bool> m_vecRows { };
	std::vector<bool> m_vecCols { };
	std::vector<SPANES> m_vecPanes { }; //vector of Panes.
};