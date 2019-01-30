#include "stdafx.h"
#include "ViewRightTL.h"
#include "version.h"

IMPLEMENT_DYNCREATE(CViewRightTL, CView)

BEGIN_MESSAGE_MAP(CViewRightTL, CView)
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_NOTIFY(LVN_GETDISPINFOW, IDC_LIST_SECHEADERS, &CViewRightTL::OnListSectionsGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFOW, IDC_LIST_IMPORT, &CViewRightTL::OnListImportGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFOW, IDC_LIST_RELOCATIONS, &CViewRightTL::OnListRelocsGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFOW, IDC_LIST_EXCEPTION, &CViewRightTL::OnListExceptionsGetDispInfo)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CViewRightTL::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();
	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	m_pFileLoader = &m_pMainDoc->m_stFileLoader;

	LOGFONT lf { };
	StringCchCopyW(lf.lfFaceName, 18, L"Consolas");
	lf.lfHeight = 22;
	if (!m_fontSummary.CreateFontIndirectW(&lf))
	{
		StringCchCopyW(lf.lfFaceName, 18, L"Times New Roman");
		m_fontSummary.CreateFontIndirectW(&lf);
	}

	if (m_pLibpe->GetImageFlags(m_dwFileSummary) != S_OK)
		return;

	m_wstrFullPath = L"Full path: " + m_pMainDoc->GetPathName();
	m_wstrFileName = m_pMainDoc->GetPathName();
	m_wstrFileName.erase(0, m_wstrFileName.find_last_of('\\') + 1);
	m_wstrFileName.insert(0, L"File name: ");

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
		m_wstrFileType = L"File type: PE32 (x86)";
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
		m_wstrFileType = L"File type: PE32+ (x64)";
	else
		m_wstrFileType = L"File type: unknown";

	WCHAR wstrVersion[MAX_PATH];
	swprintf_s(wstrVersion, MAX_PATH, L"%S, version: %u.%u.%u", PRODUCT_NAME, MAJOR_VERSION, MINOR_VERSION, MAINTENANCE_VERSION);
	m_wstrAppVersion = wstrVersion;

	m_pLibpe->GetSectionsHeaders(m_pSecHeaders);
	m_pLibpe->GetImport(m_pImport);
	m_pLibpe->GetExceptions(m_pExceptionDir);
	m_pLibpe->GetRelocations(m_pRelocTable);

	m_stListInfo.clrTooltipText = RGB(255, 255, 255);
	m_stListInfo.clrTooltipBk = RGB(0, 132, 132);
	m_stListInfo.clrHeaderText = RGB(255, 255, 255);
	m_stListInfo.clrHeaderBk = RGB(0, 132, 132);
	m_stListInfo.dwHeaderHeight = 39;

	m_lf.lfHeight = 16;
	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stListInfo.pListLogFont = &m_lf;
	m_hdrlf.lfHeight = 17;
	m_hdrlf.lfWeight = FW_BOLD;
	StringCchCopyW(m_hdrlf.lfFaceName, 16, L"Times New Roman");
	m_stListInfo.pHeaderLogFont = &m_hdrlf;

	m_menuList.CreatePopupMenu();
	m_menuList.AppendMenuW(MF_STRING, IDM_LIST_GOTODESCOFFSET, L"Go to descriptor offset...");
	m_menuList.AppendMenuW(MF_STRING, IDM_LIST_GOTODATAOFFSET, L"Go to data offset...");

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
	CreateListExceptions();
	CreateListSecurity();
	CreateListRelocations();
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
	if (LOWORD(lHint) == IDC_SHOW_RESOURCE_RBR)
		return;

	if (m_pActiveWnd)
		m_pActiveWnd->ShowWindow(SW_HIDE);

	m_fFileSummaryShow = false;

	CRect rcClient, rc;
	::GetClientRect(AfxGetMainWnd()->m_hWnd, &rcClient);
	GetClientRect(&rc);

	switch (LOWORD(lHint))
	{
	case IDC_SHOW_FILE_SUMMARY:
		m_fFileSummaryShow = true;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_DOSHEADER:
		m_listDOSHeader.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listDOSHeader;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_RICHHEADER:
		m_listRichHdr.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listRichHdr;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_NTHEADER:
		m_listNTHeader.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listNTHeader;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_FILEHEADER:
		m_listFileHeader.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listFileHeader;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_OPTIONALHEADER:
		m_listOptHeader.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listOptHeader;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_DATADIRECTORIES:
		m_listDataDirs.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listDataDirs;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_SECHEADERS:
		m_listSecHeaders.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listSecHeaders;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_EXPORT:
		m_listExportDir.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listExportDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_IAT:
	case IDC_LIST_IMPORT:
		m_listImport.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listImport;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_TREE_RESOURCE:
		m_treeResTop.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_treeResTop;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_EXCEPTION:
		m_listExceptionDir.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listExceptionDir;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_SECURITY:
		m_listSecurityDir.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listSecurityDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_RELOCATIONS:
		m_listRelocDir.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listRelocDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_DEBUG:
		m_listDebugDir.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listDebugDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_TLS:
		m_listTLSDir.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listTLSDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_LOADCONFIG:
		m_listLCD.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listLCD;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_BOUNDIMPORT:
		m_listBoundImportDir.SetWindowPos(this, 0, 0, rc.Width(), rc.Height() / 2, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listBoundImportDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_DELAYIMPORT:
		m_listDelayImportDir.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listDelayImportDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_COMDESCRIPTOR:
		m_listCOMDir.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listCOMDir;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
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
		CMemDC memDC(*pDC, this);
		CDC& rDC = memDC.GetDC();

		CRect rc;
		rDC.GetClipBox(rc);
		rDC.FillSolidRect(rc, RGB(255, 255, 255));
		rDC.SelectObject(m_fontSummary);
		GetTextExtentPoint32W(rDC.m_hDC, m_wstrFullPath.data(), m_wstrFullPath.length(), &m_sizeTextToDraw);
		rc.SetRect(20, 20, 400, m_sizeTextToDraw.cy * 6);
		if (m_sizeTextToDraw.cx > rc.Width())
			rc.right = m_sizeTextToDraw.cx + rc.left + 50;
		rDC.Rectangle(&rc);

		rDC.SetTextColor(RGB(200, 50, 30));
		GetTextExtentPoint32W(rDC.m_hDC, m_wstrAppVersion.data(), m_wstrAppVersion.length(), &m_sizeTextToDraw);
		ExtTextOutW(rDC.m_hDC, (rc.Width() - m_sizeTextToDraw.cx) / 2 + rc.left, 10, 0, nullptr,
			m_wstrAppVersion.c_str(), m_wstrAppVersion.length(), nullptr);

		rDC.SetTextColor(RGB(0, 0, 255));
		ExtTextOutW(rDC.m_hDC, 35, rc.top + m_sizeTextToDraw.cy, 0, nullptr, m_wstrFileType.data(), m_wstrFileType.length(), nullptr);
		ExtTextOutW(rDC.m_hDC, 35, rc.top + 2 * m_sizeTextToDraw.cy, 0, nullptr, m_wstrFileName.data(), m_wstrFileName.length(), nullptr);
		ExtTextOutW(rDC.m_hDC, 35, rc.top + 3 * m_sizeTextToDraw.cy, 0, nullptr, m_wstrFullPath.data(), m_wstrFullPath.length(), nullptr);
	}
}

BOOL CViewRightTL::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CViewRightTL::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveWnd)
		m_pActiveWnd->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightTL::OnListSectionsGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFOW *pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	LVITEMW* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		WCHAR wstr[50] { };
		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(wstr, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).dwOffsetSecHdrDesc);
			break;
		case 1:
		{
			auto& rstr = m_pSecHeaders->at(pItem->iItem);
			if (rstr.strSecName.empty())
				swprintf_s(wstr, 9, L"%.8S", rstr.stSecHdr.Name);
			else
				swprintf_s(wstr, 50, L"%.8S (%S)", rstr.stSecHdr.Name,
					m_pSecHeaders->at(pItem->iItem).strSecName.data());
		}
		break;
		case 2:
			swprintf_s(wstr, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.Misc.VirtualSize);
			break;
		case 3:
			swprintf_s(wstr, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.VirtualAddress);
			break;
		case 4:
			swprintf_s(wstr, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.SizeOfRawData);
			break;
		case 5:
			swprintf_s(wstr, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.PointerToRawData);
			break;
		case 6:
			swprintf_s(wstr, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.PointerToRelocations);
			break;
		case 7:
			swprintf_s(wstr, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.PointerToLinenumbers);
			break;
		case 8:
			swprintf_s(wstr, 5, L"%04X", m_pSecHeaders->at(pItem->iItem).stSecHdr.NumberOfRelocations);
			break;
		case 9:
			swprintf_s(wstr, 5, L"%04X", m_pSecHeaders->at(pItem->iItem).stSecHdr.NumberOfLinenumbers);
			break;
		case 10:
			swprintf_s(wstr, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.Characteristics);
			break;
		}
		lstrcpynW(pItem->pszText, wstr, pItem->cchTextMax);
	}

	*pResult = 0;
}

void CViewRightTL::OnListImportGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFOW *pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	LVITEMW* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		WCHAR wstr[MAX_PATH] { };
		const IMAGE_IMPORT_DESCRIPTOR* pImpDesc = &m_pImport->at(pItem->iItem).stImportDesc;

		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(wstr, 9, L"%08X", m_pImport->at(pItem->iItem).dwOffsetImpDesc);
			break;
		case 1:
			swprintf_s(wstr, MAX_PATH, L"%S (%u)", m_pImport->at(pItem->iItem).strModuleName.data(),
				m_pImport->at(pItem->iItem).vecImportFunc.size());
			break;
		case 2:
			swprintf_s(wstr, 9, L"%08X", pImpDesc->OriginalFirstThunk);
			break;
		case 3:
			swprintf_s(wstr, 9, L"%08X", pImpDesc->TimeDateStamp);
			break;
		case 4:
			swprintf_s(wstr, 9, L"%08X", pImpDesc->ForwarderChain);
			break;
		case 5:
			swprintf_s(wstr, 9, L"%08X", pImpDesc->Name);
			break;
		case 6:
			swprintf_s(wstr, 9, L"%08X", pImpDesc->FirstThunk);
			break;
		}
		lstrcpynW(pItem->pszText, wstr, pItem->cchTextMax);
	}

	*pResult = 0;
}

void CViewRightTL::OnListRelocsGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFOW *pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	LVITEMW* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		const IMAGE_BASE_RELOCATION* pReloc = &m_pRelocTable->at(pItem->iItem).stBaseReloc;
		WCHAR wstr[MAX_PATH] { };

		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(wstr, 9, L"%08X", m_pRelocTable->at(pItem->iItem).dwOffsetReloc);
			break;
		case 1:
			swprintf_s(wstr, 9, L"%08X", pReloc->VirtualAddress);
			break;
		case 2:
			swprintf_s(wstr, 9, L"%08X", pReloc->SizeOfBlock);
			break;
		case 3:
			swprintf_s(wstr, MAX_PATH, L"%u", (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD));
			break;
		}
		lstrcpynW(pItem->pszText, wstr, pItem->cchTextMax);
	}

	*pResult = 0;
}

