/****************************************************************************************************
* Copyright © 2018-2023 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include <afxcontrolbars.h>
#include "HexCtrl.h"

using namespace HEXCTRL;

class CPepperDoc;
class CFileLoader : public CWnd
{
public:
	[[nodiscard]] auto GetData()const->std::span<std::byte>;
	HRESULT LoadFile(LPCWSTR lpszFileName, CPepperDoc* pDoc); //First function to call.
	[[nodiscard]] bool IsWritable()const { return m_fWritable; }

	//Shows arbitrary offset in already loaded file (LoadFile). If pHexCtrl == nullptr the inner IHexCtrl object is used.
	HRESULT ShowOffsetInWholeFile(ULONGLONG ullOffset, ULONGLONG ullSelSize, IHexCtrl* pHexCtrl = nullptr);

	//Shows only a piece of the whole loaded file. If pHexCtrl == nullptr the inner IHexCtrl object is used.
	HRESULT ShowFilePiece(ULONGLONG ullOffset, ULONGLONG ullSize, IHexCtrl* pHexCtrl = nullptr);
	HRESULT UnloadFile(); //Unloads loaded file and all pieces, if present.
private:
	void CreateHexCtrlWnd();
	[[nodiscard]] bool IsLoaded()const;
private:
	static constexpr DWORD m_dwStyle { WS_POPUP | WS_OVERLAPPEDWINDOW };
	static constexpr DWORD m_dwExStyle { WS_EX_APPWINDOW }; //To force to the taskbar.
	struct HEXTODATACHECK {	//Was the given IHexCtrl set with a whole file data or only with a file piece?
		IHexCtrl* pHexCtrl { };
		bool      fWhole { };
	};
	CPepperDoc* m_pMainDoc { };
	IHexCtrlPtr m_pHex { HEXCTRL::CreateHexCtrl() };
	HEXCREATE m_hcs;
	HEXDATA m_hds;
	LARGE_INTEGER m_stFileSize { };	//Size of the loaded PE file.
	HANDLE m_hFile { };      //Returned by CreateFileW.
	HANDLE m_hMapObject { }; //Returned by CreateFileMappingW.
	LPVOID m_lpBase { };     //Returned by MapViewOfFile.
	std::vector<HEXTODATACHECK> m_vecCheck;
	bool m_fLoaded { false };
	bool m_fWritable { false };
};