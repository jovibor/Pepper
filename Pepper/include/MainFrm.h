/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once

class CMainFrame : public CMDIFrameWndEx
{	
	DECLARE_DYNAMIC(CMainFrame)
	CMainFrame() {}
	virtual ~CMainFrame() {}
	BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 
		CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr) override;
protected:
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) override;
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	CMFCToolBar m_wndToolBar;
	DECLARE_MESSAGE_MAP()
};
