#pragma once
#include "PepperListHeader.h"
#include <unordered_map>

class CPepperList : public CMFCListCtrl
{
public:
	DECLARE_DYNAMIC(CPepperList)
	CPepperList();
	virtual ~CPepperList() { }
	CMFCHeaderCtrl& GetHeaderCtrl() override { return m_PepperListHeader; }
	void InitHeader() override;
	void SetItemTooltip(int nItem, int nSubitem, const std::wstring& TooltipText, const std::wstring& ToolTipCaption = L"");
	DECLARE_MESSAGE_MAP()
private:
	CPepperListHeader m_PepperListHeader;
	CFont m_fontList;
	CPen m_penForRect { PS_SOLID, 1, RGB(220, 220, 220) };
	//Container for List item's tooltips.
	std::unordered_map<int, std::unordered_map<int, std::tuple< std::wstring/*tip text*/, std::wstring/*caption text*/>>> m_unmapTooltip { };
	//Flag that indicates that there is at least one tooltip in list.
	bool m_fToolTip = false;
	HWND m_hwndToolTip { };
	TOOLINFO m_stToolInfo { };
	bool m_fToolTipShow = false;
	LVHITTESTINFO m_stCurrentSubItem { };
	COLORREF m_colorListToolTipSubitem { }, m_colorListSelected { };
	bool m_fEraseBkgnd = false;
	SCROLLINFO m_stScrollInfo { sizeof(SCROLLINFO), SIF_ALL };

	bool HasToolTip(int, int, std::wstring&, std::wstring&);
	bool HasToolTip(int, int);
	void DrawItem(LPDRAWITEMSTRUCT) override;
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnPaint();
};