/*****************************************************************
* Copyright © 2018-present Jovibor https://github.com/jovibor/   *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.  *
* Official git repository: https://github.com/jovibor/Pepper/    *
* This software is available under the Apache-2.0 License.       *
*****************************************************************/
#pragma once
#include "CPepperDoc.h"
#include "CTreeEx.h"

class CViewLeft : public CView {
private:
	void OnInitialUpdate()override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)override;
	void OnDraw(CDC*)override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP();
	DECLARE_DYNCREATE(CViewLeft);
private:
	CPepperDoc* m_pMainDoc { };
	CTreeEx m_stTreeMain;
	CImageList m_ImgListRootTree;
	bool m_fCreated { false };
};