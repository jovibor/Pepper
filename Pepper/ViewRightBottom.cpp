#include "stdafx.h"
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "ViewRightBottom.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CViewRightBottom, CView)

BEGIN_MESSAGE_MAP(CViewRightBottom, CView)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CViewRightBottom::OnDraw(CDC* pDC)
{
}

void CViewRightBottom::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_ChildFrame = (CChildFrame*)GetParentFrame();

	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	if (!m_pLibpe)
		return;

	const DWORD* m_pFileSummary { };
	if (m_pLibpe->GetFileSummary(&m_pFileSummary) != S_OK)
		return;

	m_dwFileSummary = *m_pFileSummary;

	listCreateExportFuncs();
	treeCreateResourceDir();
	m_HexEdit.Create(this, CRect(0, 0, 0, 0), HEXCTRLID_SECURITY_DIR_SERTIFICATE_ID);
}

int CViewRightBottom::HexCtrlRightBottom(unsigned nSertId)
{
	if (m_pLibpe->GetSecurityTable(&m_vecSec) != S_OK)
		return -1;
	if (nSertId > m_vecSec->size())
		return -1;

	m_HexEdit.SetData(&std::get<1>(m_vecSec->at(nSertId)));
	m_HexEdit.ShowWindow(SW_SHOW);

	CRect rect;
	GetClientRect(&rect);
	m_HexEdit.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

BOOL CViewRightBottom::OnEraseBkgnd(CDC* pDC)
{
	return CScrollView::OnEraseBkgnd(pDC);
}

void CViewRightBottom::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	if (m_pActiveList)
		m_pActiveList->ShowWindow(SW_HIDE);

	CRect rect;
	GetClientRect(&rect);

	switch (LOWORD(lHint))
	{
	case LISTID_IMPORT_DLL_FUNCS:
		listCreateImportFuncs(HIWORD(lHint));
		m_pActiveList = &m_listImportFuncs;
		break;
	case LISTID_DELAY_IMPORT_DLL_FUNCS:
		listCreateDelayImportFuncs(HIWORD(lHint));
		m_pActiveList = &m_listDelayImportFuncs;
		break;
	case LISTID_EXPORT_DIR:
		m_listExportFuncs.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_listExportFuncs;
		break;
	case HEXCTRLID_SECURITY_DIR_SERTIFICATE_ID:
		HexCtrlRightBottom(HIWORD(lHint));
		m_pActiveList = &m_HexEdit;
		break;
	case LISTID_RELOCATION_DIR_RELOCS_DESC:
		listCreateRelocations(HIWORD(lHint));
		m_pActiveList = &m_listRelocationsDescription;
		break;
	case LISTID_RESOURCE_DIR:
		m_treeResourceDirBottom.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_treeResourceDirBottom;
		break;
	}
}

HBRUSH CViewRightBottom::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CScrollView::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

void CViewRightBottom::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveList)
		m_pActiveList->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

