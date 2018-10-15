#pragma once

class CMainFrame : public CMDIFrameWndEx
{
public:
	DECLARE_DYNAMIC(CMainFrame)
	CMainFrame() {};
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 
		CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);
	virtual ~CMainFrame() {};
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
protected:  // control bar embedded members
	CMFCToolBar       m_wndToolBar;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
