/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "ViewLeft.h"
#include "Utility.h"
#include "res/resource.h"

IMPLEMENT_DYNCREATE(CViewLeft, CView)

BEGIN_MESSAGE_MAP(CViewLeft, CView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CViewLeft::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_pMainDoc = static_cast<CPepperDoc*>(GetDocument());
	m_pLibpe = m_pMainDoc->m_pLibpe.get();
	const auto stFileInfo = m_pLibpe->GetFileInfo();

	//Scaling factor for HighDPI displays.
	auto pDC = GetDC();
	const auto fScale = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY) / 96.0f;
	ReleaseDC(pDC);
	const auto iImgSize = static_cast<int>(16 * fScale);

	m_ImgListRootTree.Create(iImgSize, iImgSize, ILC_COLOR32, 0, 2);
	const int iconHdr = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_PEHEADER));
	const int iconDirs = m_ImgListRootTree.Add(AfxGetApp()->LoadIconW(IDI_TREE_PEDIRS));

	CRect rect;
	GetClientRect(&rect);
	m_stTreeMain.Create(TVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | TVS_HASLINES | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		rect, this, IDC_TREE_LEFT_MAIN);

	m_stTreeMain.SetImageList(&m_ImgListRootTree, TVSIL_NORMAL);

	const auto hTreeRoot = m_stTreeMain.InsertItem(L"FILE SUMMARY");
	m_stTreeMain.SetItemState(hTreeRoot, TVIS_BOLD, TVIS_BOLD);
	m_stTreeMain.SetItemData(hTreeRoot, IDC_SHOW_FILE_SUMMARY);

	if (stFileInfo.fHasDosHdr)
	{
		const auto hTreeDosHeader = m_stTreeMain.InsertItem(L"MS-DOS Header [IMAGE_DOS_HEADER]", iconHdr, iconHdr, hTreeRoot);
		m_stTreeMain.SetItemData(hTreeDosHeader, IDC_LIST_DOSHEADER);
	}
	if (stFileInfo.fHasRichHdr)
	{
		const auto hTreeDosRich = m_stTreeMain.InsertItem(L"\u00ABRich\u00BB Header", iconHdr, iconHdr, hTreeRoot);
		m_stTreeMain.SetItemData(hTreeDosRich, IDC_LIST_RICHHEADER);
	}

	HTREEITEM hTreeNTHeaders { };
	HTREEITEM hTreeOptHeader { };

	if (stFileInfo.fIsx86 && stFileInfo.fHasNTHdr)
		hTreeNTHeaders = m_stTreeMain.InsertItem(L"NT Header [IMAGE_NT_HEADERS32]", iconHdr, iconHdr, hTreeRoot);
	else if (stFileInfo.fIsx64 && stFileInfo.fHasNTHdr)
		hTreeNTHeaders = m_stTreeMain.InsertItem(L"NT Header [IMAGE_NT_HEADERS64]", iconHdr, iconHdr, hTreeRoot);

	if (hTreeNTHeaders)
	{
		m_stTreeMain.SetItemData(hTreeNTHeaders, IDC_LIST_NTHEADER);

		if (stFileInfo.fHasNTHdr)
		{
			const auto hTreeFileHeader = m_stTreeMain.InsertItem(L"File Header [IMAGE_FILE_HEADER]", iconHdr, iconHdr,
				hTreeNTHeaders);
			m_stTreeMain.SetItemData(hTreeFileHeader, IDC_LIST_FILEHEADER);
		}

		if (stFileInfo.fIsx86 && stFileInfo.fHasNTHdr)
			hTreeOptHeader = m_stTreeMain.InsertItem(L"Optional Header [IMAGE_OPTIONAL_HEADER32]", iconHdr, iconHdr, hTreeNTHeaders);
		else if (stFileInfo.fIsx64 && stFileInfo.fHasNTHdr)
			hTreeOptHeader = m_stTreeMain.InsertItem(L"Optional Header [IMAGE_OPTIONAL_HEADER64]", iconHdr, iconHdr, hTreeNTHeaders);

		m_stTreeMain.SetItemData(hTreeOptHeader, IDC_LIST_OPTIONALHEADER);
		m_stTreeMain.Expand(hTreeNTHeaders, TVE_EXPAND);
	}

	if (stFileInfo.fHasSections)
	{
		const auto hTreeSecHeaders = m_stTreeMain.InsertItem(L"Sections Headers [IMAGE_SECTION_HEADER]", iconHdr, iconHdr, hTreeRoot);
		m_stTreeMain.SetItemData(hTreeSecHeaders, IDC_LIST_SECHEADERS);
	}

	if (const auto vecDataDirs = m_pLibpe->GetDataDirs(); vecDataDirs != nullptr)
	{
		if (hTreeOptHeader)
		{
			const auto hTreeDataDirs = m_stTreeMain.InsertItem(L"Data Directories [IMAGE_DATA_DIRECTORY]", iconHdr, iconHdr, hTreeOptHeader);
			m_stTreeMain.SetItemData(hTreeDataDirs, IDC_LIST_DATADIRECTORIES);
			m_stTreeMain.Expand(hTreeOptHeader, TVE_EXPAND);
		}
		if (stFileInfo.fHasExport) {
			const auto hTreeExportDir = m_stTreeMain.InsertItem(L"Export Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeExportDir, IDC_LIST_EXPORT);
		}
		if (stFileInfo.fHasImport) {
			const auto hTreeImportDir = m_stTreeMain.InsertItem(L"Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeImportDir, IDC_LIST_IMPORT);
		}
		if (stFileInfo.fHasResource) {
			const auto hTreeResourceDir = m_stTreeMain.InsertItem(L"Resource Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeResourceDir, IDC_TREE_RESOURCE);
		}
		if (stFileInfo.fHasException) {
			const auto hTreeExceptionDir = m_stTreeMain.InsertItem(L"Exception Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeExceptionDir, IDC_LIST_EXCEPTIONS);
		}
		if (stFileInfo.fHasSecurity) {
			const auto hTreeSecurityDir = m_stTreeMain.InsertItem(L"Security Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeSecurityDir, IDC_LIST_SECURITY);
		}
		if (stFileInfo.fHasReloc) {
			const auto hTreeRelocationDir = m_stTreeMain.InsertItem(L"Relocations Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeRelocationDir, IDC_LIST_RELOCATIONS);
		}
		if (stFileInfo.fHasDebug) {
			const auto hTreeDebugDir = m_stTreeMain.InsertItem(L"Debug Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeDebugDir, IDC_LIST_DEBUG);
		}
		if (stFileInfo.fHasArchitect) {
			const auto hTreeArchitectureDir = m_stTreeMain.InsertItem(L"Architecture Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeArchitectureDir, IDC_LIST_ARCHITECTURE);
		}
		if (stFileInfo.fHasGlobalPtr) {
			const auto hTreeGlobalPTRDir = m_stTreeMain.InsertItem(L"GlobalPTR Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeGlobalPTRDir, IDC_LIST_GLOBALPTR);
		}
		if (stFileInfo.fHasTLS) {
			const auto hTreeTLSDir = m_stTreeMain.InsertItem(L"TLS Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeTLSDir, IDC_LIST_TLS);
		}
		if (stFileInfo.fHasLoadCFG) {
			const auto hTreeLoadConfigDir = m_stTreeMain.InsertItem(L"Load Config Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeLoadConfigDir, IDC_LIST_LOADCONFIG);
		}
		if (stFileInfo.fHasBoundImp) {
			const auto hTreeBoundImportDir = m_stTreeMain.InsertItem(L"Bound Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeBoundImportDir, IDC_LIST_BOUNDIMPORT);
		}
		if (stFileInfo.fHasIAT) {
			const auto hTreeIATDir = m_stTreeMain.InsertItem(L"IAT Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeIATDir, IDC_LIST_IAT);
		}
		if (stFileInfo.fHasDelayImp) {
			const auto hTreeDelayImportDir = m_stTreeMain.InsertItem(L"Delay Import Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeDelayImportDir, IDC_LIST_DELAYIMPORT);
		}
		if (stFileInfo.fHasCOMDescr) {
			const auto hTreeCOMDescriptorDir = m_stTreeMain.InsertItem(L"COM Descriptor Directory", iconDirs, iconDirs, hTreeRoot);
			m_stTreeMain.SetItemData(hTreeCOMDescriptorDir, IDC_LIST_COMDESCRIPTOR);
		}
	}
	m_stTreeMain.Expand(hTreeRoot, TVE_EXPAND);

	m_fCreated = true;
}

BOOL CViewLeft::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	const auto pTree = reinterpret_cast<LPNMTREEVIEWW>(lParam);
	if (pTree->hdr.idFrom == IDC_TREE_LEFT_MAIN && pTree->hdr.code == TVN_SELCHANGED)
		m_pMainDoc->UpdateAllViews(this, m_stTreeMain.GetItemData(pTree->itemNew.hItem));

	return CView::OnNotify(wParam, lParam, pResult);
}

void CViewLeft::OnDraw(CDC* /*pDC*/)
{
}

BOOL CViewLeft::OnEraseBkgnd(CDC* /*pDC*/)
{
	return FALSE;
}

void CViewLeft::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_fCreated)
		m_stTreeMain.SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}