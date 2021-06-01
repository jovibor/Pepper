/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once
#include "TreeEx.h"
#include "PepperDoc.h"

class CViewLeft : public CView
{
public:
	CViewLeft() {} // protected constructor used by dynamic creation
	virtual ~CViewLeft() {}
	DECLARE_DYNCREATE(CViewLeft)
protected:
	void OnInitialUpdate() override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
	void OnDraw(CDC*) override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe;
	CPepperDoc* m_pMainDoc { };
	CTreeEx m_stTreeMain;
	CImageList m_ImgListRootTree;
	bool m_fCreated { false };
};


