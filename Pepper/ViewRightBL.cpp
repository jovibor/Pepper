/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "ViewRightBL.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CViewRightBL, CView)

BEGIN_MESSAGE_MAP(CViewRightBL, CView)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CViewRightBL::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPCHILDREN;

	return CScrollView::PreCreateWindow(cs);
}

void CViewRightBL::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();
	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	m_pFileLoader = &m_pMainDoc->m_stFileLoader;

	if (m_pLibpe->GetImageInfo(m_dwFileSummary) != S_OK)
		return;

	//Hex control for SecurityDir and TLSdir.
	m_stHC.pwndParent = this;
	m_stHC.uId = IDC_HEX_RIGHT_BL;
	m_stHexEdit.Create(m_stHC);
	m_stHexEdit.ShowWindow(SW_HIDE);

	m_stListInfo.clrTooltipText = RGB(255, 255, 255);
	m_stListInfo.clrTooltipBk = RGB(0, 132, 132);
	m_stListInfo.clrHeaderText = RGB(255, 255, 255);
	m_stListInfo.clrHeaderBk = RGB(0, 132, 132);
	m_stListInfo.dwHeaderHeight = 35;

	m_lf.lfHeight = 16;
	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stListInfo.pListLogFont = &m_lf;
	m_hdrlf.lfHeight = 17;
	m_hdrlf.lfWeight = FW_BOLD;
	StringCchCopyW(m_hdrlf.lfFaceName, 16, L"Times New Roman");
	m_stListInfo.pHeaderLogFont = &m_hdrlf;

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
		CreateHexNtHeaderEntry(HIWORD(lHint));
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
		if (m_pActiveWnd != &m_listExportFuncs)
		{
			if (m_pActiveWnd)
				m_pActiveWnd->ShowWindow(SW_HIDE);

			m_pActiveWnd = &m_listExportFuncs;
		}
		m_listExportFuncs.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	case IDC_LIST_IMPORT_ENTRY:
		CreateListImportEntry(HIWORD(lHint));
		break;
	case IDC_TREE_RESOURCE:
		if (m_pActiveWnd != &m_treeResBottom)
		{
			if (m_pActiveWnd)
				m_pActiveWnd->ShowWindow(SW_HIDE);

			m_pActiveWnd = &m_treeResBottom;
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
	default:
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
	}
}

void CViewRightBL::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveWnd)
		m_pActiveWnd->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightBL::OnDraw(CDC* pDC)
{
}

BOOL CViewRightBL::OnEraseBkgnd(CDC* pDC)
{
	return CScrollView::OnEraseBkgnd(pDC);
}

BOOL CViewRightBL::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	const LPNMTREEVIEW pTree = reinterpret_cast<LPNMTREEVIEW>(lParam);

	if (pTree->hdr.idFrom == IDC_TREE_RESOURCE_BOTTOM && pTree->hdr.code == TVN_SELCHANGED)
	{
		PCLIBPE_RESOURCE_ROOT pstResRoot;

		if (m_pLibpe->GetResources(pstResRoot) != S_OK)
			return -1;

		const DWORD_PTR dwResId = m_treeResBottom.GetItemData(pTree->itemNew.hItem);
		const long idlvlRoot = std::get<0>(m_vecResId.at(dwResId));
		const long idlvl2 = std::get<1>(m_vecResId.at(dwResId));
		const long idlvl3 = std::get<2>(m_vecResId.at(dwResId));

		auto& rootvec = pstResRoot->vecResRoot;
		if (idlvl2 >= 0)
		{
			auto& lvl2tup = rootvec.at(idlvlRoot).stResLvL2;
			auto& lvl2vec = lvl2tup.vecResLvL2;

			if (!lvl2vec.empty())
			{
				if (idlvl3 >= 0)
				{
					auto& lvl3tup = lvl2vec.at(idlvl2).stResLvL3;
					auto& lvl3vec = lvl3tup.vecResLvL3;

					if (!lvl3vec.empty())
					{
						auto& data = lvl3vec.at(idlvl3).vecResRawDataLvL3;
						//Resource data and resource type to show in CViewRightBR.
						m_stResHelper.IdResType = rootvec.at(idlvlRoot).stResDirEntryRoot.Id;
						m_stResHelper.IdResName = lvl2vec.at(idlvl2).stResDirEntryLvL2.Id;
						m_stResHelper.pData = (std::vector<std::byte>*)&data;
						m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_SHOW_RESOURCE_RBR, 0), reinterpret_cast<CObject*>(&m_stResHelper));
					}
				}
			}
		}
		else
			//Update by default, with no data — to clear view.
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_SHOW_RESOURCE_RBR, 0), nullptr);
	}

	return CScrollView::OnNotify(wParam, lParam, pResult);
}

