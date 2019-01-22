#include "stdafx.h"
#include "FileLoader.h"

CFileLoader::CFileLoader()
{
}

CFileLoader::~CFileLoader()
{
}

HRESULT CFileLoader::LoadFile(LPCWSTR lpszFileName)
{
	if (m_hFile)
		UnloadFile();

	m_hFile = CreateFileW(lpszFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_hFile == INVALID_HANDLE_VALUE)
		return E_FILE_OPEN_FAILED;

	::GetFileSizeEx(m_hFile, &m_stFileSize);
	if (m_stFileSize.QuadPart < sizeof(IMAGE_DOS_HEADER))
	{
		CloseHandle(m_hFile);
		return E_FILE_SIZE_TOO_SMALL;
	}

	m_hMapObject = CreateFileMappingW(m_hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (!m_hMapObject)
	{
		CloseHandle(m_hFile);
		return E_FILE_CREATEFILEMAPPING_FAILED;
	}

	m_lpBase = MapViewOfFile(m_hMapObject, FILE_MAP_READ, 0, 0, 0);
	if (!m_lpBase) //Not enough memory? File is too big?
	{
		if (GetLastError() == ERROR_NOT_ENOUGH_MEMORY)
		{
			//If file is too big to fit process VirtualSize limit
			//we try to allocate at least some memory to map file's beginning, where PE HEADER resides.
			//Then going to MapViewOfFile/Unmap every section individually. 
			if (!(m_lpBase = MapViewOfFile(m_hMapObject, FILE_MAP_READ, 0, 0, (DWORD_PTR)m_dwMinBytesToMap)))
			{
				CloseHandle(m_hMapObject);
				CloseHandle(m_hFile);
				return E_FILE_MAPVIEWOFFILE_FAILED;
			}
			m_fMapViewOfFileWhole = false;
			m_ullMaxPointerBound = (DWORD_PTR)m_lpBase + (DWORD_PTR)m_dwMinBytesToMap;
			::GetSystemInfo(&m_stSysInfo);
		}
		else
		{
			CloseHandle(m_hMapObject);
			CloseHandle(m_hFile);
			return E_FILE_MAPVIEWOFFILE_FAILED;
		}
	}
	else
	{
		m_fMapViewOfFileWhole = true;
		m_ullMaxPointerBound = (DWORD_PTR)m_lpBase + m_stFileSize.QuadPart;
	}

	//If file succeeded to fully map,
	//then just proceed getting all structures.
	if (m_fMapViewOfFileWhole)
	{
	}
	else //Otherwise mapping each section separately.
	{
	}

	return S_OK;
}


HRESULT CFileLoader::MapDirSection(DWORD dwOffset)
{
	/*	DWORD dwAlignedOffsetToMap;
		DWORD_PTR dwSizeToMap;

		m_dwFileOffsetToMap = dwOffset;

		if (m_dwFileOffsetToMap > m_stFileSize.QuadPart)
			return E_FILE_SECTION_DATA_CORRUPTED;

		if (m_dwFileOffsetToMap % m_stSysInfo.dwAllocationGranularity > 0)
			dwAlignedOffsetToMap = (m_dwFileOffsetToMap < m_stSysInfo.dwAllocationGranularity) ? 0 :
			(m_dwFileOffsetToMap - (m_dwFileOffsetToMap % m_stSysInfo.dwAllocationGranularity));
		else
			dwAlignedOffsetToMap = m_dwFileOffsetToMap;

		m_dwDeltaFileOffsetToMap = m_dwFileOffsetToMap - dwAlignedOffsetToMap;

		dwSizeToMap = 10000;

		if (((LONGLONG)dwAlignedOffsetToMap + dwSizeToMap) > m_stFileSize.QuadPart)
			return false;
		if (!(m_lpSectionBase = MapViewOfFile(m_hMapObject, FILE_MAP_READ, 0, dwAlignedOffsetToMap, dwSizeToMap)))
			return false;

		m_ullMaxPointerBound = (DWORD_PTR)m_lpSectionBase + dwSizeToMap;
		*/
	return 0;
}


LPVOID CFileLoader::GetMemoryHandle()
{
	return m_lpBase;
}


HRESULT CFileLoader::UnloadFile()
{
	UnmapViewOfFile(m_lpBase);
	m_lpBase = nullptr;
	CloseHandle(m_hMapObject);
	m_hMapObject = nullptr;
	CloseHandle(m_hFile);
	m_hFile = nullptr;

	return S_OK;
}


bool CFileLoader::IsLoaded()
{
	return m_lpBase ? true : false;
}

DWORD_PTR CFileLoader::GetBytesCount()
{
	return m_stFileSize.QuadPart;
}
