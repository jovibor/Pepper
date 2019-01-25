/****************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/	                *
* This is an extended and quite featured version of CMFCListCtrl class.		*
* The main difference is in CListEx::Create method, which takes one			*
* additional arg - pointer to LISTEXINFO structure, which fields are		*
* described below.															*
* Also, this class has set of additional public methods to help customize	*
* your control in many different aspects.									*
****************************************************************************/
#pragma once
#include <afxcontrolbars.h>
#include <unordered_map>

namespace LISTEX {
	/************************************************
	* Helper struct for CListEx class.				*
	************************************************/
	typedef struct LISTEXINFO {
		COLORREF clrListText { GetSysColor(COLOR_WINDOWTEXT) }; //List text color.
		COLORREF clrListBkRow1 { GetSysColor(COLOR_WINDOW) }; //List Bk color of the odd rows.
		COLORREF clrListBkRow2 { GetSysColor(COLOR_WINDOW) }; //List Bk color of the even rows.
		COLORREF clrListGrid { RGB(220, 220, 220) }; //List grid color.
		DWORD dwListGridWidth { 1 }; //Width of the list grid.
		COLORREF clrListTextSelected { GetSysColor(COLOR_HIGHLIGHTTEXT) }; //Selected item text color.
		COLORREF clrListBkSelected { GetSysColor(COLOR_HIGHLIGHT) }; //Selected item bk color.
		COLORREF clrTooltipText { GetSysColor(COLOR_INFOTEXT) }; //Tooltip window text color.
		COLORREF clrTooltipBk { GetSysColor(COLOR_INFOBK) }; //Tooltip window bk color.
		COLORREF clrListTextCellTt { GetSysColor(COLOR_WINDOWTEXT) }; //Text color of a cell that has tooltip.
		COLORREF clrListBkCellTt { RGB(170, 170, 230) }; //Bk color of a cell that has tooltip.
		const LOGFONT* pListLogFont { }; //List font.
		COLORREF clrHeaderText { GetSysColor(COLOR_WINDOWTEXT) }; //List header text color.
		COLORREF clrHeaderBk { GetSysColor(COLOR_WINDOW) }; //List header bk color.
		DWORD dwHeaderHeight { 20 }; //List header height.
		const LOGFONT* pHeaderLogFont { }; //List header font.
	} *PLISTEXINFO;

	/********************************************
	* CListExHeader class definition.			*
	********************************************/
	class CListExHeader : public CMFCHeaderCtrl
	{
	public:
		void SetHeight(DWORD dwHeight);
		void SetFont(const LOGFONT* pFontNew);
		void SetColor(COLORREF clrText, COLORREF clrBk);
		void SetColumnColor(DWORD iColumn, COLORREF clr);
		CListExHeader();
		virtual ~CListExHeader() {}
	protected:
		afx_msg void OnDrawItem(CDC* pDC, int iItem, CRect rect, BOOL bIsPressed, BOOL bIsHighlighted) override;
		afx_msg LRESULT OnLayout(WPARAM wParam, LPARAM lParam);
		DECLARE_MESSAGE_MAP()
	private:
		CFont m_fontHdr;
		COLORREF m_clrBkNWA { GetSysColor(COLOR_WINDOW) }; //Bk of non working area.
		COLORREF m_clrText { GetSysColor(COLOR_WINDOWTEXT) };
		COLORREF m_clrBk { GetSysColor(COLOR_WINDOW) };
		HDITEMW m_hdItem { }; //For drawing.
		WCHAR m_wstrHeaderText[MAX_PATH] { };
		DWORD m_dwHeaderHeight { 19 }; //Standard (default) height.
		std::unordered_map<DWORD, COLORREF> m_umapClrColumn { }; //Color of individual columns.
	};

