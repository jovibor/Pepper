/****************************************************************************************************
* Copyright © 2018-2023 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#include "stdafx.h"
#include "res/resource.h"
#include "CMainFrm.h"
#include "CPepperDoc.h"
#include <format>

import Utility;
using namespace Utility;

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
	m_wstrDocName = m_wstrDocName.substr(m_wstrDocName.find_last_of(L'\\') + 1); //Doc name with the .extension.
	const std::wstring wstrErrCaption = L"File load failed: " + m_wstrDocName;

	if (m_stFileLoader.LoadFile(lpszPathName, this) != S_OK) {
		MessageBoxW(nullptr, L"File load failed.", wstrErrCaption.data(), MB_ICONERROR);
		return FALSE;
	}

	libpe::Clibpe libPE;
	if (const auto err = libPE.OpenFile(m_stFileLoader.GetData()); err != libpe::PEOK) {
		const auto it = g_mapLibpeErrors.find(err);
		MessageBoxW(nullptr, std::vformat(L"File load failed with the libpe error code: 0x{:04X}\n{}",
			std::make_wformat_args(err, it != g_mapLibpeErrors.end() ? it->second : L"N/A")).data(),
			wstrErrCaption.data(), MB_ICONERROR);
		return FALSE;
	}

	m_optDOS = libPE.GetDOSHeader();
	m_stFileInfo.fHasDosHdr = m_optDOS.has_value();
	m_optRich = libPE.GetRichHeader();
	m_stFileInfo.fHasRichHdr = m_optRich.has_value();
	m_optNTHdr = libPE.GetNTHeader();
	m_stFileInfo.fHasNTHdr = m_optNTHdr.has_value();
	m_optDataDirs = libPE.GetDataDirs();
	m_stFileInfo.fHasDataDirs = m_optDataDirs.has_value();
	m_optSecHdr = libPE.GetSecHeaders();
	m_stFileInfo.fHasSections = m_optSecHdr.has_value();
	m_optExport = libPE.GetExport();
	m_stFileInfo.fHasExport = m_optExport.has_value();
	m_optImport = libPE.GetImport();
	m_stFileInfo.fHasImport = m_stFileInfo.fHasIAT = m_optImport.has_value();
	m_optResRoot = libPE.GetResources();
	m_stFileInfo.fHasResource = m_optResRoot.has_value();
	m_optExcept = libPE.GetExceptions();
	m_stFileInfo.fHasException = m_optExcept.has_value();
	m_optSecurity = libPE.GetSecurity();
	m_stFileInfo.fHasSecurity = m_optSecurity.has_value();
	m_optReloc = libPE.GetRelocations();
	m_stFileInfo.fHasReloc = m_optReloc.has_value();
	m_optDebug = libPE.GetDebug();
	m_stFileInfo.fHasDebug = m_optDebug.has_value();
	m_optTLS = libPE.GetTLS();
	m_stFileInfo.fHasTLS = m_optTLS.has_value();
	m_optLCD = libPE.GetLoadConfig();
	m_stFileInfo.fHasLoadCFG = m_optLCD.has_value();
	m_optBoundImp = libPE.GetBoundImport();
	m_stFileInfo.fHasBoundImp = m_optBoundImp.has_value();
	m_optDelayImp = libPE.GetDelayImport();
	m_stFileInfo.fHasDelayImp = m_optDelayImp.has_value();
	m_optComDescr = libPE.GetCOMDescriptor();
	m_stFileInfo.fHasCOMDescr = m_optComDescr.has_value();

	if (m_optNTHdr) {
		m_stFileInfo.eFileType = libpe::GetFileType(*m_optNTHdr);
	}

	if (const auto pRes = GetResources(); pRes) {
		const auto vecRes = FlatResources(*pRes);
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

	UpdateAllViews(nullptr);

	return TRUE;
}

void CPepperDoc::OnCloseDocument()
{
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
	ExtractAllResToFile(GetResources(), EResType::RTYPE_CURSOR, m_wstrDocName);
}

void CPepperDoc::OnResExtractAllIco()
{
	ExtractAllResToFile(GetResources(), EResType::RTYPE_ICON, m_wstrDocName);
}

void CPepperDoc::OnResExtractAllBmp()
{
	ExtractAllResToFile(GetResources(), EResType::RTYPE_BITMAP, m_wstrDocName);
}

void CPepperDoc::OnResExtractAllPng()
{
	ExtractAllResToFile(GetResources(), EResType::RTYPE_PNG, m_wstrDocName);
}

auto CPepperDoc::GetFileLoader()->CFileLoader&
{
	return m_stFileLoader;
}

void CPepperDoc::SetEditMode(bool fEditMode)
{
	if (!m_stFileLoader.IsWritable()) {
		MessageBoxW(AfxGetMainWnd()->GetSafeHwnd(), L"File cannot be opened for writing.\r\n"
			"Most likely it's already opened by another process.", L"Error open for writing", MB_ICONERROR);
		return;
	}

	if (!IsEditMode()) {
		if (IDYES != MessageBoxW(AfxGetMainWnd()->GetSafeHwnd(), L"Warning!\r\nYou are about to enter the Edit Mode.\r\n"
			"In this mode all the changes you make will be immediately reflected into the file!\r\n\r\n"
			"Are you sure you want to proceed?", L"Edit Mode", MB_ICONWARNING | MB_YESNO)) {
			return;
		}
	}

	m_fEditMode = fEditMode;
	UpdateAllViews(nullptr, MAKELPARAM(ID_DOC_EDITMODE, fEditMode));
}

auto CPepperDoc::GetFileInfo()->PEFILEINFO&
{
	return m_stFileInfo;
}

auto CPepperDoc::GetOffsetFromVA(ULONGLONG ullVA)->DWORD
{
	const auto ullRVA = ullVA - libpe::GetImageBase(*GetNTHeader());
	return libpe::GetOffsetFromRVA(ullRVA, *GetSecHeaders());
}

auto CPepperDoc::GetOffsetFromRVA(ULONGLONG ullRVA)->DWORD
{
	return libpe::GetOffsetFromRVA(ullRVA, *GetSecHeaders());
}

auto CPepperDoc::GetDOSHeader()->std::optional<IMAGE_DOS_HEADER>&
{
	return m_optDOS;
}

auto CPepperDoc::GetRichHeader()->std::optional<PERICHHDR_VEC>&
{
	return m_optRich;
}

auto CPepperDoc::GetNTHeader()->std::optional<PENTHDR>&
{
	return m_optNTHdr;
}

auto CPepperDoc::GetDataDirs()->std::optional<PEDATADIR_VEC>&
{
	return m_optDataDirs;
}

auto CPepperDoc::GetSecHeaders()->std::optional<PESECHDR_VEC>&
{
	return m_optSecHdr;
}

auto CPepperDoc::GetExport()->std::optional<PEEXPORT>&
{
	return m_optExport;
}

auto CPepperDoc::GetImport()->std::optional<PEIMPORT_VEC>&
{
	return m_optImport;
}

auto CPepperDoc::GetResources()->std::optional<PERESROOT>&
{
	return m_optResRoot;
}

auto CPepperDoc::GetExceptions()->std::optional<PEEXCEPTION_VEC>&
{
	return m_optExcept;
}

auto CPepperDoc::GetSecurity()->std::optional<PESECURITY_VEC>&
{
	return m_optSecurity;
}

auto CPepperDoc::GetRelocations()->std::optional<PERELOC_VEC>&
{
	return m_optReloc;
}

auto CPepperDoc::GetDebug()->std::optional<PEDEBUG_VEC>&
{
	return m_optDebug;
}

auto CPepperDoc::GetTLS()->std::optional<PETLS>&
{
	return m_optTLS;
}

auto CPepperDoc::GetLoadConfig()->std::optional<PELOADCONFIG>&
{
	return m_optLCD;
}

auto CPepperDoc::GetBoundImport()->std::optional<PEBOUNDIMPORT_VEC>&
{
	return m_optBoundImp;
}

auto CPepperDoc::GetDelayImport()->std::optional<PEDELAYIMPORT_VEC>&
{
	return m_optDelayImp;
}

auto CPepperDoc::GetCOMDescriptor()->std::optional<PECOMDESCRIPTOR>&
{
	return m_optComDescr;
}