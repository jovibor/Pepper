/****************************************************************************************************
* Copyright © 2018-2022 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#include "stdafx.h"
#include "res/resource.h"
#include "MainFrm.h"
#include "PepperDoc.h"
#include <format>

import Utility;
using namespace util;

IMPLEMENT_DYNCREATE(CPepperDoc, CDocument)

BEGIN_MESSAGE_MAP(CPepperDoc, CDocument)
	ON_COMMAND(ID_FILE_CLOSE, &CPepperDoc::OnFileClose)
	ON_UPDATE_COMMAND_UI(IDM_RES_EXTRACTALLCUR, &CPepperDoc::OnUpdateResExtractAllCur)
	ON_UPDATE_COMMAND_UI(IDM_RES_EXTRACTALLICO, &CPepperDoc::OnUpdateResExtractAllIco)
	ON_UPDATE_COMMAND_UI(IDM_RES_EXTRACTALLBMP, &CPepperDoc::OnUpdateResExtractAllBmp)
	ON_UPDATE_COMMAND_UI(IDM_RES_EXTRACTALLPNG, &CPepperDoc::OnUpdateResExtractAllPng)
	ON_COMMAND(IDM_RES_EXTRACTALLCUR, &CPepperDoc::OnResExtractAllCur)
	ON_COMMAND(IDM_RES_EXTRACTALLICO, &CPepperDoc::OnResExtractAllIco)
	ON_COMMAND(IDM_RES_EXTRACTALLBMP, &CPepperDoc::OnResExtractAllBmp)
	ON_COMMAND(IDM_RES_EXTRACTALLPNG, &CPepperDoc::OnResExtractAllPng)
END_MESSAGE_MAP()

BOOL CPepperDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	m_wstrDocName = lpszPathName;
	m_wstrDocName = m_wstrDocName.substr(m_wstrDocName.find_last_of(L'\\') + 1); //Doc name with .extension.

	if (const auto err = m_pLibpe->LoadPe(lpszPathName); err != PEOK) {
		m_wstrDocName += L" File Load Failed.";
		const auto it = g_mapLibpeErrors.find(err);
		MessageBoxW(nullptr, std::vformat(L"File load failed with libpe error code: 0x{:04X}\n{}",
			std::make_wformat_args(err, it != g_mapLibpeErrors.end() ? it->second : L"N/A")).data(),
			m_wstrDocName.data(), MB_ICONERROR);

		return FALSE;
	}

	if (const auto pRes = GetLibpe()->GetResources(); pRes != nullptr) {
		const auto vecRes = Ilibpe::FlatResources(*pRes);
		for (const auto& ref : vecRes) {
			if (ref.wTypeID == 1) { //RT_CURSOR
				m_fHasCur = true;
				continue;
			}
			if (ref.wTypeID == 2) { //RT_BITMAP
				m_fHasBmp = true;
				continue;
			}
			if (ref.wTypeID == 3) { //RT_ICON
				m_fHasIco = true;
				continue;
			}
			if (ref.wsvTypeStr == L"PNG") { //PNG
				m_fHasPng = true;
				continue;
			}
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

void CPepperDoc::OnUpdateResExtractAllCur(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_fHasCur);
}

void CPepperDoc::OnUpdateResExtractAllIco(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_fHasIco);
}

void CPepperDoc::OnUpdateResExtractAllBmp(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_fHasBmp);
}

void CPepperDoc::OnUpdateResExtractAllPng(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_fHasPng);
}

void CPepperDoc::OnResExtractAllCur()
{
	ExtractAllResToFile(*GetLibpe(), EResType::RTYPE_CURSOR, m_wstrDocName);
}

void CPepperDoc::OnResExtractAllIco()
{
	ExtractAllResToFile(*GetLibpe(), EResType::RTYPE_ICON, m_wstrDocName);
}

void CPepperDoc::OnResExtractAllBmp()
{
	ExtractAllResToFile(*GetLibpe(), EResType::RTYPE_BITMAP, m_wstrDocName);
}

void CPepperDoc::OnResExtractAllPng()
{
	ExtractAllResToFile(*GetLibpe(), EResType::RTYPE_PNG, m_wstrDocName);
}

void CPepperDoc::SetEditMode(bool fEditMode)
{
	if (!m_stFileLoader.IsWritable()) {
		MessageBoxW(AfxGetMainWnd()->GetSafeHwnd(), L"File cannot be opened for writing.\r\n"
			"Most likely it's already opened by another process.", L"Error open for writing", MB_ICONERROR);
		return;
	}

	if (!IsEditMode()) {
		if (IDYES != MessageBoxW(AfxGetMainWnd()->GetSafeHwnd(), L"Warning!\r\nYou are about to enter the Edit Mode. "
			"In this mode all changes you make will be immediately reflected to the file.\r\n"
			"Are you sure you want it?", L"Edit mode", MB_ICONINFORMATION | MB_YESNO)) {
			return;
		}
	}

	m_fEditMode = fEditMode;
	UpdateAllViews(nullptr, MAKELPARAM(ID_DOC_EDITMODE, fEditMode));
}