#include "stdafx.h"
#include "ViewRightBL.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CViewRightBL, CView)

BEGIN_MESSAGE_MAP(CViewRightBL, CView)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CViewRightBL::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();

	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	if (!m_pLibpe)
		return;

	const DWORD* pFileSummary { };
	if (m_pLibpe->GetFileSummary(pFileSummary) != S_OK)
		return;

	m_dwFileSummary = *pFileSummary;

	//Hex control for SecurityDir and TLSdir.
	m_stHexEdit.Create(this, CRect(0, 0, 0, 0), IDC_HEX_RIGHT_BOTTOM_LEFT);
	m_stHexEdit.ShowWindow(SW_HIDE);

	m_stListInfo.clrTooltipText = RGB(255, 255, 255);
	m_stListInfo.clrTooltipBk = RGB(0, 132, 132);
	m_stListInfo.clrHeaderText = RGB(255, 255, 255);
	m_stListInfo.clrHeaderBk = RGB(0, 132, 132);
	m_stListInfo.dwHeaderHeight = 35;

	m_lf.lfHeight = 16;
	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stListInfo.pListLogFont = &m_lf;
	m_hdrlf.lfHeight = 17;
	m_hdrlf.lfWeight = FW_BOLD;
	StringCchCopyW(m_hdrlf.lfFaceName, 16, L"Times New Roman");
	m_stListInfo.pHeaderLogFont = &m_hdrlf;

	CreateListExportFuncs();
	CreateTreeResources();
}

void CViewRightBL::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	if (!m_pChildFrame)
		return;
	if (LOWORD(lHint) == IDC_HEX_RIGHT_TR)
		return;

	if (m_pActiveWnd)
		m_pActiveWnd->ShowWindow(SW_HIDE);

	CRect rect;
	GetClientRect(&rect);

	switch (LOWORD(lHint))
	{
	case IDC_LIST_EXPORT:
		m_listExportFuncs.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_listExportFuncs;
		break;
	case IDC_LIST_IMPORT_ENTRY:
		CreateListImportEntry(HIWORD(lHint));
		m_pActiveWnd = &m_listImportEntry;
		break;
	case IDC_TREE_RESOURCE:
	case IDC_HEX_RIGHT_TR:
		m_treeResBottom.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_treeResBottom;
		break;
	case IDC_LIST_SECURITY_ENTRY:
		CreateHexSecurityEntry(HIWORD(lHint));
		m_pActiveWnd = &m_stHexEdit;
		break;
	case IDC_LIST_RELOCATIONS_ENTRY:
		CreateListRelocsEntry(HIWORD(lHint));
		m_pActiveWnd = &m_listRelocsEntry;
		break;
	case IDC_LIST_TLS:
		CreateHexTLS();
		m_pActiveWnd = &m_stHexEdit;
		break;
	case IDC_LIST_DELAYIMPORT_FUNCS:
		CreateListDelayImportEntry(HIWORD(lHint));
		m_pActiveWnd = &m_listDelayImportEntry;
		break;
	case IDC_LIST_DEBUG_ENTRY:
		CreateHexDebugEntry(HIWORD(lHint));
		m_pActiveWnd = &m_stHexEdit;
		break;
	}
}

void CViewRightBL::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveWnd)
		m_pActiveWnd->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightBL::OnDraw(CDC* pDC)
{
}

BOOL CViewRightBL::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	const LPNMTREEVIEW pTree = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (pTree->hdr.idFrom == IDC_TREE_RESOURCE_BOTTOM && pTree->hdr.code == TVN_SELCHANGED)
	{
		PCLIBPE_RESOURCE_ROOT_TUP pTupResRoot { };

		if (m_pLibpe->GetResourceTable(pTupResRoot) != S_OK)
			return -1;

		const DWORD_PTR dwResId = m_treeResBottom.GetItemData(pTree->itemNew.hItem);
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
						//Resource data and resource type to show in CViewRightBR.
						m_stResHelper.IdResType = std::get<0>(rootvec.at(idlvlRoot)).Id;
						m_stResHelper.IdResName = std::get<0>(lvl2vec.at(idlvl2)).Id;
						m_stResHelper.pData = (std::vector<std::byte>*)&data;
						m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_SHOW_RESOURCE_RBR, 0), reinterpret_cast<CObject*>(&m_stResHelper));
					}
				}
			}
		}
		else
			//Update by default, with no data — to clear view.
			m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_SHOW_RESOURCE_RBR, 0), nullptr);
	}

	return CScrollView::OnNotify(wParam, lParam, pResult);
}

