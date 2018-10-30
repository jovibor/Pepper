#pragma once
#include "PepperTreeCtrl.h"
#include "PepperDoc.h"

class CViewLeft : public CView
{
public:
	DECLARE_DYNCREATE(CViewLeft)
	void OnInitialUpdate() override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
	void OnDraw(CDC* /*pDC*/) override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	CViewLeft() {};           // protected constructor used by dynamic creation
	virtual ~CViewLeft() {};
	DECLARE_MESSAGE_MAP()
private:
	Ilibpe * m_pLibpe { };
	CPepperDoc* m_pMainDoc { };
	CPepperTreeCtrl m_TreeMain;
	CImageList m_ImgListRootTree;
};