void CViewRightTL::OnListExceptionsGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFOW *pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	LVITEMW* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		WCHAR wstr[9] { };

		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(wstr, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).dwOffsetRuntimeFuncDesc);
			break;
		case 1:
			swprintf_s(wstr, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).stRuntimeFuncEntry.BeginAddress);
			break;
		case 2:
			swprintf_s(wstr, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).stRuntimeFuncEntry.EndAddress);
			break;
		case 3:
			swprintf_s(wstr, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).stRuntimeFuncEntry.UnwindData);
			break;
		}
		lstrcpynW(pItem->pszText, wstr, pItem->cchTextMax);
	}

	*pResult = 0;
}

BOOL CViewRightTL::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	CScrollView::OnNotify(wParam, lParam, pResult);

	const LPNMITEMACTIVATE pNMI = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
	if (pNMI->iItem == -1)
		return TRUE;

	DWORD dwOffset { };
	ULONGLONG ullRVA { };
	switch (pNMI->hdr.idFrom)
	{
	case IDC_LIST_DOSHEADER:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_RICHHEADER:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_NTHEADER:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_FILEHEADER:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_OPTIONALHEADER:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_DATADIRECTORIES:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_SECHEADERS:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_EXPORT:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_IAT:
	case IDC_LIST_IMPORT:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_IMPORT_ENTRY, pNMI->iItem));
		else if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
			switch (pNMI->lParam)
			{
			case IDM_LIST_GOTODESCOFFSET:
				dwOffset = m_pImport->at(pNMI->iItem).dwOffsetImpDesc;
				break;
			case IDM_LIST_GOTODATAOFFSET:
				switch (pNMI->iSubItem)
				{
				case 1: //Str dll name
				case 5: //Name
					m_pLibpe->GetOffsetFromRVA(m_pImport->at(pNMI->iItem).stImportDesc.Name, dwOffset);
					break;
				case 2: //OriginalFirstThunk
					m_pLibpe->GetOffsetFromRVA(m_pImport->at(pNMI->iItem).stImportDesc.OriginalFirstThunk, dwOffset);
					break;
				case 3: //TimeDateStamp
					break;
				case 4: //ForwarderChain
					m_pLibpe->GetOffsetFromRVA(m_pImport->at(pNMI->iItem).stImportDesc.ForwarderChain, dwOffset);
					break;
				case 6: //FirstThunk
					m_pLibpe->GetOffsetFromRVA(m_pImport->at(pNMI->iItem).stImportDesc.FirstThunk, dwOffset);
					break;
				}
				break;
			}
		}
		break;

	case IDC_LIST_EXCEPTION:
	case IDC_LIST_DEBUG:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_DEBUG_ENTRY, pNMI->iItem));
		break;

	case IDC_LIST_ARCHITECTURE:
		break;
	case IDC_LIST_GLOBALPTR:
		break;
	case IDC_LIST_TLS:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_LOADCONFIG:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_BOUNDIMPORT:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
		break;
	case IDC_LIST_COMDESCRIPTOR:
		if (pNMI->hdr.code == LISTEX_MSG_MENUSELECTED)
		{
		}
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
			PCLIBPE_RESOURCE_ROOT pstResRoot;

			if (m_pLibpe->GetResources(pstResRoot) != S_OK)
				return -1;

			const DWORD_PTR dwResId = m_treeResTop.GetItemData(pTree->itemNew.hItem);
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

							if (!data.empty())
								//Send data vector pointer to CViewRightTR
								//to display raw data.
								m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_HEX_RIGHT_TR, 0), (CObject*)&data);
						}
					}
				}
			}
		}
	}
	break;
	}

	if (dwOffset)
		m_pFileLoader->LoadFile(m_pDocument->GetPathName(), dwOffset);

	return TRUE;
}

int CViewRightTL::CreateListDOSHeader()
{
	PCLIBPE_DOSHEADER pDosHeader { };
	if (m_pLibpe->GetMSDOSHeader(pDosHeader) != S_OK)
		return -1;

	m_listDOSHeader.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_DOSHEADER, &m_stListInfo);
	m_listDOSHeader.ShowWindow(SW_HIDE);
	m_listDOSHeader.InsertColumn(0, L"Offset", LVCFMT_LEFT, 90);
	m_listDOSHeader.SetHeaderColumnColor(0, g_clrOffset);
	m_listDOSHeader.InsertColumn(1, L"Name", LVCFMT_CENTER, 150);
	m_listDOSHeader.InsertColumn(2, L"Size [BYTES]", LVCFMT_LEFT, 100);
	m_listDOSHeader.InsertColumn(3, L"Value", LVCFMT_LEFT, 100);

	m_dwPeStart = pDosHeader->e_lfanew;

	WCHAR wstr[9];
	int listindex = 0;

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_magic));
	m_listDOSHeader.InsertItem(listindex, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_magic");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_magic));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 3, L"%02X", pDosHeader->e_magic & 0xFF);
	swprintf_s(&wstr[2], 3, L"%02X", (pDosHeader->e_magic >> 8) & 0xFF);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cblp));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_cblp");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_cblp));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_cblp);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cp));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_cp");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_cp));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_cp);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_crlc));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_crlc");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_crlc));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_crlc);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cparhdr));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_cparhdr");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_cparhdr));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_cparhdr);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_minalloc));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_minalloc");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_minalloc));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_minalloc);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_maxalloc));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_maxalloc");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_maxalloc));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_maxalloc);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_ss));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_ss");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_ss));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_ss);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_sp));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_sp");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_sp));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_sp);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_csum));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_csum");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_csum));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_csum);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_ip));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_ip");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_ip));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_ip);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cs));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_cs");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_cs));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_cs);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_lfarlc));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_lfarlc");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_lfarlc));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_lfarlc);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_ovno));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_ovno");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_ovno));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_ovno);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[0]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_res[0]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res[0]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res[0]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[1]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res[1]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res[1]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res[1]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[2]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res[2]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res[2]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res[2]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[3]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res[3]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res[3]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res[3]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_oemid));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_oemid");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_oemid));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_oemid);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_oeminfo));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_oeminfo");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_oeminfo));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_oeminfo);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[0]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_res2[0]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[0]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[0]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[1]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res2[1]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[1]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[1]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[2]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res2[2]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[2]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[2]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[3]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res2[3]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[3]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[3]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[4]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res2[4]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[4]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[4]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[5]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res2[5]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[5]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[5]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[6]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res2[6]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[6]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[6]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[7]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res2[7]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[7]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[7]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[8]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res2[8]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[8]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[8]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[9]));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"   e_res2[9]");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_res2[9]));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pDosHeader->e_res2[9]);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_lfanew));
	listindex = m_listDOSHeader.InsertItem(listindex + 1, wstr);
	m_listDOSHeader.SetItemText(listindex, 1, L"e_lfanew");
	swprintf_s(wstr, 2, L"%X", sizeof(pDosHeader->e_lfanew));
	m_listDOSHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDosHeader->e_lfanew);
	m_listDOSHeader.SetItemText(listindex, 3, wstr);

	return 0;
}

int CViewRightTL::CreateListRichHeader()
{
	PCLIBPE_RICHHEADER_VEC pRichHeader;
	if (m_pLibpe->GetRichHeader(pRichHeader) != S_OK)
		return -1;

	m_listRichHdr.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_RICHHEADER, &m_stListInfo);
	m_listRichHdr.ShowWindow(SW_HIDE);
	m_listRichHdr.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listRichHdr.SetHeaderColumnColor(0, g_clrOffset);
	m_listRichHdr.InsertColumn(1, L"\u2116", LVCFMT_CENTER, 35);
	m_listRichHdr.InsertColumn(2, L"ID [Hex]", LVCFMT_LEFT, 100);
	m_listRichHdr.InsertColumn(3, L"Version", LVCFMT_LEFT, 100);
	m_listRichHdr.InsertColumn(4, L"Occurrences", LVCFMT_LEFT, 100);

	WCHAR wstr[MAX_PATH];
	int listindex = 0;

	for (auto& i : *pRichHeader)
	{
		swprintf_s(wstr, 9, L"%08X", i.dwOffsetRich);
		m_listRichHdr.InsertItem(listindex, wstr);
		swprintf_s(wstr, MAX_PATH, L"%i", listindex + 1);
		m_listRichHdr.SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, MAX_PATH, L"%04X", i.wId);
		m_listRichHdr.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, MAX_PATH, L"%i", i.wVersion);
		m_listRichHdr.SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, MAX_PATH, L"%i", i.dwCount);
		m_listRichHdr.SetItemText(listindex, 4, wstr);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListNTHeader()
{
	PCLIBPE_NTHEADER_VAR pNTHdr { };
	if (m_pLibpe->GetNTHeader(pNTHdr) != S_OK)
		return -1;

	m_listNTHeader.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_NTHEADER, &m_stListInfo);
	m_listNTHeader.ShowWindow(SW_HIDE);
	m_listNTHeader.InsertColumn(0, L"Offset", LVCFMT_LEFT, 90);
	m_listNTHeader.SetHeaderColumnColor(0, g_clrOffset);
	m_listNTHeader.InsertColumn(1, L"Name", LVCFMT_CENTER, 100);
	m_listNTHeader.InsertColumn(2, L"Size [BYTES]", LVCFMT_LEFT, 100);
	m_listNTHeader.InsertColumn(3, L"Value", LVCFMT_LEFT, 100);

	WCHAR wstr[9];
	UINT listindex = 0;

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		const IMAGE_NT_HEADERS32* pNTHeader32 = &pNTHdr->stNTHdr32;

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, Signature) + m_dwPeStart);
		listindex = m_listNTHeader.InsertItem(listindex, wstr);
		m_listNTHeader.SetItemText(listindex, 1, L"Signature");
		swprintf_s(wstr, 2, L"%X", sizeof(pNTHeader32->Signature));
		m_listNTHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(&wstr[0], 3, L"%02X", (BYTE)(pNTHeader32->Signature >> 0));
		swprintf_s(&wstr[2], 3, L"%02X", (BYTE)(pNTHeader32->Signature >> 8));
		swprintf_s(&wstr[4], 3, L"%02X", (BYTE)(pNTHeader32->Signature >> 16));
		swprintf_s(&wstr[6], 3, L"%02X", (BYTE)(pNTHeader32->Signature >> 24));
		m_listNTHeader.SetItemText(listindex, 3, wstr);
	}
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
	{
		const IMAGE_NT_HEADERS64* pNTHeader64 = &pNTHdr->stNTHdr64;

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, Signature) + m_dwPeStart);
		listindex = m_listNTHeader.InsertItem(listindex, wstr);
		m_listNTHeader.SetItemText(listindex, 1, L"Signature");
		swprintf_s(wstr, 2, L"%X", sizeof(pNTHeader64->Signature));
		m_listNTHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(&wstr[0], 3, L"%02X", (BYTE)(pNTHeader64->Signature >> 0));
		swprintf_s(&wstr[2], 3, L"%02X", (BYTE)(pNTHeader64->Signature >> 8));
		swprintf_s(&wstr[4], 3, L"%02X", (BYTE)(pNTHeader64->Signature >> 16));
		swprintf_s(&wstr[6], 3, L"%02X", (BYTE)(pNTHeader64->Signature >> 24));
		m_listNTHeader.SetItemText(listindex, 3, wstr);
	}

	return 0;
}

