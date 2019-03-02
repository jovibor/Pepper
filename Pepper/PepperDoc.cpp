/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "PepperDoc.h"
#include "constants.h"

IMPLEMENT_DYNCREATE(CPepperDoc, CDocument)

BEGIN_MESSAGE_MAP(CPepperDoc, CDocument)
END_MESSAGE_MAP()

BOOL CPepperDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (Getlibpe(m_pLibpe) != S_OK)	{
		MessageBoxW(nullptr, L"Getlibpe() failed.", L"Error", MB_ICONERROR);
		return FALSE;
	}

	HRESULT hr;
	if ((hr = m_pLibpe->LoadPe(lpszPathName)) != S_OK)
	{
		WCHAR wstr[MAX_PATH];
		const auto it = g_mapLibpeErrors.find(hr);
		if (it != g_mapLibpeErrors.end())
			swprintf_s(wstr, L"File load failed with libpe error code: 0x0%X\n%s", hr, it->second.data());
		else
			swprintf_s(wstr, L"File load failed with libpe error code: 0x0%X", hr);

		MessageBoxW(nullptr, wstr, L"File load failed.", MB_ICONERROR);

		return FALSE;
	}

	m_stFileLoader.LoadFile(lpszPathName);
	UpdateAllViews(nullptr);

	return TRUE;
}

void CPepperDoc::OnCloseDocument()
{
	m_stFileLoader.UnloadFile();

	CDocument::OnCloseDocument();
}