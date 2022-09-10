/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "res/resource.h"
#include "PepperDoc.h"
#include "MainFrm.h"
#include "Utility.h"
#include <format>

IMPLEMENT_DYNCREATE(CPepperDoc, CDocument)

BEGIN_MESSAGE_MAP(CPepperDoc, CDocument)
	ON_COMMAND(ID_FILE_CLOSE, &CPepperDoc::OnFileClose)
	ON_UPDATE_COMMAND_UI(IDM_RES_EXTRACTCUR, &CPepperDoc::OnUpdateResExtractCur)
	ON_UPDATE_COMMAND_UI(IDM_RES_EXTRACTICO, &CPepperDoc::OnUpdateResExtractIco)
	ON_UPDATE_COMMAND_UI(IDM_RES_EXTRACTBMP, &CPepperDoc::OnUpdateResExtractBmp)
	ON_COMMAND(IDM_RES_EXTRACTCUR, &CPepperDoc::OnResExtractCur)
	ON_COMMAND(IDM_RES_EXTRACTICO, &CPepperDoc::OnResExtractIco)
	ON_COMMAND(IDM_RES_EXTRACTBMP, &CPepperDoc::OnResExtractBmp)
END_MESSAGE_MAP()

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

	if (const auto pRes = GetLibpe()->GetResources(); pRes != nullptr) {
		const auto vecRes = GetLibpe()->FlatResources(*pRes);
		for (const auto& ref : vecRes) {
			if (ref.wTypeID == 1) //RT_CURSOR
				m_fHasCur = true;
			if (ref.wTypeID == 2) //RT_BITMAP
				m_fHasBmp = true;
			if (ref.wTypeID == 3) //RT_ICON
				m_fHasIco = true;
		}
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

void CPepperDoc::OnFileClose()
{
	reinterpret_cast<CMainFrame*>(AfxGetMainWnd())->MDIGetActive()->SendMessageW(WM_CLOSE);
}

void CPepperDoc::OnUpdateResExtractCur(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_fHasCur);
}

void CPepperDoc::OnUpdateResExtractIco(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_fHasIco);
}

void CPepperDoc::OnUpdateResExtractBmp(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_fHasBmp);
}

void CPepperDoc::OnResExtractCur()
{
	if (CFolderPickerDialog fd; fd.DoModal() == IDOK) {
		std::wstring wstrDocName = GetPathName().GetString();
		wstrDocName = wstrDocName.substr(wstrDocName.find_last_of(L'\\') + 1); //Doc name with .extension.
		const auto wstrPath = fd.GetPathName(); //Folder name.

		if (const auto pRes = GetLibpe()->GetResources(); pRes != nullptr) {
			const auto vecRes = GetLibpe()->FlatResources(*pRes);
			auto iIndex { 0 };
			bool fAllSaveOK { true };
			for (const auto& ref : vecRes) {
				if (ref.wTypeID == 1) { //RT_CURSOR
					if (!SaveIconCur(std::format(L"{}\\{}_{:04}.cur", wstrPath.GetString(), wstrDocName, ++iIndex).data(), ref.spnData, false)) {
						fAllSaveOK = false;
					}
				}
			}
			if (fAllSaveOK) {
				MessageBoxW(nullptr, std::format(L"All {} cursors were saved successfully!", iIndex).data(), L"Success", MB_ICONINFORMATION);
			}
			else {
				MessageBoxW(nullptr, std::format(L"Some issues occured during the save process.\r\nOnly {} cursors were saved successfully.", iIndex).data(),
					L"Error", MB_ICONERROR);
			}
		}
	}
}

void CPepperDoc::OnResExtractIco()
{
	if (CFolderPickerDialog fd; fd.DoModal() == IDOK) {
		std::wstring wstrDocName = GetPathName().GetString();
		wstrDocName = wstrDocName.substr(wstrDocName.find_last_of(L'\\') + 1); //Doc name with .extension.
		const auto wstrPath = fd.GetPathName(); //Folder name.

		if (const auto pRes = GetLibpe()->GetResources(); pRes != nullptr) {
			const auto vecRes = GetLibpe()->FlatResources(*pRes);
			auto iIndex { 0 };
			bool fAllSaveOK { true };
			for (const auto& ref : vecRes) {
				if (ref.wTypeID == 3) { //RT_ICON
					if (!SaveIconCur(std::format(L"{}\\{}_{:04}.ico", wstrPath.GetString(), wstrDocName, ++iIndex).data(), ref.spnData)) {
						fAllSaveOK = false;
					}
				}
			}
			if (fAllSaveOK) {
				MessageBoxW(nullptr, std::format(L"All {} icons were saved successfully!", iIndex).data(), L"Success", MB_ICONINFORMATION);
			}
			else {
				MessageBoxW(nullptr, std::format(L"Some issues occured during the save process.\r\nOnly {} icons were saved successfully.", iIndex).data(),
					L"Error", MB_ICONERROR);
			}
		}
	}
}

void CPepperDoc::OnResExtractBmp()
{
	if (CFolderPickerDialog fd; fd.DoModal() == IDOK) {
		std::wstring wstrDocName = GetPathName().GetString();
		wstrDocName = wstrDocName.substr(wstrDocName.find_last_of(L'\\') + 1); //Doc name with .extension.
		const auto wstrPath = fd.GetPathName(); //Folder name.

		if (const auto pRes = GetLibpe()->GetResources(); pRes != nullptr) {
			const auto vecRes = GetLibpe()->FlatResources(*pRes);
			auto iIndex { 0 };
			bool fAllSaveOK { true };
			for (const auto& ref : vecRes) {
				if (ref.wTypeID == 2) { //RT_BITMAP
					if (!SaveBitmap(std::format(L"{}\\{}_{:04}.bmp", wstrPath.GetString(), wstrDocName, ++iIndex).data(), ref.spnData)) {
						fAllSaveOK = false;
					}
				}
			}
			if (fAllSaveOK) {
				MessageBoxW(nullptr, std::format(L"All {} bitmaps were saved successfully!", iIndex).data(), L"Success", MB_ICONINFORMATION);
			}
			else {
				MessageBoxW(nullptr, std::format(L"Some issues occured during the save process.\r\nOnly {} bitmaps were saved successfully.", iIndex).data(),
					L"Error", MB_ICONERROR);
			}
		}
	}
}