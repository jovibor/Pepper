/****************************************************************************************************
* Copyright © 2018-2022 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include "CSplitterEx.h"

import Utility;
using namespace util;

class CChildFrame : public CMDIChildWndEx
{
public:
	auto GetWndStatData()->std::vector<SWINDOWSTATUS>&;
	void SetWindowStatus(HWND hWnd, bool fVisible);
	CSplitterEx m_stSplitterMain, m_stSplitterRight, m_stSplitterRightTop, m_stSplitterRightBottom;
	DECLARE_DYNCREATE(CChildFrame)
private:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) override;
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	DECLARE_MESSAGE_MAP()
private:
	bool m_fSplitterCreated { false };
	UINT m_cx { }, m_cy { };
	std::vector<SWINDOWSTATUS> m_vecWndStatus { };
};