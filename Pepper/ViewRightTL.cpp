/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "ViewRightTL.h"
#include <algorithm>

IMPLEMENT_DYNCREATE(CViewRightTL, CView)

BEGIN_MESSAGE_MAP(CViewRightTL, CView)
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_NOTIFY(LVN_GETDISPINFOW, IDC_LIST_SECHEADERS, &CViewRightTL::OnListSectionsGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFOW, IDC_LIST_IMPORT, &CViewRightTL::OnListImportGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFOW, IDC_LIST_RELOCATIONS, &CViewRightTL::OnListRelocsGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFOW, IDC_LIST_EXCEPTIONS, &CViewRightTL::OnListExceptionsGetDispInfo)
	ON_NOTIFY(LISTEX_MSG_GETTOOLTIP, IDC_LIST_SECHEADERS, &CViewRightTL::OnListSecHdrGetToolTip)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_RESOURCE_TOP, &CViewRightTL::OnTreeResTopSelChange)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CViewRightTL::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_pChildFrame = static_cast<CChildFrame*>(GetParentFrame());
	m_pMainDoc = static_cast<CPepperDoc*>(GetDocument());
	m_pLibpe = m_pMainDoc->m_pLibpe;
	m_pFileLoader = &m_pMainDoc->m_stFileLoader;

	LOGFONTW lf { };
	StringCchCopyW(lf.lfFaceName, 18, L"Consolas");
	lf.lfHeight = 22;
	if (!m_fontSummary.CreateFontIndirectW(&lf))
	{
		StringCchCopyW(lf.lfFaceName, 18, L"Times New Roman");
		m_fontSummary.CreateFontIndirectW(&lf);
	}

	if (m_pLibpe->GetImageInfo(m_dwFileInfo) != S_OK)
		return;

	m_wstrFullPath = L"Full path: " + m_pMainDoc->GetPathName();
	m_wstrFileName = m_pMainDoc->GetPathName();
	m_wstrFileName.erase(0, m_wstrFileName.find_last_of('\\') + 1);
	m_wstrFileName.insert(0, L"File name: ");

	if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE32))
		m_wstrFileType = L"File type: PE32 (x86)";
	else if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE64))
		m_wstrFileType = L"File type: PE32+ (x64)";
	else
		m_wstrFileType = L"File type: unknown";

	m_wstrPepperVersion = PEPPER_VERSION_WSTR;

	//There can be the absence of some structures in PE.
	//So it's ok to return not S_OK here.
	m_pLibpe->GetSectionsHeaders(m_pSecHeaders);
	m_pLibpe->GetImport(m_pImport);
	m_pLibpe->GetExceptions(m_pExceptionDir);
	m_pLibpe->GetRelocations(m_pRelocTable);

	m_stlcs.stColor.clrTooltipText = RGB(255, 255, 255);
	m_stlcs.stColor.clrTooltipBk = RGB(0, 132, 132);
	m_stlcs.stColor.clrHdrText = RGB(255, 255, 255);
	m_stlcs.stColor.clrHdrBk = RGB(0, 132, 132);
	m_stlcs.stColor.clrHdrHglInact = RGB(0, 112, 112);
	m_stlcs.stColor.clrHdrHglAct = RGB(0, 92, 92);
	m_stlcs.pParent = this;
	m_stlcs.dwHdrHeight = 39;
	m_stlcs.fSortable = true;

	m_lf.lfHeight = 16;
	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stlcs.pListLogFont = &m_lf;
	m_hdrlf.lfHeight = 17;
	m_hdrlf.lfWeight = FW_BOLD;
	StringCchCopyW(m_hdrlf.lfFaceName, 16, L"Times New Roman");
	m_stlcs.pHdrLogFont = &m_hdrlf;

	m_menuList.CreatePopupMenu();
	m_menuList.AppendMenuW(MF_STRING, IDM_LIST_GOTODESCOFFSET, L"Go to descriptor offset");
	m_menuList.AppendMenuW(MF_STRING, IDM_LIST_GOTODATAOFFSET, L"Go to data offset");

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
	CreateListLCD();
	CreateListBoundImport();
	CreateListDelayImport();
	CreateListCOM();
}

void CViewRightTL::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	//Check m_pChildFrame to prevent some UB.
	//OnUpdate can be invoked before OnInitialUpdate, weird MFC.
	if (!m_pChildFrame || LOWORD(lHint) == IDC_SHOW_RESOURCE_RBR || LOWORD(lHint) == ID_DOC_EDITMODE)
		return;

	if (m_pwndActive)
		m_pwndActive->ShowWindow(SW_HIDE);

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
		m_listDOSHeader->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listDOSHeader;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_RICHHEADER:
		m_listRichHdr->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listRichHdr;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_NTHEADER:
		m_listNTHeader->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listNTHeader;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_FILEHEADER:
		m_listFileHeader->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listFileHeader;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_OPTIONALHEADER:
		m_listOptHeader->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listOptHeader;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_DATADIRECTORIES:
		m_listDataDirs->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listDataDirs;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_SECHEADERS:
		m_listSecHeaders->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listSecHeaders;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_EXPORT:
		m_listExportDir->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listExportDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_IAT:
	case IDC_LIST_IMPORT:
		m_listImport->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listImport;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_TREE_RESOURCE:
		m_treeResTop.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &m_treeResTop;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_EXCEPTIONS:
		m_listExceptionDir->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listExceptionDir;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	case IDC_LIST_SECURITY:
		m_listSecurityDir->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listSecurityDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_RELOCATIONS:
		m_listRelocDir->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listRelocDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_DEBUG:
		m_listDebugDir->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listDebugDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_TLS:
		m_listTLSDir->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listTLSDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_LOADCONFIG:
		m_listLCD->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listLCD;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_BOUNDIMPORT:
		m_listBoundImportDir->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listBoundImportDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_DELAYIMPORT:
		m_listDelayImportDir->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listDelayImportDir;
		m_pChildFrame->m_stSplitterRight.ShowRow(1);
		break;
	case IDC_LIST_COMDESCRIPTOR:
		m_listCOMDir->SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pwndActive = &*m_listCOMDir;
		m_pChildFrame->m_stSplitterRight.HideRow(1);
		break;
	}
	m_pChildFrame->m_stSplitterRight.RecalcLayout();
}

void CViewRightTL::OnDraw(CDC* pDC)
{
	//Printing app name/version info and
	//currently oppened file's type and name.
	if (!m_fFileSummaryShow)
		return;

	CMemDC memDC(*pDC, this);
	CDC& rDC = memDC.GetDC();

	CRect rc;
	rDC.GetClipBox(rc);
	rDC.FillSolidRect(rc, RGB(255, 255, 255));
	rDC.SelectObject(m_fontSummary);
	SIZE sizeTextToDraw;
	if (m_wstrPepperVersion.size() > m_wstrFullPath.size())
		GetTextExtentPoint32W(rDC.m_hDC, m_wstrPepperVersion.data(), static_cast<int>(m_wstrPepperVersion.size()), &sizeTextToDraw);
	else
		GetTextExtentPoint32W(rDC.m_hDC, m_wstrFullPath.data(), static_cast<int>(m_wstrFullPath.size()), &sizeTextToDraw);
	int iRectLeft = 20;
	int iRectTop = 20;
	rc.SetRect(iRectLeft, iRectTop, iRectLeft + sizeTextToDraw.cx + 40, sizeTextToDraw.cy * 6);
	rDC.Rectangle(&rc);

	rDC.SetTextColor(RGB(200, 50, 30));
	GetTextExtentPoint32W(rDC.m_hDC, m_wstrPepperVersion.data(), static_cast<int>(m_wstrPepperVersion.size()), &sizeTextToDraw);
	ExtTextOutW(rDC.m_hDC, (rc.Width() - sizeTextToDraw.cx) / 2 + rc.left, 10, 0, nullptr,
		m_wstrPepperVersion.data(), static_cast<int>(m_wstrPepperVersion.size()), nullptr);

	rDC.SetTextColor(RGB(0, 0, 255));
	ExtTextOutW(rDC.m_hDC, iRectLeft + 15, rc.top + sizeTextToDraw.cy, 0, nullptr,
		m_wstrFileName.data(), static_cast<int>(m_wstrFileName.size()), nullptr);
	ExtTextOutW(rDC.m_hDC, iRectLeft + 15, rc.top + 2 * sizeTextToDraw.cy, 0, nullptr,
		m_wstrFileType.data(), static_cast<int>(m_wstrFileType.size()), nullptr);
	ExtTextOutW(rDC.m_hDC, iRectLeft + 15, rc.top + 3 * sizeTextToDraw.cy, 0, nullptr,
		m_wstrFullPath.data(), static_cast<int>(m_wstrFullPath.size()), nullptr);
}

BOOL CViewRightTL::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	CView::OnNotify(wParam, lParam, pResult);

	const auto pNMI = reinterpret_cast<LPNMITEMACTIVATE>(lParam);

	//Menu for lists.
	if (pNMI->hdr.code == NM_RCLICK && (
		pNMI->hdr.idFrom == IDC_LIST_EXPORT || pNMI->hdr.idFrom == IDC_LIST_IMPORT
		|| pNMI->hdr.idFrom == IDC_LIST_IAT || pNMI->hdr.idFrom == IDC_LIST_TLS
		|| pNMI->hdr.idFrom == IDC_LIST_BOUNDIMPORT || pNMI->hdr.idFrom == IDC_LIST_COMDESCRIPTOR)
		)
	{
		m_iListID = pNMI->hdr.idFrom;
		m_iListItem = pNMI->iItem;
		m_iListSubItem = pNMI->iSubItem;
		POINT pt { };
		GetCursorPos(&pt);
		m_menuList.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);

		return TRUE;
	}

	switch (pNMI->hdr.idFrom)
	{
	case IDC_LIST_DOSHEADER:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_DOSHEADER_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_RICHHEADER:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_RICHHEADER_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_NTHEADER:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_NTHEADER_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_FILEHEADER:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_FILEHEADER_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_OPTIONALHEADER:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_OPTIONALHEADER_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_DATADIRECTORIES:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_DATADIRECTORIES_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_SECHEADERS:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_SECHEADERS_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_IMPORT:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_IMPORT_ENTRY, pNMI->iItem));
		else if (pNMI->hdr.code == LVN_COLUMNCLICK)
			SortImportData();
		break;
	case IDC_LIST_EXCEPTIONS:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_EXCEPTION_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_DEBUG:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_DEBUG_ENTRY, pNMI->iItem));
		break;
	case IDC_LIST_ARCHITECTURE:
	case IDC_LIST_GLOBALPTR:
		break;
	case IDC_LIST_LOADCONFIG:
		if (pNMI->hdr.code == LVN_ITEMCHANGED || pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_LOADCONFIG_ENTRY, pNMI->iItem));
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
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_LIST_DELAYIMPORT_ENTRY, pNMI->iItem));
		break;
	}

	return TRUE;
}

