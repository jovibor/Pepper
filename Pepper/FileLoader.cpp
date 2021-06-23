/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "FileLoader.h"
#include "PepperDoc.h"
#include "Utility.h"
#include <algorithm>

HRESULT CFileLoader::LoadFile(LPCWSTR lpszFileName, CPepperDoc* pDoc)
{
	if (IsLoaded() || !pDoc)
		return E_ABORT;

	bool fWritable { false }; //ReadOnly flag.
	m_hFile = CreateFileW(lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		m_hFile = CreateFileW(lpszFileName, GENERIC_READ, FILE_SHARE_READ,
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			MessageBoxW(L"CreateFileW call in FileLoader::LoadFile failed.", L"Error", MB_ICONERROR);
			return E_FILE_CREATEFILE_FAILED;
		}
	}
	else
		fWritable = true;

	::GetFileSizeEx(m_hFile, &m_stFileSize);

	m_hMapObject = CreateFileMappingW(m_hFile, nullptr, fWritable ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
	if (!m_hMapObject)
	{
		CloseHandle(m_hFile);
		MessageBoxW(L"CreateFileMappingW call in FileLoader::LoadFile failed.", L"Error", MB_ICONERROR);
		return E_FILE_CREATEFILEMAPPING_FAILED;
	}
	m_fLoaded = true;

	m_lpBase = MapViewOfFile(m_hMapObject, fWritable ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
	if (m_lpBase)
		m_fMapViewOfFileWhole = true;
	else { //Not enough memory? File is too big?
		m_fMapViewOfFileWhole = false;
		::GetNativeSystemInfo(&m_stSysInfo);
	}

	if (!CWnd::CreateEx(0, AfxRegisterWndClass(0), nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr))
		return E_ABORT;

	m_fWritable = fWritable;
	m_pMainDoc = pDoc;

	m_hcs.enCreateMode = EHexCreateMode::CREATE_POPUP;
	m_hcs.hwndParent = m_hWnd;
	m_hcs.dwExStyle = WS_EX_APPWINDOW; //To force to the taskbar.
	const auto iPosX = GetSystemMetrics(SM_CXSCREEN) / 4;
	const auto iPosY = GetSystemMetrics(SM_CYSCREEN) / 4;
	m_hcs.rect = { iPosX, iPosY, iPosX * 3, iPosY * 3 };
	m_hds.fMutable = m_pMainDoc->IsEditMode();

	return S_OK;
}

HRESULT CFileLoader::ShowOffset(ULONGLONG ullOffset, ULONGLONG ullSelSize, IHexCtrl* pHexCtrl)
{
	if (!pHexCtrl)
	{
		if (!m_pHex->IsCreated())
			m_pHex->Create(m_hcs);
		pHexCtrl = m_pHex.get();
	}

	std::byte* pData;
	if (m_fMapViewOfFileWhole)
		pData = static_cast<std::byte*>(m_lpBase);
	else
		m_hds.pHexVirtData = this;

	m_hds.fMutable = m_pMainDoc->IsEditMode();
	m_hds.pData = pData;
	m_hds.ullDataSize = static_cast<ULONGLONG>(m_stFileSize.QuadPart);

	auto const& iter = std::find_if(m_vecQuery.begin(), m_vecQuery.end(),
		[pHexCtrl](const QUERYDATA& r) {return r.hWnd == pHexCtrl->GetWindowHandle(EHexWnd::WND_MAIN); });

	bool fExist { false };
	if (iter == m_vecQuery.end())
		m_vecQuery.emplace_back(QUERYDATA { pHexCtrl->GetWindowHandle(EHexWnd::WND_MAIN) });
	else
		fExist = true;

	//If fExist we additionally check whether this window was set
	//to ShowOffset or to ShowFilePiece. If the latter we reset it to show full file. 
	if (fExist)
	{
		if (iter->fShowPiece)
		{
			iter->ullOffsetDelta = 0;
			iter->fShowPiece = false;
			pHexCtrl->SetData(m_hds);
		}
	}
	else
		pHexCtrl->SetData(m_hds);

	if (ullSelSize > 0)
	{
		std::vector<HEXSPAN> vecSel { { ullOffset, ullSelSize } };
		pHexCtrl->SetSelection(vecSel);
		if (!pHexCtrl->IsOffsetVisible(ullOffset))
			pHexCtrl->GoToOffset(ullOffset);
	}

	//If floating HexCtrl in use - bring it to front.
	if (pHexCtrl == m_pHex.get())
		::SetForegroundWindow(pHexCtrl->GetWindowHandle(EHexWnd::WND_MAIN));

	return S_OK;
}

HRESULT CFileLoader::ShowFilePiece(ULONGLONG ullOffset, ULONGLONG ullSize, IHexCtrl* pHexCtrl)
{
	if (!IsLoaded())
		return E_ABORT;

	if (!pHexCtrl)
	{
		if (!m_pHex->IsCreated())
			m_pHex->Create(m_hcs);
		pHexCtrl = m_pHex.get();
	}

	if (ullOffset >= static_cast<ULONGLONG>(m_stFileSize.QuadPart)) //Overflow check.
	{
		pHexCtrl->ClearData();
		return E_ABORT;
	}
	if (ullOffset + ullSize > static_cast<ULONGLONG>(m_stFileSize.QuadPart)) //Overflow check.
		ullSize = static_cast<ULONGLONG>(m_stFileSize.QuadPart) - ullOffset;

	std::byte* pData;
	if (m_fMapViewOfFileWhole)
	{
		m_hds.pHexVirtData = nullptr;
		pData = reinterpret_cast<std::byte*>(reinterpret_cast<DWORD_PTR>(m_lpBase) + ullOffset);
	}
	else
		m_hds.pHexVirtData = this;

	//Checking for given HWND existence in m_vecQuery. If there is no, create.
	auto const& iter = std::find_if(m_vecQuery.begin(), m_vecQuery.end(),
		[pHexCtrl](const QUERYDATA& rData) {return rData.hWnd == pHexCtrl->GetWindowHandle(EHexWnd::WND_MAIN); });

	if (iter == m_vecQuery.end())
		m_vecQuery.emplace_back(QUERYDATA { pHexCtrl->GetWindowHandle(EHexWnd::WND_MAIN), 0, 0, ullOffset, 0, nullptr, true });
	else
	{
		iter->ullOffsetDelta = ullOffset;
		iter->fShowPiece = true;
	}

	m_hds.fMutable = m_pMainDoc->IsEditMode();
	m_hds.pData = pData;
	m_hds.ullDataSize = ullSize;
	pHexCtrl->SetData(m_hds);

	return S_OK;
}

HRESULT CFileLoader::MapFileOffset(QUERYDATA& rData, ULONGLONG ullOffset, DWORD dwSize)
{
	UnmapFileOffset(rData);

	DWORD_PTR dwSizeToMap;
	if (dwSize > 0)
		dwSizeToMap = static_cast<DWORD_PTR>(dwSize);
	else
		dwSizeToMap = 0x01900000; //25MB.

	ULONGLONG ullStartOffsetMapped;
	if (ullOffset > static_cast<ULONGLONG>(dwSizeToMap))
		ullStartOffsetMapped = ullOffset - (dwSizeToMap / 2);
	else
		ullStartOffsetMapped = 0;

	DWORD dwDelta = ullStartOffsetMapped % m_stSysInfo.dwAllocationGranularity;
	if (dwDelta > 0)
		ullStartOffsetMapped = (ullStartOffsetMapped < m_stSysInfo.dwAllocationGranularity) ? 0 :
		(ullStartOffsetMapped - dwDelta);

	if (static_cast<LONGLONG>(ullStartOffsetMapped + dwSizeToMap) > m_stFileSize.QuadPart)
		dwSizeToMap = static_cast<DWORD_PTR>(m_stFileSize.QuadPart - static_cast<LONGLONG>(ullStartOffsetMapped));

	DWORD dwOffsetHigh = (ullStartOffsetMapped >> 32) & 0xFFFFFFFFUL;
	DWORD dwOffsetLow = ullStartOffsetMapped & 0xFFFFFFFFUL;
	LPVOID lpData { };
	if ((lpData = MapViewOfFile(m_hMapObject, FILE_MAP_READ, dwOffsetHigh, dwOffsetLow, dwSizeToMap)) == nullptr)
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

bool CFileLoader::Flush()
{
	if (!IsLoaded())
		return false;

	if (IsModified())
		FlushViewOfFile(m_lpBase, 0);

	return false;
}

HRESULT CFileLoader::UnloadFile()
{
	if (!m_fLoaded)
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
	m_fLoaded = false;
	m_fMapViewOfFileWhole = false;

	return S_OK;
}

bool CFileLoader::IsLoaded()const
{
	return m_fLoaded;
}

BOOL CFileLoader::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	auto pHdr = reinterpret_cast<NMHDR*>(lParam);

	switch (pHdr->code)
	{
	case HEXCTRL_MSG_DESTROY:
	{
		if (!m_fMapViewOfFileWhole)
		{
			//Look for given HWND in m_vecQuery and if exist Unmap all its data.
			auto const& iter = std::find_if(m_vecQuery.begin(), m_vecQuery.end(),
				[pHdr](const QUERYDATA& rData) { return rData.hWnd == pHdr->hwndFrom; });
			if (iter != m_vecQuery.end())
				UnmapFileOffset(*iter);
		}
	}
	break;
	}

	return CWnd::OnNotify(wParam, lParam, pResult);
}

void CFileLoader::OnHexGetData(HEXDATAINFO& hdi)
{
	auto const& iter = std::find_if(m_vecQuery.begin(), m_vecQuery.end(),
		[hWnd=hdi.hdr.hwndFrom](const QUERYDATA& rData) {return rData.hWnd == hWnd; });
	if (iter == m_vecQuery.end())
		return;

	std::byte* pData { };
	auto ullOffset = hdi.stSpan.ullOffset + iter->ullOffsetDelta;

	if (m_fMapViewOfFileWhole && ullOffset < static_cast<ULONGLONG>(m_stFileSize.QuadPart))
		pData = (static_cast<std::byte*>(m_lpBase)) + ullOffset;
	else
	{
		if (ullOffset >= iter->ullStartOffsetMapped && ullOffset < iter->ullEndOffsetMapped)
			pData = (static_cast<std::byte*>(iter->lpData)) + (ullOffset - iter->ullStartOffsetMapped);
		else
		{
			if (MapFileOffset(*iter, ullOffset) == S_OK)
				pData = (static_cast<std::byte*>(iter->lpData)) + (ullOffset - iter->ullStartOffsetMapped);
		}
	}

	hdi.pData = pData;
}

void CFileLoader::OnHexSetData(const HEXDATAINFO& hdi)
{
	m_fModified = true;
}