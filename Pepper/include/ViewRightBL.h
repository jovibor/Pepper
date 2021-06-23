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
#include "TreeEx.h"
#include "FileLoader.h"
#include "Utility.h"

#define HEXCTRL_SHARED_DLL
#include "HexCtrl.h"

using namespace HEXCTRL;
using namespace LISTEX;

class CViewRightBL : public CView
{
	DECLARE_DYNCREATE(CViewRightBL)
protected:
	CViewRightBL() {}
	virtual ~CViewRightBL() {}
	void OnInitialUpdate() override;
	void OnUpdate(CView*, LPARAM, CObject*) override;
	void OnDraw(CDC* pDC) override; // overridden to draw this view
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	int CreateHexDosHeaderEntry(DWORD dwEntry);
	int CreateHexRichHeaderEntry(DWORD dwEntry);
	int CreateHexNtHeaderEntry(); //There is only one entry in this header, no dwEntry needed.
	int CreateHexFileHeaderEntry(DWORD dwEntry);
	int CreateHexOptHeaderEntry(DWORD dwEntry);
	int CreateHexDataDirsEntry(DWORD dwEntry);
	int CreateHexSecHeadersEntry(DWORD dwEntry);
	int CreateHexLCDEntry(DWORD dwEntry);
	int CreateListExportFuncs();
	int CreateListImportEntry(DWORD dwEntry);
	int CreateListDelayImportEntry(DWORD dwEntry);
	int CreateListRelocsEntry(DWORD dwEntry);
	int CreateHexDebugEntry(DWORD dwEntry);
	int CreateTreeResources();
	int CreateHexTLS();
	int CreateHexSecurityEntry(unsigned nSertId);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void OnDocEditMode();
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe;
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CFileLoader* m_pFileLoader { };
	HWND m_hwndActive { };
	IHexCtrlPtr m_stHexEdit { CreateHexCtrl() };
	HEXCREATE m_hcs { };
	LISTEXCREATESTRUCT m_stlcs;
	IListExPtr m_listExportFuncs { CreateListEx() };
	IListExPtr m_listImportEntry { CreateListEx() };
	IListExPtr m_listDelayImportEntry { CreateListEx() };
	IListExPtr m_listRelocsEntry { CreateListEx() };
	CTreeEx m_treeResBottom;
	DWORD m_dwFileInfo { };
	CImageList m_imglTreeRes;
	std::vector<std::tuple<long, long, long>> m_vecResId { };
	LOGFONTW m_lf { }, m_hdrlf { };
};