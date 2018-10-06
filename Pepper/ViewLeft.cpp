#include "stdafx.h"
#include "Pepper.h"
#include "PepperTreeCtrl.h"
#include "ViewLeft.h"

IMPLEMENT_DYNCREATE(CViewLeft, CView)

BEGIN_MESSAGE_MAP(CViewLeft, CView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CViewLeft::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	HTREEITEM hTreeRoot { };
	HTREEITEM hTreeDosHeader { };
	HTREEITEM hTreeDosRich { };
	HTREEITEM hTreeNTHeaders { };
	HTREEITEM hTreeFileHeader { };
	HTREEITEM hTreeOptHeader { };
	HTREEITEM hTreeDataDirs { };
	HTREEITEM hTreeSecHeaders { };
	HTREEITEM hTreeExportDir { };
	HTREEITEM hTreeImportDir { };
	HTREEITEM hTreeResourceDir { };
	HTREEITEM hTreeExceptionDir { };
	HTREEITEM hTreeSecurityDir { };
	HTREEITEM hTreeRelocationDir { };
	HTREEITEM hTreeDebugDir { };
	HTREEITEM hTreeArchitectureDir { };
	HTREEITEM hTreeGlobalPTRDir { };
	HTREEITEM hTreeTLSDir { };
	HTREEITEM hTreeLoadConfigDir { };
	HTREEITEM hTreeBoundImportDir { };
	HTREEITEM hTreeIATDir { };
	HTREEITEM hTreeDelayImportDir { };
	HTREEITEM hTreeCOMDescriptorDir { };

	if(!(m_pMainDoc = (CPepperDoc*)GetDocument()))
		return;

	if(!(m_pLibpe = m_pMainDoc->m_pLibpe))
		return;

	const DWORD* m_pFileSummary { };
	if (m_pLibpe->GetFileSummary(&m_pFileSummary) != S_OK)
		return;
	
	DWORD m_dwFileSummary = *m_pFileSummary;

	m_ImgListRootTree.Create(16, 16, ILC_COLOR32, 0, 4);
	int iconHdr = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_HEADER_ICON));
	int iconDirs = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_DIR_ICON));

	CRect rect;
	GetClientRect(&rect);
	m_TreeMain.Create(TVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | TVS_HASLINES | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		rect, this, TREEID_LEFT_MAIN);

	m_TreeMain.SetImageList(&m_ImgListRootTree, TVSIL_NORMAL);

	hTreeRoot = m_TreeMain.InsertItem(L"FILE SUMMARY");
	m_TreeMain.SetItemState(hTreeRoot, TVIS_BOLD, TVIS_BOLD);
	m_TreeMain.SetItemData(hTreeRoot, LISTID_FILE_SUMMARY);

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_DOS_HEADER_FLAG))
	{
		hTreeDosHeader = m_TreeMain.InsertItem(L"MS-DOS Header [IMAGE_DOS_HEADER]", iconHdr, iconHdr, hTreeRoot);
		m_TreeMain.SetItemData(hTreeDosHeader, LISTID_DOS_HEADER);
	}
	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_RICH_HEADER_FLAG))
	{
		hTreeDosRich = m_TreeMain.InsertItem(L"\u00ABRich\u00BB Header", iconHdr, iconHdr, hTreeRoot);
		m_TreeMain.SetItemData(hTreeDosRich, LISTID_DOS_RICH);
	}

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
		hTreeNTHeaders = m_TreeMain.InsertItem(L"NT Header [IMAGE_NT_HEADERS32]", iconHdr, iconHdr, hTreeRoot);
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
		hTreeNTHeaders = m_TreeMain.InsertItem(L"NT Header [IMAGE_NT_HEADERS64]", iconHdr, iconHdr, hTreeRoot);

	m_TreeMain.SetItemData(hTreeNTHeaders, LISTID_NT_HEADER);

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_FILE_HEADER_FLAG))
	{
		hTreeFileHeader = m_TreeMain.InsertItem(L"File Header [IMAGE_FILE_HEADER]", iconHdr, iconHdr, hTreeNTHeaders);
		m_TreeMain.SetItemData(hTreeFileHeader, LISTID_FILE_HEADER);
	}

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG | IMAGE_OPTIONAL_HEADER_FLAG))
		hTreeOptHeader = m_TreeMain.InsertItem(L"Optional Header [IMAGE_OPTIONAL_HEADER32]", iconHdr, iconHdr, hTreeNTHeaders);
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG | IMAGE_OPTIONAL_HEADER_FLAG))
		hTreeOptHeader = m_TreeMain.InsertItem(L"Optional Header [IMAGE_OPTIONAL_HEADER64]", iconHdr, iconHdr, hTreeNTHeaders);

	m_TreeMain.SetItemData(hTreeOptHeader, LISTID_OPTIONAL_HEADER);

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_SECTION_HEADERS_FLAG))
	{
		hTreeSecHeaders = m_TreeMain.InsertItem(L"Section Headers [IMAGE_SECTION_HEADER]", iconHdr, iconHdr, hTreeRoot);
		m_TreeMain.SetItemData(hTreeSecHeaders, LISTID_SECHEADERS);
	}

	PLIBPE_DATADIRS_VEC vecDataDirs { };
	if (m_pLibpe->GetDataDirectories(&vecDataDirs) == S_OK)
	{
		hTreeDataDirs = m_TreeMain.InsertItem(L"Data Directories [IMAGE_DATA_DIRECTORY]", iconHdr, iconHdr, hTreeOptHeader);
		m_TreeMain.SetItemData(hTreeDataDirs, LISTID_DATA_DIRS);

		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_EXPORT_DIRECTORY_FLAG)) {
			hTreeExportDir = m_TreeMain.InsertItem(L"Export Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeExportDir, LISTID_EXPORT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_IMPORT_DIRECTORY_FLAG)) {
			hTreeImportDir = m_TreeMain.InsertItem(L"Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeImportDir, LISTID_IMPORT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_RESOURCE_DIRECTORY_FLAG)) {
			hTreeResourceDir = m_TreeMain.InsertItem(L"Resource Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeResourceDir, LISTID_RESOURCE_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_EXCEPTION_DIRECTORY_FLAG)) {
			hTreeExceptionDir = m_TreeMain.InsertItem(L"Exception Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeExceptionDir, LISTID_EXCEPTION_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_SECURITY_DIRECTORY_FLAG)) {
			hTreeSecurityDir = m_TreeMain.InsertItem(L"Security Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeSecurityDir, LISTID_SECURITY_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_BASERELOC_DIRECTORY_FLAG)) {
			hTreeRelocationDir = m_TreeMain.InsertItem(L"Relocations Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeRelocationDir, LISTID_RELOCATION_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_DEBUG_DIRECTORY_FLAG)) {
			hTreeDebugDir = m_TreeMain.InsertItem(L"Debug Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeDebugDir, LISTID_DEBUG_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_ARCHITECTURE_DIRECTORY_FLAG)) {
			hTreeArchitectureDir = m_TreeMain.InsertItem(L"Architecture Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeArchitectureDir, LISTID_ARCHITECTURE_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_GLOBALPTR_DIRECTORY_FLAG)) {
			hTreeGlobalPTRDir = m_TreeMain.InsertItem(L"GlobalPTR Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeGlobalPTRDir, LISTID_GLOBALPTR_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_TLS_DIRECTORY_FLAG)) {
			hTreeTLSDir = m_TreeMain.InsertItem(L"TLS Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeTLSDir, LISTID_TLS_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_LOADCONFIG_DIRECTORY_FLAG)) {
			hTreeLoadConfigDir = m_TreeMain.InsertItem(L"Load Config Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeLoadConfigDir, LISTID_LOAD_CONFIG_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_BOUNDIMPORT_DIRECTORY_FLAG)) {
			hTreeBoundImportDir = m_TreeMain.InsertItem(L"Bound Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeBoundImportDir, LISTID_BOUND_IMPORT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_IAT_DIRECTORY_FLAG)) {
			hTreeIATDir = m_TreeMain.InsertItem(L"IAT Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeIATDir, LISTID_IAT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_DELAYIMPORT_DIRECTORY_FLAG)) {
			hTreeDelayImportDir = m_TreeMain.InsertItem(L"Delay Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeDelayImportDir, LISTID_DELAY_IMPORT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_COMDESCRIPTOR_DIRECTORY_FLAG)) {
			hTreeCOMDescriptorDir = m_TreeMain.InsertItem(L"COM Descriptor Directory", iconDirs, iconDirs, hTreeRoot);
			m_TreeMain.SetItemData(hTreeCOMDescriptorDir, LISTID_COMDESCRIPTOR_DIR);
		}
	}

	m_TreeMain.Expand(hTreeRoot, TVE_EXPAND);
	m_TreeMain.Expand(hTreeNTHeaders, TVE_EXPAND);
	m_TreeMain.Expand(hTreeOptHeader, TVE_EXPAND);
}

BOOL CViewLeft::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LPNMTREEVIEW pTree = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (pTree->hdr.idFrom == TREEID_LEFT_MAIN && pTree->hdr.code == TVN_SELCHANGED)
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