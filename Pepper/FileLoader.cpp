#include "stdafx.h"
#include "FileLoader.h"

HRESULT CFileLoader::LoadFile(LPCWSTR lpszFileName, DWORD_PTR dwGotoOffset)
{
	if (IsLoaded())
		return ShowOffset(dwGotoOffset);

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
		m_dwFileOffsetToMap = 0;
	}

	if (!CWnd::CreateEx(0, nullptr, nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, AfxGetMainWnd()->m_hWnd, nullptr))
		return E_ABORT;

	m_Hex.Create(this, IDC_HEX_CTRL, nullptr, true);

	if (m_fMapViewOfFileWhole)
		m_Hex.SetData((PBYTE)m_lpBase, (DWORD_PTR)m_stFileSize.QuadPart);
	else
		m_Hex.SetData(nullptr, (DWORD_PTR)m_stFileSize.QuadPart, true);

	ShowOffset(dwGotoOffset);

	return S_OK;
}

HRESULT CFileLoader::MapFilePiece(DWORD dwOffset)
{/*
	if (dwOffset >= m_dwFileOffsetToMap || dwOffset <= m_ullMaxPointerBound || dwOffset >= m_stFileSize.QuadPart)
		return S_OK;

	DWORD dwAlignedOffsetToMap;
	DWORD_PTR dwSizeToMap;

	m_dwFileOffsetToMap = (dwOffset <= 0x01900000) ? 0 : dwOffset - 0x01900000; //25MB.

	if (m_dwFileOffsetToMap % m_stSysInfo.dwAllocationGranularity > 0)
		dwAlignedOffsetToMap = (m_dwFileOffsetToMap < m_stSysInfo.dwAllocationGranularity) ? 0 :
		(m_dwFileOffsetToMap - (m_dwFileOffsetToMap % m_stSysInfo.dwAllocationGranularity));
	else
		dwAlignedOffsetToMap = m_dwFileOffsetToMap;

	m_dwDeltaFileOffsetToMap = m_dwFileOffsetToMap - dwAlignedOffsetToMap;

	dwSizeToMap = 0x01900000; //25MB.
	if ((LONGLONG)(dwAlignedOffsetToMap + dwSizeToMap) > m_stFileSize.QuadPart)
		dwSizeToMap = (DWORD_PTR)(m_stFileSize.QuadPart - (LONGLONG)dwAlignedOffsetToMap);

	if (!(m_lpSectionBase = MapViewOfFile(m_hMapObject, FILE_MAP_READ, 0, dwAlignedOffsetToMap, dwSizeToMap)))
		return false;

	m_ullMaxPointerBound = (DWORD_PTR)m_lpSectionBase + dwSizeToMap;
	*/
	return 0;
}

HRESULT CFileLoader::ShowOffset(DWORD_PTR dwOffset)
{
	if (!IsLoaded())
		return E_ABORT;

	if (m_fMapViewOfFileWhole)
		m_Hex.SetSelection(dwOffset);
	else
	{
		MapFilePiece(dwOffset);
		m_Hex.SetSelection(dwOffset);
	}

	return S_OK;
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

LONGLONG CFileLoader::GetBytesCount()
{
	return m_stFileSize.QuadPart;
}

BOOL CFileLoader::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	PHEXNOTIFY pHexNtfy = (PHEXNOTIFY)lParam;
	if (pHexNtfy->hdr.idFrom == IDC_HEX_CTRL)
	{
		switch (pHexNtfy->hdr.code)
		{
		case HEXCTRL_MSG_DESTROY:
			UnloadFile();
			break;
		case HEXCTRL_MSG_SCROLLING:
			break;
		case HEXCTRL_MSG_GETDISPINFO:
			if (pHexNtfy->dwByteIndex < m_dwMinBytesToMap)
				pHexNtfy->chByte = *(unsigned char*)((DWORD_PTR)m_lpBase + pHexNtfy->dwByteIndex);
			break;
		}
	}

	return CWnd::OnNotify(wParam, lParam, pResult);
}