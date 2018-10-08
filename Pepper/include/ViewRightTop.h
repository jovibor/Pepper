#pragma once
#include "PepperTreeCtrl.h"
#include "PepperList.h"

class CViewRightTop : public CScrollView
{
public:
	virtual void OnDraw(CDC* pDC); // overridden to draw this view
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	DECLARE_DYNCREATE(CViewRightTop)
protected:
	CViewRightTop() {}; // protected constructor used by dynamic creation
	virtual ~CViewRightTop() {};
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
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
	CPepperList m_listDOSRich;
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
	CPepperTreeCtrl m_treeResourceDirTop;
	bool m_fFileSummaryShow = false;
	LONG m_dwPeStart { };
	DWORD m_dwFileSummary { };
	LONG m_ScrollWidth { };
	LONG m_ScrollHeight { };
	std::map<DWORD, std::wstring> m_mapSecFlags { };
	PCLIBPE_SECHEADER_VEC m_pSectionHeaders { };
	PCLIBPE_IMPORT_VEC m_pImportTable { };
	PCLIBPE_EXCEPTION_VEC m_pExceptionDir { };
	PCLIBPE_RELOCATION_VEC m_pRelocTable { };
	CWnd* m_pActiveList { };

	int listCreateDOSHeader();
	int listCreateDOSRich();
	int listCreateNTHeader();
	int listCreateFileHeader();
	int listCreateOptHeader();
	int listCreateDataDirs();
	int listCreateSections();
	int listCreateExportDir();
	int listCreateImportDir();
	int treeCreateResourceDir();
	int listCreateExceptionDir();
	int listCreateSecurityDir();
	int listCreateRelocDir();
	int listCreateDebugDir();
	int listCreateTLSDir();
	int listCreateLoadConfigDir();
	int listCreateBoundImportDir();
	int listCreateDelayImportDir();
	int listCreateCOMDir();
};