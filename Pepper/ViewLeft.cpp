#include "stdafx.h"
#include "ViewLeft.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CViewLeft, CView)

BEGIN_MESSAGE_MAP(CViewLeft, CView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CViewLeft::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	HTREEITEM hTreeNTHeaders { };
	HTREEITEM hTreeOptHeader { };

	if(!(m_pMainDoc = (CPepperDoc*)GetDocument()))
		return;

	if(!(m_pLibpe = m_pMainDoc->m_pLibpe))
		return;

	const DWORD* m_pFileSummary { };
	if (m_pLibpe->GetFileSummary(&m_pFileSummary) != S_OK)
		return;

	const DWORD m_dwFileSummary = *m_pFileSummary;

	m_ImgListRootTree.Create(16, 16, ILC_COLOR32, 0, 4);
	const int iconHdr = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_HEADER_ICON));
	const int iconDirs = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_DIR_ICON));

	CRect rect;
	GetClientRect(&rect);
	m_TreeMain.Create(TVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | TVS_HASLINES | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		rect, this, TREE_LEFT_MAIN);

	m_TreeMain.SetImageList(&m_ImgListRootTree, TVSIL_NORMAL);

	const HTREEITEM hTreeRoot = m_TreeMain.InsertItem(L"FILE SUMMARY");
	m_TreeMain.SetItemState(hTreeRoot, TVIS_BOLD, TVIS_BOLD);
	m_TreeMain.SetItemData(hTreeRoot, PE_FILE_SUMMARY);

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_DOS_HEADER_FLAG))
	{
		const HTREEITEM hTreeDosHeader = m_TreeMain.InsertItem(L"MS-DOS Header [IMAGE_DOS_HEADER]", iconHdr, iconHdr, hTreeRoot);
		m_TreeMain.SetItemData(hTreeDosHeader, LIST_DOSHEADER);
	}
	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_RICH_HEADER_FLAG))
	{
		const HTREEITEM hTreeDosRich = m_TreeMain.InsertItem(L"\u00ABRich\u00BB Header", iconHdr, iconHdr, hTreeRoot);
		m_TreeMain.SetItemData(hTreeDosRich, LIST_RICHHEADER);
	}

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
		hTreeNTHeaders = m_TreeMain.InsertItem(L"NT Header [IMAGE_NT_HEADERS32]", iconHdr, iconHdr, hTreeRoot);
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
		hTreeNTHeaders = m_TreeMain.InsertItem(L"NT Header [IMAGE_NT_HEADERS64]", iconHdr, iconHdr, hTreeRoot);

	m_TreeMain.SetItemData(hTreeNTHeaders, LIST_NTHEADER);

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_FILE_HEADER_FLAG))
	{
		const HTREEITEM hTreeFileHeader = m_TreeMain.InsertItem(L"File Header [IMAGE_FILE_HEADER]", iconHdr, iconHdr,
		                                                  hTreeNTHeaders);
		m_TreeMain.SetItemData(hTreeFileHeader, LIST_FILEHEADER);
	}

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG | IMAGE_OPTIONAL_HEADER_FLAG))
		hTreeOptHeader = m_TreeMain.InsertItem(L"Optional Header [IMAGE_OPTIONAL_HEADER32]", iconHdr, iconHdr, hTreeNTHeaders);
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG | IMAGE_OPTIONAL_HEADER_FLAG))
		hTreeOptHeader = m_TreeMain.InsertItem(L"Optional Header [IMAGE_OPTIONAL_HEADER64]", iconHdr, iconHdr, hTreeNTHeaders);

	m_TreeMain.SetItemData(hTreeOptHeader, LIST_OPTIONALHEADER);

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_SECTION_HEADERS_FLAG))
	{
		const HTREEITEM hTreeSecHeaders = m_TreeMain.InsertItem(L"Section Headers [IMAGE_SECTION_HEADER]", iconHdr, iconHdr,
		                                                  hTreeRoot);
		m_TreeMain.SetItemData(hTreeSecHeaders, LIST_SECHEADERS);
	}

	PCLIBPE_DATADIRS_VEC vecDataDirs { };
	if (m_pLibpe->GetDataDirectories(&vecDataDirs) == S_OK)
	{
		const HTREEITEM hTreeDataDirs = m_TreeMain.InsertItem(L"Data Directories [IMAGE_DATA_DIRECTORY]", iconHdr, iconHdr,
		                                                hTreeOptHeader);
		m_TreeMain.SetItemData(hTreeDataDirs, LIST_DATADIRECTORIES);

		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_EXPORT_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeExportDir = m_TreeMain.InsertItem(L"Export Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeExportDir, LIST_EXPORT);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_IMPORT_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeImportDir = m_TreeMain.InsertItem(L"Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeImportDir, LIST_IMPORT);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_RESOURCE_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeResourceDir = m_TreeMain.InsertItem(L"Resource Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeResourceDir, LIST_RESOURCE);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_EXCEPTION_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeExceptionDir = m_TreeMain.InsertItem(L"Exception Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeExceptionDir, LIST_EXCEPTION);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_SECURITY_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeSecurityDir = m_TreeMain.InsertItem(L"Security Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeSecurityDir, LIST_SECURITY);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_BASERELOC_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeRelocationDir = m_TreeMain.InsertItem(L"Relocations Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeRelocationDir, LIST_RELOCATIONS);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_DEBUG_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeDebugDir = m_TreeMain.InsertItem(L"Debug Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeDebugDir, LIST_DEBUG);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_ARCHITECTURE_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeArchitectureDir = m_TreeMain.InsertItem(L"Architecture Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeArchitectureDir, LIST_ARCHITECTURE);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_GLOBALPTR_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeGlobalPTRDir = m_TreeMain.InsertItem(L"GlobalPTR Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeGlobalPTRDir, LIST_GLOBALPTR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_TLS_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeTLSDir = m_TreeMain.InsertItem(L"TLS Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeTLSDir, LIST_TLS);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_LOADCONFIG_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeLoadConfigDir = m_TreeMain.InsertItem(L"Load Config Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeLoadConfigDir, LIST_LOADCONFIG);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_BOUNDIMPORT_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeBoundImportDir = m_TreeMain.InsertItem(L"Bound Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeBoundImportDir, LIST_BOUNDIMPORT);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_IAT_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeIATDir = m_TreeMain.InsertItem(L"IAT Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeIATDir, LIST_IAT);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_DELAYIMPORT_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeDelayImportDir = m_TreeMain.InsertItem(L"Delay Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeDelayImportDir, LIST_DELAYIMPORT);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_COMDESCRIPTOR_DIRECTORY_FLAG)) {
			const HTREEITEM hTreeCOMDescriptorDir = m_TreeMain.InsertItem(L"COM Descriptor Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeCOMDescriptorDir, LIST_COMDESCRIPTOR);
		}
	}

	m_TreeMain.Expand(hTreeRoot, TVE_EXPAND);
	m_TreeMain.Expand(hTreeNTHeaders, TVE_EXPAND);
	m_TreeMain.Expand(hTreeOptHeader, TVE_EXPAND);
}

BOOL CViewLeft::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	const LPNMTREEVIEW pTree = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (pTree->hdr.idFrom == TREE_LEFT_MAIN && pTree->hdr.code == TVN_SELCHANGED)
		m_pMainDoc->UpdateAllViews(this, m_TreeMain.GetItemData(pTree->itemNew.hItem));

	return CView::OnNotify(wParam, lParam, pResult);
}

void CViewLeft::OnDraw(CDC* /*pDC*/)
{
}

void CViewLeft::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	m_TreeMain.SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}