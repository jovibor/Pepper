#pragma once
#include "ListHeaderEx.h"
#include <unordered_map>

class CListEx : public CMFCListCtrl
{
public:
	DECLARE_DYNAMIC(CListEx)
	CListEx();
	virtual ~CListEx() { }
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
		COLORREF clrText = GetSysColor(COLOR_WINDOWTEXT) /*Text color.*/,
		COLORREF clrBk = GetSysColor(COLOR_WINDOW) /*Bk color.*/,
		COLORREF clrSelectedText = GetSysColor(COLOR_HIGHLIGHTTEXT) /*Selected item text color.*/,
		COLORREF clrSelectedBk = GetSysColor(COLOR_HIGHLIGHT) /*Selected item bk color.*/,
		COLORREF clrTooltipText = GetSysColor(COLOR_INFOTEXT) /*Tooltip window text color.*/,
		COLORREF clrTooltipBk = GetSysColor(COLOR_INFOBK) /*Tooltip window bk color.*/,
		COLORREF clrSubitemWithTooltipText = GetSysColor(COLOR_WINDOWTEXT) /*Text color of item that has tooltip.*/,
		COLORREF clrSubitemWithTooltipBk = RGB(170, 170, 230) /*Bk color of item that has tooltip.*/,
		CFont* pFontList = nullptr /*List font — nullptr=default.*/,
		COLORREF clrHdrText = GetSysColor(COLOR_WINDOWTEXT) /*List header text color.*/,
		COLORREF clrHdrBk = GetSysColor(COLOR_WINDOW) /*List header bk color.*/,
		DWORD dwHdrHeight = 19 /*List header height.*/,
		CFont* pFontHdr = nullptr /*List header font, nullptr=default.*/
	);
	CListHeaderEx& GetHeaderCtrl() override { return m_stListHeader; }
	int SetFont(CFont* pFontNew);
	//To remove tooltip from specific subitem just set it again with empty (L"") string.
	void SetItemTooltip(int nItem, int nSubitem, const std::wstring& strTooltip, const std::wstring& strCaption = { });
	void SetHeaderHeight(DWORD dwHeight);
	void SetHeaderColor(COLORREF clrHdrBk, COLORREF clrHdrText);
	int SetHeaderFont(CFont* pFontNew);
	void SetTooltipColor(COLORREF clrTooltipBk /*tooltip window bk color*/, COLORREF clrTooltipText /*tooltip text color*/,
		COLORREF clrSubtemWithTooltip = RGB(170, 170, 230)/*color of item that has tooltip*/);
	DECLARE_MESSAGE_MAP()
private:
	CListHeaderEx m_stListHeader;
	CFont m_fontList;
	CPen m_penForRect { PS_SOLID, 1, RGB(220, 220, 220) };
	//Container for List item's tooltips.
	std::unordered_map<int, std::unordered_map<int, std::tuple< std::wstring/*tip text*/, std::wstring/*caption text*/>>> m_umapTooltip { };
	//Flag that indicates that there is at least one tooltip in list.
	bool m_fTooltipExist { false };
	bool m_fTooltipShown { false };
	HWND m_hwndTooltip { };
	TOOLINFO m_stToolInfo { };
	LVHITTESTINFO m_stCurrentSubitem { };
	COLORREF m_clrText { };
	COLORREF m_clrBk { };
	COLORREF m_clrTextSelected { };
	COLORREF m_clrSelectedBk { };
	COLORREF m_clrSubitemWithTooltipText { };
	COLORREF m_clrSubitemWithTooltipBk { };
	COLORREF m_clrTooltipBk { };
	COLORREF m_clrTooltipText { };
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