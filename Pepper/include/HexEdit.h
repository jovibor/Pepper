#pragma once
#include "HexEditView.h"

class CHexEdit : public CWnd
{
public:
	CHexEdit() {}
	virtual ~CHexEdit() {}
	BOOL Create(CWnd* pParent, const RECT& rect, UINT nID, CFont* pFont = nullptr/*default*/);
	CHexEditView* GetActiveView() const { return m_pHexEditView; };
	BOOL SetData(const std::vector<std::byte> *pVecData) const;
	BOOL SetData(const std::string& strData) const;
	BOOL SetData(const PBYTE pData, DWORD_PTR dwCount) const;
	int SetFont(CFont* pFontNew) const;
	void SetFontSize(UINT nSize) const;
	void SetFontColor(COLORREF clrTextHex, COLORREF clrTextOffset,
		COLORREF clrTextSelected, COLORREF clrBk, COLORREF clrBkSelected) const;
private:
	DECLARE_DYNAMIC(CHexEdit)
	CHexEditView* m_pHexEditView { };
	CFont* m_pFontHexEditView { };
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};