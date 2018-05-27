#include "stdafx.h"
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "PepperTreeCtrl.h"
#include "PepperList.h"
#include "ViewRightTop.h"

IMPLEMENT_DYNCREATE(CViewRightTop, CView)

BEGIN_MESSAGE_MAP(CViewRightTop, CView)
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_NOTIFY(LVN_GETDISPINFO, LISTID_SECHEADERS, OnListSectionsGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFO, LISTID_IMPORT_DIR, OnListImportGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFO, LISTID_RELOCATION_DIR, OnListRelocGetDispInfo)
	ON_NOTIFY(LVN_GETDISPINFO, LISTID_EXCEPTION_DIR, OnListExceptionGetDispInfo)
END_MESSAGE_MAP()

void CViewRightTop::OnDraw(CDC* pDC)
{
	if (m_fFileSummaryShow)
	{
		pDC->SelectObject(m_fontSummary);
		pDC->Rectangle(20, 20, 400, 150);
		GetTextExtentPoint32W(pDC->m_hDC, m_strFileType.c_str(), m_strFileType.length(), &m_sizeLineDistance);
		pDC->SetTextColor(RGB(200, 50, 30));
		ExtTextOutW(pDC->m_hDC, 125, 10, 0, nullptr, L"Pepper 0.1 (beta)", 17, nullptr);
		pDC->SetTextColor(RGB(0, 0, 255));
		ExtTextOutW(pDC->m_hDC, 35, 25 + m_sizeLineDistance.cy, 0, nullptr, m_strFileType.c_str(), m_strFileType.length(), nullptr);
		ExtTextOutW(pDC->m_hDC, 35, 55 + m_sizeLineDistance.cy, 0, nullptr, m_strFileName.c_str(), m_strFileName.length(), nullptr);
	}
}

void CViewRightTop::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_ChildFrame = (CChildFrame*)GetParentFrame();
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
	m_strFileName.insert(0, TEXT("File name: "));

	const DWORD* m_pFileSummary { };
	if (m_pLibpe->GetFileSummary(&m_pFileSummary) != S_OK)
		return;

	m_dwFileSummary = *m_pFileSummary;

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
		m_strFileType = TEXT("File type: PE32 (x86)");
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
		m_strFileType = TEXT("File type: PE32+ (x64)");

	m_pLibpe->GetSectionHeaders(&m_pSectianHeaders);
	m_pLibpe->GetImportTable(&m_pImportTable);
	m_pLibpe->GetExceptionTable(&m_pExceptionDir);
	m_pLibpe->GetRelocationTable(&m_pRelocTable);

	m_mapSecFlags = {
		{ 0x00000000, TEXT("IMAGE_SCN_TYPE_REG\n Reserved.") },
	{ 0x00000001, TEXT("IMAGE_SCN_TYPE_DSECT\n Reserved.") },
	{ 0x00000002, TEXT("IMAGE_SCN_TYPE_NOLOAD\n Reserved.") },
	{ 0x00000004, TEXT("IMAGE_SCN_TYPE_GROUP\n Reserved.") },
	{ IMAGE_SCN_TYPE_NO_PAD, TEXT("IMAGE_SCN_TYPE_NO_PAD\n Reserved.") },
	{ 0x00000010, TEXT("IMAGE_SCN_TYPE_COPY\n Reserved.") },
	{ IMAGE_SCN_CNT_CODE, TEXT("IMAGE_SCN_CNT_CODE\n Section contains code.") },
	{ IMAGE_SCN_CNT_INITIALIZED_DATA, TEXT("IMAGE_SCN_CNT_INITIALIZED_DATA\n Section contains initialized data.") },
	{ IMAGE_SCN_CNT_UNINITIALIZED_DATA, TEXT("IMAGE_SCN_CNT_UNINITIALIZED_DATA\n Section contains uninitialized data.") },
	{ IMAGE_SCN_LNK_OTHER, TEXT("IMAGE_SCN_LNK_OTHER\n Reserved.") },
	{ IMAGE_SCN_LNK_INFO, TEXT("IMAGE_SCN_LNK_INFO\n Section contains comments or some other type of information.") },
	{ 0x00000400, TEXT("IMAGE_SCN_TYPE_OVER\n Reserved.") },
	{ IMAGE_SCN_LNK_REMOVE, TEXT("IMAGE_SCN_LNK_REMOVE\n Section contents will not become part of image.") },
	{ IMAGE_SCN_LNK_COMDAT, TEXT("IMAGE_SCN_LNK_COMDAT\n Section contents comdat.") },
	{ IMAGE_SCN_NO_DEFER_SPEC_EXC, TEXT("IMAGE_SCN_NO_DEFER_SPEC_EXC\n Reset speculative exceptions handling bits in the TLB entries for this section.") },
	{ IMAGE_SCN_GPREL, TEXT("IMAGE_SCN_GPREL\n Section content can be accessed relative to GP") },
	{ 0x00010000, TEXT("IMAGE_SCN_MEM_SYSHEAP\n Obsolete") },
	{ IMAGE_SCN_MEM_PURGEABLE, TEXT("IMAGE_SCN_MEM_PURGEABLE") },
	{ IMAGE_SCN_MEM_LOCKED, TEXT("IMAGE_SCN_MEM_LOCKED") },
	{ IMAGE_SCN_MEM_PRELOAD, TEXT("IMAGE_SCN_MEM_PRELOAD") },
	{ IMAGE_SCN_ALIGN_1BYTES, TEXT("IMAGE_SCN_ALIGN_1BYTES") },
	{ IMAGE_SCN_ALIGN_2BYTES, TEXT("IMAGE_SCN_ALIGN_2BYTES") },
	{ IMAGE_SCN_ALIGN_4BYTES, TEXT("IMAGE_SCN_ALIGN_4BYTES") },
	{ IMAGE_SCN_ALIGN_8BYTES, TEXT("IMAGE_SCN_ALIGN_8BYTES") },
	{ IMAGE_SCN_ALIGN_16BYTES, TEXT("IMAGE_SCN_ALIGN_16BYTES\n Default alignment if no others are specified.") },
	{ IMAGE_SCN_ALIGN_32BYTES, TEXT("IMAGE_SCN_ALIGN_32BYTES") },
	{ IMAGE_SCN_ALIGN_64BYTES, TEXT("IMAGE_SCN_ALIGN_64BYTES") },
	{ IMAGE_SCN_ALIGN_128BYTES, TEXT("IMAGE_SCN_ALIGN_128BYTES") },
	{ IMAGE_SCN_ALIGN_256BYTES, TEXT("IMAGE_SCN_ALIGN_256BYTES") },
	{ IMAGE_SCN_ALIGN_512BYTES, TEXT("IMAGE_SCN_ALIGN_512BYTES") },
	{ IMAGE_SCN_ALIGN_1024BYTES, TEXT("IMAGE_SCN_ALIGN_1024BYTES") },
	{ IMAGE_SCN_ALIGN_2048BYTES, TEXT("IMAGE_SCN_ALIGN_2048BYTES") },
	{ IMAGE_SCN_ALIGN_4096BYTES, TEXT("IMAGE_SCN_ALIGN_4096BYTES") },
	{ IMAGE_SCN_ALIGN_8192BYTES, TEXT("IMAGE_SCN_ALIGN_8192BYTES") },
	{ IMAGE_SCN_ALIGN_MASK, TEXT("IMAGE_SCN_ALIGN_MASK") },
	{ IMAGE_SCN_LNK_NRELOC_OVFL, TEXT("IMAGE_SCN_LNK_NRELOC_OVFL\n Section contains extended relocations.") },
	{ IMAGE_SCN_MEM_DISCARDABLE, TEXT("IMAGE_SCN_MEM_DISCARDABLE\n Section can be discarded.") },
	{ IMAGE_SCN_MEM_NOT_CACHED, TEXT("IMAGE_SCN_MEM_NOT_CACHED\n Section is not cachable.") },
	{ IMAGE_SCN_MEM_NOT_PAGED, TEXT("IMAGE_SCN_MEM_NOT_PAGED\n Section is not pageable.") },
	{ IMAGE_SCN_MEM_SHARED, TEXT("IMAGE_SCN_MEM_SHARED\n Section is shareable.") },
	{ IMAGE_SCN_MEM_EXECUTE, TEXT("IMAGE_SCN_MEM_EXECUTE\n Section is executable.") },
	{ IMAGE_SCN_MEM_READ, TEXT("IMAGE_SCN_MEM_READ\n Section is readable.") },
	{ IMAGE_SCN_MEM_WRITE, TEXT("IMAGE_SCN_MEM_WRITE\n Section is writeable.") }
	};

	m_fFileSummaryShow = true;

	listCreateDOSHeader();
	listCreateDOSRich();
	listCreateNTHeader();
	listCreateFileHeader();
	listCreateOptHeader();
	listCreateDataDirs();
	listCreateSections();
	listCreateExportDir();
	listCreateImportDir();
	treeCreateResourceDir();
	listCreateExceptionDir();
	listCreateSecurityDir();
	listCreateRelocDir();
	listCreateDebugDir();
	listCreateTLSDir();
	listCreateLoadConfigDir();
	listCreateBoundImportDir();
	listCreateDelayImportDir();
	listCreateCOMDir();
}

void CViewRightTop::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	if (m_pActiveList)
		m_pActiveList->ShowWindow(SW_HIDE);

	m_fFileSummaryShow = false;

	CRect _rectClient, rect;
	::GetClientRect(AfxGetMainWnd()->m_hWnd, &_rectClient);
	GetClientRect(&rect);

	switch (lHint)
	{
	case LISTID_FILE_SUMMARY:
		m_fFileSummaryShow = true;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_DOS_HEADER:
		m_listDOSHeader.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listDOSHeader;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_DOS_RICH:
		m_listDOSRich.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listDOSRich;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_NT_HEADER:
		m_listNTHeader.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listNTHeader;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_FILE_HEADER:
		m_listFileHeader.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listFileHeader;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_OPTIONAL_HEADER:
		m_listOptHeader.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listOptHeader;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_DATA_DIRS:
		m_listDataDirs.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listDataDirs;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_SECHEADERS:
		m_listSecHeaders.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listSecHeaders;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_EXPORT_DIR:
		m_listExportDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listExportDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height() / 2, 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_IAT_DIR:
	case LISTID_IMPORT_DIR:
		m_listImportDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listImportDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height() / 2, 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_RESOURCE_DIR:
		m_treeResourceDirTop.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_treeResourceDirTop;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height() / 2, 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_EXCEPTION_DIR:
		m_listExceptionDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listExceptionDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_SECURITY_DIR:
		m_listSecurityDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listSecurityDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height() / 2, 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_RELOCATION_DIR:
		m_listRelocDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listRelocDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height() / 2, 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_DEBUG_DIR:
		m_listDebugDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listDebugDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_TLS_DIR:
		m_listTLSDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listTLSDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height() / 2, 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_LOAD_CONFIG_DIR:
		m_listLoadConfigDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listLoadConfigDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_BOUND_IMPORT_DIR:
		m_listBoundImportDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height() / 2, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listBoundImportDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_DELAY_IMPORT_DIR:
		m_listDelayImportDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listDelayImportDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height() / 2, 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	case LISTID_COMDESCRIPTOR_DIR:
		m_listCOMDir.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listCOMDir;
		m_ChildFrame->m_RightSplitter.SetRowInfo(0, _rectClient.Height(), 0);
		m_ChildFrame->m_RightSplitter.RecalcLayout();
		break;
	}
}

void CViewRightTop::OnSize(UINT nType, int cx, int cy)
{
	if (m_pActiveList)
		m_pActiveList->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);

	CView::OnSize(nType, cx, cy);
}

