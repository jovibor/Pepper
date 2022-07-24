/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "PepperDoc.h"
#include "Utility.h"
#include <format>

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
	if (const auto err = m_pLibpe->LoadPe(lpszPathName); err != PEOK)
	{
		std::wstring wstrFileName = lpszPathName;
		if (const auto sSlash = wstrFileName.find_last_of(L'\\'); sSlash > 0)
			wstrFileName = wstrFileName.substr(sSlash + 1);
		wstrFileName += L" File Load Failed.";
		const auto it = g_mapLibpeErrors.find(err);
		MessageBoxW(nullptr, std::vformat(L"File load failed with libpe error code: 0x{:04X}\n{}",
			std::make_wformat_args(err, it != g_mapLibpeErrors.end() ? it->second : L"N/A")).data(),
			wstrFileName.data(), MB_ICONERROR);

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