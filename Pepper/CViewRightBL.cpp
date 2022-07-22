/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "CViewRightBL.h"
#include "Utility.h"
#include "res/resource.h"

IMPLEMENT_DYNCREATE(CViewRightBL, CView)

BEGIN_MESSAGE_MAP(CViewRightBL, CView)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CViewRightBL::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPCHILDREN;

	return CView::PreCreateWindow(cs);
}

void CViewRightBL::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_pChildFrame = static_cast<CChildFrame*>(GetParentFrame());
	m_pMainDoc = static_cast<CPepperDoc*>(GetDocument());
	m_pLibpe = m_pMainDoc->m_pLibpe.get();
	m_pFileLoader = &m_pMainDoc->m_stFileLoader;
	stFileInfo = m_pLibpe->GetFileInfo();

	//Hex control for SecurityDir and TLSdir.
	m_hcs.hWndParent = m_hWnd;
	m_hcs.uID = IDC_HEX_RIGHT_BL;
	m_hcs.dwStyle = WS_CHILD;
	m_stHexEdit->Create(m_hcs);

	m_stlcs.stColor.clrTooltipText = RGB(255, 255, 255);
	m_stlcs.stColor.clrTooltipBk = RGB(0, 132, 132);
	m_stlcs.stColor.clrHdrText = RGB(255, 255, 255);
	m_stlcs.stColor.clrHdrBk = RGB(0, 132, 132);
	m_stlcs.stColor.clrHdrHglInact = RGB(0, 112, 112);
	m_stlcs.stColor.clrHdrHglAct = RGB(0, 92, 92);
	m_stlcs.dwHdrHeight = 35;
	m_stlcs.pParent = this;
	m_stlcs.fSortable = true;

	auto pDC = GetDC();
	const auto iLOGPIXELSY = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY);
	m_lf.lfHeight = m_hdrlf.lfHeight = -MulDiv(11, iLOGPIXELSY, 72);
	ReleaseDC(pDC);

	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stlcs.pListLogFont = &m_lf;
	m_hdrlf.lfWeight = FW_BOLD;
	StringCchCopyW(m_hdrlf.lfFaceName, 16, L"Times New Roman");
	m_stlcs.pHdrLogFont = &m_hdrlf;

	CreateListExportFuncs();
	CreateTreeResources();
}

void CViewRightBL::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	if (!m_pChildFrame || LOWORD(lHint) == IDC_HEX_RIGHT_TR)
		return;

	CRect rc;
	GetClientRect(&rc);
	switch (LOWORD(lHint))
	{
	case IDC_LIST_DOSHEADER_ENTRY:
		CreateHexDosHeaderEntry(HIWORD(lHint));
		break;
	case IDC_LIST_RICHHEADER_ENTRY:
		CreateHexRichHeaderEntry(HIWORD(lHint));
		break;
	case IDC_LIST_NTHEADER_ENTRY:
		CreateHexNtHeaderEntry();
		break;
	case IDC_LIST_FILEHEADER_ENTRY:
		CreateHexFileHeaderEntry(HIWORD(lHint));
		break;
	case IDC_LIST_OPTIONALHEADER_ENTRY:
		CreateHexOptHeaderEntry(HIWORD(lHint));
		break;
	case IDC_LIST_DATADIRECTORIES_ENTRY:
		CreateHexDataDirsEntry(HIWORD(lHint));
		break;
	case IDC_LIST_SECHEADERS_ENTRY:
		CreateHexSecHeadersEntry(HIWORD(lHint));
		break;
	case IDC_LIST_EXPORT:
		if (m_hwndActive != m_listExportFuncs->m_hWnd)
		{
			if (m_hwndActive)
				::ShowWindow(m_hwndActive, SW_HIDE);

			m_hwndActive = m_listExportFuncs->m_hWnd;
		}
		m_listExportFuncs->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	case IDC_LIST_IMPORT_ENTRY:
		CreateListImportEntry(HIWORD(lHint));
		break;
	case IDC_TREE_RESOURCE:
		if (m_hwndActive != m_treeResBottom.m_hWnd)
		{
			if (m_hwndActive)
				::ShowWindow(m_hwndActive, SW_HIDE);

			m_hwndActive = m_treeResBottom.m_hWnd;
		}
		m_treeResBottom.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	case IDC_LIST_SECURITY_ENTRY:
		CreateHexSecurityEntry(HIWORD(lHint));
		break;
	case IDC_LIST_LOADCONFIG_ENTRY:
		CreateHexLCDEntry(HIWORD(lHint));
		break;
	case IDC_LIST_RELOCATIONS_ENTRY:
		CreateListRelocsEntry(HIWORD(lHint));
		break;
	case IDC_LIST_TLS:
		CreateHexTLS();
		break;
	case IDC_LIST_DELAYIMPORT_ENTRY:
		CreateListDelayImportEntry(HIWORD(lHint));
		break;
	case IDC_LIST_DEBUG_ENTRY:
		CreateHexDebugEntry(HIWORD(lHint));
		break;
	case ID_DOC_EDITMODE:
		OnDocEditMode();
		break;
	default:
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
	}
}

