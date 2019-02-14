/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "ViewLeft.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CViewLeft, CView)

BEGIN_MESSAGE_MAP(CViewLeft, CView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CViewLeft::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	
	DWORD dwFileSummary;
	if (m_pLibpe->GetImageFlags(dwFileSummary) != S_OK)
		return;

	m_ImgListRootTree.Create(16, 16, ILC_COLORDDB, 0, 2);
	const int iconHdr = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_HEADER_ICON));
	const int iconDirs = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_DIR_ICON));

	CRect rect;
	GetClientRect(&rect);
	m_stTreeMain.Create(TVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | TVS_HASLINES | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		rect, this, IDC_TREE_LEFT_MAIN);

	m_stTreeMain.SetImageList(&m_ImgListRootTree, TVSIL_NORMAL);

	const HTREEITEM hTreeRoot = m_stTreeMain.InsertItem(L"FILE SUMMARY");
	m_stTreeMain.SetItemState(hTreeRoot, TVIS_BOLD, TVIS_BOLD);
	m_stTreeMain.SetItemData(hTreeRoot, IDC_SHOW_FILE_SUMMARY);

	if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_DOSHEADER))
	{
		const HTREEITEM hTreeDosHeader = m_stTreeMain.InsertItem(L"MS-DOS Header [IMAGE_DOS_HEADER]", iconHdr, iconHdr, hTreeRoot);
		m_stTreeMain.SetItemData(hTreeDosHeader, IDC_LIST_DOSHEADER);
	}
	if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_RICHHEADER))
	{
		const HTREEITEM hTreeDosRich = m_stTreeMain.InsertItem(L"\u00ABRich\u00BB Header", iconHdr, iconHdr, hTreeRoot);
		m_stTreeMain.SetItemData(hTreeDosRich, IDC_LIST_RICHHEADER);
	}

	HTREEITEM hTreeNTHeaders { };
	HTREEITEM hTreeOptHeader { };

	if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_PE32) && ImageHasFlag(dwFileSummary, IMAGE_FLAG_NTHEADER))
		hTreeNTHeaders = m_stTreeMain.InsertItem(L"NT Header [IMAGE_NT_HEADERS32]", iconHdr, iconHdr, hTreeRoot);
	else if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_PE64) && ImageHasFlag(dwFileSummary, IMAGE_FLAG_NTHEADER))
		hTreeNTHeaders = m_stTreeMain.InsertItem(L"NT Header [IMAGE_NT_HEADERS64]", iconHdr, iconHdr, hTreeRoot);

	if (hTreeNTHeaders)
	{
		m_stTreeMain.SetItemData(hTreeNTHeaders, IDC_LIST_NTHEADER);

		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_FILEHEADER))
		{
			const HTREEITEM hTreeFileHeader = m_stTreeMain.InsertItem(L"File Header [IMAGE_FILE_HEADER]", iconHdr, iconHdr,
				hTreeNTHeaders);
			m_stTreeMain.SetItemData(hTreeFileHeader, IDC_LIST_FILEHEADER);
		}

		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_PE32) && ImageHasFlag(dwFileSummary, IMAGE_FLAG_OPTHEADER))
			hTreeOptHeader = m_stTreeMain.InsertItem(L"Optional Header [IMAGE_OPTIONAL_HEADER32]", iconHdr, iconHdr, hTreeNTHeaders);
		else if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_PE64) && ImageHasFlag(dwFileSummary, IMAGE_FLAG_OPTHEADER))
			hTreeOptHeader = m_stTreeMain.InsertItem(L"Optional Header [IMAGE_OPTIONAL_HEADER64]", iconHdr, iconHdr, hTreeNTHeaders);

		m_stTreeMain.SetItemData(hTreeOptHeader, IDC_LIST_OPTIONALHEADER);
		m_stTreeMain.Expand(hTreeNTHeaders, TVE_EXPAND);
	}

	if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_SECTIONS))
	{
		const HTREEITEM hTreeSecHeaders = m_stTreeMain.InsertItem(L"Sections Headers [IMAGE_SECTION_HEADER]", iconHdr, iconHdr, hTreeRoot);
		m_stTreeMain.SetItemData(hTreeSecHeaders, IDC_LIST_SECHEADERS);
	}

	PCLIBPE_DATADIRS_VEC vecDataDirs { };
	if (m_pLibpe->GetDataDirectories(vecDataDirs) == S_OK)
	{
		if (hTreeOptHeader)
		{
			const HTREEITEM hTreeDataDirs = m_stTreeMain.InsertItem(L"Data Directories [IMAGE_DATA_DIRECTORY]", iconHdr, iconHdr, hTreeOptHeader);
			m_stTreeMain.SetItemData(hTreeDataDirs, IDC_LIST_DATADIRECTORIES);
			m_stTreeMain.Expand(hTreeOptHeader, TVE_EXPAND);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_EXPORT)) {
			const HTREEITEM hTreeExportDir = m_stTreeMain.InsertItem(L"Export Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeExportDir, IDC_LIST_EXPORT);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_IMPORT)) {
			const HTREEITEM hTreeImportDir = m_stTreeMain.InsertItem(L"Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeImportDir, IDC_LIST_IMPORT);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_RESOURCE)) {
			const HTREEITEM hTreeResourceDir = m_stTreeMain.InsertItem(L"Resource Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeResourceDir, IDC_TREE_RESOURCE);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_EXCEPTION)) {
			const HTREEITEM hTreeExceptionDir = m_stTreeMain.InsertItem(L"Exception Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeExceptionDir, IDC_LIST_EXCEPTION);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_SECURITY)) {
			const HTREEITEM hTreeSecurityDir = m_stTreeMain.InsertItem(L"Security Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeSecurityDir, IDC_LIST_SECURITY);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_BASERELOC)) {
			const HTREEITEM hTreeRelocationDir = m_stTreeMain.InsertItem(L"Relocations Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeRelocationDir, IDC_LIST_RELOCATIONS);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_DEBUG)) {
			const HTREEITEM hTreeDebugDir = m_stTreeMain.InsertItem(L"Debug Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeDebugDir, IDC_LIST_DEBUG);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_ARCHITECTURE)) {
			const HTREEITEM hTreeArchitectureDir = m_stTreeMain.InsertItem(L"Architecture Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeArchitectureDir, IDC_LIST_ARCHITECTURE);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_GLOBALPTR)) {
			const HTREEITEM hTreeGlobalPTRDir = m_stTreeMain.InsertItem(L"GlobalPTR Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeGlobalPTRDir, IDC_LIST_GLOBALPTR);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_TLS)) {
			const HTREEITEM hTreeTLSDir = m_stTreeMain.InsertItem(L"TLS Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeTLSDir, IDC_LIST_TLS);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_LOADCONFIG)) {
			const HTREEITEM hTreeLoadConfigDir = m_stTreeMain.InsertItem(L"Load Config Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeLoadConfigDir, IDC_LIST_LOADCONFIG);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_BOUNDIMPORT)) {
			const HTREEITEM hTreeBoundImportDir = m_stTreeMain.InsertItem(L"Bound Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeBoundImportDir, IDC_LIST_BOUNDIMPORT);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_IAT)) {
			const HTREEITEM hTreeIATDir = m_stTreeMain.InsertItem(L"IAT Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeIATDir, IDC_LIST_IAT);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_DELAYIMPORT)) {
			const HTREEITEM hTreeDelayImportDir = m_stTreeMain.InsertItem(L"Delay Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeDelayImportDir, IDC_LIST_DELAYIMPORT);
		}
		if (ImageHasFlag(dwFileSummary, IMAGE_FLAG_COMDESCRIPTOR)) {
			const HTREEITEM hTreeCOMDescriptorDir = m_stTreeMain.InsertItem(L"COM Descriptor Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeCOMDescriptorDir, IDC_LIST_COMDESCRIPTOR);
		}
	}
	m_stTreeMain.Expand(hTreeRoot, TVE_EXPAND);
}

BOOL CViewLeft::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	const LPNMTREEVIEW pTree = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (pTree->hdr.idFrom == IDC_TREE_LEFT_MAIN && pTree->hdr.code == TVN_SELCHANGED)
		m_pMainDoc->UpdateAllViews(this, m_stTreeMain.GetItemData(pTree->itemNew.hItem));

	return CView::OnNotify(wParam, lParam, pResult);
}

void CViewLeft::OnDraw(CDC* pDC)
{
}

BOOL CViewLeft::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CViewLeft::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	m_stTreeMain.SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}