BOOL CViewRightTL::OnCommand(WPARAM wParam, LPARAM lParam)
{
	const auto wMenuID = LOWORD(wParam);
	switch (m_iListID)
	{
	case IDC_LIST_EXPORT:
		OnListExportMenuSelect(wMenuID);
		break;
	case IDC_LIST_IMPORT:
	case IDC_LIST_IAT:
		OnListImportMenuSelect(wMenuID);
		break;
	case IDC_LIST_TLS:
		OnListTLSMenuSelect(wMenuID);
		break;
	case IDC_LIST_BOUNDIMPORT:
		OnListBoundImpMenuSelect(wMenuID);
		break;
	case IDC_LIST_COMDESCRIPTOR:
		OnListCOMDescMenuSelect(wMenuID);
		break;
	default:
		break;
	}

	return CView::OnCommand(wParam, lParam);
}

BOOL CViewRightTL::OnEraseBkgnd(CDC* /*pDC*/)
{
	return FALSE;
}

void CViewRightTL::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_pwndActive)
		m_pwndActive->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightTL::OnListSectionsGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	auto* pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	auto* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).dwOffsetSecHdrDesc);
			break;
		case 1:
		{
			auto& rstr = m_pSecHeaders->at(pItem->iItem);
			if (rstr.strSecName.empty())
				swprintf_s(pItem->pszText, pItem->cchTextMax, L"%.8S", rstr.stSecHdr.Name);
			else
				swprintf_s(pItem->pszText, pItem->cchTextMax, L"%.8S (%S)", rstr.stSecHdr.Name,
					m_pSecHeaders->at(pItem->iItem).strSecName.data());
		}
		break;
		case 2:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.Misc.VirtualSize);
			break;
		case 3:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.VirtualAddress);
			break;
		case 4:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.SizeOfRawData);
			break;
		case 5:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.PointerToRawData);
			break;
		case 6:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.PointerToRelocations);
			break;
		case 7:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.PointerToLinenumbers);
			break;
		case 8:
			swprintf_s(pItem->pszText, 5, L"%04X", m_pSecHeaders->at(pItem->iItem).stSecHdr.NumberOfRelocations);
			break;
		case 9:
			swprintf_s(pItem->pszText, 5, L"%04X", m_pSecHeaders->at(pItem->iItem).stSecHdr.NumberOfLinenumbers);
			break;
		case 10:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pSecHeaders->at(pItem->iItem).stSecHdr.Characteristics);
			break;
		}
	}
}

void CViewRightTL::OnListImportGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	auto *pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	auto* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		const IMAGE_IMPORT_DESCRIPTOR* pImpDesc = &m_pImport->at(pItem->iItem).stImportDesc;
		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pImport->at(pItem->iItem).dwOffsetImpDesc);
			break;
		case 1:
			swprintf_s(pItem->pszText, pItem->cchTextMax, L"%S (%zu)", m_pImport->at(pItem->iItem).strModuleName.data(),
				m_pImport->at(pItem->iItem).vecImportFunc.size());
			break;
		case 2:
			swprintf_s(pItem->pszText, 9, L"%08X", pImpDesc->OriginalFirstThunk);
			break;
		case 3:
			swprintf_s(pItem->pszText, 9, L"%08X", pImpDesc->TimeDateStamp);
			break;
		case 4:
			swprintf_s(pItem->pszText, 9, L"%08X", pImpDesc->ForwarderChain);
			break;
		case 5:
			swprintf_s(pItem->pszText, 9, L"%08X", pImpDesc->Name);
			break;
		case 6:
			swprintf_s(pItem->pszText, 9, L"%08X", pImpDesc->FirstThunk);
			break;
		}
	}
}

void CViewRightTL::OnListRelocsGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	auto *pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	auto* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		const IMAGE_BASE_RELOCATION* pReloc = &m_pRelocTable->at(pItem->iItem).stBaseReloc;
		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pRelocTable->at(pItem->iItem).dwOffsetReloc);
			break;
		case 1:
			swprintf_s(pItem->pszText, 9, L"%08X", pReloc->VirtualAddress);
			break;
		case 2:
			swprintf_s(pItem->pszText, 9, L"%08X", pReloc->SizeOfBlock);
			break;
		case 3:
			swprintf_s(pItem->pszText, pItem->cchTextMax, L"%zu", (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD));
			break;
		}
	}
}

void CViewRightTL::OnListExceptionsGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	auto *pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	auto* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).dwOffsetRuntimeFuncDesc);
			break;
		case 1:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).stRuntimeFuncEntry.BeginAddress);
			break;
		case 2:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).stRuntimeFuncEntry.EndAddress);
			break;
		case 3:
			swprintf_s(pItem->pszText, 9, L"%08X", m_pExceptionDir->at(pItem->iItem).stRuntimeFuncEntry.UnwindData);
			break;
		}
	}
}

void CViewRightTL::OnListSecHdrGetToolTip(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	const auto pNMI = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (pNMI->iSubItem != 10)
		return;

	static const std::map<DWORD, std::wstring> mapSecFlags {
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

	std::wstring wstrTipText;
	for (auto& flags : mapSecFlags)
		if (flags.first & m_pSecHeaders->at(pNMI->iItem).stSecHdr.Characteristics)
			wstrTipText += flags.second + L"\n";

	if (!wstrTipText.empty())
	{
		static LISTEXTOOLTIP stTT { { }, L"Section Flags:" };
		stTT.wstrText = std::move(wstrTipText);
		pNMI->lParam = reinterpret_cast<LPARAM>(&stTT); //Tooltip pointer.
	}
}

void CViewRightTL::OnListExportMenuSelect(WORD wMenuID)
{
	bool fx32 = ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE32);
	bool fx64 = ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE64);
	DWORD dwOffset { }, dwSize = 0;

	PLIBPE_EXPORT pExport;
	if (m_pLibpe->GetExport(pExport) != S_OK)
		return;

	switch (wMenuID)
	{
	case IDM_LIST_GOTODESCOFFSET:
	{
		dwOffset = pExport->dwOffsetExportDesc;
		dwSize = sizeof(IMAGE_EXPORT_DIRECTORY);
	}
	break;
	case IDM_LIST_GOTODATAOFFSET:
	{
		switch (m_iListItem)
		{
		case 4: //Name
			m_pLibpe->GetOffsetFromRVA(pExport->stExportDesc.Name, dwOffset);
			dwSize = static_cast<DWORD>(pExport->strModuleName.size());
			break;
		case 8: //AddressOfFunctions
			m_pLibpe->GetOffsetFromRVA(pExport->stExportDesc.AddressOfFunctions, dwOffset);
			dwSize = 1;
			break;
		case 9: //AddressOfNames
			m_pLibpe->GetOffsetFromRVA(pExport->stExportDesc.AddressOfNames, dwOffset);
			dwSize = 1;
			break;
		case 10: //AddressOfOrdinals
			m_pLibpe->GetOffsetFromRVA(pExport->stExportDesc.AddressOfNameOrdinals, dwOffset);
			dwSize = 1;
			break;
		}
	}
	break;
	}

	if (dwSize)
		m_pFileLoader->ShowOffset(dwOffset, dwSize);
}

void CViewRightTL::OnListImportMenuSelect(WORD wMenuID)
{
	bool fx32 = ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE32);
	bool fx64 = ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE64);
	DWORD dwOffset { }, dwSize = 0;

	switch (wMenuID)
	{
	case IDM_LIST_GOTODESCOFFSET:
		dwOffset = m_pImport->at(m_iListItem).dwOffsetImpDesc;
		dwSize = sizeof(IMAGE_IMPORT_DESCRIPTOR);
		break;
	case IDM_LIST_GOTODATAOFFSET:
		switch (m_iListSubItem)
		{
		case 1: //Str dll name
		case 5: //Name
			m_pLibpe->GetOffsetFromRVA(m_pImport->at(m_iListItem).stImportDesc.Name, dwOffset);
			dwSize = static_cast<DWORD>(m_pImport->at(m_iListItem).strModuleName.size());
			break; ;
		case 2: //OriginalFirstThunk 
			m_pLibpe->GetOffsetFromRVA(m_pImport->at(m_iListItem).stImportDesc.OriginalFirstThunk, dwOffset);
			if (fx32)
				dwSize = sizeof(IMAGE_THUNK_DATA32);
			else if (fx64)
				dwSize = sizeof(IMAGE_THUNK_DATA64);
			break;
		case 3: //TimeDateStamp
			break;
		case 4: //ForwarderChain
			m_pLibpe->GetOffsetFromRVA(m_pImport->at(m_iListItem).stImportDesc.ForwarderChain, dwOffset);
			break;
		case 6: //FirstThunk
			m_pLibpe->GetOffsetFromRVA(m_pImport->at(m_iListItem).stImportDesc.FirstThunk, dwOffset);
			if (fx32)
				dwSize = sizeof(IMAGE_THUNK_DATA32);
			else if (fx64)
				dwSize = sizeof(IMAGE_THUNK_DATA64);
			break;
		}
		break;
	}

	if (dwSize)
		m_pFileLoader->ShowOffset(dwOffset, dwSize);
}

void CViewRightTL::OnListTLSMenuSelect(WORD wMenuID)
{
	bool fx32 = ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE32);
	bool fx64 = ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE64);
	DWORD dwOffset { }, dwSize = 0;

	PLIBPE_TLS pTLSDir;
	if (m_pLibpe->GetTLS(pTLSDir) != S_OK)
		return;

	switch (wMenuID)
	{
	case IDM_LIST_GOTODESCOFFSET:
		dwOffset = pTLSDir->dwOffsetTLS;
		if (fx32)
			dwSize = sizeof(IMAGE_TLS_DIRECTORY32);
		else if (fx64)
			dwSize = sizeof(IMAGE_TLS_DIRECTORY64);
		break;
	case IDM_LIST_GOTODATAOFFSET:
	{
		PLIBPE_OPTHEADER_VAR pOpt;
		if (m_pLibpe->GetOptionalHeader(pOpt) != S_OK)
			return;
		dwSize = 1;

		if (fx32)
		{
			const IMAGE_TLS_DIRECTORY32* pTLSDir32 = &pTLSDir->varTLS.stTLSDir32;

			switch (m_iListItem)
			{
			case 0: //StartAddressOfRawData
				m_pLibpe->GetOffsetFromVA(pTLSDir32->StartAddressOfRawData, dwOffset);
				break;
			case 2: //AddressOfIndex
				m_pLibpe->GetOffsetFromVA(pTLSDir32->AddressOfIndex, dwOffset);
				break;
			case 3: //AddressOfCallBacks
				m_pLibpe->GetOffsetFromVA(pTLSDir32->AddressOfCallBacks, dwOffset);
				break;
			default:
				dwSize = 0; //To not process other fields.
			}
		}
		else if (fx64)
		{
			const IMAGE_TLS_DIRECTORY64* pTLSDir64 = &pTLSDir->varTLS.stTLSDir64;

			switch (m_iListItem)
			{
			case 0: //StartAddressOfRawData
				m_pLibpe->GetOffsetFromRVA(pTLSDir64->StartAddressOfRawData, dwOffset);
				break;
			case 2: //AddressOfIndex
				m_pLibpe->GetOffsetFromRVA(pTLSDir64->AddressOfIndex, dwOffset);
				break;
			case 3: //AddressOfCallBacks
				m_pLibpe->GetOffsetFromRVA(pTLSDir64->AddressOfCallBacks, dwOffset);
				break;
			default:
				dwSize = 0; //To not process other fields.
			}
		}
	}
	break;
	}

	if (dwSize)
		m_pFileLoader->ShowOffset(dwOffset, dwSize);
}