void CViewRightBL::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_hwndActive)
		::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightBL::OnDraw(CDC* /*pDC*/)
{
}

BOOL CViewRightBL::OnEraseBkgnd(CDC* pDC)
{
	return CView::OnEraseBkgnd(pDC);
}

BOOL CViewRightBL::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT * pResult)
{
	const auto pTree = reinterpret_cast<LPNMTREEVIEWW>(lParam);

	if (pTree->hdr.idFrom == IDC_TREE_RESOURCE_BOTTOM && pTree->hdr.code == TVN_SELCHANGED)
	{
		const auto pstResRoot = m_pLibpe->GetResources();
		if (pstResRoot == nullptr)
			return -1;

		const auto& [idlvlRoot, idlvl2, idlvl3] = m_vecResId.at(m_treeResBottom.GetItemData(pTree->itemNew.hItem));
		if (idlvl2 >= 0)
		{
			auto& rootvec = pstResRoot->vecResData;
			auto& lvl2tup = rootvec[idlvlRoot].stResLvL2;
			auto& lvl2vec = lvl2tup.vecResData;

			if (!lvl2vec.empty())
			{
				if (idlvl3 >= 0)
				{
					auto& lvl3tup = lvl2vec[idlvl2].stResLvL3;
					auto& lvl3vec = lvl3tup.vecResData;

					if (!lvl3vec.empty())
					{
						auto data = &lvl3vec[idlvl3].vecRawResData;
						//Resource data and resource type to show in CViewRightBR.
						SRESDATA stResHelper { };
						stResHelper.IdResType = rootvec[idlvlRoot].stResDirEntry.Id;
						stResHelper.IdResName = lvl2vec[idlvl2].stResDirEntry.Id;
						stResHelper.pData = data;
						m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_SHOW_RESOURCE_RBR, 0), reinterpret_cast<CObject*>(&stResHelper));
					}
				}
			}
		}
		else
		{
			//Update by default, with no data — to clear the view.
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_SHOW_RESOURCE_RBR, 0), nullptr);
		}
	}

	return CView::OnNotify(wParam, lParam, pResult);
}

void CViewRightBL::OnDocEditMode()
{
	if (m_stHexEdit->IsDataSet())
		m_stHexEdit->SetMutable(m_pMainDoc->IsEditMode());
}

int CViewRightBL::CreateHexDosHeaderEntry(DWORD dwEntry)
{
	if (dwEntry >= g_mapDOSHeader.size())
		return -1;

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}

	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	const auto& ref = g_mapDOSHeader.at(dwEntry);
	m_pFileLoader->ShowOffset(ref.dwOffset, ref.dwSize, m_stHexEdit.get());

	return 0;
}

int CViewRightBL::CreateHexRichHeaderEntry(DWORD dwEntry)
{
	const auto pRichHeader = m_pLibpe->GetRichHeader();
	if (pRichHeader == nullptr || dwEntry >= pRichHeader->size())
		return -1;

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	//Each «Rich» takes 8 bytes (two DWORDs).
	m_pFileLoader->ShowOffset(pRichHeader->at(dwEntry).dwOffset, 8, m_stHexEdit.get());

	return 0;
}

