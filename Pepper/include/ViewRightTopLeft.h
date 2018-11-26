#pragma once
#include "Hexedit.h"
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "ListEx.h"
#include "PepperTreeCtrl.h"

class CViewRightTopLeft : public CScrollView
{	
protected:	
	DECLARE_DYNCREATE(CViewRightTopLeft)
	CViewRightTopLeft() {} // protected constructor used by dynamic creation
	virtual ~CViewRightTopLeft() {}	
	void OnInitialUpdate() override;
	void OnUpdate(CView*, LPARAM, CObject*) override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
	void OnDraw(CDC* pDC) override; // overridden to draw this view
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnListSectionsGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListImportGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListRelocGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListExceptionGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
private:
	std::wstring m_strVersion { };
	SIZE m_sizeTextToDraw { };
	CFont m_fontSummary;
	std::wstring m_strFileName { };
	std::wstring m_strFileType { };
	Ilibpe * m_pLibpe { };
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
	CListEx m_listImportDir;
	CListEx m_listExceptionDir;
	CListEx m_listSecurityDir;
	CListEx m_listRelocDir;
	CListEx m_listDebugDir;
	CListEx m_listLoadConfigDir;
	CListEx m_listBoundImportDir;
	CListEx m_listDelayImportDir;
	CListEx m_listTLSDir;
	CListEx m_listCOMDir;
	CPepperTreeCtrl m_treeResTop;
	HTREEITEM m_hTreeResDir { };
	bool m_fFileSummaryShow { true };
	ULONG m_dwPeStart { };
	DWORD m_dwFileSummary { };
	PCLIBPE_SECHEADERS_VEC m_pSecHeaders { };
	PCLIBPE_IMPORT_VEC m_pImportTable { };
	PCLIBPE_EXCEPTION_VEC m_pExceptionDir { };
	PCLIBPE_RELOCATION_VEC m_pRelocTable { };
	CWnd* m_pActiveList { };
	std::vector<std::tuple<long, long, long>> m_vecResId { }; //Resource id for treeCtrl

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
	int CreateListException();
	int CreateListSecurity();
	int CreateListRelocation();
	int CreateListDebug();
	int CreateListTLS();
	int CreateListLoadConfigTable();
	int CreateListBoundImport();
	int CreateListDelayImport();
	int CreateListCOM();
};