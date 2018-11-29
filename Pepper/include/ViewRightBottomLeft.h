#pragma once
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "Hexedit.h"
#include "ListEx.h"
#include "PepperTreeCtrl.h"

class CViewRightBottomLeft : public CScrollView
{
protected:
	DECLARE_DYNCREATE(CViewRightBottomLeft)
	CViewRightBottomLeft() {}
	virtual ~CViewRightBottomLeft() {}
	void OnInitialUpdate() override;
	void OnUpdate(CView*, LPARAM, CObject*) override;
	void OnDraw(CDC* pDC) override; // overridden to draw this view
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
private:
	PCLIBPE_SECURITY_VEC m_vecSec { };
	CHexEdit m_stHexEdit;
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEXINFO m_stListInfo;
	CListEx m_listExportFuncs;
	CListEx m_listImportFuncs;
	CListEx m_listDelayImportFuncs;
	CListEx m_listRelocsDesc;
	CPepperTreeCtrl m_treeResBottom;
	DWORD m_dwFileSummary { };
	CImageList m_imglTreeRes;
	CWnd* m_pActiveList { };
	std::vector<std::tuple<long, long, long>> m_vecResId { };

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


