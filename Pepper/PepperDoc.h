/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once
#include "libpe.h"
#include "CFileLoader.h"

using namespace libpe;

class CPepperDoc : public CDocument
{
public:
	CFileLoader m_stFileLoader;
	void SetEditMode(bool fEditMode);
	[[nodiscard]] bool IsEditMode() { return m_fEditMode; }
	[[nodiscard]] Ilibpe* GetLibpe() { return &*m_pLibpe; }
private:
	BOOL OnOpenDocument(LPCTSTR lpszPathName)override;
	afx_msg void OnFileClose();
	void OnCloseDocument()override;
	afx_msg void OnUpdateResExtractAllCur(CCmdUI *pCmdUI);
	afx_msg void OnUpdateResExtractAllIco(CCmdUI *pCmdUI);
	afx_msg void OnUpdateResExtractAllBmp(CCmdUI *pCmdUI);
	afx_msg void OnUpdateResExtractAllPng(CCmdUI *pCmdUI);
	afx_msg void OnResExtractAllCur();
	afx_msg void OnResExtractAllIco();
	afx_msg void OnResExtractAllBmp();
	afx_msg void OnResExtractAllPng();
	DECLARE_DYNCREATE(CPepperDoc);
	DECLARE_MESSAGE_MAP();
private:
	std::wstring m_wstrDocName; //Opened document name.
	IlibpePtr m_pLibpe { Createlibpe() };
	bool m_fEditMode { false };
	bool m_fHasCur { false };
	bool m_fHasIco { false };
	bool m_fHasBmp { false };
	bool m_fHasPng { false };
};