int CViewRightBL::CreateHexNtHeaderEntry()
{
	const auto pNTHdr = m_pLibpe->GetNTHeader();
	if (pNTHdr == nullptr)
		return -1;

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pFileLoader->ShowOffset(pNTHdr->dwOffset, sizeof(DWORD), m_stHexEdit.get()); //Shows NTHDR signature.

	return 0;
}

int CViewRightBL::CreateHexFileHeaderEntry(DWORD dwEntry)
{
	const auto pNTHdr = m_pLibpe->GetNTHeader();
	if (pNTHdr == nullptr || dwEntry >= g_mapFileHeader.size())
		return -1;

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	DWORD dwOffset = pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS32, FileHeader) + g_mapFileHeader.at(dwEntry).dwOffset;
	m_pFileLoader->ShowOffset(dwOffset, g_mapFileHeader.at(dwEntry).dwSize, m_stHexEdit.get());

	return 0;
}

int CViewRightBL::CreateHexOptHeaderEntry(DWORD dwEntry)
{
	const auto pNTHdr = m_pLibpe->GetNTHeader();
	if (pNTHdr == nullptr)
		return -1;

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}

	DWORD dwOffset { }, dwSize { };
	if (stFileInfo.fIsx86)
	{
		dwOffset = pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS32, OptionalHeader) + g_mapOptHeader32.at(dwEntry).dwOffset;
		dwSize = g_mapOptHeader32.at(dwEntry).dwSize;
	}
	else if (stFileInfo.fIsx64)
	{
		dwOffset = pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS64, OptionalHeader) + g_mapOptHeader64.at(dwEntry).dwOffset;
		dwSize = g_mapOptHeader64.at(dwEntry).dwSize;
	}

	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pFileLoader->ShowOffset(dwOffset, dwSize, m_stHexEdit.get());

	return 0;
}

int CViewRightBL::CreateHexDataDirsEntry(DWORD dwEntry)
{
	const auto pNTHdr = m_pLibpe->GetNTHeader();
	if (pNTHdr == nullptr)
		return -1;

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}

	DWORD dwOffset { }, dwSize = sizeof(IMAGE_DATA_DIRECTORY);
	if (stFileInfo.fIsx86)
		dwOffset = pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS32, OptionalHeader)
		+ offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory) + sizeof(IMAGE_DATA_DIRECTORY) * dwEntry;
	else if (stFileInfo.fIsx64)
		dwOffset = pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS64, OptionalHeader)
		+ offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory) + sizeof(IMAGE_DATA_DIRECTORY) * dwEntry;

	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pFileLoader->ShowOffset(dwOffset, dwSize, m_stHexEdit.get());

	return 0;
}

int CViewRightBL::CreateHexSecHeadersEntry(DWORD dwEntry)
{
	const auto pSecHeaders = m_pLibpe->GetSecHeaders();
	if (pSecHeaders == nullptr || dwEntry >= pSecHeaders->size())
		return -1;

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	const auto& rImageSecHeader = pSecHeaders->at(dwEntry).stSecHdr;
	m_pFileLoader->ShowFilePiece(rImageSecHeader.PointerToRawData, rImageSecHeader.SizeOfRawData, m_stHexEdit.get());

	return 0;
}

int CViewRightBL::CreateHexLCDEntry(DWORD dwEntry)
{
	const auto pLCD = m_pLibpe->GetLoadConfig();
	if (pLCD == nullptr || dwEntry >= g_mapLCD32.size())
		return -1;

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	DWORD dwOffset = pLCD->dwOffset;
	DWORD dwSize;
	if (stFileInfo.fIsx86)
	{
		dwOffset += g_mapLCD32.at(dwEntry).dwOffset;
		dwSize = g_mapLCD32.at(dwEntry).dwSize;
	}
	else
	{
		dwOffset += g_mapLCD64.at(dwEntry).dwOffset;
		dwSize = g_mapLCD64.at(dwEntry).dwSize;
	}
	m_pFileLoader->ShowOffset(dwOffset, dwSize, m_stHexEdit.get());

	return 0;
}

