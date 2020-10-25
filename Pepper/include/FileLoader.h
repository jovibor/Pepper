/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
/****************************************************************
* This is a helper class for Pepper and HexCtrl routines of		*
* showing files' offsets. It loads (maps) files into memory		*
* by demand and unloads after.									*
* Works with big files - files that can't be loaded fully		*
* at ones - as well.											*
****************************************************************/
#pragma once
#include "HexCtrl.h"

using namespace HEXCTRL;

class CPepperDoc;
class CFileLoader : public CWnd
{
public:
	//First function to call.
	HRESULT LoadFile(LPCWSTR lpszFileName, CPepperDoc* pDoc);
	[[nodiscard]] bool IsWritable()const { return m_fWritable; }
	//Shows arbitrary offset in already loaded file (LoadFile)
	//If pHexCtrl == nullptr inner CHexCtrl object is used.
	HRESULT ShowOffset(ULONGLONG ullOffset, ULONGLONG ullSelSize, IHexCtrl* pHexCtrl = nullptr);
	//Shows only a piece of the whole loaded file.
	//If pHexCtrl == nullptr inner CHexCtrl object is used.
	HRESULT ShowFilePiece(ULONGLONG ullOffset, ULONGLONG ullSize, IHexCtrl* pHexCtrl = nullptr);
	//Has file been modified in memory or not.
	[[nodiscard]] bool IsModified()const { return m_fModified; }
	bool Flush(); //Writes memory mapped file on disk.
	//Unloads loaded file and all pieces, if present.
	HRESULT UnloadFile();
private:
	/****************************************************************************
	* This is a helper structure for query information.							*
	* Every window (hWnd) can have its own set of mapped data.					*
	* So we can map different parts, of the big file,							*
	* for different windows (CHexCtrl instances) simultaneously.				*
	****************************************************************************/
	struct QUERYDATA
	{
		HWND		hWnd { };                    //IHexCtrl HWND which is querying the file's data.
		ULONGLONG	ullStartOffsetMapped { };    //File is mapped starting from this raw offset.
		ULONGLONG	ullEndOffsetMapped { };      //File's raw offset where mapping ends.
		ULONGLONG	ullOffsetDelta { };          //File (or piece) is shown starting from this offset, used in ShowFilePiece.
		DWORD		dwDeltaFileOffsetMapped { }; //Delta after ullStartOffsetMapped % m_stSysInfo.dwAllocationGranularity.
		LPVOID		lpData { };                  //File's Mapped data.
		bool		fShowPiece { false };        //Whether used in ShowOffset (false) or in ShowFilePiece (true).
	};
	bool m_fLoaded { false };
	CPepperDoc* m_pMainDoc { };
	IHexCtrlPtr m_pHex { CreateHexCtrl() };
	HEXCREATESTRUCT m_hcs;
	HEXDATASTRUCT m_hds;
	LARGE_INTEGER m_stFileSize { };	 //Size of the loaded PE file.
	HANDLE m_hFile { };
	HANDLE m_hMapObject { };	     //Returned by CreateFileMappingW.
	//Pointer to file mapping beginning,
	//no matter if mapped completely or section by section.
	LPVOID m_lpBase { };
	bool m_fMapViewOfFileWhole { };	//Is file loaded (mapped) completely, or section by section?
	//System information getting from GetSystemInfo().
	//Needed for dwAllocationGranularity.
	SYSTEM_INFO m_stSysInfo { };
	std::vector<QUERYDATA> m_vecQuery;
	const int IDC_HEX_CTRL = 0xFF; //Id of inner IHexCtrl.
	bool m_fModified { false };
	bool m_fWritable { false };
private:
	std::byte* GetData(HWND hWnd, ULONGLONG ullOffset); //For Virtual HexCtrl retrives next byte on demand.
	HRESULT MapFileOffset(QUERYDATA& rData, ULONGLONG ullOffset, DWORD dwSize = 0); //Main routine for mapping big file's parts.
	HRESULT UnmapFileOffset(QUERYDATA& rData);
	[[nodiscard]] bool IsLoaded()const;
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};