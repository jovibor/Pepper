#pragma once

class CPepperListHeader : public CMFCHeaderCtrl
{
protected:
	DECLARE_DYNAMIC(CPepperListHeader)
	CPepperListHeader();
	virtual ~CPepperListHeader() {}
	afx_msg void OnDrawItem(CDC* pDC, int iItem, CRect rect, BOOL bIsPressed, BOOL bIsHighlighted) override;
	afx_msg LRESULT OnLayout(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	CFont m_fontHeader;
	COLORREF m_colorHeader { RGB(0, 132, 132) };
	HDITEMW m_hdItem { };
	WCHAR m_strHeaderText[MAX_PATH] { };
};