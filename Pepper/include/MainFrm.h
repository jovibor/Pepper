#pragma once

class CMainFrame : public CMDIFrameWndEx
{
protected:
	DECLARE_DYNAMIC(CMainFrame)
	CMainFrame() {}
	virtual ~CMainFrame() {}
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) override;
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 
		CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr) override;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	CMFCToolBar m_wndToolBar;
	DECLARE_MESSAGE_MAP()
};
