/****************************************************************************************************
* Copyright © 2018-2023 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include <afxcontrolbars.h>
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