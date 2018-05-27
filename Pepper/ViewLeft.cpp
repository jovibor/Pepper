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

	HTREEITEM _hTreeRoot { };
	HTREEITEM _hTreeDosHeader { };
	HTREEITEM _hTreeDosRich { };
	HTREEITEM _hTreeNTHeaders { };
	HTREEITEM _hTreeFileHeader { };
	HTREEITEM _hTreeOptHeader { };
	HTREEITEM _hTreeDataDirs { };
	HTREEITEM _hTreeSecHeaders { };
	HTREEITEM _hTreeExportDir { };
	HTREEITEM _hTreeImportDir { };
	HTREEITEM _hTreeResourceDir { };
	HTREEITEM _hTreeExceptionDir { };
	HTREEITEM _hTreeSecurityDir { };
	HTREEITEM _hTreeRelocationDir { };
	HTREEITEM _hTreeDebugDir { };
	HTREEITEM _hTreeArchitectureDir { };
	HTREEITEM _hTreeGlobalPTRDir { };
	HTREEITEM _hTreeTLSDir { };
	HTREEITEM _hTreeLoadConfigDir { };
	HTREEITEM _hTreeBoundImportDir { };
	HTREEITEM _hTreeIATDir { };
	HTREEITEM _hTreeDelayImportDir { };
	HTREEITEM _hTreeCOMDescriptorDir { };

	m_pMainDoc = (CPepperDoc*)GetDocument();
	if (!m_pMainDoc)
		return;

	m_pLibpe = m_pMainDoc->m_pLibpe;
	if (!m_pLibpe)
		return;

	const DWORD* m_pFileSummary { };
	if (m_pLibpe->GetFileSummary(&m_pFileSummary) != S_OK)
		return;
	
	DWORD m_dwFileSummary = *m_pFileSummary;

	m_ImgListRootTree.Create(16, 16, ILC_COLOR32, 0, 4);
	int _iconHdr = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_HEADER_ICON));
	int _iconDirs = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_DIR_ICON));

	CRect _rect;
	GetClientRect(&_rect);
	m_TreeMain.Create(TVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | TVS_HASLINES | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		_rect, this, TREEID_LEFT_MAIN);

	m_TreeMain.SetImageList(&m_ImgListRootTree, TVSIL_NORMAL);

	_hTreeRoot = m_TreeMain.InsertItem(_T("FILE SUMMARY"));
	m_TreeMain.SetItemState(_hTreeRoot, TVIS_BOLD, TVIS_BOLD);
	m_TreeMain.SetItemData(_hTreeRoot, LISTID_FILE_SUMMARY);

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_DOS_HEADER_FLAG))
	{
		_hTreeDosHeader = m_TreeMain.InsertItem(TEXT("MS-DOS Header [IMAGE_DOS_HEADER]"), _iconHdr, _iconHdr, _hTreeRoot);
		m_TreeMain.SetItemData(_hTreeDosHeader, LISTID_DOS_HEADER);
	}
	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_RICH_HEADER_FLAG))
	{
		_hTreeDosRich = m_TreeMain.InsertItem(L"\u00ABRich\u00BB Header", _iconHdr, _iconHdr, _hTreeRoot);
		m_TreeMain.SetItemData(_hTreeDosRich, LISTID_DOS_RICH);
	}

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
		_hTreeNTHeaders = m_TreeMain.InsertItem(_T("NT Header [IMAGE_NT_HEADERS32]"), _iconHdr, _iconHdr, _hTreeRoot);
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
		_hTreeNTHeaders = m_TreeMain.InsertItem(_T("NT Header [IMAGE_NT_HEADERS64]"), _iconHdr, _iconHdr, _hTreeRoot);

	m_TreeMain.SetItemData(_hTreeNTHeaders, LISTID_NT_HEADER);

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_FILE_HEADER_FLAG))
	{
		_hTreeFileHeader = m_TreeMain.InsertItem(TEXT("File Header [IMAGE_FILE_HEADER]"), _iconHdr, _iconHdr, _hTreeNTHeaders);
		m_TreeMain.SetItemData(_hTreeFileHeader, LISTID_FILE_HEADER);
	}

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG | IMAGE_OPTIONAL_HEADER_FLAG))
		_hTreeOptHeader = m_TreeMain.InsertItem(TEXT("Optional Header [IMAGE_OPTIONAL_HEADER32]"), _iconHdr, _iconHdr, _hTreeNTHeaders);
	else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG | IMAGE_OPTIONAL_HEADER_FLAG))
		_hTreeOptHeader = m_TreeMain.InsertItem(TEXT("Optional Header [IMAGE_OPTIONAL_HEADER64]"), _iconHdr, _iconHdr, _hTreeNTHeaders);

	m_TreeMain.SetItemData(_hTreeOptHeader, LISTID_OPTIONAL_HEADER);

	if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_SECTION_HEADERS_FLAG))
	{
		_hTreeSecHeaders = m_TreeMain.InsertItem(TEXT("Section Headers [IMAGE_SECTION_HEADER]"), _iconHdr, _iconHdr, _hTreeRoot);
		m_TreeMain.SetItemData(_hTreeSecHeaders, LISTID_SECHEADERS);
	}

	PLIBPE_DATADIRS _vecDataDirs { };
	if (m_pLibpe->GetDataDirectories(&_vecDataDirs) == S_OK)
	{
		_hTreeDataDirs = m_TreeMain.InsertItem(TEXT("Data Directories [IMAGE_DATA_DIRECTORY]"), _iconHdr, _iconHdr, _hTreeOptHeader);
		m_TreeMain.SetItemData(_hTreeDataDirs, LISTID_DATA_DIRS);

		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_EXPORT_DIRECTORY_FLAG)) {
			_hTreeExportDir = m_TreeMain.InsertItem(TEXT("Export Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeExportDir, LISTID_EXPORT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_IMPORT_DIRECTORY_FLAG)) {
			_hTreeImportDir = m_TreeMain.InsertItem(TEXT("Import Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeImportDir, LISTID_IMPORT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_RESOURCE_DIRECTORY_FLAG)) {
			_hTreeResourceDir = m_TreeMain.InsertItem(TEXT("Resource Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeResourceDir, LISTID_RESOURCE_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_EXCEPTION_DIRECTORY_FLAG)) {
			_hTreeExceptionDir = m_TreeMain.InsertItem(TEXT("Exception Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeExceptionDir, LISTID_EXCEPTION_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_SECURITY_DIRECTORY_FLAG)) {
			_hTreeSecurityDir = m_TreeMain.InsertItem(TEXT("Security Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeSecurityDir, LISTID_SECURITY_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_BASERELOC_DIRECTORY_FLAG)) {
			_hTreeRelocationDir = m_TreeMain.InsertItem(TEXT("Relocations Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeRelocationDir, LISTID_RELOCATION_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_DEBUG_DIRECTORY_FLAG)) {
			_hTreeDebugDir = m_TreeMain.InsertItem(TEXT("Debug Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeDebugDir, LISTID_DEBUG_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_ARCHITECTURE_DIRECTORY_FLAG)) {
			_hTreeArchitectureDir = m_TreeMain.InsertItem(TEXT("Architecture Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeArchitectureDir, LISTID_ARCHITECTURE_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_GLOBALPTR_DIRECTORY_FLAG)) {
			_hTreeGlobalPTRDir = m_TreeMain.InsertItem(TEXT("GlobalPTR Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeGlobalPTRDir, LISTID_GLOBALPTR_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_TLS_DIRECTORY_FLAG)) {
			_hTreeTLSDir = m_TreeMain.InsertItem(TEXT("TLS Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeTLSDir, LISTID_TLS_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_LOADCONFIG_DIRECTORY_FLAG)) {
			_hTreeLoadConfigDir = m_TreeMain.InsertItem(TEXT("Load Config Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeLoadConfigDir, LISTID_LOAD_CONFIG_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_BOUNDIMPORT_DIRECTORY_FLAG)) {
			_hTreeBoundImportDir = m_TreeMain.InsertItem(TEXT("Bound Import Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeBoundImportDir, LISTID_BOUND_IMPORT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_IAT_DIRECTORY_FLAG)) {
			_hTreeIATDir = m_TreeMain.InsertItem(TEXT("IAT Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeIATDir, LISTID_IAT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_DELAYIMPORT_DIRECTORY_FLAG)) {
			_hTreeDelayImportDir = m_TreeMain.InsertItem(TEXT("Delay Import Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeDelayImportDir, LISTID_DELAY_IMPORT_DIR);
		}
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_COMDESCRIPTOR_DIRECTORY_FLAG)) {
			_hTreeCOMDescriptorDir = m_TreeMain.InsertItem(TEXT("COM Descriptor Directory"), _iconDirs, _iconDirs, _hTreeRoot);
			m_TreeMain.SetItemData(_hTreeCOMDescriptorDir, LISTID_COMDESCRIPTOR_DIR);
		}
	}

	m_TreeMain.Expand(_hTreeRoot, TVE_EXPAND);
	m_TreeMain.Expand(_hTreeNTHeaders, TVE_EXPAND);
	m_TreeMain.Expand(_hTreeOptHeader, TVE_EXPAND);
}

BOOL CViewLeft::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LPNMTREEVIEW _tree = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (_tree->hdr.idFrom == TREEID_LEFT_MAIN && _tree->hdr.code == TVN_SELCHANGED)
		m_pMainDoc->UpdateAllViews(this, m_TreeMain.GetItemData(_tree->itemNew.hItem));

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