int CViewRightTL::CreateListFileHeader()
{
	PCLIBPE_FILEHEADER pFileHeader { };
	if (m_pLibpe->GetFileHeader(pFileHeader) != S_OK)
		return -1;

	m_listFileHeader.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_FILEHEADER, &m_stListInfo);
	m_listFileHeader.ShowWindow(SW_HIDE);
	m_listFileHeader.InsertColumn(0, L"Offset", LVCFMT_LEFT, 90);
	m_listFileHeader.SetHeaderColumnColor(0, g_clrOffset);
	m_listFileHeader.InsertColumn(1, L"Name", LVCFMT_CENTER, 200);
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

	WCHAR wstr[MAX_PATH * 2];
	int listindex = 0;

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.Machine) + m_dwPeStart);
	listindex = m_listFileHeader.InsertItem(listindex, wstr);
	m_listFileHeader.SetItemText(listindex, 1, L"Machine");
	swprintf_s(wstr, 2, L"%X", sizeof(pFileHeader->Machine));
	m_listFileHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pFileHeader->Machine);
	m_listFileHeader.SetItemText(listindex, 3, wstr);
	auto iterMachine = mapMachineType.find(pFileHeader->Machine);
	if (iterMachine != mapMachineType.end())
		m_listFileHeader.SetCellTooltip(listindex, 3, iterMachine->second, L"Machine:");

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.NumberOfSections) + m_dwPeStart);
	listindex = m_listFileHeader.InsertItem(listindex + 1, wstr);
	m_listFileHeader.SetItemText(listindex, 1, L"NumberOfSections");
	swprintf_s(wstr, 2, L"%X", sizeof(pFileHeader->NumberOfSections));
	m_listFileHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pFileHeader->NumberOfSections);
	m_listFileHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.TimeDateStamp) + m_dwPeStart);
	listindex = m_listFileHeader.InsertItem(listindex + 1, wstr);
	m_listFileHeader.SetItemText(listindex, 1, L"TimeDateStamp");
	swprintf_s(wstr, 2, L"%X", sizeof(pFileHeader->TimeDateStamp));
	m_listFileHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, MAX_PATH, L"%08X", pFileHeader->TimeDateStamp);
	m_listFileHeader.SetItemText(listindex, 3, wstr);
	if (pFileHeader->TimeDateStamp)
	{
		__time64_t time = pFileHeader->TimeDateStamp;
		_wctime64_s(wstr, MAX_PATH, &time);
		m_listFileHeader.SetCellTooltip(listindex, 3, wstr, L"Time / Date:");
	}

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.PointerToSymbolTable) + m_dwPeStart);
	listindex = m_listFileHeader.InsertItem(listindex + 1, wstr);
	m_listFileHeader.SetItemText(listindex, 1, L"PointerToSymbolTable");
	swprintf_s(wstr, 2, L"%X", sizeof(pFileHeader->PointerToSymbolTable));
	m_listFileHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pFileHeader->PointerToSymbolTable);
	m_listFileHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.NumberOfSymbols) + m_dwPeStart);
	listindex = m_listFileHeader.InsertItem(listindex + 1, wstr);
	m_listFileHeader.SetItemText(listindex, 1, L"NumberOfSymbols");
	swprintf_s(wstr, 2, L"%X", sizeof(pFileHeader->NumberOfSymbols));
	m_listFileHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pFileHeader->NumberOfSymbols);
	m_listFileHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.SizeOfOptionalHeader) + m_dwPeStart);
	listindex = m_listFileHeader.InsertItem(listindex + 1, wstr);
	m_listFileHeader.SetItemText(listindex, 1, L"SizeOfOptionalHeader");
	swprintf_s(wstr, 2, L"%X", sizeof(pFileHeader->SizeOfOptionalHeader));
	m_listFileHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%04X", pFileHeader->SizeOfOptionalHeader);
	m_listFileHeader.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.Characteristics) + m_dwPeStart);
	listindex = m_listFileHeader.InsertItem(listindex + 1, wstr);
	m_listFileHeader.SetItemText(listindex, 1, L"Characteristics");
	swprintf_s(wstr, 2, L"%X", sizeof(pFileHeader->Characteristics));
	m_listFileHeader.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pFileHeader->Characteristics);
	m_listFileHeader.SetItemText(listindex, 3, wstr);
	std::wstring  strCharact { };
	for (auto& i : mapCharacteristics)
		if (i.first & pFileHeader->Characteristics)
			strCharact += i.second + L"\n";
	if (!strCharact.empty())
	{
		strCharact.erase(strCharact.size() - 1); //to remove last '\n'
		m_listFileHeader.SetCellTooltip(listindex, 3, strCharact, L"Characteristics:");
	}
	return 0;
}

int CViewRightTL::CreateListOptHeader()
{
	PCLIBPE_OPTHEADER_VAR pOptHdr;
	if (m_pLibpe->GetOptionalHeader(pOptHdr) != S_OK)
		return -1;

	m_listOptHeader.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_OPTIONALHEADER, &m_stListInfo);
	m_listOptHeader.ShowWindow(SW_HIDE);
	m_listOptHeader.InsertColumn(0, L"Offset", LVCFMT_LEFT, 90);
	m_listOptHeader.SetHeaderColumnColor(0, g_clrOffset);
	m_listOptHeader.InsertColumn(1, L"Name", LVCFMT_CENTER, 215);
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

	WCHAR wstr[MAX_PATH];
	std::wstring wstrTmp;
	int listindex = 0;

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		const IMAGE_OPTIONAL_HEADER32* pOptHdr32 = &pOptHdr->stOptHdr32;

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.Magic) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"Magic");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->Magic));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pOptHdr32->Magic);
		m_listOptHeader.SetItemText(listindex, 3, wstr);
		m_listOptHeader.SetCellTooltip(listindex, 3, L"IMAGE_NT_OPTIONAL_HDR32_MAGIC", L"Magic:");

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorLinkerVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MajorLinkerVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->MajorLinkerVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%02X", pOptHdr32->MajorLinkerVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorLinkerVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MinorLinkerVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->MinorLinkerVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%02X", pOptHdr32->MinorLinkerVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfCode) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfCode");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SizeOfCode));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SizeOfCode);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfInitializedData) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfInitializedData");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SizeOfInitializedData));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SizeOfInitializedData);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfUninitializedData) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfUninitializedData");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SizeOfUninitializedData));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SizeOfUninitializedData);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.AddressOfEntryPoint) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"AddressOfEntryPoint");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->AddressOfEntryPoint));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->AddressOfEntryPoint);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.BaseOfCode) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"BaseOfCode");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->BaseOfCode));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->BaseOfCode);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.BaseOfData) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"BaseOfData");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->BaseOfData));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->BaseOfData);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.ImageBase) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"ImageBase");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->ImageBase));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->ImageBase);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SectionAlignment) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SectionAlignment");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SectionAlignment));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SectionAlignment);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.FileAlignment) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"FileAlignment");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->FileAlignment));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->FileAlignment);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorOperatingSystemVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MajorOperatingSystemVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->MajorOperatingSystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr32->MajorOperatingSystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorOperatingSystemVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MinorOperatingSystemVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->MinorOperatingSystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr32->MinorOperatingSystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorImageVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MajorImageVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->MajorImageVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr32->MajorImageVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorImageVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MinorImageVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->MinorImageVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr32->MinorImageVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorSubsystemVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MajorSubsystemVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->MajorSubsystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr32->MajorSubsystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorSubsystemVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MinorSubsystemVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->MinorSubsystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr32->MinorSubsystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.Win32VersionValue) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"Win32VersionValue");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->Win32VersionValue));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->Win32VersionValue);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfImage) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfImage");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SizeOfImage));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SizeOfImage);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfHeaders) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfHeaders");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SizeOfHeaders));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SizeOfHeaders);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.CheckSum) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"CheckSum");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->CheckSum));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->CheckSum);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.Subsystem) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"Subsystem");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->Subsystem));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr32->Subsystem);
		m_listOptHeader.SetItemText(listindex, 3, wstr);
		if (mapSubSystem.find(pOptHdr32->Subsystem) != mapSubSystem.end())
		{
			swprintf_s(wstr, MAX_PATH, L"%S", mapSubSystem.at(pOptHdr32->Subsystem).c_str());
			m_listOptHeader.SetCellTooltip(listindex, 3, wstr, L"Subsystem:");
		}

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DllCharacteristics) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"DllCharacteristics");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->DllCharacteristics));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pOptHdr32->DllCharacteristics);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		wstrTmp.clear();
		for (auto& i : mapDllCharacteristics)
		{
			if (i.first & pOptHdr32->DllCharacteristics)
				wstrTmp += i.second + L"\n";
		}
		if (!wstrTmp.empty())
		{
			wstrTmp.erase(wstrTmp.size() - 1);//to remove last '\n'
			m_listOptHeader.SetCellTooltip(listindex, 3, wstrTmp, L"DllCharacteristics:");
		}

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfStackReserve) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfStackReserve");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SizeOfStackReserve));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SizeOfStackReserve);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfStackCommit) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfStackCommit");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SizeOfStackCommit));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SizeOfStackCommit);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfHeapReserve) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfHeapReserve");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SizeOfHeapReserve));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SizeOfHeapReserve);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfHeapCommit) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfHeapCommit");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->SizeOfHeapCommit));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->SizeOfHeapCommit);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.LoaderFlags) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"LoaderFlags");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->LoaderFlags));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->LoaderFlags);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.NumberOfRvaAndSizes) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"NumberOfRvaAndSizes");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr32->NumberOfRvaAndSizes));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr32->NumberOfRvaAndSizes);
		m_listOptHeader.SetItemText(listindex, 3, wstr);
	}
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
	{
		const IMAGE_OPTIONAL_HEADER64* pOptHdr64 = &pOptHdr->stOptHdr64;

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.Magic) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"Magic");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->Magic));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pOptHdr64->Magic);
		m_listOptHeader.SetItemText(listindex, 3, wstr);
		m_listOptHeader.SetCellTooltip(listindex, 3, L"IMAGE_NT_OPTIONAL_HDR64_MAGIC", L"Magic:");

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorLinkerVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MajorLinkerVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->MajorLinkerVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%02X", pOptHdr64->MajorLinkerVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorLinkerVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MinorLinkerVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->MinorLinkerVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%02X", pOptHdr64->MinorLinkerVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfCode) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfCode");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SizeOfCode));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->SizeOfCode);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfInitializedData) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfInitializedData");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SizeOfInitializedData));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->SizeOfInitializedData);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfUninitializedData) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfUninitializedData");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SizeOfUninitializedData));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->SizeOfUninitializedData);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.AddressOfEntryPoint) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"AddressOfEntryPoint");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->AddressOfEntryPoint));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->AddressOfEntryPoint);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.BaseOfCode) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"BaseOfCode");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->BaseOfCode));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->BaseOfCode);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.ImageBase) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"ImageBase");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->ImageBase));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pOptHdr64->ImageBase);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SectionAlignment) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SectionAlignment");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SectionAlignment));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->SectionAlignment);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.FileAlignment) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"FileAlignment");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->FileAlignment));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->FileAlignment);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorOperatingSystemVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MajorOperatingSystemVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->MajorOperatingSystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr64->MajorOperatingSystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorOperatingSystemVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MinorOperatingSystemVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->MinorOperatingSystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr64->MinorOperatingSystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorImageVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MajorImageVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->MajorImageVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr64->MajorImageVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorImageVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->MinorImageVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr64->MinorImageVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorSubsystemVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MajorSubsystemVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->MajorSubsystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr64->MajorSubsystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorSubsystemVersion) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"MinorSubsystemVersion");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->MinorSubsystemVersion));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr64->MinorSubsystemVersion);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.Win32VersionValue) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"Win32VersionValue");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->Win32VersionValue));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->Win32VersionValue);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfImage) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfImage");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SizeOfImage));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->SizeOfImage);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeaders) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfHeaders");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SizeOfHeaders));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->SizeOfHeaders);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.CheckSum) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"CheckSum");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->CheckSum));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->CheckSum);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.Subsystem) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"Subsystem");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->Subsystem));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pOptHdr64->Subsystem);
		m_listOptHeader.SetItemText(listindex, 3, wstr);
		if (mapSubSystem.find(pOptHdr64->Subsystem) != mapSubSystem.end())
		{
			swprintf_s(wstr, MAX_PATH, L"%S", mapSubSystem.at(pOptHdr64->Subsystem).c_str());
			m_listOptHeader.SetCellTooltip(listindex, 3, wstr, L"Subsystem:");
		}

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DllCharacteristics) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"DllCharacteristics");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->DllCharacteristics));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pOptHdr64->DllCharacteristics);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		wstrTmp.clear();
		for (auto& i : mapDllCharacteristics)
		{
			if (i.first & pOptHdr64->DllCharacteristics)
				wstrTmp += i.second + L"\n";
		}
		if (!wstrTmp.empty())
		{
			wstrTmp.erase(wstrTmp.size() - 1);//to remove last '\n'
			m_listOptHeader.SetCellTooltip(listindex, 3, wstrTmp, L"DllCharacteristics:");
		}

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfStackReserve) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SizeOfStackReserve));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pOptHdr64->SizeOfStackReserve);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfStackCommit) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfStackCommit");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SizeOfStackCommit));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pOptHdr64->SizeOfStackCommit);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeapReserve) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfHeapReserve");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SizeOfHeapReserve));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pOptHdr64->SizeOfHeapReserve);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeapCommit) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"SizeOfHeapCommit");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->SizeOfHeapCommit));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pOptHdr64->SizeOfHeapCommit);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.LoaderFlags) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"LoaderFlags");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->LoaderFlags));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->LoaderFlags);
		m_listOptHeader.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.NumberOfRvaAndSizes) + m_dwPeStart);
		listindex = m_listOptHeader.InsertItem(listindex + 1, wstr);
		m_listOptHeader.SetItemText(listindex, 1, L"NumberOfRvaAndSizes");
		swprintf_s(wstr, 2, L"%X", sizeof(pOptHdr64->NumberOfRvaAndSizes));
		m_listOptHeader.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pOptHdr64->NumberOfRvaAndSizes);
		m_listOptHeader.SetItemText(listindex, 3, wstr);
	}

	return 0;
}

