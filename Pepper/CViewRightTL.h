/*****************************************************************
* Copyright © 2018-present Jovibor https://github.com/jovibor/   *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.  *
* Official git repository: https://github.com/jovibor/Pepper/    *
* This software is available under the Apache-2.0 License.       *
*****************************************************************/
#pragma once
#include "CChildFrm.h"
#include "CPepperDoc.h"
#include "CTreeEx.h"
#include "CFileLoader.h"
#include "HexCtrl.h"

import Utility;

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
	[[nodiscard]] auto GetListByID(UINT_PTR uListID) -> LISTEX::CListEx*;
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
	ut::PEFILEINFO m_stFileInfo;
	CFont m_fontSummary;
	std::wstring m_wstrPepperVersion;
	std::wstring m_wstrFileName;
	std::wstring m_wstrFileType;
	std::wstring m_wstrFullPath;
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEX::LISTEXCREATE m_stlcs;
	LISTEX::CListEx m_listDOSHeader;
	LISTEX::CListEx m_listRichHdr;
	LISTEX::CListEx m_listNTHeader;
	LISTEX::CListEx m_listFileHeader;
	LISTEX::CListEx m_listOptHeader;
	LISTEX::CListEx m_listDataDirs;
	LISTEX::CListEx m_listSecHeaders;
	LISTEX::CListEx m_listExportDir;
	LISTEX::CListEx m_listImport;
	LISTEX::CListEx m_listExceptionDir;
	LISTEX::CListEx m_listSecurityDir;
	LISTEX::CListEx m_listRelocDir;
	LISTEX::CListEx m_listDebugDir;
	LISTEX::CListEx m_listLCD;
	LISTEX::CListEx m_listBoundImportDir;
	LISTEX::CListEx m_listDelayImportDir;
	LISTEX::CListEx m_listTLSDir;
	LISTEX::CListEx m_listCOMDir;
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