/****************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/						    *
* This software is available under the "MIT License modified with The Commons Clause".  *
* https://github.com/jovibor/Pepper/blob/master/LICENSE                                 *
*                                                                                       *
* This is a HEX control for MFC apps, implemented as CWnd derived class.			    *
* The usage is quite simple:														    *
* 1. Construct CHexCtrl object — HEXControl::CHexCtrl myHex;						    *
* 2. Call myHex.Create member function to create an instance.   					    *
* 3. Call myHex.SetData method to set the data and its size to display as hex.	        *
****************************************************************************************/
#pragma once
#include <vector>
#include <tuple>
#include "HexCtrlRes.h"

namespace HEXControl
{
	struct HEXSEARCH
	{
		std::wstring wstrSearch { };
		DWORD dwSearchType { }; //Hex, Ascii, Unicode, etc...
		DWORD dwStartAt { }; //Offset search should start at.
		int iDirection { };
		bool fWrap { }; //Was search wrapped?
		int iWrap { }; //Wrap direction.
		bool fSecondMatch { false }; //First or subsequent match. 
		bool fFound { };
		bool fCount { true }; //Do we count matches or just print "Found".
	};

	/********************************************
	* CHexDlgSearch class definition.			*
	********************************************/
	class CHexDlgSearch : public CDialogEx
	{
	public:
		friend class CHexView;
		CHexDlgSearch(CWnd* m_pParent = nullptr) {}
		virtual ~CHexDlgSearch() {}
		BOOL Create(UINT nIDTemplate, CHexView* pParentWnd);
		CHexView* GetParent();
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		virtual BOOL OnInitDialog();
		afx_msg void OnButtonSearchF();
		afx_msg void OnButtonSearchB();
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		afx_msg void OnClose();
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnRadioBnRange(UINT nID);
		DECLARE_MESSAGE_MAP()
		void SearchCallback();
		void ClearAll();
	private:
		CHexView* m_pParent { };
		HEXSEARCH m_stSearch { };
		DWORD m_dwnOccurrence { };
		int m_iRadioCurrent { };
		COLORREF m_clrSearchFailed { RGB(200, 0, 0) };
		COLORREF m_clrSearchFound { RGB(0, 200, 0) };
		CBrush m_stBrushDefault;
		COLORREF m_clrMenu { GetSysColor(COLOR_MENU) };
	};

	/********************************************
	* CHexDlgAbout class definition.			*
	********************************************/
	class CHexDlgAbout : public CDialogEx
	{
	public:
		CHexDlgAbout(CWnd* m_pParent = nullptr) : CDialogEx(IDD_HEXCTRL_DIALOG_ABOUT) {}
		virtual ~CHexDlgAbout() {}
	protected:
		virtual BOOL OnInitDialog() override;
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		DECLARE_MESSAGE_MAP()
	private:
		bool m_fGithubLink { true };
		HCURSOR m_curHand { };
		HCURSOR m_curArrow { };
		HFONT m_fontDefault { };
		HFONT m_fontUnderline { };
		CBrush m_stBrushDefault;
		COLORREF m_clrMenu { GetSysColor(COLOR_MENU) };
	};

