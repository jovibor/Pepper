#include "stdafx.h"
#include "ViewRightBottomLeft.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CViewRightBottomLeft, CView)

BEGIN_MESSAGE_MAP(CViewRightBottomLeft, CView)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CViewRightBottomLeft::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();

	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	if (!m_pLibpe)
		return;

	const DWORD* m_pFileSummary { };
	if (m_pLibpe->GetFileSummary(&m_pFileSummary) != S_OK)
		return;

	m_dwFileSummary = *m_pFileSummary;

	//Hex control for SecurityDir and TLSdir.
	m_stHexEdit.Create(this, CRect(0, 0, 0, 0), IDC_HEX_RIGHT_BOTTOM_LEFT);
	m_stHexEdit.ShowWindow(SW_HIDE);

	m_stListInfo.clrListTooltipText = RGB(255, 255, 255);
	m_stListInfo.clrListTooltipBk = RGB(0, 132, 132);
	m_stListInfo.clrHeaderText = RGB(255, 255, 255);
	m_stListInfo.clrHeaderBk = RGB(0, 132, 132);
	m_stListInfo.dwHeaderHeight = 35;

	CreateListExportFuncs();
	CreateTreeResources();
}

void CViewRightBottomLeft::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	if (!m_pChildFrame)
		return;

	if (m_pActiveList)
		m_pActiveList->ShowWindow(SW_HIDE);

	CRect rect;
	GetClientRect(&rect);

	switch (LOWORD(lHint))
	{
	case IDC_LIST_EXPORT:
		m_listExportFuncs.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listExportFuncs;
		break;
	case IDC_LIST_IMPORT_ENTRY:
		CreateListImportEntry(HIWORD(lHint));
		m_pActiveList = &m_listImportFuncs;
		break;
	case IDC_TREE_RESOURCE:
	case IDC_HEX_RIGHT_TOP_RIGHT:
		m_treeResBottom.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_treeResBottom;
		break;
	case IDC_LIST_SECURITY_ENTRY:
		CreateHexSecurityEntry(HIWORD(lHint));
		m_pActiveList = &m_stHexEdit;
		break;
	case IDC_LIST_RELOCATIONS_ENTRY:
		CreateListRelocsEntry(HIWORD(lHint));
		m_pActiveList = &m_listRelocsDesc;
		break;
	case IDC_LIST_TLS:
		CreateHexTLS();
		m_pActiveList = &m_stHexEdit;
		break;
	case IDC_LIST_DELAYIMPORT_FUNCS:
		CreateListDelayImportEntry(HIWORD(lHint));
		m_pActiveList = &m_listDelayImportFuncs;
		break;
	case IDC_LIST_DEBUG_ENTRY:
		CreateHexDebugEntry(HIWORD(lHint));
		m_pActiveList = &m_stHexEdit;
		break;
	}
}

HBRUSH CViewRightBottomLeft::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CScrollView::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

void CViewRightBottomLeft::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveList)
		m_pActiveList->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightBottomLeft::OnDraw(CDC* pDC)
{
}

BOOL CViewRightBottomLeft::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	const LPNMTREEVIEW pTree = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (pTree->hdr.idFrom == IDC_TREE_RESOURCE_BOTTOM && pTree->hdr.code == TVN_SELCHANGED)
	{
		PCLIBPE_RESOURCE_ROOT_TUP pTupResRoot { };

		if (m_pLibpe->GetResourceTable(&pTupResRoot) != S_OK)
			return -1;

		const DWORD_PTR dwResId = m_treeResBottom.GetItemData(pTree->itemNew.hItem);
		const long idlvlRoot = std::get<0>(m_vecResId.at(dwResId));
		const long idlvl2 = std::get<1>(m_vecResId.at(dwResId));
		const long idlvl3 = std::get<2>(m_vecResId.at(dwResId));

		auto rootvec = std::get<1>(*pTupResRoot);
		if (idlvl2 >= 0)
		{
			auto lvl2tup = std::get<4>(rootvec.at(idlvlRoot));
			auto lvl2vec = std::get<1>(lvl2tup);

			if (!lvl2vec.empty())
			{
				if (idlvl3 >= 0)
				{
					auto lvl3tup = std::get<4>(lvl2vec.at(idlvl2));
					auto lvl3vec = std::get<1>(lvl3tup);

					if (!lvl3vec.empty())
					{
						auto data = std::get<3>(lvl3vec.at(idlvl3));

						if (!data.empty()) { }
					}
				}
			}
		}
	}

	return CScrollView::OnNotify(wParam, lParam, pResult);
}

BOOL CViewRightBottomLeft::OnEraseBkgnd(CDC* pDC)
{
	return CScrollView::OnEraseBkgnd(pDC);
}

