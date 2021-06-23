/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once
#include "ChildFrm.h"
#include "PepperDoc.h"

#define HEXCTRL_SHARED_DLL
#include "HexCtrl.h"

using namespace HEXCTRL;

class CViewRightTR : public CView
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
	void OnDocEditMode();
	void CreateHexResources(const IMAGE_RESOURCE_DATA_ENTRY* pRes);
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe;
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CFileLoader* m_pFileLoader { };
	HWND m_hwndActive { };
	IHexCtrlPtr m_stHexEdit { CreateHexCtrl() };
	HEXCREATE m_hcs { };
};