int CViewRightBL::CreateHexDosHeaderEntry(DWORD dwEntry)
{
	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}
	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	auto& ref = g_mapDOSHeader.at(dwEntry);
	m_pFileLoader->ShowOffset(ref.dwOffset, ref.dwSize, &m_stHexEdit);

	return 0;
}

int CViewRightBL::CreateHexRichHeaderEntry(DWORD dwEntry)
{
	PCLIBPE_RICHHEADER_VEC pRichHeader;
	if (m_pLibpe->GetRichHeader(pRichHeader) != S_OK)
		return -1;

	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	//Each «Rich» takes 8 bytes (two DWORDs).
	m_pFileLoader->ShowOffset(pRichHeader->at(dwEntry).dwOffsetRich, 8, &m_stHexEdit);

	return 0;
}

int CViewRightBL::CreateHexNtHeaderEntry(DWORD dwEntry)
{
	PCLIBPE_NTHEADER pNTHdr;
	if (m_pLibpe->GetNTHeader(pNTHdr) != S_OK)
		return -1;

	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}
	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pFileLoader->ShowOffset(pNTHdr->dwOffsetNTHdrDesc, sizeof(DWORD), &m_stHexEdit); //Shows NTHDR signature.

	return 0;
}

int CViewRightBL::CreateHexFileHeaderEntry(DWORD dwEntry)
{
	PCLIBPE_NTHEADER pNTHdr;
	if (m_pLibpe->GetNTHeader(pNTHdr) != S_OK)
		return -1;

	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}
	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	DWORD dwOffset = pNTHdr->dwOffsetNTHdrDesc + offsetof(IMAGE_NT_HEADERS32, FileHeader) + g_mapFileHeader.at(dwEntry).dwOffset;
	m_pFileLoader->ShowOffset(dwOffset, g_mapFileHeader.at(dwEntry).dwSize, &m_stHexEdit);

	return 0;
}

int CViewRightBL::CreateHexOptHeaderEntry(DWORD dwEntry)
{
	PCLIBPE_OPTHEADER_VAR pOptHdr;
	if (m_pLibpe->GetOptionalHeader(pOptHdr) != S_OK)
		return -1;
	PCLIBPE_NTHEADER pNTHdr;
	if (m_pLibpe->GetNTHeader(pNTHdr) != S_OK)
		return -1;

	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}

	DWORD dwOffset { }, dwSize { };
	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		dwOffset = pNTHdr->dwOffsetNTHdrDesc + offsetof(IMAGE_NT_HEADERS32, OptionalHeader) + g_mapOptHeader32.at(dwEntry).dwOffset;
		dwSize = g_mapOptHeader32.at(dwEntry).dwSize;
	}
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
	{
		dwOffset = pNTHdr->dwOffsetNTHdrDesc + offsetof(IMAGE_NT_HEADERS64, OptionalHeader) + g_mapOptHeader64.at(dwEntry).dwOffset;
		dwSize = g_mapOptHeader64.at(dwEntry).dwSize;
	}

	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pFileLoader->ShowOffset(dwOffset, dwSize, &m_stHexEdit);

	return 0;
}