BOOL CViewRightBL::OnEraseBkgnd(CDC* pDC)
{
	return CScrollView::OnEraseBkgnd(pDC);
}

int CViewRightBL::CreateHexSecurityEntry(unsigned nSertId)
{
	if (m_pLibpe->GetSecurityTable(m_vecSec) != S_OK)
		return -1;
	if (nSertId > m_vecSec->size())
		return -1;

	const auto& secEntry = std::get<1>(m_vecSec->at(nSertId));
	m_stHexEdit.SetData((PBYTE)secEntry.data(), secEntry.size());

	CRect rect;
	GetClientRect(&rect);
	m_stHexEdit.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	return 0;
}

int CViewRightBL::CreateListImportEntry(DWORD dwEntry)
{
	PCLIBPE_IMPORT_VEC m_pImportTable { };

	if (m_pLibpe->GetImportTable(m_pImportTable) != S_OK)
		return -1;

	if (dwEntry > m_pImportTable->size())
		return -1;

	m_listImportEntry.DestroyWindow();
	m_listImportEntry.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_IMPORT_ENTRY, &m_stListInfo);

	m_listImportEntry.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listImportEntry.InsertColumn(0, L"Function Name", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 175);
	m_listImportEntry.InsertColumn(1, L"Ordinal / Hint", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listImportEntry.InsertColumn(2, L"ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 150);

	int listindex = 0;
	TCHAR str[MAX_PATH] { };

	m_listImportEntry.SetRedraw(FALSE);
	for (auto& i : std::get<2>(m_pImportTable->at(dwEntry)))
	{
		swprintf_s(str, 256, L"%S", std::get<1>(i).c_str());
		m_listImportEntry.InsertItem(listindex, str);
		swprintf_s(str, 17, L"%04llX", std::get<0>(i));
		m_listImportEntry.SetItemText(listindex, 1, str);
		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(str, 9, L"%08llX", std::get<2>(i));
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(str, 17, L"%016llX", std::get<2>(i));
		m_listImportEntry.SetItemText(listindex, 2, str);

		listindex++;
	}
	m_listImportEntry.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listImportEntry.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateListDelayImportEntry(DWORD dwEntry)
{
	PCLIBPE_DELAYIMPORT_VEC pDelayImport { };

	if (m_pLibpe->GetDelayImportTable(pDelayImport) != S_OK)
		return -1;

	if (dwEntry > pDelayImport->size())
		return -1;

	m_listDelayImportEntry.DestroyWindow();
	m_listDelayImportEntry.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_DELAYIMPORT_FUNCS, &m_stListInfo);

	m_listDelayImportEntry.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDelayImportEntry.InsertColumn(0, L"Function Name", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 300);
	m_listDelayImportEntry.InsertColumn(1, L"Ordinal / Hint", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listDelayImportEntry.InsertColumn(2, L"ImportNameTable ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 200);
	m_listDelayImportEntry.InsertColumn(3, L"ImportAddressTable ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 220);
	m_listDelayImportEntry.InsertColumn(4, L"BoundImportAddressTable ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 250);
	m_listDelayImportEntry.InsertColumn(5, L"UnloadInformationTable ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 240);

	int listindex = 0;
	TCHAR str[MAX_PATH] { };

	m_listDelayImportEntry.SetRedraw(FALSE);
	for (auto&i : std::get<2>(pDelayImport->at(dwEntry)))
	{
		swprintf_s(str, 256, L"%S", std::get<1>(i).c_str());
		m_listDelayImportEntry.InsertItem(listindex, str);
		swprintf_s(str, 17, L"%04llX", std::get<0>(i));
		m_listDelayImportEntry.SetItemText(listindex, 1, str);
		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(str, 9, L"%08llX", std::get<2>(i));
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(str, 17, L"%016llX", std::get<2>(i));
		m_listDelayImportEntry.SetItemText(listindex, 2, str);
		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(str, 9, L"%08llX", std::get<3>(i));
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(str, 17, L"%016llX", std::get<3>(i));
		m_listDelayImportEntry.SetItemText(listindex, 3, str);
		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(str, 9, L"%08llX", std::get<4>(i));
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(str, 17, L"%016llX", std::get<4>(i));
		m_listDelayImportEntry.SetItemText(listindex, 4, str);
		if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE32))
			swprintf_s(str, 9, L"%08llX", std::get<5>(i));
		else if (ImageHasFlag(m_dwFileSummary, IMAGE_FLAG_PE64))
			swprintf_s(str, 17, L"%016llX", std::get<5>(i));
		m_listDelayImportEntry.SetItemText(listindex, 5, str);

		listindex++;
	}
	m_listDelayImportEntry.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listDelayImportEntry.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateListExportFuncs()
{
	PCLIBPE_EXPORT_TUP pExportTable { };

	if (m_pLibpe->GetExportTable(pExportTable) != S_OK)
		return -1;

	m_listExportFuncs.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_EXPORT_FUNCS, &m_stListInfo);
	m_listExportFuncs.ShowWindow(SW_HIDE);
	m_listExportFuncs.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listExportFuncs.InsertColumn(0, L"Function RVA", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 100);
	m_listExportFuncs.InsertColumn(1, L"Ordinal", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listExportFuncs.InsertColumn(2, L"Name", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 250);
	m_listExportFuncs.InsertColumn(3, L"Forwarder Name", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 400);

	m_listExportFuncs.SetRedraw(FALSE); //to increase the speed of List populating
	int listindex = 0;
	WCHAR str[MAX_PATH] { };

	for (auto& i : std::get<2>(*pExportTable))
	{
		swprintf_s(str, 9, L"%08X", std::get<0>(i));
		m_listExportFuncs.InsertItem(listindex, str);
		swprintf_s(str, 9, L"%u", std::get<1>(i));
		m_listExportFuncs.SetItemText(listindex, 1, str);
		swprintf_s(str, MAX_PATH, L"%S", std::get<2>(i).c_str());
		m_listExportFuncs.SetItemText(listindex, 2, str);
		swprintf_s(str, MAX_PATH, L"%S", std::get<3>(i).c_str());
		m_listExportFuncs.SetItemText(listindex, 3, str);

		listindex++;
	}
	m_listExportFuncs.SetRedraw(TRUE);

	return 0;
}

int CViewRightBL::CreateListRelocsEntry(DWORD dwEntry)
{
	PCLIBPE_RELOCATION_VEC pRelocTable { };

	if (m_pLibpe->GetRelocationTable(pRelocTable) != S_OK)
		return -1;
	if (pRelocTable->empty() || pRelocTable->size() < dwEntry)
		return -1;

	m_listRelocsEntry.DestroyWindow();

	m_listRelocsEntry.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_RELOCATIONS_ENTRY, &m_stListInfo);
	m_listRelocsEntry.ShowWindow(SW_HIDE);
	m_listRelocsEntry.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listRelocsEntry.InsertColumn(0, L"Reloc type", LVCFMT_CENTER, 250);
	m_listRelocsEntry.InsertColumn(1, L"Offset", LVCFMT_LEFT, 100);

	const std::map<WORD, std::wstring> mapRelocTypes {
		{ IMAGE_REL_BASED_ABSOLUTE, L"IMAGE_REL_BASED_ABSOLUTE" },
	{ IMAGE_REL_BASED_HIGH, L"IMAGE_REL_BASED_HIGH" },
	{ IMAGE_REL_BASED_LOW, L"IMAGE_REL_BASED_LOW" },
	{ IMAGE_REL_BASED_HIGHLOW, L"IMAGE_REL_BASED_HIGHLOW" },
	{ IMAGE_REL_BASED_HIGHADJ, L"IMAGE_REL_BASED_HIGHADJ" },
	{ IMAGE_REL_BASED_MACHINE_SPECIFIC_5, L"IMAGE_REL_BASED_MACHINE_SPECIFIC_5" },
	{ IMAGE_REL_BASED_RESERVED, L"IMAGE_REL_BASED_RESERVED" },
	{ IMAGE_REL_BASED_MACHINE_SPECIFIC_7, L"IMAGE_REL_BASED_MACHINE_SPECIFIC_7" },
	{ IMAGE_REL_BASED_MACHINE_SPECIFIC_8, L"IMAGE_REL_BASED_MACHINE_SPECIFIC_8" },
	{ IMAGE_REL_BASED_MACHINE_SPECIFIC_9, L"IMAGE_REL_BASED_MACHINE_SPECIFIC_9" },
	{ IMAGE_REL_BASED_DIR64, L"IMAGE_REL_BASED_DIR64" }
	};

	int listindex = 0;
	WCHAR str[MAX_PATH] { };

	m_listRelocsEntry.SetRedraw(FALSE);
	for (auto& iterRelocs : std::get<1>(pRelocTable->at(dwEntry)))
	{
		WORD wRelocType = std::get<0>(iterRelocs);
		auto it = mapRelocTypes.find(wRelocType);
		if (it != mapRelocTypes.end())
			swprintf_s(str, MAX_PATH, L"%s", it->second.data());
		else
			swprintf_s(str, MAX_PATH, L"%u", wRelocType);

		m_listRelocsEntry.InsertItem(listindex, str);
		swprintf_s(str, 5, L"%04X", std::get<1>(iterRelocs));
		m_listRelocsEntry.SetItemText(listindex, 1, str);

		listindex++;
	}
	m_listRelocsEntry.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listRelocsEntry.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateHexDebugEntry(DWORD dwEntry)
{
	PCLIBPE_DEBUG_VEC pDebug;
	if (m_pLibpe->GetDebugTable(pDebug) != S_OK)
		return -1;

	const auto& debugEntry = std::get<1>(pDebug->at(dwEntry));
	m_stHexEdit.SetData((PBYTE)debugEntry.data(), debugEntry.size());

	CRect rect;
	GetClientRect(&rect);
	m_stHexEdit.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBL::CreateTreeResources()
{
	PCLIBPE_RESOURCE_ROOT_TUP pTupResRoot { };

	if (m_pLibpe->GetResourceTable(pTupResRoot) != S_OK)
		return -1;

	m_treeResBottom.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		TVS_LINESATROOT, CRect(0, 0, 0, 0), this, IDC_TREE_RESOURCE_BOTTOM);
	m_treeResBottom.ShowWindow(SW_HIDE);

	WCHAR str[MAX_PATH] { };

	m_imglTreeRes.Create(16, 16, ILC_COLOR32, 0, 4);
	const int iconDirs = m_imglTreeRes.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_DIR_ICON));
	m_treeResBottom.SetImageList(&m_imglTreeRes, TVSIL_NORMAL);
	long ilvlRoot = 0, ilvl2 = 0, ilvl3 = 0;

	//Creating a treeCtrl and setting, with SetItemData(),
	//a unique id for each node, that is an index in vector (m_vecResId),
	//that holds tuple of three IDs of resource — Type, Name, LangID.
	for (auto& iterRoot : std::get<1>(*pTupResRoot))
	{
		const IMAGE_RESOURCE_DIRECTORY_ENTRY* pResDirEntry = &std::get<0>(iterRoot);
		if (pResDirEntry->NameIsString)
			//Enclose in double quotes.
			swprintf(str, MAX_PATH, L"\u00AB%s\u00BB", std::get<1>(iterRoot).c_str());
		else
		{	//Setting Treectrl root node name depending on Resource typeID.
			if (g_mapResType.find(pResDirEntry->Id) != g_mapResType.end())
				swprintf(str, MAX_PATH, L"%s [Id: %u]", g_mapResType.at(pResDirEntry->Id).data(), pResDirEntry->Id);
			else
				swprintf(str, MAX_PATH, L"%u", pResDirEntry->Id);
		}

		const HTREEITEM treeRoot = m_treeResBottom.InsertItem(str, iconDirs, iconDirs);
		m_vecResId.emplace_back(ilvlRoot, -1, -1);
		m_treeResBottom.SetItemData(treeRoot, m_vecResId.size() - 1);
		ilvl2 = 0;
		LIBPE_RESOURCE_LVL2_TUP pTupResLvL2 = std::get<4>(iterRoot);

		for (auto& iterLvL2 : std::get<1>(pTupResLvL2))
		{
			m_vecResId.emplace_back(ilvlRoot, ilvl2, -1);
			ilvl3 = 0;

			//			pResDirEntry = &std::get<0>(iterLvL2);
			LIBPE_RESOURCE_LVL3_TUP pTupResLvL3 = std::get<4>(iterLvL2);

			for (auto& iterLvL3 : std::get<1>(pTupResLvL3))
			{
				pResDirEntry = &std::get<0>(iterLvL3);
				if (pResDirEntry->NameIsString)
					swprintf(str, MAX_PATH, L"«%s» - lang: %i", std::get<1>(iterLvL2).c_str(), pResDirEntry->Id);
				else
					swprintf(str, MAX_PATH, L"%u - lang: %i", std::get<0>(iterLvL2).Id, pResDirEntry->Id);

				m_vecResId.emplace_back(ilvlRoot, ilvl2, ilvl3);
				m_treeResBottom.SetItemData(m_treeResBottom.InsertItem(str, treeRoot), m_vecResId.size() - 1);
				ilvl3++;
			}
			ilvl2++;
		}
		ilvlRoot++;
	}

	return 0;
}

int CViewRightBL::CreateHexTLS()
{
	PCLIBPE_TLS_TUP pTLS;
	if (m_pLibpe->GetTLSTable(pTLS) != S_OK)
		return -1;

	const auto& tls = std::get<1>(*pTLS);
	m_stHexEdit.SetData((PBYTE)tls.data(), tls.size());

	CRect rect;
	GetClientRect(&rect);
	m_stHexEdit.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}