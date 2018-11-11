#pragma once
#include "Hexedit.h"
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "PepperList.h"
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
	CPepperList m_listDOSHeader;
	CPepperList m_listRichHdr;
	CPepperList m_listNTHeader;
	CPepperList m_listFileHeader;
	CPepperList m_listOptHeader;
	CPepperList m_listDataDirs;
	CPepperList m_listSecHeaders;
	CPepperList m_listExportDir;
	CPepperList m_listImportDir;
	CPepperList m_listExceptionDir;
	CPepperList m_listSecurityDir;
	CPepperList m_listRelocDir;
	CPepperList m_listDebugDir;
	CPepperList m_listLoadConfigDir;
	CPepperList m_listBoundImportDir;
	CPepperList m_listDelayImportDir;
	CPepperList m_listTLSDir;
	CPepperList m_listCOMDir;
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
	int CreateListRichHdr();
	int CreateListNTHeader();
	int CreateListFileHeader();
	int CreateListOptHeader();
	int CreateListDataDirs();
	int CreateListSecHdrs();
	int CreateListExportDir();
	int CreateListImportDir();
	int CreateTreeResDir();
	int CreateListExceptionDir();
	int CreateListSecurityDir();
	int CreateListRelocDir();
	int CreateListDebugDir();
	int CreateListTLSDir();
	int CreateListLoadConfigDir();
	int CreateListBoundImportDir();
	int CreateListDelayImportDir();
	int CreateListCOMDir();
};