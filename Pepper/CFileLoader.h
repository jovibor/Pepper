/****************************************************************************************************
* Copyright © 2018-2022 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include "HexCtrl.h"

using namespace HEXCTRL;

class CPepperDoc;
class CFileLoader : public CWnd
{
public:
	HRESULT LoadFile(LPCWSTR lpszFileName, CPepperDoc* pDoc); //First function to call.
	[[nodiscard]] bool IsWritable()const { return m_fWritable; }

	//Shows arbitrary offset in already loaded file (LoadFile). If pHexCtrl == nullptr the inner IHexCtrl object is used.
	HRESULT ShowOffsetInWholeFile(ULONGLONG ullOffset, ULONGLONG ullSelSize, IHexCtrl* pHexCtrl = nullptr);

	//Shows only a piece of the whole loaded file. If pHexCtrl == nullptr the inner CHexCtrl object is used.
	HRESULT ShowFilePiece(ULONGLONG ullOffset, ULONGLONG ullSize, IHexCtrl* pHexCtrl = nullptr);
	[[nodiscard]] bool IsModified()const { return m_fModified; } //Has file been modified in memory or not.
	bool Flush();         //Writes memory mapped file on disk.
	HRESULT UnloadFile(); //Unloads loaded file and all pieces, if present.
private:
	[[nodiscard]] bool IsLoaded()const;
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void CreateHexCtrlWnd();
private:
	//Does given IHexCtrl set with a whole file data or only with a file piece.
	struct HEXTODATACHECK {
		IHexCtrl* pHexCtrl { };
		bool      fWhole { };
	};
	bool m_fLoaded { false };
	bool m_fModified { false };
	bool m_fWritable { false };
	CPepperDoc* m_pMainDoc { };
	IHexCtrlPtr m_pHex { HEXCTRL::CreateHexCtrl() };
	HEXCREATE m_hcs;
	HEXDATA m_hds;
	const DWORD m_dwStyle { WS_POPUP | WS_OVERLAPPEDWINDOW };
	const DWORD m_dwExStyle { WS_EX_APPWINDOW }; //To force to the taskbar.
	LARGE_INTEGER m_stFileSize { };	 //Size of the loaded PE file.
	HANDLE m_hFile { };
	HANDLE m_hMapObject { };	     //Returned by CreateFileMappingW.
	LPVOID m_lpBase { };
	std::vector<HEXTODATACHECK> m_vecCheck;
};