void CViewRightTL::OnListBoundImpMenuSelect(WORD wMenuID)
{
	DWORD dwOffset { }, dwSize = 0;

	PLIBPE_BOUNDIMPORT_VEC pBoundImp;
	if (m_pLibpe->GetBoundImport(pBoundImp) != S_OK)
		return;

	switch (wMenuID)
	{
	case IDM_LIST_GOTODESCOFFSET:
	{
		dwOffset = pBoundImp->at(m_iListItem).dwOffsetBoundImpDesc;
		dwSize = sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR);
	}
	break;
	case IDM_LIST_GOTODATAOFFSET:
	{
		switch (m_iListSubItem)
		{
		case 3: //OffsetModuleName
			dwOffset = m_pLibpe->GetOffsetFromRVA(pBoundImp->at(m_iListItem).stBoundImpDesc.OffsetModuleName, dwOffset);
			dwSize = static_cast<DWORD>(pBoundImp->at(m_iListItem).strBoundName.size());
			break;
		}
	}
	break;
	}

	if (dwSize)
		m_pFileLoader->ShowOffset(dwOffset, dwSize);
}

void CViewRightTL::OnListCOMDescMenuSelect(WORD wMenuID)
{
	DWORD dwOffset { }, dwSize = 0;

	PLIBPE_COMDESCRIPTOR pCOMDesc;
	if (m_pLibpe->GetCOMDescriptor(pCOMDesc) != S_OK)
		return;

	switch (wMenuID)
	{
	case IDM_LIST_GOTODESCOFFSET:
	{
		dwOffset = pCOMDesc->dwOffsetComDesc;
		dwSize = sizeof(IMAGE_COR20_HEADER);
	}
	break;
	case IDM_LIST_GOTODATAOFFSET:
		//TODO: IDC_LIST_COMDESCRIPTOR->IDM_LIST_GOTODATAOFFSET.
		break;
	}

	if (dwSize)
		m_pFileLoader->ShowOffset(dwOffset, dwSize);
}

void CViewRightTL::OnTreeResTopSelChange(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	const auto pTree = reinterpret_cast<LPNMTREEVIEWW>(pNMHDR);
	if (pTree->itemNew.hItem == m_hTreeResDir)
		return;

	PLIBPE_RESOURCE_ROOT pResRoot;
	if (m_pLibpe->GetResources(pResRoot) != S_OK)
		return;

	const auto& [idlvlRoot, idlvl2, idlvl3] = m_vecResId.at(m_treeResTop.GetItemData(pTree->itemNew.hItem));
	if (idlvl2 >= 0)
	{
		auto& lvl2st = pResRoot->vecResRoot.at(idlvlRoot).stResLvL2;
		auto& lvl2vec = lvl2st.vecResLvL2;

		if (!lvl2vec.empty())
		{
			if (idlvl3 >= 0)
			{
				auto& lvl3st = lvl2vec.at(idlvl2).stResLvL3;
				auto& lvl3vec = lvl3st.vecResLvL3;

				if (!lvl3vec.empty())
				{
					auto data = &lvl3vec.at(idlvl3).stResDataEntryLvL3;
					//Send data pointer to CViewRightTR to display raw data.
					m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_HEX_RIGHT_TR, 0), reinterpret_cast<CObject*>(data));
				}
			}
		}
	}
}

int CViewRightTL::CreateListDOSHeader()
{
	PLIBPE_DOSHEADER pDosHeader;
	if (m_pLibpe->GetMSDOSHeader(pDosHeader) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_DOSHEADER;
	m_listDOSHeader->Create(m_stlcs);
	m_listDOSHeader->ShowWindow(SW_HIDE);
	m_listDOSHeader->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listDOSHeader->SetColumn(0, &stCol);
	m_listDOSHeader->SetHdrColumnColor(0, g_clrOffset);
	m_listDOSHeader->InsertColumn(1, L"Name", LVCFMT_CENTER, 150);
	m_listDOSHeader->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listDOSHeader->InsertColumn(3, L"Value", LVCFMT_CENTER, 100);

	m_dwPeStart = pDosHeader->e_lfanew;

	for (unsigned i = 0; i < g_mapDOSHeader.size(); i++)
	{
		WCHAR wstr[18];
		auto& ref = g_mapDOSHeader.at(i);
		DWORD dwOffset = ref.dwOffset;
		DWORD dwSize = ref.dwSize;

		//Get a pointer to an offset and then take only needed amount of bytes (by &...).
		DWORD dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(pDosHeader) + dwOffset)) &
			(DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));
		if (i == 0)
			dwValue = (dwValue & 0xFF00) >> 8 | (dwValue & 0xFF) << 8;

		swprintf_s(wstr, 9, L"%08lX", dwOffset);
		m_listDOSHeader->InsertItem(i, wstr);
		m_listDOSHeader->SetItemText(i, 1, ref.wstrName.data());
		swprintf_s(wstr, 17, L"%lu", dwSize);
		m_listDOSHeader->SetItemText(i, 2, wstr);
		swprintf_s(wstr, 9, dwSize == 2 ? L"%04X" : L"%08X", dwValue);
		m_listDOSHeader->SetItemText(i, 3, wstr);
	}

	return 0;
}

int CViewRightTL::CreateListRichHeader()
{
	PLIBPE_RICHHEADER_VEC pRichHeader;
	if (m_pLibpe->GetRichHeader(pRichHeader) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_RICHHEADER;
	m_listRichHdr->Create(m_stlcs);
	m_listRichHdr->ShowWindow(SW_HIDE);
	m_listRichHdr->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listRichHdr->SetColumn(0, &stCol);
	m_listRichHdr->SetHdrColumnColor(0, g_clrOffset);
	m_listRichHdr->InsertColumn(1, L"\u2116", LVCFMT_CENTER, 35);
	m_listRichHdr->InsertColumn(2, L"ID [Hex]", LVCFMT_CENTER, 100);
	m_listRichHdr->InsertColumn(3, L"Version", LVCFMT_CENTER, 100);
	m_listRichHdr->InsertColumn(4, L"Occurrences", LVCFMT_CENTER, 100);
	m_listRichHdr->SetColumnSortMode(1, true, EListExSortMode::SORT_NUMERIC);
	m_listRichHdr->SetColumnSortMode(4, true, EListExSortMode::SORT_NUMERIC);

	WCHAR wstr[18];
	int listindex = 0;
	for (const auto& i : *pRichHeader)
	{
		swprintf_s(wstr, 9, L"%08X", i.dwOffsetRich);
		m_listRichHdr->InsertItem(listindex, wstr);
		swprintf_s(wstr, 17, L"%i", listindex + 1);
		m_listRichHdr->SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 17, L"%04X", i.wId);
		m_listRichHdr->SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 17, L"%u", i.wVersion);
		m_listRichHdr->SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, 17, L"%u", i.dwCount);
		m_listRichHdr->SetItemText(listindex, 4, wstr);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListNTHeader()
{
	PLIBPE_NTHEADER pNTHdr;
	if (m_pLibpe->GetNTHeader(pNTHdr) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_NTHEADER;
	m_listNTHeader->Create(m_stlcs);
	m_listNTHeader->ShowWindow(SW_HIDE);
	m_listNTHeader->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listNTHeader->SetColumn(0, &stCol);
	m_listNTHeader->SetHdrColumnColor(0, g_clrOffset);
	m_listNTHeader->InsertColumn(1, L"Name", LVCFMT_CENTER, 100);
	m_listNTHeader->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listNTHeader->InsertColumn(3, L"Value", LVCFMT_CENTER, 100);

	WCHAR wstr[9];
	UINT listindex = 0;
	if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE32))
	{
		const IMAGE_NT_HEADERS32* pNTHeader32 = &pNTHdr->varHdr.stNTHdr32;

		swprintf_s(wstr, 9, L"%08zX", offsetof(IMAGE_NT_HEADERS32, Signature) + m_dwPeStart);
		listindex = m_listNTHeader->InsertItem(listindex, wstr);
		m_listNTHeader->SetItemText(listindex, 1, L"Signature");
		swprintf_s(wstr, 2, L"%zX", sizeof(pNTHeader32->Signature));
		m_listNTHeader->SetItemText(listindex, 2, wstr);
		swprintf_s(&wstr[0], 3, L"%02X", static_cast<BYTE>(pNTHeader32->Signature >> 0));
		swprintf_s(&wstr[2], 3, L"%02X", static_cast<BYTE>(pNTHeader32->Signature >> 8));
		swprintf_s(&wstr[4], 3, L"%02X", static_cast<BYTE>(pNTHeader32->Signature >> 16));
		swprintf_s(&wstr[6], 3, L"%02X", static_cast<BYTE>(pNTHeader32->Signature >> 24));
		m_listNTHeader->SetItemText(listindex, 3, wstr);
	}
	else if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE64))
	{
		const IMAGE_NT_HEADERS64* pNTHeader64 = &pNTHdr->varHdr.stNTHdr64;

		swprintf_s(wstr, 9, L"%08zX", offsetof(IMAGE_NT_HEADERS64, Signature) + m_dwPeStart);
		listindex = m_listNTHeader->InsertItem(listindex, wstr);
		m_listNTHeader->SetItemText(listindex, 1, L"Signature");
		swprintf_s(wstr, 2, L"%zX", sizeof(pNTHeader64->Signature));
		m_listNTHeader->SetItemText(listindex, 2, wstr);
		swprintf_s(&wstr[0], 3, L"%02X", static_cast<BYTE>(pNTHeader64->Signature >> 0));
		swprintf_s(&wstr[2], 3, L"%02X", static_cast<BYTE>(pNTHeader64->Signature >> 8));
		swprintf_s(&wstr[4], 3, L"%02X", static_cast<BYTE>(pNTHeader64->Signature >> 16));
		swprintf_s(&wstr[6], 3, L"%02X", static_cast<BYTE>(pNTHeader64->Signature >> 24));
		m_listNTHeader->SetItemText(listindex, 3, wstr);
	}

	return 0;
}

