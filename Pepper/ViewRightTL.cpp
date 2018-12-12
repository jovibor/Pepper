#include "stdafx.h"
#include "ViewRightTL.h"
#include "version.h"

IMPLEMENT_DYNCREATE(CViewRightTL, CView)

BEGIN_MESSAGE_MAP(CViewRightTL, CView)
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_SECHEADERS, &CViewRightTL::OnListSectionsGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_IMPORT, &CViewRightTL::OnListImportGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_RELOCATIONS, &CViewRightTL::OnListRelocGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_EXCEPTION, &CViewRightTL::OnListExceptionGetDispInfo)
END_MESSAGE_MAP()

void CViewRightTL::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();
	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	if (!m_pLibpe)
		return;

	LOGFONT lf { };
	StringCchCopyW(lf.lfFaceName, 18, L"Consolas");
	lf.lfHeight = 22;
	if (!m_fontSummary.CreateFontIndirectW(&lf))
	{
		StringCchCopyW(lf.lfFaceName, 18, L"Times New Roman");
		m_fontSummary.CreateFontIndirectW(&lf);
	}

	m_strFileName = m_pMainDoc->GetPathName();
	m_strFileName.erase(0, m_strFileName.find_last_of('\\') + 1);
	m_strFileName.insert(0, L"File name: ");

	const DWORD* m_pFileSummary { };
	if (m_pLibpe->GetFileSummary(m_pFileSummary) != S_OK)
		return;

	m_dwFileSummary = *m_pFileSummary;

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
		m_strFileType = L"File type: PE32 (x86)";
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
		m_strFileType = L"File type: PE32+ (x64)";
	else
		m_strFileType = L"File type: unknown";

	WCHAR strVersion[MAX_PATH] { };
	swprintf_s(strVersion, MAX_PATH, L"%S, version: %u.%u.%u", PRODUCT_NAME, MAJOR_VERSION, MINOR_VERSION, MAINTENANCE_VERSION);
	m_strVersion = strVersion;

	m_pLibpe->GetSectionsHeaders(m_pSecHeaders);
	m_pLibpe->GetImportTable(m_pImportTable);
	m_pLibpe->GetExceptionTable(m_pExceptionDir);
	m_pLibpe->GetRelocationTable(m_pRelocTable);

	m_stListInfo.clrListTooltipText = RGB(255, 255, 255);
	m_stListInfo.clrListTooltipBk = RGB(0, 132, 132);
	m_stListInfo.clrHeaderText = RGB(255, 255, 255);
	m_stListInfo.clrHeaderBk = RGB(0, 132, 132);
	m_stListInfo.dwHeaderHeight = 39;

	CreateListDOSHeader();
	CreateListRichHeader();
	CreateListNTHeader();
	CreateListFileHeader();
	CreateListOptHeader();
	CreateListDataDirectories();
	CreateListSecHeaders();
	CreateListExport();
	CreateListImport();
	CreateTreeResources();
	CreateListException();
	CreateListSecurity();
	CreateListRelocation();
	CreateListDebug();
	CreateListTLS();
	CreateListLoadConfigTable();
	CreateListBoundImport();
	CreateListDelayImport();
	CreateListCOM();
}

void CViewRightTL::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	//Check m_pChildFrame to prevent some UB.
	//OnUpdate can be invoked before OnInitialUpdate, weird MFC.
	if (!m_pChildFrame)
		return;

	if (m_pActiveList)
		m_pActiveList->ShowWindow(SW_HIDE);

	m_fFileSummaryShow = false;

	CRect rectClient, rect;
	::GetClientRect(AfxGetMainWnd()->m_hWnd, &rectClient);
	GetClientRect(&rect);

	switch (lHint)
	{
	case IDC_SHOW_FILE_SUMMARY:
		m_fFileSummaryShow = true;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_DOSHEADER:
		m_listDOSHeader.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listDOSHeader;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_RICHHEADER:
		m_listRichHdr.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listRichHdr;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_NTHEADER:
		m_listNTHeader.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listNTHeader;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_FILEHEADER:
		m_listFileHeader.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listFileHeader;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_OPTIONALHEADER:
		m_listOptHeader.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listOptHeader;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_DATADIRECTORIES:
		m_listDataDirs.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listDataDirs;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_SECHEADERS:
		m_listSecHeaders.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listSecHeaders;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_EXPORT:
		m_listExportDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listExportDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height() / 2, 0);
		break;
	case IDC_LIST_IAT:
	case IDC_LIST_IMPORT:
		m_listImport.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listImport;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height() / 2, 0);
		break;
	case IDC_TREE_RESOURCE:
		m_treeResTop.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_treeResTop;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height() / 2, 0);
		break;
	case IDC_LIST_EXCEPTION:
		m_listExceptionDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listExceptionDir;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_SECURITY:
		m_listSecurityDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listSecurityDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height() / 2, 0);
		break;
	case IDC_LIST_RELOCATIONS:
		m_listRelocDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listRelocDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height() / 2, 0);
		break;
	case IDC_LIST_DEBUG:
		m_listDebugDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listDebugDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height() / 2, 0);
		break;
	case IDC_LIST_TLS:
		m_listTLSDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listTLSDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height() / 2, 0);
		break;
	case IDC_LIST_LOADCONFIG:
		m_listLoadConfigDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listLoadConfigDir;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_BOUNDIMPORT:
		m_listBoundImportDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height() / 2, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listBoundImportDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	case IDC_LIST_DELAYIMPORT:
		m_listDelayImportDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listDelayImportDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height() / 2, 0);
		break;
	case IDC_LIST_COMDESCRIPTOR:
		m_listCOMDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listCOMDir;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		m_pChildFrame->m_stSplitterRight.SetRowInfo(0, rectClient.Height(), 0);
		break;
	}
	m_pChildFrame->m_stSplitterRight.RecalcLayout();
}

void CViewRightTL::OnDraw(CDC* pDC)
{
	//Printing app name/version info and
	//currently oppened file's type and name.
	if (m_fFileSummaryShow)
	{
		pDC->SelectObject(m_fontSummary);
		CRect rect { 20, 20, 400, 150 };

		GetTextExtentPoint32W(pDC->m_hDC, m_strFileName.c_str(), m_strFileName.length(), &m_sizeTextToDraw);
		if (m_sizeTextToDraw.cx > rect.Width())
			rect.right = m_sizeTextToDraw.cx + rect.left + 30;
		pDC->Rectangle(&rect);

		pDC->SetTextColor(RGB(200, 50, 30));
		GetTextExtentPoint32W(pDC->m_hDC, m_strVersion.c_str(), m_strVersion.length(), &m_sizeTextToDraw);
		ExtTextOutW(pDC->m_hDC, (rect.Width() - m_sizeTextToDraw.cx) / 2 + rect.left, 10, 0, nullptr,
			m_strVersion.c_str(), m_strVersion.length(), nullptr);

		pDC->SetTextColor(RGB(0, 0, 255));
		ExtTextOutW(pDC->m_hDC, 35, 25 + m_sizeTextToDraw.cy, 0, nullptr, m_strFileType.c_str(), m_strFileType.length(), nullptr);
		ExtTextOutW(pDC->m_hDC, 35, 55 + m_sizeTextToDraw.cy, 0, nullptr, m_strFileName.c_str(), m_strFileName.length(), nullptr);
	}
}

void CViewRightTL::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveList)
		m_pActiveList->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightTL::OnListSectionsGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	LV_ITEM* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		WCHAR str[50] { };

		switch (pItem->iSubItem)
		{
		case 0:
			if (std::get<1>(m_pSecHeaders->at(pItem->iItem)).empty())
				swprintf_s(str, 9, L"%.8S", std::get<0>(m_pSecHeaders->at(pItem->iItem)).Name);
			else
				swprintf_s(str, 50, L"%.8S (%S)", std::get<0>(m_pSecHeaders->at(pItem->iItem)).Name,
					std::get<1>(m_pSecHeaders->at(pItem->iItem)).c_str());
			break;
		case 1:
			swprintf_s(str, 9, L"%08X", std::get<0>(m_pSecHeaders->at(pItem->iItem)).Misc.VirtualSize);
			break;
		case 2:
			swprintf_s(str, 9, L"%08X", std::get<0>(m_pSecHeaders->at(pItem->iItem)).VirtualAddress);
			break;
		case 3:
			swprintf_s(str, 9, L"%08X", std::get<0>(m_pSecHeaders->at(pItem->iItem)).SizeOfRawData);
			break;
		case 4:
			swprintf_s(str, 9, L"%08X", std::get<0>(m_pSecHeaders->at(pItem->iItem)).PointerToRawData);
			break;
		case 5:
			swprintf_s(str, 9, L"%08X", std::get<0>(m_pSecHeaders->at(pItem->iItem)).PointerToRelocations);
			break;
		case 6:
			swprintf_s(str, 9, L"%08X", std::get<0>(m_pSecHeaders->at(pItem->iItem)).PointerToLinenumbers);
			break;
		case 7:
			swprintf_s(str, 5, L"%04X", std::get<0>(m_pSecHeaders->at(pItem->iItem)).NumberOfRelocations);
			break;
		case 8:
			swprintf_s(str, 5, L"%04X", std::get<0>(m_pSecHeaders->at(pItem->iItem)).NumberOfLinenumbers);
			break;
		case 9:
			swprintf_s(str, 9, L"%08X", std::get<0>(m_pSecHeaders->at(pItem->iItem)).Characteristics);
			break;
		}
		lstrcpynW(pItem->pszText, str, pItem->cchTextMax);
	}

	*pResult = 0;
}

void CViewRightTL::OnListImportGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	LV_ITEM* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		WCHAR str[MAX_PATH] { };
		const IMAGE_IMPORT_DESCRIPTOR* pImpDescriptor = &std::get<0>(m_pImportTable->at(pItem->iItem));

		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(str, MAX_PATH, L"%S (%u)", std::get<1>(m_pImportTable->at(pItem->iItem)).c_str(),
				std::get<2>(m_pImportTable->at(pItem->iItem)).size());
			break;
		case 1:
			swprintf_s(str, 9, L"%08X", pImpDescriptor->OriginalFirstThunk);
			break;
		case 2:
			swprintf_s(str, 9, L"%08X", pImpDescriptor->TimeDateStamp);
			break;
		case 3:
			swprintf_s(str, 9, L"%08X", pImpDescriptor->ForwarderChain);
			break;
		case 4:
			swprintf_s(str, 9, L"%08X", pImpDescriptor->Name);
			break;
		case 5:
			swprintf_s(str, 9, L"%08X", pImpDescriptor->FirstThunk);
			break;
		}
		lstrcpynW(pItem->pszText, str, pItem->cchTextMax);
	}

	*pResult = 0;
}

void CViewRightTL::OnListRelocGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	LVITEMW* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		const IMAGE_BASE_RELOCATION* pDirReloc = &std::get<0>(m_pRelocTable->at(pItem->iItem));
		WCHAR str[MAX_PATH] { };

		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(str, 9, L"%08X", pDirReloc->VirtualAddress);
			break;
		case 1:
			swprintf_s(str, 9, L"%08X", pDirReloc->SizeOfBlock);
			break;
		case 2:
			swprintf_s(str, MAX_PATH, L"%u", (pDirReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD));
			break;
		}
		lstrcpynW(pItem->pszText, str, pItem->cchTextMax);
	}

	*pResult = 0;
}

void CViewRightTL::OnListExceptionGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	LV_ITEM* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		WCHAR str[9] { };

		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(str, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).BeginAddress);
			break;
		case 1:
			swprintf_s(str, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).EndAddress);
			break;
		case 2:
			swprintf_s(str, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).UnwindData);
			break;
		}
		lstrcpynW(pItem->pszText, str, pItem->cchTextMax);
	}

	*pResult = 0;
}

BOOL CViewRightTL::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	CScrollView::OnNotify(wParam, lParam, pResult);

	const LPNMITEMACTIVATE pNMI = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
	if (pNMI->iItem == -1)
		return TRUE;

	switch (pNMI->hdr.idFrom)
	{
	case IDC_LIST_IMPORT:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_IMPORT_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_SECURITY:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_SECURITY_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_RELOCATIONS:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_RELOCATIONS_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_DELAYIMPORT:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_DELAYIMPORT_FUNCS, pNMI->iItem));
		break;
	case IDC_TREE_RESOURCE_TOP:
	{
		const LPNMTREEVIEW pTree = reinterpret_cast<LPNMTREEVIEW>(lParam);
		if (pTree->hdr.code == TVN_SELCHANGED && pTree->itemNew.hItem != m_hTreeResDir)
		{
			PCLIBPE_RESOURCE_ROOT_TUP pTupResRoot { };

			if (m_pLibpe->GetResourceTable(pTupResRoot) != S_OK)
				return -1;

			const DWORD_PTR dwResId = m_treeResTop.GetItemData(pTree->itemNew.hItem);
			const long idlvlRoot = std::get<0>(m_vecResId.at(dwResId));
			const long idlvl2 = std::get<1>(m_vecResId.at(dwResId));
			const long idlvl3 = std::get<2>(m_vecResId.at(dwResId));

			auto& rootvec = std::get<1>(*pTupResRoot);
			if (idlvl2 >= 0)
			{
				auto& lvl2tup = std::get<4>(rootvec.at(idlvlRoot));
				auto& lvl2vec = std::get<1>(lvl2tup);

				if (!lvl2vec.empty())
				{
					if (idlvl3 >= 0)
					{
						auto& lvl3tup = std::get<4>(lvl2vec.at(idlvl2));
						auto& lvl3vec = std::get<1>(lvl3tup);

						if (!lvl3vec.empty())
						{
							auto& data = std::get<3>(lvl3vec.at(idlvl3));

							if (!data.empty())
								//Send data vector pointer to CViewRightTR
								//to display raw data.
								m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_HEX_RIGHT_TOP_RIGHT, 0), (CObject*)&data);
						}
					}
				}
			}
		}
	}
	break;
	case IDC_LIST_DEBUG:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_DEBUG_ENTRY, pNMI->iItem));
		break;
	}
	return TRUE;
}