int CViewRightBL::CreateListExportFuncs()
{
	if (!m_listExportFuncs->IsCreated())
	{
		m_stlcs.dwStyle = 0;
		m_stlcs.uID = IDC_LIST_EXPORT_FUNCS;
		m_listExportFuncs->Create(m_stlcs);
		m_listExportFuncs->ShowWindow(SW_HIDE);
		m_listExportFuncs->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
		LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
		m_listExportFuncs->SetColumn(0, &stCol);
		m_listExportFuncs->SetHdrColumnColor(0, g_clrOffset);
		m_listExportFuncs->InsertColumn(1, L"Function RVA", LVCFMT_CENTER, 100);
		m_listExportFuncs->InsertColumn(2, L"Ordinal", LVCFMT_CENTER, 100);
		m_listExportFuncs->InsertColumn(3, L"Name", LVCFMT_CENTER, 250);
		m_listExportFuncs->InsertColumn(4, L"Forwarder Name", LVCFMT_CENTER, 400);
	}

	const auto pExport = m_pLibpe->GetExport();
	if (pExport == nullptr)
		return -1;

	int listindex = 0;
	WCHAR wstr[MAX_PATH];
	const auto dwOffset = m_pLibpe->GetOffsetFromRVA(pExport->stExportDesc.AddressOfFunctions);

	m_listExportFuncs->SetRedraw(FALSE); //to increase the speed of List populating
	for (auto& i : pExport->vecFuncs)
	{
		swprintf_s(wstr, 9, L"%08lX", static_cast<DWORD>(dwOffset + sizeof(DWORD) * i.dwOrdinal));
		m_listExportFuncs->InsertItem(listindex, wstr);

		swprintf_s(wstr, 9, L"%08X", i.dwRVA);
		m_listExportFuncs->SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 17, L"%u", i.dwOrdinal);
		m_listExportFuncs->SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, MAX_PATH, L"%S", i.strFuncName.data());
		m_listExportFuncs->SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, MAX_PATH, L"%S", i.strForwarderName.data());
		m_listExportFuncs->SetItemText(listindex, 4, wstr);

		listindex++;
	}
	m_listExportFuncs->SetRedraw(TRUE);

	return 0;
}