BOOL CViewRightTop::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{/*
 RECT rect;
 GetClientRect(&rect);
 if ((rect.bottom - rect.top) > m_ScrollHeight)
 return 0;

 SCROLLINFO si;
 si.cbSize = sizeof(si);
 si.fMask = SIF_ALL;
 GetScrollInfo(SB_VERT, &si);
 int _yCurPos = si.nPos;

 si.nPos -= zDelta;

 // Set the position and then retrieve it.  Due to adjustments
 // by Windows it may not be the same as the value set.
 si.fMask = SIF_POS;
 SetScrollInfo(SB_VERT, &si, TRUE);
 GetScrollInfo(SB_VERT, &si);

 // If the position has changed, scroll window and update it.
 if (si.nPos != _yCurPos)
 ScrollWindow(0, (_yCurPos - si.nPos), NULL, NULL);*/

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CViewRightTop::OnListSectionsGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	LV_ITEM* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		WCHAR str[9] { };

		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(str, 9, L"%.8S", m_pSectianHeaders->at(pItem->iItem).Name);
			break;
		case 1:
			swprintf_s(str, 9, L"%08X", m_pSectianHeaders->at(pItem->iItem).Misc.VirtualSize);
			break;
		case 2:
			swprintf_s(str, 9, L"%08X", m_pSectianHeaders->at(pItem->iItem).VirtualAddress);
			break;
		case 3:
			swprintf_s(str, 9, L"%08X", m_pSectianHeaders->at(pItem->iItem).SizeOfRawData);
			break;
		case 4:
			swprintf_s(str, 9, L"%08X", m_pSectianHeaders->at(pItem->iItem).PointerToRawData);
			break;
		case 5:
			swprintf_s(str, 9, L"%08X", m_pSectianHeaders->at(pItem->iItem).PointerToRelocations);
			break;
		case 6:
			swprintf_s(str, 9, L"%08X", m_pSectianHeaders->at(pItem->iItem).PointerToLinenumbers);
			break;
		case 7:
			swprintf_s(str, 5, L"%04X", m_pSectianHeaders->at(pItem->iItem).NumberOfRelocations);
			break;
		case 8:
			swprintf_s(str, 5, L"%04X", m_pSectianHeaders->at(pItem->iItem).NumberOfLinenumbers);
			break;
		case 9:
			swprintf_s(str, 9, L"%08X", m_pSectianHeaders->at(pItem->iItem).Characteristics);
			break;
		}
		lstrcpynW(pItem->pszText, str, pItem->cchTextMax);
	}

	*pResult = 0;

}

void CViewRightTop::OnListImportGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	LV_ITEM* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		WCHAR str[MAX_PATH] { };
		const IMAGE_IMPORT_DESCRIPTOR* _pImpDescriptor = &std::get<0>(m_pImportTable->at(pItem->iItem));

		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(str, MAX_PATH, L"%S (%u)", std::get<1>(m_pImportTable->at(pItem->iItem)).c_str(),
				std::get<2>(m_pImportTable->at(pItem->iItem)).size());
			break;
		case 1:
			swprintf_s(str, 9, L"%08X", _pImpDescriptor->OriginalFirstThunk);
			break;
		case 2:
			swprintf_s(str, 9, L"%08X", _pImpDescriptor->TimeDateStamp);
			break;
		case 3:
			swprintf_s(str, 9, L"%08X", _pImpDescriptor->ForwarderChain);
			break;
		case 4:
			swprintf_s(str, 9, L"%08X", _pImpDescriptor->Name);
			break;
		case 5:
			swprintf_s(str, 9, L"%08X", _pImpDescriptor->FirstThunk);
			break;
		}
		lstrcpynW(pItem->pszText, str, pItem->cchTextMax);
	}

	*pResult = 0;
}

void CViewRightTop::OnListRelocGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	LVITEMW* pItem = &pDispInfo->item;

	if (pItem->mask & LVIF_TEXT)
	{
		const IMAGE_BASE_RELOCATION* _pDirReloc = &std::get<0>(m_pRelocTable->at(pItem->iItem));
		WCHAR str[MAX_PATH] { };

		switch (pItem->iSubItem)
		{
		case 0:
			swprintf_s(str, 9, L"%08X", _pDirReloc->VirtualAddress);
			break;
		case 1:
			swprintf_s(str, 9, L"%08X", _pDirReloc->SizeOfBlock);
			break;
		case 2:
			swprintf_s(str, MAX_PATH, L"%u", (_pDirReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD));
			break;
		}
		lstrcpynW(pItem->pszText, str, pItem->cchTextMax);
	}

	*pResult = 0;
}

void CViewRightTop::OnListExceptionGetDispInfo(NMHDR * pNMHDR, LRESULT * pResult)
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

BOOL CViewRightTop::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LPNMITEMACTIVATE _pNMI = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
	if (_pNMI->iItem == -1)
		return TRUE;

	switch (_pNMI->hdr.idFrom)
	{
	case LISTID_IMPORT_DIR:
		if (_pNMI->hdr.code == LVN_ITEMCHANGED || _pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(LISTID_IMPORT_DLL_FUNCS, _pNMI->iItem));
		break;
	case LISTID_SECURITY_DIR:
		if (_pNMI->hdr.code == LVN_ITEMCHANGED || _pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(HEXCTRLID_SECURITY_DIR_SERTIFICATE_ID, _pNMI->iItem));
		break;
	case LISTID_RELOCATION_DIR:
		if (_pNMI->hdr.code == LVN_ITEMCHANGED || _pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(LISTID_RELOCATION_DIR_RELOCS_DESCRIPTION, _pNMI->iItem));
		break;
	case LISTID_DELAY_IMPORT_DIR:
		if (_pNMI->hdr.code == LVN_ITEMCHANGED || _pNMI->hdr.code == NM_CLICK)
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(LISTID_DELAY_IMPORT_DLL_FUNCS, _pNMI->iItem));
		break;
	}


	LPNMTREEVIEW _tree = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (_tree->hdr.idFrom == TREEID_RESOURCE_TOP && _tree->hdr.code == TVN_SELCHANGED)
	{
		PLIBPE_RESOURCE_ROOT _pTupleResRoot { };

		if (m_pLibpe->GetResourceTable(&_pTupleResRoot) != S_OK)
			return -1;

		PLIBPE_RESOURCE_LVL2 _pTupleResLvL2 { };
		PLIBPE_RESOURCE_LVL3 _pTupleResLvL3 { };

		DWORD_PTR _nResId = m_treeResourceDirTop.GetItemData(_tree->itemNew.hItem);

		PIMAGE_RESOURCE_DIRECTORY_ENTRY _pResDirEntry { };

		DWORD_PTR i = 1;//Resource ID (incremental) to set as SetItemData
		////Main loop to extract Resources from tuple
		for (auto& iterRoot : std::get<1>(*_pTupleResRoot))
		{
			if (i == _nResId)
			{
				if (!std::get<3>(iterRoot).empty())
				{

				}
				break;
			}
			i++;

			_pTupleResLvL2 = &std::get<4>(iterRoot);
			for (auto& iterLvL2 : std::get<1>(*_pTupleResLvL2))
			{
				if (i == _nResId)
				{
					if (!std::get<3>(iterLvL2).empty())
					{

					}
					break;
				}
				i++;

				_pTupleResLvL3 = &std::get<4>(iterLvL2);
				for (auto& iterLvL3 : std::get<1>(*_pTupleResLvL3))
				{
					if (i == _nResId)
					{
						if (!std::get<3>(iterLvL3).empty())
						{

						}
						break;
					}
					i++;
				}
			}
		}
	}

	return CScrollView::OnNotify(wParam, lParam, pResult);
}

int CViewRightTop::listCreateDOSHeader()
{
	PLIBPE_DOSHEADER _pDosHeader { };
	if (m_pLibpe->GetMSDOSHeader(&_pDosHeader) != S_OK)
		return -1;

	m_listDOSHeader.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_DOS_HEADER);
	m_listDOSHeader.ShowWindow(SW_HIDE);
	m_listDOSHeader.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDOSHeader.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 150);
	m_listDOSHeader.InsertColumn(1, _T("Offset"), LVCFMT_LEFT, 100);
	m_listDOSHeader.InsertColumn(2, _T("Size [BYTES]"), LVCFMT_LEFT, 100);
	m_listDOSHeader.InsertColumn(3, _T("Value"), LVCFMT_LEFT, 100);

	m_dwPeStart = _pDosHeader->e_lfanew;

	WCHAR str[9] { };
	int _listindex = 0;

	m_listDOSHeader.InsertItem(_listindex, _T("e_magic"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_magic));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_magic));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 3, L"%02X", BYTE(_pDosHeader->e_magic));
	swprintf_s(&str[2], 3, L"%02X", *((BYTE*)(&_pDosHeader->e_magic) + 1));
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_cblp"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cblp));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_cblp));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_cblp);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_cp"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cp));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_cp));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_cp);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_crlc"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_crlc));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_crlc));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_crlc);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_cparhdr"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cparhdr));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_cparhdr));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_cparhdr);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_minalloc"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_minalloc));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_minalloc));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_minalloc);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_maxalloc"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_maxalloc));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_maxalloc));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_maxalloc);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_ss"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_ss));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_ss));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_ss);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_sp"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_sp));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_sp));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_sp);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_csum"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_csum));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_csum));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_csum);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_ip"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_ip));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_ip));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_ip);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_cs"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_cs));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_cs));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_cs);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_lfarlc"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_lfarlc));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_lfarlc));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_lfarlc);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_ovno"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_ovno));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_ovno));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_ovno);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_res[0]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[0]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res[0]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res[0]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res[1]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[1]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res[1]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res[1]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res[2]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[2]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res[2]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res[2]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res[3]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res[3]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res[3]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res[3]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_oemid"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_oemid));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_oemid));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_oemid);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_oeminfo"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_oeminfo));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_oeminfo));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_oeminfo);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_res2[0]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[0]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[0]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[0]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res2[1]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[1]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[1]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[1]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res2[2]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[2]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[2]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[2]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res2[3]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[3]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[3]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[3]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res2[4]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[4]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[4]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[4]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res2[5]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[5]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[5]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[5]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res2[6]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[6]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[6]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[6]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res2[7]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[7]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[7]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[7]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res2[8]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[8]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[8]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[8]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("   e_res2[9]"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_res2[9]));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_res2[9]));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pDosHeader->e_res2[9]);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listDOSHeader.InsertItem(_listindex + 1, _T("e_lfanew"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_DOS_HEADER, e_lfanew));
	m_listDOSHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pDosHeader->e_lfanew));
	m_listDOSHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", _pDosHeader->e_lfanew);
	m_listDOSHeader.SetItemText(_listindex, 3, str);

	return 0;
}

int CViewRightTop::listCreateDOSRich()
{
	PLIBPE_RICH _pRichHeader { };
	if (m_pLibpe->GetMSDOSRichHeader(&_pRichHeader) != S_OK)
		return -1;

	m_listDOSRich.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_DOS_RICH);
	m_listDOSRich.ShowWindow(SW_HIDE);
	m_listDOSRich.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDOSRich.InsertColumn(0, _T("№"), LVCFMT_CENTER, 30);
	m_listDOSRich.InsertColumn(1, _T("ID [Hex]"), LVCFMT_LEFT, 100);
	m_listDOSRich.InsertColumn(2, _T("Version"), LVCFMT_LEFT, 100);
	m_listDOSRich.InsertColumn(3, _T("Occurrences"), LVCFMT_LEFT, 100);

	WCHAR str[MAX_PATH] { };
	int _listindex = 0;

	for (auto& i : *_pRichHeader)
	{
		swprintf_s(str, MAX_PATH, L"%i", _listindex + 1);
		m_listDOSRich.InsertItem(_listindex, str);
		swprintf_s(str, MAX_PATH, L"%04X", std::get<0>(i));
		m_listDOSRich.SetItemText(_listindex, 1, str);
		swprintf_s(str, MAX_PATH, L"%i", std::get<1>(i));
		m_listDOSRich.SetItemText(_listindex, 2, str);
		swprintf_s(str, MAX_PATH, L"%i", std::get<2>(i));
		m_listDOSRich.SetItemText(_listindex, 3, str);

		_listindex++;
	}

	return 0;
}

