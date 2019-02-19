/****************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/						    *
* This software is available under the "MIT License modified with The Commons Clause".  *
* https://github.com/jovibor/Pepper/blob/master/LICENSE                                 *
* This is a Hex control for MFC apps, implemented as CWnd derived class.				*
* The usage is quite simple:														    *
* 1. Construct CHexCtrl object — HEXCTRL::CHexCtrl myHex;							    *
* 2. Call myHex.Create member function to create an instance.   					    *
* 3. Call myHex.SetData method to set the data and its size to display as hex.	        *
****************************************************************************************/
#pragma once
#pragma comment(lib, "Dwmapi.lib")
#include <afxcontrolbars.h>
#include <vector>
#include "HexCtrlRes.h"
#include "ScrollEx.h"

namespace HEXCTRL
{
	struct HEXCREATESTRUCT
	{
		CWnd*		pwndParent { };				//Parent window pointer.
		UINT		uId { };					//Hex control id.
		DWORD		dwExStyles { };				//Extended window styles.
		CRect		rc { };						//Initial rect.
		bool		fFloat { false };			//Is float or child - incorporated into another window.
		const		LOGFONT* pLogFont { };		//Font to be used. Default if it's nullptr.
		CWnd*		pwndMsg { };				//Pointer to the window that is going to recieve command messages, 
												//such as HEXCTRL_MSG_GETDISPINFO. If zero - parent window is used.
	};

	struct HEXNOTIFY
	{
		NMHDR			hdr;			//Standart Windows header.
		ULONGLONG		ullByteIndex;	//Index of the byte to draw next.
		unsigned char	chByte;			//Value of that byte to send back.
	};
	using PHEXNOTIFY = HEXNOTIFY * ;

	/********************************************
	* CHexDlgAbout class definition.			*
	********************************************/
	class CHexDlgAbout : public CDialogEx
	{
	public:
		CHexDlgAbout(CWnd* m_pParent = nullptr) : CDialogEx(IDD_HEXCTRL_ABOUT) {}
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
	class CHexCtrl;
	class CHexDlgSearch : public CDialogEx
	{
	private:
		struct HEXSEARCH
		{
			std::wstring	wstrSearch { };			//String search for.
			DWORD			dwSearchType { };		//Hex, Ascii, Unicode, etc...
			ULONGLONG		ullStartAt { };			//An offset, search should start at.
			int				iDirection { };
			bool			fWrap { false };		//Was search wrapped?
			int				iWrap { };				//Wrap direction.
			bool			fSecondMatch { false }; //First or subsequent match. 
			bool			fFound { false };
			bool			fCount { true };		//Do we count matches or just print "Found".
		};
	public:
		friend class CHexCtrl;
		CHexDlgSearch(CWnd* m_pParent = nullptr) {}
		virtual ~CHexDlgSearch() {}
		BOOL Create(UINT nIDTemplate, CHexCtrl* pParentWnd);
		CHexCtrl* GetParent() const;
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
		CHexCtrl* m_pParent { };
		HEXSEARCH m_stSearch { };
		DWORD m_dwnOccurrence { };
		int m_iRadioCurrent { };
		COLORREF m_clrSearchFailed { RGB(200, 0, 0) };
		COLORREF m_clrSearchFound { RGB(0, 200, 0) };
		CBrush m_stBrushDefault;
		COLORREF m_clrMenu { GetSysColor(COLOR_MENU) };
	};

