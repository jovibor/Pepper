/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "HexCtrl.h"
#include "ListEx/ListEx.h"
#include "TreeEx.h"
#include "FileLoader.h"
#include "constants.h"

using namespace LISTEX;
using namespace HEXCTRL;

class CViewRightTL : public CView
{
	DECLARE_DYNCREATE(CViewRightTL)
protected:
	CViewRightTL() {}
	virtual ~CViewRightTL() {}
	void OnInitialUpdate() override;
	void OnUpdate(CView*, LPARAM, CObject*) override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)override;
	BOOL OnCommand(WPARAM wParam, LPARAM lParam)override;
	void OnDraw(CDC* pDC) override; // overridden to draw this view.
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnListSectionsGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListImportGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListRelocsGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListExceptionsGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListSecHdrGetToolTip(NMHDR *pNMHDR, LRESULT *pResult);
	void OnListExportMenuSelect(WORD wMenuID);
	void OnListImportMenuSelect(WORD wMenuID);
	void OnListTLSMenuSelect(WORD wMenuID);
	void OnListBoundImpMenuSelect(WORD wMenuID);
	void OnListCOMDescMenuSelect(WORD wMenuID);
	afx_msg void OnTreeResTopSelChange(NMHDR *pNMHDR, LRESULT *pResult);
	void SortImportData();
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe;
	CFont m_fontSummary;
	std::wstring m_wstrPepperVersion { };
	std::wstring m_wstrFileName { };
	std::wstring m_wstrFileType { };
	std::wstring m_wstrFullPath { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEXCREATESTRUCT m_stlcs;
	IListExPtr m_listDOSHeader { CreateListEx() };
	IListExPtr m_listRichHdr { CreateListEx() };
	IListExPtr m_listNTHeader { CreateListEx() };
	IListExPtr m_listFileHeader { CreateListEx() };
	IListExPtr m_listOptHeader { CreateListEx() };
	IListExPtr m_listDataDirs { CreateListEx() };
	IListExPtr m_listSecHeaders { CreateListEx() };
	IListExPtr m_listExportDir { CreateListEx() };
	IListExPtr m_listImport { CreateListEx() };
	IListExPtr m_listExceptionDir { CreateListEx() };
	IListExPtr m_listSecurityDir { CreateListEx() };
	IListExPtr m_listRelocDir { CreateListEx() };
	IListExPtr m_listDebugDir { CreateListEx() };
	IListExPtr m_listLCD { CreateListEx() };
	IListExPtr m_listBoundImportDir { CreateListEx() };
	IListExPtr m_listDelayImportDir { CreateListEx() };
	IListExPtr m_listTLSDir { CreateListEx() };
	IListExPtr m_listCOMDir { CreateListEx() };
	CTreeEx m_treeResTop;

	//Resource id for given treeCtrl node.
	//Used in treeCtrl.SetItemData(...), so that we associate tree item with the corresponding 
	//resource's index (lvlRoot, lvl2, lvl3), for future use (in OnNotify()).
	std::vector<std::tuple<long, long, long>> m_vecResId { };
	HTREEITEM m_hTreeResDir { };
	bool m_fFileSummaryShow { true };
	ULONG m_dwPeStart { };
	DWORD m_dwFileInfo { };
	PLIBPE_SECHEADERS_VEC m_pSecHeaders { };
	PLIBPE_IMPORT_VEC m_pImport { };
	PLIBPE_EXCEPTION_VEC m_pExceptionDir { };
	PLIBPE_RELOCATION_VEC m_pRelocTable { };
	CWnd* m_pwndActive { };
	LOGFONTW m_lf { }, m_hdrlf { };
	CMenu m_menuList;
	CFileLoader* m_pFileLoader { };
	UINT_PTR m_iListID { };      //List ID munu clicked at.
	int m_iListItem { };    //List item munu clicked at.
	int m_iListSubItem { }; //List SubItem menu clicked at.

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
	int CreateListLCD();
	int CreateListBoundImport();
	int CreateListDelayImport();
	int CreateListCOM();
};