int CViewRightTop::listCreateNTHeader()
{
	PLIBPE_NTHEADER _pNTHeader { };
	if (m_pLibpe->GetNTHeader(&_pNTHeader) != S_OK)
		return -1;

	m_listNTHeader.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_NT_HEADER);
	m_listNTHeader.ShowWindow(SW_HIDE);
	m_listNTHeader.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listNTHeader.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 100);
	m_listNTHeader.InsertColumn(1, _T("Offset"), LVCFMT_LEFT, 100);
	m_listNTHeader.InsertColumn(2, _T("Size [BYTES]"), LVCFMT_LEFT, 100);
	m_listNTHeader.InsertColumn(3, _T("Value"), LVCFMT_LEFT, 100);

	const IMAGE_NT_HEADERS32* _pNTHeader32 { };
	const IMAGE_NT_HEADERS64* _pNTHeader64 { };
	WCHAR str[9] { };
	UINT _listindex = 0;

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
	{
		_pNTHeader32 = &std::get<0>(*_pNTHeader);

		_listindex = m_listNTHeader.InsertItem(_listindex, _T("Signature"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, Signature) + m_dwPeStart);
		m_listNTHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pNTHeader32->Signature));
		m_listNTHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 3, L"%02X", (BYTE)_pNTHeader32->Signature);
		swprintf_s(&str[2], 3, L"%02X", *((BYTE*)(&_pNTHeader32->Signature) + 1));
		swprintf_s(&str[4], 3, L"%02X", *((BYTE*)(&_pNTHeader32->Signature) + 2));
		swprintf_s(&str[6], 3, L"%02X", *((BYTE*)(&_pNTHeader32->Signature) + 3));
		m_listNTHeader.SetItemText(_listindex, 3, str);
	}
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
	{
		_pNTHeader64 = &std::get<1>(*_pNTHeader);

		_listindex = m_listNTHeader.InsertItem(_listindex, _T("Signature"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, Signature) + m_dwPeStart);
		m_listNTHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pNTHeader64->Signature));
		m_listNTHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 3, L"%02X", (BYTE)_pNTHeader64->Signature);
		swprintf_s(&str[2], 3, L"%02X", *((BYTE*)(&_pNTHeader64->Signature) + 1));
		swprintf_s(&str[4], 3, L"%02X", *((BYTE*)(&_pNTHeader64->Signature) + 2));
		swprintf_s(&str[6], 3, L"%02X", *((BYTE*)(&_pNTHeader64->Signature) + 3));
		m_listNTHeader.SetItemText(_listindex, 3, str);
	}

	return 0;
}

int CViewRightTop::listCreateFileHeader()
{
	PLIBPE_FILEHEADER _pFileHeader { };
	if (m_pLibpe->GetFileHeader(&_pFileHeader) != S_OK)
		return -1;

	m_listFileHeader.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_FILE_HEADER);

	m_listFileHeader.ShowWindow(SW_HIDE);
	m_listFileHeader.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	m_listFileHeader.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 200);
	m_listFileHeader.InsertColumn(1, _T("Offset"), LVCFMT_LEFT, 100);
	m_listFileHeader.InsertColumn(2, _T("Size [BYTES]"), LVCFMT_LEFT, 100);
	m_listFileHeader.InsertColumn(3, _T("Value"), LVCFMT_LEFT, 300);

	std::map<WORD, std::wstring> _mapMachineType {
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

	std::map<WORD, std::wstring> _mapCharacteristics {
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
	int _listindex = 0;

	_listindex = m_listFileHeader.InsertItem(_listindex, _T("Machine"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.Machine) + m_dwPeStart);
	m_listFileHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pFileHeader->Machine));
	m_listFileHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pFileHeader->Machine);
	m_listFileHeader.SetItemText(_listindex, 3, str);
	for (auto&i : _mapMachineType)
		if (i.first == _pFileHeader->Machine)
			m_listFileHeader.SetItemToolTip(_listindex, 3, i.second, TEXT("Machine:"));

	_listindex = m_listFileHeader.InsertItem(_listindex + 1, _T("NumberOfSections"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.NumberOfSections) + m_dwPeStart);
	m_listFileHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pFileHeader->NumberOfSections));
	m_listFileHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pFileHeader->NumberOfSections);
	m_listFileHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listFileHeader.InsertItem(_listindex + 1, _T("TimeDateStamp"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.TimeDateStamp) + m_dwPeStart);
	m_listFileHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pFileHeader->TimeDateStamp));
	m_listFileHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, MAX_PATH, L"%08X", _pFileHeader->TimeDateStamp);
	m_listFileHeader.SetItemText(_listindex, 3, str);
	if (_pFileHeader->TimeDateStamp)
	{
		__time64_t _time = _pFileHeader->TimeDateStamp;
		_wctime64_s(str, MAX_PATH, &_time);
		m_listFileHeader.SetItemToolTip(_listindex, 3, str, TEXT("Time / Date:"));
	}

	_listindex = m_listFileHeader.InsertItem(_listindex + 1, _T("PointerToSymbolTable"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.PointerToSymbolTable) + m_dwPeStart);
	m_listFileHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pFileHeader->PointerToSymbolTable));
	m_listFileHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", _pFileHeader->PointerToSymbolTable);
	m_listFileHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listFileHeader.InsertItem(_listindex + 1, _T("NumberOfSymbols"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.NumberOfSymbols) + m_dwPeStart);
	m_listFileHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pFileHeader->NumberOfSymbols));
	m_listFileHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", _pFileHeader->NumberOfSymbols);
	m_listFileHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listFileHeader.InsertItem(_listindex + 1, _T("SizeOfOptionalHeader"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.SizeOfOptionalHeader) + m_dwPeStart);
	m_listFileHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pFileHeader->SizeOfOptionalHeader));
	m_listFileHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%04X", _pFileHeader->SizeOfOptionalHeader);
	m_listFileHeader.SetItemText(_listindex, 3, str);

	_listindex = m_listFileHeader.InsertItem(_listindex + 1, _T("Characteristics"));
	swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, FileHeader.Characteristics) + m_dwPeStart);
	m_listFileHeader.SetItemText(_listindex, 1, str);
	swprintf_s(str, 2, L"%X", sizeof(_pFileHeader->Characteristics));
	m_listFileHeader.SetItemText(_listindex, 2, str);
	swprintf_s(str, 5, L"%04X", _pFileHeader->Characteristics);
	m_listFileHeader.SetItemText(_listindex, 3, str);
	std::wstring  _strCharact { };
	for (auto& i : _mapCharacteristics)
		if (i.first & _pFileHeader->Characteristics)
			_strCharact += i.second + L"\n";
	if (_strCharact.size())
	{
		_strCharact.erase(_strCharact.size() - 1);//to remove last '\n'
		m_listFileHeader.SetItemToolTip(_listindex, 3, _strCharact, TEXT("Characteristics:"));
	}
	return 0;
}