int CViewRightTL::CreateListFileHeader()
{
	PLIBPE_FILEHEADER pFileHeader;
	if (m_pLibpe->GetFileHeader(pFileHeader) != S_OK)
		return -1;

	PLIBPE_NTHEADER pNTHdr;
	if (m_pLibpe->GetNTHeader(pNTHdr) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_FILEHEADER;
	m_listFileHeader->Create(m_stlcs);
	m_listFileHeader->ShowWindow(SW_HIDE);
	m_listFileHeader->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listFileHeader->SetColumn(0, &stCol);
	m_listFileHeader->SetHdrColumnColor(0, g_clrOffset);
	m_listFileHeader->InsertColumn(1, L"Name", LVCFMT_CENTER, 200);
	m_listFileHeader->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listFileHeader->InsertColumn(3, L"Value", LVCFMT_CENTER, 300);

	const std::map<WORD, std::wstring> mapMachineType {
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_UNKNOWN),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_TARGET_HOST),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_I386),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_R3000),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_R4000),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_R10000),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_WCEMIPSV2),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_ALPHA),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_SH3),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_SH3DSP),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_SH3E),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_SH4),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_SH5),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_ARM),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_THUMB),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_ARMNT),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_AM33),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_POWERPC),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_POWERPCFP),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_IA64),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_MIPS16),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_ALPHA64),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_MIPSFPU),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_MIPSFPU16),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_TRICORE),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_CEF),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_EBC),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_AMD64),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_M32R),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_ARM64),
		TO_WSTR_MAP(IMAGE_FILE_MACHINE_CEE),
	};
	const std::map<WORD, std::wstring> mapCharacteristics {
		TO_WSTR_MAP(IMAGE_FILE_RELOCS_STRIPPED),
		TO_WSTR_MAP(IMAGE_FILE_EXECUTABLE_IMAGE),
		TO_WSTR_MAP(IMAGE_FILE_LINE_NUMS_STRIPPED),
		TO_WSTR_MAP(IMAGE_FILE_LOCAL_SYMS_STRIPPED),
		TO_WSTR_MAP(IMAGE_FILE_AGGRESIVE_WS_TRIM),
		TO_WSTR_MAP(IMAGE_FILE_LARGE_ADDRESS_AWARE),
		TO_WSTR_MAP(IMAGE_FILE_BYTES_REVERSED_LO),
		TO_WSTR_MAP(IMAGE_FILE_32BIT_MACHINE),
		TO_WSTR_MAP(IMAGE_FILE_DEBUG_STRIPPED),
		TO_WSTR_MAP(IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP),
		TO_WSTR_MAP(IMAGE_FILE_NET_RUN_FROM_SWAP),
		TO_WSTR_MAP(IMAGE_FILE_SYSTEM),
		TO_WSTR_MAP(IMAGE_FILE_DLL),
		TO_WSTR_MAP(IMAGE_FILE_UP_SYSTEM_ONLY),
		TO_WSTR_MAP(IMAGE_FILE_BYTES_REVERSED_HI)
	};

	for (unsigned i = 0; i < g_mapFileHeader.size(); i++)
	{
		WCHAR wstr[350];
		auto& ref = g_mapFileHeader.at(i);
		DWORD dwOffset = ref.dwOffset;
		DWORD dwSize = ref.dwSize;
		DWORD dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(&pNTHdr->varHdr.stNTHdr32.FileHeader) + dwOffset)) &
			(DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

		swprintf_s(wstr, 9, L"%08zX", pNTHdr->dwOffsetNTHdrDesc + offsetof(IMAGE_NT_HEADERS32, FileHeader) + dwOffset);
		m_listFileHeader->InsertItem(i, wstr);
		m_listFileHeader->SetItemText(i, 1, ref.wstrName.data());
		swprintf_s(wstr, 17, L"%lu", dwSize);
		m_listFileHeader->SetItemText(i, 2, wstr);
		swprintf_s(wstr, 9, dwSize == 2 ? L"%04X" : L"%08X", dwValue);
		m_listFileHeader->SetItemText(i, 3, wstr);

		if (i == 0) { //Machine
			auto iterMachine = mapMachineType.find(pFileHeader->Machine);
			if (iterMachine != mapMachineType.end())
				m_listFileHeader->SetCellTooltip(i, 3, iterMachine->second.data(), L"Machine:");
		}
		else if (i == 2) { //TimeDateStamp	
			if (pFileHeader->TimeDateStamp) {
				__time64_t time = pFileHeader->TimeDateStamp;
				_wctime64_s(wstr, MAX_PATH, &time);
				m_listFileHeader->SetCellTooltip(i, 3, wstr, L"Time / Date:");
			}
		}
		else if (i == 6) //Characteristics
		{
			std::wstring  wstrCharact;
			for (auto& iter : mapCharacteristics)
				if (iter.first & pFileHeader->Characteristics)
					wstrCharact += iter.second + L"\n";
			if (!wstrCharact.empty()) {
				wstrCharact.erase(wstrCharact.size() - 1); //to remove last '\n'
				m_listFileHeader->SetCellTooltip(i, 3, wstrCharact.data(), L"Characteristics:");
			}
		}
	}

	return 0;
}

int CViewRightTL::CreateListOptHeader()
{
	PLIBPE_OPTHEADER_VAR pOptHdr;
	if (m_pLibpe->GetOptionalHeader(pOptHdr) != S_OK)
		return -1;

	PLIBPE_NTHEADER pNTHdr;
	if (m_pLibpe->GetNTHeader(pNTHdr) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_OPTIONALHEADER;
	m_listOptHeader->Create(m_stlcs);
	m_listOptHeader->ShowWindow(SW_HIDE);
	m_listOptHeader->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listOptHeader->SetColumn(0, &stCol);
	m_listOptHeader->SetHdrColumnColor(0, g_clrOffset);
	m_listOptHeader->InsertColumn(1, L"Name", LVCFMT_CENTER, 215);
	m_listOptHeader->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listOptHeader->InsertColumn(3, L"Value", LVCFMT_CENTER, 140);

	const std::map<WORD, std::wstring> mapSubSystem {
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_UNKNOWN),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_NATIVE),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_WINDOWS_GUI),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_WINDOWS_CUI),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_OS2_CUI),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_POSIX_CUI),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_NATIVE_WINDOWS),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_WINDOWS_CE_GUI),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_EFI_APPLICATION),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_EFI_ROM),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_XBOX),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION),
		TO_WSTR_MAP(IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG)
	};

	const std::map<WORD, std::wstring> mapMagic {
		TO_WSTR_MAP(IMAGE_NT_OPTIONAL_HDR32_MAGIC),
		TO_WSTR_MAP(IMAGE_NT_OPTIONAL_HDR64_MAGIC),
		TO_WSTR_MAP(IMAGE_ROM_OPTIONAL_HDR_MAGIC)
	};

	const std::map<WORD, std::wstring> mapDllCharacteristics {
		{ 0x0001, L"IMAGE_LIBRARY_PROCESS_INIT" },
		{ 0x0002, L"IMAGE_LIBRARY_PROCESS_TERM" },
		{ 0x0004, L"IMAGE_LIBRARY_THREAD_INIT" },
		{ 0x0008, L"IMAGE_LIBRARY_THREAD_TERM" },
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_NX_COMPAT),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_NO_ISOLATION),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_NO_SEH),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_NO_BIND),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_APPCONTAINER),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_WDM_DRIVER),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_GUARD_CF),
		TO_WSTR_MAP(IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE)
	};

	WCHAR wstr[18];
	std::wstring wstrTooltip;
	if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE32))
	{
		for (unsigned i = 0; i < g_mapOptHeader32.size(); i++)
		{
			const IMAGE_OPTIONAL_HEADER32* pOptHdr32 = &pOptHdr->stOptHdr32;
			auto& ref = g_mapOptHeader32.at(i);
			DWORD dwOffset = ref.dwOffset;
			DWORD dwSize = ref.dwSize;
			DWORD dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(pOptHdr32) + dwOffset)) &
				(DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

			swprintf_s(wstr, 9, L"%08zX", pNTHdr->dwOffsetNTHdrDesc + offsetof(IMAGE_NT_HEADERS32, OptionalHeader) + dwOffset);
			m_listOptHeader->InsertItem(i, wstr);
			m_listOptHeader->SetItemText(i, 1, ref.wstrName.data());
			swprintf_s(wstr, 17, L"%lu", dwSize);
			m_listOptHeader->SetItemText(i, 2, wstr);
			swprintf_s(wstr, 9, dwSize == 1 ? L"%02X" : (dwSize == 2 ? L"%04X" : L"%08X"), dwValue);
			m_listOptHeader->SetItemText(i, 3, wstr);

			if (i == 0) //TimeDateStamp
			{
				auto it = mapMagic.find(pOptHdr32->Magic);
				if (it != mapMagic.end())
					m_listOptHeader->SetCellTooltip(i, 3, it->second.data(), L"Magic:");
			}
			else if (i == 22) //Subsystem
			{
				auto it = mapSubSystem.find(pOptHdr32->Subsystem);
				if (it != mapSubSystem.end())
					m_listOptHeader->SetCellTooltip(i, 3, it->second.data(), L"Subsystem:");
			}
			else if (i == 23) //Dllcharacteristics
			{
				for (auto& iter : mapDllCharacteristics) {
					if (iter.first & pOptHdr32->DllCharacteristics)
						wstrTooltip += iter.second + L"\n";
				}
				if (!wstrTooltip.empty()) {
					wstrTooltip.erase(wstrTooltip.size() - 1); //to remove last '\n'
					m_listOptHeader->SetCellTooltip(i, 3, wstrTooltip.data(), L"DllCharacteristics:");
				}
			}
		}
	}
	else if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE64))
	{
		for (unsigned i = 0; i < g_mapOptHeader64.size(); i++)
		{
			const IMAGE_OPTIONAL_HEADER64* pOptHdr64 = &pOptHdr->stOptHdr64;
			auto& ref = g_mapOptHeader64.at(i);
			DWORD dwOffset = ref.dwOffset;
			DWORD dwSize = ref.dwSize;
			ULONGLONG ullValue = *(reinterpret_cast<PULONGLONG>(reinterpret_cast<DWORD_PTR>(pOptHdr64) + dwOffset)) &
				(ULONGLONG_MAX >> ((sizeof(ULONGLONG) - dwSize) * 8));

			swprintf_s(wstr, 9, L"%08zX", pNTHdr->dwOffsetNTHdrDesc + offsetof(IMAGE_NT_HEADERS64, OptionalHeader) + dwOffset);
			m_listOptHeader->InsertItem(i, wstr);
			m_listOptHeader->SetItemText(i, 1, ref.wstrName.data());
			swprintf_s(wstr, 17, L"%lu", dwSize);
			m_listOptHeader->SetItemText(i, 2, wstr);
			swprintf_s(wstr, 17, dwSize == 1 ? L"%02X" : (dwSize == 2 ? L"%04X" : (dwSize == 4 ? L"%08X" : L"%016llX")), ullValue);
			m_listOptHeader->SetItemText(i, 3, wstr);

			if (i == 0) //TimeDateStamp
			{
				auto it = mapMagic.find(pOptHdr64->Magic);
				if (it != mapMagic.end())
					m_listOptHeader->SetCellTooltip(i, 3, it->second.data(), L"Magic:");
			}
			else if (i == 21) //Subsystem
			{
				auto it = mapSubSystem.find(pOptHdr64->Subsystem);
				if (it != mapSubSystem.end())
					m_listOptHeader->SetCellTooltip(i, 3, it->second.data(), L"Subsystem:");
			}
			else if (i == 22) //Dllcharacteristics
			{
				for (auto& iter : mapDllCharacteristics) {
					if (iter.first & pOptHdr64->DllCharacteristics)
						wstrTooltip += iter.second + L"\n";
				}
				if (!wstrTooltip.empty()) {
					wstrTooltip.erase(wstrTooltip.size() - 1); //to remove last '\n'
					m_listOptHeader->SetCellTooltip(i, 3, wstrTooltip.data(), L"DllCharacteristics:");
				}
			}
		}
	}

	return 0;
}