int CViewRightTL::CreateListDOSHeader()
{
	PCLIBPE_DOSHEADER pDosHeader { };
	if (m_pLibpe->GetMSDOSHeader(pDosHeader) != S_OK)
		return -1;

	m_listDOSHeader.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_DOSHEADER, &m_stListInfo);
	m_listDOSHeader.ShowWindow(SW_HIDE);
	m_listDOSHeader.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDOSHeader.InsertColumn(0, L"Name", LVCFMT_CENTER, 150);
	m_listDOSHeader.InsertColumn(1, L"Offset", LVCFMT_LEFT, 100);
	m_listDOSHeader.InsertColumn(2, L"Size [BYTES]", LVCFMT_LEFT, 100);
	m_listDOSHeader.InsertColumn(3, L"Value", LVCFMT_LEFT, 100);

	m_dwPeStart = pDosHeader->e_lfanew;

	WCHAR str[9] { };
	int listindex = 0;

	m_listDOSHeader.InsertItem(listindex, L"e_magic");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_magic));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_magic));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 3, L"%02X", pDosHeader->e_magic & 0xFF);
	swprintf_s(&str[2], 3, L"%02X", (pDosHeader->e_magic >> 8) & 0xFF);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_cblp");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cblp));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_cblp));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_cblp);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_cp");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cp));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_cp));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_cp);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_crlc");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_crlc));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_crlc));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_crlc);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_cparhdr");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cparhdr));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_cparhdr));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_cparhdr);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_minalloc");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_minalloc));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_minalloc));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_minalloc);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_maxalloc");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_maxalloc));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_maxalloc));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_maxalloc);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_ss");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_ss));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_ss));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_ss);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_sp");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_sp));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_sp));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_sp);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_csum");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_csum));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_csum));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_csum);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_ip");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_ip));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_ip));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_ip);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_cs");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cs));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_cs));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_cs);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_lfarlc");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_lfarlc));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_lfarlc));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_lfarlc);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_ovno");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_ovno));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_ovno));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_ovno);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_res[0]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[0]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res[0]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res[0]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res[1]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[1]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res[1]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res[1]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res[2]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[2]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res[2]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res[2]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res[3]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[3]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res[3]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res[3]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_oemid");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_oemid));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_oemid));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_oemid);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_oeminfo");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_oeminfo));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_oeminfo));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_oeminfo);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_res2[0]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[0]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[0]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[0]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res2[1]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[1]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[1]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[1]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res2[2]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[2]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[2]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[2]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res2[3]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[3]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[3]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[3]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res2[4]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[4]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[4]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[4]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res2[5]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[5]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[5]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[5]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res2[6]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[6]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[6]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[6]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res2[7]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[7]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[7]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[7]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res2[8]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[8]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[8]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[8]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"   e_res2[9]");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[9]));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_res2[9]));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pDosHeader->e_res2[9]);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	listindex = m_listDOSHeader.InsertItem(listindex + 1, L"e_lfanew");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_lfanew));
	m_listDOSHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pDosHeader->e_lfanew));
	m_listDOSHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDosHeader->e_lfanew);
	m_listDOSHeader.SetItemText(listindex, 3, str);

	return 0;
}

int CViewRightTL::CreateListRichHeader()
{
	PCLIBPE_RICHHEADER_VEC pRichHeader { };
	if (m_pLibpe->GetRichHeader(pRichHeader) != S_OK)
		return -1;

	m_listRichHdr.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_RICHHEADER, &m_stListInfo);
	m_listRichHdr.ShowWindow(SW_HIDE);
	m_listRichHdr.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listRichHdr.InsertColumn(0, L"\u2116", LVCFMT_CENTER, 35);
	m_listRichHdr.InsertColumn(1, L"ID [Hex]", LVCFMT_LEFT, 100);
	m_listRichHdr.InsertColumn(2, L"Version", LVCFMT_LEFT, 100);
	m_listRichHdr.InsertColumn(3, L"Occurrences", LVCFMT_LEFT, 100);

	WCHAR str[MAX_PATH] { };
	int listindex = 0;

	for (auto& i : *pRichHeader)
	{
		swprintf_s(str, MAX_PATH, L"%i", listindex + 1);
		m_listRichHdr.InsertItem(listindex, str);
		swprintf_s(str, MAX_PATH, L"%04X", std::get<0>(i));
		m_listRichHdr.SetItemText(listindex, 1, str);
		swprintf_s(str, MAX_PATH, L"%i", std::get<1>(i));
		m_listRichHdr.SetItemText(listindex, 2, str);
		swprintf_s(str, MAX_PATH, L"%i", std::get<2>(i));
		m_listRichHdr.SetItemText(listindex, 3, str);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListNTHeader()
{
	PCLIBPE_NTHEADER_VAR pVarNTHdr { };
	if (m_pLibpe->GetNTHeader(pVarNTHdr) != S_OK)
		return -1;

	m_listNTHeader.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_NTHEADER, &m_stListInfo);
	m_listNTHeader.ShowWindow(SW_HIDE);
	m_listNTHeader.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listNTHeader.InsertColumn(0, L"Name", LVCFMT_CENTER, 100);
	m_listNTHeader.InsertColumn(1, L"Offset", LVCFMT_LEFT, 100);
	m_listNTHeader.InsertColumn(2, L"Size [BYTES]", LVCFMT_LEFT, 100);
	m_listNTHeader.InsertColumn(3, L"Value", LVCFMT_LEFT, 100);

	WCHAR str[9] { };
	UINT listindex = 0;

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		const IMAGE_NT_HEADERS32* pNTHeader32 = &std::get<0>(*pVarNTHdr);

		listindex = m_listNTHeader.InsertItem(listindex, L"Signature");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, Signature) + m_dwPeStart);
		m_listNTHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pNTHeader32->Signature));
		m_listNTHeader.SetItemText(listindex, 2, str);
		swprintf_s(&str[0], 3, L"%02X", (BYTE)(pNTHeader32->Signature >> 0));
		swprintf_s(&str[2], 3, L"%02X", (BYTE)(pNTHeader32->Signature >> 8));
		swprintf_s(&str[4], 3, L"%02X", (BYTE)(pNTHeader32->Signature >> 16));
		swprintf_s(&str[6], 3, L"%02X", (BYTE)(pNTHeader32->Signature >> 24));
		m_listNTHeader.SetItemText(listindex, 3, str);
	}
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
	{
		const IMAGE_NT_HEADERS64* pNTHeader64 = &std::get<1>(*pVarNTHdr);

		listindex = m_listNTHeader.InsertItem(listindex, L"Signature");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, Signature) + m_dwPeStart);
		m_listNTHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pNTHeader64->Signature));
		m_listNTHeader.SetItemText(listindex, 2, str);
		swprintf_s(&str[0], 3, L"%02X", (BYTE)(pNTHeader64->Signature >> 0));
		swprintf_s(&str[2], 3, L"%02X", (BYTE)(pNTHeader64->Signature >> 8));
		swprintf_s(&str[4], 3, L"%02X", (BYTE)(pNTHeader64->Signature >> 16));
		swprintf_s(&str[6], 3, L"%02X", (BYTE)(pNTHeader64->Signature >> 24));
		m_listNTHeader.SetItemText(listindex, 3, str);
	}

	return 0;
}

