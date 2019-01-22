#pragma once

class CFileLoader
{
public:
	CFileLoader();
	~CFileLoader();
	HRESULT LoadFile(LPCWSTR lpszFileName);
private:
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
private:
	HRESULT MapDirSection(DWORD dwOffset);
public:
	LPVOID GetMemoryHandle();
	HRESULT UnloadFile();
	bool IsLoaded();
	DWORD_PTR GetBytesCount();
};