int CViewRightTL::CreateListDataDirectories()
{
	PLIBPE_DATADIRS_VEC pvecDataDirs;
	if (m_pLibpe->GetDataDirectories(pvecDataDirs) != S_OK)
		return -1;
	PLIBPE_NTHEADER pNTHdr;
	if (m_pLibpe->GetNTHeader(pNTHdr) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_DATADIRECTORIES;
	m_listDataDirs->Create(m_stlcs);
	m_listDataDirs->ShowWindow(SW_HIDE);
	m_listDataDirs->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listDataDirs->SetColumn(0, &stCol);
	m_listDataDirs->SetHdrColumnColor(0, g_clrOffset);
	m_listDataDirs->InsertColumn(1, L"Name", LVCFMT_CENTER, 200);
	m_listDataDirs->InsertColumn(2, L"Directory RVA", LVCFMT_CENTER, 100);
	m_listDataDirs->InsertColumn(3, L"Directory Size", LVCFMT_CENTER, 100);
	m_listDataDirs->InsertColumn(4, L"Resides in Section", LVCFMT_CENTER, 125);

	WCHAR wstr[9];
	DWORD dwDataDirsOffset { };
	if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE32))
		dwDataDirsOffset = offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory);
	else if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE64))
		dwDataDirsOffset = offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory);

	for (auto i = 0; i < pvecDataDirs->size(); i++)
	{
		auto ref = pvecDataDirs->at(static_cast<size_t>(i));
		auto pDataDirs = &ref.stDataDir;

		swprintf_s(wstr, 9, L"%08zX", pNTHdr->dwOffsetNTHdrDesc + dwDataDirsOffset + sizeof(IMAGE_DATA_DIRECTORY) * i);
		m_listDataDirs->InsertItem(i, wstr);
		m_listDataDirs->SetItemText(i, 1, g_mapDataDirs.at(static_cast<WORD>(i)).data());
		swprintf_s(wstr, 9, L"%08X", pDataDirs->VirtualAddress);
		m_listDataDirs->SetItemText(i, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pDataDirs->Size);
		m_listDataDirs->SetItemText(i, 3, wstr);

		if (!ref.strSecResidesIn.empty())
		{
			swprintf_s(wstr, 9, L"%.8S", ref.strSecResidesIn.data());
			m_listDataDirs->SetItemText(i, 4, wstr);
		}
		if (i == IMAGE_DIRECTORY_ENTRY_SECURITY && pDataDirs->VirtualAddress)
			m_listDataDirs->SetCellTooltip(i, 2, L"This address is the file's raw offset on disk.");
	}

	return 0;
}

int CViewRightTL::CreateListSecHeaders()
{
	if (!m_pSecHeaders)
		return -1;

	m_stlcs.dwStyle = LVS_OWNERDATA;
	m_stlcs.uID = IDC_LIST_SECHEADERS;
	m_listSecHeaders->Create(m_stlcs);
	m_listSecHeaders->ShowWindow(SW_HIDE);
	m_listSecHeaders->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listSecHeaders->SetColumn(0, &stCol);
	m_listSecHeaders->SetHdrColumnColor(0, g_clrOffset);
	m_listSecHeaders->InsertColumn(1, L"Name", LVCFMT_CENTER, 150);
	m_listSecHeaders->InsertColumn(2, L"Virtual Size", LVCFMT_CENTER, 100);
	m_listSecHeaders->InsertColumn(3, L"Virtual Address", LVCFMT_CENTER, 125);
	m_listSecHeaders->InsertColumn(4, L"SizeOfRawData", LVCFMT_CENTER, 125);
	m_listSecHeaders->InsertColumn(5, L"PointerToRawData", LVCFMT_CENTER, 125);
	m_listSecHeaders->InsertColumn(6, L"PointerToRelocations", LVCFMT_CENTER, 150);
	m_listSecHeaders->InsertColumn(7, L"PointerToLinenumbers", LVCFMT_CENTER, 160);
	m_listSecHeaders->InsertColumn(8, L"NumberOfRelocations", LVCFMT_CENTER, 150);
	m_listSecHeaders->InsertColumn(9, L"NumberOfLinenumbers", LVCFMT_CENTER, 160);
	m_listSecHeaders->InsertColumn(10, L"Characteristics", LVCFMT_CENTER, 115);
	m_listSecHeaders->SetItemCountEx(static_cast<int>(m_pSecHeaders->size()), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);

	return 0;
}

int CViewRightTL::CreateListExport()
{
	PLIBPE_EXPORT pExport;
	if (m_pLibpe->GetExport(pExport) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_EXPORT;
	m_listExportDir->Create(m_stlcs);
	m_listExportDir->ShowWindow(SW_HIDE);
	m_listExportDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listExportDir->SetColumn(0, &stCol);
	m_listExportDir->SetHdrColumnColor(0, g_clrOffset);
	m_listExportDir->InsertColumn(1, L"Name", LVCFMT_CENTER, 250);
	m_listExportDir->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listExportDir->InsertColumn(3, L"Value", LVCFMT_CENTER, 300);

	const IMAGE_EXPORT_DIRECTORY * pExportDesc = &pExport->stExportDesc;
	for (unsigned i = 0; i < g_mapExport.size(); i++)
	{
		WCHAR wstr[MAX_PATH];
		auto& ref = g_mapExport.at(i);
		DWORD dwOffset = ref.dwOffset;
		DWORD dwSize = ref.dwSize;
		DWORD dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(pExportDesc) + dwOffset)) &
			(DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

		swprintf_s(wstr, 9, L"%08lX", pExport->dwOffsetExportDesc + dwOffset);
		m_listExportDir->InsertItem(i, wstr);
		m_listExportDir->SetItemText(i, 1, ref.wstrName.data());
		swprintf_s(wstr, 17, L"%lu", dwSize);
		m_listExportDir->SetItemText(i, 2, wstr);

		if (i == 4) //Name
			swprintf_s(wstr, MAX_PATH, L"%08lX (%S)", dwValue, pExport->strModuleName.data());
		else
			swprintf_s(wstr, 9, dwSize == 2 ? L"%04X" : L"%08X", dwValue);

		m_listExportDir->SetItemText(i, 3, wstr);

		if (i == 1 && pExportDesc->TimeDateStamp)
		{
			__time64_t time = pExportDesc->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &time);
			m_listExportDir->SetCellTooltip(i, 3, wstr, L"Time / Date:");
		}
	}

	return 0;
}

