#pragma once

class CTreeEx : public CTreeCtrl
{	
	DECLARE_DYNAMIC(CTreeEx)
	CTreeEx() {}
	virtual ~CTreeEx() {}
protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};