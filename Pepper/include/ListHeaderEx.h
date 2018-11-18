#pragma once

class CListHeaderEx : public CMFCHeaderCtrl
{
public:
	void SetHeight(DWORD dwHeight);
	void SetHdrColor(COLORREF clrBk, COLORREF clrText);
	int SetFont(CFont* pFontNew);
protected:
	DECLARE_DYNAMIC(CListHeaderEx)
	CListHeaderEx();
	virtual ~CListHeaderEx() {}
	afx_msg void OnDrawItem(CDC* pDC, int iItem, CRect rect, BOOL bIsPressed, BOOL bIsHighlighted) override;
	afx_msg LRESULT OnLayout(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	CFont m_fontHdr;
	COLORREF m_clrHdr { GetSysColor(COLOR_WINDOW) };
	//COLORREF m_clrHdr { RGB(0, 132, 132) };
	COLORREF m_clrText { GetSysColor(COLOR_WINDOWTEXT) };
	//For drawing.
	HDITEMW m_hdItem { };
	WCHAR m_strHeaderText[MAX_PATH] { };
	DWORD m_dwHeaderHeight { 19 };
};