int CViewRightTop::listCreateOptHeader()
{
	PLIBPE_OPTHEADER _pOptHeader { };
	if (m_pLibpe->GetOptionalHeader(&_pOptHeader) != S_OK)
		return -1;

	m_listOptHeader.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_OPTIONAL_HEADER);
	m_listOptHeader.ShowWindow(SW_HIDE);
	m_listOptHeader.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	m_listOptHeader.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 215);
	m_listOptHeader.InsertColumn(1, _T("Offset"), LVCFMT_LEFT, 100);
	m_listOptHeader.InsertColumn(2, _T("Size [BYTES]"), LVCFMT_LEFT, 100);
	m_listOptHeader.InsertColumn(3, _T("Value"), LVCFMT_LEFT, 140);

	std::map<WORD, std::string> _mapSubSystem {
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

	std::map<WORD, std::wstring> _mapDllCharacteristics {
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

	const IMAGE_OPTIONAL_HEADER32* _pOptHeader32 { };
	const IMAGE_OPTIONAL_HEADER64* _pOptHeader64 { };

	WCHAR str[MAX_PATH] { };
	std::wstring _strTmp { };
	int _listindex = 0;

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
	{
		_pOptHeader32 = &std::get<0>(*_pOptHeader);

		_listindex = m_listOptHeader.InsertItem(_listindex, _T("Magic"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.Magic) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->Magic));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%04X", _pOptHeader32->Magic);
		m_listOptHeader.SetItemText(_listindex, 3, str);
		m_listOptHeader.SetItemToolTip(_listindex, 3, L"IMAGE_NT_OPTIONAL_HDR32_MAGIC", TEXT("Magic:"));

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MajorLinkerVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorLinkerVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->MajorLinkerVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%02X", _pOptHeader32->MajorLinkerVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MinorLinkerVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorLinkerVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->MinorLinkerVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%02X", _pOptHeader32->MinorLinkerVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfCode"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfCode) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SizeOfCode));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SizeOfCode);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfInitializedData"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfInitializedData) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SizeOfInitializedData));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SizeOfInitializedData);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfUninitializedData"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfUninitializedData) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SizeOfUninitializedData));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SizeOfUninitializedData);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("AddressOfEntryPoint"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.AddressOfEntryPoint) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->AddressOfEntryPoint));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->AddressOfEntryPoint);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("BaseOfCode"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.BaseOfCode) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->BaseOfCode));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->BaseOfCode);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("BaseOfData"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.BaseOfData) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->BaseOfData));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->BaseOfData);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("ImageBase"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.ImageBase) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->ImageBase));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->ImageBase);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SectionAlignment"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SectionAlignment) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SectionAlignment));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SectionAlignment);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("FileAlignment"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.FileAlignment) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->FileAlignment));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->FileAlignment);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MajorOperatingSystemVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorOperatingSystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->MajorOperatingSystemVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader32->MajorOperatingSystemVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MinorOperatingSystemVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorOperatingSystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->MinorOperatingSystemVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader32->MinorOperatingSystemVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MajorImageVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorImageVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->MajorImageVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader32->MajorImageVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MinorImageVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorImageVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->MinorImageVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader32->MinorImageVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MajorSubsystemVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MajorSubsystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->MajorSubsystemVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader32->MajorSubsystemVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MinorSubsystemVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.MinorSubsystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->MinorSubsystemVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader32->MinorSubsystemVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("Win32VersionValue"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.Win32VersionValue) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->Win32VersionValue));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->Win32VersionValue);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfImage"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfImage) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SizeOfImage));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SizeOfImage);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfHeaders"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfHeaders) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SizeOfHeaders));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SizeOfHeaders);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("CheckSum"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.CheckSum) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->CheckSum));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->CheckSum);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("Subsystem"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.Subsystem) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->Subsystem));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader32->Subsystem);
		m_listOptHeader.SetItemText(_listindex, 3, str);
		if (_mapSubSystem.find(_pOptHeader32->Subsystem) != _mapSubSystem.end())
		{
			swprintf_s(str, MAX_PATH, L"%S", _mapSubSystem.at(_pOptHeader32->Subsystem).c_str());
			m_listOptHeader.SetItemToolTip(_listindex, 3, str, TEXT("Subsystem:"));
		}

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("DllCharacteristics"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DllCharacteristics) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->DllCharacteristics));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%04X", _pOptHeader32->DllCharacteristics);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_strTmp.clear();
		for (auto& i : _mapDllCharacteristics)
		{
			if (i.first & _pOptHeader32->DllCharacteristics)
				_strTmp += i.second + L"\n";
		}
		if (_strTmp.size())
		{
			_strTmp.erase(_strTmp.size() - 1);//to remove last '\n'
			m_listOptHeader.SetItemToolTip(_listindex, 3, _strTmp, TEXT("DllCharacteristics:"));
		}

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfStackReserve"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfStackReserve) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SizeOfStackReserve));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SizeOfStackReserve);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfStackCommit"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfStackCommit) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SizeOfStackCommit));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SizeOfStackCommit);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfHeapReserve"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfHeapReserve) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SizeOfHeapReserve));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SizeOfHeapReserve);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfHeapCommit"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.SizeOfHeapCommit) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->SizeOfHeapCommit));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->SizeOfHeapCommit);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("LoaderFlags"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.LoaderFlags) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->LoaderFlags));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->LoaderFlags);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("NumberOfRvaAndSizes"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS32, OptionalHeader.NumberOfRvaAndSizes) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader32->NumberOfRvaAndSizes));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader32->NumberOfRvaAndSizes);
		m_listOptHeader.SetItemText(_listindex, 3, str);
	}
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
	{
		_pOptHeader64 = &std::get<1>(*_pOptHeader);

		_listindex = m_listOptHeader.InsertItem(_listindex, _T("Magic"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.Magic) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->Magic));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%04X", _pOptHeader64->Magic);
		m_listOptHeader.SetItemText(_listindex, 3, str);
		m_listOptHeader.SetItemToolTip(_listindex, 3, L"IMAGE_NT_OPTIONAL_HDR64_MAGIC", TEXT("Magic:"));

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MajorLinkerVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorLinkerVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->MajorLinkerVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%02X", _pOptHeader64->MajorLinkerVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MinorLinkerVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorLinkerVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->MinorLinkerVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%02X", _pOptHeader64->MinorLinkerVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfCode"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfCode) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SizeOfCode));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->SizeOfCode);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfInitializedData"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfInitializedData) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SizeOfInitializedData));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->SizeOfInitializedData);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfUninitializedData"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfUninitializedData) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SizeOfUninitializedData));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->SizeOfUninitializedData);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("AddressOfEntryPoint"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.AddressOfEntryPoint) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->AddressOfEntryPoint));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->AddressOfEntryPoint);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("BaseOfCode"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.BaseOfCode) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->BaseOfCode));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->BaseOfCode);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("ImageBase"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.ImageBase) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->ImageBase));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", _pOptHeader64->ImageBase);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SectionAlignment"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SectionAlignment) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SectionAlignment));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->SectionAlignment);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("FileAlignment"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.FileAlignment) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->FileAlignment));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->FileAlignment);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MajorOperatingSystemVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorOperatingSystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->MajorOperatingSystemVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader64->MajorOperatingSystemVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MinorOperatingSystemVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorOperatingSystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->MinorOperatingSystemVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader64->MinorOperatingSystemVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MajorImageVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorImageVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->MajorImageVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader64->MajorImageVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MinorImageVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorImageVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->MinorImageVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader64->MinorImageVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MajorSubsystemVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorSubsystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->MajorSubsystemVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader64->MajorSubsystemVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("MinorSubsystemVersion"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MinorSubsystemVersion) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->MinorSubsystemVersion));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader64->MinorSubsystemVersion);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("Win32VersionValue"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.Win32VersionValue) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->Win32VersionValue));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->Win32VersionValue);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfImage"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfImage) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SizeOfImage));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->SizeOfImage);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfHeaders"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeaders) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SizeOfHeaders));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->SizeOfHeaders);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("CheckSum"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.CheckSum) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->CheckSum));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->CheckSum);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("Subsystem"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.Subsystem) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->Subsystem));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%04X", _pOptHeader64->Subsystem);
		m_listOptHeader.SetItemText(_listindex, 3, str);
		if (_mapSubSystem.find(_pOptHeader64->Subsystem) != _mapSubSystem.end())
		{
			swprintf_s(str, MAX_PATH, L"%S", _mapSubSystem.at(_pOptHeader64->Subsystem).c_str());
			m_listOptHeader.SetItemToolTip(_listindex, 3, str, TEXT("Subsystem:"));
		}

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("DllCharacteristics"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DllCharacteristics) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->DllCharacteristics));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%04X", _pOptHeader64->DllCharacteristics);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_strTmp.clear();
		for (auto& i : _mapDllCharacteristics)
		{
			if (i.first & _pOptHeader64->DllCharacteristics)
				_strTmp += i.second + L"\n";
		}
		if (_strTmp.size())
		{
			_strTmp.erase(_strTmp.size() - 1);//to remove last '\n'
			m_listOptHeader.SetItemToolTip(_listindex, 3, _strTmp, TEXT("DllCharacteristics:"));
		}

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfStackReserve"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfStackReserve) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SizeOfStackReserve));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", _pOptHeader64->SizeOfStackReserve);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfStackCommit"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfStackCommit) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SizeOfStackCommit));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", _pOptHeader64->SizeOfStackCommit);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfHeapReserve"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeapReserve) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SizeOfHeapReserve));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", _pOptHeader64->SizeOfHeapReserve);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("SizeOfHeapCommit"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeapCommit) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->SizeOfHeapCommit));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 17, L"%016llX", _pOptHeader64->SizeOfHeapCommit);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("LoaderFlags"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.LoaderFlags) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->LoaderFlags));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->LoaderFlags);
		m_listOptHeader.SetItemText(_listindex, 3, str);

		_listindex = m_listOptHeader.InsertItem(_listindex + 1, _T("NumberOfRvaAndSizes"));
		swprintf_s(str, 9, L"%08X", offsetof(IMAGE_NT_HEADERS64, OptionalHeader.NumberOfRvaAndSizes) + m_dwPeStart);
		m_listOptHeader.SetItemText(_listindex, 1, str);
		swprintf_s(str, 2, L"%X", sizeof(_pOptHeader64->NumberOfRvaAndSizes));
		m_listOptHeader.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pOptHeader64->NumberOfRvaAndSizes);
		m_listOptHeader.SetItemText(_listindex, 3, str);
	}

	return 0;
}

int CViewRightTop::listCreateDataDirs()
{
	PLIBPE_DATADIRS _pDataDirs { };
	if (m_pLibpe->GetDataDirectories(&_pDataDirs) != S_OK)
		return -1;

	m_listDataDirs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_DATA_DIRS);
	m_listDataDirs.ShowWindow(SW_HIDE);
	m_listDataDirs.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDataDirs.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 200);
	m_listDataDirs.InsertColumn(1, _T("Offset"), LVCFMT_LEFT, 100);
	m_listDataDirs.InsertColumn(2, _T("Directory RVA"), LVCFMT_LEFT, 100);
	m_listDataDirs.InsertColumn(3, _T("Directory Size"), LVCFMT_LEFT, 100);
	m_listDataDirs.InsertColumn(4, _T("Resides in Section"), LVCFMT_LEFT, 125);

	PIMAGE_NT_HEADERS32 _ntH32 { };
	PIMAGE_NT_HEADERS64 _ntH64 { };
	WCHAR str[9] { };
	UINT _listindex = 0;

	DWORD _dwDataDirsOffset { };

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
		_dwDataDirsOffset = m_dwPeStart + offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
		_dwDataDirsOffset = m_dwPeStart + offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);

	const IMAGE_DATA_DIRECTORY* pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXPORT));

	_listindex = m_listDataDirs.InsertItem(_listindex, _T("Export Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXPORT)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_IMPORT));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Import Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_IMPORT)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_RESOURCE));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Resource Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_RESOURCE)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXCEPTION));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Exception Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_EXCEPTION)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_SECURITY));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Security Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	if (pDataDirs->VirtualAddress)
		m_listDataDirs.SetItemToolTip(_listindex, 2, TEXT("This address is file RAW offset on disk"));
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_SECURITY)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_BASERELOC));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Relocation Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_BASERELOC)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_DEBUG));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Debug Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_DEBUG)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_ARCHITECTURE));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Architecture Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_ARCHITECTURE)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_GLOBALPTR));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Global PTR"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_GLOBALPTR)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_TLS));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("TLS Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_TLS)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Load Config Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Bound Import Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_IAT));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("IAT Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_IAT)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("Delay Import Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	pDataDirs = &std::get<0>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR));

	_dwDataDirsOffset += sizeof(IMAGE_DATA_DIRECTORY);
	_listindex = m_listDataDirs.InsertItem(_listindex + 1, _T("COM Descriptor Directory"));
	swprintf_s(str, 9, L"%08X", _dwDataDirsOffset);
	m_listDataDirs.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->VirtualAddress);
	m_listDataDirs.SetItemText(_listindex, 2, str);
	swprintf_s(str, 9, L"%08X", pDataDirs->Size);
	m_listDataDirs.SetItemText(_listindex, 3, str);
	swprintf_s(str, 9, L"%.8S", std::get<1>(_pDataDirs->at(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)).c_str());
	m_listDataDirs.SetItemText(_listindex, 4, str);

	return 0;
}

int CViewRightTop::listCreateSections()
{
	if (!m_pSectianHeaders)
		return -1;

	m_listSecHeaders.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDATA | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_SECHEADERS);
	m_listSecHeaders.ShowWindow(SW_HIDE);
	m_listSecHeaders.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_listSecHeaders.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 75);
	m_listSecHeaders.InsertColumn(1, _T("Virtual Size"), LVCFMT_LEFT, 100);
	m_listSecHeaders.InsertColumn(2, _T("Virtual Address"), LVCFMT_LEFT, 125);
	m_listSecHeaders.InsertColumn(3, _T("SizeOfRawData"), LVCFMT_LEFT, 125);
	m_listSecHeaders.InsertColumn(4, _T("PointerToRawData"), LVCFMT_LEFT, 125);
	m_listSecHeaders.InsertColumn(5, _T("PointerToRelocations"), LVCFMT_LEFT, 150);
	m_listSecHeaders.InsertColumn(6, _T("PointerToLinenumbers"), LVCFMT_LEFT, 160);
	m_listSecHeaders.InsertColumn(7, _T("NumberOfRelocations"), LVCFMT_LEFT, 150);
	m_listSecHeaders.InsertColumn(8, _T("NumberOfLinenumbers"), LVCFMT_LEFT, 160);
	m_listSecHeaders.InsertColumn(9, _T("Characteristics"), LVCFMT_LEFT, 115);
	m_listSecHeaders.SetItemCountEx(m_pSectianHeaders->size(), LVSICF_NOSCROLL);

	UINT _listindex = 0;
	std::wstring _strTipText { };

	for (auto &i : *m_pSectianHeaders)
	{
		for (auto& flags : m_mapSecFlags)
			if (flags.first & i.Characteristics)
				_strTipText += flags.second + TEXT("\n");

		if (!_strTipText.empty())
		{
			m_listSecHeaders.SetItemToolTip(_listindex, 9, _strTipText, TEXT("Section Flags:"));
			_strTipText.clear();
		}

		_listindex++;
	}

	return 0;
}

