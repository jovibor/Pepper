/****************************************************************************************************
* Copyright © 2018-2023 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#include "stdafx.h"
#include "CViewRightTL.h"
#include "strsafe.h"
#include <algorithm>
#include <format>

IMPLEMENT_DYNCREATE(CViewRightTL, CView)

BEGIN_MESSAGE_MAP(CViewRightTL, CView)
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_NOTIFY(LVN_GETDISPINFOW, IDC_LIST_SECHEADERS, &CViewRightTL::OnListSecHdrGetDispInfo)
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
	m_pFileLoader = &m_pMainDoc->GetFileLoader();

	LOGFONTW lf { };
	StringCchCopyW(lf.lfFaceName, 9, L"Consolas");
	auto pDC = GetDC();
	const auto iLOGPIXELSY = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY);
	ReleaseDC(pDC);
	lf.lfHeight = -MulDiv(14, iLOGPIXELSY, 72);
	if (!m_fontSummary.CreateFontIndirectW(&lf)) {
		StringCchCopyW(lf.lfFaceName, 18, L"Times New Roman");
		m_fontSummary.CreateFontIndirectW(&lf);
	}

	m_stFileInfo = m_pMainDoc->GetFileInfo();

	m_wstrFullPath = L"Full path: " + m_pMainDoc->GetPathName();
	m_wstrFileName = m_pMainDoc->GetPathName();
	m_wstrFileName.erase(0, m_wstrFileName.find_last_of('\\') + 1);
	m_wstrFileName.insert(0, L"File name: ");

	if (m_stFileInfo.eFileType == EFileType::PE32) {
		m_wstrFileType = L"File type: PE32 (x86)";
	}
	else if (m_stFileInfo.eFileType == EFileType::PE64) {
		m_wstrFileType = L"File type: PE32+ (x64)";
	}
	else {
		m_wstrFileType = L"File type: unknown";
	}
	m_wstrPepperVersion = std::format(L"Pepper v{}.{}.{}", Utility::PEPPER_VERSION_MAJOR,
		Utility::PEPPER_VERSION_MINOR, Utility::PEPPER_VERSION_PATCH);

	m_stlcs.stColor.clrTooltipText = RGB(255, 255, 255);
	m_stlcs.stColor.clrTooltipBk = RGB(0, 132, 132);
	m_stlcs.stColor.clrHdrText = RGB(255, 255, 255);
	m_stlcs.stColor.clrHdrBk = RGB(0, 132, 132);
	m_stlcs.stColor.clrHdrHglInact = RGB(0, 112, 112);
	m_stlcs.stColor.clrHdrHglAct = RGB(0, 92, 92);
	m_stlcs.pParent = this;
	m_stlcs.dwHdrHeight = 39;
	m_stlcs.fSortable = true;

	m_lf.lfHeight = m_hdrlf.lfHeight = -MulDiv(11, iLOGPIXELSY, 72);
	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stlcs.pListLogFont = &m_lf;
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
	CreateListDataDirs();
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
	const auto iMsg = LOWORD(lHint);
	if (iMsg == MSG_MDITAB_ACTIVATE || iMsg == MSG_MDITAB_DISACTIVATE) {
		return; //No further handling if it's tab Activate/Disactivate messages.
	}

	//Check m_pChildFrame to prevent some UB.
	//OnUpdate can be invoked before OnInitialUpdate, weird MFC.
	if (!m_pChildFrame || LOWORD(lHint) == IDC_SHOW_RESOURCE_RBR || LOWORD(lHint) == ID_DOC_EDITMODE)
		return;

	if (m_pwndActive)
		m_pwndActive->ShowWindow(SW_HIDE);

	m_fFileSummaryShow = false;
	bool fShowRow { true };
	switch (LOWORD(lHint)) {
	case IDC_SHOW_FILE_SUMMARY:
		m_fFileSummaryShow = true;
		m_pwndActive = nullptr;
		break;
	case IDC_LIST_DOSHEADER:
		m_pwndActive = &*m_listDOSHeader;
		break;
	case IDC_LIST_RICHHEADER:
		m_pwndActive = &*m_listRichHdr;
		break;
	case IDC_LIST_NTHEADER:
		m_pwndActive = &*m_listNTHeader;
		break;
	case IDC_LIST_FILEHEADER:
		m_pwndActive = &*m_listFileHeader;
		break;
	case IDC_LIST_OPTIONALHEADER:
		m_pwndActive = &*m_listOptHeader;
		break;
	case IDC_LIST_DATADIRECTORIES:
		m_pwndActive = &*m_listDataDirs;
		break;
	case IDC_LIST_SECHEADERS:
		m_pwndActive = &*m_listSecHeaders;
		break;
	case IDC_LIST_EXPORT:
		m_pwndActive = &*m_listExportDir;
		break;
	case IDC_LIST_IAT:
	case IDC_LIST_IMPORT:
		m_pwndActive = &*m_listImport;
		break;
	case IDC_TREE_RESOURCE:
		m_pwndActive = &m_treeResTop;
		break;
	case IDC_LIST_EXCEPTIONS:
		m_pwndActive = &*m_listExceptionDir;
		fShowRow = false;
		break;
	case IDC_LIST_SECURITY:
		m_pwndActive = &*m_listSecurityDir;
		break;
	case IDC_LIST_RELOCATIONS:
		m_pwndActive = &*m_listRelocDir;
		break;
	case IDC_LIST_DEBUG:
		m_pwndActive = &*m_listDebugDir;
		break;
	case IDC_LIST_TLS:
		m_pwndActive = &*m_listTLSDir;
		break;
	case IDC_LIST_LOADCONFIG:
		m_pwndActive = &*m_listLCD;
		break;
	case IDC_LIST_BOUNDIMPORT:
		m_pwndActive = &*m_listBoundImportDir;
		break;
	case IDC_LIST_DELAYIMPORT:
		m_pwndActive = &*m_listDelayImportDir;
		break;
	case IDC_LIST_COMDESCRIPTOR:
		m_pwndActive = &*m_listCOMDir;
		fShowRow = false;
		break;
	}
	if (m_pwndActive) {
		CRect rcClient;
		GetClientRect(&rcClient);
		m_pwndActive->SetWindowPos(this, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	fShowRow ? m_pChildFrame->GetSplitRight().ShowRow(1) : m_pChildFrame->GetSplitRight().HideRow(1);
	m_pChildFrame->GetSplitRight().RecalcLayout();
}

void CViewRightTL::OnDraw(CDC* pDC)
{
	//Printing app name/version info and
	//currently oppened file's type and name.
	if (!m_fFileSummaryShow)
		return;

	CMemDC memDC(*pDC, this);
	auto& rDC = memDC.GetDC();

	CRect rc;
	rDC.GetClipBox(rc);
	rDC.FillSolidRect(rc, RGB(255, 255, 255));
	rDC.SelectObject(m_fontSummary);
	SIZE sizeTextToDraw;
	if (m_wstrPepperVersion.size() > m_wstrFullPath.size())
		GetTextExtentPoint32W(rDC.m_hDC, m_wstrPepperVersion.data(), static_cast<int>(m_wstrPepperVersion.size()), &sizeTextToDraw);
	else
		GetTextExtentPoint32W(rDC.m_hDC, m_wstrFullPath.data(), static_cast<int>(m_wstrFullPath.size()), &sizeTextToDraw);
	constexpr auto iRectLeft = 20;
	constexpr auto iRectTop = 20;
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
	if (pNMI->hdr.code == NM_RCLICK && (pNMI->hdr.idFrom == IDC_LIST_EXPORT || pNMI->hdr.idFrom == IDC_LIST_IMPORT
		|| pNMI->hdr.idFrom == IDC_LIST_IAT || pNMI->hdr.idFrom == IDC_LIST_TLS
		|| pNMI->hdr.idFrom == IDC_LIST_BOUNDIMPORT || pNMI->hdr.idFrom == IDC_LIST_COMDESCRIPTOR)) {
		m_iListID = pNMI->hdr.idFrom;
		m_iListItem = pNMI->iItem;
		m_iListSubItem = pNMI->iSubItem;
		POINT pt { };
		GetCursorPos(&pt);
		m_menuList.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);

		return TRUE;
	}

	switch (pNMI->hdr.idFrom) {
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
	switch (m_iListID) {
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

void CViewRightTL::OnListSecHdrGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	auto* pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	auto* pItem = &pDispInfo->item;
	const auto& pSecHeaders = m_pMainDoc->GetSecHeaders();

	if (pItem->mask & LVIF_TEXT) {
		const auto& refAt = pSecHeaders->at(pItem->iItem);
		const auto& refDescr = refAt.stSecHdr;
		switch (pItem->iSubItem) {
		case 0:
			*std::format_to(pItem->pszText, L"{:08X}", refAt.dwOffset) = '\0';
			break;
		case 1:
			if (refAt.strSecName.empty())
				*std::format_to(pItem->pszText, L"{:.8}", StrToWstr(std::string_view(reinterpret_cast<const char*>(refDescr.Name), 8))) = '\0';
			else
				*std::format_to(pItem->pszText, L"{:.8} ({})", StrToWstr(std::string_view(reinterpret_cast<const char*>(refDescr.Name), 8)),
					StrToWstr(refAt.strSecName)) = '\0';
			break;
		case 2:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.Misc.VirtualSize) = '\0';
			break;
		case 3:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.VirtualAddress) = '\0';
			break;
		case 4:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.SizeOfRawData) = '\0';
			break;
		case 5:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.PointerToRawData) = '\0';
			break;
		case 6:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.PointerToRelocations) = '\0';
			break;
		case 7:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.PointerToLinenumbers) = '\0';
			break;
		case 8:
			*std::format_to(pItem->pszText, L"{:04X}", refDescr.NumberOfRelocations) = '\0';
			break;
		case 9:
			*std::format_to(pItem->pszText, L"{:04X}", refDescr.NumberOfLinenumbers) = '\0';
			break;
		case 10:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.Characteristics) = '\0';
			break;
		}
	}
}

void CViewRightTL::OnListSecHdrGetToolTip(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	const auto pNMI = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMI->iSubItem != 10)
		return;

	const auto& pSecHeaders = m_pMainDoc->GetSecHeaders();
	std::wstring wstrTipText;
	for (const auto& flags : MapSecHdrCharact) {
		if (flags.first & pSecHeaders->at(pNMI->iItem).stSecHdr.Characteristics) {
			wstrTipText += flags.second;
			wstrTipText += L"\n";
		}
	}
	if (!wstrTipText.empty()) {
		static LISTEXTOOLTIP stTT { { }, L"Section Flags:" };
		stTT.wstrText = std::move(wstrTipText);
		pNMI->lParam = reinterpret_cast<LPARAM>(&stTT); //Tooltip pointer.
	}
}

void CViewRightTL::OnListImportGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	const auto* pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	const auto* pItem = &pDispInfo->item;
	const auto& pImport = m_pMainDoc->GetImport();

	if (pItem->mask & LVIF_TEXT) {
		const auto& refAt = pImport->at(pItem->iItem);
		const auto& refDescr = refAt.stImportDesc;
		switch (pItem->iSubItem) {
		case 0:
			*std::format_to(pItem->pszText, L"{:08X}", refAt.dwOffset) = '\0';
			break;
		case 1:
			*std::format_to(pItem->pszText, L"{} ({})", StrToWstr(refAt.strModuleName), refAt.vecImportFunc.size()) = '\0';
			break;
		case 2:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.OriginalFirstThunk) = '\0';
			break;
		case 3:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.TimeDateStamp) = '\0';
			break;
		case 4:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.ForwarderChain) = '\0';
			break;
		case 5:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.Name) = '\0';
			break;
		case 6:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.FirstThunk) = '\0';
			break;
		}
	}
}

void CViewRightTL::OnListRelocsGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	const auto* pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	const auto* pItem = &pDispInfo->item;
	const auto& pRelocTable = m_pMainDoc->GetRelocations();

	if (pItem->mask & LVIF_TEXT) {
		const auto& refDescr = pRelocTable->at(pItem->iItem).stBaseReloc;
		switch (pItem->iSubItem) {
		case 0:
			*std::format_to(pItem->pszText, L"{:08X}", pRelocTable->at(pItem->iItem).dwOffset) = '\0';
			break;
		case 1:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.VirtualAddress) = '\0';
			break;
		case 2:
			*std::format_to(pItem->pszText, L"{:08X}", refDescr.SizeOfBlock) = '\0';
			break;
		case 3:
			*std::format_to(pItem->pszText, L"{}", (refDescr.SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD)) = '\0';
			break;
		}
	}
}

void CViewRightTL::OnListExceptionsGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	const auto* pDispInfo = reinterpret_cast<NMLVDISPINFOW*>(pNMHDR);
	const auto* pItem = &pDispInfo->item;
	const auto& pExceptionDir = m_pMainDoc->GetExceptions();

	if (pItem->mask & LVIF_TEXT) {
		const auto& ref = pExceptionDir->at(pItem->iItem);
		DWORD dwFmtData { };
		switch (pItem->iSubItem) {
		case 0:
			dwFmtData = ref.dwOffset;
			break;
		case 1:
			dwFmtData = ref.stRuntimeFuncEntry.BeginAddress;
			break;
		case 2:
			dwFmtData = ref.stRuntimeFuncEntry.EndAddress;
			break;
		case 3:
			dwFmtData = ref.stRuntimeFuncEntry.UnwindData;
			break;
		}
		*std::format_to(pItem->pszText, L"{:08X}", dwFmtData) = '\0';
	}
}

void CViewRightTL::OnListExportMenuSelect(WORD wMenuID)
{
	DWORD dwOffset { }, dwSize = 0;

	const auto& pExport = m_pMainDoc->GetExport();
	if (!pExport)
		return;

	switch (wMenuID) {
	case IDM_LIST_GOTODESCOFFSET:
	{
		dwOffset = pExport->dwOffset;
		dwSize = sizeof(IMAGE_EXPORT_DIRECTORY);
	}
	break;
	case IDM_LIST_GOTODATAOFFSET:
	{
		switch (m_iListItem) {
		case 4: //Name
			dwOffset = m_pMainDoc->GetOffsetFromRVA(pExport->stExportDesc.Name);
			dwSize = static_cast<DWORD>(pExport->strModuleName.size());
			break;
		case 8: //AddressOfFunctions
			dwOffset = m_pMainDoc->GetOffsetFromRVA(pExport->stExportDesc.AddressOfFunctions);
			dwSize = 1;
			break;
		case 9: //AddressOfNames
			dwOffset = m_pMainDoc->GetOffsetFromRVA(pExport->stExportDesc.AddressOfNames);
			dwSize = 1;
			break;
		case 10: //AddressOfOrdinals
			dwOffset = m_pMainDoc->GetOffsetFromRVA(pExport->stExportDesc.AddressOfNameOrdinals);
			dwSize = 1;
			break;
		}
	}
	break;
	}

	if (dwSize)
		m_pFileLoader->ShowOffsetInWholeFile(dwOffset, dwSize);
}

void CViewRightTL::OnListImportMenuSelect(WORD wMenuID)
{
	const auto& pImport = m_pMainDoc->GetImport();
	DWORD dwOffset { };
	DWORD dwSize = 0;

	switch (wMenuID) {
	case IDM_LIST_GOTODESCOFFSET:
		dwOffset = pImport->at(m_iListItem).dwOffset;
		dwSize = sizeof(IMAGE_IMPORT_DESCRIPTOR);
		break;
	case IDM_LIST_GOTODATAOFFSET:
		switch (m_iListSubItem) {
		case 1: //Str dll name
		case 5: //Name
			dwOffset = m_pMainDoc->GetOffsetFromRVA(pImport->at(m_iListItem).stImportDesc.Name);
			dwSize = static_cast<DWORD>(pImport->at(m_iListItem).strModuleName.size());
			break; ;
		case 2: //OriginalFirstThunk 
			dwOffset = m_pMainDoc->GetOffsetFromRVA(pImport->at(m_iListItem).stImportDesc.OriginalFirstThunk);
			if (m_stFileInfo.eFileType == EFileType::PE32) {
				dwSize = sizeof(IMAGE_THUNK_DATA32);
			}
			else if (m_stFileInfo.eFileType == EFileType::PE64) {
				dwSize = sizeof(IMAGE_THUNK_DATA64);
			}
			break;
		case 3: //TimeDateStamp
			break;
		case 4: //ForwarderChain
			dwOffset = m_pMainDoc->GetOffsetFromRVA(pImport->at(m_iListItem).stImportDesc.ForwarderChain);
			break;
		case 6: //FirstThunk
			dwOffset = m_pMainDoc->GetOffsetFromRVA(pImport->at(m_iListItem).stImportDesc.FirstThunk);
			if (m_stFileInfo.eFileType == EFileType::PE32) {
				dwSize = sizeof(IMAGE_THUNK_DATA32);
			}
			else if (m_stFileInfo.eFileType == EFileType::PE64) {
				dwSize = sizeof(IMAGE_THUNK_DATA64);
			}
			break;
		}
		break;
	}

	if (dwSize)
		m_pFileLoader->ShowOffsetInWholeFile(dwOffset, dwSize);
}

void CViewRightTL::OnListTLSMenuSelect(WORD wMenuID)
{
	DWORD dwOffset { };
	DWORD dwSize = 0;

	const auto& pTLSDir = m_pMainDoc->GetTLS();
	if (!pTLSDir)
		return;

	switch (wMenuID) {
	case IDM_LIST_GOTODESCOFFSET:
		dwOffset = pTLSDir->dwOffset;
		if (m_stFileInfo.eFileType == EFileType::PE32) {
			dwSize = sizeof(IMAGE_TLS_DIRECTORY32);
		}
		else if (m_stFileInfo.eFileType == EFileType::PE64) {
			dwSize = sizeof(IMAGE_TLS_DIRECTORY64);
		}
		break;
	case IDM_LIST_GOTODATAOFFSET:
	{
		dwSize = 1; //Just highlight a starting address of one of a TLS field.
		if (m_stFileInfo.eFileType == EFileType::PE32) {
			const auto pTLSDir32 = &pTLSDir->unTLS.stTLSDir32;

			switch (m_iListItem) {
			case 0: //StartAddressOfRawData
				dwOffset = m_pMainDoc->GetOffsetFromVA(pTLSDir32->StartAddressOfRawData);
				break;
			case 2: //AddressOfIndex
				dwOffset = m_pMainDoc->GetOffsetFromVA(pTLSDir32->AddressOfIndex);
				break;
			case 3: //AddressOfCallBacks
				dwOffset = m_pMainDoc->GetOffsetFromVA(pTLSDir32->AddressOfCallBacks);
				break;
			default:
				dwSize = 0; //To not process other fields.
			}
		}
		else if (m_stFileInfo.eFileType == EFileType::PE64) {
			const auto pTLSDir64 = &pTLSDir->unTLS.stTLSDir64;

			switch (m_iListItem) {
			case 0: //StartAddressOfRawData
				dwOffset = m_pMainDoc->GetOffsetFromRVA(pTLSDir64->StartAddressOfRawData);
				break;
			case 2: //AddressOfIndex
				dwOffset = m_pMainDoc->GetOffsetFromRVA(pTLSDir64->AddressOfIndex);
				break;
			case 3: //AddressOfCallBacks
				dwOffset = m_pMainDoc->GetOffsetFromRVA(pTLSDir64->AddressOfCallBacks);
				break;
			default:
				dwSize = 0; //To not process other fields.
			}
		}
	}
	break;
	}

	if (dwSize > 0)
		m_pFileLoader->ShowOffsetInWholeFile(dwOffset, dwSize);
}

void CViewRightTL::OnListBoundImpMenuSelect(WORD wMenuID)
{
	DWORD dwOffset { };
	DWORD dwSize = 0;
	const auto& pBoundImp = m_pMainDoc->GetBoundImport();
	if (!pBoundImp)
		return;

	switch (wMenuID) {
	case IDM_LIST_GOTODESCOFFSET:
	{
		dwOffset = pBoundImp->at(m_iListItem).dwOffset;
		dwSize = sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR);
	}
	break;
	case IDM_LIST_GOTODATAOFFSET:
	{
		switch (m_iListSubItem) {
		case 3: //OffsetModuleName
			dwOffset = m_pMainDoc->GetOffsetFromRVA(pBoundImp->at(m_iListItem).stBoundImpDesc.OffsetModuleName);
			dwSize = static_cast<DWORD>(pBoundImp->at(m_iListItem).strBoundName.size());
			break;
		}
	}
	break;
	}

	if (dwSize)
		m_pFileLoader->ShowOffsetInWholeFile(dwOffset, dwSize);
}

void CViewRightTL::OnListCOMDescMenuSelect(WORD wMenuID)
{
	DWORD dwOffset { };
	DWORD dwSize = 0;
	const auto& pCOMDesc = m_pMainDoc->GetCOMDescriptor();
	if (!pCOMDesc)
		return;

	switch (wMenuID) {
	case IDM_LIST_GOTODESCOFFSET:
	{
		dwOffset = pCOMDesc->dwOffset;
		dwSize = sizeof(IMAGE_COR20_HEADER);
	}
	break;
	case IDM_LIST_GOTODATAOFFSET:
		//TODO: IDC_LIST_COMDESCRIPTOR->IDM_LIST_GOTODATAOFFSET.
		break;
	}

	if (dwSize)
		m_pFileLoader->ShowOffsetInWholeFile(dwOffset, dwSize);
}

void CViewRightTL::OnTreeResTopSelChange(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	const auto pTree = reinterpret_cast<LPNMTREEVIEWW>(pNMHDR);
	if (pTree->itemNew.hItem == m_hTreeResRoot)
		return;

	const auto& pstResRoot = m_pMainDoc->GetResources();
	if (!pstResRoot)
		return;

	const auto& [idlvlRoot, idlvl2, idlvl3] = m_vecResId.at(m_treeResTop.GetItemData(pTree->itemNew.hItem));
	if (idlvl2 >= 0) {
		const auto& rootvec = pstResRoot->vecResData;
		const auto& lvl2st = rootvec[idlvlRoot].stResLvL2;
		const auto& lvl2vec = lvl2st.vecResData;
		if (!lvl2vec.empty()) {
			if (idlvl3 >= 0) {
				const auto& lvl3st = lvl2vec[idlvl2].stResLvL3;
				const auto& lvl3vec = lvl3st.vecResData;
				if (!lvl3vec.empty()) {
					auto data = &lvl3vec.at(idlvl3).stResDataEntry;

					//Send data pointer to CViewRightTR to display raw data.
					m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_HEX_RIGHT_TR, 0),
						reinterpret_cast<CObject*>(const_cast<IMAGE_RESOURCE_DATA_ENTRY*>(data)));
				}
			}
		}
	}
}

void CViewRightTL::CreateListDOSHeader()
{
	const auto& pDosHeader = m_pMainDoc->GetDOSHeader();
	if (!pDosHeader)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_DOSHEADER;
	m_listDOSHeader->Create(m_stlcs);
	m_listDOSHeader->ShowWindow(SW_HIDE);
	m_listDOSHeader->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listDOSHeader->SetHdrColumnColor(0, g_clrOffset);
	m_listDOSHeader->InsertColumn(1, L"Name", LVCFMT_CENTER, 150);
	m_listDOSHeader->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listDOSHeader->InsertColumn(3, L"Value", LVCFMT_CENTER, 100);

	for (auto iter { 0U }; iter < g_mapDOSHeader.size(); ++iter) {
		const auto& ref = g_mapDOSHeader.at(iter);
		const auto dwOffset = ref.dwOffset;
		const auto dwSize = ref.dwSize;

		//Get a pointer to an offset and then take only needed amount of bytes (by &...).
		auto dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(&*pDosHeader) + dwOffset))
			& (DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

		if (iter == 0) //e_magic
			dwValue = (dwValue & 0xFF00) >> 8 | (dwValue & 0xFF) << 8;

		m_listDOSHeader->InsertItem(iter, std::format(L"{:08X}", dwOffset).data());
		m_listDOSHeader->SetItemText(iter, 1, ref.wstrName.data());
		m_listDOSHeader->SetItemText(iter, 2, std::format(L"{}", dwSize).data());
		m_listDOSHeader->SetItemText(iter, 3, std::vformat(dwSize == sizeof(WORD) ? L"{:04X}" : L"{:08X}", std::make_wformat_args(dwValue)).data());
	}
}

void CViewRightTL::CreateListRichHeader()
{
	const auto& pRichHeader = m_pMainDoc->GetRichHeader();
	if (!pRichHeader)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_RICHHEADER;
	m_listRichHdr->Create(m_stlcs);
	m_listRichHdr->ShowWindow(SW_HIDE);
	m_listRichHdr->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listRichHdr->SetHdrColumnColor(0, g_clrOffset);
	m_listRichHdr->InsertColumn(1, L"\u2116", LVCFMT_CENTER, 35);
	m_listRichHdr->InsertColumn(2, L"ID [Hex]", LVCFMT_CENTER, 100);
	m_listRichHdr->InsertColumn(3, L"Version", LVCFMT_CENTER, 100);
	m_listRichHdr->InsertColumn(4, L"Occurrences", LVCFMT_CENTER, 100);
	m_listRichHdr->SetColumnSortMode(1, true, EListExSortMode::SORT_NUMERIC);
	m_listRichHdr->SetColumnSortMode(4, true, EListExSortMode::SORT_NUMERIC);

	auto listindex { 0 };
	for (const auto& iter : *pRichHeader) {
		m_listRichHdr->InsertItem(listindex, std::format(L"{:08X}", iter.dwOffset).data());
		m_listRichHdr->SetItemText(listindex, 1, std::format(L"{}", listindex + 1).data());
		m_listRichHdr->SetItemText(listindex, 2, std::format(L"{:04X}", iter.wId).data());
		m_listRichHdr->SetItemText(listindex, 3, std::format(L"{}", iter.wVersion).data());
		m_listRichHdr->SetItemText(listindex, 4, std::format(L"{}", iter.dwCount).data());
		++listindex;
	}
}

void CViewRightTL::CreateListNTHeader()
{
	const auto& pNTHdr = m_pMainDoc->GetNTHeader();
	if (!pNTHdr)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_NTHEADER;
	m_listNTHeader->Create(m_stlcs);
	m_listNTHeader->ShowWindow(SW_HIDE);
	m_listNTHeader->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listNTHeader->SetHdrColumnColor(0, g_clrOffset);
	m_listNTHeader->InsertColumn(1, L"Name", LVCFMT_CENTER, 100);
	m_listNTHeader->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listNTHeader->InsertColumn(3, L"Value", LVCFMT_CENTER, 100);

	const auto pDescr = &pNTHdr->unHdr.stNTHdr32;
	m_listNTHeader->InsertItem(0, std::format(L"{:08X}", pNTHdr->dwOffset).data());
	m_listNTHeader->SetItemText(0, 1, L"Signature");
	m_listNTHeader->SetItemText(0, 2, std::format(L"{}", sizeof(pDescr->Signature)).data());
	const auto dwSignSwapped = ((pDescr->Signature & 0xFF000000) >> 24) | ((pDescr->Signature & 0x00FF0000) >> 8)
		| ((pDescr->Signature & 0x0000FF00) << 8) | ((pDescr->Signature & 0x000000FF) << 24);
	m_listNTHeader->SetItemText(0, 3, std::format(L"{:08X}", dwSignSwapped).data());

	const auto iterSigASCII = reinterpret_cast<const char*>(&pDescr->Signature);
	const auto iterSigASCIIEnd = iterSigASCII + sizeof(pDescr->Signature);
	m_listNTHeader->SetCellTooltip(0, 3, std::wstring(iterSigASCII, iterSigASCIIEnd), L"Signature as ASCII:");
}

void CViewRightTL::CreateListFileHeader()
{
	const auto& pNTHdr = m_pMainDoc->GetNTHeader();
	if (!pNTHdr)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_FILEHEADER;
	m_listFileHeader->Create(m_stlcs);
	m_listFileHeader->ShowWindow(SW_HIDE);
	m_listFileHeader->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listFileHeader->SetHdrColumnColor(0, g_clrOffset);
	m_listFileHeader->InsertColumn(1, L"Name", LVCFMT_CENTER, 200);
	m_listFileHeader->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listFileHeader->InsertColumn(3, L"Value", LVCFMT_CENTER, 300);

	for (unsigned iter { 0 }; iter < g_mapFileHeader.size(); ++iter) {
		const auto pDescr = &pNTHdr->unHdr.stNTHdr32.FileHeader;
		const auto& ref = g_mapFileHeader.at(iter);
		const auto dwOffset = ref.dwOffset;
		const auto dwSize = ref.dwSize;
		const auto dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(pDescr) + dwOffset))
			& (DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

		m_listFileHeader->InsertItem(iter, std::format(L"{:08X}", pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS32, FileHeader) + dwOffset).data());
		m_listFileHeader->SetItemText(iter, 1, ref.wstrName.data());
		m_listFileHeader->SetItemText(iter, 2, std::format(L"{}", dwSize).data());
		m_listFileHeader->SetItemText(iter, 3, std::vformat(dwSize == sizeof(WORD) ? L"{:04X}" : L"{:08X}", std::make_wformat_args(dwValue)).data());

		if (iter == 0) { //Machine
			if (const auto iterMachine = MapFileHdrMachine.find(pDescr->Machine); iterMachine != MapFileHdrMachine.end())
				m_listFileHeader->SetCellTooltip(iter, 3, iterMachine->second.data(), L"Machine:");
		}
		else if (iter == 2) { //TimeDateStamp	
			if (const auto time = static_cast<__time64_t>(pDescr->TimeDateStamp); time > 0) {
				wchar_t buff[64];
				_wctime64_s(buff, std::size(buff), &time);
				m_listFileHeader->SetCellTooltip(iter, 3, buff, L"Time / Date:");
			}
		}
		else if (iter == 6) { //Characteristics
			std::wstring  wstrCharact;
			for (const auto& flags : MapFileHdrCharact) {
				if (flags.first & pDescr->Characteristics) {
					wstrCharact += flags.second;
					wstrCharact += L"\n";
				}
			}
			if (!wstrCharact.empty()) {
				wstrCharact.erase(wstrCharact.size() - 1); //to remove last '\n'
				m_listFileHeader->SetCellTooltip(iter, 3, wstrCharact.data(), L"Characteristics:");
			}
		}
	}
}

void CViewRightTL::CreateListOptHeader()
{
	const auto& pNTHdr = m_pMainDoc->GetNTHeader();
	if (!pNTHdr)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_OPTIONALHEADER;
	m_listOptHeader->Create(m_stlcs);
	m_listOptHeader->ShowWindow(SW_HIDE);
	m_listOptHeader->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listOptHeader->SetHdrColumnColor(0, g_clrOffset);
	m_listOptHeader->InsertColumn(1, L"Name", LVCFMT_CENTER, 215);
	m_listOptHeader->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listOptHeader->InsertColumn(3, L"Value", LVCFMT_CENTER, 140);

	const auto dwOffsetBase = m_stFileInfo.eFileType == EFileType::PE32 ? pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS32, OptionalHeader) :
		pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS64, OptionalHeader);
	const auto dwSubsystemPos = m_stFileInfo.eFileType == EFileType::PE32 ? 22U : 21U;  //Position in struct is different in x86 and x64.
	const auto dwDllCharactPos = m_stFileInfo.eFileType == EFileType::PE32 ? 23U : 22U; //Position in struct is different in x86 and x64.
	const auto lmbOptHdr = [&](const auto& stOptHdr, const auto& mapOptHdr) {
		for (auto iter { 0U }; iter < mapOptHdr.size(); ++iter) {
			const auto& ref = mapOptHdr.at(iter);
			const auto dwOffset = ref.dwOffset;
			const auto dwSize = ref.dwSize;
			const auto ullValue = *(reinterpret_cast<PULONGLONG>(reinterpret_cast<DWORD_PTR>(&stOptHdr) + dwOffset))
				& (ULONGLONG_MAX >> ((sizeof(ULONGLONG) - dwSize) * 8));

			m_listOptHeader->InsertItem(iter, std::format(L"{:08X}", dwOffsetBase + dwOffset).data());
			m_listOptHeader->SetItemText(iter, 1, ref.wstrName.data());
			m_listOptHeader->SetItemText(iter, 2, std::format(L"{}", dwSize).data());
			m_listOptHeader->SetItemText(iter, 3, std::vformat(dwSize == sizeof(BYTE) ? L"{:02X}"
				: (dwSize == sizeof(WORD) ? L"{:04X}" : (dwSize == sizeof(DWORD) ? L"{:08X}" : L"{:016X}")), std::make_wformat_args(ullValue)).data());

			if (iter == 0) { //Magic.
				if (const auto it = MapOptHdrMagic.find(stOptHdr.Magic); it != MapOptHdrMagic.end())
					m_listOptHeader->SetCellTooltip(iter, 3, it->second.data(), L"Magic:");
			}
			else if (iter == dwSubsystemPos) { //Subsystem.
				if (const auto it = MapOptHdrSubsystem.find(stOptHdr.Subsystem); it != MapOptHdrSubsystem.end())
					m_listOptHeader->SetCellTooltip(iter, 3, it->second.data(), L"Subsystem:");
			}
			else if (iter == dwDllCharactPos) { //DllCharacteristics.
				std::wstring wstrCharact;
				for (const auto& flags : MapOptHdrDllCharact) {
					if (flags.first & stOptHdr.DllCharacteristics) {
						wstrCharact += flags.second;
						wstrCharact += L"\n";
					}
				}
				if (!wstrCharact.empty()) {
					wstrCharact.erase(wstrCharact.size() - 1); //to remove last '\n'
					m_listOptHeader->SetCellTooltip(iter, 3, wstrCharact.data(), L"DllCharacteristics:");
				}
			}
		}
	};
	m_stFileInfo.eFileType == EFileType::PE32 ? lmbOptHdr(pNTHdr->unHdr.stNTHdr32.OptionalHeader, g_mapOptHeader32)
		: lmbOptHdr(pNTHdr->unHdr.stNTHdr64.OptionalHeader, g_mapOptHeader64);
}

void CViewRightTL::CreateListDataDirs()
{
	const auto& pvecDataDirs = m_pMainDoc->GetDataDirs();
	if (!pvecDataDirs)
		return;

	const auto& pNTHdr = m_pMainDoc->GetNTHeader();
	if (!pNTHdr)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_DATADIRECTORIES;
	m_listDataDirs->Create(m_stlcs);
	m_listDataDirs->ShowWindow(SW_HIDE);
	m_listDataDirs->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listDataDirs->SetHdrColumnColor(0, g_clrOffset);
	m_listDataDirs->InsertColumn(1, L"Name", LVCFMT_CENTER, 200);
	m_listDataDirs->InsertColumn(2, L"Directory RVA", LVCFMT_CENTER, 100);
	m_listDataDirs->InsertColumn(3, L"Directory Size", LVCFMT_CENTER, 100);
	m_listDataDirs->InsertColumn(4, L"Resides in Section", LVCFMT_CENTER, 125);

	const auto dwDataDirsOffset = m_stFileInfo.eFileType == EFileType::PE32 ? offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory) :
		offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory);
	for (auto iter { 0U }; iter < pvecDataDirs->size(); ++iter) {
		const auto& ref = pvecDataDirs->at(static_cast<size_t>(iter));
		const auto pDescr = &ref.stDataDir;

		m_listDataDirs->InsertItem(iter, std::format(L"{:08X}", pNTHdr->dwOffset + dwDataDirsOffset + sizeof(IMAGE_DATA_DIRECTORY) * iter).data());
		m_listDataDirs->SetItemText(iter, 1, g_mapDataDirs.at(static_cast<WORD>(iter)).data());
		m_listDataDirs->SetItemText(iter, 2, std::format(L"{:08X}", pDescr->VirtualAddress).data());
		if (iter == IMAGE_DIRECTORY_ENTRY_SECURITY && pDescr->VirtualAddress > 0)
			m_listDataDirs->SetCellTooltip(iter, 2, L"This address is the file's raw offset on disk.");

		m_listDataDirs->SetItemText(iter, 3, std::format(L"{:08X}", pDescr->Size).data());
		if (!ref.strSection.empty()) { //Resides in Section.
			m_listDataDirs->SetItemText(iter, 4, std::format(L"{:.8}", StrToWstr(ref.strSection)).data());
		}
	}
}

void CViewRightTL::CreateListSecHeaders()
{
	const auto& pSecHeaders = m_pMainDoc->GetSecHeaders();
	if (!pSecHeaders)
		return;

	m_stlcs.dwStyle = LVS_OWNERDATA;
	m_stlcs.uID = IDC_LIST_SECHEADERS;
	m_listSecHeaders->Create(m_stlcs);
	m_listSecHeaders->ShowWindow(SW_HIDE);
	m_listSecHeaders->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
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
	m_listSecHeaders->SetItemCountEx(static_cast<int>(pSecHeaders->size()), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
}

void CViewRightTL::CreateListExport()
{
	const auto& pExport = m_pMainDoc->GetExport();
	if (!pExport)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_EXPORT;
	m_listExportDir->Create(m_stlcs);
	m_listExportDir->ShowWindow(SW_HIDE);
	m_listExportDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listExportDir->SetHdrColumnColor(0, g_clrOffset);
	m_listExportDir->InsertColumn(1, L"Name", LVCFMT_CENTER, 250);
	m_listExportDir->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listExportDir->InsertColumn(3, L"Value", LVCFMT_CENTER, 300);

	const auto pDescr = &pExport->stExportDesc;
	for (auto iter { 0U }; iter < g_mapExport.size(); ++iter) {
		const auto& ref = g_mapExport.at(iter);
		const auto dwOffset = ref.dwOffset;
		const auto dwSize = ref.dwSize;
		const auto dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(pDescr) + dwOffset))
			& (DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

		m_listExportDir->InsertItem(iter, std::format(L"{:08X}", pExport->dwOffset + dwOffset).data());
		m_listExportDir->SetItemText(iter, 1, ref.wstrName.data());
		m_listExportDir->SetItemText(iter, 2, std::format(L"{}", dwSize).data());

		if (iter == 4) //Name
			m_listExportDir->SetItemText(iter, 3, std::format(L"{:08X} ({})", dwValue, StrToWstr(pExport->strModuleName)).data());
		else
			m_listExportDir->SetItemText(iter, 3, std::vformat(dwSize == sizeof(WORD) ? L"{:04X}" : L"{:08X}", std::make_wformat_args(dwValue)).data());

		if (const auto time = static_cast<__time64_t>(pDescr->TimeDateStamp); iter == 1 && time > 0) { //TimeDate
			wchar_t buff[64];
			_wctime64_s(buff, std::size(buff), &time);
			m_listExportDir->SetCellTooltip(iter, 3, buff, L"Time / Date:");
		}
	}
}

void CViewRightTL::CreateListImport()
{
	const auto& pImport = m_pMainDoc->GetImport();
	if (!pImport)
		return;

	m_stlcs.dwStyle = LVS_OWNERDATA;
	m_stlcs.uID = IDC_LIST_IMPORT;
	m_listImport->Create(m_stlcs);
	m_listImport->ShowWindow(SW_HIDE);
	m_listImport->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listImport->SetHdrColumnColor(0, g_clrOffset);
	m_listImport->InsertColumn(1, L"Module Name (funcs number)", LVCFMT_CENTER, 300);
	m_listImport->InsertColumn(2, L"OriginalFirstThunk\n(Import Lookup Table)", LVCFMT_CENTER, 170);
	m_listImport->InsertColumn(3, L"TimeDateStamp", LVCFMT_CENTER, 115);
	m_listImport->InsertColumn(4, L"ForwarderChain", LVCFMT_CENTER, 110);
	m_listImport->InsertColumn(5, L"Name RVA", LVCFMT_CENTER, 90);
	m_listImport->InsertColumn(6, L"FirstThunk (IAT)", LVCFMT_CENTER, 135);
	m_listImport->SetItemCountEx(static_cast<int>(pImport->size()), LVSICF_NOSCROLL);
}

void CViewRightTL::CreateTreeResources()
{
	const auto& pResRoot = m_pMainDoc->GetResources();
	if (!pResRoot)
		return;

	m_treeResTop.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CRect(0, 0, 0, 0), this, IDC_TREE_RESOURCE_TOP);
	m_treeResTop.ShowWindow(SW_HIDE);
	m_hTreeResRoot = m_treeResTop.InsertItem(L""); //Actual name is set at the bottom of this method.

	auto ilvlRoot { 0 };
	int iResTotal { };
	for (const auto& iterRoot : pResRoot->vecResData) //Main loop to extract Resources.
	{
		const auto pResDirEntryRoot = &iterRoot.stResDirEntry; //Level Root IMAGE_RESOURCE_DIRECTORY_ENTRY
		std::wstring wstr;
		if (pResDirEntryRoot->NameIsString)
			wstr = std::format(L"Entry: {} [Name: {}]", ilvlRoot, iterRoot.wstrResName);
		else {
			if (const auto iter = MapResID.find(pResDirEntryRoot->Id); iter != MapResID.end())
				wstr = std::format(L"Entry: {} [Id: {}, {}]", ilvlRoot, pResDirEntryRoot->Id, iter->second);
			else
				wstr = std::format(L"Entry: {} [Id: {}]", ilvlRoot, pResDirEntryRoot->Id);
		}

		HTREEITEM hTreeTop;
		if (pResDirEntryRoot->DataIsDirectory) {
			hTreeTop = m_treeResTop.InsertItem(wstr.data(), m_hTreeResRoot);
			m_vecResId.emplace_back(ilvlRoot, -1, -1);
			m_treeResTop.SetItemData(hTreeTop, m_vecResId.size() - 1);

			auto ilvl2 { 0 };
			const auto pstResLvL2 = &iterRoot.stResLvL2;
			for (const auto& iterLvL2 : pstResLvL2->vecResData) {
				const auto pResDirEntry2 = &iterLvL2.stResDirEntry; //Level 2 IMAGE_RESOURCE_DIRECTORY_ENTRY
				if (pResDirEntry2->NameIsString)
					wstr = std::format(L"Entry: {} Name: {}", ilvl2, iterLvL2.wstrResName);
				else
					wstr = std::format(L"Entry: {} Id: {}", ilvl2, pResDirEntry2->Id);

				HTREEITEM hTreeLvL2;
				if (pResDirEntry2->DataIsDirectory) {
					hTreeLvL2 = m_treeResTop.InsertItem(wstr.data(), hTreeTop);
					m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
					m_treeResTop.SetItemData(hTreeLvL2, m_vecResId.size() - 1);

					auto ilvl3 { 0 };
					const auto pstResLvL3 = &iterLvL2.stResLvL3;
					for (const auto& iterLvL3 : pstResLvL3->vecResData) {
						const auto pResDirEntry3 = &iterLvL3.stResDirEntry; //Level 3 IMAGE_RESOURCE_DIRECTORY_ENTRY
						if (pResDirEntry3->NameIsString)
							wstr = std::format(L"Entry: {} Name: {}", ilvl3, iterLvL3.wstrResName);
						else
							wstr = std::format(L"Entry: {} Id: {}", ilvl3, pResDirEntry3->Id);

						const auto htreeLvL3 = m_treeResTop.InsertItem(wstr.data(), hTreeLvL2);
						m_vecResId.emplace_back(ilvlRoot, ilvl2, ilvl3);
						m_treeResTop.SetItemData(htreeLvL3, m_vecResId.size() - 1);
						++iResTotal;
						++ilvl3;
					}
				}
				else { //DATA lvl2
					hTreeLvL2 = m_treeResTop.InsertItem(wstr.data(), hTreeTop);
					m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
					m_treeResTop.SetItemData(hTreeLvL2, m_vecResId.size() - 1);
					++iResTotal;
				}
				++ilvl2;
			}
		}
		else { //DATA lvlroot
			hTreeTop = m_treeResTop.InsertItem(wstr.data(), m_hTreeResRoot);
			m_vecResId.emplace_back(ilvlRoot, -1, -1);
			m_treeResTop.SetItemData(hTreeTop, m_vecResId.size() - 1);
			++iResTotal;
		}
		++ilvlRoot;
	}
	m_treeResTop.SetItemText(m_hTreeResRoot, std::format(L"Resources total: {}", iResTotal).data());
	m_treeResTop.Expand(m_hTreeResRoot, TVE_EXPAND);
}

void CViewRightTL::CreateListExceptions()
{
	const auto& pExceptionDir = m_pMainDoc->GetExceptions();
	if (!pExceptionDir)
		return;

	m_stlcs.dwStyle = LVS_OWNERDATA;
	m_stlcs.uID = IDC_LIST_EXCEPTIONS;
	m_listExceptionDir->Create(m_stlcs);
	m_listExceptionDir->ShowWindow(SW_HIDE);
	m_listExceptionDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listExceptionDir->SetHdrColumnColor(0, g_clrOffset);
	m_listExceptionDir->InsertColumn(1, L"BeginAddress", LVCFMT_CENTER, 100);
	m_listExceptionDir->InsertColumn(2, L"EndAddress", LVCFMT_CENTER, 100);
	m_listExceptionDir->InsertColumn(3, L"UnwindData/InfoAddress", LVCFMT_CENTER, 180);
	m_listExceptionDir->SetItemCountEx(static_cast<int>(pExceptionDir->size()), LVSICF_NOSCROLL);
}

void CViewRightTL::CreateListSecurity()
{
	const auto& pSecurityDir = m_pMainDoc->GetSecurity();
	if (!pSecurityDir)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_SECURITY;
	m_listSecurityDir->Create(m_stlcs);
	m_listSecurityDir->ShowWindow(SW_HIDE);
	m_listSecurityDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listSecurityDir->SetHdrColumnColor(0, g_clrOffset);
	m_listSecurityDir->InsertColumn(1, L"dwLength", LVCFMT_CENTER, 100);
	m_listSecurityDir->InsertColumn(2, L"wRevision", LVCFMT_CENTER, 100);
	m_listSecurityDir->InsertColumn(3, L"wCertificateType", LVCFMT_CENTER, 180);

	int listindex { };
	for (const auto& iter : *pSecurityDir) {
		m_listSecurityDir->InsertItem(listindex, std::format(L"{:08X}", iter.dwOffset).data());
		const auto pDescr = &iter.stWinSert;
		m_listSecurityDir->SetItemText(listindex, 1, std::format(L"{:08X}", pDescr->dwLength).data());
		m_listSecurityDir->SetItemText(listindex, 2, std::format(L"{:04X}", pDescr->wRevision).data());
		if (const auto iterRevision = MapWinCertRevision.find(pDescr->wRevision); iterRevision != MapWinCertRevision.end())
			m_listSecurityDir->SetCellTooltip(listindex, 2, iterRevision->second.data(), L"Certificate revision:");
		m_listSecurityDir->SetItemText(listindex, 3, std::format(L"{:04X}", pDescr->wCertificateType).data());
		if (const auto iterType = MapWinCertType.find(pDescr->wCertificateType); iterType != MapWinCertType.end())
			m_listSecurityDir->SetCellTooltip(listindex, 3, iterType->second.data(), L"Certificate type:");

		++listindex;
	}
}

void CViewRightTL::CreateListRelocations()
{
	const auto& pRelocTable = m_pMainDoc->GetRelocations();
	if (!pRelocTable)
		return;

	m_stlcs.dwStyle = LVS_OWNERDATA;
	m_stlcs.uID = IDC_LIST_RELOCATIONS;
	m_listRelocDir->Create(m_stlcs);
	m_listRelocDir->ShowWindow(SW_HIDE);
	m_listRelocDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listRelocDir->SetHdrColumnColor(0, g_clrOffset);
	m_listRelocDir->InsertColumn(1, L"Virtual Address", LVCFMT_CENTER, 115);
	m_listRelocDir->InsertColumn(2, L"Block Size", LVCFMT_CENTER, 100);
	m_listRelocDir->InsertColumn(3, L"Entries", LVCFMT_CENTER, 100);
	m_listRelocDir->SetItemCountEx(static_cast<int>(pRelocTable->size()), LVSICF_NOSCROLL);
}

void CViewRightTL::CreateListDebug()
{
	const auto& pDebugDir = m_pMainDoc->GetDebug();
	if (!pDebugDir)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_DEBUG;
	m_listDebugDir->Create(m_stlcs);
	m_listDebugDir->ShowWindow(SW_HIDE);
	m_listDebugDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listDebugDir->SetHdrColumnColor(0, g_clrOffset);
	m_listDebugDir->InsertColumn(1, L"Characteristics", LVCFMT_CENTER, 115);
	m_listDebugDir->InsertColumn(2, L"TimeDateStamp", LVCFMT_CENTER, 150);
	m_listDebugDir->InsertColumn(3, L"MajorVersion", LVCFMT_CENTER, 100);
	m_listDebugDir->InsertColumn(4, L"MinorVersion", LVCFMT_CENTER, 100);
	m_listDebugDir->InsertColumn(5, L"Type", LVCFMT_CENTER, 90);
	m_listDebugDir->InsertColumn(6, L"SizeOfData", LVCFMT_CENTER, 100);
	m_listDebugDir->InsertColumn(7, L"AddressOfRawData", LVCFMT_CENTER, 170);
	m_listDebugDir->InsertColumn(8, L"PointerToRawData", LVCFMT_CENTER, 140);

	int listindex { 0 };
	for (const auto& iter : *pDebugDir) {
		const auto pDescr = &iter.stDebugDir;

		m_listDebugDir->InsertItem(listindex, std::format(L"{:08X}", iter.dwOffset).data());
		m_listDebugDir->SetItemText(listindex, 1, std::format(L"{:08X}", pDescr->Characteristics).data());
		m_listDebugDir->SetItemText(listindex, 2, std::format(L"{:08X}", pDescr->TimeDateStamp).data());
		if (const auto time = static_cast<__time64_t>(pDescr->TimeDateStamp); time > 0) {
			wchar_t buff[64];
			_wctime64_s(buff, std::size(buff), &time);
			m_listDebugDir->SetCellTooltip(listindex, 2, buff, L"Time / Date:");
		}
		m_listDebugDir->SetItemText(listindex, 3, std::format(L"{:04X}", pDescr->MajorVersion).data());
		m_listDebugDir->SetItemText(listindex, 4, std::format(L"{:04X}", pDescr->MinorVersion).data());
		m_listDebugDir->SetItemText(listindex, 5, std::format(L"{:08X}", pDescr->Type).data());
		if (const auto iterDType = MapDbgType.find(pDescr->Type); iterDType != MapDbgType.end())
			m_listDebugDir->SetCellTooltip(listindex, 5, iterDType->second.data(), L"Debug type:");
		m_listDebugDir->SetItemText(listindex, 6, std::format(L"{:08X}", pDescr->SizeOfData).data());
		m_listDebugDir->SetItemText(listindex, 7, std::format(L"{:08X}", pDescr->AddressOfRawData).data());
		m_listDebugDir->SetItemText(listindex, 8, std::format(L"{:08X}", pDescr->PointerToRawData).data());
		++listindex;
	}
}

void CViewRightTL::CreateListTLS()
{
	const auto& pTLSDir = m_pMainDoc->GetTLS();
	if (!pTLSDir)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_TLS;
	m_listTLSDir->Create(m_stlcs);
	m_listTLSDir->ShowWindow(SW_HIDE);
	m_listTLSDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listTLSDir->SetHdrColumnColor(0, g_clrOffset);
	m_listTLSDir->InsertColumn(1, L"Name", LVCFMT_CENTER, 250);
	m_listTLSDir->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 110);
	m_listTLSDir->InsertColumn(3, L"Value", LVCFMT_CENTER, 150);

	const auto lmbTLS = [&](const auto& stPETLS, const auto& mapTLS) {
		for (auto iterMap { 0U }; iterMap < mapTLS.size(); ++iterMap) {
			const auto& ref = mapTLS.at(iterMap);
			const auto dwOffset = ref.dwOffset;
			const auto dwSize = ref.dwSize;
			const auto ullValue = *(reinterpret_cast<PULONGLONG>(reinterpret_cast<DWORD_PTR>(&stPETLS) + dwOffset))
				& (ULONGLONG_MAX >> ((sizeof(ULONGLONG) - dwSize) * 8)); //Masking to remove structure's adjacent fields data.

			m_listTLSDir->InsertItem(iterMap, std::format(L"{:08X}", pTLSDir->dwOffset + dwOffset).data());
			m_listTLSDir->SetItemText(iterMap, 1, ref.wstrName.data());
			m_listTLSDir->SetItemText(iterMap, 2, std::format(L"{}", dwSize).data());
			m_listTLSDir->SetItemText(iterMap, 3, std::vformat(dwSize == sizeof(DWORD) ? L"{:08X}" : L"{:016X}", std::make_wformat_args(ullValue)).data());

			if (iterMap == 5) { //Characteristics
				if (const auto iterCharact = MapTLSCharact.find(stPETLS.Characteristics); iterCharact != MapTLSCharact.end())
					m_listTLSDir->SetCellTooltip(iterMap, 3, iterCharact->second.data(), L"Characteristics:");
			}
		}
	};
	m_stFileInfo.eFileType == EFileType::PE32 ? lmbTLS(pTLSDir->unTLS.stTLSDir32, g_mapTLS32) : lmbTLS(pTLSDir->unTLS.stTLSDir64, g_mapTLS64);
}

void CViewRightTL::CreateListLCD()
{
	const auto& pLCD = m_pMainDoc->GetLoadConfig();
	if (!pLCD)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_LOADCONFIG;
	m_listLCD->Create(m_stlcs);
	m_listLCD->ShowWindow(SW_HIDE);
	m_listLCD->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listLCD->SetHdrColumnColor(0, g_clrOffset);
	m_listLCD->InsertColumn(1, L"Name", LVCFMT_CENTER, 330);
	m_listLCD->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 110);
	m_listLCD->InsertColumn(3, L"Value", LVCFMT_CENTER, 300);

	const auto lmbLCD = [&](const auto& stPELCD, const auto& mapLCD) {
		for (auto iterMap { 0U }; iterMap < mapLCD.size(); ++iterMap) {
			const auto& ref = mapLCD.at(iterMap);
			if (ref.dwOffset >= stPELCD.Size) //No-more than the size of the struct, that is the first struct's member.
				break;

			const auto dwOffset = ref.dwOffset;
			const auto dwSize = ref.dwSize;
			const auto ullValue = *(reinterpret_cast<PULONGLONG>(reinterpret_cast<DWORD_PTR>(&stPELCD) + dwOffset))
				& (ULONGLONG_MAX >> ((sizeof(ULONGLONG) - dwSize) * 8)); //Masking to remove structure's adjacent fields data.

			m_listLCD->InsertItem(iterMap, std::format(L"{:08X}", pLCD->dwOffset + dwOffset).data());
			m_listLCD->SetItemText(iterMap, 1, ref.wstrName.data());
			m_listLCD->SetItemText(iterMap, 2, std::format(L"{}", dwSize).data());
			m_listLCD->SetItemText(iterMap, 3, std::vformat(dwSize == sizeof(WORD) ? L"{:04X}" : (dwSize == sizeof(DWORD) ? L"{:08X}" : L"{:016X}"),
				std::make_wformat_args(ullValue)).data());

			if (iterMap == 1) { //TimeDateStamp
				if (const auto time = static_cast<__time64_t>(stPELCD.TimeDateStamp); stPELCD.TimeDateStamp > 0) {
					wchar_t buff[64];
					_wctime64_s(buff, std::size(buff), &time);
					m_listLCD->SetCellTooltip(iterMap, 2, buff, L"Time / Date:");
				}
			}
			else if (iterMap == 24) { //GuardFlags
				std::wstring wstrGFlags;
				for (const auto& flags : MapLCDGuardFlags) {
					if (flags.first & stPELCD.GuardFlags) {
						wstrGFlags += flags.second;
						wstrGFlags += L"\n";
					}
				}
				if (!wstrGFlags.empty())
					m_listLCD->SetCellTooltip(iterMap, 3, wstrGFlags.data(), L"GuardFlags:");
			}
		}
	};
	m_stFileInfo.eFileType == EFileType::PE32 ? lmbLCD(pLCD->unLCD.stLCD32, g_mapLCD32) : lmbLCD(pLCD->unLCD.stLCD64, g_mapLCD64);
}

void CViewRightTL::CreateListBoundImport()
{
	const auto& pBoundImp = m_pMainDoc->GetBoundImport();
	if (!pBoundImp)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_BOUNDIMPORT;
	m_listBoundImportDir->Create(m_stlcs);
	m_listBoundImportDir->ShowWindow(SW_HIDE);
	m_listBoundImportDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listBoundImportDir->SetHdrColumnColor(0, g_clrOffset);
	m_listBoundImportDir->InsertColumn(1, L"Module Name", LVCFMT_CENTER, 290);
	m_listBoundImportDir->InsertColumn(2, L"TimeDateStamp", LVCFMT_CENTER, 130);
	m_listBoundImportDir->InsertColumn(3, L"OffsetModuleName", LVCFMT_CENTER, 140);
	m_listBoundImportDir->InsertColumn(4, L"NumberOfModuleForwarderRefs", LVCFMT_CENTER, 220);

	int listindex { };
	for (const auto& iter : *pBoundImp) {
		m_listBoundImportDir->InsertItem(listindex, std::format(L"{:08X}", iter.dwOffset).data());

		const auto pDescr = &iter.stBoundImpDesc;
		m_listBoundImportDir->SetItemText(listindex, 1, StrToWstr(iter.strBoundName).data());
		m_listBoundImportDir->SetItemText(listindex, 2, std::format(L"{:08X}", pDescr->TimeDateStamp).data());
		if (const auto time = static_cast<__time64_t>(pDescr->TimeDateStamp); time > 0) {
			wchar_t buff[64];
			_wctime64_s(buff, std::size(buff), &time);
			m_listBoundImportDir->SetCellTooltip(listindex, 2, buff, L"Time / Date:");
		}
		m_listBoundImportDir->SetItemText(listindex, 3, std::format(L"{:04X}", pDescr->OffsetModuleName).data());
		m_listBoundImportDir->SetItemText(listindex, 4, std::format(L"{:04X}", pDescr->NumberOfModuleForwarderRefs).data());
		++listindex;
	}
}

void CViewRightTL::CreateListDelayImport()
{
	const auto& pDelayImp = m_pMainDoc->GetDelayImport();
	if (!pDelayImp)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_DELAYIMPORT;
	m_listDelayImportDir->Create(m_stlcs);
	m_listDelayImportDir->ShowWindow(SW_HIDE);
	m_listDelayImportDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
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

	int listindex { };
	for (const auto& iter : *pDelayImp) {
		m_listDelayImportDir->InsertItem(listindex, std::format(L"{:08X}", iter.dwOffset).data());

		const auto pDescr = &iter.stDelayImpDesc;
		m_listDelayImportDir->SetItemText(listindex, 1, std::format(L"{} ({})", StrToWstr(iter.strModuleName), iter.vecDelayImpFunc.size()).data());
		m_listDelayImportDir->SetItemText(listindex, 2, std::format(L"{:08X}", pDescr->Attributes.AllAttributes).data());
		m_listDelayImportDir->SetItemText(listindex, 3, std::format(L"{:08X}", pDescr->DllNameRVA).data());
		m_listDelayImportDir->SetItemText(listindex, 4, std::format(L"{:08X}", pDescr->ModuleHandleRVA).data());
		m_listDelayImportDir->SetItemText(listindex, 5, std::format(L"{:08X}", pDescr->ImportAddressTableRVA).data());
		m_listDelayImportDir->SetItemText(listindex, 6, std::format(L"{:08X}", pDescr->ImportNameTableRVA).data());
		m_listDelayImportDir->SetItemText(listindex, 7, std::format(L"{:08X}", pDescr->BoundImportAddressTableRVA).data());
		m_listDelayImportDir->SetItemText(listindex, 8, std::format(L"{:08X}", pDescr->UnloadInformationTableRVA).data());
		m_listDelayImportDir->SetItemText(listindex, 9, std::format(L"{:08X}", pDescr->TimeDateStamp).data());
		if (const auto time = static_cast<__time64_t>(pDescr->TimeDateStamp); time > 0) {
			wchar_t buff[64];
			_wctime64_s(buff, std::size(buff), &time);
			m_listDelayImportDir->SetCellTooltip(listindex, 8, buff, L"Time / Date:");
		}
		++listindex;
	}
}

void CViewRightTL::CreateListCOM()
{
	const auto& pCOMDesc = m_pMainDoc->GetCOMDescriptor();
	if (!pCOMDesc)
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_COMDESCRIPTOR;
	m_listCOMDir->Create(m_stlcs);
	m_listCOMDir->ShowWindow(SW_HIDE);
	m_listCOMDir->InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listCOMDir->SetHdrColumnColor(0, g_clrOffset);
	m_listCOMDir->InsertColumn(1, L"Name", LVCFMT_CENTER, 300);
	m_listCOMDir->InsertColumn(2, L"Size [BYTES]", LVCFMT_CENTER, 100);
	m_listCOMDir->InsertColumn(3, L"Value", LVCFMT_CENTER, 300);

	for (auto iter = 0U; iter < g_mapComDir.size(); ++iter) {
		const auto& ref = g_mapComDir.at(iter);
		const auto dwOffset = ref.dwOffset;
		const auto dwSize = ref.dwSize;
		const auto dwValue = *(reinterpret_cast<PDWORD>(reinterpret_cast<DWORD_PTR>(&pCOMDesc->stCorHdr) + dwOffset))
			& (DWORD_MAX >> ((sizeof(DWORD) - dwSize) * 8));

		m_listCOMDir->InsertItem(iter, std::format(L"{:08X}", pCOMDesc->dwOffset + dwOffset).data());
		m_listCOMDir->SetItemText(iter, 1, ref.wstrName.data());
		m_listCOMDir->SetItemText(iter, 2, std::format(L"{}", dwSize).data());
		m_listCOMDir->SetItemText(iter, 3, std::vformat(dwSize == sizeof(WORD) ? L"{:04X}" : L"{:08X}", std::make_wformat_args(dwValue)).data());

		if (iter == 5) {
			std::wstring wstrFlags;
			for (const auto& flags : MapCOR20Flags) {
				if (flags.first & pCOMDesc->stCorHdr.Flags) {
					wstrFlags += flags.second;
					wstrFlags += L"\n";
				}
			}
			if (!wstrFlags.empty())
				m_listCOMDir->SetCellTooltip(iter, 3, wstrFlags.data(), L"Flags:");
		}
	}
}

void CViewRightTL::SortImportData()
{
	auto& pImport = m_pMainDoc->GetImport();
	std::sort(pImport->begin(), pImport->end(),
		[&](const auto& ref1, const auto& ref2) {
			int iCompare { };
			switch (m_listImport->GetSortColumn()) {
			case 0:
				iCompare = ref1.dwOffset < ref2.dwOffset ? -1 : 1;
				break;
			case 1:
				iCompare = ref1.strModuleName.compare(ref2.strModuleName);
				break;
			case 2:
				iCompare = ref1.stImportDesc.OriginalFirstThunk < ref2.stImportDesc.OriginalFirstThunk ? -1 : 1;
				break;
			case 3:
				iCompare = ref1.stImportDesc.TimeDateStamp < ref2.stImportDesc.TimeDateStamp ? -1 : 1;
				break;
			case 4:
				iCompare = ref1.stImportDesc.ForwarderChain < ref2.stImportDesc.ForwarderChain ? -1 : 1;
				break;
			case 5:
				iCompare = ref1.stImportDesc.Name < ref2.stImportDesc.Name ? -1 : 1;
				break;
			case 6:
				iCompare = ref1.stImportDesc.FirstThunk < ref2.stImportDesc.FirstThunk ? -1 : 1;
				break;
			}

			bool result { false };
			if (m_listImport->GetSortAscending()) {
				if (iCompare < 0)
					result = true;
			}
			else {
				if (iCompare > 0)
					result = true;
			}

			return result;
		});

	m_listImport->RedrawWindow();
}