int CViewRightBottomLeft::CreateHexSecurityEntry(unsigned nSertId)
{
	if (m_pLibpe->GetSecurityTable(&m_vecSec) != S_OK)
		return -1;
	if (nSertId > m_vecSec->size())
		return -1;

	m_stHexEdit.SetData(&std::get<1>(m_vecSec->at(nSertId)));

	CRect rect;
	GetClientRect(&rect);
	m_stHexEdit.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBottomLeft::CreateListImportEntry(DWORD dwEntry)
{
	PCLIBPE_IMPORT_VEC m_pImportTable { };

	if (m_pLibpe->GetImportTable(&m_pImportTable) != S_OK)
		return -1;

	if (dwEntry > m_pImportTable->size())
		return -1;

	m_listImportFuncs.DestroyWindow();
	m_listImportFuncs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT, CRect(0, 0, 0, 0), this, IDC_LIST_IMPORT_ENTRY, &m_stListInfo);

	m_listImportFuncs.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listImportFuncs.InsertColumn(0, L"Function Name", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 175);
	m_listImportFuncs.InsertColumn(1, L"Ordinal / Hint", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listImportFuncs.InsertColumn(2, L"ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 150);

	int listindex = 0;
	TCHAR str[MAX_PATH] { };

	m_listImportFuncs.SetRedraw(FALSE);
	for (auto& i : std::get<2>(m_pImportTable->at(dwEntry)))
	{
		swprintf_s(str, 256, L"%S", std::get<1>(i).c_str());
		m_listImportFuncs.InsertItem(listindex, str);
		swprintf_s(str, 17, L"%04llX", std::get<0>(i));
		m_listImportFuncs.SetItemText(listindex, 1, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<2>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<2>(i));
		m_listImportFuncs.SetItemText(listindex, 2, str);

		listindex++;
	}
	m_listImportFuncs.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listImportFuncs.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBottomLeft::CreateListDelayImportEntry(DWORD dwEntry)
{
	PCLIBPE_DELAYIMPORT_VEC pDelayImport { };

	if (m_pLibpe->GetDelayImportTable(&pDelayImport) != S_OK)
		return -1;

	if (dwEntry > pDelayImport->size())
		return -1;

	m_listDelayImportFuncs.DestroyWindow();
	m_listDelayImportFuncs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT,		CRect(0, 0, 0, 0), this, IDC_LIST_DELAYIMPORT_FUNCS, &m_stListInfo);

	m_listDelayImportFuncs.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDelayImportFuncs.InsertColumn(0, L"Function Name", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 300);
	m_listDelayImportFuncs.InsertColumn(1, L"Ordinal / Hint", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listDelayImportFuncs.InsertColumn(2, L"ImportNameTable ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 200);
	m_listDelayImportFuncs.InsertColumn(3, L"ImportAddressTable ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 220);
	m_listDelayImportFuncs.InsertColumn(4, L"BoundImportAddressTable ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 250);
	m_listDelayImportFuncs.InsertColumn(5, L"UnloadInformationTable ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 240);

	int listindex = 0;
	TCHAR str[MAX_PATH] { };

	m_listDelayImportFuncs.SetRedraw(FALSE);
	for (auto&i : std::get<2>(pDelayImport->at(dwEntry)))
	{
		swprintf_s(str, 256, L"%S", std::get<1>(i).c_str());
		m_listDelayImportFuncs.InsertItem(listindex, str);
		swprintf_s(str, 17, L"%04llX", std::get<0>(i));
		m_listDelayImportFuncs.SetItemText(listindex, 1, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<2>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<2>(i));
		m_listDelayImportFuncs.SetItemText(listindex, 2, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<3>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<3>(i));
		m_listDelayImportFuncs.SetItemText(listindex, 3, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<4>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<4>(i));
		m_listDelayImportFuncs.SetItemText(listindex, 4, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<5>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<5>(i));
		m_listDelayImportFuncs.SetItemText(listindex, 5, str);

		listindex++;
	}
	m_listDelayImportFuncs.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listDelayImportFuncs.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBottomLeft::CreateListExportFuncs()
{
	PCLIBPE_EXPORT_TUP pExportTable { };

	if (m_pLibpe->GetExportTable(&pExportTable) != S_OK)
		return -1;

	m_listExportFuncs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT,		CRect(0, 0, 0, 0), this, IDC_LIST_EXPORT_FUNCS, &m_stListInfo);
	m_listExportFuncs.ShowWindow(SW_HIDE);
	m_listExportFuncs.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listExportFuncs.InsertColumn(0, L"Function RVA", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 100);
	m_listExportFuncs.InsertColumn(1, L"Ordinal", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listExportFuncs.InsertColumn(2, L"Name", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 250);
	m_listExportFuncs.InsertColumn(3, L"Forwarder Name", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 400);

	m_listExportFuncs.SetRedraw(FALSE); //to increase the speed of List populating
	int listindex = 0;
	TCHAR str[MAX_PATH] { };

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

int CViewRightBottomLeft::CreateListRelocsEntry(DWORD dwEntry)
{
	PCLIBPE_RELOCATION_VEC pRelocTable { };

	if (m_pLibpe->GetRelocationTable(&pRelocTable) != S_OK)
		return -1;
	if (pRelocTable->empty())
		return -1;
	if (pRelocTable->size() < dwEntry)
		return -1;
	if (std::get<1>(pRelocTable->at(dwEntry)).empty())
		return -1;

	m_listRelocsDesc.DestroyWindow();

	m_listRelocsDesc.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT,		CRect(0, 0, 0, 0), this, IDC_LIST_RELOCATIONS_ENTRY, &m_stListInfo);
	m_listRelocsDesc.ShowWindow(SW_HIDE);
	m_listRelocsDesc.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listRelocsDesc.InsertColumn(0, L"Reloc type", LVCFMT_CENTER, 250);
	m_listRelocsDesc.InsertColumn(1, L"Offset", LVCFMT_LEFT, 100);

	const char* relocTypes [] {
		"IMAGE_REL_BASED_ABSOLUTE",
		"IMAGE_REL_BASED_HIGH",
		"IMAGE_REL_BASED_LOW",
		"IMAGE_REL_BASED_HIGHLOW",
		"IMAGE_REL_BASED_HIGHADJ",
		"IMAGE_REL_BASED_MACHINE_SPECIFIC_5",
		"IMAGE_REL_BASED_RESERVED",
		"IMAGE_REL_BASED_MACHINE_SPECIFIC_7",
		"IMAGE_REL_BASED_MACHINE_SPECIFIC_8",
		"IMAGE_REL_BASED_MACHINE_SPECIFIC_9",
		"IMAGE_REL_BASED_DIR64"
	};

	int listindex = 0;
	WCHAR str[MAX_PATH] { };

	m_listRelocsDesc.SetRedraw(FALSE);
	for (auto& i : std::get<1>(pRelocTable->at(dwEntry)))
	{
		if (std::get<0>(i) <= sizeof(relocTypes) / sizeof(char*))
			swprintf_s(str, MAX_PATH, L"%S", relocTypes[std::get<0>(i)]);
		else
			swprintf_s(str, MAX_PATH, L"%u", std::get<0>(i));

		m_listRelocsDesc.InsertItem(listindex, str);
		swprintf_s(str, 5, L"%04X", std::get<1>(i));
		m_listRelocsDesc.SetItemText(listindex, 1, str);

		listindex++;
	}
	m_listRelocsDesc.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listRelocsDesc.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBottomLeft::CreateHexDebugEntry(DWORD dwEntry)
{
	PCLIBPE_DEBUG_VEC pDebugDir;
	if (m_pLibpe->GetDebugTable(&pDebugDir) != S_OK)
		return -1;

	m_stHexEdit.SetData(&std::get<1>(pDebugDir->at(dwEntry)));

	CRect rect;
	GetClientRect(&rect);
	m_stHexEdit.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBottomLeft::CreateTreeResources()
{
	PCLIBPE_RESOURCE_ROOT_TUP pTupResRoot { };

	if (m_pLibpe->GetResourceTable(&pTupResRoot) != S_OK)
		return -1;

	m_treeResBottom.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		TVS_LINESATROOT, CRect(0, 0, 0, 0), this, IDC_TREE_RESOURCE_BOTTOM);
	m_treeResBottom.ShowWindow(SW_HIDE);

	WCHAR str[MAX_PATH] { };

	m_imglTreeRes.Create(16, 16, ILC_COLOR32, 0, 4);
	const int iconDirs = m_imglTreeRes.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_DIR_ICON));
	m_treeResBottom.SetImageList(&m_imglTreeRes, TVSIL_NORMAL);
	long ilvlRoot = 0, ilvl2 = 0, ilvl3 = 0;

	//Creating a treeCtrl and setting, with SetItemData(...),
	//a unique id for each node, that is an index in vector (m_vecResId),
	//that holds tuple of three IDs of resource — Type, Name, LangID. 
	for (auto& iterRoot : std::get<1>(*pTupResRoot))
	{
		const IMAGE_RESOURCE_DIRECTORY_ENTRY* pResDirEntry = &std::get<0>(iterRoot);
		if (pResDirEntry->NameIsString)
			//Enclose in double quotes.
			swprintf(str, MAX_PATH, L"\u00AB%s\u00BB", std::get<1>(iterRoot).c_str());
		else
		{
			if (g_mapResType.find(pResDirEntry->Id) != g_mapResType.end())
				swprintf(str, MAX_PATH, L"%s", g_mapResType.at(pResDirEntry->Id).c_str());
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

int CViewRightBottomLeft::CreateHexTLS()
{
	PCLIBPE_TLS_TUP pTLSDir;
	if (m_pLibpe->GetTLSTable(&pTLSDir) != S_OK)
		return -1;

	m_stHexEdit.SetData(&std::get<1>(*pTLSDir));

	CRect rect;
	GetClientRect(&rect);
	m_stHexEdit.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}