int CViewRightTL::CreateListDataDirectories()
{
	PCLIBPE_DATADIRS_VEC pLibPeDataDirs { };
	if (m_pLibpe->GetDataDirectories(pLibPeDataDirs) != S_OK)
		return -1;

	m_listDataDirs.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_DATADIRECTORIES, &m_stListInfo);
	m_listDataDirs.ShowWindow(SW_HIDE);
	m_listDataDirs.InsertColumn(0, L"Offset", LVCFMT_LEFT, 90);
	m_listDataDirs.SetHeaderColumnColor(0, g_clrOffset);
	m_listDataDirs.InsertColumn(1, L"Name", LVCFMT_CENTER, 200);
	m_listDataDirs.InsertColumn(2, L"Directory RVA", LVCFMT_LEFT, 100);
	m_listDataDirs.InsertColumn(3, L"Directory Size", LVCFMT_LEFT, 100);
	m_listDataDirs.InsertColumn(4, L"Resides in Section", LVCFMT_LEFT, 125);

	WCHAR wstr[9];
	UINT listindex = 0;
	DWORD dwDataDirsOffset { };

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
		dwDataDirsOffset = m_dwPeStart + offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
		dwDataDirsOffset = m_dwPeStart + offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);

	const IMAGE_DATA_DIRECTORY* pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXPORT).stDataDir;

	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Export Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXPORT).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);
	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_IMPORT).stDataDir;

	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Import Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_IMPORT).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_RESOURCE).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Resource Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_RESOURCE).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXCEPTION).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Exception Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXCEPTION).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_SECURITY).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Security Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	if (pDataDirs->VirtualAddress)
		m_listDataDirs.SetCellTooltip(listindex, 2, L"This address is a file raw offset on disk.");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_SECURITY).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_BASERELOC).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Relocation Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_BASERELOC).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_DEBUG).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Debug Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_DEBUG).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_ARCHITECTURE).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Architecture Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_ARCHITECTURE).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_GLOBALPTR).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Global PTR");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_GLOBALPTR).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_TLS).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"TLS Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_TLS).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Load Config Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Bound Import Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_IAT).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"IAT Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_IAT).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"Delay Import Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	pDataDirs = &pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR).stDataDir;
	dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	swprintf_s(wstr, 9, L"%08X", dwDataDirsOffset);
	listindex = m_listDataDirs.InsertItem(listindex + 1, wstr);
	m_listDataDirs.SetItemText(listindex, 1, L"COM Descriptor Directory");
	swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(listindex, 3, wstr);
	swprintf_s(wstr, 9, L"%.8S", pLibPeDataDirs->at(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR).strSecResidesIn.data());
	m_listDataDirs.SetItemText(listindex, 4, wstr);

	return 0;
}

int CViewRightTL::CreateListSecHeaders()
{
	if (!m_pSecHeaders)
		return -1;

	m_listSecHeaders.Create(WS_CHILD | WS_VISIBLE | LVS_OWNERDATA, CRect(0, 0, 0, 0), this, IDC_LIST_SECHEADERS, &m_stListInfo);
	m_listSecHeaders.ShowWindow(SW_HIDE);
	m_listSecHeaders.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listSecHeaders.SetHeaderColumnColor(0, g_clrOffset);
	m_listSecHeaders.InsertColumn(1, L"Name", LVCFMT_CENTER, 150);
	m_listSecHeaders.InsertColumn(2, L"Virtual Size", LVCFMT_LEFT, 100);
	m_listSecHeaders.InsertColumn(3, L"Virtual Address", LVCFMT_LEFT, 125);
	m_listSecHeaders.InsertColumn(4, L"SizeOfRawData", LVCFMT_LEFT, 125);
	m_listSecHeaders.InsertColumn(5, L"PointerToRawData", LVCFMT_LEFT, 125);
	m_listSecHeaders.InsertColumn(6, L"PointerToRelocations", LVCFMT_LEFT, 150);
	m_listSecHeaders.InsertColumn(7, L"PointerToLinenumbers", LVCFMT_LEFT, 160);
	m_listSecHeaders.InsertColumn(8, L"NumberOfRelocations", LVCFMT_LEFT, 150);
	m_listSecHeaders.InsertColumn(9, L"NumberOfLinenumbers", LVCFMT_LEFT, 160);
	m_listSecHeaders.InsertColumn(10, L"Characteristics", LVCFMT_LEFT, 115);
	m_listSecHeaders.SetItemCountEx(m_pSecHeaders->size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);

	std::map<DWORD, std::wstring> mapSecFlags {
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
	std::wstring wstrTipText;

	for (auto& iterSecHdrs : *m_pSecHeaders)
	{
		for (auto& flags : mapSecFlags)
			if (flags.first & iterSecHdrs.stSecHdr.Characteristics)
				wstrTipText += flags.second + L"\n";

		if (!wstrTipText.empty())
		{
			m_listSecHeaders.SetCellTooltip(listindex, 10, wstrTipText, L"Section Flags:");
			wstrTipText.clear();
		}

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListExport()
{
	PCLIBPE_EXPORT pExport;
	if (m_pLibpe->GetExport(pExport) != S_OK)
		return -1;

	WCHAR wstr[MAX_PATH];
	int listindex = 0;
	const IMAGE_EXPORT_DIRECTORY* pExportDesc = &pExport->stExportDesc;

	m_listExportDir.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_EXPORT, &m_stListInfo);
	m_listExportDir.ShowWindow(SW_HIDE);
	m_listExportDir.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listExportDir.SetHeaderColumnColor(0, g_clrOffset);
	m_listExportDir.InsertColumn(1, L"Name", LVCFMT_CENTER, 250);
	m_listExportDir.InsertColumn(2, L"Size [BYTES]", LVCFMT_LEFT, 100);
	m_listExportDir.InsertColumn(3, L"Value", LVCFMT_LEFT, 300);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc);
	m_listExportDir.InsertItem(listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"Export flags (Characteristics)");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->Characteristics));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pExportDesc->Characteristics);
	m_listExportDir.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, TimeDateStamp));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"Time/Date Stamp");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->TimeDateStamp));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, MAX_PATH, L"%08X", pExportDesc->TimeDateStamp);
	m_listExportDir.SetItemText(listindex, 3, wstr);
	if (pExportDesc->TimeDateStamp)
	{
		__time64_t time = pExportDesc->TimeDateStamp;
		_wctime64_s(wstr, MAX_PATH, &time);
		m_listExportDir.SetCellTooltip(listindex, 3, wstr, L"Time / Date:");
	}

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, MajorVersion));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"MajorVersion");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->MajorVersion));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pExportDesc->MajorVersion);
	m_listExportDir.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, MinorVersion));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"MinorVersion");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->MinorVersion));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 5, L"%04X", pExportDesc->MinorVersion);
	m_listExportDir.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, Name));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"Name RVA");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->Name));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, MAX_PATH, L"%08X (%S)", pExportDesc->Name, pExport->strModuleName.data());
	m_listExportDir.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, Base));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"Base (OrdinalBase)");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->Base));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pExportDesc->Base);
	m_listExportDir.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, NumberOfFunctions));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"NumberOfFunctions");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->NumberOfFunctions));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pExportDesc->NumberOfFunctions);
	m_listExportDir.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, NumberOfNames));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"NumberOfNames");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->NumberOfNames));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pExportDesc->NumberOfNames);
	m_listExportDir.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, AddressOfFunctions));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"AddressOfFunctions");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->AddressOfFunctions));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pExportDesc->AddressOfFunctions);
	m_listExportDir.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, AddressOfNames));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"AddressOfNames");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->AddressOfNames));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pExportDesc->AddressOfNames);
	m_listExportDir.SetItemText(listindex, 3, wstr);

	swprintf_s(wstr, 9, L"%08X", pExport->dwOffsetExportDesc + offsetof(IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals));
	m_listExportDir.InsertItem(++listindex, wstr);
	m_listExportDir.SetItemText(listindex, 1, L"AddressOfNameOrdinals");
	swprintf_s(wstr, 2, L"%X", sizeof(pExportDesc->AddressOfNameOrdinals));
	m_listExportDir.SetItemText(listindex, 2, wstr);
	swprintf_s(wstr, 9, L"%08X", pExportDesc->AddressOfNameOrdinals);
	m_listExportDir.SetItemText(listindex, 3, wstr);

	return 0;
}