int CViewRightTL::CreateListFileHeader()
{
	PCLIBPE_FILEHEADER pFileHeader { };
	if (m_pLibpe->GetFileHeader(pFileHeader) != S_OK)
		return -1;

	m_listFileHeader.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_FILEHEADER, &m_stListInfo);

	m_listFileHeader.ShowWindow(SW_HIDE);
	m_listFileHeader.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listFileHeader.InsertColumn(0, L"Name", LVCFMT_CENTER, 200);
	m_listFileHeader.InsertColumn(1, L"Offset", LVCFMT_LEFT, 100);
	m_listFileHeader.InsertColumn(2, L"Size [BYTES]", LVCFMT_LEFT, 100);
	m_listFileHeader.InsertColumn(3, L"Value", LVCFMT_LEFT, 300);

	std::map<WORD, std::wstring> mapMachineType {
		{ IMAGE_FILE_MACHINE_UNKNOWN, L"IMAGE_FILE_MACHINE_UNKNOWN" },
	{ IMAGE_FILE_MACHINE_TARGET_HOST, L"IMAGE_FILE_MACHINE_TARGET_HOST" },
	{ IMAGE_FILE_MACHINE_I386, L"IMAGE_FILE_MACHINE_I386" },
	{ IMAGE_FILE_MACHINE_R3000, L"IMAGE_FILE_MACHINE_R3000" },
	{ IMAGE_FILE_MACHINE_R4000, L"IMAGE_FILE_MACHINE_R4000" },
	{ IMAGE_FILE_MACHINE_R10000, L"IMAGE_FILE_MACHINE_R10000" },
	{ IMAGE_FILE_MACHINE_WCEMIPSV2, L"IMAGE_FILE_MACHINE_WCEMIPSV2" },
	{ IMAGE_FILE_MACHINE_ALPHA, L"IMAGE_FILE_MACHINE_ALPHA" },
	{ IMAGE_FILE_MACHINE_SH3, L"IMAGE_FILE_MACHINE_SH3" },
	{ IMAGE_FILE_MACHINE_SH3DSP, L"IMAGE_FILE_MACHINE_SH3DSP" },
	{ IMAGE_FILE_MACHINE_SH3E, L"IMAGE_FILE_MACHINE_SH3E" },
	{ IMAGE_FILE_MACHINE_SH4, L"IMAGE_FILE_MACHINE_SH4" },
	{ IMAGE_FILE_MACHINE_SH5, L"IMAGE_FILE_MACHINE_SH5" },
	{ IMAGE_FILE_MACHINE_ARM, L"IMAGE_FILE_MACHINE_ARM" },
	{ IMAGE_FILE_MACHINE_THUMB, L"IMAGE_FILE_MACHINE_THUMB" },
	{ IMAGE_FILE_MACHINE_ARMNT, L"IMAGE_FILE_MACHINE_ARMNT" },
	{ IMAGE_FILE_MACHINE_AM33, L"IMAGE_FILE_MACHINE_AM33" },
	{ IMAGE_FILE_MACHINE_POWERPC, L"IMAGE_FILE_MACHINE_POWERPC" },
	{ IMAGE_FILE_MACHINE_POWERPCFP, L"IMAGE_FILE_MACHINE_POWERPCFP" },
	{ IMAGE_FILE_MACHINE_IA64, L"IMAGE_FILE_MACHINE_IA64" },
	{ IMAGE_FILE_MACHINE_MIPS16, L"IMAGE_FILE_MACHINE_MIPS16" },
	{ IMAGE_FILE_MACHINE_ALPHA64, L"IMAGE_FILE_MACHINE_ALPHA64" },
	{ IMAGE_FILE_MACHINE_MIPSFPU, L"IMAGE_FILE_MACHINE_MIPSFPU" },
	{ IMAGE_FILE_MACHINE_MIPSFPU16, L"IMAGE_FILE_MACHINE_MIPSFPU16" },
	{ IMAGE_FILE_MACHINE_TRICORE, L"IMAGE_FILE_MACHINE_TRICORE" },
	{ IMAGE_FILE_MACHINE_CEF, L"IMAGE_FILE_MACHINE_CEF" },
	{ IMAGE_FILE_MACHINE_EBC, L"IMAGE_FILE_MACHINE_EBC" },
	{ IMAGE_FILE_MACHINE_AMD64, L"IMAGE_FILE_MACHINE_AMD64" },
	{ IMAGE_FILE_MACHINE_M32R, L"IMAGE_FILE_MACHINE_M32R" },
	{ IMAGE_FILE_MACHINE_ARM64, L"IMAGE_FILE_MACHINE_ARM64" },
	{ IMAGE_FILE_MACHINE_CEE, L"IMAGE_FILE_MACHINE_CEE" }
	};

	std::map<WORD, std::wstring> mapCharacteristics {
		{ IMAGE_FILE_RELOCS_STRIPPED, L"IMAGE_FILE_RELOCS_STRIPPED" },
	{ IMAGE_FILE_EXECUTABLE_IMAGE, L"IMAGE_FILE_EXECUTABLE_IMAGE" },
	{ IMAGE_FILE_LINE_NUMS_STRIPPED, L"IMAGE_FILE_LINE_NUMS_STRIPPED" },
	{ IMAGE_FILE_LOCAL_SYMS_STRIPPED, L"IMAGE_FILE_LOCAL_SYMS_STRIPPED" },
	{ IMAGE_FILE_AGGRESIVE_WS_TRIM, L"IMAGE_FILE_AGGRESIVE_WS_TRIM" },
	{ IMAGE_FILE_LARGE_ADDRESS_AWARE, L"IMAGE_FILE_LARGE_ADDRESS_AWARE" },
	{ IMAGE_FILE_BYTES_REVERSED_LO, L"IMAGE_FILE_BYTES_REVERSED_LO" },
	{ IMAGE_FILE_32BIT_MACHINE, L"IMAGE_FILE_32BIT_MACHINE" },
	{ IMAGE_FILE_DEBUG_STRIPPED, L"IMAGE_FILE_DEBUG_STRIPPED" },
	{ IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP, L"IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP" },
	{ IMAGE_FILE_NET_RUN_FROM_SWAP, L"IMAGE_FILE_NET_RUN_FROM_SWAP" },
	{ IMAGE_FILE_SYSTEM, L"IMAGE_FILE_SYSTEM" },
	{ IMAGE_FILE_DLL, L"IMAGE_FILE_DLL" },
	{ IMAGE_FILE_UP_SYSTEM_ONLY, L"IMAGE_FILE_UP_SYSTEM_ONLY" },
	{ IMAGE_FILE_BYTES_REVERSED_HI, L"IMAGE_FILE_BYTES_REVERSED_HI" },
	};

	WCHAR str[MAX_PATH * 2] { };
	int listindex = 0;

	listindex = m_listFileHeader.InsertItem(listindex, L"Machine");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.Machine) + m_dwPeStart);
	m_listFileHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pFileHeader->Machine));
	m_listFileHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pFileHeader->Machine);
	m_listFileHeader.SetItemText(listindex, 3, str);
	auto iterMachine = mapMachineType.find(pFileHeader->Machine);
	if (iterMachine != mapMachineType.end())
		m_listFileHeader.SetItemTooltip(listindex, 3, iterMachine->second, L"Machine:");

	listindex = m_listFileHeader.InsertItem(listindex + 1, L"NumberOfSections");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.NumberOfSections) + m_dwPeStart);
	m_listFileHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pFileHeader->NumberOfSections));
	m_listFileHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pFileHeader->NumberOfSections);
	m_listFileHeader.SetItemText(listindex, 3, str);

	listindex = m_listFileHeader.InsertItem(listindex + 1, L"TimeDateStamp");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.TimeDateStamp) + m_dwPeStart);
	m_listFileHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pFileHeader->TimeDateStamp));
	m_listFileHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, MAX_PATH, L"%08X", pFileHeader->TimeDateStamp);
	m_listFileHeader.SetItemText(listindex, 3, str);
	if (pFileHeader->TimeDateStamp)
	{
		__time64_t time = pFileHeader->TimeDateStamp;
		_wctime64_s(str, MAX_PATH, &time);
		m_listFileHeader.SetItemTooltip(listindex, 3, str, L"Time / Date:");
	}

	listindex = m_listFileHeader.InsertItem(listindex + 1, L"PointerToSymbolTable");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.PointerToSymbolTable) + m_dwPeStart);
	m_listFileHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pFileHeader->PointerToSymbolTable));
	m_listFileHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pFileHeader->PointerToSymbolTable);
	m_listFileHeader.SetItemText(listindex, 3, str);

	listindex = m_listFileHeader.InsertItem(listindex + 1, L"NumberOfSymbols");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.NumberOfSymbols) + m_dwPeStart);
	m_listFileHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pFileHeader->NumberOfSymbols));
	m_listFileHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pFileHeader->NumberOfSymbols);
	m_listFileHeader.SetItemText(listindex, 3, str);

	listindex = m_listFileHeader.InsertItem(listindex + 1, L"SizeOfOptionalHeader");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.SizeOfOptionalHeader) + m_dwPeStart);
	m_listFileHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pFileHeader->SizeOfOptionalHeader));
	m_listFileHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%04X", pFileHeader->SizeOfOptionalHeader);
	m_listFileHeader.SetItemText(listindex, 3, str);

	listindex = m_listFileHeader.InsertItem(listindex + 1, L"Characteristics");
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.Characteristics) + m_dwPeStart);
	m_listFileHeader.SetItemText(listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(pFileHeader->Characteristics));
	m_listFileHeader.SetItemText(listindex, 2, str);
	swprintf_s(str, 5, L"%04X", pFileHeader->Characteristics);
	m_listFileHeader.SetItemText(listindex, 3, str);
	std::wstring  strCharact { };
	for (auto& i : mapCharacteristics)
		if (i.first & pFileHeader->Characteristics)
			strCharact += i.second + L"\n";
	if (!strCharact.empty())
	{
		strCharact.erase(strCharact.size() - 1); //to remove last '\n'
		m_listFileHeader.SetItemTooltip(listindex, 3, strCharact, L"Characteristics:");
	}
	return 0;
}

int CViewRightTL::CreateListOptHeader()
{
	PCLIBPE_OPTHEADER_VAR pOptionalHdr { };
	if (m_pLibpe->GetOptionalHeader(pOptionalHdr) != S_OK)
		return -1;

	m_listOptHeader.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_OPTIONALHEADER, &m_stListInfo);
	m_listOptHeader.ShowWindow(SW_HIDE);
	m_listOptHeader.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	m_listOptHeader.InsertColumn(0, L"Name", LVCFMT_CENTER, 215);
	m_listOptHeader.InsertColumn(1, L"Offset", LVCFMT_LEFT, 100);
	m_listOptHeader.InsertColumn(2, L"Size [BYTES]", LVCFMT_LEFT, 100);
	m_listOptHeader.InsertColumn(3, L"Value", LVCFMT_LEFT, 140);

	std::map<WORD, std::string> mapSubSystem {
		{ IMAGE_SUBSYSTEM_UNKNOWN, "IMAGE_SUBSYSTEM_UNKNOWN" },
	{ IMAGE_SUBSYSTEM_NATIVE, "IMAGE_SUBSYSTEM_NATIVE" },
	{ IMAGE_SUBSYSTEM_WINDOWS_GUI, "IMAGE_SUBSYSTEM_WINDOWS_GUI" },
	{ IMAGE_SUBSYSTEM_WINDOWS_CUI, "IMAGE_SUBSYSTEM_WINDOWS_CUI" },
	{ IMAGE_SUBSYSTEM_OS2_CUI, "IMAGE_SUBSYSTEM_OS2_CUI" },
	{ IMAGE_SUBSYSTEM_POSIX_CUI, "IMAGE_SUBSYSTEM_POSIX_CUI" },
	{ IMAGE_SUBSYSTEM_NATIVE_WINDOWS, "IMAGE_SUBSYSTEM_NATIVE_WINDOWS" },
	{ IMAGE_SUBSYSTEM_WINDOWS_CE_GUI, "IMAGE_SUBSYSTEM_WINDOWS_CE_GUI" },
	{ IMAGE_SUBSYSTEM_EFI_APPLICATION, "IMAGE_SUBSYSTEM_EFI_APPLICATION" },
	{ IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER, "IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER" },
	{ IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER, "IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER" },
	{ IMAGE_SUBSYSTEM_EFI_ROM, "IMAGE_SUBSYSTEM_EFI_ROM" },
	{ IMAGE_SUBSYSTEM_XBOX, "IMAGE_SUBSYSTEM_XBOX" },
	{ IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION, "IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION" },
	{ IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG, "IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG" }
	};

	std::map<WORD, std::wstring> mapDllCharacteristics {
		{ 0x0001, L"IMAGE_LIBRARY_PROCESS_INIT" },
	{ 0x0002, L"IMAGE_LIBRARY_PROCESS_TERM" },
	{ 0x0004, L"IMAGE_LIBRARY_THREAD_INIT" },
	{ 0x0008, L"IMAGE_LIBRARY_THREAD_TERM" },
	{ IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA, L"IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA" },
	{ IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE, L"IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE" },
	{ IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY, L"IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY" },
	{ IMAGE_DLLCHARACTERISTICS_NX_COMPAT, L"IMAGE_DLLCHARACTERISTICS_NX_COMPAT" },
	{ IMAGE_DLLCHARACTERISTICS_NO_ISOLATION, L"IMAGE_DLLCHARACTERISTICS_NO_ISOLATION" },
	{ IMAGE_DLLCHARACTERISTICS_NO_SEH, L"IMAGE_DLLCHARACTERISTICS_NO_SEH" },
	{ IMAGE_DLLCHARACTERISTICS_NO_BIND, L"IMAGE_DLLCHARACTERISTICS_NO_BIND" },
	{ IMAGE_DLLCHARACTERISTICS_APPCONTAINER, L"IMAGE_DLLCHARACTERISTICS_APPCONTAINER" },
	{ IMAGE_DLLCHARACTERISTICS_WDM_DRIVER, L"IMAGE_DLLCHARACTERISTICS_WDM_DRIVER" },
	{ IMAGE_DLLCHARACTERISTICS_GUARD_CF, L"IMAGE_DLLCHARACTERISTICS_GUARD_CF" },
	{ IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE, L"IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE" }
	};

	WCHAR str[MAX_PATH] { };
	std::wstring strTmp { };
	int listindex = 0;

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		const IMAGE_OPTIONAL_HEADER32* pOptHeader32 = &std::get<0>(*pOptionalHdr);

		listindex = m_listOptHeader.InsertItem(listindex, L"Magic");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.Magic) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->Magic));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%04X", pOptHeader32->Magic);
		m_listOptHeader.SetItemText(listindex, 3, str);
		m_listOptHeader.SetItemTooltip(listindex, 3, L"IMAGE_NT_OPTIONAL_HDR32_MAGIC", L"Magic:");

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MajorLinkerVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorLinkerVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->MajorLinkerVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%02X", pOptHeader32->MajorLinkerVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MinorLinkerVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorLinkerVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->MinorLinkerVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%02X", pOptHeader32->MinorLinkerVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfCode");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfCode) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SizeOfCode));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SizeOfCode);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfInitializedData");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfInitializedData) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SizeOfInitializedData));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SizeOfInitializedData);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfUninitializedData");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfUninitializedData) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SizeOfUninitializedData));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SizeOfUninitializedData);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"AddressOfEntryPoint");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.AddressOfEntryPoint) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->AddressOfEntryPoint));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->AddressOfEntryPoint);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"BaseOfCode");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.BaseOfCode) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->BaseOfCode));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->BaseOfCode);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"BaseOfData");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.BaseOfData) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->BaseOfData));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->BaseOfData);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"ImageBase");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.ImageBase) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->ImageBase));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->ImageBase);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SectionAlignment");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SectionAlignment) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SectionAlignment));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SectionAlignment);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"FileAlignment");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.FileAlignment) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->FileAlignment));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->FileAlignment);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MajorOperatingSystemVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorOperatingSystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->MajorOperatingSystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader32->MajorOperatingSystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MinorOperatingSystemVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorOperatingSystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->MinorOperatingSystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader32->MinorOperatingSystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MajorImageVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorImageVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->MajorImageVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader32->MajorImageVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MinorImageVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorImageVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->MinorImageVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader32->MinorImageVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MajorSubsystemVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorSubsystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->MajorSubsystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader32->MajorSubsystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MinorSubsystemVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorSubsystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->MinorSubsystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader32->MinorSubsystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"Win32VersionValue");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.Win32VersionValue) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->Win32VersionValue));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->Win32VersionValue);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfImage");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfImage) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SizeOfImage));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SizeOfImage);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfHeaders");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfHeaders) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SizeOfHeaders));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SizeOfHeaders);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"CheckSum");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.CheckSum) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->CheckSum));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->CheckSum);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"Subsystem");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.Subsystem) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->Subsystem));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader32->Subsystem);
		m_listOptHeader.SetItemText(listindex, 3, str);
		if (mapSubSystem.find(pOptHeader32->Subsystem) != mapSubSystem.end())
		{
			swprintf_s(str, MAX_PATH, L"%S", mapSubSystem.at(pOptHeader32->Subsystem).c_str());
			m_listOptHeader.SetItemTooltip(listindex, 3, str, L"Subsystem:");
		}

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"DllCharacteristics");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DllCharacteristics) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->DllCharacteristics));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%04X", pOptHeader32->DllCharacteristics);
		m_listOptHeader.SetItemText(listindex, 3, str);

		strTmp.clear();
		for (auto& i : mapDllCharacteristics)
		{
			if (i.first & pOptHeader32->DllCharacteristics)
				strTmp += i.second + L"\n";
		}
		if (!strTmp.empty())
		{
			strTmp.erase(strTmp.size() - 1);//to remove last '\n'
			m_listOptHeader.SetItemTooltip(listindex, 3, strTmp, L"DllCharacteristics:");
		}

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfStackReserve");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfStackReserve) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SizeOfStackReserve));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SizeOfStackReserve);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfStackCommit");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfStackCommit) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SizeOfStackCommit));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SizeOfStackCommit);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfHeapReserve");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfHeapReserve) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SizeOfHeapReserve));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SizeOfHeapReserve);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfHeapCommit");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfHeapCommit) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->SizeOfHeapCommit));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->SizeOfHeapCommit);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"LoaderFlags");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.LoaderFlags) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->LoaderFlags));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->LoaderFlags);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"NumberOfRvaAndSizes");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.NumberOfRvaAndSizes) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader32->NumberOfRvaAndSizes));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader32->NumberOfRvaAndSizes);
		m_listOptHeader.SetItemText(listindex, 3, str);
	}
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
	{
		const IMAGE_OPTIONAL_HEADER64* pOptHeader64 = &std::get<1>(*pOptionalHdr);

		listindex = m_listOptHeader.InsertItem(listindex, L"Magic");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.Magic) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->Magic));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%04X", pOptHeader64->Magic);
		m_listOptHeader.SetItemText(listindex, 3, str);
		m_listOptHeader.SetItemTooltip(listindex, 3, L"IMAGE_NT_OPTIONAL_HDR64_MAGIC", L"Magic:");

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MajorLinkerVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorLinkerVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->MajorLinkerVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%02X", pOptHeader64->MajorLinkerVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MinorLinkerVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorLinkerVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->MinorLinkerVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%02X", pOptHeader64->MinorLinkerVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfCode");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfCode) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SizeOfCode));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->SizeOfCode);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfInitializedData");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfInitializedData) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SizeOfInitializedData));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->SizeOfInitializedData);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfUninitializedData");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfUninitializedData) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SizeOfUninitializedData));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->SizeOfUninitializedData);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"AddressOfEntryPoint");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.AddressOfEntryPoint) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->AddressOfEntryPoint));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->AddressOfEntryPoint);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"BaseOfCode");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.BaseOfCode) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->BaseOfCode));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->BaseOfCode);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"ImageBase");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.ImageBase) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->ImageBase));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", pOptHeader64->ImageBase);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SectionAlignment");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SectionAlignment) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SectionAlignment));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->SectionAlignment);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"FileAlignment");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.FileAlignment) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->FileAlignment));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->FileAlignment);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MajorOperatingSystemVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorOperatingSystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->MajorOperatingSystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader64->MajorOperatingSystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MinorOperatingSystemVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorOperatingSystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->MinorOperatingSystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader64->MinorOperatingSystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MajorImageVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorImageVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->MajorImageVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader64->MajorImageVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MinorImageVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorImageVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->MinorImageVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader64->MinorImageVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MajorSubsystemVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorSubsystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->MajorSubsystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader64->MajorSubsystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"MinorSubsystemVersion");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorSubsystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->MinorSubsystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader64->MinorSubsystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"Win32VersionValue");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.Win32VersionValue) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->Win32VersionValue));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->Win32VersionValue);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfImage");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfImage) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SizeOfImage));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->SizeOfImage);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfHeaders");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeaders) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SizeOfHeaders));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->SizeOfHeaders);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"CheckSum");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.CheckSum) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->CheckSum));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->CheckSum);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"Subsystem");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.Subsystem) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->Subsystem));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%04X", pOptHeader64->Subsystem);
		m_listOptHeader.SetItemText(listindex, 3, str);
		if (mapSubSystem.find(pOptHeader64->Subsystem) != mapSubSystem.end())
		{
			swprintf_s(str, MAX_PATH, L"%S", mapSubSystem.at(pOptHeader64->Subsystem).c_str());
			m_listOptHeader.SetItemTooltip(listindex, 3, str, L"Subsystem:");
		}

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"DllCharacteristics");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DllCharacteristics) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->DllCharacteristics));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%04X", pOptHeader64->DllCharacteristics);
		m_listOptHeader.SetItemText(listindex, 3, str);

		strTmp.clear();
		for (auto& i : mapDllCharacteristics)
		{
			if (i.first & pOptHeader64->DllCharacteristics)
				strTmp += i.second + L"\n";
		}
		if (!strTmp.empty())
		{
			strTmp.erase(strTmp.size() - 1);//to remove last '\n'
			m_listOptHeader.SetItemTooltip(listindex, 3, strTmp, L"DllCharacteristics:");
		}

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfStackReserve");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfStackReserve) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SizeOfStackReserve));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", pOptHeader64->SizeOfStackReserve);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfStackCommit");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfStackCommit) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SizeOfStackCommit));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", pOptHeader64->SizeOfStackCommit);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfHeapReserve");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeapReserve) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SizeOfHeapReserve));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", pOptHeader64->SizeOfHeapReserve);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"SizeOfHeapCommit");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeapCommit) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->SizeOfHeapCommit));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", pOptHeader64->SizeOfHeapCommit);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"LoaderFlags");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.LoaderFlags) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->LoaderFlags));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->LoaderFlags);
		m_listOptHeader.SetItemText(listindex, 3, str);

		listindex = m_listOptHeader.InsertItem(listindex + 1, L"NumberOfRvaAndSizes");
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.NumberOfRvaAndSizes) + m_dwPeStart);
		m_listOptHeader.SetItemText(listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(pOptHeader64->NumberOfRvaAndSizes));
		m_listOptHeader.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pOptHeader64->NumberOfRvaAndSizes);
		m_listOptHeader.SetItemText(listindex, 3, str);
	}

	return 0;
}

