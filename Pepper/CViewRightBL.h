/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "ListEx/ListEx.h"
#include "CTreeEx.h"
#include "CFileLoader.h"
#include "HexCtrl.h"

import Utility;

using namespace HEXCTRL;
using namespace LISTEX;

class CViewRightBL : public CView
{
private:
	void OnInitialUpdate()override;
	void OnUpdate(CView*, LPARAM, CObject*)override;
	void OnDraw(CDC* pDC)override; // overridden to draw this view
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void CreateListExportFuncs();
	void CreateListImportFuncs();
	void CreateTreeResources();
	void CreateListRelocsEntry();
	void CreateListDelayImpFuncs();
	void ShowDosHdrHexEntry(DWORD dwEntry);
	void ShowRichHdrHexEntry(DWORD dwEntry);
	void ShowNtHdrHexEntry(); //There is only one entry in this header, no dwEntry needed.
	void ShowFileHdrHexEntry(DWORD dwEntry);
	void ShowOptHdrHexEntry(DWORD dwEntry);
	void ShowDataDirsHexEntry(DWORD dwEntry);
	void ShowSecHdrHexEntry(DWORD dwEntry);
	void ShowLCDHexEntry(DWORD dwEntry);
	void ShowImportListEntry(DWORD dwEntry);
	void ShowDelayImpListEntry(DWORD dwEntry);
	void ShowRelocsListEntry(DWORD dwEntry);
	void ShowDebugHexEntry(DWORD dwEntry);
	void ShowTLSHex();
	void ShowSecurityHexEntry(unsigned nSertId);
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)override;
	BOOL PreCreateWindow(CREATESTRUCT& cs)override;
	void OnDocEditMode();
	DECLARE_MESSAGE_MAP()
		DECLARE_DYNCREATE(CViewRightBL)
private:
	Ilibpe* m_pLibpe { };
	PEFILEINFO stFileInfo;
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CFileLoader* m_pFileLoader { };
	HWND m_hwndActive { };
	IHexCtrlPtr m_stHexEdit { CreateHexCtrl() };
	HEXCREATE m_hcs { };
	LISTEXCREATE m_stlcs;
	IListExPtr m_listExportFuncs { CreateListEx() };
	IListExPtr m_listImportEntry { CreateListEx() };
	IListExPtr m_listDelayImportEntry { CreateListEx() };
	IListExPtr m_listRelocsEntry { CreateListEx() };
	CTreeEx m_treeResBottom;
	CImageList m_imglTreeRes;
	std::vector<std::tuple<long, long, long>> m_vecResId { };
	LOGFONTW m_lf { }, m_hdrlf { };
};