int CViewRightTL::CreateListImport()
{
	if (!m_pImport)
		return -1;

	m_stlcs.dwStyle = LVS_OWNERDATA;
	m_stlcs.uID = IDC_LIST_IMPORT;
	m_listImport->Create(m_stlcs);
	m_listImport->ShowWindow(SW_HIDE);
	m_listImport->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listImport->SetColumn(0, &stCol);
	m_listImport->SetHdrColumnColor(0, g_clrOffset);
	m_listImport->InsertColumn(1, L"Module Name (funcs number)", LVCFMT_CENTER, 300);
	m_listImport->InsertColumn(2, L"OriginalFirstThunk\n(Import Lookup Table)", LVCFMT_CENTER, 170);
	m_listImport->InsertColumn(3, L"TimeDateStamp", LVCFMT_CENTER, 115);
	m_listImport->InsertColumn(4, L"ForwarderChain", LVCFMT_CENTER, 110);
	m_listImport->InsertColumn(5, L"Name RVA", LVCFMT_CENTER, 90);
	m_listImport->InsertColumn(6, L"FirstThunk (IAT)", LVCFMT_CENTER, 135);
	m_listImport->SetItemCountEx(static_cast<int>(m_pImport->size()), LVSICF_NOSCROLL);

	WCHAR wstr[MAX_PATH];
	int listindex = 0;
	for (auto& i : *m_pImport)
	{
		const IMAGE_IMPORT_DESCRIPTOR* pImpDesc = &i.stImportDesc;
		if (pImpDesc->TimeDateStamp)
		{
			__time64_t time = pImpDesc->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &time);
			m_listImport->SetCellTooltip(listindex, 3, wstr, L"Time / Date:");
		}
		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateTreeResources()
{
	PLIBPE_RESOURCE_ROOT pResRoot;
	if (m_pLibpe->GetResources(pResRoot) != S_OK)
		return -1;

	m_treeResTop.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CRect(0, 0, 0, 0), this, IDC_TREE_RESOURCE_TOP);
	m_treeResTop.ShowWindow(SW_HIDE);

	m_hTreeResDir = m_treeResTop.InsertItem(L"Resources tree");

	WCHAR wstr[MAX_PATH];
	HTREEITEM htreeRoot { }, htreeLvL2 { };
	long ilvlRoot = 0, ilvl2, ilvl3;

	//Main loop to extract Resources.
	for (auto& iterRoot : pResRoot->vecResRoot)
	{
		auto pResDirEntry = &iterRoot.stResDirEntryRoot; //Level Root IMAGE_RESOURCE_DIRECTORY_ENTRY
		if (pResDirEntry->DataIsDirectory)
		{
			if (pResDirEntry->NameIsString)
				swprintf(wstr, MAX_PATH, L"Entry: %li [Name: %s]", ilvlRoot, iterRoot.wstrResNameRoot.data());
			else
			{
				auto iter = g_mapResType.find(pResDirEntry->Id);
				if (iter != g_mapResType.end())
					swprintf(wstr, MAX_PATH, L"Entry: %li [Id: %u, %s]", ilvlRoot, pResDirEntry->Id, iter->second.data());
				else
					swprintf(wstr, MAX_PATH, L"Entry: %li [Id: %u]", ilvlRoot, pResDirEntry->Id);
			}
			htreeRoot = m_treeResTop.InsertItem(wstr, m_hTreeResDir);
			m_vecResId.emplace_back(ilvlRoot, -1, -1);
			m_treeResTop.SetItemData(htreeRoot, m_vecResId.size() - 1);
			ilvl2 = 0;

			auto pstResLvL2 = &iterRoot.stResLvL2;
			for (auto& iterLvL2 : pstResLvL2->vecResLvL2)
			{
				pResDirEntry = &iterLvL2.stResDirEntryLvL2; //Level 2 IMAGE_RESOURCE_DIRECTORY_ENTRY
				if (pResDirEntry->DataIsDirectory)
				{
					if (pResDirEntry->NameIsString)
						swprintf(wstr, MAX_PATH, L"Entry: %li, Name: %s", ilvl2, iterLvL2.wstrResNameLvL2.data());
					else
						swprintf(wstr, MAX_PATH, L"Entry: %li, Id: %u", ilvl2, pResDirEntry->Id);

					htreeLvL2 = m_treeResTop.InsertItem(wstr, htreeRoot);
					m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
					m_treeResTop.SetItemData(htreeLvL2, m_vecResId.size() - 1);
					ilvl3 = 0;

					auto pstResLvL3 = &iterLvL2.stResLvL3;
					for (auto& iterLvL3 : pstResLvL3->vecResLvL3)
					{
						pResDirEntry = &iterLvL3.stResDirEntryLvL3; //Level 3 IMAGE_RESOURCE_DIRECTORY_ENTRY
						if (pResDirEntry->NameIsString)
							swprintf(wstr, MAX_PATH, L"Entry: %li, Name: %s", ilvl3, iterLvL3.wstrResNameLvL3.data());
						else
							swprintf(wstr, MAX_PATH, L"Entry: %li, lang: %u", ilvl3, pResDirEntry->Id);

						auto htreeLvL3 = m_treeResTop.InsertItem(wstr, htreeLvL2);
						m_vecResId.emplace_back(ilvlRoot, ilvl2, ilvl3);
						m_treeResTop.SetItemData(htreeLvL3, m_vecResId.size() - 1);
						ilvl3++;
					}
				}
				else
				{	//DATA lvl2
					pResDirEntry = &iterLvL2.stResDirEntryLvL2;
					if (pResDirEntry->NameIsString)
						swprintf(wstr, MAX_PATH, L"Entry: %li, Name: %s", ilvl2, iterLvL2.wstrResNameLvL2.data());
					else
						swprintf(wstr, MAX_PATH, L"Entry: %li, lang: %u", ilvl2, pResDirEntry->Id);

					htreeLvL2 = m_treeResTop.InsertItem(wstr, htreeRoot);
					m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
					m_treeResTop.SetItemData(htreeLvL2, m_vecResId.size() - 1);
				}
				ilvl2++;
			}
		}
		else
		{	//DATA lvlroot
			pResDirEntry = &iterRoot.stResDirEntryRoot;

			if (pResDirEntry->NameIsString)
				swprintf(wstr, MAX_PATH, L"Entry: %li, Name: %s", ilvlRoot, iterRoot.wstrResNameRoot.data());
			else
				swprintf(wstr, MAX_PATH, L"Entry: %li, lang: %u", ilvlRoot, pResDirEntry->Id);

			htreeRoot = m_treeResTop.InsertItem(wstr, m_hTreeResDir);
			m_vecResId.emplace_back(ilvlRoot, -1, -1);
			m_treeResTop.SetItemData(htreeRoot, m_vecResId.size() - 1);
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

	m_stlcs.dwStyle = LVS_OWNERDATA;
	m_stlcs.uID = IDC_LIST_EXCEPTIONS;
	m_listExceptionDir->Create(m_stlcs);
	m_listExceptionDir->ShowWindow(SW_HIDE);
	m_listExceptionDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listExceptionDir->SetColumn(0, &stCol);
	m_listExceptionDir->SetHdrColumnColor(0, g_clrOffset);
	m_listExceptionDir->InsertColumn(1, L"BeginAddress", LVCFMT_CENTER, 100);
	m_listExceptionDir->InsertColumn(2, L"EndAddress", LVCFMT_CENTER, 100);
	m_listExceptionDir->InsertColumn(3, L"UnwindData/InfoAddress", LVCFMT_CENTER, 180);
	m_listExceptionDir->SetItemCountEx(static_cast<int>(m_pExceptionDir->size()), LVSICF_NOSCROLL);

	return 0;
}

int CViewRightTL::CreateListSecurity()
{
	PLIBPE_SECURITY_VEC pSecurityDir;
	if (m_pLibpe->GetSecurity(pSecurityDir) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_SECURITY;
	m_listSecurityDir->Create(m_stlcs);
	m_listSecurityDir->ShowWindow(SW_HIDE);
	m_listSecurityDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listSecurityDir->SetColumn(0, &stCol);
	m_listSecurityDir->SetHdrColumnColor(0, g_clrOffset);
	m_listSecurityDir->InsertColumn(1, L"dwLength", LVCFMT_CENTER, 100);
	m_listSecurityDir->InsertColumn(2, L"wRevision", LVCFMT_CENTER, 100);
	m_listSecurityDir->InsertColumn(3, L"wCertificateType", LVCFMT_CENTER, 180);

	const std::map<WORD, std::wstring> mapSertRevision {
		TO_WSTR_MAP(WIN_CERT_REVISION_1_0),
		TO_WSTR_MAP(WIN_CERT_REVISION_2_0)
	};

	const std::map<WORD, std::wstring> mapSertType {
		TO_WSTR_MAP(WIN_CERT_TYPE_X509),
		TO_WSTR_MAP(WIN_CERT_TYPE_PKCS_SIGNED_DATA),
		TO_WSTR_MAP(WIN_CERT_TYPE_RESERVED_1),
		TO_WSTR_MAP(WIN_CERT_TYPE_TS_STACK_SIGNED),
	};

	int listindex = 0;
	WCHAR wstr[9];
	for (auto& i : *pSecurityDir)
	{
		swprintf_s(wstr, 9, L"%08X", i.dwOffsetWinCertDesc);
		m_listSecurityDir->InsertItem(listindex, wstr);

		const WIN_CERTIFICATE* pSert = &i.stWinSert;
		swprintf_s(wstr, 9, L"%08X", pSert->dwLength);
		m_listSecurityDir->SetItemText(listindex, 1, wstr);

		swprintf_s(wstr, 5, L"%04X", pSert->wRevision);
		m_listSecurityDir->SetItemText(listindex, 2, wstr);

		auto iterRevision = mapSertRevision.find(pSert->wRevision);
		if (iterRevision != mapSertRevision.end())
			m_listSecurityDir->SetCellTooltip(listindex, 2, iterRevision->second.data(), L"Certificate revision:");

		swprintf_s(wstr, 5, L"%04X", pSert->wCertificateType);
		m_listSecurityDir->SetItemText(listindex, 3, wstr);

		auto iterType = mapSertType.find(pSert->wCertificateType);
		if (iterType != mapSertType.end())
			m_listSecurityDir->SetCellTooltip(listindex, 3, iterType->second.data(), L"Certificate type:");

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListRelocations()
{
	if (!m_pRelocTable)
		return -1;

	m_stlcs.dwStyle = LVS_OWNERDATA;
	m_stlcs.uID = IDC_LIST_RELOCATIONS;
	m_listRelocDir->Create(m_stlcs);
	m_listRelocDir->ShowWindow(SW_HIDE);
	m_listRelocDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listRelocDir->SetColumn(0, &stCol);
	m_listRelocDir->SetHdrColumnColor(0, g_clrOffset);
	m_listRelocDir->InsertColumn(1, L"Virtual Address", LVCFMT_CENTER, 115);
	m_listRelocDir->InsertColumn(2, L"Block Size", LVCFMT_CENTER, 100);
	m_listRelocDir->InsertColumn(3, L"Entries", LVCFMT_CENTER, 100);
	m_listRelocDir->SetItemCountEx(static_cast<int>(m_pRelocTable->size()), LVSICF_NOSCROLL);

	return 0;
}

int CViewRightTL::CreateListDebug()
{
	PLIBPE_DEBUG_VEC pDebugDir;
	if (m_pLibpe->GetDebug(pDebugDir) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_DEBUG;
	m_listDebugDir->Create(m_stlcs);
	m_listDebugDir->ShowWindow(SW_HIDE);
	m_listDebugDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listDebugDir->SetColumn(0, &stCol);
	m_listDebugDir->SetHdrColumnColor(0, g_clrOffset);
	m_listDebugDir->InsertColumn(1, L"Characteristics", LVCFMT_CENTER, 115);
	m_listDebugDir->InsertColumn(2, L"TimeDateStamp", LVCFMT_CENTER, 150);
	m_listDebugDir->InsertColumn(3, L"MajorVersion", LVCFMT_CENTER, 100);
	m_listDebugDir->InsertColumn(4, L"MinorVersion", LVCFMT_CENTER, 100);
	m_listDebugDir->InsertColumn(5, L"Type", LVCFMT_CENTER, 90);
	m_listDebugDir->InsertColumn(6, L"SizeOfData", LVCFMT_CENTER, 100);
	m_listDebugDir->InsertColumn(7, L"AddressOfRawData", LVCFMT_CENTER, 170);
	m_listDebugDir->InsertColumn(8, L"PointerToRawData", LVCFMT_CENTER, 140);

	const std::map<DWORD, std::wstring> mapDebugType {
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_UNKNOWN),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_COFF),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_CODEVIEW),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_FPO),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_MISC),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_EXCEPTION),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_FIXUP),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_OMAP_TO_SRC),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_OMAP_FROM_SRC),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_BORLAND),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_RESERVED10),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_CLSID),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_VC_FEATURE),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_POGO),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_ILTCG),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_MPX),
		TO_WSTR_MAP(IMAGE_DEBUG_TYPE_REPRO)
	};

	WCHAR wstr[MAX_PATH];
	int listindex = 0;
	for (auto& i : *pDebugDir)
	{
		const IMAGE_DEBUG_DIRECTORY* pDebug = &i.stDebugDir;

		swprintf_s(wstr, 9, L"%08X", i.dwOffsetDebug);
		m_listDebugDir->InsertItem(listindex, wstr);
		swprintf_s(wstr, 9, L"%08X", pDebug->Characteristics);
		m_listDebugDir->SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 9, L"%08X", pDebug->TimeDateStamp);
		m_listDebugDir->SetItemText(listindex, 2, wstr);
		if (pDebug->TimeDateStamp)
		{
			__time64_t time = pDebug->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &time);
			m_listDebugDir->SetCellTooltip(listindex, 2, wstr, L"Time / Date:");
		}
		swprintf_s(wstr, 5, L"%04X", pDebug->MajorVersion);
		m_listDebugDir->SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, 5, L"%04X", pDebug->MinorVersion);
		m_listDebugDir->SetItemText(listindex, 4, wstr);
		swprintf_s(wstr, 9, L"%08X", pDebug->Type);
		m_listDebugDir->SetItemText(listindex, 5, wstr);
		auto iter = mapDebugType.find(pDebug->Type);
		if (iter != mapDebugType.end())
			m_listDebugDir->SetCellTooltip(listindex, 5, iter->second.data(), L"Debug type:");
		swprintf_s(wstr, 9, L"%08X", pDebug->SizeOfData);
		m_listDebugDir->SetItemText(listindex, 6, wstr);
		swprintf_s(wstr, 9, L"%08X", pDebug->AddressOfRawData);
		m_listDebugDir->SetItemText(listindex, 7, wstr);
		swprintf_s(wstr, 9, L"%08X", pDebug->PointerToRawData);
		m_listDebugDir->SetItemText(listindex, 8, wstr);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListTLS()
{
	PLIBPE_TLS pTLSDir;
	if (m_pLibpe->GetTLS(pTLSDir) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_TLS;
	m_listTLSDir->Create(m_stlcs);
	m_listTLSDir->ShowWindow(SW_HIDE);
	m_listTLSDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listTLSDir->SetColumn(0, &stCol);
	m_listTLSDir->SetHdrColumnColor(0, g_clrOffset);
	m_listTLSDir->InsertColumn(1, L"Name", LVCFMT_CENTER, 250);
	m_listTLSDir->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 110);
	m_listTLSDir->InsertColumn(3, L"Value", LVCFMT_CENTER, 150);

	const std::map<DWORD, std::wstring> mapCharact {
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_1BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_2BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_4BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_8BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_16BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_32BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_64BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_128BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_256BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_512BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_1024BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_2048BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_4096BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_8192BYTES),
		TO_WSTR_MAP(IMAGE_SCN_ALIGN_MASK)
	};

	WCHAR wstr[18];
	if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE32))
	{
		const IMAGE_TLS_DIRECTORY32* pTLSDir32 = &pTLSDir->varTLS.stTLSDir32;
		for (unsigned i = 0; i < g_mapTLS32.size(); i++)
		{
			auto& ref = g_mapTLS32.at(i);
			DWORD dwOffset = ref.dwOffset;
			DWORD dwSize = ref.dwSize;
			DWORD dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(pTLSDir32) + dwOffset)) &
				(DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

			swprintf_s(wstr, 9, L"%08lX", pTLSDir->dwOffsetTLS + dwOffset);
			m_listTLSDir->InsertItem(i, wstr);
			m_listTLSDir->SetItemText(i, 1, ref.wstrName.data());
			swprintf_s(wstr, 17, L"%lu", dwSize);
			m_listTLSDir->SetItemText(i, 2, wstr);
			swprintf_s(wstr, 9, L"%08lX", dwValue);
			m_listTLSDir->SetItemText(i, 3, wstr);

			if (i == 5) { //Characteristics
				auto iterCharact = mapCharact.find(pTLSDir32->Characteristics);
				if (iterCharact != mapCharact.end())
					m_listTLSDir->SetCellTooltip(i, 3, iterCharact->second.data(), L"Characteristics:");
			}
		}
	}
	else if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE64))
	{
		const IMAGE_TLS_DIRECTORY64* pTLSDir64 = &pTLSDir->varTLS.stTLSDir64;
		for (unsigned i = 0; i < g_mapTLS64.size(); i++)
		{
			auto& ref = g_mapTLS64.at(i);
			DWORD dwOffset = ref.dwOffset;
			DWORD dwSize = ref.dwSize;
			ULONGLONG ullValue = *(reinterpret_cast<PULONGLONG>(reinterpret_cast<DWORD_PTR>(pTLSDir64) + dwOffset)) &
				(ULONGLONG_MAX >> ((sizeof(ULONGLONG) - dwSize) * 8));

			swprintf_s(wstr, 9, L"%08lX", pTLSDir->dwOffsetTLS + dwOffset);
			m_listTLSDir->InsertItem(i, wstr);
			m_listTLSDir->SetItemText(i, 1, ref.wstrName.data());
			swprintf_s(wstr, 17, L"%lu", dwSize);
			m_listTLSDir->SetItemText(i, 2, wstr);
			swprintf_s(wstr, 17, dwSize == 4 ? L"%08X" : L"%016llX", ullValue);
			m_listTLSDir->SetItemText(i, 3, wstr);

			if (i == 5) { //Characteristics
				auto iterCharact = mapCharact.find(pTLSDir64->Characteristics);
				if (iterCharact != mapCharact.end())
					m_listTLSDir->SetCellTooltip(i, 3, iterCharact->second.data(), L"Characteristics:");
			}
		}
	}

	return 0;
}

