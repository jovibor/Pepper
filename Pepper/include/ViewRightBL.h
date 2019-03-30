/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "HexCtrl.h"
#include "ListEx.h"
#include "TreeEx.h"
#include "FileLoader.h"
#include "constants.h"

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
	int CreateHexNtHeaderEntry(DWORD dwEntry);
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
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CFileLoader* m_pFileLoader { };
	CWnd* m_pActiveWnd { };
	CHexCtrl m_stHexEdit;
	HEXCREATESTRUCT m_hcs { };
	LISTEXINFO m_stListInfo;
	CListEx m_listExportFuncs;
	CListEx m_listImportEntry;
	CListEx m_listDelayImportEntry;
	CListEx m_listRelocsEntry;
	CTreeEx m_treeResBottom;
	DWORD m_dwFileInfo { };
	CImageList m_imglTreeRes;
	std::vector<std::tuple<long, long, long>> m_vecResId { };
	LOGFONT m_lf { }, m_hdrlf { };
	RESHELPER m_stResHelper { };
};