int CViewRightTL::CreateListImport()
{
	if (!m_pImport)
		return -1;

	m_listImport.Create(WS_VISIBLE | LVS_OWNERDATA, CRect(0, 0, 0, 0), this, IDC_LIST_IMPORT, &m_stListInfo);
	m_listImport.ShowWindow(SW_HIDE);
	m_listImport.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listImport.SetHeaderColumnColor(0, g_clrOffset);
	m_listImport.InsertColumn(1, L"Module Name (funcs number)", LVCFMT_CENTER, 300);
	m_listImport.InsertColumn(2, L"OriginalFirstThunk\n(Import Lookup Table)", LVCFMT_LEFT, 170);
	m_listImport.InsertColumn(3, L"TimeDateStamp", LVCFMT_LEFT, 115);
	m_listImport.InsertColumn(4, L"ForwarderChain", LVCFMT_LEFT, 110);
	m_listImport.InsertColumn(5, L"Name RVA", LVCFMT_LEFT, 90);
	m_listImport.InsertColumn(6, L"FirstThunk (IAT)", LVCFMT_LEFT, 135);
	m_listImport.SetItemCountEx(m_pImport->size(), LVSICF_NOSCROLL);
	m_listImport.SetListMenu(&m_menuList);

	WCHAR wstr[MAX_PATH];
	int listindex = 0;

	for (auto& i : *m_pImport)
	{
		const IMAGE_IMPORT_DESCRIPTOR* pImpDesc = &i.stImportDesc;
		if (pImpDesc->TimeDateStamp)
		{
			__time64_t time = pImpDesc->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &time);
			m_listImport.SetCellTooltip(listindex, 3, wstr, L"Time / Date:");
		}

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateTreeResources()
{
	PCLIBPE_RESOURCE_ROOT pResRoot;

	if (m_pLibpe->GetResources(pResRoot) != S_OK)
		return -1;

	m_treeResTop.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CRect(0, 0, 0, 0), this, IDC_TREE_RESOURCE_TOP);
	m_treeResTop.ShowWindow(SW_HIDE);

	m_hTreeResDir = m_treeResTop.InsertItem(L"Resource tree.");

	WCHAR wstr[MAX_PATH];
	HTREEITEM treeRoot { }, treeLvL2 { };
	long ilvlRoot = 0, ilvl2 = 0, ilvl3 = 0;

	//Main loop to extract Resources from tuple.
	for (auto& iterRoot : pResRoot->vecResRoot)
	{
		const IMAGE_RESOURCE_DIRECTORY_ENTRY* pResDirEntry = &iterRoot.stResDirEntryRoot;
		if (pResDirEntry->DataIsDirectory)
		{
			if (pResDirEntry->NameIsString)
				swprintf(wstr, MAX_PATH, L"Entry: %i [Name: %s]", ilvlRoot, iterRoot.wstrResNameRoot.data());
			else
			{
				if (g_mapResType.find(pResDirEntry->Id) != g_mapResType.end())
					swprintf(wstr, MAX_PATH, L"Entry: %i [Id: %u, %s]",
						ilvlRoot, pResDirEntry->Id, g_mapResType.at(pResDirEntry->Id).data());
				else
					swprintf(wstr, MAX_PATH, L"Entry: %i [Id: %u]", ilvlRoot, pResDirEntry->Id);
			}
			treeRoot = m_treeResTop.InsertItem(wstr, m_hTreeResDir);
			m_vecResId.emplace_back(ilvlRoot, -1, -1);
			m_treeResTop.SetItemData(treeRoot, m_vecResId.size() - 1);
			ilvl2 = 0;

			const PCLIBPE_RESOURCE_LVL2 pstResLvL2 = &iterRoot.stResLvL2;
			for (auto& iterLvL2 : pstResLvL2->vecResLvL2)
			{
				pResDirEntry = &iterLvL2.stResDirEntryLvL2;
				if (pResDirEntry->DataIsDirectory)
				{
					if (pResDirEntry->NameIsString)
						swprintf(wstr, MAX_PATH, L"Entry: %i, Name: %s", ilvl2, iterLvL2.wstrResNameLvL2.data());
					else
						swprintf(wstr, MAX_PATH, L"Entry: %i, Id: %u", ilvl2, pResDirEntry->Id);

					treeLvL2 = m_treeResTop.InsertItem(wstr, treeRoot);
					m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
					m_treeResTop.SetItemData(treeLvL2, m_vecResId.size() - 1);
					ilvl3 = 0;

					const PCLIBPE_RESOURCE_LVL3 pstResLvL3 = &iterLvL2.stResLvL3;
					for (auto& iterLvL3 : pstResLvL3->vecResLvL3)
					{
						pResDirEntry = &iterLvL3.stResDirEntryLvL3;

						if (pResDirEntry->NameIsString)
							swprintf(wstr, MAX_PATH, L"Entry: %i, Name: %s", ilvl3, iterLvL3.wstrResNameLvL3.data());
						else
							swprintf(wstr, MAX_PATH, L"Entry: %i, lang: %u", ilvl3, pResDirEntry->Id);

						const HTREEITEM treeLvL3 = m_treeResTop.InsertItem(wstr, treeLvL2);
						m_vecResId.emplace_back(ilvlRoot, ilvl2, ilvl3);
						m_treeResTop.SetItemData(treeLvL3, m_vecResId.size() - 1);

						ilvl3++;
					}
				}
				else
				{	//DATA lvl2
					pResDirEntry = &iterLvL2.stResDirEntryLvL2;

					if (pResDirEntry->NameIsString)
						swprintf(wstr, MAX_PATH, L"Entry: %i, Name: %s", ilvl2, iterLvL2.wstrResNameLvL2.data());
					else
						swprintf(wstr, MAX_PATH, L"Entry: %i, lang: %u", ilvl2, pResDirEntry->Id);

					treeLvL2 = m_treeResTop.InsertItem(wstr, treeRoot);
					m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
					m_treeResTop.SetItemData(treeLvL2, m_vecResId.size() - 1);
				}
				ilvl2++;
			}
		}
		else
		{	//DATA lvlroot
			pResDirEntry = &iterRoot.stResDirEntryRoot;

			if (pResDirEntry->NameIsString)
				swprintf(wstr, MAX_PATH, L"Entry: %i, Name: %s", ilvlRoot, iterRoot.wstrResNameRoot.data());
			else
				swprintf(wstr, MAX_PATH, L"Entry: %i, lang: %u", ilvlRoot, pResDirEntry->Id);

			treeRoot = m_treeResTop.InsertItem(wstr, m_hTreeResDir);
			m_vecResId.emplace_back(ilvlRoot, -1, -1);
			m_treeResTop.SetItemData(treeRoot, m_vecResId.size() - 1);
		}
		ilvlRoot++;
	}
	m_treeResTop.Expand(m_hTreeResDir, TVE_EXPAND);

	return 0;
}

int CViewRightTL::CreateListExceptions()
{
	if (!m_pExceptionDir)
		return -1;

	m_listExceptionDir.Create(WS_CHILD | WS_VISIBLE | LVS_OWNERDATA, CRect(0, 0, 0, 0), this, IDC_LIST_EXCEPTION, &m_stListInfo);
	m_listExceptionDir.ShowWindow(SW_HIDE);
	m_listExceptionDir.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listExceptionDir.SetHeaderColumnColor(0, g_clrOffset);
	m_listExceptionDir.InsertColumn(1, L"BeginAddress", LVCFMT_CENTER, 100);
	m_listExceptionDir.InsertColumn(2, L"EndAddress", LVCFMT_LEFT, 100);
	m_listExceptionDir.InsertColumn(3, L"UnwindData/InfoAddress", LVCFMT_LEFT, 180);
	m_listExceptionDir.SetItemCountEx(m_pExceptionDir->size(), LVSICF_NOSCROLL);

	return 0;
}

int CViewRightTL::CreateListSecurity()
{
	PCLIBPE_SECURITY_VEC pSecurityDir { };
	if (m_pLibpe->GetSecurity(pSecurityDir) != S_OK)
		return -1;

	m_listSecurityDir.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_SECURITY, &m_stListInfo);
	m_listSecurityDir.ShowWindow(SW_HIDE);
	m_listSecurityDir.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listSecurityDir.SetHeaderColumnColor(0, g_clrOffset);
	m_listSecurityDir.InsertColumn(1, L"dwLength", LVCFMT_CENTER, 100);
	m_listSecurityDir.InsertColumn(2, L"wRevision", LVCFMT_LEFT, 100);
	m_listSecurityDir.InsertColumn(3, L"wCertificateType", LVCFMT_LEFT, 180);

	int listindex = 0;
	WCHAR wstr[9];
	for (auto& i : *pSecurityDir)
	{
		swprintf_s(wstr, 9, L"%08X", i.dwOffsetWinCertDesc);
		m_listSecurityDir.InsertItem(listindex, wstr);

		const WIN_CERTIFICATE* pSert = &i.stWinSert;
		swprintf_s(wstr, 9, L"%08X", pSert->dwLength);
		m_listSecurityDir.SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 5, L"%04X", pSert->wRevision);
		m_listSecurityDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pSert->wCertificateType);
		m_listSecurityDir.SetItemText(listindex, 3, wstr);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListRelocations()
{
	if (!m_pRelocTable)
		return -1;

	m_listRelocDir.Create(WS_CHILD | WS_VISIBLE | LVS_OWNERDATA, CRect(0, 0, 0, 0), this, IDC_LIST_RELOCATIONS, &m_stListInfo);
	m_listRelocDir.ShowWindow(SW_HIDE);
	m_listRelocDir.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listRelocDir.SetHeaderColumnColor(0, g_clrOffset);
	m_listRelocDir.InsertColumn(1, L"Virtual Address", LVCFMT_CENTER, 115);
	m_listRelocDir.InsertColumn(2, L"Block Size", LVCFMT_LEFT, 100);
	m_listRelocDir.InsertColumn(3, L"Entries", LVCFMT_LEFT, 100);
	m_listRelocDir.SetItemCountEx(m_pRelocTable->size(), LVSICF_NOSCROLL);

	return 0;
}

