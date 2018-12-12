/****************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/	                *
* This is a HEX control class for MFC based apps, derived from CWnd.		*
* The usage is quite simple:												*
* 1. Construct CHexCtrl object.												*
* 2. Call Create(...) member function to create an instance.				*
* 3. Use one of SetData(...) methods to set actual data to display as hex.	*
* 4. Set window position, if needed, with hexCtrl.SetWindowPos(...).		*
*****************************************************************************/
#pragma once
#include <vector>

#ifndef __cpp_lib_byte
#define __cpp17_conformant 0
#elif __cpp_lib_byte < 201603
#define __cpp17_conformant 0
#else
#define __cpp17_conformant 1
#endif
static_assert(__cpp17_conformant, "C++17 conformant compiler is required (MSVS 15.7 with /std:c++17 or higher).");

class CHexCtrl : public CWnd
{
private:
	/********************************************
	* CHexView class definition.				*
	********************************************/
	class CHexView : public CScrollView
	{
	public:
		BOOL Create(CWnd* pParent, const RECT& rect, UINT nID, CCreateContext* pContext, CFont* pFont);
		void SetData(const std::vector<std::byte>& vecData);
		void SetData(const std::string& strData);
		void SetData(const PBYTE pData, DWORD_PTR dwCount);
		void ClearData();
		int SetFont(CFont* pFontNew);
		void SetFontSize(UINT nSize);
		void SetFontColor(COLORREF clrTextHex, COLORREF clrTextOffset,
			COLORREF clrTextSelected, COLORREF clrBk, COLORREF clrBkSelected);
		UINT GetFontSize();
	protected:
		DECLARE_DYNCREATE(CHexView)
		CHexView() {}
		virtual ~CHexView() {}
		void OnInitialUpdate() override;
		void OnDraw(CDC* pDC) override;
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		void OnMenuRange(UINT nID);
		int HitTest(LPPOINT); // Is any hex chunk withing given LPPOINT?
		int CopyToClipboard(UINT nType);
		void Recalc();
		DECLARE_MESSAGE_MAP()
	private:
		const BYTE* m_pRawData { };
		CRect m_rcClient;
		DWORD_PTR m_dwRawDataCount { };
		SIZE m_sizeLetter { }; //Current font's letter size (width, height).
		CFont m_fontHexView;
		CPen m_penLines { PS_SOLID, 1, RGB(200, 200, 200) };
		COLORREF m_clrTextHex { GetSysColor(COLOR_WINDOWTEXT) };
		COLORREF m_clrTextOffset { RGB(0, 0, 180) };
		COLORREF m_clrTextSelected { RGB(0, 0, 180) };
		COLORREF m_clrTextBk { GetSysColor(COLOR_WINDOW) };
		COLORREF m_clrTextBkSelected { RGB(200, 200, 255) };
		int m_iIndentAscii { }; //Offset of Ascii text begining.
		int m_iIndentFirstHexChunk { }; //First HEX chunk indentation.
		int m_iIndentBetweenHexChunks { }; //Indent between begining of two HEX chunks.
		int m_iIndentBetweenAscii { }; //Indent between ASCII chars.
		int m_iIndentBetween78 { }; //Additional indent to add after 7-th Hex chunk.
		int m_iHeightHeaderRect { }; //Height of the header where offset (0 1 2... D E F) resides.
		int m_iFirstVertLine { }, m_iSecondVertLine { }, m_iThirdVertLine { }, m_iFourthVertLine { }; //Vertical line indent.
		int m_iFirstHorizLine { }, m_iSecondHorizLine { }, m_iThirdHorizLine { }, m_iFourthHorizLine { }; //Horizontal line indent.
		int m_iHeightBottomRect { 25 };
		WCHAR m_strOffset[9] { };
		const wchar_t* const m_strHexMap = L"0123456789ABCDEF";
		SCROLLINFO m_stScrollInfo { sizeof(SCROLLINFO), SIF_ALL };
		SCROLLBARINFO m_stSBI { sizeof(SCROLLBARINFO) };
		bool m_fSecondLaunch { false };
		bool m_fLMousePressed { false };
		bool m_fSelected { false };
		DWORD m_dwSelectionStart { }, m_dwSelectionEnd { };
		CRect m_rcSpaceBetweenHex { }; //Space between hex chunks, needs for selection draw.
		CBrush m_brTextBk { m_clrTextBk };
		CBrush m_brTextBkSelection { m_clrTextBkSelected };
		CMenu m_menuPopup;
		int m_iHeightWorkArea { }; //Needed for mouse selection point.y calculation.
	};
public:
	CHexCtrl() {}
	virtual ~CHexCtrl() {}
	BOOL Create(CWnd* pParent, const RECT& rect, UINT nID, CFont* pFont = nullptr/*default*/);
	CHexView* GetActiveView() const { return m_pHexView; };
	void SetData(const std::vector<std::byte>& vecData) const;
	void SetData(const std::string& strData) const;
	void SetData(const PBYTE pData, DWORD_PTR dwCount) const;
	void ClearData();
	int SetFont(CFont* pFontNew) const;
	void SetFontSize(UINT nSize) const;
	void SetFontColor(COLORREF clrTextHex, COLORREF clrTextOffset,
		COLORREF clrTextSelected, COLORREF clrBk, COLORREF clrBkSelected) const;
private:
	DECLARE_DYNAMIC(CHexCtrl)
	CHexView* m_pHexView { };
	CFont* m_pFontHexView { };
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

/****************************
* Internal identificators	*
****************************/

constexpr auto IDC_MENU_POPUP_COPY_AS_HEX = 0x01;
constexpr auto IDC_MENU_POPUP_COPY_AS_HEX_FORMATTED = 0x02;
constexpr auto IDC_MENU_POPUP_COPY_AS_ASCII = 0x03;

constexpr auto CLIPBOARD_COPY_AS_HEX = 0x01;
constexpr auto CLIPBOARD_COPY_AS_HEX_FORMATTED = 0x02;
constexpr auto CLIPBOARD_COPY_AS_ASCII = 0x03;