int CViewRightTL::CreateListDataDirectories()
{
	PCLIBPE_DATADIRS_VEC pLibPeDataDirs { };
	if (m_pLibpe->GetDataDirectories(pLibPeDataDirs) != S_OK)
		return -1;

	m_listDataDirs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_DATADIRECTORIES, &m_stListInfo);
	m_listDataDirs.ShowWindow(SW_HIDE);
	m_listDataDirs.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDataDirs.InsertColumn(0, L"Name", LVCFMT_CENTER, 200);
	m_listDataDirs.InsertColumn(1, L"Offset", LVCFMT_LEFT, 100);
	m_listDataDirs.InsertColumn(2, L"Directory RVA", LVCFMT_LEFT, 100);
	m_listDataDirs.InsertColumn(3, L"Directory Size", LVCFMT_LEFT, 100);
	m_listDataDirs.InsertColumn(4, L"Resides in Section", LVCFMT_LEFT, 125);

	WCHAR str[9] { };
	UINT listindex = 0;
	DWORD dwDataDirsOffset { };

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
		dwDataDirsOffset = m_dwPeStart + offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
		dwDataDirsOffset = m_dwPeStart + offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);

	const IMAGE_DATA_DIRECTORY* pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXPORT));

	listindex = m_listDataDirs.InsertItem(listindex, L"Export Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXPORT)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_IMPORT));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Import Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_IMPORT)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_RESOURCE));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Resource Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_RESOURCE)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXCEPTION));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Exception Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXCEPTION)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_SECURITY));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Security Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	if (pDataDirs->VirtualAddress)
		m_listDataDirs.SetItemTooltip(listindex, 2, L"This address is a file raw offset on disk.");
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_SECURITY)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_BASERELOC));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Relocation Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_BASERELOC)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_DEBUG));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Debug Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_DEBUG)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_ARCHITECTURE));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Architecture Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_ARCHITECTURE)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_GLOBALPTR));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Global PTR");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_GLOBALPTR)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_TLS));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"TLS Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_TLS)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Load Config Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Bound Import Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_IAT));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"IAT Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_IAT)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"Delay Import Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	pDataDirs = &std::get<0>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR));

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	listindex = m_listDataDirs.InsertItem(listindex + 1, L"COM Descriptor Directory");
	swprintf_s(str, 9, L"%08X", dwDataDirsOffset);
	m_listDataDirs.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)).c_str());
	m_listDataDirs.SetItemText(listindex, 4, str);

	return 0;
}

