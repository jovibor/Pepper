/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
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
	libpe_ptr m_pLibpe {};
	CFileLoader m_stFileLoader;
private:
	CPepperDoc() {}
	virtual ~CPepperDoc() {}
	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	virtual void OnCloseDocument();
	DECLARE_MESSAGE_MAP()
};