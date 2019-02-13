/****************************************************************
* This is a helper class for Pepper and HexCtrl routines of		*
* showing files' offsets. It loads (maps) files into memory		*
* by demand and unloads then. Works with big files - files that *
* can't	be loaded fully at ones - as well.						*
*																*
****************************************************************/
#pragma once
#include "HexCtrl.h"

using namespace HEXCTRL;

class CFileLoader : public CWnd
{
public:
	CFileLoader() {};
	~CFileLoader() {};
	HRESULT LoadFile(LPCWSTR lpszFileName, ULONGLONG ullGotoOffset = 0);
	HRESULT UnloadFile();
private:
	CHexCtrl m_stHex;
	//Size of the loaded PE file.
	LARGE_INTEGER m_stFileSize { };
	HANDLE m_hFile { };
	//Returned by CreateFileMappingW.
	HANDLE m_hMapObject { };
	//Pointer to file mapping beginning,
	//no matter if mapped completely or section by section.
	LPVOID m_lpBase { };
	//Pointer to beginning of mapping if mapped section by section.
	LPVOID m_lpSectionBase { };
	//Delta after file mapping alignment.
	//m_dwDeltaFileOffsetMapped = m_dwFileOffsetToMap - dwAlignedAddressToMap;
	//dwAlignedAddressToMap = (m_dwFileOffsetToMap < SysInfo.dwAllocationGranularity) ? 0 :
	//(m_dwFileOffsetToMap - (m_dwFileOffsetToMap % SysInfo.dwAllocationGranularity));
	DWORD m_dwDeltaFileOffsetMapped { };
	//Is file loaded (mapped) completely, or section by section?
	bool m_fMapViewOfFileWhole { };
	//System information getting from GetSystemInfo().
	//Needed for dwAllocationGranularity.
	SYSTEM_INFO m_stSysInfo { };
	//For big files that can't be mapped at one time
	//shows offset the mapping begins from.
	ULONGLONG m_ullStartOffsetMapped { };
	//The mapping's end: start + map_size. 
	//We can grab bytes that are less than this offset ( < m_ullEndOffsetMapped).
	ULONGLONG m_ullEndOffsetMapped { };

	const int IDC_HEX_CTRL = 0xFF;
private:
	unsigned char GetByte(ULONGLONG ullOffset); //For Virtual HexCtrl retrives next byte on demand.
	HRESULT ShowOffset(ULONGLONG ullOffset);
	HRESULT MapFileOffset(ULONGLONG ullOffset); //Main routine for mapping big file's parts.
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};