	/********************************************
	* CHexView class definition.				*
	********************************************/
	class CHexView : public CScrollView
	{
	public:
		friend class CHexDlgSearch;
		DECLARE_DYNCREATE(CHexView)
		BOOL Create(CWnd* pWndParent, const RECT& rc, UINT iId, CCreateContext* pContext, const LOGFONT* pLogFont);
		void SetData(const unsigned char* pData, DWORD_PTR dwCount);
		void ClearData();
		void SetSelection(DWORD dwOffset);
		void SetFont(const LOGFONT* pLogFontNew);
		void SetFontSize(UINT uiSize);
		UINT GetFontSize();
		void SetColor(COLORREF clrTextHex, COLORREF clrTextAscii, COLORREF clrTextCaption,
			COLORREF clrBk, COLORREF clrBkSelected);
		void SetCapacity(DWORD dwCapacity);
	protected:
		CHexView() {}
		virtual ~CHexView() {}
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
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		void OnMenuRange(UINT nID);
		void Recalc();
		int HitTest(LPPOINT); //Is any hex chunk withing given point?
		void CopyToClipboard(UINT nType);
		void Search(HEXSEARCH& rSearch);
		void SetSelection(DWORD dwClick, DWORD dwStart, DWORD dwBytes);
		void UpdateInfoText();
		DECLARE_MESSAGE_MAP()
	private:
		const BYTE* m_pRawData { };
		DWORD_PTR m_dwRawDataCount { };
		DWORD m_dwGridCapacity { 16 };
		DWORD m_dwGridBlockSize { m_dwGridCapacity / 2 }; //Size of block before space delimiter.
		CRect m_rcClient;
		SIZE m_sizeLetter { }; //Current font's letter size (width, height).
		CFont m_fontHexView;
		CFont m_fontRectBottom;
		CHexDlgSearch m_dlgSearch;
		CHexDlgAbout m_dlgAbout;
		CMenu m_menuPopup;
		COLORREF m_clrTextHex { GetSysColor(COLOR_WINDOWTEXT) };
		COLORREF m_clrTextAscii { GetSysColor(COLOR_WINDOWTEXT) };
		COLORREF m_clrTextCaption { RGB(0, 0, 180) };
		COLORREF m_clrBk { GetSysColor(COLOR_WINDOW) };
		COLORREF m_clrBkSelected { RGB(200, 200, 255) };
		COLORREF m_clrTextBottomRect { GetSysColor(COLOR_WINDOWTEXT) };
		COLORREF m_clrBkBottomRect { RGB(250, 250, 250) };
		CBrush m_stBrushBk { m_clrBk };
		CBrush m_stBrushBkSelected { m_clrBkSelected };
		CPen m_penLines { PS_SOLID, 1, RGB(200, 200, 200) };
		int m_iIndentAscii { }; //Indent of Ascii text begining.
		int m_iIndentFirstHexChunk { }; //First HEX chunk indent.
		int m_iIndentTextCapacityY { }; //Caption text (0 1 2... D E F...) vertical offset.
		int m_iIndentBottomLine { 1 }; //Bottom line indent from window's bottom.
		int m_iSpaceBetweenHexChunks { }; //Space between begining of two HEX chunks.
		int m_iSpaceBetweenAscii { }; //Space between Ascii chars.
		int m_iSpaceBetweenBlocks { }; //Additional space between hex chunks after half of capacity.
		int m_iHeightTopRect { }; //Height of the header where offsets (0 1 2... D E F...) reside.
		int m_iHeightBottomRect { 22 }; //Height of bottom Info rect.
		int m_iHeightBottomOffArea { m_iHeightBottomRect + m_iIndentBottomLine }; //Height of not visible rect from window's bottom to m_iThirdHorizLine.
		int m_iHeightWorkArea { }; //Needed for mouse selection point.y calculation.
		int m_iFirstVertLine { }, m_iSecondVertLine { }, m_iThirdVertLine { }, m_iFourthVertLine { }; //Vertical lines indent.
		DWORD m_dwSelectionStart { }, m_dwSelectionEnd { }, m_dwSelectionClick { }, m_dwBytesSelected { };
		const wchar_t* const m_pwszHexMap = L"0123456789ABCDEF";
		std::wstring m_wstrBottomText { };
		bool m_fSecondLaunch { false };
		bool m_fLMousePressed { false };
	};

	/********************************************
	* CHexCtrl class definition.				*
	********************************************/
	class CHexCtrl : public CWnd
	{
	public:
		DECLARE_DYNAMIC(CHexCtrl)
		CHexCtrl() {}
		virtual ~CHexCtrl() {}
		BOOL Create(CWnd* pWndParent, const RECT& rc, UINT iCtrlId, bool fFloat = false, const LOGFONT* pLogFont = nullptr);
		CHexView* GetActiveView() const { return m_pHexView; };
		void SetData(const PBYTE pData, DWORD_PTR dwCount) const;
		void ClearData();
		void SetSelection(DWORD dwOffset);
		void SetFont(const LOGFONT* pLogFontNew) const;
		void SetFontSize(UINT nSize) const;
		void SetColor(COLORREF clrTextHex = GetSysColor(COLOR_WINDOWTEXT),
			COLORREF clrTextAscii = GetSysColor(COLOR_WINDOWTEXT),
			COLORREF clrTextCaption = RGB(0, 0, 180),
			COLORREF clrBk = GetSysColor(COLOR_WINDOW),
			COLORREF clrBkSelected = RGB(200, 200, 255)) const;
	private:
		DECLARE_MESSAGE_MAP()
		CHexView* m_pHexView { };
		const LOGFONT* m_pLogFontHexView { };
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	};

	/****************************
	* Internal identificators	*
	****************************/

	constexpr auto IDC_MENU_POPUP_SEARCH = 0x01;
	constexpr auto IDC_MENU_POPUP_COPY_AS_HEX = 0x02;
	constexpr auto IDC_MENU_POPUP_COPY_AS_HEX_FORMATTED = 0x03;
	constexpr auto IDC_MENU_POPUP_COPY_AS_ASCII = 0x04;
	constexpr auto IDC_MENU_POPUP_ABOUT = 0x05;

	constexpr auto CLIPBOARD_COPY_AS_HEX = 0x01;
	constexpr auto CLIPBOARD_COPY_AS_HEX_FORMATTED = 0x02;
	constexpr auto CLIPBOARD_COPY_AS_ASCII = 0x03;

	constexpr auto HEXCTRL_SEARCH_HEX = 0x01;
	constexpr auto HEXCTRL_SEARCH_ASCII = 0x02;
	constexpr auto HEXCTRL_SEARCH_UNICODE = 0x03;

	constexpr auto HEXCTRL_SEARCH_FORWARD = 1;
	constexpr auto HEXCTRL_SEARCH_BACKWARD = -1;
	constexpr auto HEXCTRL_SEARCH_NOTFOUND = 0x00;
	constexpr auto HEXCTRL_SEARCH_FOUND = 0x01;
	constexpr auto HEXCTRL_SEARCH_WRAP_BEGINNING = 0x02;
	constexpr auto HEXCTRL_SEARCH_WRAP_END = 0x03;
};