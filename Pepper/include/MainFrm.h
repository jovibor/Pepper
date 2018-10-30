#pragma once

class CMainFrame : public CMDIFrameWndEx
{
public:
	DECLARE_DYNAMIC(CMainFrame)
	CMainFrame() {};
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 
		CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr) override;
	virtual ~CMainFrame() {};
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
protected:  // control bar embedded members
	CMFCToolBar       m_wndToolBar;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	DECLARE_MESSAGE_MAP()
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) override;
public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