int CViewRightTL::CreateListDebug()
{
	PCLIBPE_DEBUG_VEC pDebugDir { };

	if (m_pLibpe->GetDebug(pDebugDir) != S_OK)
		return -1;

	m_listDebugDir.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_DEBUG, &m_stListInfo);
	m_listDebugDir.ShowWindow(SW_HIDE);
	m_listDebugDir.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listDebugDir.SetHeaderColumnColor(0, g_clrOffset);
	m_listDebugDir.InsertColumn(1, L"Characteristics", LVCFMT_CENTER, 115);
	m_listDebugDir.InsertColumn(2, L"TimeDateStamp", LVCFMT_LEFT, 150);
	m_listDebugDir.InsertColumn(3, L"MajorVersion", LVCFMT_LEFT, 100);
	m_listDebugDir.InsertColumn(4, L"MinorVersion", LVCFMT_LEFT, 100);
	m_listDebugDir.InsertColumn(5, L"Type", LVCFMT_LEFT, 90);
	m_listDebugDir.InsertColumn(6, L"SizeOfData", LVCFMT_LEFT, 100);
	m_listDebugDir.InsertColumn(7, L"AddressOfRawData", LVCFMT_LEFT, 170);
	m_listDebugDir.InsertColumn(8, L"PointerToRawData", LVCFMT_LEFT, 140);

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
	WCHAR wstr[MAX_PATH];

	for (auto& i : *pDebugDir)
	{
		const IMAGE_DEBUG_DIRECTORY* pDebug = &i.stDebugDir;

		swprintf_s(wstr, 9, L"%08X", i.dwOffsetDebug);
		m_listDebugDir.InsertItem(listindex, wstr);
		swprintf_s(wstr, 9, L"%08X", pDebug->Characteristics);
		m_listDebugDir.SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, MAX_PATH, L"%08X", pDebug->TimeDateStamp);
		m_listDebugDir.SetItemText(listindex, 2, wstr);
		if (pDebug->TimeDateStamp)
		{
			__time64_t time = pDebug->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &time);
			m_listDebugDir.SetCellTooltip(listindex, 2, wstr, L"Time / Date:");
		}
		swprintf_s(wstr, 5, L"%04u", pDebug->MajorVersion);
		m_listDebugDir.SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, 5, L"%04u", pDebug->MinorVersion);
		m_listDebugDir.SetItemText(listindex, 4, wstr);
		swprintf_s(wstr, 9, L"%08X", pDebug->Type);
		m_listDebugDir.SetItemText(listindex, 5, wstr);
		for (auto&j : mapDebugType)
			if (j.first == pDebug->Type)
				m_listDebugDir.SetCellTooltip(listindex, 5, j.second);
		swprintf_s(wstr, 9, L"%08X", pDebug->SizeOfData);
		m_listDebugDir.SetItemText(listindex, 6, wstr);
		swprintf_s(wstr, 9, L"%08X", pDebug->AddressOfRawData);
		m_listDebugDir.SetItemText(listindex, 7, wstr);
		swprintf_s(wstr, 9, L"%08X", pDebug->PointerToRawData);
		m_listDebugDir.SetItemText(listindex, 8, wstr);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListTLS()
{
	PCLIBPE_TLS pTLSDir;
	if (m_pLibpe->GetTLS(pTLSDir) != S_OK)
		return -1;

	m_listTLSDir.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_TLS, &m_stListInfo);
	m_listTLSDir.ShowWindow(SW_HIDE);
	m_listTLSDir.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listTLSDir.SetHeaderColumnColor(0, g_clrOffset);
	m_listTLSDir.InsertColumn(1, L"Name", LVCFMT_CENTER, 250);
	m_listTLSDir.InsertColumn(2, L"Size [BYTES]", LVCFMT_LEFT, 110);
	m_listTLSDir.InsertColumn(3, L"Value", LVCFMT_LEFT, 150);

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
	WCHAR wstr[MAX_PATH];

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		const IMAGE_TLS_DIRECTORY32*  pTLSDir32 = &pTLSDir->varTLS.stTLSDir32;

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS);
		m_listTLSDir.InsertItem(listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"StartAddressOfRawData");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir32->StartAddressOfRawData));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pTLSDir32->StartAddressOfRawData);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY32, EndAddressOfRawData));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"EndAddressOfRawData");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir32->EndAddressOfRawData));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pTLSDir32->EndAddressOfRawData);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY32, AddressOfIndex));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"AddressOfIndex");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir32->AddressOfIndex));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pTLSDir32->AddressOfIndex);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY32, AddressOfCallBacks));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"AddressOfCallBacks");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir32->AddressOfCallBacks));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pTLSDir32->AddressOfCallBacks);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY32, SizeOfZeroFill));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"SizeOfZeroFill");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir32->SizeOfZeroFill));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pTLSDir32->SizeOfZeroFill);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY32, Characteristics));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"Characteristics");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir32->Characteristics));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pTLSDir32->Characteristics);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		auto iterCharact = mapCharact.find(pTLSDir32->Characteristics);
		if (iterCharact != mapCharact.end())
			m_listTLSDir.SetCellTooltip(listindex, 3, iterCharact->second, L"Characteristics:");
	}
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
	{
		const IMAGE_TLS_DIRECTORY64* pTLSDir64 = &pTLSDir->varTLS.stTLSDir64;

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS);
		m_listTLSDir.InsertItem(listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"StartAddressOfRawData");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir64->StartAddressOfRawData));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pTLSDir64->StartAddressOfRawData);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY64, EndAddressOfRawData));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"EndAddressOfRawData");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir64->EndAddressOfRawData));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pTLSDir64->EndAddressOfRawData);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY64, AddressOfIndex));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"AddressOfIndex");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir64->AddressOfIndex));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pTLSDir64->AddressOfIndex);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY64, AddressOfCallBacks));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"AddressOfCallBacks");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir64->AddressOfCallBacks));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pTLSDir64->AddressOfCallBacks);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY64, SizeOfZeroFill));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"SizeOfZeroFill");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir64->SizeOfZeroFill));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pTLSDir64->SizeOfZeroFill);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		swprintf_s(wstr, 9, L"%08X", pTLSDir->dwOffsetTLS + offsetof(IMAGE_TLS_DIRECTORY64, Characteristics));
		m_listTLSDir.InsertItem(++listindex, wstr);
		m_listTLSDir.SetItemText(listindex, 1, L"Characteristics");
		swprintf_s(wstr, 3, L"%u", sizeof(pTLSDir64->Characteristics));
		m_listTLSDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pTLSDir64->Characteristics);
		m_listTLSDir.SetItemText(listindex, 3, wstr);

		auto iterCharact = mapCharact.find(pTLSDir64->Characteristics);
		if (iterCharact != mapCharact.end())
			m_listTLSDir.SetCellTooltip(listindex, 3, iterCharact->second, L"Characteristics:");
	}

	return 0;
}