int CViewRightTL::CreateListSecHeaders()
{
	if (!m_pSecHeaders)
		return -1;

	m_listSecHeaders.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDATA | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_SECHEADERS, &m_stListInfo);
	m_listSecHeaders.ShowWindow(SW_HIDE);
	m_listSecHeaders.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listSecHeaders.InsertColumn(0, L"Name", LVCFMT_CENTER, 150);
	m_listSecHeaders.InsertColumn(1, L"Virtual Size", LVCFMT_LEFT, 100);
	m_listSecHeaders.InsertColumn(2, L"Virtual Address", LVCFMT_LEFT, 125);
	m_listSecHeaders.InsertColumn(3, L"SizeOfRawData", LVCFMT_LEFT, 125);
	m_listSecHeaders.InsertColumn(4, L"PointerToRawData", LVCFMT_LEFT, 125);
	m_listSecHeaders.InsertColumn(5, L"PointerToRelocations", LVCFMT_LEFT, 150);
	m_listSecHeaders.InsertColumn(6, L"PointerToLinenumbers", LVCFMT_LEFT, 160);
	m_listSecHeaders.InsertColumn(7, L"NumberOfRelocations", LVCFMT_LEFT, 150);
	m_listSecHeaders.InsertColumn(8, L"NumberOfLinenumbers", LVCFMT_LEFT, 160);
	m_listSecHeaders.InsertColumn(9, L"Characteristics", LVCFMT_LEFT, 115);
	m_listSecHeaders.SetItemCountEx(m_pSecHeaders->size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);

	std::map<DWORD, std::wstring> mapSecFlags = {
		{ 0x00000000, L"IMAGE_SCN_TYPE_REG\n Reserved." },
	{ 0x00000001, L"IMAGE_SCN_TYPE_DSECT\n Reserved." },
	{ 0x00000002, L"IMAGE_SCN_TYPE_NOLOAD\n Reserved." },
	{ 0x00000004, L"IMAGE_SCN_TYPE_GROUP\n Reserved." },
	{ IMAGE_SCN_TYPE_NO_PAD, L"IMAGE_SCN_TYPE_NO_PAD\n Reserved." },
	{ 0x00000010, L"IMAGE_SCN_TYPE_COPY\n Reserved." },
	{ IMAGE_SCN_CNT_CODE, L"IMAGE_SCN_CNT_CODE\n Section contains code." },
	{ IMAGE_SCN_CNT_INITIALIZED_DATA, L"IMAGE_SCN_CNT_INITIALIZED_DATA\n Section contains initialized data." },
	{ IMAGE_SCN_CNT_UNINITIALIZED_DATA, L"IMAGE_SCN_CNT_UNINITIALIZED_DATA\n Section contains uninitialized data." },
	{ IMAGE_SCN_LNK_OTHER, L"IMAGE_SCN_LNK_OTHER\n Reserved." },
	{ IMAGE_SCN_LNK_INFO, L"IMAGE_SCN_LNK_INFO\n Section contains comments or some other type of information." },
	{ 0x00000400, L"IMAGE_SCN_TYPE_OVER\n Reserved." },
	{ IMAGE_SCN_LNK_REMOVE, L"IMAGE_SCN_LNK_REMOVE\n Section contents will not become part of image." },
	{ IMAGE_SCN_LNK_COMDAT, L"IMAGE_SCN_LNK_COMDAT\n Section contents comdat." },
	{ IMAGE_SCN_NO_DEFER_SPEC_EXC, L"IMAGE_SCN_NO_DEFER_SPEC_EXC\n Reset speculative exceptions handling bits in the TLB entries for this section." },
	{ IMAGE_SCN_GPREL, L"IMAGE_SCN_GPREL\n Section content can be accessed relative to GP" },
	{ 0x00010000, L"IMAGE_SCN_MEM_SYSHEAP\n Obsolete" },
	{ IMAGE_SCN_MEM_PURGEABLE, L"IMAGE_SCN_MEM_PURGEABLE" },
	{ IMAGE_SCN_MEM_LOCKED, L"IMAGE_SCN_MEM_LOCKED" },
	{ IMAGE_SCN_MEM_PRELOAD, L"IMAGE_SCN_MEM_PRELOAD" },
	{ IMAGE_SCN_ALIGN_1BYTES, L"IMAGE_SCN_ALIGN_1BYTES" },
	{ IMAGE_SCN_ALIGN_2BYTES, L"IMAGE_SCN_ALIGN_2BYTES" },
	{ IMAGE_SCN_ALIGN_4BYTES, L"IMAGE_SCN_ALIGN_4BYTES" },
	{ IMAGE_SCN_ALIGN_8BYTES, L"IMAGE_SCN_ALIGN_8BYTES" },
	{ IMAGE_SCN_ALIGN_16BYTES, L"IMAGE_SCN_ALIGN_16BYTES\n Default alignment if no others are specified." },
	{ IMAGE_SCN_ALIGN_32BYTES, L"IMAGE_SCN_ALIGN_32BYTES" },
	{ IMAGE_SCN_ALIGN_64BYTES, L"IMAGE_SCN_ALIGN_64BYTES" },
	{ IMAGE_SCN_ALIGN_128BYTES, L"IMAGE_SCN_ALIGN_128BYTES" },
	{ IMAGE_SCN_ALIGN_256BYTES, L"IMAGE_SCN_ALIGN_256BYTES" },
	{ IMAGE_SCN_ALIGN_512BYTES, L"IMAGE_SCN_ALIGN_512BYTES" },
	{ IMAGE_SCN_ALIGN_1024BYTES, L"IMAGE_SCN_ALIGN_1024BYTES" },
	{ IMAGE_SCN_ALIGN_2048BYTES, L"IMAGE_SCN_ALIGN_2048BYTES" },
	{ IMAGE_SCN_ALIGN_4096BYTES, L"IMAGE_SCN_ALIGN_4096BYTES" },
	{ IMAGE_SCN_ALIGN_8192BYTES, L"IMAGE_SCN_ALIGN_8192BYTES" },
	{ IMAGE_SCN_ALIGN_MASK, L"IMAGE_SCN_ALIGN_MASK" },
	{ IMAGE_SCN_LNK_NRELOC_OVFL, L"IMAGE_SCN_LNK_NRELOC_OVFL\n Section contains extended relocations." },
	{ IMAGE_SCN_MEM_DISCARDABLE, L"IMAGE_SCN_MEM_DISCARDABLE\n Section can be discarded." },
	{ IMAGE_SCN_MEM_NOT_CACHED, L"IMAGE_SCN_MEM_NOT_CACHED\n Section is not cachable." },
	{ IMAGE_SCN_MEM_NOT_PAGED, L"IMAGE_SCN_MEM_NOT_PAGED\n Section is not pageable." },
	{ IMAGE_SCN_MEM_SHARED, L"IMAGE_SCN_MEM_SHARED\n Section is shareable." },
	{ IMAGE_SCN_MEM_EXECUTE, L"IMAGE_SCN_MEM_EXECUTE\n Section is executable." },
	{ IMAGE_SCN_MEM_READ, L"IMAGE_SCN_MEM_READ\n Section is readable." },
	{ IMAGE_SCN_MEM_WRITE, L"IMAGE_SCN_MEM_WRITE\n Section is writeable." }
	};

	UINT listindex = 0;
	std::wstring strTipText { };

	for (auto& iterSecHdrs : *m_pSecHeaders)
	{
		for (auto& flags : mapSecFlags)
			if (flags.first & std::get<0>(iterSecHdrs).Characteristics)
				strTipText += flags.second + L"\n";

		if (!strTipText.empty())
		{
			m_listSecHeaders.SetItemTooltip(listindex, 9, strTipText, L"Section Flags:");
			strTipText.clear();
		}

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListExport()
{
	PCLIBPE_EXPORT_TUP pExportTable { };
	if (m_pLibpe->GetExportTable(pExportTable) != S_OK)
		return -1;

	WCHAR str[MAX_PATH] { };
	int listindex = 0;
	const IMAGE_EXPORT_DIRECTORY* pExportDir = &std::get<0>(*pExportTable);

	m_listExportDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_EXPORT, &m_stListInfo);
	m_listExportDir.ShowWindow(SW_HIDE);
	m_listExportDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listExportDir.InsertColumn(0, L"Name", LVCFMT_CENTER, 250);
	m_listExportDir.InsertColumn(1, L"Size [BYTES]", LVCFMT_LEFT, 100);
	m_listExportDir.InsertColumn(2, L"Value", LVCFMT_LEFT, 300);

	m_listExportDir.InsertItem(listindex, L"Export flags (Characteristics)");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->Characteristics));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pExportDir->Characteristics);
	m_listExportDir.SetItemText(listindex, 2, str);

	listindex = m_listExportDir.InsertItem(listindex + 1, L"Time/Date Stamp");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->TimeDateStamp));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, MAX_PATH, L"%08X", pExportDir->TimeDateStamp);
	m_listExportDir.SetItemText(listindex, 2, str);
	if (pExportDir->TimeDateStamp)
	{
		__time64_t time = pExportDir->TimeDateStamp;
		_wctime64_s(str, MAX_PATH, &time);
		m_listExportDir.SetItemTooltip(listindex, 2, str, L"Time / Date:");
	}

	listindex = m_listExportDir.InsertItem(listindex + 1, L"MajorVersion");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->MajorVersion));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, 5, L"%04X", pExportDir->MajorVersion);
	m_listExportDir.SetItemText(listindex, 2, str);

	listindex = m_listExportDir.InsertItem(listindex + 1, L"MinorVersion");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->MinorVersion));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, 5, L"%04X", pExportDir->MinorVersion);
	m_listExportDir.SetItemText(listindex, 2, str);

	listindex = m_listExportDir.InsertItem(listindex + 1, L"Name RVA");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->Name));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, MAX_PATH, L"%08X (%S)", pExportDir->Name, std::get<1>(*pExportTable).c_str());
	m_listExportDir.SetItemText(listindex, 2, str);

	listindex = m_listExportDir.InsertItem(listindex + 1, L"Base (OrdinalBase)");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->Base));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pExportDir->Base);
	m_listExportDir.SetItemText(listindex, 2, str);

	listindex = m_listExportDir.InsertItem(listindex + 1, L"NumberOfFunctions");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->NumberOfFunctions));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pExportDir->NumberOfFunctions);
	m_listExportDir.SetItemText(listindex, 2, str);

	listindex = m_listExportDir.InsertItem(listindex + 1, L"NumberOfNames");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->NumberOfNames));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pExportDir->NumberOfNames);
	m_listExportDir.SetItemText(listindex, 2, str);

	listindex = m_listExportDir.InsertItem(listindex + 1, L"AddressOfFunctions");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->AddressOfFunctions));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pExportDir->AddressOfFunctions);
	m_listExportDir.SetItemText(listindex, 2, str);

	listindex = m_listExportDir.InsertItem(listindex + 1, L"AddressOfNames");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->AddressOfNames));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pExportDir->AddressOfNames);
	m_listExportDir.SetItemText(listindex, 2, str);

	listindex = m_listExportDir.InsertItem(listindex + 1, L"AddressOfNameOrdinals");
	swprintf_s(str, 2, L"%X", sizeof(pExportDir->AddressOfNameOrdinals));
	m_listExportDir.SetItemText(listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pExportDir->AddressOfNameOrdinals);
	m_listExportDir.SetItemText(listindex, 2, str);

	return 0;
}

int CViewRightTL::CreateListImport()
{
	if (!m_pImportTable)
		return -1;

	m_listImport.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDATA | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_IMPORT, &m_stListInfo);
	m_listImport.ShowWindow(SW_HIDE);
	m_listImport.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listImport.InsertColumn(0, L"Module Name (funcs number)", LVCFMT_CENTER, 330);
	m_listImport.InsertColumn(1, L"OriginalFirstThunk\n(Import Lookup Table)", LVCFMT_LEFT, 170);
	m_listImport.InsertColumn(2, L"TimeDateStamp", LVCFMT_LEFT, 115);
	m_listImport.InsertColumn(3, L"ForwarderChain", LVCFMT_LEFT, 110);
	m_listImport.InsertColumn(4, L"Name RVA", LVCFMT_LEFT, 90);
	m_listImport.InsertColumn(5, L"FirstThunk (IAT)", LVCFMT_LEFT, 135);
	m_listImport.SetItemCountEx(m_pImportTable->size(), LVSICF_NOSCROLL);

	WCHAR str[MAX_PATH] { };
	int listindex = 0;

	for (auto& i : *m_pImportTable)
	{
		const IMAGE_IMPORT_DESCRIPTOR* pImportDesc = &std::get<0>(i);
		if (pImportDesc->TimeDateStamp)
		{
			__time64_t time = pImportDesc->TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &time);
			m_listImport.SetItemTooltip(listindex, 2, str, L"Time / Date:");
		}
		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateTreeResources()
{
	PCLIBPE_RESOURCE_ROOT_TUP pTupResRoot { };

	if (m_pLibpe->GetResourceTable(pTupResRoot) != S_OK)
		return -1;

	m_treeResTop.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CRect(0, 0, 0, 0), this, IDC_TREE_RESOURCE_TOP);
	m_treeResTop.ShowWindow(SW_HIDE);

	m_hTreeResDir = m_treeResTop.InsertItem(L"Resources tree.");

	WCHAR str[MAX_PATH] { };
	HTREEITEM treeRoot { }, treeLvL2 { };
	long ilvlRoot = 0, ilvl2 = 0, ilvl3 = 0;

	//Main loop to extract Resources from tuple.
	for (auto& iterRoot : std::get<1>(*pTupResRoot))
	{
		const IMAGE_RESOURCE_DIRECTORY_ENTRY* pResDirEntry = &std::get<0>(iterRoot);
		if (pResDirEntry->DataIsDirectory)
		{
			if (pResDirEntry->NameIsString)
				swprintf(str, MAX_PATH, L"Entry: %i, Name: %s", ilvlRoot, std::get<1>(iterRoot).c_str());
			else
			{
				if (g_mapResType.find(pResDirEntry->Id) != g_mapResType.end())
					swprintf(str, MAX_PATH, L"Entry: %i, Id: %u, %s", ilvlRoot, pResDirEntry->Id, g_mapResType.at(pResDirEntry->Id).c_str());
				else
					swprintf(str, MAX_PATH, L"Entry: %i, Id: %u", ilvlRoot, pResDirEntry->Id);
			}
			treeRoot = m_treeResTop.InsertItem(str, m_hTreeResDir);
			m_vecResId.emplace_back(ilvlRoot, -1, -1);
			m_treeResTop.SetItemData(treeRoot, m_vecResId.size() - 1);
			ilvl2 = 0;

			const PCLIBPE_RESOURCE_LVL2_TUP pTupResLvL2 = &std::get<4>(iterRoot);
			for (auto& iterLvL2 : std::get<1>(*pTupResLvL2))
			{
				pResDirEntry = &std::get<0>(iterLvL2);
				if (pResDirEntry->DataIsDirectory)
				{
					if (pResDirEntry->NameIsString)
						swprintf(str, MAX_PATH, L"Entry: %i, Name: %s", ilvl2, std::get<1>(iterLvL2).c_str());
					else
						swprintf(str, MAX_PATH, L"Entry: %i, Id: %u", ilvl2, pResDirEntry->Id);

					treeLvL2 = m_treeResTop.InsertItem(str, treeRoot);
					m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
					m_treeResTop.SetItemData(treeLvL2, m_vecResId.size() - 1);
					ilvl3 = 0;

					const PCLIBPE_RESOURCE_LVL3_TUP pTupResLvL3 = &std::get<4>(iterLvL2);
					for (auto& iterLvL3 : std::get<1>(*pTupResLvL3))
					{
						pResDirEntry = &std::get<0>(iterLvL3);

						if (pResDirEntry->NameIsString)
							swprintf(str, MAX_PATH, L"Entry: %i, Name: %s", ilvl3, std::get<1>(iterLvL3).c_str());
						else
							swprintf(str, MAX_PATH, L"Entry: %i, lang: %u", ilvl3, pResDirEntry->Id);

						const HTREEITEM treeLvL3 = m_treeResTop.InsertItem(str, treeLvL2);
						m_vecResId.emplace_back(ilvlRoot, ilvl2, ilvl3);
						m_treeResTop.SetItemData(treeLvL3, m_vecResId.size() - 1);

						ilvl3++;
					}
				}
				else
				{	//DATA lvl2
					pResDirEntry = &std::get<0>(iterLvL2);

					if (pResDirEntry->NameIsString)
						swprintf(str, MAX_PATH, L"Entry: %i, Name: %s", ilvl2, std::get<1>(iterLvL2).c_str());
					else
						swprintf(str, MAX_PATH, L"Entry: %i, lang: %u", ilvl2, pResDirEntry->Id);

					treeLvL2 = m_treeResTop.InsertItem(str, treeRoot);
					m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
					m_treeResTop.SetItemData(treeLvL2, m_vecResId.size() - 1);
				}
				ilvl2++;
			}
		}
		else
		{	//DATA lvlroot
			pResDirEntry = &std::get<0>(iterRoot);

			if (pResDirEntry->NameIsString)
				swprintf(str, MAX_PATH, L"Entry: %i, Name: %s", ilvlRoot, std::get<1>(iterRoot).c_str());
			else
				swprintf(str, MAX_PATH, L"Entry: %i, lang: %u", ilvlRoot, pResDirEntry->Id);

			treeRoot = m_treeResTop.InsertItem(str, m_hTreeResDir);
			m_vecResId.emplace_back(ilvlRoot, -1, -1);
			m_treeResTop.SetItemData(treeRoot, m_vecResId.size() - 1);
		}
		ilvlRoot++;
	}
	m_treeResTop.Expand(m_hTreeResDir, TVE_EXPAND);

	return 0;
}