int CViewRightBL::CreateListImportEntry(DWORD dwEntry)
{
	if (!m_listImportEntry->IsCreated())
	{
		m_stlcs.dwStyle = 0;
		m_stlcs.uID = IDC_LIST_IMPORT_ENTRY;
		m_listImportEntry->Create(m_stlcs);
		m_listImportEntry->InsertColumn(0, L"Offset", 0, 90);
		LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
		m_listImportEntry->SetColumn(0, &stCol);
		m_listImportEntry->SetHdrColumnColor(0, g_clrOffset);
		m_listImportEntry->InsertColumn(1, L"Function Name", LVCFMT_CENTER, 175);
		m_listImportEntry->InsertColumn(2, L"Ordinal / Hint", LVCFMT_CENTER, 100);
		m_listImportEntry->InsertColumn(3, L"AddressOfData", LVCFMT_CENTER, 150);
		m_listImportEntry->InsertColumn(4, L"Thunk RVA", LVCFMT_CENTER, 150);
	}
	else
		m_listImportEntry->DeleteAllItems();

	const auto m_pImport = m_pLibpe->GetImport();
	if (m_pImport == nullptr || dwEntry >= m_pImport->size())
		return -1;

	if (m_hwndActive)
		::ShowWindow(m_hwndActive, SW_HIDE);
	m_hwndActive = m_listImportEntry->m_hWnd;

	int listindex = 0;
	WCHAR wstr[MAX_PATH];
	auto& rImp = m_pImport->at(dwEntry).stImportDesc;
	auto dwThunkOffset = m_pLibpe->GetOffsetFromRVA(rImp.OriginalFirstThunk ? rImp.OriginalFirstThunk : rImp.FirstThunk);
	DWORD dwThunkRVA = rImp.OriginalFirstThunk ? rImp.OriginalFirstThunk : rImp.FirstThunk;

	m_listImportEntry->SetRedraw(FALSE);
	for (auto& i : m_pImport->at(dwEntry).vecImportFunc)
	{
		swprintf_s(wstr, 9, L"%08lX", dwThunkOffset);
		m_listImportEntry->InsertItem(listindex, wstr);
		if (stFileInfo.fIsx86)
			dwThunkOffset += sizeof(IMAGE_THUNK_DATA32);
		else if (stFileInfo.fIsx64)
			dwThunkOffset += sizeof(IMAGE_THUNK_DATA64);

		swprintf_s(wstr, MAX_PATH, L"%S", i.strFuncName.data());
		m_listImportEntry->SetItemText(listindex, 1, wstr);

		if (stFileInfo.fIsx86)
		{
			if (i.unThunk.stThunk32.u1.Ordinal & IMAGE_ORDINAL_FLAG32)
				swprintf_s(wstr, 5, L"%04X", IMAGE_ORDINAL32(i.unThunk.stThunk32.u1.Ordinal));
			else
				swprintf_s(wstr, 5, L"%04X", i.stImpByName.Hint);
		}
		else if (stFileInfo.fIsx64)
		{
			if (i.unThunk.stThunk64.u1.Ordinal & IMAGE_ORDINAL_FLAG64)
				swprintf_s(wstr, 5, L"%04llX", IMAGE_ORDINAL64(i.unThunk.stThunk64.u1.Ordinal));
			else
				swprintf_s(wstr, 5, L"%04X", i.stImpByName.Hint);
		}
		m_listImportEntry->SetItemText(listindex, 2, wstr);

		if (stFileInfo.fIsx86)
			swprintf_s(wstr, 9, L"%08X", i.unThunk.stThunk32.u1.AddressOfData);
		else if (stFileInfo.fIsx64)
			swprintf_s(wstr, 17, L"%016llX", i.unThunk.stThunk64.u1.AddressOfData);
		m_listImportEntry->SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08lX", dwThunkRVA);
		m_listImportEntry->SetItemText(listindex, 4, wstr);
		if (stFileInfo.fIsx86)
			dwThunkRVA += sizeof(IMAGE_THUNK_DATA32);
		else if (stFileInfo.fIsx64)
			dwThunkRVA += sizeof(IMAGE_THUNK_DATA64);

		listindex++;
	}
	m_listImportEntry->SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listImportEntry->SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateHexSecurityEntry(unsigned nSertId)
{
	const auto pSec = m_pLibpe->GetSecurity();
	if (pSec == nullptr || nSertId >= pSec->size())
		return -1;

	const auto& secEntry = pSec->at(nSertId).stWinSert;
	DWORD dwStart = pSec->at(nSertId).dwOffset + offsetof(WIN_CERTIFICATE, bCertificate);
	DWORD dwCertSize = static_cast<DWORD_PTR>(secEntry.dwLength) - offsetof(WIN_CERTIFICATE, bCertificate);
	m_pFileLoader->ShowFilePiece(dwStart, dwCertSize, m_stHexEdit.get());

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateListDelayImportEntry(DWORD dwEntry)
{
	if (!m_listDelayImportEntry->IsCreated())
	{
		m_stlcs.dwStyle = 0;
		m_stlcs.uID = IDC_LIST_DELAYIMPORT_ENTRY;
		m_listDelayImportEntry->Create(m_stlcs);
		m_listDelayImportEntry->InsertColumn(0, L"Offset", 0, 90);
		LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
		m_listDelayImportEntry->SetColumn(0, &stCol);
		m_listDelayImportEntry->SetHdrColumnColor(0, g_clrOffset);
		m_listDelayImportEntry->InsertColumn(1, L"Function Name", LVCFMT_CENTER, 300);
		m_listDelayImportEntry->InsertColumn(2, L"Ordinal / Hint", LVCFMT_CENTER, 100);
		m_listDelayImportEntry->InsertColumn(3, L"ImportNameTable AddresOfData", 0, 220);
		m_listDelayImportEntry->InsertColumn(4, L"IAT AddresOfData", LVCFMT_CENTER, 200);
		m_listDelayImportEntry->InsertColumn(5, L"BoundIAT AddresOfData", LVCFMT_CENTER, 230);
		m_listDelayImportEntry->InsertColumn(6, L"UnloadInfoTable AddresOfData", LVCFMT_CENTER, 240);
	}
	else
		m_listDelayImportEntry->DeleteAllItems();

	const auto pDelayImport = m_pLibpe->GetDelayImport();
	if (pDelayImport == nullptr || dwEntry >= pDelayImport->size())
		return -1;

	int listindex = 0;
	WCHAR wstr[MAX_PATH];

	auto dwThunkOffset = m_pLibpe->GetOffsetFromRVA(pDelayImport->at(dwEntry).stDelayImpDesc.ImportNameTableRVA);

	m_listDelayImportEntry->SetRedraw(FALSE);
	for (auto&i : pDelayImport->at(dwEntry).vecDelayImpFunc)
	{
		swprintf_s(wstr, 9, L"%08lX", dwThunkOffset);
		m_listDelayImportEntry->InsertItem(listindex, wstr);
		if (stFileInfo.fIsx86)
			dwThunkOffset += sizeof(IMAGE_THUNK_DATA32);
		else if (stFileInfo.fIsx64)
			dwThunkOffset += sizeof(IMAGE_THUNK_DATA64);

		swprintf_s(wstr, 256, L"%S", i.strFuncName.data());
		m_listDelayImportEntry->SetItemText(listindex, 1, wstr);

		if (stFileInfo.fIsx86)
		{
			if (i.unThunk.st32.stImportNameTable.u1.Ordinal & IMAGE_ORDINAL_FLAG32)
				swprintf_s(wstr, 5, L"%04X", IMAGE_ORDINAL32(i.unThunk.st32.stImportNameTable.u1.Ordinal));
			else
				swprintf_s(wstr, 5, L"%04X", i.stImpByName.Hint);
		}
		else if (stFileInfo.fIsx64)
		{
			if (i.unThunk.st64.stImportNameTable.u1.Ordinal & IMAGE_ORDINAL_FLAG64)
				swprintf_s(wstr, 5, L"%04llX", IMAGE_ORDINAL64(i.unThunk.st64.stImportNameTable.u1.Ordinal));
			else
				swprintf_s(wstr, 5, L"%04X", i.stImpByName.Hint);
		}
		m_listDelayImportEntry->SetItemText(listindex, 2, wstr);

		if (stFileInfo.fIsx86)
			swprintf_s(wstr, 9, L"%08X", i.unThunk.st32.stImportNameTable.u1.AddressOfData);
		else if (stFileInfo.fIsx64)
			swprintf_s(wstr, 17, L"%016llX", i.unThunk.st64.stImportNameTable.u1.AddressOfData);
		m_listDelayImportEntry->SetItemText(listindex, 3, wstr);

		if (stFileInfo.fIsx86)
			swprintf_s(wstr, 9, L"%08X", i.unThunk.st32.stImportAddressTable.u1.AddressOfData);
		else if (stFileInfo.fIsx64)
			swprintf_s(wstr, 17, L"%016llX", i.unThunk.st64.stImportAddressTable.u1.AddressOfData);
		m_listDelayImportEntry->SetItemText(listindex, 4, wstr);

		if (stFileInfo.fIsx86)
			swprintf_s(wstr, 9, L"%08X", i.unThunk.st32.stBoundImportAddressTable.u1.AddressOfData);
		else if (stFileInfo.fIsx64)
			swprintf_s(wstr, 17, L"%016llX", i.unThunk.st64.stBoundImportAddressTable.u1.AddressOfData);
		m_listDelayImportEntry->SetItemText(listindex, 5, wstr);

		if (stFileInfo.fIsx86)
			swprintf_s(wstr, 9, L"%08X", i.unThunk.st32.stUnloadInformationTable.u1.AddressOfData);
		else if (stFileInfo.fIsx64)
			swprintf_s(wstr, 17, L"%016llX", i.unThunk.st64.stUnloadInformationTable.u1.AddressOfData);
		m_listDelayImportEntry->SetItemText(listindex, 6, wstr);

		listindex++;
	}
	m_listDelayImportEntry->SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listDelayImportEntry->SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_hwndActive = m_listDelayImportEntry->m_hWnd;

	return 0;
}

int CViewRightBL::CreateListRelocsEntry(DWORD dwEntry)
{
	if (!m_listRelocsEntry->IsCreated())
	{
		m_stlcs.dwStyle = 0;
		m_stlcs.uID = IDC_LIST_RELOCATIONS_ENTRY;
		m_listRelocsEntry->Create(m_stlcs);
		m_listRelocsEntry->ShowWindow(SW_HIDE);
		m_listRelocsEntry->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
		LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
		m_listRelocsEntry->SetColumn(0, &stCol);
		m_listRelocsEntry->SetHdrColumnColor(0, g_clrOffset);
		m_listRelocsEntry->InsertColumn(1, L"Reloc type", LVCFMT_CENTER, 250);
		m_listRelocsEntry->InsertColumn(2, L"Offset to apply", LVCFMT_CENTER, 120);
	}
	else
		m_listRelocsEntry->DeleteAllItems();

	const auto pReloc = m_pLibpe->GetRelocations();
	if (pReloc == nullptr || dwEntry >= pReloc->size())
		return -1;

	const std::unordered_map<WORD, std::wstring> mapRelocTypes {
		TO_WSTR_MAP(IMAGE_REL_BASED_ABSOLUTE),
		TO_WSTR_MAP(IMAGE_REL_BASED_HIGH),
		TO_WSTR_MAP(IMAGE_REL_BASED_LOW),
		TO_WSTR_MAP(IMAGE_REL_BASED_HIGHLOW),
		TO_WSTR_MAP(IMAGE_REL_BASED_HIGHADJ),
		TO_WSTR_MAP(IMAGE_REL_BASED_MACHINE_SPECIFIC_5),
		TO_WSTR_MAP(IMAGE_REL_BASED_RESERVED),
		TO_WSTR_MAP(IMAGE_REL_BASED_MACHINE_SPECIFIC_7),
		TO_WSTR_MAP(IMAGE_REL_BASED_MACHINE_SPECIFIC_8),
		TO_WSTR_MAP(IMAGE_REL_BASED_MACHINE_SPECIFIC_9),
		TO_WSTR_MAP(IMAGE_REL_BASED_DIR64)
	};

	int listindex = 0;
	WCHAR wstr[MAX_PATH];

	m_listRelocsEntry->SetRedraw(FALSE);
	for (auto& iterRelocs : pReloc->at(dwEntry).vecRelocData)
	{
		swprintf_s(wstr, 9, L"%08X", iterRelocs.dwOffset);
		m_listRelocsEntry->InsertItem(listindex, wstr);

		auto it = mapRelocTypes.find(iterRelocs.wRelocType);
		if (it != mapRelocTypes.end())
			swprintf_s(wstr, MAX_PATH, L"%s", it->second.data());
		else
			swprintf_s(wstr, MAX_PATH, L"%u", iterRelocs.wRelocType);

		m_listRelocsEntry->SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 5, L"%04X", iterRelocs.wRelocOffset);
		m_listRelocsEntry->SetItemText(listindex, 2, wstr);

		listindex++;
	}
	m_listRelocsEntry->SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listRelocsEntry->SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_hwndActive = m_listRelocsEntry->m_hWnd;

	return 0;
}

int CViewRightBL::CreateHexDebugEntry(DWORD dwEntry)
{
	const auto pDebug = m_pLibpe->GetDebug();
	if (pDebug == nullptr || dwEntry >= pDebug->size())
		return -1;

	const auto& rDebugDir = pDebug->at(dwEntry).stDebugDir;
	m_pFileLoader->ShowFilePiece(rDebugDir.PointerToRawData, rDebugDir.SizeOfData, m_stHexEdit.get());

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateTreeResources()
{
	const auto pstResRoot = m_pLibpe->GetResources();
	if (pstResRoot == nullptr)
		return -1;

	m_treeResBottom.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		TVS_LINESATROOT, CRect(0, 0, 0, 0), this, IDC_TREE_RESOURCE_BOTTOM);
	m_treeResBottom.ShowWindow(SW_HIDE);

	WCHAR wstr[MAX_PATH];

	//Scaling factor for HighDPI displays.
	auto pDC = GetDC();
	const auto fScale = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY) / 96.0f;
	ReleaseDC(pDC);
	const auto iImgSize = static_cast<int>(16 * fScale);

	m_imglTreeRes.Create(iImgSize, iImgSize, ILC_COLOR32, 0, 4);
	const auto iconDirs = m_imglTreeRes.Add(AfxGetApp()->LoadIconW(IDI_TREE_PERESOURCE));
	m_treeResBottom.SetImageList(&m_imglTreeRes, TVSIL_NORMAL);
	long ilvlRoot = 0;

	//Creating a treeCtrl and setting, with SetItemData(),
	//a unique id for each node, that is an index in vector (m_vecResId),
	//that holds tuple of three IDs of resource — Type, Name, LangID.
	for (auto& iterRoot : pstResRoot->vecResData)
	{
		const IMAGE_RESOURCE_DIRECTORY_ENTRY* pResDirEntry = &iterRoot.stResDirEntry;
		if (pResDirEntry->NameIsString)
			//Enclose in double quotes.
			swprintf_s(wstr, MAX_PATH, L"\u00AB%s\u00BB", iterRoot.wstrResName.data());
		else
		{	//Setting Treectrl root node name depending on Resource typeID.
			auto iter = g_mapResType.find(pResDirEntry->Id);
			if (iter != g_mapResType.end())
				swprintf_s(wstr, MAX_PATH, L"%s [Id: %u]", iter->second.data(), pResDirEntry->Id);
			else
				swprintf_s(wstr, MAX_PATH, L"%u", pResDirEntry->Id);
		}

		const auto treeRoot = m_treeResBottom.InsertItem(wstr, iconDirs, iconDirs);
		m_vecResId.emplace_back(ilvlRoot, -1, -1);
		m_treeResBottom.SetItemData(treeRoot, m_vecResId.size() - 1);
		long ilvl2 = 0;
		auto& refResLvL2 = iterRoot.stResLvL2; //Resource level 2.

		for (auto& iterLvL2 : refResLvL2.vecResData)
		{
			m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
			long ilvl3 = 0;
			auto& refResLvL3 = iterLvL2.stResLvL3;

			for (auto& iterLvL3 : refResLvL3.vecResData)
			{
				pResDirEntry = &iterLvL3.stResDirEntry;
				if (pResDirEntry->NameIsString)
					swprintf_s(wstr, MAX_PATH, L"«%s» - lang: %i", iterLvL2.wstrResName.data(), pResDirEntry->Id);
				else
					swprintf_s(wstr, MAX_PATH, L"%u - lang: %i", iterLvL2.stResDirEntry.Id, pResDirEntry->Id);

				m_vecResId.emplace_back(ilvlRoot, ilvl2, ilvl3);
				m_treeResBottom.SetItemData(m_treeResBottom.InsertItem(wstr, treeRoot), m_vecResId.size() - 1);
				ilvl3++;
			}
			ilvl2++;
		}
		ilvlRoot++;
	}

	return 0;
}

