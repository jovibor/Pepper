/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
/************************************************************
* CFileLoader implementation.								*
************************************************************/
#include "stdafx.h"
#include "FileLoader.h"
#include <algorithm>

HRESULT CFileLoader::LoadFile(LPCWSTR lpszFileName)
{
	if (IsCreated())
		return S_OK;

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
	m_fCreated = true;

	m_lpBase = MapViewOfFile(m_hMapObject, FILE_MAP_READ, 0, 0, 0);
	if (m_lpBase)
	{
		m_fMapViewOfFileWhole = true;
	}
	else //Not enough memory? File is too big?
	{
		m_fMapViewOfFileWhole = false;
		::GetNativeSystemInfo(&m_stSysInfo);
	}

	if (!CWnd::CreateEx(0, AfxRegisterWndClass(0), nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr))
		return E_ABORT;

	return S_OK;
}

HRESULT CFileLoader::ShowOffset(ULONGLONG ullOffset, CHexCtrl* pHexCtrl)
{
	if (!pHexCtrl)
	{
		m_stHex.Create(this, IDC_HEX_CTRL, 0, nullptr, true);
		pHexCtrl = &m_stHex;
	}

	bool fVirtual;
	PBYTE pData;
	if (m_fMapViewOfFileWhole)
	{
		fVirtual = false;
		pData = (PBYTE)m_lpBase;
	}
	else
	{
		fVirtual = true;
		pData = nullptr;
	}

	auto const& iter = std::find_if(m_vecQuery.begin(), m_vecQuery.end(), [pHexCtrl](const QUERYDATA& r) {return r.hWnd == pHexCtrl->m_hWnd; });
	if (iter == m_vecQuery.end())
		m_vecQuery.emplace_back(QUERYDATA { pHexCtrl->m_hWnd });
	else
		iter->ullOffsetDelta = 0;

	pHexCtrl->SetData(pData, (ULONGLONG)m_stFileSize.QuadPart, fVirtual, ullOffset, this);

	return S_OK;
}

HRESULT CFileLoader::ShowFilePiece(ULONGLONG ullOffset, ULONGLONG ullSize, CHexCtrl* pHexCtrl)
{
	if (!IsCreated())
		return E_ABORT;

	if (!pHexCtrl)
		pHexCtrl = &m_stHex;

	bool fVirtual;
	PBYTE pData;

	if (m_fMapViewOfFileWhole)
	{
		fVirtual = false;
		pData = (PBYTE)((DWORD_PTR)m_lpBase + ullOffset);
	}
	else
	{
		fVirtual = true;
		pData = nullptr;
	}

	//Checking for given HWND existence in m_vecQuery. If there is no, create.
	auto const& iter = std::find_if(m_vecQuery.begin(), m_vecQuery.end(),
		[pHexCtrl](const QUERYDATA& rData) {return rData.hWnd == pHexCtrl->m_hWnd; });
	if (iter == m_vecQuery.end())
		m_vecQuery.emplace_back(QUERYDATA { pHexCtrl->m_hWnd, 0, 0, ullOffset, 0, nullptr });
	else
		iter->ullOffsetDelta = ullOffset;

	pHexCtrl->SetData(pData, ullSize, fVirtual, 0, this);

	return S_OK;
}