int CViewRightTop::listCreateExportDir()
{
	PLIBPE_EXPORT _pExportTable { };
	if (m_pLibpe->GetExportTable(&_pExportTable) != S_OK)
		return -1;

	WCHAR str[MAX_PATH] { };
	int _listindex = 0;
	const IMAGE_EXPORT_DIRECTORY* _pExportDir = &std::get<0>(*_pExportTable);

	m_listExportDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_EXPORT_DIR);
	m_listExportDir.ShowWindow(SW_HIDE);
	m_listExportDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listExportDir.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 250);
	m_listExportDir.InsertColumn(1, _T("Size [BYTES]"), LVCFMT_LEFT, 100);
	m_listExportDir.InsertColumn(2, _T("Value"), LVCFMT_LEFT, 300);

	m_listExportDir.InsertItem(_listindex, _T("Export flags (Characteristics)"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->Characteristics));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", _pExportDir->Characteristics);
	m_listExportDir.SetItemText(_listindex, 2, str);

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("Time/Date Stamp"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->TimeDateStamp));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, MAX_PATH, L"%08X", _pExportDir->TimeDateStamp);
	m_listExportDir.SetItemText(_listindex, 2, str);
	if (_pExportDir->TimeDateStamp)
	{
		__time64_t _time = _pExportDir->TimeDateStamp;
		_wctime64_s(str, MAX_PATH, &_time);
		m_listExportDir.SetItemToolTip(_listindex, 2, str, TEXT("Time / Date:"));
	}

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("MajorVersion"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->MajorVersion));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, 5, L"%04X", _pExportDir->MajorVersion);
	m_listExportDir.SetItemText(_listindex, 2, str);

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("MinorVersion"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->MinorVersion));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, 5, L"%04X", _pExportDir->MinorVersion);
	m_listExportDir.SetItemText(_listindex, 2, str);

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("Name RVA"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->Name));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, MAX_PATH, L"%08X (%S)", _pExportDir->Name, std::get<1>(*_pExportTable).c_str());
	m_listExportDir.SetItemText(_listindex, 2, str);

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("Base (OrdinalBase)"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->Base));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", _pExportDir->Base);
	m_listExportDir.SetItemText(_listindex, 2, str);

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("NumberOfFunctions"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->NumberOfFunctions));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", _pExportDir->NumberOfFunctions);
	m_listExportDir.SetItemText(_listindex, 2, str);

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("NumberOfNames"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->NumberOfNames));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", _pExportDir->NumberOfNames);
	m_listExportDir.SetItemText(_listindex, 2, str);

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("AddressOfFunctions"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->AddressOfFunctions));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", _pExportDir->AddressOfFunctions);
	m_listExportDir.SetItemText(_listindex, 2, str);

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("AddressOfNames"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->AddressOfNames));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", _pExportDir->AddressOfNames);
	m_listExportDir.SetItemText(_listindex, 2, str);

	_listindex = m_listExportDir.InsertItem(_listindex + 1, _T("AddressOfNameOrdinals"));
	swprintf_s(str, 2, L"%X", sizeof(_pExportDir->AddressOfNameOrdinals));
	m_listExportDir.SetItemText(_listindex, 1, str);
	swprintf_s(str, 9, L"%08X", _pExportDir->AddressOfNameOrdinals);
	m_listExportDir.SetItemText(_listindex, 2, str);

	return 0;
}

int CViewRightTop::listCreateImportDir()
{
	if (!m_pImportTable)
		return -1;

	m_listImportDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDATA | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_IMPORT_DIR);
	m_listImportDir.ShowWindow(SW_HIDE);
	m_listImportDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listImportDir.InsertColumn(0, _T("Module Name (funcs number)"), LVCFMT_CENTER, 330);
	m_listImportDir.InsertColumn(1, _T("OriginalFirstThunk\n(Import Lookup Table)"), LVCFMT_LEFT, 170);
	m_listImportDir.InsertColumn(2, _T("TimeDateStamp"), LVCFMT_LEFT, 115);
	m_listImportDir.InsertColumn(3, _T("ForwarderChain"), LVCFMT_LEFT, 110);
	m_listImportDir.InsertColumn(4, _T("Name RVA"), LVCFMT_LEFT, 90);
	m_listImportDir.InsertColumn(5, _T("FirstThunk (IAT)"), LVCFMT_LEFT, 135);
	m_listImportDir.SetItemCountEx(m_pImportTable->size(), LVSICF_NOSCROLL);

	return 0;
}

int CViewRightTop::treeCreateResourceDir()
{
	PLIBPE_RESOURCE_ROOT _pTupleResRoot { };

	if (m_pLibpe->GetResourceTable(&_pTupleResRoot) != S_OK)
		return -1;

	PLIBPE_RESOURCE_LVL2 _pTupleResLvL2 { };
	PLIBPE_RESOURCE_LVL3 _pTupleResLvL3 { };

	m_treeResourceDirTop.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CRect(0, 0, 0, 0), this, TREEID_RESOURCE_TOP);
	m_treeResourceDirTop.ShowWindow(SW_HIDE);

	HTREEITEM _hTreeResDir = m_treeResourceDirTop.InsertItem(_T("Root Directory"));

	const IMAGE_RESOURCE_DIRECTORY_ENTRY* _pResDirEntry { };
	WCHAR str[MAX_PATH] { };
	HTREEITEM _treeRoot { }, _treeLvL2 { }, _treeLvL3 { };
	int _idRootEntry = 0, _idLvL2Entry = 0, _idLvL3Entry = 0;
	DWORD_PTR _nResId { };//Resource ID (incremental) to set as SetItemData

	////Main loop to extract Resources from tuple
	for (auto& iterRoot : std::get<1>(*_pTupleResRoot))
	{
		_pResDirEntry = &std::get<0>(iterRoot);
		if (_pResDirEntry->DataIsDirectory)
		{
			if (_pResDirEntry->NameIsString)
				swprintf(str, MAX_PATH, L"Entry: %i, Name: %s", _idRootEntry, std::get<1>(iterRoot).c_str());
			else
				swprintf(str, MAX_PATH, L"Entry: %i, Id: %u", _idRootEntry, _pResDirEntry->Id);

			_treeRoot = m_treeResourceDirTop.InsertItem(str, _hTreeResDir);
			_nResId++;
			m_treeResourceDirTop.SetItemData(_treeRoot, _nResId);
		}
		else
		{
			//DATA
		}
		_pTupleResLvL2 = &std::get<4>(iterRoot);
		for (auto& iterLvL2 : std::get<1>(*_pTupleResLvL2))
		{
			_pResDirEntry = &std::get<0>(iterLvL2);
			if (_pResDirEntry->DataIsDirectory)
			{
				if (_pResDirEntry->NameIsString)
					swprintf(str, MAX_PATH, L"Entry: %i, Name: %s", _idLvL2Entry, std::get<1>(iterLvL2).c_str());
				else
					swprintf(str, MAX_PATH, L"Entry: %i, Id: %u", _idLvL2Entry, _pResDirEntry->Id);

				_treeLvL2 = m_treeResourceDirTop.InsertItem(str, _treeRoot);
				_nResId++;
				m_treeResourceDirTop.SetItemData(_treeLvL2, _nResId);
			}
			else
			{
				//DATA
			}
			_pTupleResLvL3 = &std::get<4>(iterLvL2);
			for (auto& iterLvL3 : std::get<1>(*_pTupleResLvL3))
			{
				_pResDirEntry = &std::get<0>(iterLvL3);

				if (_pResDirEntry->NameIsString)
					swprintf(str, MAX_PATH, L"Entry: %i, Name: %s", _idLvL3Entry, std::get<1>(iterLvL3).c_str());
				else
					swprintf(str, MAX_PATH, L"Entry: %i, lang: %u", _idLvL3Entry, _pResDirEntry->Id);

				_treeLvL3 = m_treeResourceDirTop.InsertItem(str, _treeLvL2);
				_nResId++;
				m_treeResourceDirTop.SetItemData(_treeLvL3, _nResId);

				_idLvL3Entry++;
			}
			_idLvL3Entry = 0;
			_idLvL2Entry++;
		}
		_idLvL2Entry = 0;
		_idRootEntry++;
	}

	m_treeResourceDirTop.Expand(_hTreeResDir, TVE_EXPAND);

	return 0;
}

int CViewRightTop::listCreateExceptionDir()
{
	if (!m_pExceptionDir)
		return -1;

	m_listExceptionDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDATA | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_EXCEPTION_DIR);
	m_listExceptionDir.ShowWindow(SW_HIDE);
	m_listExceptionDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listExceptionDir.InsertColumn(0, _T("BeginAddress"), LVCFMT_CENTER, 100);
	m_listExceptionDir.InsertColumn(1, _T("EndAddress"), LVCFMT_LEFT, 100);
	m_listExceptionDir.InsertColumn(2, _T("UnwindData/InfoAddress"), LVCFMT_LEFT, 180);
	m_listExceptionDir.SetItemCountEx(m_pExceptionDir->size(), LVSICF_NOSCROLL);

	return 0;
}

int CViewRightTop::listCreateSecurityDir()
{
	PLIBPE_SECURITY _pSecurityDir { };
	if (m_pLibpe->GetSecurityTable(&_pSecurityDir) != S_OK)
		return -1;

	m_listSecurityDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_SECURITY_DIR);
	m_listSecurityDir.ShowWindow(SW_HIDE);
	m_listSecurityDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listSecurityDir.InsertColumn(0, _T("dwLength"), LVCFMT_CENTER, 100);
	m_listSecurityDir.InsertColumn(1, _T("wRevision"), LVCFMT_LEFT, 100);
	m_listSecurityDir.InsertColumn(2, _T("wCertificateType"), LVCFMT_LEFT, 180);

	int _listindex = 0;
	TCHAR str[9] { };
	for (auto& i : *_pSecurityDir)
	{
		WIN_CERTIFICATE _pSert = std::get<0>(i);
		swprintf_s(str, 9, L"%08X", _pSert.dwLength);
		m_listSecurityDir.InsertItem(_listindex, str);
		swprintf_s(str, 5, L"%04X", _pSert.wRevision);
		m_listSecurityDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 5, L"%04X", _pSert.wCertificateType);
		m_listSecurityDir.SetItemText(_listindex, 2, str);

		_listindex++;
	}

	return 0;
}

int CViewRightTop::listCreateRelocDir()
{
	if (!m_pRelocTable)
		return -1;

	m_listRelocDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDATA | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_RELOCATION_DIR);
	m_listRelocDir.ShowWindow(SW_HIDE);
	m_listRelocDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listRelocDir.InsertColumn(0, _T("Virtual Address"), LVCFMT_CENTER, 115);
	m_listRelocDir.InsertColumn(1, _T("Block Size"), LVCFMT_LEFT, 100);
	m_listRelocDir.InsertColumn(2, _T("Entries"), LVCFMT_LEFT, 100);
	m_listRelocDir.SetItemCountEx(m_pRelocTable->size(), LVSICF_NOSCROLL);

	return 0;
}

