/****************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/						    *
* This software is available under the "MIT License modified with The Commons Clause".  *
* https://github.com/jovibor/Pepper/blob/master/LICENSE                                 *
*                                                                                       *
* This is a Hex control for MFC apps, implemented as CWnd derived class.				*
* The usage is quite simple:														    *
* 1. Construct CHexCtrl object — HEXCTRL::CHexCtrl myHex;							    *
* 2. Call myHex.Create member function to create an instance.   					    *
* 3. Call myHex.SetData method to set the data and its size to display as hex.	        *
****************************************************************************************/
#pragma once
#include <vector>
#include "HexCtrlRes.h"

namespace HEXCTRL
{
	struct HEXSEARCH
	{
		std::wstring	wstrSearch { };
		DWORD			dwSearchType { }; //Hex, Ascii, Unicode, etc...
		DWORD			dwStartAt { }; //Offset search should start at.
		int				iDirection { };
		bool			fWrap { }; //Was search wrapped?
		int				iWrap { }; //Wrap direction.
		bool			fSecondMatch { false }; //First or subsequent match. 
		bool			fFound { };
		bool			fCount { true }; //Do we count matches or just print "Found".
	};

	struct HEXNOTIFY
	{
		NMHDR			hdr;
		DWORD_PTR		dwByteIndex;
		unsigned char	chByte;
	};
	using PHEXNOTIFY = HEXNOTIFY * ;

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
	* CHexDlgSearch class definition.			*
	********************************************/
	class CHexView;
	class CHexDlgSearch : public CDialogEx
	{
	public:
		friend class CHexView;
		CHexDlgSearch(CWnd* m_pParent = nullptr) {}
		virtual ~CHexDlgSearch() {}
		BOOL Create(UINT nIDTemplate, CHexView* pParentWnd);
		CHexView* GetParent() const;
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		virtual BOOL OnInitDialog();
		afx_msg void OnButtonSearchF();
		afx_msg void OnButtonSearchB();
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg void OnClose();
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnRadioBnRange(UINT nID);
		void SearchCallback();
		void ClearAll();
		DECLARE_MESSAGE_MAP()
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
	* CHexView class definition.				*
	********************************************/
	class CHexCtrl;
	class CHexView : public CScrollView
	{
	public:
		friend class CHexDlgSearch;
		DECLARE_DYNCREATE(CHexView)
		BOOL Create(CHexCtrl* pWndParent, const RECT& rc, UINT uiId, CCreateContext* pContext, const LOGFONT* pLogFont);
		void SetData(const unsigned char* pData, DWORD_PTR dwCount, bool fVirtual);
		void ClearData();
		void SetSelection(DWORD_PTR dwOffset, DWORD dwCount);
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
		virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		void Recalc();
		int HitTest(LPPOINT); //Is any hex chunk withing given point?
		void CopyToClipboard(UINT nType);
		void Search(HEXSEARCH& rSearch);
		void SetSelection(DWORD_PTR dwClick, DWORD_PTR dwStart, DWORD dwBytes, bool fHighlight = false);
		void UpdateInfoText();
		DECLARE_MESSAGE_MAP()
	private:
		bool m_fVirtual { false };
		const BYTE* m_pData { };
		DWORD_PTR m_dwDataCount { };
		DWORD m_dwGridCapacity { 16 };
		DWORD m_dwGridBlockSize { m_dwGridCapacity / 2 }; //Size of block before space delimiter.
		CHexCtrl* m_pwndParent { };
		CWnd* m_pwndGrParent { }; //Parent of CHexCtrl.
		CRect m_rcClient;
		SIZE m_sizeLetter { }; //Current font's letter size (width, height).
		CFont m_fontHexView;
		CFont m_fontBottomRect;
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
		const CBrush m_stBrushBk { m_clrBk };
		const CBrush m_stBrushBkSelected { m_clrBkSelected };
		CPen m_penLines { PS_SOLID, 1, RGB(200, 200, 200) };
		int m_iIndentAscii { }; //Indent of Ascii text begining.
		int m_iIndentFirstHexChunk { }; //First hex chunk indent.
		int m_iIndentTextCapacityY { }; //Caption text (0 1 2... D E F...) vertical offset.
		int m_iIndentBottomLine { 1 }; //Bottom line indent from window's bottom.
		int m_iSpaceBetweenHexChunks { }; //Space between begining of two hex chunks.
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
		enum HEXCTRL_SEARCH
		{
			SEARCH_HEX = 0x01, SEARCH_ASCII = 0x02,
			SEARCH_UNICODE = 0x03, SEARCH_FORWARD = 1,
			SEARCH_BACKWARD = -1, SEARCH_NOTFOUND = 0,
			SEARCH_FOUND = 0x01, SEARCH_BEGINNING = 0x02,
			SEARCH_END = 0x03,
		};
		static constexpr auto CLIPBOARD_COPY_AS_HEX { 0x01 };
		static constexpr auto CLIPBOARD_COPY_AS_HEX_FORMATTED { 0x02 };
		static constexpr auto CLIPBOARD_COPY_AS_ASCII { 0x03 };
		static constexpr auto IDM_POPUP_SEARCH = 0x8001;
		static constexpr auto IDM_POPUP_COPYASHEX = 0x8002;
		static constexpr auto IDM_POPUP_COPYASHEXFORMATTED = 0x8003;
		static constexpr auto IDM_POPUP_COPYASASCII = 0x8004;
		static constexpr auto IDM_POPUP_ABOUT = 0x8005;
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
		BOOL Create(CWnd* pwndParent, UINT uiCtrlId, const CRect* pRect = nullptr, bool fFloat = false, const LOGFONT* pLogFont = nullptr);
		CHexView* GetActiveView() const { return m_pHexView; };
		void SetData(const PBYTE pData, DWORD_PTR dwCount, bool fVirtual = false) const;
		void ClearData();
		void SetSelection(DWORD_PTR dwOffset, DWORD dwBytes = 1);
		void SetFont(const LOGFONT* pLogFontNew) const;
		void SetFontSize(UINT nSize) const;
		void SetColor(COLORREF clrTextHex = GetSysColor(COLOR_WINDOWTEXT),
			COLORREF clrTextAscii = GetSysColor(COLOR_WINDOWTEXT),
			COLORREF clrTextCaption = RGB(0, 0, 180),
			COLORREF clrBk = GetSysColor(COLOR_WINDOW),
			COLORREF clrBkSelected = RGB(200, 200, 255)) const;
		int GetDlgCtrlID() const;
		CWnd* GetParent() const;
	private:
		DECLARE_MESSAGE_MAP()
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg void OnDestroy();
	private:
		CHexView* m_pHexView { };
		CWnd* m_pwndParentOwner { };
		const LOGFONT* m_pLogFontHexView { };
		UINT m_dwCtrlId { };
		bool m_fFloat { false };
		bool m_fCreated { false };
	};

	/************************************************
	* WM_NOTIFY message codes (NMHDR.code values)	*
	************************************************/

	constexpr auto HEXCTRL_MSG_DESTROY = 0x00ff;
	constexpr auto HEXCTRL_MSG_SCROLLING = 0x0100;
	constexpr auto HEXCTRL_MSG_GETDISPINFO = 0x0101;
};