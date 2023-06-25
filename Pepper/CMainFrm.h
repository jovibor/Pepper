/****************************************************************************************************
* Copyright © 2018-2023 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include <afxcontrolbars.h>

import Utility;
using namespace Utility;

class CMainFrame : public CMDIFrameWndEx
{
public:
	BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
		CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr)override;
	int& GetChildFramesCount();
private:
	afx_msg void OnAppEditmode();
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)override;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg BOOL OnEraseMDIClientBackground(CDC* pDC)override;
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	LRESULT OnTabActivate(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateAppEditmode(CCmdUI* pCmdUI);
	BOOL PreCreateWindow(CREATESTRUCT& cs)override;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_DYNAMIC(CMainFrame);
	DECLARE_MESSAGE_MAP();
private:
	static LRESULT MDIClientProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwData);
	static void MDIClientSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
	inline static CFont m_fontMDIClient;
	inline static int m_iLOGPIXELSY { };
	CMFCToolBar m_wndToolBar;
	CWnd* pWndMBtnCurrDown { };
	int m_iChildFrames { };    //Amount of active child frames.
	bool m_fClosing { false }; //Indicates that the app is closing now, to avoid dialogs' flickering on exit.
};