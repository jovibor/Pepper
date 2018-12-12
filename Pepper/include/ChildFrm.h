#pragma once
#include "SplitterEx.h"

class CChildFrame : public CMDIChildWndEx
{
public:
	CChildFrame() {};
protected:
	DECLARE_DYNCREATE(CChildFrame)
	virtual ~CChildFrame() {};
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	CSplitterEx m_stSplitterMain, m_stSplitterRight, m_stSplitterRightTop, m_stSplitterRightBottom;
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) override;
	bool m_fSplitterCreated { false };
	UINT m_cx { }, m_cy { };
	DECLARE_MESSAGE_MAP()
};