int CViewRightTL::CreateListException()
{
	if (!m_pExceptionDir)
		return -1;

	m_listExceptionDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDATA | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_EXCEPTION, &m_stListInfo);
	m_listExceptionDir.ShowWindow(SW_HIDE);
	m_listExceptionDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listExceptionDir.InsertColumn(0, L"BeginAddress", LVCFMT_CENTER, 100);
	m_listExceptionDir.InsertColumn(1, L"EndAddress", LVCFMT_LEFT, 100);
	m_listExceptionDir.InsertColumn(2, L"UnwindData/InfoAddress", LVCFMT_LEFT, 180);
	m_listExceptionDir.SetItemCountEx(m_pExceptionDir->size(), LVSICF_NOSCROLL);

	return 0;
}

int CViewRightTL::CreateListSecurity()
{
	PCLIBPE_SECURITY_VEC pSecurityDir { };
	if (m_pLibpe->GetSecurityTable(pSecurityDir) != S_OK)
		return -1;

	m_listSecurityDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_SECURITY, &m_stListInfo);
	m_listSecurityDir.ShowWindow(SW_HIDE);
	m_listSecurityDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listSecurityDir.InsertColumn(0, L"dwLength", LVCFMT_CENTER, 100);
	m_listSecurityDir.InsertColumn(1, L"wRevision", LVCFMT_LEFT, 100);
	m_listSecurityDir.InsertColumn(2, L"wCertificateType", LVCFMT_LEFT, 180);

	int listindex = 0;
	TCHAR str[9] { };
	for (auto& i : *pSecurityDir)
	{
		WIN_CERTIFICATE pSert = std::get<0>(i);
		swprintf_s(str, 9, L"%08X", pSert.dwLength);
		m_listSecurityDir.InsertItem(listindex, str);
		swprintf_s(str, 5, L"%04X", pSert.wRevision);
		m_listSecurityDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 5, L"%04X", pSert.wCertificateType);
		m_listSecurityDir.SetItemText(listindex, 2, str);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListRelocation()
{
	if (!m_pRelocTable)
		return -1;

	m_listRelocDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDATA | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_RELOCATIONS, &m_stListInfo);
	m_listRelocDir.ShowWindow(SW_HIDE);
	m_listRelocDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listRelocDir.InsertColumn(0, L"Virtual Address", LVCFMT_CENTER, 115);
	m_listRelocDir.InsertColumn(1, L"Block Size", LVCFMT_LEFT, 100);
	m_listRelocDir.InsertColumn(2, L"Entries", LVCFMT_LEFT, 100);
	m_listRelocDir.SetItemCountEx(m_pRelocTable->size(), LVSICF_NOSCROLL);

	return 0;
}

int CViewRightTL::CreateListDebug()
{
	PCLIBPE_DEBUG_VEC pDebugDir { };

	if (m_pLibpe->GetDebugTable(pDebugDir) != S_OK)
		return -1;

	m_listDebugDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_DEBUG, &m_stListInfo);
	m_listDebugDir.ShowWindow(SW_HIDE);
	m_listDebugDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDebugDir.InsertColumn(0, L"Characteristics", LVCFMT_CENTER, 115);
	m_listDebugDir.InsertColumn(1, L"TimeDateStamp", LVCFMT_LEFT, 150);
	m_listDebugDir.InsertColumn(2, L"MajorVersion", LVCFMT_LEFT, 100);
	m_listDebugDir.InsertColumn(3, L"MinorVersion", LVCFMT_LEFT, 100);
	m_listDebugDir.InsertColumn(4, L"Type", LVCFMT_LEFT, 90);
	m_listDebugDir.InsertColumn(5, L"SizeOfData", LVCFMT_LEFT, 100);
	m_listDebugDir.InsertColumn(6, L"AddressOfRawData", LVCFMT_LEFT, 170);
	m_listDebugDir.InsertColumn(7, L"PointerToRawData", LVCFMT_LEFT, 140);

	std::map<DWORD, std::wstring> mapDebugType {
		{ IMAGE_DEBUG_TYPE_UNKNOWN, L"IMAGE_DEBUG_TYPE_UNKNOWN" },
	{ IMAGE_DEBUG_TYPE_COFF, L"IMAGE_DEBUG_TYPE_COFF" },
	{ IMAGE_DEBUG_TYPE_CODEVIEW, L"IMAGE_DEBUG_TYPE_CODEVIEW" },
	{ IMAGE_DEBUG_TYPE_FPO, L"IMAGE_DEBUG_TYPE_FPO" },
	{ IMAGE_DEBUG_TYPE_MISC, L"IMAGE_DEBUG_TYPE_MISC" },
	{ IMAGE_DEBUG_TYPE_EXCEPTION, L"IMAGE_DEBUG_TYPE_EXCEPTION" },
	{ IMAGE_DEBUG_TYPE_FIXUP, L"IMAGE_DEBUG_TYPE_FIXUP" },
	{ IMAGE_DEBUG_TYPE_OMAP_TO_SRC, L"IMAGE_DEBUG_TYPE_OMAP_TO_SRC" },
	{ IMAGE_DEBUG_TYPE_OMAP_FROM_SRC, L"IMAGE_DEBUG_TYPE_OMAP_FROM_SRC" },
	{ IMAGE_DEBUG_TYPE_BORLAND, L"IMAGE_DEBUG_TYPE_BORLAND" },
	{ IMAGE_DEBUG_TYPE_RESERVED10, L"IMAGE_DEBUG_TYPE_RESERVED10" },
	{ IMAGE_DEBUG_TYPE_CLSID, L"IMAGE_DEBUG_TYPE_CLSID" },
	{ IMAGE_DEBUG_TYPE_VC_FEATURE, L"IMAGE_DEBUG_TYPE_VC_FEATURE" },
	{ IMAGE_DEBUG_TYPE_POGO, L"IMAGE_DEBUG_TYPE_POGO" },
	{ IMAGE_DEBUG_TYPE_ILTCG, L"IMAGE_DEBUG_TYPE_ILTCG" },
	{ IMAGE_DEBUG_TYPE_MPX, L"IMAGE_DEBUG_TYPE_MPX" },
	{ IMAGE_DEBUG_TYPE_REPRO, L"IMAGE_DEBUG_TYPE_REPRO" }
	};

	int listindex = 0;
	TCHAR str[MAX_PATH] { };

	for (auto& i : *pDebugDir)
	{
		const IMAGE_DEBUG_DIRECTORY* pDebug = &std::get<0>(i);
		swprintf_s(str, 9, L"%08X", std::get<0>(i).Characteristics);
		m_listDebugDir.InsertItem(listindex, str);
		swprintf_s(str, MAX_PATH, L"%08X", pDebug->TimeDateStamp);
		m_listDebugDir.SetItemText(listindex, 1, str);
		if (pDebug->TimeDateStamp)
		{
			__time64_t _time = pDebug->TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &_time);
			m_listDebugDir.SetItemTooltip(listindex, 1, str, L"Time / Date:");
		}
		swprintf_s(str, 5, L"%04u", pDebug->MajorVersion);
		m_listDebugDir.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%04u", pDebug->MinorVersion);
		m_listDebugDir.SetItemText(listindex, 3, str);
		swprintf_s(str, 9, L"%08X", pDebug->Type);
		m_listDebugDir.SetItemText(listindex, 4, str);
		for (auto&j : mapDebugType)
			if (j.first == pDebug->Type)
				m_listDebugDir.SetItemTooltip(listindex, 4, j.second);
		swprintf_s(str, 9, L"%08X", pDebug->SizeOfData);
		m_listDebugDir.SetItemText(listindex, 5, str);
		swprintf_s(str, 9, L"%08X", pDebug->AddressOfRawData);
		m_listDebugDir.SetItemText(listindex, 6, str);
		swprintf_s(str, 9, L"%08X", pDebug->PointerToRawData);
		m_listDebugDir.SetItemText(listindex, 7, str);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListTLS()
{
	PCLIBPE_TLS_TUP pTLSDir { };
	if (m_pLibpe->GetTLSTable(pTLSDir) != S_OK)
		return -1;

	m_listTLSDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_TLS, &m_stListInfo);
	m_listTLSDir.ShowWindow(SW_HIDE);
	m_listTLSDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listTLSDir.InsertColumn(0, L"Name", LVCFMT_CENTER, 250);
	m_listTLSDir.InsertColumn(1, L"Size [BYTES]", LVCFMT_LEFT, 110);
	m_listTLSDir.InsertColumn(2, L"Value", LVCFMT_LEFT, 150);

	std::map<DWORD, std::wstring> mapCharact {
		{ IMAGE_SCN_ALIGN_1BYTES, L"IMAGE_SCN_ALIGN_1BYTES" },
	{ IMAGE_SCN_ALIGN_2BYTES, L"IMAGE_SCN_ALIGN_2BYTES" },
	{ IMAGE_SCN_ALIGN_4BYTES, L"IMAGE_SCN_ALIGN_4BYTES" },
	{ IMAGE_SCN_ALIGN_8BYTES, L"IMAGE_SCN_ALIGN_8BYTES" },
	{ IMAGE_SCN_ALIGN_16BYTES, L"IMAGE_SCN_ALIGN_16BYTES" },
	{ IMAGE_SCN_ALIGN_32BYTES, L"IMAGE_SCN_ALIGN_32BYTES" },
	{ IMAGE_SCN_ALIGN_64BYTES, L"IMAGE_SCN_ALIGN_64BYTES" },
	{ IMAGE_SCN_ALIGN_128BYTES, L"IMAGE_SCN_ALIGN_128BYTES" },
	{ IMAGE_SCN_ALIGN_256BYTES, L"IMAGE_SCN_ALIGN_256BYTES" },
	{ IMAGE_SCN_ALIGN_512BYTES, L"IMAGE_SCN_ALIGN_512BYTES" },
	{ IMAGE_SCN_ALIGN_1024BYTES, L"IMAGE_SCN_ALIGN_1024BYTES" },
	{ IMAGE_SCN_ALIGN_2048BYTES, L"IMAGE_SCN_ALIGN_2048BYTES" },
	{ IMAGE_SCN_ALIGN_4096BYTES, L"IMAGE_SCN_ALIGN_4096BYTES" },
	{ IMAGE_SCN_ALIGN_8192BYTES, L"IMAGE_SCN_ALIGN_8192BYTES" },
	{ IMAGE_SCN_ALIGN_MASK, L"IMAGE_SCN_ALIGN_MASK" }
	};

	int listindex = 0;
	TCHAR str[MAX_PATH] { };

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		const IMAGE_TLS_DIRECTORY32*  pTLSDir32 = &std::get<IMAGE_TLS_DIRECTORY32>(std::get<0>(*pTLSDir));

		listindex = m_listTLSDir.InsertItem(listindex, L"StartAddressOfRawData");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir32->StartAddressOfRawData));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pTLSDir32->StartAddressOfRawData);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"EndAddressOfRawData");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir32->EndAddressOfRawData));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pTLSDir32->EndAddressOfRawData);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"AddressOfIndex");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir32->AddressOfIndex));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pTLSDir32->AddressOfIndex);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"AddressOfCallBacks");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir32->AddressOfCallBacks));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pTLSDir32->AddressOfCallBacks);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"SizeOfZeroFill");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir32->SizeOfZeroFill));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pTLSDir32->SizeOfZeroFill);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"Characteristics");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir32->Characteristics));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pTLSDir32->Characteristics);
		m_listTLSDir.SetItemText(listindex, 2, str);

		auto iterCharact = mapCharact.find(pTLSDir32->Characteristics);
		if (iterCharact != mapCharact.end())
			m_listTLSDir.SetItemTooltip(listindex, 2, iterCharact->second, L"Characteristics:");
	}
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
	{
		const IMAGE_TLS_DIRECTORY64* pTLSDir64 = &std::get<IMAGE_TLS_DIRECTORY64>(std::get<0>(*pTLSDir));

		listindex = m_listTLSDir.InsertItem(listindex, L"StartAddressOfRawData");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir64->StartAddressOfRawData));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pTLSDir64->StartAddressOfRawData);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"EndAddressOfRawData");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir64->EndAddressOfRawData));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pTLSDir64->EndAddressOfRawData);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"AddressOfIndex");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir64->AddressOfIndex));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pTLSDir64->AddressOfIndex);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"AddressOfCallBacks");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir64->AddressOfCallBacks));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pTLSDir64->AddressOfCallBacks);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"SizeOfZeroFill");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir64->SizeOfZeroFill));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pTLSDir64->SizeOfZeroFill);
		m_listTLSDir.SetItemText(listindex, 2, str);

		listindex = m_listTLSDir.InsertItem(listindex + 1, L"Characteristics");
		swprintf_s(str, 3, L"%u", sizeof(pTLSDir64->Characteristics));
		m_listTLSDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pTLSDir64->Characteristics);
		m_listTLSDir.SetItemText(listindex, 2, str);

		auto iterCharact = mapCharact.find(pTLSDir64->Characteristics);
		if (iterCharact != mapCharact.end())
			m_listTLSDir.SetItemTooltip(listindex, 2, iterCharact->second, L"Characteristics:");
	}

	return 0;
}

