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

class CPepperDoc : public CDocument {
public:
	[[nodiscard]] auto GetFileLoader() -> CFileLoader&;
	void SetEditMode(bool fEditMode);
	[[nodiscard]] bool IsEditMode()const { return m_fEditMode; }
	[[nodiscard]] auto GetFileInfo() -> ut::PEFILEINFO&;
	[[nodiscard]] auto GetOffsetFromVA(ULONGLONG ullVA) -> DWORD;
	[[nodiscard]] auto GetOffsetFromRVA(ULONGLONG ullRVA) -> DWORD;
	[[nodiscard]] auto GetDOSHeader() -> std::optional<IMAGE_DOS_HEADER>&;
	[[nodiscard]] auto GetRichHeader() -> std::optional<libpe::PERICHHDR_VEC>&;
	[[nodiscard]] auto GetNTHeader() -> std::optional<libpe::PENTHDR>&;
	[[nodiscard]] auto GetDataDirs() -> std::optional<libpe::PEDATADIR_VEC>&;
	[[nodiscard]] auto GetSecHeaders() -> std::optional<libpe::PESECHDR_VEC>&;
	[[nodiscard]] auto GetExport() -> std::optional<libpe::PEEXPORT>&;
	[[nodiscard]] auto GetImport() -> std::optional<libpe::PEIMPORT_VEC>&;
	[[nodiscard]] auto GetResources() -> std::optional<libpe::PERESROOT>&;
	[[nodiscard]] auto GetExceptions() -> std::optional<libpe::PEEXCEPTION_VEC>&;
	[[nodiscard]] auto GetSecurity() -> std::optional<libpe::PESECURITY_VEC>&;
	[[nodiscard]] auto GetRelocations() -> std::optional<libpe::PERELOC_VEC>&;
	[[nodiscard]] auto GetDebug() -> std::optional<libpe::PEDEBUG_VEC>&;
	[[nodiscard]] auto GetTLS() -> std::optional<libpe::PETLS>&;
	[[nodiscard]] auto GetLoadConfig() -> std::optional<libpe::PELOADCONFIG>&;
	[[nodiscard]] auto GetBoundImport() -> std::optional<libpe::PEBOUNDIMPORT_VEC>&;
	[[nodiscard]] auto GetDelayImport() -> std::optional<libpe::PEDELAYIMPORT_VEC>&;
	[[nodiscard]] auto GetCOMDescriptor() -> std::optional<libpe::PECOMDESCRIPTOR>&;
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
	std::optional<libpe::PERICHHDR_VEC> m_optRich;
	std::optional<libpe::PENTHDR> m_optNTHdr;
	std::optional<libpe::PEDATADIR_VEC> m_optDataDirs;
	std::optional<libpe::PESECHDR_VEC> m_optSecHdr;
	std::optional<libpe::PEEXPORT> m_optExport;
	std::optional<libpe::PEIMPORT_VEC> m_optImport;
	std::optional<libpe::PERESROOT> m_optResRoot;
	std::optional<libpe::PEEXCEPTION_VEC> m_optExcept;
	std::optional<libpe::PESECURITY_VEC> m_optSecurity;
	std::optional<libpe::PERELOC_VEC> m_optReloc;
	std::optional<libpe::PEDEBUG_VEC> m_optDebug;
	std::optional<libpe::PETLS> m_optTLS;
	std::optional<libpe::PELOADCONFIG> m_optLCD;
	std::optional<libpe::PEBOUNDIMPORT_VEC> m_optBoundImp;
	std::optional<libpe::PEDELAYIMPORT_VEC> m_optDelayImp;
	std::optional<libpe::PECOMDESCRIPTOR> m_optComDescr;
	ut::PEFILEINFO m_stFileInfo { };
	bool m_fEditMode { false };
	bool m_fHasCur { false };
	bool m_fHasIco { false };
	bool m_fHasBmp { false };
	bool m_fHasPng { false };
};