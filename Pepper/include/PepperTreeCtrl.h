#pragma once

class CPepperTreeCtrl : public CTreeCtrl
{
protected:
	DECLARE_DYNAMIC(CPepperTreeCtrl)
	CPepperTreeCtrl() {}
	virtual ~CPepperTreeCtrl() {}
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};