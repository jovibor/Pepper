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
#include "FileLoader.h"

using namespace libpe;

class CPepperDoc : public CDocument
{
public:
	DECLARE_DYNCREATE(CPepperDoc)
	libpe_ptr m_pLibpe { };
	CFileLoader m_stFileLoader;
	void SetEditMode(bool fEditMode);
	bool IsEditMode() { return m_fEditMode; }
private:
	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	virtual void OnCloseDocument();
	bool m_fEditMode { false };
};