int CViewRightBL::CreateHexDataDirsEntry(DWORD dwEntry)
{
	PCLIBPE_OPTHEADER_VAR pOptHdr;
	if (m_pLibpe->GetOptionalHeader(pOptHdr) != S_OK)
		return -1;
	PCLIBPE_NTHEADER pNTHdr;
	if (m_pLibpe->GetNTHeader(pNTHdr) != S_OK)
		return -1;

	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}

	DWORD dwOffset { }, dwSize = sizeof(IMAGE_DATA_DIRECTORY);
	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
		dwOffset = pNTHdr->dwOffsetNTHdrDesc + offsetof(IMAGE_NT_HEADERS32, OptionalHeader)
		+ offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory) + sizeof(IMAGE_DATA_DIRECTORY) * dwEntry;
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
		dwOffset = pNTHdr->dwOffsetNTHdrDesc + offsetof(IMAGE_NT_HEADERS64, OptionalHeader)
		+ offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory) + sizeof(IMAGE_DATA_DIRECTORY) * dwEntry;

	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pFileLoader->ShowOffset(dwOffset, dwSize, &m_stHexEdit);

	return 0;
}

int CViewRightBL::CreateHexSecHeadersEntry(DWORD dwEntry)
{
	PCLIBPE_SECHEADERS_VEC pSecHeaders;
	if (m_pLibpe->GetSectionsHeaders(pSecHeaders) != S_OK)
		return -1;

	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}
	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	auto& rImageSecHeader = pSecHeaders->at(dwEntry).stSecHdr;
	m_pFileLoader->ShowFilePiece(rImageSecHeader.PointerToRawData, rImageSecHeader.SizeOfRawData, &m_stHexEdit);

	return 0;
}

int CViewRightBL::CreateHexLCDEntry(DWORD dwEntry)
{
	PCLIBPE_LOADCONFIG pLCD;
	if (m_pLibpe->GetLoadConfig(pLCD) != S_OK)
		return -1;

	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}
	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	DWORD dwOffset = pLCD->dwOffsetLCD;
	DWORD dwSize;
	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		dwOffset += g_mapLCD32.at(dwEntry).dwOffset;
		dwSize = g_mapLCD32.at(dwEntry).dwSize;
	}
	else
	{
		dwOffset += g_mapLCD64.at(dwEntry).dwOffset;
		dwSize = g_mapLCD64.at(dwEntry).dwSize;
	}
	m_pFileLoader->ShowOffset(dwOffset, dwSize, &m_stHexEdit);

	return 0;
}

int CViewRightBL::CreateListExportFuncs()
{
	PCLIBPE_EXPORT pExport;
	if (m_pLibpe->GetExport(pExport) != S_OK)
		return -1;

	m_listExportFuncs.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_EXPORT_FUNCS, &m_stListInfo);
	m_listExportFuncs.ShowWindow(SW_HIDE);
	m_listExportFuncs.InsertColumn(0, L"Offset", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 90);
	m_listExportFuncs.SetHeaderColumnColor(0, g_clrOffset);
	m_listExportFuncs.InsertColumn(1, L"Function RVA", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 100);
	m_listExportFuncs.InsertColumn(2, L"Ordinal", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listExportFuncs.InsertColumn(3, L"Name", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 250);
	m_listExportFuncs.InsertColumn(4, L"Forwarder Name", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 400);

	int listindex = 0;
	WCHAR wstr[MAX_PATH];

	DWORD dwOffset;
	m_pLibpe->GetOffsetFromRVA(pExport->stExportDesc.AddressOfFunctions, dwOffset);

	m_listExportFuncs.SetRedraw(FALSE); //to increase the speed of List populating
	for (auto& i : pExport->vecFuncs)
	{
		swprintf_s(wstr, 9, L"%08X", dwOffset + sizeof(DWORD) * i.dwOrdinal);
		m_listExportFuncs.InsertItem(listindex, wstr);

		swprintf_s(wstr, 9, L"%08X", i.dwRVA);
		m_listExportFuncs.SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 9, L"%u", i.dwOrdinal);
		m_listExportFuncs.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, MAX_PATH, L"%S", i.strFuncName.data());
		m_listExportFuncs.SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, MAX_PATH, L"%S", i.strForwarderName.data());
		m_listExportFuncs.SetItemText(listindex, 4, wstr);

		listindex++;
	}
	m_listExportFuncs.SetRedraw(TRUE);

	return 0;
}

