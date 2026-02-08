/*****************************************************************
* Copyright © 2018-present Jovibor https://github.com/jovibor/   *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.  *
* Official git repository: https://github.com/jovibor/Pepper/    *
* This software is available under the Apache-2.0 License.       *
*****************************************************************/
#pragma once
#include "CSplitterEx.h"

class CChildFrame : public CMDIChildWndEx {
public:
	[[nodiscard]] auto GetSplitRight() -> CSplitterEx&;
	[[nodiscard]] auto GetSplitRightTop() -> CSplitterEx&;
	[[nodiscard]] auto GetSplitRightBot() -> CSplitterEx&;
private:
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)override;
	DECLARE_MESSAGE_MAP();
	DECLARE_DYNCREATE(CChildFrame);
private:
	CSplitterEx m_stSplitterMain;
	CSplitterEx m_stSplitterRight;
	CSplitterEx m_stSplitterRightTop;
	CSplitterEx m_stSplitterRightBottom;
	UINT m_cx { };
	UINT m_cy { };
	bool m_fSplitterCreated { false };
	bool m_fClosing { false }; //Indicates that tab is closing now.
};