int CViewRightTL::CreateListLoadConfigTable()
{
	PCLIBPE_LOADCONFIGTABLE_VAR pLCD { };
	if (m_pLibpe->GetLoadConfigTable(pLCD) != S_OK)
		return -1;

	PCLIBPE_DATADIRS_VEC pDataDirs { };
	if (m_pLibpe->GetDataDirectories(pDataDirs) != S_OK)
		return -1;

	m_listLoadConfigDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_LOADCONFIG, &m_stListInfo);
	m_listLoadConfigDir.ShowWindow(SW_HIDE);
	m_listLoadConfigDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listLoadConfigDir.InsertColumn(0, L"Name", LVCFMT_CENTER, 330);
	m_listLoadConfigDir.InsertColumn(1, L"Size [BYTES]", LVCFMT_LEFT, 110);
	m_listLoadConfigDir.InsertColumn(2, L"Value", LVCFMT_LEFT, 300);

	std::map<WORD, std::wstring> mapGuardFlags {
		{ IMAGE_GUARD_CF_INSTRUMENTED, L"IMAGE_GUARD_CF_INSTRUMENTED\n Module performs control flow integrity checks using system-supplied support" },
	{ IMAGE_GUARD_CFW_INSTRUMENTED, L"IMAGE_GUARD_CFW_INSTRUMENTED\n Module performs control flow and write integrity checks" },
	{ IMAGE_GUARD_CF_FUNCTION_TABLE_PRESENT, L"IMAGE_GUARD_CF_FUNCTION_TABLE_PRESENT\n Module contains valid control flow target metadata" },
	{ IMAGE_GUARD_SECURITY_COOKIE_UNUSED, L"IMAGE_GUARD_SECURITY_COOKIE_UNUSED\n Module does not make use of the /GS security cookie" },
	{ IMAGE_GUARD_PROTECT_DELAYLOAD_IAT, L"IMAGE_GUARD_PROTECT_DELAYLOAD_IAT\n Module supports read only delay load IAT" },
	{ IMAGE_GUARD_DELAYLOAD_IAT_IN_ITS_OWN_SECTION, L"IMAGE_GUARD_DELAYLOAD_IAT_IN_ITS_OWN_SECTION\n Delayload import table in its own .didat section (with nothing else in it) that can be freely reprotected" },
	{ IMAGE_GUARD_CF_EXPORT_SUPPRESSION_INFO_PRESENT, L"IMAGE_GUARD_CF_EXPORT_SUPPRESSION_INFO_PRESENT\n Module contains suppressed export information. This also infers that the address taken IAT table is also present in the load config." },
	{ IMAGE_GUARD_CF_ENABLE_EXPORT_SUPPRESSION, L"IMAGE_GUARD_CF_ENABLE_EXPORT_SUPPRESSION\n Module enables suppression of exports" },
	{ IMAGE_GUARD_CF_LONGJUMP_TABLE_PRESENT, L"IMAGE_GUARD_CF_LONGJUMP_TABLE_PRESENT\n Module contains longjmp target information" },
	{ IMAGE_GUARD_RF_INSTRUMENTED, L"IMAGE_GUARD_RF_INSTRUMENTED\n Module contains return flow instrumentation and metadata" },
	{ IMAGE_GUARD_RF_ENABLE, L"IMAGE_GUARD_RF_ENABLE\n Module requests that the OS enable return flow protection" },
	{ IMAGE_GUARD_RF_STRICT, L"IMAGE_GUARD_RF_STRICT\n Module requests that the OS enable return flow protection in strict mode" },
	{ IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_MASK, L"IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_MASK\n Stride of Guard CF function table encoded in these bits (additional count of bytes per element)" },
	{ IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_SHIFT, L"IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_SHIFT\n Shift to right-justify Guard CF function table stride" }
	};

	int listindex = 0;
	TCHAR str[MAX_PATH] { };
	std::wstring _tooltip { };
	DWORD dwLCTSize = std::get<0>(pDataDirs->at(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG)).Size;
	DWORD dwTotalSize { };

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		const IMAGE_LOAD_CONFIG_DIRECTORY32* pLoadConfDir32 = &std::get<0>(*pLCD);

		listindex = m_listLoadConfigDir.InsertItem(listindex, L"Size");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->Size));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->Size);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"TimeDateStamp");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->TimeDateStamp));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, MAX_PATH, L"%08X", pLoadConfDir32->TimeDateStamp);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);
		if (pLoadConfDir32->TimeDateStamp)
		{
			__time64_t _time = pLoadConfDir32->TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &_time);
			m_listLoadConfigDir.SetItemTooltip(listindex, 2, str, L"Time / Date:");
		}

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"MajorVersion");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->MajorVersion));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 5, L"%04X", pLoadConfDir32->MajorVersion);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"MinorVersion");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->MinorVersion));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 5, L"%04X", pLoadConfDir32->MinorVersion);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GlobalFlagsClear");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GlobalFlagsClear));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GlobalFlagsClear);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GlobalFlagsSet");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GlobalFlagsSet));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GlobalFlagsSet);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CriticalSectionDefaultTimeout");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->CriticalSectionDefaultTimeout));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->CriticalSectionDefaultTimeout);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DeCommitFreeBlockThreshold");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->DeCommitFreeBlockThreshold));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->DeCommitFreeBlockThreshold);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DeCommitTotalFreeThreshold");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->DeCommitTotalFreeThreshold));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->DeCommitTotalFreeThreshold);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"LockPrefixTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->LockPrefixTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->LockPrefixTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"MaximumAllocationSize");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->MaximumAllocationSize));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->MaximumAllocationSize);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"VirtualMemoryThreshold");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->VirtualMemoryThreshold));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->VirtualMemoryThreshold);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"ProcessHeapFlags");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->ProcessHeapFlags));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->ProcessHeapFlags);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"ProcessAffinityMask");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->ProcessAffinityMask));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->ProcessAffinityMask);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CSDVersion");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->CSDVersion));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 5, L"%04X", pLoadConfDir32->CSDVersion);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DependentLoadFlags");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->DependentLoadFlags));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 5, L"%04X", pLoadConfDir32->DependentLoadFlags);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"EditList");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->EditList));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->EditList);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"SecurityCookie");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->SecurityCookie));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->SecurityCookie);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"SEHandlerTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->SEHandlerTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->SEHandlerTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"SEHandlerCount");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->SEHandlerCount));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->SEHandlerCount);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardCFCheckFunctionPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardCFCheckFunctionPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardCFCheckFunctionPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardCFDispatchFunctionPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardCFDispatchFunctionPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardCFDispatchFunctionPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardCFFunctionTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardCFFunctionTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardCFFunctionTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardCFFunctionCount");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardCFFunctionCount));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardCFFunctionCount);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardFlags");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardFlags));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardFlags);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);
		_tooltip.clear();
		for (auto&i : mapGuardFlags)
			if (i.first & pLoadConfDir32->GuardFlags)
				_tooltip += i.second + L"\n";
		if (!_tooltip.empty())
			m_listLoadConfigDir.SetItemTooltip(listindex, 2, _tooltip, L"GuardFlags:");

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CodeIntegrity.Flags");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->CodeIntegrity.Flags));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%04X", pLoadConfDir32->CodeIntegrity.Flags);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CodeIntegrity.Catalog");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->CodeIntegrity.Catalog));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%04X", pLoadConfDir32->CodeIntegrity.Catalog);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CodeIntegrity.CatalogOffset");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->CodeIntegrity.CatalogOffset));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->CodeIntegrity.CatalogOffset);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CodeIntegrity.Reserved");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->CodeIntegrity.Reserved));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->CodeIntegrity.Reserved);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardAddressTakenIatEntryTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardAddressTakenIatEntryTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardAddressTakenIatEntryTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardAddressTakenIatEntryCount");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardAddressTakenIatEntryCount));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardAddressTakenIatEntryCount);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardLongJumpTargetTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardLongJumpTargetTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardLongJumpTargetTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardLongJumpTargetCount");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardLongJumpTargetCount));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardLongJumpTargetCount);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DynamicValueRelocTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->DynamicValueRelocTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->DynamicValueRelocTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CHPEMetadataPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->CHPEMetadataPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->CHPEMetadataPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardRFFailureRoutine");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardRFFailureRoutine));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardRFFailureRoutine);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardRFFailureRoutineFunctionPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardRFFailureRoutineFunctionPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardRFFailureRoutineFunctionPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DynamicValueRelocTableOffset");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->DynamicValueRelocTableOffset));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->DynamicValueRelocTableOffset);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DynamicValueRelocTableSection");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->DynamicValueRelocTableSection));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%04X", pLoadConfDir32->DynamicValueRelocTableSection);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"Reserved2");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->Reserved2));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%04X", pLoadConfDir32->Reserved2);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardRFVerifyStackPointerFunctionPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->GuardRFVerifyStackPointerFunctionPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->GuardRFVerifyStackPointerFunctionPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"HotPatchTableOffset");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->HotPatchTableOffset));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->HotPatchTableOffset);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"Reserved3");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->Reserved3));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->Reserved3);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"EnclaveConfigurationPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir32->EnclaveConfigurationPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir32->EnclaveConfigurationPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);
	}
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
	{
		const IMAGE_LOAD_CONFIG_DIRECTORY64* pLoadConfDir64 = &std::get<1>(*pLCD);

		dwTotalSize += sizeof(pLoadConfDir64->Size);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex, L"Size");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->Size));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->Size);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->TimeDateStamp);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"TimeDateStamp");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->TimeDateStamp));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, MAX_PATH, L"%08X", pLoadConfDir64->TimeDateStamp);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);
		if (pLoadConfDir64->TimeDateStamp)
		{
			__time64_t _time = pLoadConfDir64->TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &_time);
			m_listLoadConfigDir.SetItemTooltip(listindex, 2, str, L"Time / Date:");
		}

		dwTotalSize += sizeof(pLoadConfDir64->MajorVersion);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"MajorVersion");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->MajorVersion));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 5, L"%04X", pLoadConfDir64->MajorVersion);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->MinorVersion);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"MinorVersion");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->MinorVersion));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 5, L"%04X", pLoadConfDir64->MinorVersion);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GlobalFlagsClear);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GlobalFlagsClear");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GlobalFlagsClear));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->GlobalFlagsClear);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GlobalFlagsSet);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GlobalFlagsSet");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GlobalFlagsSet));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->GlobalFlagsSet);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->CriticalSectionDefaultTimeout);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CriticalSectionDefaultTimeout");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->CriticalSectionDefaultTimeout));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->CriticalSectionDefaultTimeout);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->DeCommitFreeBlockThreshold);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DeCommitFreeBlockThreshold");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->DeCommitFreeBlockThreshold));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->DeCommitFreeBlockThreshold);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->DeCommitTotalFreeThreshold);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DeCommitTotalFreeThreshold");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->DeCommitTotalFreeThreshold));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->DeCommitTotalFreeThreshold);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->LockPrefixTable);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"LockPrefixTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->LockPrefixTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->LockPrefixTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->MaximumAllocationSize);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"MaximumAllocationSize");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->MaximumAllocationSize));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->MaximumAllocationSize);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->VirtualMemoryThreshold);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"VirtualMemoryThreshold");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->VirtualMemoryThreshold));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->VirtualMemoryThreshold);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->ProcessHeapFlags);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"ProcessHeapFlags");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->ProcessHeapFlags));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->ProcessHeapFlags);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->ProcessAffinityMask);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"ProcessAffinityMask");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->ProcessAffinityMask));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->ProcessAffinityMask);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->CSDVersion);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CSDVersion");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->CSDVersion));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 5, L"%04X", pLoadConfDir64->CSDVersion);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->DependentLoadFlags);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DependentLoadFlags");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->DependentLoadFlags));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 5, L"%04X", pLoadConfDir64->DependentLoadFlags);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->EditList);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"EditList");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->EditList));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->EditList);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->SecurityCookie);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"SecurityCookie");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->SecurityCookie));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->SecurityCookie);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->SEHandlerTable);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"SEHandlerTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->SEHandlerTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->SEHandlerTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->SEHandlerCount);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"SEHandlerCount");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->SEHandlerCount));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->SEHandlerCount);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardCFCheckFunctionPointer);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardCFCheckFunctionPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardCFCheckFunctionPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardCFCheckFunctionPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardCFDispatchFunctionPointer);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardCFDispatchFunctionPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardCFDispatchFunctionPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardCFDispatchFunctionPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardCFFunctionTable);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardCFFunctionTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardCFFunctionTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardCFFunctionTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardCFFunctionCount);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardCFFunctionCount");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardCFFunctionCount));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardCFFunctionCount);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardFlags);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardFlags");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardFlags));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->GuardFlags);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);
		_tooltip.clear();
		for (auto&i : mapGuardFlags)
			if (i.first & pLoadConfDir64->GuardFlags)
				_tooltip += i.second + L"\n";
		if (!_tooltip.empty())
			m_listLoadConfigDir.SetItemTooltip(listindex, 2, _tooltip, L"GuardFlags:");

		dwTotalSize += sizeof(pLoadConfDir64->CodeIntegrity.Flags);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CodeIntegrity.Flags");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->CodeIntegrity.Flags));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%04X", pLoadConfDir64->CodeIntegrity.Flags);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->CodeIntegrity.Catalog);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CodeIntegrity.Catalog");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->CodeIntegrity.Catalog));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%04X", pLoadConfDir64->CodeIntegrity.Catalog);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->CodeIntegrity.CatalogOffset);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CodeIntegrity.CatalogOffset");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->CodeIntegrity.CatalogOffset));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->CodeIntegrity.CatalogOffset);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->CodeIntegrity.Reserved);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CodeIntegrity.Reserved");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->CodeIntegrity.Reserved));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->CodeIntegrity.Reserved);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardAddressTakenIatEntryTable);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardAddressTakenIatEntryTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardAddressTakenIatEntryTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardAddressTakenIatEntryTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardAddressTakenIatEntryCount);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardAddressTakenIatEntryCount");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardAddressTakenIatEntryCount));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardAddressTakenIatEntryCount);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardLongJumpTargetTable);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardLongJumpTargetTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardLongJumpTargetTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardLongJumpTargetTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardLongJumpTargetCount);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardLongJumpTargetCount");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardLongJumpTargetCount));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardLongJumpTargetCount);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->DynamicValueRelocTable);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DynamicValueRelocTable");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->DynamicValueRelocTable));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->DynamicValueRelocTable);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->CHPEMetadataPointer);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"CHPEMetadataPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->CHPEMetadataPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->CHPEMetadataPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardRFFailureRoutine);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardRFFailureRoutine");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardRFFailureRoutine));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardRFFailureRoutine);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardRFFailureRoutineFunctionPointer);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardRFFailureRoutineFunctionPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardRFFailureRoutineFunctionPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardRFFailureRoutineFunctionPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->DynamicValueRelocTableOffset);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DynamicValueRelocTableOffset");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->DynamicValueRelocTableOffset));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->DynamicValueRelocTableOffset);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->DynamicValueRelocTableSection);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"DynamicValueRelocTableSection");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->DynamicValueRelocTableSection));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%04X", pLoadConfDir64->DynamicValueRelocTableSection);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->Reserved2);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"Reserved2");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->Reserved2));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%04X", pLoadConfDir64->Reserved2);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->GuardRFVerifyStackPointerFunctionPointer);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"GuardRFVerifyStackPointerFunctionPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->GuardRFVerifyStackPointerFunctionPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->GuardRFVerifyStackPointerFunctionPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->HotPatchTableOffset);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"HotPatchTableOffset");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->HotPatchTableOffset));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->HotPatchTableOffset);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->Reserved3);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"Reserved3");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->Reserved3));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pLoadConfDir64->Reserved3);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);

		dwTotalSize += sizeof(pLoadConfDir64->EnclaveConfigurationPointer);
		if (dwTotalSize > dwLCTSize)
			return 0;
		listindex = m_listLoadConfigDir.InsertItem(listindex + 1, L"EnclaveConfigurationPointer");
		swprintf_s(str, 3, L"%u", sizeof(pLoadConfDir64->EnclaveConfigurationPointer));
		m_listLoadConfigDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", pLoadConfDir64->EnclaveConfigurationPointer);
		m_listLoadConfigDir.SetItemText(listindex, 2, str);
	}

	return 0;
}

