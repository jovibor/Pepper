#pragma once
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "HexCtrl.h"
#include "ListEx.h"
#include "TreeEx.h"

class CViewRightBL : public CScrollView
{
protected:
	DECLARE_DYNCREATE(CViewRightBL)
	CViewRightBL() {}
	virtual ~CViewRightBL() {}
	void OnInitialUpdate() override;
	void OnUpdate(CView*, LPARAM, CObject*) override;
	void OnDraw(CDC* pDC) override; // overridden to draw this view
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
private:
	PCLIBPE_SECURITY_VEC m_vecSec { };
	CHexCtrl m_stHexEdit;
	CHexCtrl m_hex;
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEXINFO m_stListInfo;
	CListEx m_listExportFuncs;
	CListEx m_listImportEntry;
	CListEx m_listDelayImportEntry;
	CListEx m_listRelocsEntry;
	CTreeEx m_treeResBottom;
	DWORD m_dwFileSummary { };
	CImageList m_imglTreeRes;
	CWnd* m_pActiveWnd { };
	std::vector<std::tuple<long, long, long>> m_vecResId { };
	LOGFONT m_lf { }, m_hdrlf { };

	int CreateListImportEntry(DWORD dwEntry);
	int CreateListDelayImportEntry(DWORD dwEntry);
	int CreateListExportFuncs();
	int CreateListRelocsEntry(DWORD dwEntry);
	int CreateHexDebugEntry(DWORD dwEntry);
	int CreateTreeResources();
	int CreateHexTLS();
	int CreateHexSecurityEntry(unsigned nSertId);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};