int CViewRightTop::listCreateDebugDir()
{
	PLIBPE_DEBUG _pDebugDir { };

	if (m_pLibpe->GetDebugTable(&_pDebugDir) != S_OK)
		return -1;

	m_listDebugDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_DEBUG_DIR);
	m_listDebugDir.ShowWindow(SW_HIDE);
	m_listDebugDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDebugDir.InsertColumn(0, _T("Characteristics"), LVCFMT_CENTER, 115);
	m_listDebugDir.InsertColumn(1, _T("TimeDateStamp"), LVCFMT_LEFT, 150);
	m_listDebugDir.InsertColumn(2, _T("MajorVersion"), LVCFMT_LEFT, 100);
	m_listDebugDir.InsertColumn(3, _T("MinorVersion"), LVCFMT_LEFT, 100);
	m_listDebugDir.InsertColumn(4, _T("Type"), LVCFMT_LEFT, 90);
	m_listDebugDir.InsertColumn(5, _T("SizeOfData"), LVCFMT_LEFT, 100);
	m_listDebugDir.InsertColumn(6, _T("AddressOfRawData"), LVCFMT_LEFT, 170);
	m_listDebugDir.InsertColumn(7, _T("PointerToRawData"), LVCFMT_LEFT, 140);

	std::map<DWORD, std::wstring> _mapDebugType {
		{ IMAGE_DEBUG_TYPE_UNKNOWN, TEXT("IMAGE_DEBUG_TYPE_UNKNOWN") },
	{ IMAGE_DEBUG_TYPE_COFF, TEXT("IMAGE_DEBUG_TYPE_COFF") },
	{ IMAGE_DEBUG_TYPE_CODEVIEW, TEXT("IMAGE_DEBUG_TYPE_CODEVIEW") },
	{ IMAGE_DEBUG_TYPE_FPO, TEXT("IMAGE_DEBUG_TYPE_FPO") },
	{ IMAGE_DEBUG_TYPE_MISC, TEXT("IMAGE_DEBUG_TYPE_MISC") },
	{ IMAGE_DEBUG_TYPE_EXCEPTION, TEXT("IMAGE_DEBUG_TYPE_EXCEPTION") },
	{ IMAGE_DEBUG_TYPE_FIXUP, TEXT("IMAGE_DEBUG_TYPE_FIXUP") },
	{ IMAGE_DEBUG_TYPE_OMAP_TO_SRC, TEXT("IMAGE_DEBUG_TYPE_OMAP_TO_SRC") },
	{ IMAGE_DEBUG_TYPE_OMAP_FROM_SRC, TEXT("IMAGE_DEBUG_TYPE_OMAP_FROM_SRC") },
	{ IMAGE_DEBUG_TYPE_BORLAND, TEXT("IMAGE_DEBUG_TYPE_BORLAND") },
	{ IMAGE_DEBUG_TYPE_RESERVED10, TEXT("IMAGE_DEBUG_TYPE_RESERVED10") },
	{ IMAGE_DEBUG_TYPE_CLSID, TEXT("IMAGE_DEBUG_TYPE_CLSID") },
	{ IMAGE_DEBUG_TYPE_VC_FEATURE, TEXT("IMAGE_DEBUG_TYPE_VC_FEATURE") },
	{ IMAGE_DEBUG_TYPE_POGO, TEXT("IMAGE_DEBUG_TYPE_POGO") },
	{ IMAGE_DEBUG_TYPE_ILTCG, TEXT("IMAGE_DEBUG_TYPE_ILTCG") },
	{ IMAGE_DEBUG_TYPE_MPX, TEXT("IMAGE_DEBUG_TYPE_MPX") },
	{ IMAGE_DEBUG_TYPE_REPRO, TEXT("IMAGE_DEBUG_TYPE_REPRO") }
	};

	int _listindex = 0;
	TCHAR str[MAX_PATH] { };

	for (auto& i : *_pDebugDir)
	{
		swprintf_s(str, 9, L"%08X", i.Characteristics);
		m_listDebugDir.InsertItem(_listindex, str);
		swprintf_s(str, MAX_PATH, L"%08X", i.TimeDateStamp);
		m_listDebugDir.SetItemText(_listindex, 1, str);
		if (i.TimeDateStamp)
		{
			__time64_t _time = i.TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &_time);
			m_listDebugDir.SetItemToolTip(_listindex, 1, str, TEXT("Time / Date:"));
		}
		swprintf_s(str, 5, L"%04u", i.MajorVersion);
		m_listDebugDir.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%04u", i.MinorVersion);
		m_listDebugDir.SetItemText(_listindex, 3, str);
		swprintf_s(str, 9, L"%08X", i.Type);
		m_listDebugDir.SetItemText(_listindex, 4, str);
		for (auto&j : _mapDebugType)
			if (j.first == i.Type)
				m_listDebugDir.SetItemToolTip(_listindex, 4, j.second);
		swprintf_s(str, 9, L"%08X", i.SizeOfData);
		m_listDebugDir.SetItemText(_listindex, 5, str);
		swprintf_s(str, 9, L"%08X", i.AddressOfRawData);
		m_listDebugDir.SetItemText(_listindex, 6, str);
		swprintf_s(str, 9, L"%08X", i.PointerToRawData);
		m_listDebugDir.SetItemText(_listindex, 7, str);

		_listindex++;
	}

	return 0;
}

int CViewRightTop::listCreateTLSDir()
{
	PLIBPE_TLS _pTLSDir { };
	if (m_pLibpe->GetTLSTable(&_pTLSDir) != S_OK)
		return -1;

	m_listTLSDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_TLS_DIR);
	m_listTLSDir.ShowWindow(SW_HIDE);
	m_listTLSDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listTLSDir.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 250);
	m_listTLSDir.InsertColumn(1, _T("Size [BYTES]"), LVCFMT_LEFT, 110);
	m_listTLSDir.InsertColumn(2, _T("Value"), LVCFMT_LEFT, 150);

	std::map<DWORD, std::wstring> _mapCharact {
		{ IMAGE_SCN_ALIGN_1BYTES, TEXT("IMAGE_SCN_ALIGN_1BYTES") },
	{ IMAGE_SCN_ALIGN_2BYTES, TEXT("IMAGE_SCN_ALIGN_2BYTES") },
	{ IMAGE_SCN_ALIGN_4BYTES, TEXT("IMAGE_SCN_ALIGN_4BYTES") },
	{ IMAGE_SCN_ALIGN_8BYTES, TEXT("IMAGE_SCN_ALIGN_8BYTES") },
	{ IMAGE_SCN_ALIGN_16BYTES, TEXT("IMAGE_SCN_ALIGN_16BYTES") },
	{ IMAGE_SCN_ALIGN_32BYTES, TEXT("IMAGE_SCN_ALIGN_32BYTES") },
	{ IMAGE_SCN_ALIGN_64BYTES, TEXT("IMAGE_SCN_ALIGN_64BYTES") },
	{ IMAGE_SCN_ALIGN_128BYTES, TEXT("IMAGE_SCN_ALIGN_128BYTES") },
	{ IMAGE_SCN_ALIGN_256BYTES, TEXT("IMAGE_SCN_ALIGN_256BYTES") },
	{ IMAGE_SCN_ALIGN_512BYTES, TEXT("IMAGE_SCN_ALIGN_512BYTES") },
	{ IMAGE_SCN_ALIGN_1024BYTES, TEXT("IMAGE_SCN_ALIGN_1024BYTES") },
	{ IMAGE_SCN_ALIGN_2048BYTES, TEXT("IMAGE_SCN_ALIGN_2048BYTES") },
	{ IMAGE_SCN_ALIGN_4096BYTES, TEXT("IMAGE_SCN_ALIGN_4096BYTES") },
	{ IMAGE_SCN_ALIGN_8192BYTES, TEXT("IMAGE_SCN_ALIGN_8192BYTES") },
	{ IMAGE_SCN_ALIGN_MASK, TEXT("IMAGE_SCN_ALIGN_MASK") }
	};

	int _listindex = 0;
	TCHAR str[MAX_PATH] { };

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
	{
		const IMAGE_TLS_DIRECTORY32*  _pTLSDir32 = &std::get<0>(*_pTLSDir);

		_listindex = m_listTLSDir.InsertItem(_listindex, _T("StartAddressOfRawData"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir32->StartAddressOfRawData));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pTLSDir32->StartAddressOfRawData);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("EndAddressOfRawData"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir32->EndAddressOfRawData));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pTLSDir32->EndAddressOfRawData);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("AddressOfIndex"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir32->AddressOfIndex));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pTLSDir32->AddressOfIndex);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("AddressOfCallBacks"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir32->AddressOfCallBacks));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pTLSDir32->AddressOfCallBacks);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("SizeOfZeroFill"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir32->SizeOfZeroFill));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pTLSDir32->SizeOfZeroFill);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("Characteristics"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir32->Characteristics));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pTLSDir32->Characteristics);
		m_listTLSDir.SetItemText(_listindex, 2, str);
		for (auto& i : _mapCharact)
			if (i.first == _pTLSDir32->Characteristics)
				m_listTLSDir.SetItemToolTip(_listindex, 2, i.second, TEXT("Characteristics:"));
	}
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
	{
		const IMAGE_TLS_DIRECTORY64* _pTLSDir64 = &std::get<1>(*_pTLSDir);

		_listindex = m_listTLSDir.InsertItem(_listindex, _T("StartAddressOfRawData"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir64->StartAddressOfRawData));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pTLSDir64->StartAddressOfRawData);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("EndAddressOfRawData"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir64->EndAddressOfRawData));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pTLSDir64->EndAddressOfRawData);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("AddressOfIndex"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir64->AddressOfIndex));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pTLSDir64->AddressOfIndex);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("AddressOfCallBacks"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir64->AddressOfCallBacks));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pTLSDir64->AddressOfCallBacks);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("SizeOfZeroFill"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir64->SizeOfZeroFill));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pTLSDir64->SizeOfZeroFill);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		_listindex = m_listTLSDir.InsertItem(_listindex + 1, _T("Characteristics"));
		swprintf_s(str, 3, L"%u", sizeof(_pTLSDir64->Characteristics));
		m_listTLSDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pTLSDir64->Characteristics);
		m_listTLSDir.SetItemText(_listindex, 2, str);

		for (auto& i : _mapCharact)
			if (i.first == _pTLSDir64->Characteristics)
				m_listTLSDir.SetItemToolTip(_listindex, 2, i.second, TEXT("Characteristics:"));
	}

	return 0;
}