int CViewRightBottom::listCreateImportFuncs(UINT dllId)
{
	PCLIBPE_IMPORT_VEC m_pImportTable { };

	if (m_pLibpe->GetImportTable(&m_pImportTable) != S_OK)
		return -1;

	if (dllId > m_pImportTable->size())
		return -1;

	m_listImportFuncs.DestroyWindow();
	m_listImportFuncs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_IMPORT_DLL_FUNCS);

	m_listImportFuncs.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listImportFuncs.InsertColumn(0, L"Function Name", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 175);
	m_listImportFuncs.InsertColumn(1, L"Ordinal / Hint", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listImportFuncs.InsertColumn(2, L"ThunkRVA", LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 150);

	int listindex = 0;
	TCHAR str[MAX_PATH] { };

	m_listImportFuncs.SetRedraw(FALSE);
	for (auto& i : std::get<2>(m_pImportTable->at(dllId)))
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

int CViewRightBottom::listCreateDelayImportFuncs(UINT dllId)
{
	PCLIBPE_DELAYIMPORT_VEC pDelayImport { };

	if (m_pLibpe->GetDelayImportTable(&pDelayImport) != S_OK)
		return -1;

	if (dllId > pDelayImport->size())
		return -1;

	m_listDelayImportFuncs.DestroyWindow();
	m_listDelayImportFuncs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_DELAY_IMPORT_DLL_FUNCS);

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
	for (auto&i : std::get<2>(pDelayImport->at(dllId)))
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

int CViewRightBottom::listCreateExportFuncs()
{
	PCLIBPE_EXPORT_TUP pExportTable { };

	if (m_pLibpe->GetExportTable(&pExportTable) != S_OK)
		return -1;

	m_listExportFuncs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_EXPORT_FUNCS);
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

int CViewRightBottom::listCreateRelocations(UINT blockID)
{
	PCLIBPE_RELOCATION_VEC pRelocTable { };

	if (m_pLibpe->GetRelocationTable(&pRelocTable) != S_OK)
		return -1;

	m_listRelocationsDescription.DestroyWindow();

	m_listRelocationsDescription.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_RELOCATION_TYPE);
	m_listRelocationsDescription.ShowWindow(SW_HIDE);
	m_listRelocationsDescription.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listRelocationsDescription.InsertColumn(0, L"Reloc type", LVCFMT_CENTER, 250);
	m_listRelocationsDescription.InsertColumn(1, L"Offset", LVCFMT_LEFT, 100);

	char* relocTypes [] {
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
	TCHAR str[MAX_PATH] { };

	m_listRelocationsDescription.SetRedraw(FALSE);
	for (auto& i : std::get<1>(pRelocTable->at(blockID)))
	{
		if (std::get<0>(i) <= sizeof(relocTypes) / sizeof(char*))
			swprintf_s(str, MAX_PATH, L"%S", relocTypes[std::get<0>(i)]);
		else
			swprintf_s(str, MAX_PATH, L"%u", std::get<0>(i));

		m_listRelocationsDescription.InsertItem(listindex, str);
		swprintf_s(str, 5, L"%04X", std::get<1>(i));
		m_listRelocationsDescription.SetItemText(listindex, 1, str);

		listindex++;
	}
	m_listRelocationsDescription.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listRelocationsDescription.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBottom::treeCreateResourceDir()
{
	PCLIBPE_RESOURCE_ROOT_TUP pResourceTable { };

	if (m_pLibpe->GetResourceTable(&pResourceTable) != S_OK)
		return -1;

	LIBPE_RESOURCE_LVL2_TUP pResourceLvL2 { };
	LIBPE_RESOURCE_LVL3_TUP pResourceLvL3 { };

	m_treeResourceDirBottom.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		TVS_LINESATROOT, CRect(0, 0, 0, 0), this, TREEID_RESOURCE_BOTTOM);
	m_treeResourceDirBottom.ShowWindow(SW_HIDE);

	std::map<WORD, std::wstring> mapResType {
		{ 1, L"CURSOR" },
	{ 2, L"BITMAP" },
	{ 3, L"ICON" },
	{ 4, L"MENU" },
	{ 5, L"DIALOG" },
	{ 6, L"STRING" },
	{ 7, L"FONTDIR" },
	{ 8, L"FONT" },
	{ 9, L"ACCELERATOR" },
	{ 10, L"RCDATA" },
	{ 11, L"MESSAGETABLE" },
	{ 12, L"GROUP_CURSOR" },
	{ 14, L"GROUP_ICON" },
	{ 16, L"VERSION" },
	{ 17, L"DLGINCLUDE" },
	{ 19, L"PLUGPLAY" },
	{ 20, L"VXD" },
	{ 21, L"ANICURSOR" },
	{ 22, L"ANIICON" },
	{ 23, L"HTML" },
	{ 24, L"MANIFEST" }
	};

	WCHAR str[MAX_PATH] { };

	m_imglTreeResource.Create(16, 16, ILC_COLOR32, 0, 4);
	int iconDirs = m_imglTreeResource.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_DIR_ICON));
	m_treeResourceDirBottom.SetImageList(&m_imglTreeResource, TVSIL_NORMAL);
	DWORD_PTR nResId { };//Resource ID (incremental) to extract later on

	for (auto& iterRoot : std::get<1>(*pResourceTable))
	{
		const IMAGE_RESOURCE_DIRECTORY_ENTRY* pResDirEntry = &std::get<0>(iterRoot);
		if (pResDirEntry->NameIsString)
			//Enclose in double quotes
			swprintf(str, MAX_PATH, L"\u00AB%s\u00BB", std::get<1>(iterRoot).c_str());
		else
		{
			if (mapResType.find(pResDirEntry->Id) != mapResType.end())
				swprintf(str, MAX_PATH, L"%s", mapResType.at(pResDirEntry->Id).c_str());
			else
				swprintf(str, MAX_PATH, L"%u", pResDirEntry->Id);
		}
		const HTREEITEM treeRoot = m_treeResourceDirBottom.InsertItem(str, iconDirs, iconDirs);
		nResId++;
		m_treeResourceDirBottom.SetItemData(treeRoot, nResId);

		pResourceLvL2 = std::get<4>(iterRoot);
		for (auto& iterLvL2 : std::get<1>(pResourceLvL2))
		{
			pResDirEntry = &std::get<0>(iterLvL2);
			nResId++;

			pResourceLvL3 = std::get<4>(iterLvL2);
			for (auto& iterLvL3 : std::get<1>(pResourceLvL3))
			{
				pResDirEntry = &std::get<0>(iterLvL3);
				if (pResDirEntry->NameIsString)
					swprintf(str, MAX_PATH, L"«%s» - lang: %i", std::get<1>(iterLvL2).c_str(), pResDirEntry->Id);
				else
					swprintf(str, MAX_PATH, L"%u - lang: %i", std::get<0>(iterLvL2).Id, pResDirEntry->Id);

				m_treeResourceDirBottom.SetItemData(m_treeResourceDirBottom.InsertItem(str, treeRoot), nResId);
				nResId++;
			}
		}
	}

	return 0;
}

