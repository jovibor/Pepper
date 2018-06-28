#pragma once

class CHexEditView : public CScrollView
{
public:
	DECLARE_DYNCREATE(CHexEditView)
	CHexEditView() {};           // protected constructor used by dynamic creation
	virtual ~CHexEditView();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL Create(CWnd* pParent, const RECT& rect, UINT nID, CCreateContext* pContext, CFont* pFont);
	BOOL SetData(const std::vector<std::byte> *vecData);
	BOOL SetData(const std::string_view strData, UINT nCount);
	void SetFontSize(UINT nSize);
	CFont* SetFont(CFont* pFont);
	UINT GetFontSize();
protected:
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	void Recalc();

	DECLARE_MESSAGE_MAP()
private:
	const BYTE* m_pRawData { };
	CRect m_rectClient;
	UINT m_dwRawDataCount { };
	SIZE m_sizeLetter { };//Current font's letter size (width, height)
	CFont* m_pFontHexView { };
	CFont* m_pFontDefaultHexView { };
	CPen m_penLines { PS_SOLID, 1, RGB(200, 200, 200) };
	COLORREF m_colorTextOffset { RGB(0, 0, 180) };
	COLORREF m_colorTextHex { RGB(0, 0, 0) };
	UINT m_nOffsetAscii { };//Indent of Ascii text
	UINT m_nIndentBetweenHexChunk { };//indent between two HEX chunks
	UINT m_nIndentBetweenAscii { };//Indent between ASCII chars
	UINT m_nIndentBetween78 { };//Additional indent to add after 7-th Hex chunk
	UINT m_nTopHeaderWidth { };
	UINT m_nFirstVertLine { }, m_nSecondVertLine { }, m_nThirdVertLine { }, m_nFourthVertLine { };
	UINT m_nFirstHorizLine { }, m_nSecondHorizLine { }, m_nThirdHorizLine { }, m_nFourthHorizLine { };
	UINT m_nBottomRectWidth { 25 };
	UINT m_nFirstHexChunkIndent { };
	//	UINT m_nSecondHex { };
	WCHAR m_strOffset[90] { };
	const wchar_t* m_strHexMap = L"0123456789ABCDEF";
	SCROLLINFO m_stScrollInfo { sizeof(SCROLLINFO), SIF_ALL };
	bool m_fSecondLaunch = false;
	bool m_fEraseBkgnd = false;
};


