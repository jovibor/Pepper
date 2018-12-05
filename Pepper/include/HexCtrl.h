/****************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/	                *
* This is a HEX control class for MFC based apps, derived from CWnd.		*
* The usage is quite simple:												*
* 1. Construct CHexCtrl object.												*
* 2. Call Create(...) member function to create an instance.				*
* 3. Use one of SetData(...) methods to set actual data to display as hex.	*
* 4. Set window position, if needed, with hexCtrl.SetWindowPos(...).		*
*****************************************************************************/
#pragma once
#include "HexView.h"

class CHexCtrl : public CWnd
{
public:
	CHexCtrl() {}
	virtual ~CHexCtrl() {}
	BOOL Create(CWnd* pParent, const RECT& rect, UINT nID, CFont* pFont = nullptr/*default*/);
	CHexView* GetActiveView() const { return m_pHexView; };
	void SetData(const std::vector<std::byte>& vecData) const;
	void SetData(const std::string& strData) const;
	void SetData(const PBYTE pData, DWORD_PTR dwCount) const;
	void ClearData();
	int SetFont(CFont* pFontNew) const;
	void SetFontSize(UINT nSize) const;
	void SetFontColor(COLORREF clrTextHex, COLORREF clrTextOffset,
		COLORREF clrTextSelected, COLORREF clrBk, COLORREF clrBkSelected) const;
private:
	DECLARE_DYNAMIC(CHexCtrl)
	CHexView* m_pHexView { };
	CFont* m_pFontHexView { };
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};