HRESULT CFileLoader::MapFileOffset(QUERYDATA& rData, ULONGLONG ullOffset, DWORD dwSize)
{
	UnmapFileOffset(rData);

	DWORD_PTR dwSizeToMap;
	if (dwSize > 0)
		dwSizeToMap = dwSize;
	else
		dwSizeToMap = 0x01900000; //25MB.


	ULONGLONG ullStartOffsetMapped { }, ullEndOffsetMapped { };
	if (ullOffset > (ULONGLONG)dwSizeToMap)
		ullStartOffsetMapped = ullOffset - (dwSizeToMap / 2);
	else
		ullStartOffsetMapped = 0;

	DWORD dwDelta = ullStartOffsetMapped % m_stSysInfo.dwAllocationGranularity;
	if (dwDelta > 0)
		ullStartOffsetMapped = (ullStartOffsetMapped < m_stSysInfo.dwAllocationGranularity) ? 0 :
		(ullStartOffsetMapped - dwDelta);

	if ((LONGLONG)(ullStartOffsetMapped + dwSizeToMap) > m_stFileSize.QuadPart)
		dwSizeToMap = (DWORD)(m_stFileSize.QuadPart - (LONGLONG)ullStartOffsetMapped);

	DWORD dwOffsetHigh = (ullStartOffsetMapped >> 32) & 0xFFFFFFFFul;
	DWORD dwOffsetLow = ullStartOffsetMapped & 0xFFFFFFFFul;
	LPVOID lpData { };
	if (!(lpData = MapViewOfFile(m_hMapObject, FILE_MAP_READ, dwOffsetHigh, dwOffsetLow, dwSizeToMap)))
		return E_FILE_MAPVIEWOFFILE_SECTION_FAILED;

	rData.ullStartOffsetMapped = ullStartOffsetMapped;
	rData.ullEndOffsetMapped = ullStartOffsetMapped + dwSizeToMap;
	rData.dwDeltaFileOffsetMapped = dwDelta;
	rData.lpData = lpData;

	return S_OK;
}

HRESULT CFileLoader::UnmapFileOffset(QUERYDATA& rData)
{
	if (!rData.lpData)
		return E_ABORT;

	UnmapViewOfFile(rData.lpData);
	rData.lpData = nullptr;
	rData.ullStartOffsetMapped = 0;
	rData.ullEndOffsetMapped = 0;

	return S_OK;
}

HRESULT CFileLoader::UnloadFile()
{
	if (!m_fCreated)
		return E_ABORT;

	for (auto& i : m_vecQuery)
		UnmapFileOffset(i);

	if (m_lpBase)
		UnmapViewOfFile(m_lpBase);
	if (m_hMapObject)
		CloseHandle(m_hMapObject);
	if (m_hFile)
		CloseHandle(m_hFile);

	m_lpBase = nullptr;
	m_hMapObject = nullptr;
	m_hFile = nullptr;
	m_fCreated = false;
	m_fMapViewOfFileWhole = false;

	return S_OK;
}

bool CFileLoader::IsCreated()
{
	return m_fCreated;
}

bool CFileLoader::IsLoaded()
{
	return m_fMapViewOfFileWhole;
}

BOOL CFileLoader::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	PHEXNOTIFY pHexNtfy = (PHEXNOTIFY)lParam;

	switch (pHexNtfy->hdr.code)
	{
	case HEXCTRL_MSG_DESTROY:
	{
		auto const& iter = std::find_if(m_vecQuery.begin(), m_vecQuery.end(),
			[pHexNtfy](const QUERYDATA& rData) {return rData.hWnd == pHexNtfy->hdr.hwndFrom; });
		if (iter != m_vecQuery.end())
			UnmapFileOffset(*iter);
		break;
	}
	case HEXCTRL_MSG_GETDISPINFO:
		pHexNtfy->chByte = GetByte(pHexNtfy->hdr.hwndFrom, pHexNtfy->ullByteIndex);
		break;
	}

	return CWnd::OnNotify(wParam, lParam, pResult);
}

unsigned char CFileLoader::GetByte(HWND hWnd, ULONGLONG ullOffset)
{
	auto const& iter = std::find_if(m_vecQuery.begin(), m_vecQuery.end(),
		[hWnd](const QUERYDATA& rData) {return rData.hWnd == hWnd; });
	if (iter == m_vecQuery.end())
		return 0;

	unsigned char chByte;
	ullOffset += iter->ullOffsetDelta;

	if (m_fMapViewOfFileWhole && ullOffset < (ULONGLONG)m_stFileSize.QuadPart)
		chByte = ((PBYTE)m_lpBase)[ullOffset];
	else
	{
		if (ullOffset >= iter->ullStartOffsetMapped && ullOffset < iter->ullEndOffsetMapped)
			chByte = ((PBYTE)iter->lpData)[ullOffset - iter->ullStartOffsetMapped];
		else
		{
			if (MapFileOffset(*iter, ullOffset) == S_OK)
				chByte = ((PBYTE)iter->lpData)[ullOffset - iter->ullStartOffsetMapped];
			else
				chByte = 0;
		}
	}

	return chByte;
}