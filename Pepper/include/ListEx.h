/******************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/	                  *
* This is an extended and quite featured version of CMFCListCtrl class.		  *
* The main difference is in Create() method, which takes one additional arg:  *
* pointer to LISTEXINFO structure, which fields are described below.          *
* Also, this class has set of additional methods, such as: SetFont(),		  *
* SetItemTooltip(), SetTooltipColor(), SetHeaderColor(), SetHeaderHeight(),   *
* SetHeaderFont(). It's implemented as «ownerdraw» list.					  *
******************************************************************************/
#pragma once
#include "ListHeaderEx.h"
#include <unordered_map>

typedef struct LISTEXINFO {
	COLORREF clrListText { GetSysColor(COLOR_WINDOWTEXT) }; //List text color.
	COLORREF clrListBk { GetSysColor(COLOR_WINDOW) }; //List Bk color.
	COLORREF clrListGrid { RGB(220, 220, 220) }; //List grid color.
	DWORD dwListGridWidth { 1 }; //Width of the list's grid.
	COLORREF clrListTextSelected { GetSysColor(COLOR_HIGHLIGHTTEXT) }; //Selected item text color.
	COLORREF clrListBkSelected { GetSysColor(COLOR_HIGHLIGHT) }; //Selected item bk color.
	COLORREF clrListTooltipText { GetSysColor(COLOR_INFOTEXT) }; //Tooltip window text color.
	COLORREF clrListTooltipBk { GetSysColor(COLOR_INFOBK) }; //Tooltip window bk color.
	COLORREF clrListSubitemWithTooltipText = { GetSysColor(COLOR_WINDOWTEXT) }; //Text color of item that has tooltip.
	COLORREF clrListSubitemWithTooltipBk = { RGB(170, 170, 230) }; //Bk color of item that has tooltip.
	CFont* pListFont { nullptr }; //List font, nullptr=default.
	COLORREF clrHeaderText { GetSysColor(COLOR_WINDOWTEXT) }; //List header text color.
	COLORREF clrHeaderBk { GetSysColor(COLOR_WINDOW) }; //List header bk color.
	DWORD dwHeaderHeight { 19 }; //List header height.
	CFont* pHeaderFont { nullptr }; //List header font, nullptr=default.
} *PLISTEXINFO;

class CListEx : public CMFCListCtrl
{
public:
	DECLARE_DYNAMIC(CListEx)
	CListEx();
	virtual ~CListEx() { }
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, PLISTEXINFO pInfo = nullptr);
	CListHeaderEx& GetHeaderCtrl() override { return m_stListHeader; }
	int SetFont(CFont* pFontNew);
	//To remove tooltip from specific subitem just set it again with empty (L"") string.
	void SetItemTooltip(int nItem, int nSubitem, const std::wstring& strTooltip, const std::wstring& strCaption = { });
	void SetTooltipColor(COLORREF clrTooltipText, COLORREF clrTooltipBk, COLORREF clrTextItemTt = GetSysColor(COLOR_WINDOWTEXT),
		COLORREF clrBkItemTt = RGB(170, 170, 230));
	void SetHeaderColor(COLORREF clrHdrText, COLORREF clrHdrBk);
	void SetHeaderHeight(DWORD dwHeight);
	int SetHeaderFont(CFont* pFontNew);
	DECLARE_MESSAGE_MAP()
private:
	CListHeaderEx m_stListHeader;
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
	COLORREF m_clrText { GetSysColor(COLOR_WINDOWTEXT) };
	COLORREF m_clrBk { GetSysColor(COLOR_WINDOW) };
	COLORREF m_clrGrid { RGB(220, 220, 220) };
	DWORD m_dwGridWidth { 1 };
	COLORREF m_clrTextSelected { GetSysColor(COLOR_HIGHLIGHTTEXT) };
	COLORREF m_clrBkSelected { GetSysColor(COLOR_HIGHLIGHT) };
	COLORREF m_clrTooltipText { GetSysColor(COLOR_INFOTEXT) };
	COLORREF m_clrTooltipBk { GetSysColor(COLOR_INFOBK) };
	COLORREF m_clrTextItemTt { GetSysColor(COLOR_WINDOWTEXT) };
	COLORREF m_clrBkItemTt { RGB(170, 170, 230) };
	COLORREF m_clrHeaderText { GetSysColor(COLOR_WINDOWTEXT) };
	COLORREF m_clrHeaderBk { GetSysColor(COLOR_WINDOW) };
	DWORD m_dwHeaderHeight { 19 };
	SCROLLINFO m_stScrollInfo { sizeof(SCROLLINFO), SIF_ALL };

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
};

constexpr auto ID_TIMER_TOOLTIP = 0x01;