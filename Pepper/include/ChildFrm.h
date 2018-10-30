#pragma once

class CChildFrame : public CMDIChildWndEx
{
public:
	DECLARE_DYNCREATE(CChildFrame)
	CChildFrame() {};
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	virtual ~CChildFrame() {};
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CSplitterWndEx m_MainSplitter, m_RightSplitter;
protected:
	bool m_fSpliterCreated = false;
	DECLARE_MESSAGE_MAP()
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) override;
	UINT m_cx { }, m_cy { };
};
