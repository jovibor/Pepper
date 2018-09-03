#pragma once
#include "HexEditView.h"

class CHexEdit : public CWnd
{
	DECLARE_DYNAMIC(CHexEdit)
public:
	CHexEdit() {};
	virtual ~CHexEdit() {};
	BOOL Create(CWnd* pParent, const RECT& rect, UINT nID, CFont* pFont = nullptr/*default*/);
	CHexEditView* GetActiveView() { return m_pHexEditView; };
	BOOL SetData(const std::vector<std::byte> *vecData);
	BOOL SetData(const std::string_view strData, UINT nCount = 0);
	CFont* SetFont(CFont* pFont);
	void SetFontSize(UINT nSize);
	void SetFontColor(COLORREF clrHex, COLORREF clrOffset);
private:
	CHexEditView* m_pHexEditView { };
	CFont* m_pFontHexEditView { };
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};


