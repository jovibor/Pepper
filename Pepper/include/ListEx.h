/****************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/	                *
* This is an extended and quite featured version of CMFCListCtrl class.		*
* The main difference is in CListEx::Create method, which takes one			*
* additional arg: pointer to LISTEXINFO structure, which fields are			*
* described below.															*
* Also, this class has set of additional methods: SetFont, SetItemTooltip,	*
* SetTooltipColor, SetHeaderColor, SetHeaderHeight,	SetHeaderFont,			*
* SetHeaderColumnColor.														*
****************************************************************************/
#pragma once
#include <unordered_map>

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
	COLORREF clrListTextTooltip { GetSysColor(COLOR_INFOTEXT) }; //Tooltip window text color.
	COLORREF clrListBkTooltip { GetSysColor(COLOR_INFOBK) }; //Tooltip window bk color.
	COLORREF clrListTextCellTt { GetSysColor(COLOR_WINDOWTEXT) }; //Text color of a cell that has tooltip.
	COLORREF clrListBkCellTt { RGB(170, 170, 230) }; //Bk color of a cell that has tooltip.
	const LOGFONT* pListLogFont { nullptr }; //List font.
	COLORREF clrHeaderText { GetSysColor(COLOR_WINDOWTEXT) }; //List header text color.
	COLORREF clrHeaderBk { GetSysColor(COLOR_WINDOW) }; //List header bk color.
	DWORD dwHeaderHeight { 19 }; //List header height.
	const LOGFONT* pHeaderLogFont { nullptr }; //List header font.
} *PLISTEXINFO;

/********************************************
* CListEx class definition.					*
********************************************/
class CListEx : public CMFCListCtrl
{
private:
	/********************************************
	* CListExHeader class definition.			*
	********************************************/
	class CListExHeader : public CMFCHeaderCtrl
	{
	public:
		void SetHeight(DWORD dwHeight);
		void SetFont(const LOGFONT* pFontNew);
		void SetColor(COLORREF clrText, COLORREF clrBk);
		void SetColumnColor(DWORD nColumn, COLORREF clr);
		CListExHeader();
		virtual ~CListExHeader() {}
	protected:
		afx_msg void OnDrawItem(CDC* pDC, int iItem, CRect rect, BOOL bIsPressed, BOOL bIsHighlighted) override;
		afx_msg LRESULT OnLayout(WPARAM wParam, LPARAM lParam);
		DECLARE_MESSAGE_MAP()
		CFont m_fontHdr;
		COLORREF m_clrText { GetSysColor(COLOR_WINDOWTEXT) };
		COLORREF m_clrBk { GetSysColor(COLOR_WINDOW) };
		HDITEMW m_hdItem { }; //For drawing.
		WCHAR m_strHeaderText[MAX_PATH] { };
		DWORD m_dwHeaderHeight { 19 }; //Standard (default) height.
		std::unordered_map<DWORD, COLORREF> m_mapClrColumn { }; //Color of individual columns.
	};
/////////////////////////////////////////////////////////////////////////////////
public:
	DECLARE_DYNAMIC(CListEx)
	CListEx() {}
	virtual ~CListEx() {}
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, PLISTEXINFO pInfo = nullptr);
	CListExHeader& GetHeaderCtrl() override { return m_stListHeader; }
	void SetFont(const LOGFONT* pLogFontNew);
	//To remove tooltip from specific subitem just set it again with empty (L"") string.
	void SetItemTooltip(int nItem, int nSubitem, const std::wstring& strTooltip, const std::wstring& strCaption = { });
	void SetTooltipColor(COLORREF clrTooltipText, COLORREF clrTooltipBk,
		COLORREF clrTextCellTt = GetSysColor(COLOR_WINDOWTEXT), //Text color of a cell that has tooltip.
		COLORREF clrBkCellTt = RGB(170, 170, 230)); //Bk color of a cell that has tooltip.
	void SetHeaderColor(COLORREF clrHdrText, COLORREF clrHdrBk);
	void SetHeaderHeight(DWORD dwHeight);
	void SetHeaderFont(const LOGFONT* pLogFontNew);
	void SetHeaderColumnColor(DWORD nColumn, COLORREF clr);
	DECLARE_MESSAGE_MAP()
private:
	CListExHeader m_stListHeader;
	CFont m_fontList;
	CPen m_penGrid;
	//Container for List item's tooltips.
	std::unordered_map<int, std::unordered_map<int, std::tuple< std::wstring/*tip text*/, std::wstring/*caption text*/>>> m_umapTooltip { };
	//Flag that indicates that there is at least one tooltip in list.
	bool m_fTooltipExist { false };
	bool m_fTooltipShown { false };
	HWND m_hwndTooltip { };
	TOOLINFO m_stToolInfo { };
	LVHITTESTINFO m_stCurrentSubitem { };
	COLORREF m_clrBackground { GetSysColor(COLOR_WINDOW) };
	COLORREF m_clrText { GetSysColor(COLOR_WINDOWTEXT) };
	COLORREF m_clrBkRow1 { GetSysColor(COLOR_WINDOW) };
	COLORREF m_clrBkRow2 { GetSysColor(COLOR_WINDOW) };
	COLORREF m_clrGrid { RGB(220, 220, 220) };
	DWORD m_dwGridWidth { 1 };
	COLORREF m_clrTextSelected { GetSysColor(COLOR_HIGHLIGHTTEXT) };
	COLORREF m_clrBkSelected { GetSysColor(COLOR_HIGHLIGHT) };
	COLORREF m_clrTextTooltip { GetSysColor(COLOR_INFOTEXT) };
	COLORREF m_clrBkTooltip { GetSysColor(COLOR_INFOBK) };
	COLORREF m_clrTextSubitemTt { GetSysColor(COLOR_WINDOWTEXT) };
	COLORREF m_clrBkSubitemTt { RGB(170, 170, 230) };

	void InitHeader() override;
	bool HasTooltip(int, int, std::wstring** ppStrTipText = nullptr, std::wstring** ppStrTipCaption = nullptr);
	void DrawItem(LPDRAWITEMSTRUCT) override;
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
};

constexpr auto ID_TIMER_TOOLTIP = 0x01;