int CViewRightTop::listCreateLoadConfigDir()
{
	PLIBPE_LOADCONFIGTABLE _pLoadConfigTable { };
	if (m_pLibpe->GetLoadConfigTable(&_pLoadConfigTable) != S_OK)
		return -1;

	PLIBPE_DATADIRS _pDirs { };
	if (m_pLibpe->GetDataDirectories(&_pDirs) != S_OK)
		return -1;

	m_listLoadConfigDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_LOAD_CONFIG_DIR);
	m_listLoadConfigDir.ShowWindow(SW_HIDE);
	m_listLoadConfigDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listLoadConfigDir.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 330);
	m_listLoadConfigDir.InsertColumn(1, _T("Size [BYTES]"), LVCFMT_LEFT, 110);
	m_listLoadConfigDir.InsertColumn(2, _T("Value"), LVCFMT_LEFT, 300);

	std::map<WORD, std::wstring> _mapGuardFlags {
		{ IMAGE_GUARD_CF_INSTRUMENTED, TEXT("IMAGE_GUARD_CF_INSTRUMENTED\n Module performs control flow integrity checks using system-supplied support") },
	{ IMAGE_GUARD_CFW_INSTRUMENTED, TEXT("IMAGE_GUARD_CFW_INSTRUMENTED\n Module performs control flow and write integrity checks") },
	{ IMAGE_GUARD_CF_FUNCTION_TABLE_PRESENT, TEXT("IMAGE_GUARD_CF_FUNCTION_TABLE_PRESENT\n Module contains valid control flow target metadata") },
	{ IMAGE_GUARD_SECURITY_COOKIE_UNUSED, TEXT("IMAGE_GUARD_SECURITY_COOKIE_UNUSED\n Module does not make use of the /GS security cookie") },
	{ IMAGE_GUARD_PROTECT_DELAYLOAD_IAT, TEXT("IMAGE_GUARD_PROTECT_DELAYLOAD_IAT\n Module supports read only delay load IAT") },
	{ IMAGE_GUARD_DELAYLOAD_IAT_IN_ITS_OWN_SECTION, TEXT("IMAGE_GUARD_DELAYLOAD_IAT_IN_ITS_OWN_SECTION\n Delayload import table in its own .didat section (with nothing else in it) that can be freely reprotected") },
	{ IMAGE_GUARD_CF_EXPORT_SUPPRESSION_INFO_PRESENT, TEXT("IMAGE_GUARD_CF_EXPORT_SUPPRESSION_INFO_PRESENT\n Module contains suppressed export information. This also infers that the address taken IAT table is also present in the load config.") },
	{ IMAGE_GUARD_CF_ENABLE_EXPORT_SUPPRESSION, TEXT("IMAGE_GUARD_CF_ENABLE_EXPORT_SUPPRESSION\n Module enables suppression of exports") },
	{ IMAGE_GUARD_CF_LONGJUMP_TABLE_PRESENT, TEXT("IMAGE_GUARD_CF_LONGJUMP_TABLE_PRESENT\n Module contains longjmp target information") },
	{ IMAGE_GUARD_RF_INSTRUMENTED, TEXT("IMAGE_GUARD_RF_INSTRUMENTED\n Module contains return flow instrumentation and metadata") },
	{ IMAGE_GUARD_RF_ENABLE, TEXT("IMAGE_GUARD_RF_ENABLE\n Module requests that the OS enable return flow protection") },
	{ IMAGE_GUARD_RF_STRICT, TEXT("IMAGE_GUARD_RF_STRICT\n Module requests that the OS enable return flow protection in strict mode") },
	{ IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_MASK, TEXT("IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_MASK\n Stride of Guard CF function table encoded in these bits (additional count of bytes per element)") },
	{ IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_SHIFT, TEXT("IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_SHIFT\n Shift to right-justify Guard CF function table stride") }
	};

	const IMAGE_LOAD_CONFIG_DIRECTORY32* _pLoadConfDir32 { };
	const IMAGE_LOAD_CONFIG_DIRECTORY64* _pLoadConfDir64 { };
	int _listindex = 0;
	TCHAR str[MAX_PATH] { };
	std::wstring _tooltip { };
	DWORD _dwLCTSize = std::get<0>(_pDirs->at(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG)).Size;
	DWORD _dwTotalSize { };

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
	{
		_pLoadConfDir32 = &std::get<0>(*_pLoadConfigTable);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex, _T("Size"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->Size));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->Size);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("TimeDateStamp"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->TimeDateStamp));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, MAX_PATH, L"%08X", _pLoadConfDir32->TimeDateStamp);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);
		if (_pLoadConfDir32->TimeDateStamp)
		{
			__time64_t _time = _pLoadConfDir32->TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &_time);
			m_listLoadConfigDir.SetItemToolTip(_listindex, 2, str, TEXT("Time / Date:"));
		}

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("MajorVersion"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->MajorVersion));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 5, L"%04X", _pLoadConfDir32->MajorVersion);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("MinorVersion"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->MinorVersion));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 5, L"%04X", _pLoadConfDir32->MinorVersion);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GlobalFlagsClear"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GlobalFlagsClear));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GlobalFlagsClear);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GlobalFlagsSet"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GlobalFlagsSet));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GlobalFlagsSet);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CriticalSectionDefaultTimeout"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->CriticalSectionDefaultTimeout));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->CriticalSectionDefaultTimeout);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DeCommitFreeBlockThreshold"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->DeCommitFreeBlockThreshold));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->DeCommitFreeBlockThreshold);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DeCommitTotalFreeThreshold"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->DeCommitTotalFreeThreshold));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->DeCommitTotalFreeThreshold);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("LockPrefixTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->LockPrefixTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->LockPrefixTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("MaximumAllocationSize"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->MaximumAllocationSize));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->MaximumAllocationSize);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("VirtualMemoryThreshold"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->VirtualMemoryThreshold));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->VirtualMemoryThreshold);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("ProcessHeapFlags"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->ProcessHeapFlags));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->ProcessHeapFlags);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("ProcessAffinityMask"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->ProcessAffinityMask));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->ProcessAffinityMask);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CSDVersion"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->CSDVersion));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 5, L"%04X", _pLoadConfDir32->CSDVersion);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DependentLoadFlags"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->DependentLoadFlags));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 5, L"%04X", _pLoadConfDir32->DependentLoadFlags);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("EditList"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->EditList));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->EditList);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("SecurityCookie"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->SecurityCookie));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->SecurityCookie);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("SEHandlerTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->SEHandlerTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->SEHandlerTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("SEHandlerCount"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->SEHandlerCount));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->SEHandlerCount);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardCFCheckFunctionPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardCFCheckFunctionPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardCFCheckFunctionPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardCFDispatchFunctionPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardCFDispatchFunctionPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardCFDispatchFunctionPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardCFFunctionTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardCFFunctionTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardCFFunctionTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardCFFunctionCount"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardCFFunctionCount));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardCFFunctionCount);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardFlags"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardFlags));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardFlags);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);
		_tooltip.clear();
		for (auto&i : _mapGuardFlags)
			if (i.first & _pLoadConfDir32->GuardFlags)
				_tooltip += i.second + TEXT("\n");
		if (!_tooltip.empty())
			m_listLoadConfigDir.SetItemToolTip(_listindex, 2, _tooltip, TEXT("GuardFlags:"));

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CodeIntegrity.Flags"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->CodeIntegrity.Flags));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%04X", _pLoadConfDir32->CodeIntegrity.Flags);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CodeIntegrity.Catalog"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->CodeIntegrity.Catalog));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%04X", _pLoadConfDir32->CodeIntegrity.Catalog);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CodeIntegrity.CatalogOffset"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->CodeIntegrity.CatalogOffset));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->CodeIntegrity.CatalogOffset);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CodeIntegrity.Reserved"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->CodeIntegrity.Reserved));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->CodeIntegrity.Reserved);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardAddressTakenIatEntryTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardAddressTakenIatEntryTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardAddressTakenIatEntryTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardAddressTakenIatEntryCount"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardAddressTakenIatEntryCount));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardAddressTakenIatEntryCount);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardLongJumpTargetTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardLongJumpTargetTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardLongJumpTargetTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardLongJumpTargetCount"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardLongJumpTargetCount));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardLongJumpTargetCount);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DynamicValueRelocTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->DynamicValueRelocTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->DynamicValueRelocTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CHPEMetadataPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->CHPEMetadataPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->CHPEMetadataPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardRFFailureRoutine"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardRFFailureRoutine));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardRFFailureRoutine);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardRFFailureRoutineFunctionPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardRFFailureRoutineFunctionPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardRFFailureRoutineFunctionPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DynamicValueRelocTableOffset"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->DynamicValueRelocTableOffset));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->DynamicValueRelocTableOffset);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DynamicValueRelocTableSection"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->DynamicValueRelocTableSection));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%04X", _pLoadConfDir32->DynamicValueRelocTableSection);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("Reserved2"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->Reserved2));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%04X", _pLoadConfDir32->Reserved2);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardRFVerifyStackPointerFunctionPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->GuardRFVerifyStackPointerFunctionPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->GuardRFVerifyStackPointerFunctionPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("HotPatchTableOffset"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->HotPatchTableOffset));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->HotPatchTableOffset);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("Reserved3"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->Reserved3));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->Reserved3);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("EnclaveConfigurationPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir32->EnclaveConfigurationPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir32->EnclaveConfigurationPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);
	}
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
	{
		_pLoadConfDir64 = &std::get<1>(*_pLoadConfigTable);

		_dwTotalSize += sizeof(_pLoadConfDir64->Size);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex, _T("Size"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->Size));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->Size);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->TimeDateStamp);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("TimeDateStamp"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->TimeDateStamp));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, MAX_PATH, L"%08X", _pLoadConfDir64->TimeDateStamp);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);
		if (_pLoadConfDir64->TimeDateStamp)
		{
			__time64_t _time = _pLoadConfDir64->TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &_time);
			m_listLoadConfigDir.SetItemToolTip(_listindex, 2, str, TEXT("Time / Date:"));
		}

		_dwTotalSize += sizeof(_pLoadConfDir64->MajorVersion);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("MajorVersion"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->MajorVersion));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 5, L"%04X", _pLoadConfDir64->MajorVersion);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->MinorVersion);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("MinorVersion"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->MinorVersion));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 5, L"%04X", _pLoadConfDir64->MinorVersion);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GlobalFlagsClear);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GlobalFlagsClear"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GlobalFlagsClear));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->GlobalFlagsClear);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GlobalFlagsSet);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GlobalFlagsSet"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GlobalFlagsSet));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->GlobalFlagsSet);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->CriticalSectionDefaultTimeout);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CriticalSectionDefaultTimeout"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->CriticalSectionDefaultTimeout));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->CriticalSectionDefaultTimeout);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->DeCommitFreeBlockThreshold);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DeCommitFreeBlockThreshold"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->DeCommitFreeBlockThreshold));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->DeCommitFreeBlockThreshold);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->DeCommitTotalFreeThreshold);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DeCommitTotalFreeThreshold"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->DeCommitTotalFreeThreshold));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->DeCommitTotalFreeThreshold);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->LockPrefixTable);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("LockPrefixTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->LockPrefixTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->LockPrefixTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->MaximumAllocationSize);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("MaximumAllocationSize"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->MaximumAllocationSize));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->MaximumAllocationSize);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->VirtualMemoryThreshold);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("VirtualMemoryThreshold"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->VirtualMemoryThreshold));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->VirtualMemoryThreshold);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->ProcessHeapFlags);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("ProcessHeapFlags"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->ProcessHeapFlags));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->ProcessHeapFlags);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->ProcessAffinityMask);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("ProcessAffinityMask"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->ProcessAffinityMask));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->ProcessAffinityMask);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->CSDVersion);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CSDVersion"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->CSDVersion));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 5, L"%04X", _pLoadConfDir64->CSDVersion);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->DependentLoadFlags);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DependentLoadFlags"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->DependentLoadFlags));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 5, L"%04X", _pLoadConfDir64->DependentLoadFlags);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->EditList);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("EditList"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->EditList));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->EditList);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->SecurityCookie);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("SecurityCookie"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->SecurityCookie));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->SecurityCookie);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->SEHandlerTable);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("SEHandlerTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->SEHandlerTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->SEHandlerTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->SEHandlerCount);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("SEHandlerCount"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->SEHandlerCount));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->SEHandlerCount);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardCFCheckFunctionPointer);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardCFCheckFunctionPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardCFCheckFunctionPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardCFCheckFunctionPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardCFDispatchFunctionPointer);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardCFDispatchFunctionPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardCFDispatchFunctionPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardCFDispatchFunctionPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardCFFunctionTable);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardCFFunctionTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardCFFunctionTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardCFFunctionTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardCFFunctionCount);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardCFFunctionCount"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardCFFunctionCount));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardCFFunctionCount);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardFlags);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardFlags"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardFlags));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->GuardFlags);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);
		_tooltip.clear();
		for (auto&i : _mapGuardFlags)
			if (i.first & _pLoadConfDir64->GuardFlags)
				_tooltip += i.second + TEXT("\n");
		if (!_tooltip.empty())
			m_listLoadConfigDir.SetItemToolTip(_listindex, 2, _tooltip, TEXT("GuardFlags:"));

		_dwTotalSize += sizeof(_pLoadConfDir64->CodeIntegrity.Flags);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CodeIntegrity.Flags"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->CodeIntegrity.Flags));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%04X", _pLoadConfDir64->CodeIntegrity.Flags);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->CodeIntegrity.Catalog);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CodeIntegrity.Catalog"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->CodeIntegrity.Catalog));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%04X", _pLoadConfDir64->CodeIntegrity.Catalog);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->CodeIntegrity.CatalogOffset);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CodeIntegrity.CatalogOffset"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->CodeIntegrity.CatalogOffset));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->CodeIntegrity.CatalogOffset);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->CodeIntegrity.Reserved);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CodeIntegrity.Reserved"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->CodeIntegrity.Reserved));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->CodeIntegrity.Reserved);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardAddressTakenIatEntryTable);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardAddressTakenIatEntryTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardAddressTakenIatEntryTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardAddressTakenIatEntryTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardAddressTakenIatEntryCount);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardAddressTakenIatEntryCount"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardAddressTakenIatEntryCount));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardAddressTakenIatEntryCount);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardLongJumpTargetTable);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardLongJumpTargetTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardLongJumpTargetTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardLongJumpTargetTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardLongJumpTargetCount);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardLongJumpTargetCount"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardLongJumpTargetCount));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardLongJumpTargetCount);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->DynamicValueRelocTable);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DynamicValueRelocTable"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->DynamicValueRelocTable));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->DynamicValueRelocTable);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->CHPEMetadataPointer);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("CHPEMetadataPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->CHPEMetadataPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->CHPEMetadataPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardRFFailureRoutine);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardRFFailureRoutine"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardRFFailureRoutine));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardRFFailureRoutine);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardRFFailureRoutineFunctionPointer);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardRFFailureRoutineFunctionPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardRFFailureRoutineFunctionPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardRFFailureRoutineFunctionPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->DynamicValueRelocTableOffset);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DynamicValueRelocTableOffset"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->DynamicValueRelocTableOffset));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->DynamicValueRelocTableOffset);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->DynamicValueRelocTableSection);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("DynamicValueRelocTableSection"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->DynamicValueRelocTableSection));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%04X", _pLoadConfDir64->DynamicValueRelocTableSection);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->Reserved2);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("Reserved2"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->Reserved2));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%04X", _pLoadConfDir64->Reserved2);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->GuardRFVerifyStackPointerFunctionPointer);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("GuardRFVerifyStackPointerFunctionPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->GuardRFVerifyStackPointerFunctionPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->GuardRFVerifyStackPointerFunctionPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->HotPatchTableOffset);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("HotPatchTableOffset"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->HotPatchTableOffset));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->HotPatchTableOffset);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->Reserved3);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("Reserved3"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->Reserved3));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pLoadConfDir64->Reserved3);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);

		_dwTotalSize += sizeof(_pLoadConfDir64->EnclaveConfigurationPointer);
		if (_dwTotalSize > _dwLCTSize)
			return 0;
		_listindex = m_listLoadConfigDir.InsertItem(_listindex + 1, _T("EnclaveConfigurationPointer"));
		swprintf_s(str, 3, L"%u", sizeof(_pLoadConfDir64->EnclaveConfigurationPointer));
		m_listLoadConfigDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 17, L"%016llX", _pLoadConfDir64->EnclaveConfigurationPointer);
		m_listLoadConfigDir.SetItemText(_listindex, 2, str);
	}

	return 0;
}

