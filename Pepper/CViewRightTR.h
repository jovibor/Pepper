/****************************************************************************************************
* Copyright © 2018-2022 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include "CChildFrm.h"
#include "CPepperDoc.h"
#include "HexCtrl.h"

using namespace HEXCTRL;

class CViewRightTR : public CView
{
private:
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void OnDocEditMode();
	void OnMDITabActivate(bool fActivate);
	void CreateHexResources(const IMAGE_RESOURCE_DATA_ENTRY* pRes);
	DECLARE_MESSAGE_MAP();
	DECLARE_DYNCREATE(CViewRightTR);
private:
	Ilibpe* m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CFileLoader* m_pFileLoader { };
	HWND m_hwndActive { };
	IHexCtrlPtr m_stHexEdit { CreateHexCtrl() };
	HEXCREATE m_hcs { };
	std::vector<HWND> m_vecHWNDVisible;
};