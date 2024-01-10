/****************************************************************************************************
* Copyright © 2018-2024 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include <afxcontrolbars.h>
#include "CFileLoader.h"
import Utility;
import libpe;
using namespace libpe;

class CPepperDoc : public CDocument
{
public:
	[[nodiscard]] auto GetFileLoader() -> CFileLoader&;
	void SetEditMode(bool fEditMode);
	[[nodiscard]] bool IsEditMode() { return m_fEditMode; }
	[[nodiscard]] auto GetFileInfo() -> Utility::PEFILEINFO&;
	[[nodiscard]] auto GetOffsetFromVA(ULONGLONG ullVA) -> DWORD;
	[[nodiscard]] auto GetOffsetFromRVA(ULONGLONG ullRVA) -> DWORD;
	[[nodiscard]] auto GetDOSHeader() -> std::optional<IMAGE_DOS_HEADER>&;
	[[nodiscard]] auto GetRichHeader() -> std::optional<PERICHHDR_VEC>&;
	[[nodiscard]] auto GetNTHeader() -> std::optional<PENTHDR>&;
	[[nodiscard]] auto GetDataDirs() -> std::optional<PEDATADIR_VEC>&;
	[[nodiscard]] auto GetSecHeaders() -> std::optional<PESECHDR_VEC>&;
	[[nodiscard]] auto GetExport() -> std::optional<PEEXPORT>&;
	[[nodiscard]] auto GetImport() -> std::optional<PEIMPORT_VEC>&;
	[[nodiscard]] auto GetResources() -> std::optional<PERESROOT>&;
	[[nodiscard]] auto GetExceptions() -> std::optional<PEEXCEPTION_VEC>&;
	[[nodiscard]] auto GetSecurity() -> std::optional<PESECURITY_VEC>&;
	[[nodiscard]] auto GetRelocations() -> std::optional<PERELOC_VEC>&;
	[[nodiscard]] auto GetDebug() -> std::optional<PEDEBUG_VEC>&;
	[[nodiscard]] auto GetTLS() -> std::optional<PETLS>&;
	[[nodiscard]] auto GetLoadConfig() -> std::optional<PELOADCONFIG>&;
	[[nodiscard]] auto GetBoundImport() -> std::optional<PEBOUNDIMPORT_VEC>&;
	[[nodiscard]] auto GetDelayImport() -> std::optional<PEDELAYIMPORT_VEC>&;
	[[nodiscard]] auto GetCOMDescriptor() -> std::optional<PECOMDESCRIPTOR>&;
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
	CFileLoader m_stFileLoader;
	std::wstring m_wstrDocName; //Opened document name.
	std::optional<IMAGE_DOS_HEADER> m_optDOS;
	std::optional<PERICHHDR_VEC> m_optRich;
	std::optional<PENTHDR> m_optNTHdr;
	std::optional<PEDATADIR_VEC> m_optDataDirs;
	std::optional<PESECHDR_VEC> m_optSecHdr;
	std::optional<PEEXPORT> m_optExport;
	std::optional<PEIMPORT_VEC> m_optImport;
	std::optional<PERESROOT> m_optResRoot;
	std::optional<PEEXCEPTION_VEC> m_optExcept;
	std::optional<PESECURITY_VEC> m_optSecurity;
	std::optional<PERELOC_VEC> m_optReloc;
	std::optional<PEDEBUG_VEC> m_optDebug;
	std::optional<PETLS> m_optTLS;
	std::optional<PELOADCONFIG> m_optLCD;
	std::optional<PEBOUNDIMPORT_VEC> m_optBoundImp;
	std::optional<PEDELAYIMPORT_VEC> m_optDelayImp;
	std::optional<PECOMDESCRIPTOR> m_optComDescr;
	Utility::PEFILEINFO m_stFileInfo { };
	bool m_fEditMode { false };
	bool m_fHasCur { false };
	bool m_fHasIco { false };
	bool m_fHasBmp { false };
	bool m_fHasPng { false };
};