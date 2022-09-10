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
	afx_msg void OnUpdateResExtractCur(CCmdUI *pCmdUI);
	afx_msg void OnUpdateResExtractIco(CCmdUI *pCmdUI);
	afx_msg void OnUpdateResExtractBmp(CCmdUI *pCmdUI);
	afx_msg void OnResExtractCur();
	afx_msg void OnResExtractIco();
	afx_msg void OnResExtractBmp();
	DECLARE_DYNCREATE(CPepperDoc);
	DECLARE_MESSAGE_MAP();
private:
	IlibpePtr m_pLibpe { Createlibpe() };
	bool m_fEditMode { false };
	bool m_fHasCur { false };
	bool m_fHasIco { false };
	bool m_fHasBmp { false };
};