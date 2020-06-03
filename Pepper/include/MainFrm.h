/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once
#include "constants.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
	BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
		CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr)override;
	int& GetChildFramesCount();
	void SetCurrFramePtrNull();
protected:
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)override;
	BOOL PreCreateWindow(CREATESTRUCT& cs)override;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnAppEditmode();
	afx_msg void OnUpdateAppEditmode(CCmdUI *pCmdUI);
	LRESULT OnTabActivate(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
private:
	CMFCToolBar m_wndToolBar;
	CWnd* pWndMBtnCurDown { };
	std::vector<SWINDOWSTATUS>* m_pCurrFrameData { };
	int m_iChildFrames { };    //Amount of active child frames.
	bool m_fClosing { false }; //Indicates that thr app is closing now, to avoid dialogs' flickering on exit.
};