	/********************************************
	* CHexCtrl class definition.				*
	********************************************/
	class CHexCtrl : public CWnd
	{
	public:
		friend class CHexDlgSearch;
		CHexCtrl() {}
		virtual ~CHexCtrl() {}
		bool Create(const HEXCREATESTRUCT& hc); //Main initialization method, CHexCtrl::Create.
		/************************************************************************************************************
		* CHexCtrl::SetData:																						*
		* 1. Pointer to data, not used if it's virtual control 2. Size of data to see as hex.						*
		* 3. Is virtual? 4. Offset to scroll to after creation.														*
		* 5. Pointer to window to send command messages to. If nullptr, parent window is used.						*
		************************************************************************************************************/
		void SetData(const unsigned char* pData, ULONGLONG ullSize, bool fVirtual = false, ULONGLONG ullOffset = 0, CWnd* pwndMsg = nullptr);
		void ClearData();
		void SetSelection(ULONGLONG ullOffset, ULONGLONG ullSize = 1);
		void SetFont(const LOGFONT* pLogFontNew);
		void SetFontSize(UINT uiSize);
		UINT GetFontSize();
		void SetColor(COLORREF clrTextHex, COLORREF clrTextAscii, COLORREF clrTextCaption,
			COLORREF clrBk, COLORREF clrBkSelected);
		void SetCapacity(DWORD dwCapacity);
		int GetDlgCtrlID() const;
		CWnd* GetParent() const;
	protected:
		DECLARE_MESSAGE_MAP()
		void OnDraw(CDC* pDC) {} //All drawing is in OnPaint.
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnPaint();
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg BOOL OnNcActivate(BOOL bActive);
		afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
		afx_msg void OnNcPaint();
		afx_msg void OnDestroy();
		void Recalc();
		void CalcWorkAreaHeight(int iClientHeight);
		void CalcScrollSizes(int iClientHeight, int iClientWidth, ULONGLONG ullCurLine);
		void CalcScrollPageSize();
		ULONGLONG GetCurrentLineV();
		ULONGLONG HitTest(LPPOINT); //Is any hex chunk withing given point?
		void HexPoint(ULONGLONG ullChunk, ULONGLONG& ullCx, ULONGLONG& ullCy);
		void CopyToClipboard(UINT nType);
		void Search(CHexDlgSearch::HEXSEARCH& rSearch);
		void SetSelection(ULONGLONG dwClick, ULONGLONG dwStart, ULONGLONG dwBytes, bool fHighlight = false);
		void UpdateInfoText();
	private:
		bool m_fCreated { false };
		bool m_fFloat { false };
		bool m_fVirtual { false };
		const unsigned char* m_pData { };
		ULONGLONG m_ullDataCount { };
		DWORD m_dwGridCapacity { 16 };
		DWORD m_dwGridBlockSize { m_dwGridCapacity / 2 }; //Size of block before space delimiter.
		CWnd* m_pwndParentOwner { };
		CWnd* m_pwndMsg { };
		SIZE m_sizeLetter { 1, 1 }; //Current font's letter size (width, height).
		CFont m_fontHexView;
		CFont m_fontBottomRect;
		CHexDlgSearch m_dlgSearch;
		CHexDlgAbout m_dlgAbout;
		CScrollEx m_stScrollV;
		CScrollEx m_stScrollH;
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
		int m_iDistanceBetweenHexChunks { }; //Space between begining of two hex chunks.
		int m_iSpaceBetweenAscii { }; //Space between Ascii chars.
		int m_iSpaceBetweenBlocks { }; //Additional space between hex chunks after half of capacity.
		int m_iHeightTopRect { }; //Height of the header where offsets (0 1 2... D E F...) reside.
		int m_iHeightBottomRect { 22 }; //Height of bottom Info rect.
		int m_iHeightBottomOffArea { m_iHeightBottomRect + m_iIndentBottomLine }; //Height of not visible rect from window's bottom to m_iThirdHorizLine.
		int m_iHeightWorkArea { }; //Needed for mouse selection point.y calculation.
		int m_iFirstVertLine { }, m_iSecondVertLine { }, m_iThirdVertLine { }, m_iFourthVertLine { }; //Vertical lines indent.
		ULONGLONG m_ullSelectionStart { }, m_ullSelectionEnd { }, m_ullSelectionClick { }, m_ullBytesSelected { };
		const wchar_t* const m_pwszHexMap = L"0123456789ABCDEF";
		std::wstring m_wstrBottomText { };
		std::wstring m_wstrErrVirtual { L"This function isn't supported when in virtual mode!" };
		bool m_fLMousePressed { false };
		UINT m_dwCtrlId { };
		//Enums.
		enum HEXCTRL_CLIPBOARD {
			COPY_AS_HEX = 0x01,
			COPY_AS_HEX_FORMATTED = 0x02,
			COPY_AS_ASCII = 0x03
		};
		enum HEXCTRL_MENU {
			IDM_POPUP_SEARCH = 0x8001,
			IDM_POPUP_COPYASHEX = 0x8002,
			IDM_POPUP_COPYASHEXFORMATTED = 0x8003,
			IDM_POPUP_COPYASASCII = 0x8004,
			IDM_POPUP_ABOUT = 0x8005
		};
		enum HEXCTRL_SEARCH {
			SEARCH_HEX = 0x10, SEARCH_ASCII = 0x20, SEARCH_UNICODE = 0x30,
			SEARCH_FORWARD = 1, SEARCH_BACKWARD = -1,
			SEARCH_NOTFOUND = 0, SEARCH_FOUND = 0xF,
			SEARCH_BEGINNING = 0xA, SEARCH_END = 0xB,
		};
	};

	/************************************************
	* WM_NOTIFY message codes (NMHDR.code values)	*
	************************************************/

	constexpr auto HEXCTRL_MSG_DESTROY = 0x00FF;
	constexpr auto HEXCTRL_MSG_GETDISPINFO = 0x0100;
};