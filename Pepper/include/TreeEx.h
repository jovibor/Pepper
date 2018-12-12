#pragma once

class CTreeEx : public CTreeCtrl
{
protected:
	DECLARE_DYNAMIC(CTreeEx)
	CTreeEx() {}
	virtual ~CTreeEx() {}
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};