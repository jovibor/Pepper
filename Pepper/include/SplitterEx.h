#pragma once

class CSplitterEx : public CSplitterWndEx
{
public:
	DECLARE_DYNAMIC(CSplitterEx)
	CSplitterEx();
	virtual ~CSplitterEx();
	BOOL CreateStatic(CWnd* pParent, int nRows, int nCols, DWORD dwStyle = WS_CHILD | WS_VISIBLE, UINT nID = AFX_IDW_PANE_FIRST) override;
	int HideRow(int nRow);
	int HideCol(int nCol);
	int ShowRow(int nRow);
	int ShowCol(int nCol);
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
	DECLARE_MESSAGE_MAP()
private:
	//tuple of: row, is visible?
	std::vector<std::tuple<int, bool>> m_vecRows { };
	std::vector<std::tuple<int, bool>> m_vecCols { };
};