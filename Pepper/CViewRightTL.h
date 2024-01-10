/****************************************************************************************************
* Copyright © 2018-2024 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include "CChildFrm.h"
#include "CPepperDoc.h"
#include "ListEx/ListEx.h"
#include "CTreeEx.h"
#include "CFileLoader.h"
#include "HexCtrl.h"

import Utility;

using namespace LISTEX;
using namespace HEXCTRL;
using namespace Utility;

class CViewRightTL : public CView
{
private:
	void OnInitialUpdate()override;
	void OnUpdate(CView*, LPARAM, CObject*)override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)override;
	BOOL OnCommand(WPARAM wParam, LPARAM lParam)override;
	void OnDraw(CDC* pDC)override; // overridden to draw this view.
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnListSecHdrGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
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
	DECLARE_DYNCREATE(CViewRightTL);
	DECLARE_MESSAGE_MAP();
	void CreateListDOSHeader();
	void CreateListRichHeader();
	void CreateListNTHeader();
	void CreateListFileHeader();
	void CreateListOptHeader();
	void CreateListDataDirs();
	void CreateListSecHeaders();
	void CreateListExport();
	void CreateListImport();
	void CreateTreeResources();
	void CreateListExceptions();
	void CreateListSecurity();
	void CreateListRelocations();
	void CreateListDebug();
	void CreateListTLS();
	void CreateListLCD();
	void CreateListBoundImport();
	void CreateListDelayImport();
	void CreateListCOM();
	void SortImportData();
private:
	PEFILEINFO m_stFileInfo;
	CFont m_fontSummary;
	std::wstring m_wstrPepperVersion { };
	std::wstring m_wstrFileName { };
	std::wstring m_wstrFileType { };
	std::wstring m_wstrFullPath { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEXCREATE m_stlcs;
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
	HTREEITEM m_hTreeResRoot { };
	CWnd* m_pwndActive { };
	LOGFONTW m_lf { };
	LOGFONTW m_hdrlf { };
	CMenu m_menuList;
	CFileLoader* m_pFileLoader { };
	UINT_PTR m_iListID { }; //List ID munu clicked at.
	int m_iListItem { };    //List item munu clicked at.
	int m_iListSubItem { }; //List SubItem menu clicked at.
	bool m_fFileSummaryShow { true };
};