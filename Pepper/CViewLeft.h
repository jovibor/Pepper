/****************************************************************************************************
* Copyright © 2018-2022 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include "CTreeEx.h"
#include "CPepperDoc.h"

class CViewLeft : public CView
{
private:
	void OnInitialUpdate()override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)override;
	void OnDraw(CDC*)override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP();
	DECLARE_DYNCREATE(CViewLeft);
private:
	Ilibpe* m_pLibpe { };
	CPepperDoc* m_pMainDoc { };
	CTreeEx m_stTreeMain;
	CImageList m_ImgListRootTree;
	bool m_fCreated { false };
};