int CViewRightBL::CreateHexTLS()
{
	const auto pTLS = m_pLibpe->GetTLS();
	if (pTLS == nullptr)
		return -1;

	if (m_hwndActive != m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN))
	{
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	ULONGLONG ullStartAdr { }, ullEndAdr { };
	if (stFileInfo.fIsx86)
	{
		ullStartAdr = pTLS->unTLS.stTLSDir32.StartAddressOfRawData;
		ullEndAdr = pTLS->unTLS.stTLSDir32.EndAddressOfRawData;
	}
	else if (stFileInfo.fIsx64)
	{
		ullStartAdr = pTLS->unTLS.stTLSDir64.StartAddressOfRawData;
		ullEndAdr = pTLS->unTLS.stTLSDir64.EndAddressOfRawData;
	}

	const auto dwOffsetStart = m_pLibpe->GetOffsetFromVA(ullStartAdr);
	const auto dwOffsetEnd = m_pLibpe->GetOffsetFromVA(ullEndAdr);

	DWORD dwSize { 0 };
	if (dwOffsetEnd > dwOffsetStart)
		dwSize = dwOffsetEnd - dwOffsetStart;

	m_pFileLoader->ShowFilePiece(dwOffsetStart, dwSize, m_stHexEdit.get());

	return 0;
}