#pragma once
#include "HexCtrl.h"

using namespace HEXControl;

class CFileLoader : public CWnd
{
public:
	CFileLoader() {};
	~CFileLoader() {};
	HRESULT LoadFile(LPCWSTR lpszFileName, DWORD_PTR dwGotoOffset = 0);
	HRESULT UnloadFile();
	bool IsLoaded();
	HRESULT ShowOffset(DWORD_PTR);
	LONGLONG GetBytesCount();
private:
	CHexCtrl m_Hex;
	//Size of the loaded PE file.
	LARGE_INTEGER m_stFileSize { };
	//Maximum address that can be dereferensed.
	ULONGLONG m_ullMaxPointerBound { };
	const DWORD m_dwMinBytesToMap { 0xFFFF };
	HANDLE m_hFile { };
	//Returned by CreateFileMappingW.
	HANDLE m_hMapObject { };
	//Pointer to file mapping beginning,
	//no matter if mapped completely or section by section.
	LPVOID m_lpBase { };
	//Pointer to beginning of mapping if mapped section by section.
	LPVOID m_lpSectionBase { };
	//For big files that can't be mapped completely
	//shows offset the mapping begins from.
	DWORD m_dwFileOffsetToMap { };
	//Delta after file mapping alignment.
	//m_dwDeltaFileOffsetToMap = m_dwFileOffsetToMap - dwAlignedAddressToMap;
	//dwAlignedAddressToMap = (m_dwFileOffsetToMap < SysInfo.dwAllocationGranularity) ? 0 :
	//(m_dwFileOffsetToMap - (m_dwFileOffsetToMap % SysInfo.dwAllocationGranularity));
	DWORD m_dwDeltaFileOffsetToMap { };
	//Is file loaded (mapped) completely, or section by section?
	bool m_fMapViewOfFileWhole { };
	//System information getting from GetSystemInfo().
	//Needed for dwAllocationGranularity.
	SYSTEM_INFO m_stSysInfo { };

	const int IDC_HEX_CTRL = 0xFF;
private:
	HRESULT MapFilePiece(DWORD dwOffset);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};

