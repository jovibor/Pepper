/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
/********************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/					*
* Extended CSplitterWnd class with the ability to Hide/Show individual			*
* rows and columns.																*
* Has five additional methods: 1.HideRow() 2. ShowRow()							*
* 3. HideCol(), 4. ShowCol() 5. AddNested().									*
* Public method AddNested() is used when dealing with nested splitters.			*
* It's needed to properly handle Hide/Show row/cols methods on splitters		*
* without generic views, i.e. splitters that is used as host for other, nested	*
* splitters. Below is an example:												*
* CSplitterEx mainSpl, leftSpl, rightSpl;										*
* mainSpl.CreateStatic(this, 2, 1);												*
* leftSpl.CreateStatic(&mainSpl, 1, 2, dwStyle, mainSpl.IdFromRowCol(0, 0));	*
* rightSpl.CreateStatic(&mainSpl, 1, 2, dwStyle, mainSpl.IdFromRowCol(1, 0));	*
* mainSpl.AddNested(0, 0, &leftSpl); mainSpl.AddNested(1, 0, &rightSpl);		*
********************************************************************************/
#pragma once
#include <vector>
#include <tuple>

class CSplitterEx : public CSplitterWndEx
{
public:
	DECLARE_DYNAMIC(CSplitterEx)
	BOOL CreateStatic(CWnd* m_pParent, int nRows, int nCols, DWORD dwStyle = WS_CHILD | WS_VISIBLE, UINT nID = AFX_IDW_PANE_FIRST) override;
	BOOL CreateView(int row, int col, CRuntimeClass* pViewClass, SIZE sizeInit, CCreateContext* pContext);
	bool AddNested(int row, int col, CWnd* pNested);
	bool HideRow(UINT nRow);
	bool ShowRow(UINT nRow);
	bool HideCol(UINT nCol);
	bool ShowCol(UINT nCol);
	DECLARE_MESSAGE_MAP()
protected:
	void RecalcPanes();
	void StartTracking(int ht);
	void StopTracking(BOOL fAccept);
	afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
	virtual void OnInvertTracker(const CRect& rect);
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
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
	//row, column, pPane.
	std::vector<SPANES> m_vecPanes { };
};