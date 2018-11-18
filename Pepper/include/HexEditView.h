#pragma once

class CHexEditView : public CScrollView
{
public:
	BOOL Create(CWnd* pParent, const RECT& rect, UINT nID, CCreateContext* pContext, CFont* pFont);
	BOOL SetData(const std::vector<std::byte> *vecData);
	BOOL SetData(const std::string& strData);
	int SetFont(CFont* pFontNew);
	void SetFontSize(UINT nSize);
	void SetFontColor(COLORREF clrTextHex, COLORREF clrTextOffset, 
		COLORREF clrTextSelected, COLORREF clrBk, COLORREF clrBkSelected);
	UINT GetFontSize();
protected:
	DECLARE_DYNCREATE(CHexEditView)
	CHexEditView() {}
	virtual ~CHexEditView() {}
	void OnInitialUpdate() override;     // first time after construct
	void OnDraw(CDC* pDC) override;      // overridden to draw this view
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	int HitTest(LPPOINT); // Is any hex chunk withing given LPPOINT? 
	void Recalc();
	DECLARE_MESSAGE_MAP()
private:
	const BYTE* m_pRawData { };
	CRect m_rcClient;
	UINT m_dwRawDataCount { };
	SIZE m_sizeLetter { }; //Current font's letter size (width, height).
	CFont m_fontHexView;
	CPen m_penLines { PS_SOLID, 1, RGB(200, 200, 200) };
	COLORREF m_clrTextHex { GetSysColor(COLOR_WINDOWTEXT) };
	COLORREF m_clrTextOffset { RGB(0, 0, 180) };
	COLORREF m_clrTextSelected { RGB(0, 0, 180) };
	COLORREF m_clrTextBk { GetSysColor(COLOR_WINDOW) };
	COLORREF m_clrTextBkSelected { RGB(200, 200, 255) };
	int m_nIndentAscii { }; //Offset of Ascii text begining.
	int m_nIndentFirstHexChunk { }; //First HEX chunk indentation.
	int m_nIndentBetweenHexChunks { }; //Indent between begining of two HEX chunks.
	int m_nIndentBetweenAscii { }; //Indent between ASCII chars.
	int m_nIndentBetween78 { }; //Additional indent to add after 7-th Hex chunk.
	int m_nHeaderRectWidth { }; //Width of the header where offset (0 1 2... D E F) resides.
	int m_nFirstVertLine { }, m_nSecondVertLine { }, m_nThirdVertLine { }, m_nFourthVertLine { }; //Vertical line indent.
	int m_nFirstHorizLine { }, m_nSecondHorizLine { }, m_nThirdHorizLine { }, m_nFourthHorizLine { }; //Horizontal line indent.
	int m_nBottomRectWidth { 25 };
	//	UINT m_nSecondHex { };
	WCHAR m_strOffset[9] { };
	const wchar_t* const m_strHexMap = L"0123456789ABCDEF";
	SCROLLINFO m_stScrollInfo { sizeof(SCROLLINFO), SIF_ALL };
	bool m_fSecondLaunch { false };
	bool m_fLMousePressed { false };
	bool m_fSelection { false };
	DWORD m_dwSelectionStart { }, m_dwSelectionEnd { };
	CRect m_rcSpaceBetweenHex { }; //Space between hex chunks, needs for selection draw.
	CBrush m_brTextBkSelection { m_clrTextBkSelected };
	CBrush m_brTextBkDefault { m_clrTextBk };
};