int CViewRightTop::listCreateBoundImportDir()
{
	PLIBPE_BOUNDIMPORT _pBoundImport { };

	if (m_pLibpe->GetBoundImportTable(&_pBoundImport) != S_OK)
		return -1;

	m_listBoundImportDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_DELAY_IMPORT_DIR);
	m_listBoundImportDir.ShowWindow(SW_HIDE);
	m_listBoundImportDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listBoundImportDir.InsertColumn(0, _T("Module Name"), LVCFMT_CENTER, 290);
	m_listBoundImportDir.InsertColumn(1, _T("TimeDateStamp"), LVCFMT_LEFT, 130);
	m_listBoundImportDir.InsertColumn(2, _T("OffsetModuleName"), LVCFMT_LEFT, 140);
	m_listBoundImportDir.InsertColumn(3, _T("NumberOfModuleForwarderRefs"), LVCFMT_LEFT, 220);

	WCHAR str[MAX_PATH] { };
	int _listindex = 0;
	const IMAGE_BOUND_IMPORT_DESCRIPTOR* _pBoundImpDir { };

	for (auto& i : *_pBoundImport)
	{
		_pBoundImpDir = &std::get<0>(i);
		swprintf_s(str, MAX_PATH, L"%S", std::get<1>(i).c_str());
		m_listBoundImportDir.InsertItem(_listindex, str);
		swprintf_s(str, MAX_PATH, L"%08X", _pBoundImpDir->TimeDateStamp);
		m_listBoundImportDir.SetItemText(_listindex, 1, str);
		if (_pBoundImpDir->TimeDateStamp)
		{
			__time64_t _time = _pBoundImpDir->TimeDateStamp;
			_wctime64_s(str, MAX_PATH, &_time);
			m_listBoundImportDir.SetItemToolTip(_listindex, 1, str, TEXT("Time / Date:"));
		}
		swprintf_s(str, 5, L"%04X", _pBoundImpDir->OffsetModuleName);
		m_listBoundImportDir.SetItemText(_listindex, 2, str);
		swprintf_s(str, 5, L"%04u", _pBoundImpDir->NumberOfModuleForwarderRefs);
		m_listBoundImportDir.SetItemText(_listindex, 3, str);

		_listindex++;
	}

	return 0;
}

int CViewRightTop::listCreateDelayImportDir()
{
	PLIBPE_DELAYIMPORT _pDelayImport { };

	if (m_pLibpe->GetDelayImportTable(&_pDelayImport) != S_OK)
		return -1;

	m_listDelayImportDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_DELAY_IMPORT_DIR);
	m_listDelayImportDir.ShowWindow(SW_HIDE);
	m_listDelayImportDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDelayImportDir.InsertColumn(0, _T("Module Name (funcs number)"), LVCFMT_CENTER, 290);
	m_listDelayImportDir.InsertColumn(1, _T("Attributes"), LVCFMT_LEFT, 100);
	m_listDelayImportDir.InsertColumn(2, _T("DllNameRVA"), LVCFMT_LEFT, 105);
	m_listDelayImportDir.InsertColumn(3, _T("ModuleHandleRVA"), LVCFMT_LEFT, 140);
	m_listDelayImportDir.InsertColumn(4, _T("ImportAddressTableRVA"), LVCFMT_LEFT, 160);
	m_listDelayImportDir.InsertColumn(5, _T("ImportNameTableRVA"), LVCFMT_LEFT, 150);
	m_listDelayImportDir.InsertColumn(6, _T("BoundImportAddressTableRVA"), LVCFMT_LEFT, 150);
	m_listDelayImportDir.InsertColumn(7, _T("UnloadInformationTableRVA"), LVCFMT_LEFT, 150);
	m_listDelayImportDir.InsertColumn(8, _T("TimeDateStamp"), LVCFMT_LEFT, 115);

	int _listindex = 0;
	WCHAR str[MAX_PATH] { };

	for (auto& i : *_pDelayImport)
	{
		const IMAGE_DELAYLOAD_DESCRIPTOR* _pDelayImpDir = &std::get<0>(i);
		swprintf_s(str, MAX_PATH, L"%S (%u)", std::get<1>(i).c_str(), std::get<2>(i).size());
		m_listDelayImportDir.InsertItem(_listindex, str);
		swprintf_s(str, 9, L"%08X", _pDelayImpDir->Attributes.AllAttributes);
		m_listDelayImportDir.SetItemText(_listindex, 1, str);
		swprintf_s(str, 9, L"%08X", _pDelayImpDir->DllNameRVA);
		m_listDelayImportDir.SetItemText(_listindex, 2, str);
		swprintf_s(str, 9, L"%08X", _pDelayImpDir->ModuleHandleRVA);
		m_listDelayImportDir.SetItemText(_listindex, 3, str);
		swprintf_s(str, 9, L"%08X", _pDelayImpDir->ImportAddressTableRVA);
		m_listDelayImportDir.SetItemText(_listindex, 4, str);
		swprintf_s(str, 9, L"%08X", _pDelayImpDir->ImportNameTableRVA);
		m_listDelayImportDir.SetItemText(_listindex, 5, str);
		swprintf_s(str, 9, L"%08X", _pDelayImpDir->BoundImportAddressTableRVA);
		m_listDelayImportDir.SetItemText(_listindex, 6, str);
		swprintf_s(str, 9, L"%08X", _pDelayImpDir->UnloadInformationTableRVA);
		m_listDelayImportDir.SetItemText(_listindex, 7, str);
		swprintf_s(str, 9, L"%08X", _pDelayImpDir->TimeDateStamp);
		m_listDelayImportDir.SetItemText(_listindex, 8, str);

		_listindex++;
	}

	return 0;
}

int CViewRightTop::listCreateCOMDir()
{
	PLIBPE_COM_DESCRIPTOR _pCOMDesc { };

	if (m_pLibpe->GetCOMDescriptorTable(&_pCOMDesc) != S_OK)
		return -1;

	m_listCOMDir.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_DELAY_IMPORT_DIR);
	m_listCOMDir.ShowWindow(SW_HIDE);
	m_listCOMDir.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listCOMDir.InsertColumn(0, _T("Name"), LVCFMT_CENTER, 300);
	m_listCOMDir.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 100);

	std::map<DWORD, std::wstring> _mapFlags {
		{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_ILONLY, L"COMIMAGE_FLAGS_ILONLY" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_32BITREQUIRED, L"COMIMAGE_FLAGS_32BITREQUIRED" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_IL_LIBRARY, L"COMIMAGE_FLAGS_IL_LIBRARY" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_STRONGNAMESIGNED, L"COMIMAGE_FLAGS_STRONGNAMESIGNED" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_NATIVE_ENTRYPOINT, L"COMIMAGE_FLAGS_NATIVE_ENTRYPOINT" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_TRACKDEBUGDATA, L"COMIMAGE_FLAGS_TRACKDEBUGDATA" },
	{ ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_32BITPREFERRED, L"COMIMAGE_FLAGS_32BITPREFERRED" }
	};

	int _listindex = 0;
	WCHAR str[MAX_PATH] { };
	std::wstring _strToolTip { };

	m_listCOMDir.InsertItem(_listindex, L"cb");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->cb);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"MajorRuntimeVersion");
	swprintf_s(str, 5, L"%04X", _pCOMDesc->MajorRuntimeVersion);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"MinorRuntimeVersion");
	swprintf_s(str, 5, L"%04X", _pCOMDesc->MinorRuntimeVersion);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"MetaData.RVA");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->MetaData.VirtualAddress);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"MetaData.Size");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->MetaData.Size);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"Flags");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->Flags);
	m_listCOMDir.SetItemText(_listindex, 1, str);
	for (auto&i : _mapFlags)
		if (i.first & _pCOMDesc->Flags)
			_strToolTip += i.second + L"\n";
	if (!_strToolTip.empty())
		m_listCOMDir.SetItemToolTip(_listindex, 1, _strToolTip, L"Flags:");

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"EntryPointToken");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->EntryPointToken);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"Resources.RVA");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->Resources.VirtualAddress);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"Resources.Size");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->Resources.Size);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"StrongNameSignature.RVA");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->StrongNameSignature.VirtualAddress);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"StrongNameSignature.Size");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->StrongNameSignature.Size);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"CodeManagerTable.RVA");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->CodeManagerTable.VirtualAddress);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"CodeManagerTable.Size");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->CodeManagerTable.Size);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"VTableFixups.RVA");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->VTableFixups.VirtualAddress);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"VTableFixups.Size");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->VTableFixups.Size);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"ExportAddressTableJumps.RVA");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->ExportAddressTableJumps.VirtualAddress);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"ExportAddressTableJumps.Size");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->ExportAddressTableJumps.Size);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"ManagedNativeHeader.RVA");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->ManagedNativeHeader.VirtualAddress);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	_listindex = m_listCOMDir.InsertItem(_listindex + 1, L"ManagedNativeHeader.Size");
	swprintf_s(str, 9, L"%08X", _pCOMDesc->ManagedNativeHeader.Size);
	m_listCOMDir.SetItemText(_listindex, 1, str);

	return 0;
}