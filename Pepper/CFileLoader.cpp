/****************************************************************************************************
* Copyright © 2018-2022 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#include "stdafx.h"
#include "CFileLoader.h"
#include "CPepperDoc.h"
#include "res/resource.h"
#include <algorithm>

import Utility;

void CFileLoader::CreateHexCtrlWnd()
{
	m_pHex->Create(m_hcs);

	const auto hWndHex = m_pHex->GetWindowHandle(EHexWnd::WND_MAIN);
	const auto iWidthActual = m_pHex->GetActualWidth() + GetSystemMetrics(SM_CXVSCROLL);
	CRect rcHex(0, 0, iWidthActual, iWidthActual); //Square window.
	AdjustWindowRectEx(rcHex, m_dwStyle, FALSE, m_dwExStyle);
	const auto iWidth = rcHex.Width();
	const auto iHeight = rcHex.Height() - rcHex.Height() / 3;
	const auto iPosX = GetSystemMetrics(SM_CXSCREEN) / 2 - iWidth / 2;
	const auto iPosY = GetSystemMetrics(SM_CYSCREEN) / 2 - iHeight / 2;
	::SetWindowPos(hWndHex, nullptr, iPosX, iPosY, iWidth, iHeight, SWP_SHOWWINDOW);

	const auto hIconSmall = static_cast<HICON>(LoadImageW(AfxGetInstanceHandle(), MAKEINTRESOURCEW(IDI_HEXCTRL_LOGO), IMAGE_ICON, 0, 0, 0));
	const auto hIconBig = static_cast<HICON>(LoadImageW(AfxGetInstanceHandle(), MAKEINTRESOURCEW(IDI_HEXCTRL_LOGO), IMAGE_ICON, 96, 96, 0));
	if (hIconSmall != nullptr) {
		::SendMessageW(m_pHex->GetWindowHandle(EHexWnd::WND_MAIN), WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
		::SendMessageW(m_pHex->GetWindowHandle(EHexWnd::WND_MAIN), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconBig));
	}
}

bool CFileLoader::Flush()
{
	if (!IsLoaded())
		return false;

	if (IsModified())
		FlushViewOfFile(m_lpBase, 0);

	return false;
}

HRESULT CFileLoader::LoadFile(LPCWSTR lpszFileName, CPepperDoc* pDoc)
{
	if (IsLoaded() || !pDoc)
		return E_ABORT;

	bool fWritable { false }; //ReadOnly flag.
	m_hFile = CreateFileW(lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_hFile == INVALID_HANDLE_VALUE) {
		m_hFile = CreateFileW(lpszFileName, GENERIC_READ, FILE_SHARE_READ,
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (m_hFile == INVALID_HANDLE_VALUE) {
			MessageBoxW(L"CreateFileW call in FileLoader::LoadFile failed.", L"Error", MB_ICONERROR);
			return E_ABORT;
		}
	}
	else
		fWritable = true;

	::GetFileSizeEx(m_hFile, &m_stFileSize);

	m_hMapObject = CreateFileMappingW(m_hFile, nullptr, fWritable ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
	if (!m_hMapObject) {
		CloseHandle(m_hFile);
		MessageBoxW(L"CreateFileMappingW call in FileLoader::LoadFile failed.", L"Error", MB_ICONERROR);
		return E_ABORT;
	}
	m_fLoaded = true;

	m_lpBase = MapViewOfFile(m_hMapObject, fWritable ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);

	if (!CWnd::CreateEx(0, AfxRegisterWndClass(0), nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr))
		return E_ABORT;

	m_fWritable = fWritable;
	m_pMainDoc = pDoc;
	m_hcs.dwStyle = m_dwStyle;
	m_hcs.dwExStyle = m_dwExStyle; //To force to the taskbar.
	m_hcs.hWndParent = m_hWnd;
	m_hds.fMutable = m_pMainDoc->IsEditMode();

	return S_OK;
}

HRESULT CFileLoader::ShowOffsetInWholeFile(ULONGLONG ullOffset, ULONGLONG ullSelSize, IHexCtrl* pHexCtrl)
{
	if (!pHexCtrl) {
		if (!m_pHex->IsCreated())
			CreateHexCtrlWnd();
		pHexCtrl = m_pHex.get();
	}

	//To not resetting the data every time, when all we need is just to set a new selection.
	//If given IHexCtrl already set with a whole file data, we won't call pHexCtrl->SetData() again.
	if (const auto iter = std::find_if(m_vecCheck.begin(), m_vecCheck.end(), [pHexCtrl](const HEXTODATACHECK& ref) {
		return ref.pHexCtrl == pHexCtrl; }); iter == m_vecCheck.end() || !iter->fWhole) {
		m_hds.fMutable = m_pMainDoc->IsEditMode();
		m_hds.spnData = { static_cast<std::byte*>(m_lpBase), static_cast<std::size_t>(m_stFileSize.QuadPart) };
		pHexCtrl->SetData(m_hds);

		if (iter == m_vecCheck.end()) {
			m_vecCheck.emplace_back(pHexCtrl, true);
		}
		else {
			iter->fWhole = true;
		}
	}

	if (ullSelSize > 0) {
		std::vector<HEXSPAN> vecSel { { ullOffset, ullSelSize } };
		pHexCtrl->SetSelection(vecSel);
		if (!pHexCtrl->IsOffsetVisible(ullOffset)) {
			pHexCtrl->GoToOffset(ullOffset);
		}
	}

	//If floating HexCtrl is in use we bring it to the front.
	if (pHexCtrl == m_pHex.get())
		::SetForegroundWindow(pHexCtrl->GetWindowHandle(EHexWnd::WND_MAIN));

	return S_OK;
}

HRESULT CFileLoader::ShowFilePiece(ULONGLONG ullOffset, ULONGLONG ullSize, IHexCtrl* pHexCtrl)
{
	if (!IsLoaded())
		return E_ABORT;

	if (!pHexCtrl) {
		if (!m_pHex->IsCreated())
			CreateHexCtrlWnd();
		pHexCtrl = m_pHex.get();
	}

	if (ullOffset >= static_cast<ULONGLONG>(m_stFileSize.QuadPart)) { //Overflow check.
		pHexCtrl->ClearData();
		return E_ABORT;
	}
	if (ullOffset + ullSize > static_cast<ULONGLONG>(m_stFileSize.QuadPart)) //Overflow check.
		ullSize = static_cast<ULONGLONG>(m_stFileSize.QuadPart) - ullOffset;

	if (const auto iter = std::find_if(m_vecCheck.begin(), m_vecCheck.end(), [pHexCtrl](const HEXTODATACHECK& ref) {
		return ref.pHexCtrl == pHexCtrl; }); iter == m_vecCheck.end()) {
		m_vecCheck.emplace_back(pHexCtrl, false);
	}
	else if (iter->fWhole) {
		iter->fWhole = false;
	}

	m_hds.fMutable = m_pMainDoc->IsEditMode();
	m_hds.spnData = { reinterpret_cast<std::byte*>(reinterpret_cast<DWORD_PTR>(m_lpBase) + ullOffset), static_cast<std::size_t>(ullSize) };
	pHexCtrl->SetData(m_hds);

	return S_OK;
}

HRESULT CFileLoader::UnloadFile()
{
	if (!m_fLoaded)
		return E_ABORT;

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

	return S_OK;
}

bool CFileLoader::IsLoaded()const
{
	return m_fLoaded;
}

BOOL CFileLoader::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	return CWnd::OnNotify(wParam, lParam, pResult);
}