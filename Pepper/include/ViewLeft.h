#pragma once
#include "TreeEx.h"
#include "PepperDoc.h"

class CViewLeft : public CView
{
public:
	CViewLeft() {} // protected constructor used by dynamic creation
	virtual ~CViewLeft() {}
	DECLARE_DYNCREATE(CViewLeft)
protected:
	void OnInitialUpdate() override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
	void OnDraw(CDC*) override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
private:
	libpe_ptr m_pLibpe { };
	CPepperDoc* m_pMainDoc { };
	CTreeEx m_stTreeMain;
	CImageList m_ImgListRootTree;
};