int CViewRightBL::CreateListImportEntry(DWORD dwEntry)
{
	PCLIBPE_IMPORT_VEC m_pImport;
	if (m_pLibpe->GetImport(m_pImport) != S_OK || dwEntry > m_pImport->size())
		return -1;

	if (m_pActiveWnd)
		m_pActiveWnd->ShowWindow(SW_HIDE);
	m_pActiveWnd = &m_listImportEntry;

	m_listImportEntry.DestroyWindow();
	m_listImportEntry.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_IMPORT_ENTRY, &m_stListInfo);
	m_listImportEntry.InsertColumn(0, L"Offset", 0, 90);
	m_listImportEntry.SetHeaderColumnColor(0, g_clrOffset);
	m_listImportEntry.InsertColumn(1, L"Function Name", 0, 175);
	m_listImportEntry.InsertColumn(2, L"Ordinal / Hint", 0, 100);
	m_listImportEntry.InsertColumn(3, L"AddressOfData", 0, 150);
	m_listImportEntry.InsertColumn(4, L"Thunk RVA", 0, 150);

	int listindex = 0;
	WCHAR wstr[MAX_PATH];
	auto& rImp = m_pImport->at(dwEntry).stImportDesc;
	DWORD dwThunkOffset;
	m_pLibpe->GetOffsetFromRVA(rImp.OriginalFirstThunk ? rImp.OriginalFirstThunk : rImp.FirstThunk, dwThunkOffset);
	DWORD dwThunkRVA = rImp.OriginalFirstThunk ? rImp.OriginalFirstThunk : rImp.FirstThunk;

	m_listImportEntry.SetRedraw(FALSE);
	for (auto& i : m_pImport->at(dwEntry).vecImportFunc)
	{
		swprintf_s(wstr, 9, L"%08X", dwThunkOffset);
		m_listImportEntry.InsertItem(listindex, wstr);
		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			dwThunkOffset += sizeof(IMAGE_THUNK_DATA32);
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			dwThunkOffset += sizeof(IMAGE_THUNK_DATA64);

		swprintf_s(wstr, MAX_PATH, L"%S", i.strFuncName.data());
		m_listImportEntry.SetItemText(listindex, 1, wstr);

		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
		{
			if (i.varThunk.stThunk32.u1.Ordinal & IMAGE_ORDINAL_FLAG32)
				swprintf_s(wstr, 5, L"%04X", IMAGE_ORDINAL32(i.varThunk.stThunk32.u1.Ordinal));
			else
				swprintf_s(wstr, 5, L"%04X", i.stImpByName.Hint);
		}
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
		{
			if (i.varThunk.stThunk64.u1.Ordinal & IMAGE_ORDINAL_FLAG64)
				swprintf_s(wstr, 5, L"%04llX", IMAGE_ORDINAL64(i.varThunk.stThunk64.u1.Ordinal));
			else
				swprintf_s(wstr, 5, L"%04X", i.stImpByName.Hint);
		}
		m_listImportEntry.SetItemText(listindex, 2, wstr);

		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(wstr, 9, L"%08X", i.varThunk.stThunk32.u1.AddressOfData);
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(wstr, 17, L"%016llX", i.varThunk.stThunk64.u1.AddressOfData);
		m_listImportEntry.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", dwThunkRVA);
		m_listImportEntry.SetItemText(listindex, 4, wstr);
		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			dwThunkRVA += sizeof(IMAGE_THUNK_DATA32);
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			dwThunkRVA += sizeof(IMAGE_THUNK_DATA64);

		listindex++;
	}
	m_listImportEntry.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listImportEntry.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateHexSecurityEntry(unsigned nSertId)
{
	PCLIBPE_SECURITY_VEC pSec;
	if (m_pLibpe->GetSecurity(pSec) != S_OK || nSertId > pSec->size())
		return -1;

	const auto& secEntry = pSec->at(nSertId).stWinSert;
	DWORD dwStart = pSec->at(nSertId).dwOffsetWinCertDesc + offsetof(WIN_CERTIFICATE, bCertificate);
	DWORD dwCertSize = (DWORD_PTR)secEntry.dwLength - offsetof(WIN_CERTIFICATE, bCertificate);
	m_pFileLoader->ShowFilePiece(dwStart, dwCertSize, &m_stHexEdit);

	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}
	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateListDelayImportEntry(DWORD dwEntry)
{
	PCLIBPE_DELAYIMPORT_VEC pDelayImport;
	if (m_pLibpe->GetDelayImport(pDelayImport) != S_OK || dwEntry > pDelayImport->size())
		return -1;

	m_listDelayImportEntry.DestroyWindow();
	m_listDelayImportEntry.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_DELAYIMPORT_ENTRY, &m_stListInfo);
	m_listDelayImportEntry.InsertColumn(0, L"Offset", 0, 90);
	m_listDelayImportEntry.SetHeaderColumnColor(0, g_clrOffset);
	m_listDelayImportEntry.InsertColumn(1, L"Function Name", 0, 300);
	m_listDelayImportEntry.InsertColumn(2, L"Ordinal / Hint", 0, 100);
	m_listDelayImportEntry.InsertColumn(3, L"ImportNameTable AddresOfData", 0, 220);
	m_listDelayImportEntry.InsertColumn(4, L"IAT AddresOfData", 0, 200);
	m_listDelayImportEntry.InsertColumn(5, L"BoundIAT AddresOfData", 0, 230);
	m_listDelayImportEntry.InsertColumn(6, L"UnloadInfoTable AddresOfData", 0, 240);

	int listindex = 0;
	WCHAR wstr[MAX_PATH];

	DWORD dwThunkOffset;
	m_pLibpe->GetOffsetFromRVA(pDelayImport->at(dwEntry).stDelayImpDesc.ImportNameTableRVA, dwThunkOffset);

	m_listDelayImportEntry.SetRedraw(FALSE);
	for (auto&i : pDelayImport->at(dwEntry).vecDelayImpFunc)
	{
		swprintf_s(wstr, 9, L"%08X", dwThunkOffset);
		m_listDelayImportEntry.InsertItem(listindex, wstr);
		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			dwThunkOffset += sizeof(IMAGE_THUNK_DATA32);
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			dwThunkOffset += sizeof(IMAGE_THUNK_DATA64);

		swprintf_s(wstr, 256, L"%S", i.strFuncName.data());
		m_listDelayImportEntry.SetItemText(listindex, 1, wstr);

		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
		{
			if (i.varThunk.st32.stImportNameTable.u1.Ordinal & IMAGE_ORDINAL_FLAG32)
				swprintf_s(wstr, 5, L"%04X", IMAGE_ORDINAL32(i.varThunk.st32.stImportNameTable.u1.Ordinal));
			else
				swprintf_s(wstr, 5, L"%04X", i.stImpByName.Hint);
		}
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
		{
			if (i.varThunk.st64.stImportNameTable.u1.Ordinal & IMAGE_ORDINAL_FLAG64)
				swprintf_s(wstr, 5, L"%04llX", IMAGE_ORDINAL64(i.varThunk.st64.stImportNameTable.u1.Ordinal));
			else
				swprintf_s(wstr, 5, L"%04X", i.stImpByName.Hint);
		}
		m_listDelayImportEntry.SetItemText(listindex, 2, wstr);

		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(wstr, 9, L"%08X", i.varThunk.st32.stImportNameTable.u1.AddressOfData);
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(wstr, 17, L"%016llX", i.varThunk.st64.stImportNameTable.u1.AddressOfData);
		m_listDelayImportEntry.SetItemText(listindex, 3, wstr);

		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(wstr, 9, L"%08X", i.varThunk.st32.stImportAddressTable.u1.AddressOfData);
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(wstr, 17, L"%016llX", i.varThunk.st64.stImportAddressTable.u1.AddressOfData);
		m_listDelayImportEntry.SetItemText(listindex, 4, wstr);

		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(wstr, 9, L"%08X", i.varThunk.st32.stBoundImportAddressTable.u1.AddressOfData);
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(wstr, 17, L"%016llX", i.varThunk.st64.stBoundImportAddressTable.u1.AddressOfData);
		m_listDelayImportEntry.SetItemText(listindex, 5, wstr);

		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(wstr, 9, L"%08X", i.varThunk.st32.stUnloadInformationTable.u1.AddressOfData);
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(wstr, 17, L"%016llX", i.varThunk.st64.stUnloadInformationTable.u1.AddressOfData);
		m_listDelayImportEntry.SetItemText(listindex, 6, wstr);

		listindex++;
	}
	m_listDelayImportEntry.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listDelayImportEntry.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pActiveWnd = &m_listDelayImportEntry;

	return 0;
}

