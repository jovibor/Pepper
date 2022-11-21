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
	CSplitterEx m_stSplitterMain;
	CSplitterEx m_stSplitterRight;
	CSplitterEx m_stSplitterRightTop;
	CSplitterEx m_stSplitterRightBottom;
private:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL PreCreateWindow(CREATESTRUCT& cs)override;
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)override;
	DECLARE_MESSAGE_MAP();
	DECLARE_DYNCREATE(CChildFrame);
private:
	bool m_fSplitterCreated { false };
	UINT m_cx { };
	UINT m_cy { };
};