int CViewRightTL::CreateListLoadConfigTable()
{
	PCLIBPE_LOADCONFIG pLCD;
	if (m_pLibpe->GetLoadConfig(pLCD) != S_OK)
		return -1;

	PCLIBPE_DATADIRS_VEC pDataDirs;
	if (m_pLibpe->GetDataDirectories(pDataDirs) != S_OK)
		return -1;

	m_listLCD.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_LOADCONFIG, &m_stListInfo);
	m_listLCD.ShowWindow(SW_HIDE);
	m_listLCD.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listLCD.SetHeaderColumnColor(0, g_clrOffset);
	m_listLCD.InsertColumn(1, L"Name", LVCFMT_CENTER, 330);
	m_listLCD.InsertColumn(2, L"Size [BYTES]", LVCFMT_LEFT, 110);
	m_listLCD.InsertColumn(3, L"Value", LVCFMT_LEFT, 300);

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
	WCHAR wstr[MAX_PATH];
	std::wstring wstrTooltip;
	DWORD dwLCDSize;
	DWORD dwTotalSize { };

	if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
	{
		const IMAGE_LOAD_CONFIG_DIRECTORY32* pLCD32 = &pLCD->varLCD.stLCD32;
		dwLCDSize = pLCD32->Size;

		dwTotalSize += sizeof(pLCD32->Size);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, Size));
		m_listLCD.InsertItem(listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"Size");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->Size));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->Size);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->TimeDateStamp);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, TimeDateStamp));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"TimeDateStamp");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->TimeDateStamp));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, MAX_PATH, L"%08X", pLCD32->TimeDateStamp);
		m_listLCD.SetItemText(listindex, 3, wstr);
		if (pLCD32->TimeDateStamp)
		{
			__time64_t time = pLCD32->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &time);
			m_listLCD.SetCellTooltip(listindex, 2, wstr, L"Time / Date:");
		}

		dwTotalSize += sizeof(pLCD32->MajorVersion);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, MajorVersion));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"MajorVersion");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->MajorVersion));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pLCD32->MajorVersion);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->MinorVersion);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, MinorVersion));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"MinorVersion");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->MinorVersion));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pLCD32->MinorVersion);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GlobalFlagsClear);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsClear));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GlobalFlagsClear");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GlobalFlagsClear));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GlobalFlagsClear);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GlobalFlagsSet);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsSet));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GlobalFlagsSet");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GlobalFlagsSet));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GlobalFlagsSet);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->CriticalSectionDefaultTimeout);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, CriticalSectionDefaultTimeout));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CriticalSectionDefaultTimeout");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->CriticalSectionDefaultTimeout));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->CriticalSectionDefaultTimeout);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->DeCommitFreeBlockThreshold);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitFreeBlockThreshold));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DeCommitFreeBlockThreshold");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->DeCommitFreeBlockThreshold));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->DeCommitFreeBlockThreshold);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->DeCommitTotalFreeThreshold);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitTotalFreeThreshold));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DeCommitTotalFreeThreshold");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->DeCommitTotalFreeThreshold));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->DeCommitTotalFreeThreshold);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->LockPrefixTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, LockPrefixTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"LockPrefixTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->LockPrefixTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->LockPrefixTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->MaximumAllocationSize);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, MaximumAllocationSize));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"MaximumAllocationSize");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->MaximumAllocationSize));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->MaximumAllocationSize);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->VirtualMemoryThreshold);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, VirtualMemoryThreshold));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"VirtualMemoryThreshold");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->VirtualMemoryThreshold));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->VirtualMemoryThreshold);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->ProcessHeapFlags);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, ProcessHeapFlags));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"ProcessHeapFlags");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->ProcessHeapFlags));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->ProcessHeapFlags);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->ProcessAffinityMask);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, ProcessAffinityMask));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"ProcessAffinityMask");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->ProcessAffinityMask));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->ProcessAffinityMask);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->CSDVersion);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, CSDVersion));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CSDVersion");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->CSDVersion));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pLCD32->CSDVersion);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->DependentLoadFlags);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, DependentLoadFlags));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DependentLoadFlags");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->DependentLoadFlags));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pLCD32->DependentLoadFlags);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->EditList);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, EditList));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"EditList");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->EditList));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->EditList);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->SecurityCookie);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, SecurityCookie));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"SecurityCookie");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->SecurityCookie));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->SecurityCookie);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->SEHandlerTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"SEHandlerTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->SEHandlerTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->SEHandlerTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->SEHandlerCount);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerCount));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"SEHandlerCount");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->SEHandlerCount));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->SEHandlerCount);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardCFCheckFunctionPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFCheckFunctionPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardCFCheckFunctionPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardCFCheckFunctionPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardCFCheckFunctionPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardCFDispatchFunctionPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFDispatchFunctionPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardCFDispatchFunctionPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardCFDispatchFunctionPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardCFDispatchFunctionPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardCFFunctionTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardCFFunctionTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardCFFunctionTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardCFFunctionTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardCFFunctionCount);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionCount));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardCFFunctionCount");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardCFFunctionCount));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardCFFunctionCount);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardFlags);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardFlags));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardFlags");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardFlags));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardFlags);
		m_listLCD.SetItemText(listindex, 3, wstr);
		wstrTooltip.clear();
		for (auto&i : mapGuardFlags)
			if (i.first & pLCD32->GuardFlags)
				wstrTooltip += i.second + L"\n";
		if (!wstrTooltip.empty())
			m_listLCD.SetCellTooltip(listindex, 3, wstrTooltip, L"GuardFlags:");

		dwTotalSize += sizeof(pLCD32->CodeIntegrity.Flags);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Catalog));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CodeIntegrity.Catalog");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->CodeIntegrity.Flags));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pLCD32->CodeIntegrity.Flags);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->CodeIntegrity.Catalog);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Catalog));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CodeIntegrity.Catalog");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->CodeIntegrity.Catalog));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pLCD32->CodeIntegrity.Catalog);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->CodeIntegrity.CatalogOffset);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.CatalogOffset));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CatalogOffset");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->CodeIntegrity.CatalogOffset));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->CodeIntegrity.CatalogOffset);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->CodeIntegrity.Reserved);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Reserved));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"Reserved");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->CodeIntegrity.Reserved));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->CodeIntegrity.Reserved);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardAddressTakenIatEntryTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardAddressTakenIatEntryTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardAddressTakenIatEntryTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardAddressTakenIatEntryTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardAddressTakenIatEntryCount);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryCount));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardAddressTakenIatEntryCount");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardAddressTakenIatEntryCount));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardAddressTakenIatEntryCount);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardLongJumpTargetTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardLongJumpTargetTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardLongJumpTargetTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardLongJumpTargetTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardLongJumpTargetCount);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetCount));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardLongJumpTargetCount");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardLongJumpTargetCount));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardLongJumpTargetCount);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->DynamicValueRelocTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DynamicValueRelocTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->DynamicValueRelocTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->DynamicValueRelocTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->CHPEMetadataPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, CHPEMetadataPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CHPEMetadataPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->CHPEMetadataPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->CHPEMetadataPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardRFFailureRoutine);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutine));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardRFFailureRoutine");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardRFFailureRoutine));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardRFFailureRoutine);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardRFFailureRoutineFunctionPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutineFunctionPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardRFFailureRoutineFunctionPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardRFFailureRoutineFunctionPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardRFFailureRoutineFunctionPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->DynamicValueRelocTableOffset);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableOffset));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DynamicValueRelocTableOffset");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->DynamicValueRelocTableOffset));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->DynamicValueRelocTableOffset);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->DynamicValueRelocTableSection);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableSection));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DynamicValueRelocTableSection");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->DynamicValueRelocTableSection));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pLCD32->DynamicValueRelocTableSection);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->Reserved2);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved2));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"Reserved2");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->Reserved2));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pLCD32->Reserved2);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->GuardRFVerifyStackPointerFunctionPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFVerifyStackPointerFunctionPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardRFVerifyStackPointerFunctionPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->GuardRFVerifyStackPointerFunctionPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->GuardRFVerifyStackPointerFunctionPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->HotPatchTableOffset);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, HotPatchTableOffset));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"HotPatchTableOffset");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->HotPatchTableOffset));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->HotPatchTableOffset);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->Reserved3);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved3));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"Reserved3");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->Reserved3));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->Reserved3);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD32->EnclaveConfigurationPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, EnclaveConfigurationPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"EnclaveConfigurationPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD32->EnclaveConfigurationPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD32->EnclaveConfigurationPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);
	}
	else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
	{
		const IMAGE_LOAD_CONFIG_DIRECTORY64* pLCD64 = &pLCD->varLCD.stLCD64;
		dwLCDSize = pLCD64->Size;

		dwTotalSize += sizeof(pLCD64->Size);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, Size));
		m_listLCD.InsertItem(listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"Size");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->Size));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->Size);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->TimeDateStamp);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, TimeDateStamp));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"TimeDateStamp");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->TimeDateStamp));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, MAX_PATH, L"%08X", pLCD64->TimeDateStamp);
		m_listLCD.SetItemText(listindex, 3, wstr);
		if (pLCD64->TimeDateStamp)
		{
			__time64_t time = pLCD64->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &time);
			m_listLCD.SetCellTooltip(listindex, 2, wstr, L"Time / Date:");
		}

		dwTotalSize += sizeof(pLCD64->MajorVersion);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, MajorVersion));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"MajorVersion");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->MajorVersion));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pLCD64->MajorVersion);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->MinorVersion);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, MinorVersion));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"MinorVersion");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->MinorVersion));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pLCD64->MinorVersion);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GlobalFlagsClear);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsClear));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GlobalFlagsClear");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GlobalFlagsClear));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->GlobalFlagsClear);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GlobalFlagsSet);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsSet));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GlobalFlagsSet");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GlobalFlagsSet));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->GlobalFlagsSet);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->CriticalSectionDefaultTimeout);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, CriticalSectionDefaultTimeout));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CriticalSectionDefaultTimeout");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->CriticalSectionDefaultTimeout));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->CriticalSectionDefaultTimeout);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->DeCommitFreeBlockThreshold);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitFreeBlockThreshold));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DeCommitFreeBlockThreshold");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->DeCommitFreeBlockThreshold));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->DeCommitFreeBlockThreshold);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->DeCommitTotalFreeThreshold);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitTotalFreeThreshold));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DeCommitTotalFreeThreshold");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->DeCommitTotalFreeThreshold));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->DeCommitTotalFreeThreshold);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->LockPrefixTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, LockPrefixTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"LockPrefixTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->LockPrefixTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->LockPrefixTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->MaximumAllocationSize);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, MaximumAllocationSize));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"MaximumAllocationSize");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->MaximumAllocationSize));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->MaximumAllocationSize);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->VirtualMemoryThreshold);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, VirtualMemoryThreshold));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"VirtualMemoryThreshold");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->VirtualMemoryThreshold));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->VirtualMemoryThreshold);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->ProcessHeapFlags);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, ProcessHeapFlags));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"ProcessHeapFlags");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->ProcessHeapFlags));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->ProcessHeapFlags);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->ProcessAffinityMask);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, ProcessAffinityMask));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"ProcessAffinityMask");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->ProcessAffinityMask));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->ProcessAffinityMask);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->CSDVersion);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, CSDVersion));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CSDVersion");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->CSDVersion));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pLCD64->CSDVersion);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->DependentLoadFlags);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, DependentLoadFlags));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DependentLoadFlags");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->DependentLoadFlags));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 5, L"%04X", pLCD64->DependentLoadFlags);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->EditList);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, EditList));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"EditList");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->EditList));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->EditList);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->SecurityCookie);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, SecurityCookie));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"SecurityCookie");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->SecurityCookie));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->SecurityCookie);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->SEHandlerTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"SEHandlerTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->SEHandlerTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->SEHandlerTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->SEHandlerCount);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerCount));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"SEHandlerCount");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->SEHandlerCount));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->SEHandlerCount);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardCFCheckFunctionPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFCheckFunctionPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardCFCheckFunctionPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardCFCheckFunctionPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardCFCheckFunctionPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardCFDispatchFunctionPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFDispatchFunctionPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardCFDispatchFunctionPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardCFDispatchFunctionPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardCFDispatchFunctionPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardCFFunctionTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardCFFunctionTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardCFFunctionTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardCFFunctionTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardCFFunctionCount);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionCount));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardCFFunctionCount");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardCFFunctionCount));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardCFFunctionCount);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardFlags);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardFlags));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardFlags");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardFlags));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->GuardFlags);
		m_listLCD.SetItemText(listindex, 3, wstr);
		wstrTooltip.clear();
		for (auto&i : mapGuardFlags)
			if (i.first & pLCD64->GuardFlags)
				wstrTooltip += i.second + L"\n";
		if (!wstrTooltip.empty())
			m_listLCD.SetCellTooltip(listindex, 3, wstrTooltip, L"GuardFlags:");

		dwTotalSize += sizeof(pLCD64->CodeIntegrity.Flags);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Catalog));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CodeIntegrity.Catalog");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->CodeIntegrity.Flags));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pLCD64->CodeIntegrity.Flags);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->CodeIntegrity.Catalog);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Catalog));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CodeIntegrity.Catalog");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->CodeIntegrity.Catalog));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pLCD64->CodeIntegrity.Catalog);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->CodeIntegrity.CatalogOffset);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.CatalogOffset));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CatalogOffset");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->CodeIntegrity.CatalogOffset));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->CodeIntegrity.CatalogOffset);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->CodeIntegrity.Reserved);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Reserved));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"Reserved");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->CodeIntegrity.Reserved));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->CodeIntegrity.Reserved);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardAddressTakenIatEntryTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardAddressTakenIatEntryTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardAddressTakenIatEntryTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardAddressTakenIatEntryTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardAddressTakenIatEntryCount);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryCount));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardAddressTakenIatEntryCount");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardAddressTakenIatEntryCount));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardAddressTakenIatEntryCount);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardLongJumpTargetTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardLongJumpTargetTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardLongJumpTargetTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardLongJumpTargetTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardLongJumpTargetCount);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetCount));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardLongJumpTargetCount");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardLongJumpTargetCount));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardLongJumpTargetCount);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->DynamicValueRelocTable);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTable));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DynamicValueRelocTable");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->DynamicValueRelocTable));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->DynamicValueRelocTable);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->CHPEMetadataPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, CHPEMetadataPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"CHPEMetadataPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->CHPEMetadataPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->CHPEMetadataPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardRFFailureRoutine);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutine));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardRFFailureRoutine");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardRFFailureRoutine));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardRFFailureRoutine);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardRFFailureRoutineFunctionPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutineFunctionPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardRFFailureRoutineFunctionPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardRFFailureRoutineFunctionPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardRFFailureRoutineFunctionPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->DynamicValueRelocTableOffset);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableOffset));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DynamicValueRelocTableOffset");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->DynamicValueRelocTableOffset));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->DynamicValueRelocTableOffset);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->DynamicValueRelocTableSection);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableSection));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"DynamicValueRelocTableSection");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->DynamicValueRelocTableSection));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pLCD64->DynamicValueRelocTableSection);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->Reserved2);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved2));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"Reserved2");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->Reserved2));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%04X", pLCD64->Reserved2);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->GuardRFVerifyStackPointerFunctionPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFVerifyStackPointerFunctionPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"GuardRFVerifyStackPointerFunctionPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->GuardRFVerifyStackPointerFunctionPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->GuardRFVerifyStackPointerFunctionPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->HotPatchTableOffset);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, HotPatchTableOffset));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"HotPatchTableOffset");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->HotPatchTableOffset));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->HotPatchTableOffset);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->Reserved3);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved3));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"Reserved3");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->Reserved3));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pLCD64->Reserved3);
		m_listLCD.SetItemText(listindex, 3, wstr);

		dwTotalSize += sizeof(pLCD64->EnclaveConfigurationPointer);
		if (dwTotalSize > dwLCDSize)
			return 0;

		swprintf_s(wstr, 9, L"%08X", pLCD->dwOffsetLCD + offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, EnclaveConfigurationPointer));
		m_listLCD.InsertItem(++listindex, wstr);
		m_listLCD.SetItemText(listindex, 1, L"EnclaveConfigurationPointer");
		swprintf_s(wstr, 3, L"%u", sizeof(pLCD64->EnclaveConfigurationPointer));
		m_listLCD.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%016llX", pLCD64->EnclaveConfigurationPointer);
		m_listLCD.SetItemText(listindex, 3, wstr);
	}

	return 0;
}

