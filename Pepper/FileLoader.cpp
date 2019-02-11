/************************************************************
* CFileLoader implementation.								*
************************************************************/
#include "stdafx.h"
#include "FileLoader.h"

HRESULT CFileLoader::LoadFile(LPCWSTR lpszFileName, ULONGLONG ullGotoOffset)
{
	if (m_lpBase || m_lpSectionBase)
		return ShowOffset(ullGotoOffset);

	m_hFile = CreateFileW(lpszFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_hFile == INVALID_HANDLE_VALUE)
		return E_FILE_OPEN_FAILED;

	::GetFileSizeEx(m_hFile, &m_stFileSize);

	m_hMapObject = CreateFileMappingW(m_hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (!m_hMapObject)
	{
		CloseHandle(m_hFile);
		return E_FILE_CREATEFILEMAPPING_FAILED;
	}

	m_lpBase = MapViewOfFile(m_hMapObject, FILE_MAP_READ, 0, 0, 0);
	if (m_lpBase)
	{
		m_fMapViewOfFileWhole = true;
		m_ullStartOffsetMapped = 0;
		m_ullEndOffsetMapped = m_stFileSize.QuadPart;
	}
	else //Not enough memory? File is too big?
	{
		m_fMapViewOfFileWhole = false;
		m_ullStartOffsetMapped = m_ullEndOffsetMapped = 0;
		::GetNativeSystemInfo(&m_stSysInfo);
	}

	if (!CWnd::CreateEx(0, AfxRegisterWndClass(0), nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, AfxGetMainWnd()->m_hWnd, nullptr))
		return E_ABORT;

	m_stHex.Create(this, IDC_HEX_CTRL, nullptr, true);

	if (m_fMapViewOfFileWhole)
		m_stHex.SetData((PBYTE)m_lpBase, (ULONGLONG)m_stFileSize.QuadPart);
	else
		m_stHex.SetData(nullptr, (ULONGLONG)m_stFileSize.QuadPart, true);

	ShowOffset(ullGotoOffset);

	return S_OK;
}

HRESULT CFileLoader::MapFileOffset(ULONGLONG dwOffset)
{
	if (m_lpSectionBase)
		UnmapViewOfFile(m_lpSectionBase);

	DWORD_PTR dwSizeToMap = 0x01900000; //25MB.

	if (dwOffset > (ULONGLONG)dwSizeToMap)
		m_ullStartOffsetMapped = dwOffset - (dwSizeToMap / 2);
	else
		m_ullStartOffsetMapped = 0;

	m_dwDeltaFileOffsetMapped = m_ullStartOffsetMapped % m_stSysInfo.dwAllocationGranularity;
	if (m_dwDeltaFileOffsetMapped > 0)
		m_ullStartOffsetMapped = (m_ullStartOffsetMapped < m_stSysInfo.dwAllocationGranularity) ? 0 :
		(m_ullStartOffsetMapped - m_dwDeltaFileOffsetMapped);

	if ((LONGLONG)(m_ullStartOffsetMapped + dwSizeToMap) > m_stFileSize.QuadPart)
		dwSizeToMap = (DWORD)(m_stFileSize.QuadPart - (LONGLONG)m_ullStartOffsetMapped);

	DWORD dwOffsetHigh = (m_ullStartOffsetMapped >> 32) & 0xFFFFFFFFul;
	DWORD dwOffsetLow = m_ullStartOffsetMapped & 0xFFFFFFFFul;
	if (!(m_lpSectionBase = MapViewOfFile(m_hMapObject, FILE_MAP_READ, dwOffsetHigh, dwOffsetLow, dwSizeToMap)))
		return E_FILE_MAPVIEWOFFILE_SECTION_FAILED;

	m_ullEndOffsetMapped = m_ullStartOffsetMapped + dwSizeToMap;

	return S_OK;
}

HRESULT CFileLoader::ShowOffset(ULONGLONG ullOffset)
{
	m_stHex.SetSelection(ullOffset);

	return S_OK;
}

HRESULT CFileLoader::UnloadFile()
{
	if (m_lpBase)
		UnmapViewOfFile(m_lpBase);
	if (m_hMapObject)
		CloseHandle(m_hMapObject);
	if (m_hFile)
		CloseHandle(m_hFile);
	if (m_lpSectionBase)
		UnmapViewOfFile(m_lpSectionBase);

	m_lpSectionBase = nullptr;
	m_lpBase = nullptr;
	m_hMapObject = nullptr;
	m_hFile = nullptr;

	return S_OK;
}

unsigned char CFileLoader::GetByte(ULONGLONG ullOffset)
{
	unsigned char chByte;

	if (m_fMapViewOfFileWhole && ullOffset < m_ullEndOffsetMapped)
		chByte = ((PBYTE)m_lpBase)[ullOffset];
	else
	{
		if (ullOffset >= m_ullStartOffsetMapped && ullOffset < m_ullEndOffsetMapped)
			chByte = ((PBYTE)m_lpSectionBase)[ullOffset - m_ullStartOffsetMapped];
		else
		{
			if (MapFileOffset(ullOffset) == S_OK)
				chByte = ((PBYTE)m_lpSectionBase)[ullOffset - m_ullStartOffsetMapped];
			else
				chByte = 0;
		}
	}

	return chByte;
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
		case HEXCTRL_MSG_GETDISPINFO:
			pHexNtfy->chByte = GetByte(pHexNtfy->ullByteIndex);
			break;
		}
	}

	return CWnd::OnNotify(wParam, lParam, pResult);
}