#pragma once
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "Hexedit.h"
#include "PepperList.h"
#include "PepperTreeCtrl.h"

class CViewRightBottomLeft : public CScrollView
{
protected:
	DECLARE_DYNCREATE(CViewRightBottomLeft)
	CViewRightBottomLeft() {};
	virtual ~CViewRightBottomLeft() {};
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
	Ilibpe* m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CPepperList m_listImportFuncs;
	CPepperList m_listDelayImportFuncs;
	CPepperList m_listExportFuncs;
	CPepperList m_listRelocsDesc;
	CPepperTreeCtrl m_treeResBottom;
	DWORD m_dwFileSummary { };
	CImageList m_imglTreeRes;
	CWnd* m_pActiveList { };
	std::vector<std::tuple<long, long, long>> m_vecResId { };

	int CreateListImportFuncs(UINT dllId);
	int CreateListDelayImportFuncs(UINT dllId);
	int CreateListExportFuncs();
	int CreateListRelocs(UINT blockID);
	int CreateTreeResDir();
	int CreateHexTLS();
	int CreateHexSecuritySertId(unsigned nSertId);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};


