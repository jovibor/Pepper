/****************************************************************************************************
* Copyright © 2018-2024 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include "CChildFrm.h"
#include "CPepperDoc.h"
#include "CTreeEx.h"
#include "CFileLoader.h"
#include "HexCtrl.h"

import Utility;

using namespace LISTEX;
using namespace HEXCTRL;
using namespace Utility;

class CViewRightTL : public CView {
private:
	struct TOOLTIP {
		UINT_PTR uListID { };
		UINT uID { };
		int iSubItem { };
		std::wstring wstrCaption;
		std::wstring wstrTT;
	};
	auto GetToolTip(UINT_PTR uListID, int iItem, int iSubItem) -> const TOOLTIP*;
	[[nodiscard]] auto GetListByID(UINT_PTR uListID) -> CListEx*;
	void SetToolTip(UINT_PTR uListID, int iItem, int iSubItem, std::wstring_view wsvTT, std::wstring_view wsvCaption = { });
	void OnInitialUpdate()override;
	void OnUpdate(CView*, LPARAM, CObject*)override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)override;
	BOOL OnCommand(WPARAM wParam, LPARAM lParam)override;
	void OnDraw(CDC* pDC)override; // overridden to draw this view.
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnListSecHdrGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListImportGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListRelocsGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListExceptionsGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListSecHdrGetToolTip(NMHDR *pNMHDR, LRESULT *pResult);
	void OnListGetColor(NMHDR* pNMHDR);
	void OnListGetToolTip(NMHDR* pNMHDR);
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

	//Resource id for given treeCtrl node.
	//Used in treeCtrl.SetItemData(...), so that we associate tree item with the corresponding 
	//resource's index (lvlRoot, lvl2, lvl3), for future use (in OnNotify()).
	std::vector<std::tuple<long, long, long>> m_vecResId { };
	std::vector<TOOLTIP> m_vecTT;
	HTREEITEM m_hTreeResRoot { };
	HWND m_hWndActive { };
	LOGFONTW m_lf { };
	LOGFONTW m_hdrlf { };
	CMenu m_menuList;
	CFileLoader* m_pFileLoader { };
	UINT_PTR m_iListID { }; //List ID munu clicked at.
	int m_iListItem { };    //List item munu clicked at.
	int m_iListSubItem { }; //List SubItem menu clicked at.
	bool m_fFileSummaryShow { true };
};