/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "PepperDoc.h"
#include "constants.h"

IMPLEMENT_DYNCREATE(CPepperDoc, CDocument)

void CPepperDoc::SetEditMode(bool fEditMode)
{
	if (!m_stFileLoader.IsWritable())
	{
		MessageBoxW(AfxGetMainWnd()->GetSafeHwnd(), L"File cannot be opened for writing.\r\n"
			"Most likely it's already opened by another process.", L"Error open for writing", MB_ICONERROR);
		return;
	}

	if (!IsEditMode())
	{
		if (IDYES != MessageBoxW(AfxGetMainWnd()->GetSafeHwnd(), L"Warning!\r\nYou are about to enter the Edit Mode. "
			"In this mode all changes you make will be immediately reflected to the file.\r\n"
			"Are you sure you want it?", L"Edit mode", MB_ICONINFORMATION | MB_YESNO))
		{
			return;
		}
	}
	m_fEditMode = fEditMode;
	UpdateAllViews(nullptr, MAKELPARAM(ID_DOC_EDITMODE, fEditMode));
}

BOOL CPepperDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!(m_pLibpe = Createlibpe())) {
		MessageBoxW(nullptr, L"Createlibpe() failed.", L"Error", MB_ICONERROR);
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

	m_stFileLoader.LoadFile(lpszPathName, this);
	UpdateAllViews(nullptr);

	return TRUE;
}

void CPepperDoc::OnCloseDocument()
{
	m_stFileLoader.Flush();
	m_stFileLoader.UnloadFile();

	CDocument::OnCloseDocument();
}