	/********************************************
	* CListEx class definition.					*
	********************************************/
	class CListEx : public CMFCListCtrl
	{
	public:
		DECLARE_DYNAMIC(CListEx)
		CListEx() {}
		virtual ~CListEx() {}
		BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, PLISTEXINFO pListExInfo = nullptr);
		CListExHeader& GetHeaderCtrl() override { return m_stListHeader; }
		void SetFont(const LOGFONT* pLogFontNew);
		void SetFontSize(UINT uiSize);
		UINT GetFontSize();
		//To remove tooltip from specific subitem just set it again with empty (L"") string.
		void SetCellTooltip(int iItem, int iSubitem, const std::wstring& wstrTooltip, const std::wstring& wstrCaption = { });
		void SetCellMenu(int iItem, int iSubitem, CMenu* pMenu);
		void SetListMenu(CMenu* pMenu);
		void SetCellData(int iItem, int iSubitem, DWORD_PTR dwData);
		DWORD_PTR GetCellData(int iItem, int iSubitem);
		void SetTooltipColor(COLORREF clrTooltipText, COLORREF clrTooltipBk,
			COLORREF clrTextCellTt = GetSysColor(COLOR_WINDOWTEXT), //Text color of a cell that has tooltip.
			COLORREF clrBkCellTt = RGB(170, 170, 230)); //Bk color of a cell that has tooltip.
		void SetHeaderColor(COLORREF clrHdrText, COLORREF clrHdrBk);
		void SetHeaderHeight(DWORD dwHeight);
		void SetHeaderFont(const LOGFONT* pLogFontNew);
		void SetHeaderColumnColor(DWORD nColumn, COLORREF clr);
		DECLARE_MESSAGE_MAP()
	protected:
		void InitHeader() override;
		bool HasTooltip(int iItem, int iSubitem, std::wstring** ppwstrText = nullptr, std::wstring** ppwstrCaption = nullptr);
		bool HasMenu(int iItem, int iSubitem, CMenu** ppMenu = nullptr);
		void DrawItem(LPDRAWITEMSTRUCT) override;
		afx_msg void OnPaint();
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg void OnKillFocus(CWnd* pNewWnd);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
		virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnTimer(UINT_PTR nIDEvent);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
		afx_msg void OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnHdnTrack(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnDestroy();
	private:
		CListExHeader m_stListHeader;
		CFont m_fontList;
		CPen m_penGrid;
		bool m_fTtShown { false };
		HWND m_hwndTt { };
		TOOLINFO m_stToolInfo { };
		LVHITTESTINFO m_stCurrCell { };
		COLORREF m_clrBkNWA { GetSysColor(COLOR_WINDOW) }; //Bk of non working area.
		COLORREF m_clrText { GetSysColor(COLOR_WINDOWTEXT) };
		COLORREF m_clrBkRow1 { GetSysColor(COLOR_WINDOW) };
		COLORREF m_clrBkRow2 { GetSysColor(COLOR_WINDOW) };
		COLORREF m_clrGrid { RGB(220, 220, 220) };
		DWORD m_dwGridWidth { 1 };
		COLORREF m_clrTextSelected { GetSysColor(COLOR_HIGHLIGHTTEXT) };
		COLORREF m_clrBkSelected { GetSysColor(COLOR_HIGHLIGHT) };
		COLORREF m_clrTooltipText { GetSysColor(COLOR_INFOTEXT) };
		COLORREF m_clrTooltipBk { GetSysColor(COLOR_INFOBK) };
		COLORREF m_clrTextCellTt { GetSysColor(COLOR_WINDOWTEXT) };
		COLORREF m_clrBkCellTt { RGB(170, 170, 230) };
		CMenu* m_pListMenu { };
		//Container for List item's tooltips.
		std::unordered_map<int, std::unordered_map<int, std::tuple<std::wstring/*tip text*/, std::wstring/*caption text*/>>> m_umapCellTt { };
		std::unordered_map<int, std::unordered_map<int, CMenu*>> m_umapCellMenu { };
		std::unordered_map<int, std::unordered_map<int, DWORD_PTR>> m_umapCellData { };
		NMITEMACTIVATE m_stNMII { };
		const int ID_TIMER_TOOLTIP { 0x01 };
	};
	
	/****************************************************************************
	* WM_NOTIFY codes (NMHDR.code values)										*
	****************************************************************************/
	constexpr auto LISTEX_MSG_MENUSELECTED = 0x00001000;

	/*******************Setting the manifest for ComCtl32.dll version 6.***********************/
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
}