int CViewRightTL::CreateListBoundImport()
{
	PCLIBPE_BOUNDIMPORT_VEC pBoundImport { };

	if (m_pLibpe->GetBoundImportTable(pBoundImport) != S_OK)
		return -1;

	m_listBoundImportDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_DELAYIMPORT, &m_stListInfo);
	m_listBoundImportDir.ShowWindow(SW_HIDE);
	m_listBoundImportDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listBoundImportDir.InsertColumn(0, L"Module Name", LVCFMT_CENTER, 290);
	m_listBoundImportDir.InsertColumn(1, L"TimeDateStamp", LVCFMT_LEFT, 130);
	m_listBoundImportDir.InsertColumn(2, L"OffsetModuleName", LVCFMT_LEFT, 140);
	m_listBoundImportDir.InsertColumn(3, L"NumberOfModuleForwarderRefs", LVCFMT_LEFT, 220);

	WCHAR str[MAX_PATH] { };
	int listindex = 0;

	for (auto& i : *pBoundImport)
	{
		const IMAGE_BOUND_IMPORT_DESCRIPTOR* pBoundImpDir = &std::get<0>(i);
		swprintf_s(str, MAX_PATH, L"%S", std::get<1>(i).c_str());
		m_listBoundImportDir.InsertItem(listindex, str);
		swprintf_s(str, MAX_PATH, L"%08X", pBoundImpDir->TimeDateStamp);
		m_listBoundImportDir.SetItemText(listindex, 1, str);
		if (pBoundImpDir->TimeDateStamp)
		{
			__time64_t _time = pBoundImpDir->TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &_time);
			m_listBoundImportDir.SetItemTooltip(listindex, 1, str, L"Time / Date:");
		}
		swprintf_s(str, 5, L"%04X", pBoundImpDir->OffsetModuleName);
		m_listBoundImportDir.SetItemText(listindex, 2, str);
		swprintf_s(str, 5, L"%04u", pBoundImpDir->NumberOfModuleForwarderRefs);
		m_listBoundImportDir.SetItemText(listindex, 3, str);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListDelayImport()
{
	PCLIBPE_DELAYIMPORT_VEC pDelayImport { };

	if (m_pLibpe->GetDelayImportTable(pDelayImport) != S_OK)
		return -1;

	m_listDelayImportDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_DELAYIMPORT, &m_stListInfo);
	m_listDelayImportDir.ShowWindow(SW_HIDE);
	m_listDelayImportDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDelayImportDir.InsertColumn(0, L"Module Name (funcs number)", LVCFMT_CENTER, 260);
	m_listDelayImportDir.InsertColumn(1, L"Attributes", LVCFMT_LEFT, 100);
	m_listDelayImportDir.InsertColumn(2, L"DllNameRVA", LVCFMT_LEFT, 105);
	m_listDelayImportDir.InsertColumn(3, L"ModuleHandleRVA", LVCFMT_LEFT, 140);
	m_listDelayImportDir.InsertColumn(4, L"ImportAddressTableRVA", LVCFMT_LEFT, 160);
	m_listDelayImportDir.InsertColumn(5, L"ImportNameTableRVA", LVCFMT_LEFT, 150);
	m_listDelayImportDir.InsertColumn(6, L"BoundImportAddressTableRVA", LVCFMT_LEFT, 200);
	m_listDelayImportDir.InsertColumn(7, L"UnloadInformationTableRVA", LVCFMT_LEFT, 190);
	m_listDelayImportDir.InsertColumn(8, L"TimeDateStamp", LVCFMT_LEFT, 115);

	int listindex = 0;
	WCHAR str[MAX_PATH] { };

	for (auto& i : *pDelayImport)
	{
		const IMAGE_DELAYLOAD_DESCRIPTOR* pDelayImpDir = &std::get<0>(i);
		swprintf_s(str, MAX_PATH, L"%S (%u)", std::get<1>(i).c_str(), std::get<2>(i).size());
		m_listDelayImportDir.InsertItem(listindex, str);
		swprintf_s(str, 9, L"%08X", pDelayImpDir->Attributes.AllAttributes);
		m_listDelayImportDir.SetItemText(listindex, 1, str);
		swprintf_s(str, 9, L"%08X", pDelayImpDir->DllNameRVA);
		m_listDelayImportDir.SetItemText(listindex, 2, str);
		swprintf_s(str, 9, L"%08X", pDelayImpDir->ModuleHandleRVA);
		m_listDelayImportDir.SetItemText(listindex, 3, str);
		swprintf_s(str, 9, L"%08X", pDelayImpDir->ImportAddressTableRVA);
		m_listDelayImportDir.SetItemText(listindex, 4, str);
		swprintf_s(str, 9, L"%08X", pDelayImpDir->ImportNameTableRVA);
		m_listDelayImportDir.SetItemText(listindex, 5, str);
		swprintf_s(str, 9, L"%08X", pDelayImpDir->BoundImportAddressTableRVA);
		m_listDelayImportDir.SetItemText(listindex, 6, str);
		swprintf_s(str, 9, L"%08X", pDelayImpDir->UnloadInformationTableRVA);
		m_listDelayImportDir.SetItemText(listindex, 7, str);
		swprintf_s(str, 9, L"%08X", pDelayImpDir->TimeDateStamp);
		m_listDelayImportDir.SetItemText(listindex, 8, str);
		if (pDelayImpDir->TimeDateStamp)
		{
			__time64_t time = pDelayImpDir->TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &time);
			m_listDelayImportDir.SetItemTooltip(listindex, 8, str, L"Time / Date:");
		}

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListCOM()
{
	PCLIBPE_COMDESCRIPTOR pCOMDesc { };

	if (m_pLibpe->GetCOMDescriptorTable(pCOMDesc) != S_OK)
		return -1;

	m_listCOMDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_DELAYIMPORT, &m_stListInfo);
	m_listCOMDir.ShowWindow(SW_HIDE);
	m_listCOMDir.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listCOMDir.InsertColumn(0, L"Name", LVCFMT_CENTER, 300);
	m_listCOMDir.InsertColumn(1, L"Value", LVCFMT_LEFT, 100);

	std::map<DWORD, std::wstring> mapFlags {
		{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_ILONLY, L"COMIMAGE_FLAGS_ILONLY" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_32BITREQUIRED, L"COMIMAGE_FLAGS_32BITREQUIRED" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_IL_LIBRARY, L"COMIMAGE_FLAGS_IL_LIBRARY" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_STRONGNAMESIGNED, L"COMIMAGE_FLAGS_STRONGNAMESIGNED" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_NATIVE_ENTRYPOINT, L"COMIMAGE_FLAGS_NATIVE_ENTRYPOINT" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_TRACKDEBUGDATA, L"COMIMAGE_FLAGS_TRACKDEBUGDATA" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_32BITPREFERRED, L"COMIMAGE_FLAGS_32BITPREFERRED" }
	};

	int listindex = 0;
	WCHAR str[MAX_PATH] { };
	std::wstring strToolTip { };

	m_listCOMDir.InsertItem(listindex, L"cb");
	swprintf_s(str, 9, L"%08X", pCOMDesc->cb);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"MajorRuntimeVersion");
	swprintf_s(str, 5, L"%04X", pCOMDesc->MajorRuntimeVersion);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"MinorRuntimeVersion");
	swprintf_s(str, 5, L"%04X", pCOMDesc->MinorRuntimeVersion);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"MetaData.RVA");
	swprintf_s(str, 9, L"%08X", pCOMDesc->MetaData.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"MetaData.Size");
	swprintf_s(str, 9, L"%08X", pCOMDesc->MetaData.Size);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"Flags");
	swprintf_s(str, 9, L"%08X", pCOMDesc->Flags);
	m_listCOMDir.SetItemText(listindex, 1, str);
	for (auto&i : mapFlags)
		if (i.first & pCOMDesc->Flags)
			strToolTip += i.second + L"\n";
	if (!strToolTip.empty())
		m_listCOMDir.SetItemTooltip(listindex, 1, strToolTip, L"Flags:");

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"EntryPointToken");
	swprintf_s(str, 9, L"%08X", pCOMDesc->EntryPointToken);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"Resources.RVA");
	swprintf_s(str, 9, L"%08X", pCOMDesc->Resources.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"Resources.Size");
	swprintf_s(str, 9, L"%08X", pCOMDesc->Resources.Size);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"StrongNameSignature.RVA");
	swprintf_s(str, 9, L"%08X", pCOMDesc->StrongNameSignature.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"StrongNameSignature.Size");
	swprintf_s(str, 9, L"%08X", pCOMDesc->StrongNameSignature.Size);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"CodeManagerTable.RVA");
	swprintf_s(str, 9, L"%08X", pCOMDesc->CodeManagerTable.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"CodeManagerTable.Size");
	swprintf_s(str, 9, L"%08X", pCOMDesc->CodeManagerTable.Size);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"VTableFixups.RVA");
	swprintf_s(str, 9, L"%08X", pCOMDesc->VTableFixups.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"VTableFixups.Size");
	swprintf_s(str, 9, L"%08X", pCOMDesc->VTableFixups.Size);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"ExportAddressTableJumps.RVA");
	swprintf_s(str, 9, L"%08X", pCOMDesc->ExportAddressTableJumps.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"ExportAddressTableJumps.Size");
	swprintf_s(str, 9, L"%08X", pCOMDesc->ExportAddressTableJumps.Size);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"ManagedNativeHeader.RVA");
	swprintf_s(str, 9, L"%08X", pCOMDesc->ManagedNativeHeader.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 1, str);

	listindex = m_listCOMDir.InsertItem(listindex + 1, L"ManagedNativeHeader.Size");
	swprintf_s(str, 9, L"%08X", pCOMDesc->ManagedNativeHeader.Size);
	m_listCOMDir.SetItemText(listindex, 1, str);

	return 0;
}