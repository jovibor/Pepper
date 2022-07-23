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
#include "Utility.h"
#include "HexCtrl.h"

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
	void CreateHexDosHeaderEntry(DWORD dwEntry);
	void CreateHexRichHeaderEntry(DWORD dwEntry);
	void CreateHexNtHeaderEntry(); //There is only one entry in this header, no dwEntry needed.
	void CreateHexFileHeaderEntry(DWORD dwEntry);
	void CreateHexOptHeaderEntry(DWORD dwEntry);
	void CreateHexDataDirsEntry(DWORD dwEntry);
	void CreateHexSecHeadersEntry(DWORD dwEntry);
	void CreateHexLCDEntry(DWORD dwEntry);
	void CreateListExportFuncs();
	void CreateListImportEntry(DWORD dwEntry);
	void CreateListDelayImportEntry(DWORD dwEntry);
	void CreateListRelocsEntry(DWORD dwEntry);
	void CreateHexDebugEntry(DWORD dwEntry);
	void CreateTreeResources();
	void CreateHexTLS();
	void CreateHexSecurityEntry(unsigned nSertId);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
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