int CViewRightTL::CreateListLCD()
{
	PLIBPE_LOADCONFIG pLCD;
	if (m_pLibpe->GetLoadConfig(pLCD) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_LOADCONFIG;
	m_listLCD->Create(m_stlcs);
	m_listLCD->ShowWindow(SW_HIDE);
	m_listLCD->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listLCD->SetColumn(0, &stCol);
	m_listLCD->SetHdrColumnColor(0, g_clrOffset);
	m_listLCD->InsertColumn(1, L"Name", LVCFMT_CENTER, 330);
	m_listLCD->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 110);
	m_listLCD->InsertColumn(3, L"Value", LVCFMT_CENTER, 300);

	const std::map<WORD, std::wstring> mapGuardFlags {
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

	WCHAR wstr[MAX_PATH];
	std::wstring wstrTooltip;
	if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE32))
	{
		const IMAGE_LOAD_CONFIG_DIRECTORY32* pLCD32 = &pLCD->varLCD.stLCD32;
		for (unsigned i = 0; i < g_mapLCD32.size(); i++)
		{
			auto& ref = g_mapLCD32.at(i);
			if (ref.dwOffset >= pLCD32->Size)
				break;

			DWORD dwOffset = ref.dwOffset;
			DWORD dwSize = ref.dwSize;
			DWORD dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(pLCD32) + dwOffset)) &
				(DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

			swprintf_s(wstr, 9, L"%08lX", pLCD->dwOffsetLCD + dwOffset);
			m_listLCD->InsertItem(i, wstr);
			m_listLCD->SetItemText(i, 1, ref.wstrName.data());
			swprintf_s(wstr, 17, L"%lu", dwSize);
			m_listLCD->SetItemText(i, 2, wstr);
			swprintf_s(wstr, 9, dwSize == 2 ? L"%04X" : L"%08X", dwValue);
			m_listLCD->SetItemText(i, 3, wstr);

			if (i == 1) //TimeDateStamp
			{
				if (pLCD32->TimeDateStamp) {
					__time64_t time = pLCD32->TimeDateStamp;
					_wctime64_s(wstr, MAX_PATH, &time);
					m_listLCD->SetCellTooltip(i, 2, wstr, L"Time / Date:");
				}
			}
			else if (i == 24) //GuardFlags
			{
				wstrTooltip.clear();
				for (auto& it : mapGuardFlags)
					if (it.first & pLCD32->GuardFlags)
						wstrTooltip += it.second + L"\n";
				if (!wstrTooltip.empty())
					m_listLCD->SetCellTooltip(i, 3, wstrTooltip.data(), L"GuardFlags:");
			}
		}
	}
	else if (ImageHasFlag(m_dwFileInfo, IMAGE_FLAG_PE64))
	{
		const IMAGE_LOAD_CONFIG_DIRECTORY64* pLCD64 = &pLCD->varLCD.stLCD64;
		for (unsigned i = 0; i < g_mapLCD64.size(); i++)
		{
			auto& ref = g_mapLCD64.at(i);
			if (ref.dwOffset >= pLCD64->Size)
				break;

			DWORD dwOffset = ref.dwOffset;
			DWORD dwSize = ref.dwSize;
			ULONGLONG ullValue = *(reinterpret_cast<PULONGLONG>(reinterpret_cast<DWORD_PTR>(pLCD64) + dwOffset)) &
				(ULONGLONG_MAX >> ((sizeof(ULONGLONG) - dwSize) * 8));

			swprintf_s(wstr, 9, L"%08lX", pLCD->dwOffsetLCD + dwOffset);
			m_listLCD->InsertItem(i, wstr);
			m_listLCD->SetItemText(i, 1, ref.wstrName.data());
			swprintf_s(wstr, 17, L"%lu", dwSize);
			m_listLCD->SetItemText(i, 2, wstr);
			swprintf_s(wstr, 17, dwSize == 2 ? L"%04X" : (dwSize == 4 ? L"%08X" : L"%016llX"), ullValue);
			m_listLCD->SetItemText(i, 3, wstr);

			if (i == 1) //TimeDateStamp
			{
				if (pLCD64->TimeDateStamp) {
					__time64_t time = pLCD64->TimeDateStamp;
					_wctime64_s(wstr, MAX_PATH, &time);
					m_listLCD->SetCellTooltip(i, 2, wstr, L"Time / Date:");
				}
			}
			else if (i == 24) //GuardFlags
			{
				wstrTooltip.clear();
				for (auto & it : mapGuardFlags)
					if (it.first & pLCD64->GuardFlags)
						wstrTooltip += it.second + L"\n";
				if (!wstrTooltip.empty())
					m_listLCD->SetCellTooltip(i, 3, wstrTooltip.data(), L"GuardFlags:");
			}
		}
	}

	return 0;
}

