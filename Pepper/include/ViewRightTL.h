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
#include "ListEx.h"
#include "TreeEx.h"
#include "HexCtrl.h"
#include "FileLoader.h"

using namespace LISTEX;
using namespace HEXCTRL;

class CViewRightTL : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightTL)
protected:
	CViewRightTL() {}
	virtual ~CViewRightTL() {}
	void OnInitialUpdate() override;
	void OnUpdate(CView*, LPARAM, CObject*) override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
	void OnDraw(CDC* pDC) override; // overridden to draw this view.
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnListSectionsGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListImportGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListRelocsGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListExceptionsGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe { };
	SIZE m_sizeTextToDraw { };
	CFont m_fontSummary;
	std::wstring m_wstrAppVersion { };
	std::wstring m_wstrFileName { };
	std::wstring m_wstrFileType { };
	std::wstring m_wstrFullPath { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEXINFO m_stListInfo;
	CListEx m_listDOSHeader;
	CListEx m_listRichHdr;
	CListEx m_listNTHeader;
	CListEx m_listFileHeader;
	CListEx m_listOptHeader;
	CListEx m_listDataDirs;
	CListEx m_listSecHeaders;
	CListEx m_listExportDir;
	CListEx m_listImport;
	CListEx m_listExceptionDir;
	CListEx m_listSecurityDir;
	CListEx m_listRelocDir;
	CListEx m_listDebugDir;
	CListEx m_listLCD;
	CListEx m_listBoundImportDir;
	CListEx m_listDelayImportDir;
	CListEx m_listTLSDir;
	CListEx m_listCOMDir;
	CTreeEx m_treeResTop;
	HTREEITEM m_hTreeResDir { };
	bool m_fFileSummaryShow { true };
	ULONG m_dwPeStart { };
	DWORD m_dwFileSummary { };
	PCLIBPE_SECHEADERS_VEC m_pSecHeaders { };
	PCLIBPE_IMPORT_VEC m_pImport { };
	PCLIBPE_EXCEPTION_VEC m_pExceptionDir { };
	PCLIBPE_RELOCATION_VEC m_pRelocTable { };
	CWnd* m_pActiveWnd { };
	//Resource id for given treeCtrl node.
	//Used in treeCtrl.SetItemData(...), so that we associate tree item
	//with corresponding Resource index (lvlRoot, lvl2, lvl3), for future use (in OnNotify()).
	std::vector<std::tuple<long, long, long>> m_vecResId { };
	LOGFONT m_lf { }, m_hdrlf { };
	CMenu m_menuList;
	CFileLoader* m_pFileLoader { };

	int CreateListDOSHeader();
	int CreateListRichHeader();
	int CreateListNTHeader();
	int CreateListFileHeader();
	int CreateListOptHeader();
	int CreateListDataDirectories();
	int CreateListSecHeaders();
	int CreateListExport();
	int CreateListImport();
	int CreateTreeResources();
	int CreateListExceptions();
	int CreateListSecurity();
	int CreateListRelocations();
	int CreateListDebug();
	int CreateListTLS();
	int CreateListLoadConfigTable();
	int CreateListBoundImport();
	int CreateListDelayImport();
	int CreateListCOM();
};