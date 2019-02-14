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

using namespace HEXCTRL;
using namespace LISTEX;

class CViewRightBL : public CScrollView
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
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CWnd* m_pActiveWnd { };
	CHexCtrl m_stHexEdit;
	CFileLoader* m_pFileLoader { };
	CFileLoader m_stFileLoader;
	std::vector<std::byte> m_vecDebug;
	LISTEXINFO m_stListInfo;
	CListEx m_listExportFuncs;
	CListEx m_listImportEntry;
	CListEx m_listDelayImportEntry;
	CListEx m_listRelocsEntry;
	CTreeEx m_treeResBottom;
	DWORD m_dwFileSummary { };
	CImageList m_imglTreeRes;
	std::vector<std::tuple<long, long, long>> m_vecResId { };
	LOGFONT m_lf { }, m_hdrlf { };
	RESHELPER m_stResHelper { };
	PCLIBPE_SECURITY_VEC m_vecSec { };

	int CreateListImportEntry(DWORD dwEntry);
	int CreateListDelayImportEntry(DWORD dwEntry);
	int CreateListExportFuncs();
	int CreateListRelocsEntry(DWORD dwEntry);
	int CreateHexDebugEntry(DWORD dwEntry);
	int CreateTreeResources();
	int CreateHexTLS();
	int CreateHexSecurityEntry(unsigned nSertId);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};