int CViewRightTL::CreateListBoundImport()
{
	PLIBPE_BOUNDIMPORT_VEC pBoundImp;
	if (m_pLibpe->GetBoundImport(pBoundImp) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_BOUNDIMPORT;
	m_listBoundImportDir->Create(m_stlcs);
	m_listBoundImportDir->ShowWindow(SW_HIDE);
	m_listBoundImportDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listBoundImportDir->SetColumn(0, &stCol);
	m_listBoundImportDir->SetHdrColumnColor(0, g_clrOffset);
	m_listBoundImportDir->InsertColumn(1, L"Module Name", LVCFMT_CENTER, 290);
	m_listBoundImportDir->InsertColumn(2, L"TimeDateStamp", LVCFMT_CENTER, 130);
	m_listBoundImportDir->InsertColumn(3, L"OffsetModuleName", LVCFMT_CENTER, 140);
	m_listBoundImportDir->InsertColumn(4, L"NumberOfModuleForwarderRefs", LVCFMT_CENTER, 220);

	WCHAR wstr[MAX_PATH];
	int listindex = 0;

	for (auto& i : *pBoundImp)
	{
		swprintf_s(wstr, 9, L"%08X", i.dwOffsetBoundImpDesc);
		m_listBoundImportDir->InsertItem(listindex, wstr);

		const IMAGE_BOUND_IMPORT_DESCRIPTOR* pBoundImpDir = &i.stBoundImpDesc;
		swprintf_s(wstr, MAX_PATH, L"%S", i.strBoundName.data());
		m_listBoundImportDir->SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, MAX_PATH, L"%08X", pBoundImpDir->TimeDateStamp);
		m_listBoundImportDir->SetItemText(listindex, 2, wstr);
		if (pBoundImpDir->TimeDateStamp)
		{
			__time64_t _time = pBoundImpDir->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &_time);
			m_listBoundImportDir->SetCellTooltip(listindex, 2, wstr, L"Time / Date:");
		}
		swprintf_s(wstr, 5, L"%04X", pBoundImpDir->OffsetModuleName);
		m_listBoundImportDir->SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, 5, L"%04X", pBoundImpDir->NumberOfModuleForwarderRefs);
		m_listBoundImportDir->SetItemText(listindex, 4, wstr);

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListDelayImport()
{
	PLIBPE_DELAYIMPORT_VEC pDelayImp;
	if (m_pLibpe->GetDelayImport(pDelayImp) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_DELAYIMPORT;
	m_listDelayImportDir->Create(m_stlcs);
	m_listDelayImportDir->ShowWindow(SW_HIDE);
	m_listDelayImportDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listDelayImportDir->SetColumn(0, &stCol);
	m_listDelayImportDir->SetHdrColumnColor(0, g_clrOffset);
	m_listDelayImportDir->InsertColumn(1, L"Module Name (funcs number)", LVCFMT_CENTER, 260);
	m_listDelayImportDir->InsertColumn(2, L"Attributes", LVCFMT_CENTER, 100);
	m_listDelayImportDir->InsertColumn(3, L"DllNameRVA", LVCFMT_CENTER, 105);
	m_listDelayImportDir->InsertColumn(4, L"ModuleHandleRVA", LVCFMT_CENTER, 140);
	m_listDelayImportDir->InsertColumn(5, L"ImportAddressTableRVA", LVCFMT_CENTER, 160);
	m_listDelayImportDir->InsertColumn(6, L"ImportNameTableRVA", LVCFMT_CENTER, 150);
	m_listDelayImportDir->InsertColumn(7, L"BoundImportAddressTableRVA", LVCFMT_CENTER, 200);
	m_listDelayImportDir->InsertColumn(8, L"UnloadInformationTableRVA", LVCFMT_CENTER, 190);
	m_listDelayImportDir->InsertColumn(9, L"TimeDateStamp", LVCFMT_CENTER, 115);

	int listindex = 0;
	WCHAR wstr[MAX_PATH];

	for (auto& i : *pDelayImp)
	{
		swprintf_s(wstr, 9, L"%08X", i.dwOffsetDelayImpDesc);
		m_listDelayImportDir->InsertItem(listindex, wstr);

		const IMAGE_DELAYLOAD_DESCRIPTOR* pDelayImpDir = &i.stDelayImpDesc;
		swprintf_s(wstr, MAX_PATH, L"%S (%zu)", i.strModuleName.data(), i.vecDelayImpFunc.size());
		m_listDelayImportDir->SetItemText(listindex, 1, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->Attributes.AllAttributes);
		m_listDelayImportDir->SetItemText(listindex, 2, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->DllNameRVA);
		m_listDelayImportDir->SetItemText(listindex, 3, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->ModuleHandleRVA);
		m_listDelayImportDir->SetItemText(listindex, 4, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->ImportAddressTableRVA);
		m_listDelayImportDir->SetItemText(listindex, 5, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->ImportNameTableRVA);
		m_listDelayImportDir->SetItemText(listindex, 6, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->BoundImportAddressTableRVA);
		m_listDelayImportDir->SetItemText(listindex, 7, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->UnloadInformationTableRVA);
		m_listDelayImportDir->SetItemText(listindex, 8, wstr);
		swprintf_s(wstr, 9, L"%08X", pDelayImpDir->TimeDateStamp);
		m_listDelayImportDir->SetItemText(listindex, 9, wstr);
		if (pDelayImpDir->TimeDateStamp) {
			__time64_t time = pDelayImpDir->TimeDateStamp;
			_wctime64_s(wstr, MAX_PATH, &time);
			m_listDelayImportDir->SetCellTooltip(listindex, 8, wstr, L"Time / Date:");
		}

		listindex++;
	}

	return 0;
}

int CViewRightTL::CreateListCOM()
{
	PLIBPE_COMDESCRIPTOR pCOMDesc;
	if (m_pLibpe->GetCOMDescriptor(pCOMDesc) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_COMDESCRIPTOR;
	m_listCOMDir->Create(m_stlcs);
	m_listCOMDir->ShowWindow(SW_HIDE);
	m_listCOMDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_listCOMDir->SetColumn(0, &stCol);
	m_listCOMDir->SetHdrColumnColor(0, g_clrOffset);
	m_listCOMDir->InsertColumn(1, L"Name", LVCFMT_CENTER, 300);
	m_listCOMDir->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listCOMDir->InsertColumn(3, L"Value", LVCFMT_CENTER, 300);

	const std::map<DWORD, std::wstring> mapFlags {
		{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_ILONLY, L"COMIMAGE_FLAGS_ILONLY" },
		{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_32BITREQUIRED, L"COMIMAGE_FLAGS_32BITREQUIRED" },
		{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_IL_LIBRARY, L"COMIMAGE_FLAGS_IL_LIBRARY" },
		{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_STRONGNAMESIGNED, L"COMIMAGE_FLAGS_STRONGNAMESIGNED" },
		{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_NATIVE_ENTRYPOINT, L"COMIMAGE_FLAGS_NATIVE_ENTRYPOINT" },
		{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_TRACKDEBUGDATA, L"COMIMAGE_FLAGS_TRACKDEBUGDATA" },
		{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_32BITPREFERRED, L"COMIMAGE_FLAGS_32BITPREFERRED" }
	};

	const IMAGE_COR20_HEADER * pCom = &pCOMDesc->stCorHdr;
	for (unsigned i = 0; i < g_mapComDir.size(); i++)
	{
		WCHAR wstr[18];
		std::wstring wstrToolTip;
		auto& ref = g_mapComDir.at(i);
		DWORD dwOffset = ref.dwOffset;
		DWORD dwSize = ref.dwSize;
		DWORD dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(pCom) + dwOffset)) &
			(DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

		swprintf_s(wstr, 9, L"%08lX", pCOMDesc->dwOffsetComDesc + dwOffset);
		m_listCOMDir->InsertItem(i, wstr);
		m_listCOMDir->SetItemText(i, 1, ref.wstrName.data());
		swprintf_s(wstr, 17, L"%lu", dwSize);
		m_listCOMDir->SetItemText(i, 2, wstr);
		swprintf_s(wstr, 9, dwSize == 2 ? L"%04X" : L"%08X", dwValue);
		m_listCOMDir->SetItemText(i, 3, wstr);

		if (i == 5)
		{
			for (auto& iter : mapFlags)
				if (iter.first & pCOMDesc->stCorHdr.Flags)
					wstrToolTip += iter.second + L"\n";
			if (!wstrToolTip.empty())
				m_listCOMDir->SetCellTooltip(i, 3, wstrToolTip.data(), L"Flags:");
		}
	}

	return 0;
}

void CViewRightTL::SortImportData()
{
	std::sort(m_pImport->begin(), m_pImport->end(),
		[&](const LIBPE_IMPORT_MODULE& ref1, const LIBPE_IMPORT_MODULE& ref2)
		{
			std::wstring wstr1, wstr2;
			wstr1.resize(32);
			wstr2.resize(32);

			int iCompare { };
			switch (m_listImport->GetSortColumn())
			{
			case 0:
				wstr1.resize(swprintf_s(wstr1.data(), 9, L"%08X", ref1.dwOffsetImpDesc));
				wstr2.resize(swprintf_s(wstr2.data(), 9, L"%08X", ref2.dwOffsetImpDesc));
				iCompare = wstr1.compare(wstr2);
				break;
			case 1:
				iCompare = ref1.strModuleName.compare(ref2.strModuleName);
				break;
			case 2:
				wstr1.resize(swprintf_s(wstr1.data(), 9, L"%08X", ref1.stImportDesc.OriginalFirstThunk));
				wstr2.resize(swprintf_s(wstr2.data(), 9, L"%08X", ref2.stImportDesc.OriginalFirstThunk));
				iCompare = wstr1.compare(wstr2);
				break;
			case 3:
				wstr1.resize(swprintf_s(wstr1.data(), 9, L"%08X", ref1.stImportDesc.TimeDateStamp));
				wstr2.resize(swprintf_s(wstr2.data(), 9, L"%08X", ref2.stImportDesc.TimeDateStamp));
				iCompare = wstr1.compare(wstr2);
				break;
			case 4:
				wstr1.resize(swprintf_s(wstr1.data(), 9, L"%08X", ref1.stImportDesc.ForwarderChain));
				wstr2.resize(swprintf_s(wstr2.data(), 9, L"%08X", ref2.stImportDesc.ForwarderChain));
				iCompare = wstr1.compare(wstr2);
				break;
			case 5:
				wstr1.resize(swprintf_s(wstr1.data(), 9, L"%08X", ref1.stImportDesc.Name));
				wstr2.resize(swprintf_s(wstr2.data(), 9, L"%08X", ref2.stImportDesc.Name));
				iCompare = wstr1.compare(wstr2);
				break;
			case 6:
				wstr1.resize(swprintf_s(wstr1.data(), 9, L"%08X", ref1.stImportDesc.FirstThunk));
				wstr2.resize(swprintf_s(wstr2.data(), 9, L"%08X", ref2.stImportDesc.FirstThunk));
				iCompare = wstr1.compare(wstr2);
				break;
			}

			bool result { false };
			if (m_listImport->GetSortAscending())
			{
				if (iCompare < 0)
					result = true;
			}
			else
			{
				if (iCompare > 0)
					result = true;
			}

			return result;

		});

	m_listImport->RedrawWindow();
}