int CViewRightBL::CreateListRelocsEntry(DWORD dwEntry)
{
	PCLIBPE_RELOCATION_VEC pReloc;
	if (m_pLibpe->GetRelocations(pReloc) != S_OK || pReloc->empty() || pReloc->size() < dwEntry)
		return -1;

	m_listRelocsEntry.DestroyWindow();
	m_listRelocsEntry.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_RELOCATIONS_ENTRY, &m_stListInfo);
	m_listRelocsEntry.ShowWindow(SW_HIDE);
	m_listRelocsEntry.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listRelocsEntry.SetHeaderColumnColor(0, g_clrOffset);
	m_listRelocsEntry.InsertColumn(1, L"Reloc type", LVCFMT_CENTER, 250);
	m_listRelocsEntry.InsertColumn(2, L"Offset to apply", LVCFMT_LEFT, 120);

	const std::map<WORD, std::wstring> mapRelocTypes {
		{ IMAGE_REL_BASED_ABSOLUTE, L"IMAGE_REL_BASED_ABSOLUTE" },
	{ IMAGE_REL_BASED_HIGH, L"IMAGE_REL_BASED_HIGH" },
	{ IMAGE_REL_BASED_LOW, L"IMAGE_REL_BASED_LOW" },
	{ IMAGE_REL_BASED_HIGHLOW, L"IMAGE_REL_BASED_HIGHLOW" },
	{ IMAGE_REL_BASED_HIGHADJ, L"IMAGE_REL_BASED_HIGHADJ" },
	{ IMAGE_REL_BASED_MACHINE_SPECIFIC_5, L"IMAGE_REL_BASED_MACHINE_SPECIFIC_5" },
	{ IMAGE_REL_BASED_RESERVED, L"IMAGE_REL_BASED_RESERVED" },
	{ IMAGE_REL_BASED_MACHINE_SPECIFIC_7, L"IMAGE_REL_BASED_MACHINE_SPECIFIC_7" },
	{ IMAGE_REL_BASED_MACHINE_SPECIFIC_8, L"IMAGE_REL_BASED_MACHINE_SPECIFIC_8" },
	{ IMAGE_REL_BASED_MACHINE_SPECIFIC_9, L"IMAGE_REL_BASED_MACHINE_SPECIFIC_9" },
	{ IMAGE_REL_BASED_DIR64, L"IMAGE_REL_BASED_DIR64" }
	};

	int listindex = 0;
	WCHAR wstr[MAX_PATH];

	m_listRelocsEntry.SetRedraw(FALSE);
	for (auto& iterRelocs : pReloc->at(dwEntry).vecRelocData)
	{
		swprintf_s(wstr, 9, L"%08X", iterRelocs.dwOffsetRelocData);
		m_listRelocsEntry.InsertItem(listindex, wstr);

		auto it = mapRelocTypes.find(iterRelocs.wRelocType);
		if (it != mapRelocTypes.end())
			swprintf_s(wstr, MAX_PATH, L"%s", it->second.data());
		else
			swprintf_s(wstr, MAX_PATH, L"%u", iterRelocs.wRelocType);

		m_listRelocsEntry.SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 5, L"%04X", iterRelocs.wRelocOffset);
		m_listRelocsEntry.SetItemText(listindex, 2, wstr);

		listindex++;
	}
	m_listRelocsEntry.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listRelocsEntry.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pActiveWnd = &m_listRelocsEntry;

	return 0;
}

