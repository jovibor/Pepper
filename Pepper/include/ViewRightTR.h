/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "HexCtrl.h"

using namespace HEXCTRL;

class CViewRightTR : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightTR)
protected:
	CViewRightTR() {}
	virtual ~CViewRightTR() {}
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CWnd* m_pActiveWnd { };
	CHexCtrl m_stHexEdit;
	HEXCREATESTRUCT m_stHC { };
};