int CViewRightTL::CreateListBoundImport()
{
	PCLIBPE_BOUNDIMPORT_VEC pBoundImp;

	if (m_pLibpe->GetBoundImport(pBoundImp) != S_OK)
		return -1;

	m_listBoundImportDir.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_DELAYIMPORT, &m_stListInfo);
	m_listBoundImportDir.ShowWindow(SW_HIDE);
	m_listBoundImportDir.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listBoundImportDir.SetHeaderColumnColor(0, g_clrOffset);
	m_listBoundImportDir.InsertColumn(1, L"Module Name", LVCFMT_CENTER, 290);
	m_listBoundImportDir.InsertColumn(2, L"TimeDateStamp", LVCFMT_LEFT, 130);
	m_listBoundImportDir.InsertColumn(3, L"OffsetModuleName", LVCFMT_LEFT, 140);
	m_listBoundImportDir.InsertColumn(4, L"NumberOfModuleForwarderRefs", LVCFMT_LEFT, 220);

	WCHAR wstr[MAX_PATH];
	int listindex = 0;

	for (auto& i : *pBoundImp)
	{
		swprintf_s(wstr, 9, L"%08X", i.dwOffsetBoundImpDesc);
		m_listBoundImportDir.InsertItem(listindex, wstr);

		const IMAGE_BOUND_IMPORT_DESCRIPTOR* pBoundImpDir = &i.stBoundImpDesc;
		swprintf_s(wstr, MAX_PATH, L"%S", i.strBoundName.data());
		m_listBoundImportDir.SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, MAX_PATH, L"%08X", pBoundImpDir->TimeDateStamp);
		m_listBoundImportDir.SetItemText(listindex, 2, wstr);
		if (pBoundImpDir->TimeDateStamp)
		{
			__time64_t _time = pBoundImpDir->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &_time);
			m_listBoundImportDir.SetCellTooltip(listindex, 2, wstr, L"Time / Date:");
		}
		swprintf_s(wstr, 5, L"%04X", pBoundImpDir->OffsetModuleName);
		m_listBoundImportDir.SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, 5, L"%04u", pBoundImpDir->NumberOfModuleForwarderRefs);
		m_listBoundImportDir.SetItemText(listindex, 4, wstr);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListDelayImport()
{
	PCLIBPE_DELAYIMPORT_VEC pDelayImp;

	if (m_pLibpe->GetDelayImport(pDelayImp) != S_OK)
		return -1;

	m_listDelayImportDir.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_DELAYIMPORT, &m_stListInfo);
	m_listDelayImportDir.ShowWindow(SW_HIDE);
	m_listDelayImportDir.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listDelayImportDir.SetHeaderColumnColor(0, g_clrOffset);
	m_listDelayImportDir.InsertColumn(1, L"Module Name (funcs number)", LVCFMT_CENTER, 260);
	m_listDelayImportDir.InsertColumn(2, L"Attributes", LVCFMT_LEFT, 100);
	m_listDelayImportDir.InsertColumn(3, L"DllNameRVA", LVCFMT_LEFT, 105);
	m_listDelayImportDir.InsertColumn(4, L"ModuleHandleRVA", LVCFMT_LEFT, 140);
	m_listDelayImportDir.InsertColumn(5, L"ImportAddressTableRVA", LVCFMT_LEFT, 160);
	m_listDelayImportDir.InsertColumn(6, L"ImportNameTableRVA", LVCFMT_LEFT, 150);
	m_listDelayImportDir.InsertColumn(7, L"BoundImportAddressTableRVA", LVCFMT_LEFT, 200);
	m_listDelayImportDir.InsertColumn(8, L"UnloadInformationTableRVA", LVCFMT_LEFT, 190);
	m_listDelayImportDir.InsertColumn(9, L"TimeDateStamp", LVCFMT_LEFT, 115);

	int listindex = 0;
	WCHAR wstr[MAX_PATH];

	for (auto& i : *pDelayImp)
	{
		swprintf_s(wstr, 9, L"%08X", i.dwOffsetDelayImpDesc);
		m_listDelayImportDir.InsertItem(listindex, wstr);

		const IMAGE_DELAYLOAD_DESCRIPTOR* pDelayImpDir = &i.stDelayImpDesc;
		swprintf_s(wstr, MAX_PATH, L"%S (%u)", i.strModuleName.data(), i.vecDelayImpFunc.size());
		m_listDelayImportDir.SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->Attributes.AllAttributes);
		m_listDelayImportDir.SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->DllNameRVA);
		m_listDelayImportDir.SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->ModuleHandleRVA);
		m_listDelayImportDir.SetItemText(listindex, 4, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->ImportAddressTableRVA);
		m_listDelayImportDir.SetItemText(listindex, 5, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->ImportNameTableRVA);
		m_listDelayImportDir.SetItemText(listindex, 6, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->BoundImportAddressTableRVA);
		m_listDelayImportDir.SetItemText(listindex, 7, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->UnloadInformationTableRVA);
		m_listDelayImportDir.SetItemText(listindex, 8, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->TimeDateStamp);
		m_listDelayImportDir.SetItemText(listindex, 9, wstr);
		if (pDelayImpDir->TimeDateStamp)
		{
			__time64_t time = pDelayImpDir->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &time);
			m_listDelayImportDir.SetCellTooltip(listindex, 8, wstr, L"Time / Date:");
		}

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListCOM()
{
	PCLIBPE_COMDESCRIPTOR pCOMDesc;

	if (m_pLibpe->GetCOMDescriptor(pCOMDesc) != S_OK)
		return -1;

	m_listCOMDir.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_DELAYIMPORT, &m_stListInfo);
	m_listCOMDir.ShowWindow(SW_HIDE);
	m_listCOMDir.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listCOMDir.SetHeaderColumnColor(0, g_clrOffset);
	m_listCOMDir.InsertColumn(1, L"Name", LVCFMT_CENTER, 300);
	m_listCOMDir.InsertColumn(2, L"Value", LVCFMT_LEFT, 100);

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
	WCHAR wstr[MAX_PATH];
	std::wstring wstrToolTip;

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, cb));
	m_listCOMDir.InsertItem(listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"cb");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.cb);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, MajorRuntimeVersion));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"MajorRuntimeVersion");
	swprintf_s(wstr, 5, L"%04X", pCOMDesc->stCorHdr.MajorRuntimeVersion);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, MinorRuntimeVersion));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"MinorRuntimeVersion");
	swprintf_s(wstr, 5, L"%04X", pCOMDesc->stCorHdr.MinorRuntimeVersion);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, MetaData.VirtualAddress));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"MetaData.RVA");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.MetaData.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, MetaData.Size));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"MetaData.Size");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.MetaData.Size);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, Flags));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"Flags");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.Flags);
	m_listCOMDir.SetItemText(listindex, 2, wstr);
	for (auto&i : mapFlags)
		if (i.first & pCOMDesc->stCorHdr.Flags)
			wstrToolTip += i.second + L"\n";
	if (!wstrToolTip.empty())
		m_listCOMDir.SetCellTooltip(listindex, 2, wstrToolTip, L"Flags:");

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, EntryPointToken));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"EntryPointToken");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.EntryPointToken);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, Resources.VirtualAddress));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"Resources.RVA");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.Resources.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, Resources.Size));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"Resources.Size");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.Resources.Size);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, StrongNameSignature.VirtualAddress));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"StrongNameSignature.RVA");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.StrongNameSignature.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, StrongNameSignature.Size));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"StrongNameSignature.Size");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.StrongNameSignature.Size);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, CodeManagerTable.VirtualAddress));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"CodeManagerTable.RVA");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.CodeManagerTable.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, CodeManagerTable.Size));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"CodeManagerTable.Size");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.CodeManagerTable.Size);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, VTableFixups.VirtualAddress));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"VTableFixups.RVA");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.VTableFixups.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, VTableFixups.Size));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"VTableFixups.Size");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.VTableFixups.Size);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, ExportAddressTableJumps.VirtualAddress));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"ExportAddressTableJumps.RVA");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.ExportAddressTableJumps.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, ExportAddressTableJumps.Size));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"ExportAddressTableJumps.Size");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.ExportAddressTableJumps.Size);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, ManagedNativeHeader.VirtualAddress));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"ManagedNativeHeader.RVA");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.ManagedNativeHeader.VirtualAddress);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	swprintf_s(wstr, 9, L"%08X", pCOMDesc->dwOffsetComDesc + offsetof(IMAGE_COR20_HEADER, ManagedNativeHeader.Size));
	m_listCOMDir.InsertItem(++listindex, wstr);
	m_listCOMDir.SetItemText(listindex, 1, L"ManagedNativeHeader.Size");
	swprintf_s(wstr, 9, L"%08X", pCOMDesc->stCorHdr.ManagedNativeHeader.Size);
	m_listCOMDir.SetItemText(listindex, 2, wstr);

	return 0;
}