int CViewRightBL::CreateHexDebugEntry(DWORD dwEntry)
{
	PCLIBPE_DEBUG_VEC pDebug;
	if (m_pLibpe->GetDebug(pDebug) != S_OK)
		return -1;

	auto& rDebugDir = pDebug->at(dwEntry).stDebugDir;
	m_pFileLoader->ShowFilePiece(rDebugDir.PointerToRawData, rDebugDir.SizeOfData, &m_stHexEdit);

	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}
	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateTreeResources()
{
	PCLIBPE_RESOURCE_ROOT pstResRoot;
	if (m_pLibpe->GetResources(pstResRoot) != S_OK)
		return -1;

	m_treeResBottom.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		TVS_LINESATROOT, CRect(0, 0, 0, 0), this, IDC_TREE_RESOURCE_BOTTOM);
	m_treeResBottom.ShowWindow(SW_HIDE);

	WCHAR wstr[MAX_PATH];

	m_imglTreeRes.Create(16, 16, ILC_COLOR32, 0, 4);
	const int iconDirs = m_imglTreeRes.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_DIR_ICON));
	m_treeResBottom.SetImageList(&m_imglTreeRes, TVSIL_NORMAL);
	long ilvlRoot = 0, ilvl2 = 0, ilvl3 = 0;

	//Creating a treeCtrl and setting, with SetItemData(),
	//a unique id for each node, that is an index in vector (m_vecResId),
	//that holds tuple of three IDs of resource — Type, Name, LangID.
	for (auto& iterRoot : pstResRoot->vecResRoot)
	{
		const IMAGE_RESOURCE_DIRECTORY_ENTRY* pResDirEntry = &iterRoot.stResDirEntryRoot;
		if (pResDirEntry->NameIsString)
			//Enclose in double quotes.
			swprintf(wstr, MAX_PATH, L"\u00AB%s\u00BB", iterRoot.wstrResNameRoot.data());
		else
		{	//Setting Treectrl root node name depending on Resource typeID.
			if (g_mapResType.find(pResDirEntry->Id) != g_mapResType.end())
				swprintf(wstr, MAX_PATH, L"%s [Id: %u]", g_mapResType.at(pResDirEntry->Id).data(), pResDirEntry->Id);
			else
				swprintf(wstr, MAX_PATH, L"%u", pResDirEntry->Id);
		}

		const HTREEITEM treeRoot = m_treeResBottom.InsertItem(wstr, iconDirs, iconDirs);
		m_vecResId.emplace_back(ilvlRoot, -1, -1);
		m_treeResBottom.SetItemData(treeRoot, m_vecResId.size() - 1);
		ilvl2 = 0;
		LIBPE_RESOURCE_LVL2 pstResLvL2 = iterRoot.stResLvL2;

		for (auto& iterLvL2 : pstResLvL2.vecResLvL2)
		{
			m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
			ilvl3 = 0;

			//			pResDirEntry = &std::get<0>(iterLvL2);
			LIBPE_RESOURCE_LVL3 pstResLvL3 = iterLvL2.stResLvL3;

			for (auto& iterLvL3 : pstResLvL3.vecResLvL3)
			{
				pResDirEntry = &iterLvL3.stResDirEntryLvL3;
				if (pResDirEntry->NameIsString)
					swprintf(wstr, MAX_PATH, L"«%s» - lang: %i", iterLvL2.wstrResNameLvL2.data(), pResDirEntry->Id);
				else
					swprintf(wstr, MAX_PATH, L"%u - lang: %i", iterLvL2.stResDirEntryLvL2.Id, pResDirEntry->Id);

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
	PCLIBPE_TLS pTLS;
	if (m_pLibpe->GetTLS(pTLS) != S_OK)
		return -1;

	if (m_pActiveWnd != &m_stHexEdit)
	{
		if (m_pActiveWnd)
			m_pActiveWnd->ShowWindow(SW_HIDE);
		m_pActiveWnd = &m_stHexEdit;
	}
	CRect rc;
	GetClientRect(&rc);
	m_stHexEdit.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pFileLoader->ShowFilePiece(pTLS->dwRawDataOffset, pTLS->dwRawDataSize, &m_stHexEdit);

	return 0;
}