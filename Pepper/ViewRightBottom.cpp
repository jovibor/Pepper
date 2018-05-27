#include "stdafx.h"
#include "Pepper.h"
#include "PepperDoc.h"
#include "PepperTreeCtrl.h"
#include "PepperList.h"
#include "ViewRightBottom.h"

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
	m_HexEdit.CreateCtrl(this, CRect(0, 0, 0, 0), HEXCTRLID_SECURITY_DIR_SERTIFICATE_ID);
}

int CViewRightBottom::HexCtrlRightBottom(unsigned nSertId)
{
	if (m_pLibpe->GetSecurityTable(&_vecSec) != S_OK)
		return -1;
	if (nSertId > _vecSec->size())
		return -1;

	m_HexEdit.SetData(&std::get<1>(_vecSec->at(nSertId)));
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
	case LISTID_RELOCATION_DIR_RELOCS_DESCRIPTION:
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
	PLIBPE_IMPORT m_pImportTable { };

	if (m_pLibpe->GetImportTable(&m_pImportTable) != S_OK)
		return -1;

	if (dllId > m_pImportTable->size())
		return -1;

	m_listImportFuncs.DestroyWindow();
	m_listImportFuncs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_IMPORT_DLL_FUNCS);

	m_listImportFuncs.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listImportFuncs.InsertColumn(0, _T("Function Name"), LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 175);
	m_listImportFuncs.InsertColumn(1, _T("Ordinal / Hint"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listImportFuncs.InsertColumn(2, _T("ThunkRVA"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 150);

	int _listindex = 0;
	TCHAR str[MAX_PATH] { };

	m_listImportFuncs.SetRedraw(FALSE);
	for (auto& i : std::get<2>(m_pImportTable->at(dllId)))
	{
		swprintf_s(str, 256, L"%S", std::get<1>(i).c_str());
		m_listImportFuncs.InsertItem(_listindex, str);
		swprintf_s(str, 17, L"%04llX", std::get<0>(i));
		m_listImportFuncs.SetItemText(_listindex, 1, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<2>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<2>(i));
		m_listImportFuncs.SetItemText(_listindex, 2, str);

		_listindex++;
	}
	m_listImportFuncs.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listImportFuncs.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBottom::listCreateDelayImportFuncs(UINT dllId)
{
	PLIBPE_DELAYIMPORT _pDelayImport { };

	if (m_pLibpe->GetDelayImportTable(&_pDelayImport) != S_OK)
		return -1;

	if (dllId > _pDelayImport->size())
		return -1;

	m_listDelayImportFuncs.DestroyWindow();
	m_listDelayImportFuncs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_DELAY_IMPORT_DLL_FUNCS);

	m_listDelayImportFuncs.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listDelayImportFuncs.InsertColumn(0, _T("Function Name"), LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 300);
	m_listDelayImportFuncs.InsertColumn(1, _T("Ordinal / Hint"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listDelayImportFuncs.InsertColumn(2, _T("ImportNameTable ThunkRVA"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 200);
	m_listDelayImportFuncs.InsertColumn(3, _T("ImportAddressTable ThunkRVA"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 220);
	m_listDelayImportFuncs.InsertColumn(4, _T("BoundImportAddressTable ThunkRVA"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 250);
	m_listDelayImportFuncs.InsertColumn(5, _T("UnloadInformationTable ThunkRVA"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 240);

	int _listindex = 0;
	TCHAR str[MAX_PATH] { };

	m_listDelayImportFuncs.SetRedraw(FALSE);
	for (auto&i : std::get<2>(_pDelayImport->at(dllId)))
	{
		swprintf_s(str, 256, L"%S", std::get<1>(i).c_str());
		m_listDelayImportFuncs.InsertItem(_listindex, str);
		swprintf_s(str, 17, L"%04llX", std::get<0>(i));
		m_listDelayImportFuncs.SetItemText(_listindex, 1, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<2>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<2>(i));
		m_listDelayImportFuncs.SetItemText(_listindex, 2, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<3>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<3>(i));
		m_listDelayImportFuncs.SetItemText(_listindex, 3, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<4>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<4>(i));
		m_listDelayImportFuncs.SetItemText(_listindex, 4, str);
		if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE32_FLAG))
			swprintf_s(str, 9, L"%08llX", std::get<5>(i));
		else if (IMAGE_HAS_FLAG(m_dwFileSummary, IMAGE_PE64_FLAG))
			swprintf_s(str, 17, L"%016llX", std::get<5>(i));
		m_listDelayImportFuncs.SetItemText(_listindex, 5, str);

		_listindex++;
	}
	m_listDelayImportFuncs.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listDelayImportFuncs.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBottom::listCreateExportFuncs()
{
	PLIBPE_EXPORT _pExportTable { };

	if (m_pLibpe->GetExportTable(&_pExportTable) != S_OK)
		return -1;

	m_listExportFuncs.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_EXPORT_FUNCS);
	m_listExportFuncs.ShowWindow(SW_HIDE);
	m_listExportFuncs.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listExportFuncs.InsertColumn(0, _T("Function RVA"), LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 100);
	m_listExportFuncs.InsertColumn(1, _T("Ordinal"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 100);
	m_listExportFuncs.InsertColumn(2, _T("Name"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 250);
	m_listExportFuncs.InsertColumn(3, _T("Forwarder Name"), LVCFMT_LEFT | LVCFMT_FIXED_WIDTH, 400);

	m_listExportFuncs.SetRedraw(FALSE); //to increase the speed of List populating
	int _listindex = 0;
	TCHAR str[MAX_PATH] { };

	for (auto& i : std::get<2>(*_pExportTable))
	{
		swprintf_s(str, 9, L"%08X", std::get<0>(i));
		m_listExportFuncs.InsertItem(_listindex, str);
		swprintf_s(str, 9, L"%u", std::get<1>(i));
		m_listExportFuncs.SetItemText(_listindex, 1, str);
		swprintf_s(str, MAX_PATH, L"%S", std::get<2>(i).c_str());
		m_listExportFuncs.SetItemText(_listindex, 2, str);
		swprintf_s(str, MAX_PATH, L"%S", std::get<3>(i).c_str());
		m_listExportFuncs.SetItemText(_listindex, 3, str);

		_listindex++;
	}
	m_listExportFuncs.SetRedraw(TRUE);

	return 0;
}

int CViewRightBottom::listCreateRelocations(UINT blockID)
{
	PLIBPE_RELOCATION _pRelocTable { };

	if (m_pLibpe->GetRelocationTable(&_pRelocTable) != S_OK)
		return -1;

	m_listRelocationsDescription.DestroyWindow();

	m_listRelocationsDescription.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_OWNERDRAWFIXED | LVS_REPORT,
		CRect(0, 0, 0, 0), this, LISTID_RELOCATION_TYPE);
	m_listRelocationsDescription.ShowWindow(SW_HIDE);
	m_listRelocationsDescription.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_listRelocationsDescription.InsertColumn(0, _T("Reloc type"), LVCFMT_CENTER, 250);
	m_listRelocationsDescription.InsertColumn(1, _T("Offset"), LVCFMT_LEFT, 100);

	char* _relocTypes [] {
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

	int _listindex = 0;
	TCHAR str[MAX_PATH] { };

	m_listRelocationsDescription.SetRedraw(FALSE);
	for (auto& i : std::get<1>(_pRelocTable->at(blockID)))
	{
		if (std::get<0>(i) <= sizeof(_relocTypes) / sizeof(char*))
			swprintf_s(str, MAX_PATH, L"%S", _relocTypes[std::get<0>(i)]);
		else
			swprintf_s(str, MAX_PATH, L"%u", std::get<0>(i));
		m_listRelocationsDescription.InsertItem(_listindex, str);
		swprintf_s(str, 5, L"%04X", std::get<1>(i));
		m_listRelocationsDescription.SetItemText(_listindex, 1, str);

		_listindex++;
	}
	m_listRelocationsDescription.SetRedraw(TRUE);

	CRect rect;
	GetClientRect(&rect);
	m_listRelocationsDescription.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	return 0;
}

int CViewRightBottom::treeCreateResourceDir()
{
	PLIBPE_RESOURCE_ROOT _pResourceTable { };

	if (m_pLibpe->GetResourceTable(&_pResourceTable) != S_OK)
		return -1;

	LIBPE_RESOURCE_LVL2 _pResourceLvL2 { };
	LIBPE_RESOURCE_LVL3 _pResourceLvL3 { };

	m_treeResourceDirBottom.Create(TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		TVS_LINESATROOT, CRect(0, 0, 0, 0), this, TREEID_RESOURCE_BOTTOM);
	m_treeResourceDirBottom.ShowWindow(SW_HIDE);

	std::map<WORD, std::wstring> _mapResType {
		{ 1, TEXT("CURSOR") },
	{ 2, TEXT("BITMAP") },
	{ 3, TEXT("ICON") },
	{ 4, TEXT("MENU") },
	{ 5, TEXT("DIALOG") },
	{ 6, TEXT("STRING") },
	{ 7, TEXT("FONTDIR") },
	{ 8, TEXT("FONT") },
	{ 9, TEXT("ACCELERATOR") },
	{ 10, TEXT("RCDATA") },
	{ 11, TEXT("MESSAGETABLE") },
	{ 12, TEXT("GROUP_CURSOR") },
	{ 14, TEXT("GROUP_ICON") },
	{ 16, TEXT("VERSION") },
	{ 17, TEXT("DLGINCLUDE") },
	{ 19, TEXT("PLUGPLAY") },
	{ 20, TEXT("VXD") },
	{ 21, TEXT("ANICURSOR") },
	{ 22, TEXT("ANIICON") },
	{ 23, TEXT("HTML") },
	{ 24, TEXT("MANIFEST") }
	};

	const IMAGE_RESOURCE_DIRECTORY_ENTRY* _pResDirEntry { };
	WCHAR str[MAX_PATH] { };
	HTREEITEM _treeRoot { };

	m_imglTreeResource.Create(16, 16, ILC_COLOR32, 0, 4);
	int _iconDirs = m_imglTreeResource.Add(AfxGetApp()->LoadIconW(IDI_TREE_MAIN_DIR_ICON));
	m_treeResourceDirBottom.SetImageList(&m_imglTreeResource, TVSIL_NORMAL);
	DWORD_PTR _nResId { };//Resource ID (incremental) to extract later on

	for (auto& iterRoot : std::get<1>(*_pResourceTable))
	{
		_pResDirEntry = &std::get<0>(iterRoot);
		if (_pResDirEntry->NameIsString)
			//Enclose in double quotes
			swprintf(str, MAX_PATH, L"\u00AB%s\u00BB", std::get<1>(iterRoot).c_str());
		else
		{
			if (_mapResType.find(_pResDirEntry->Id) != _mapResType.end())
				swprintf(str, MAX_PATH, L"%s", _mapResType.at(_pResDirEntry->Id).c_str());
			else
				swprintf(str, MAX_PATH, L"%u", _pResDirEntry->Id);
		}
		_treeRoot = m_treeResourceDirBottom.InsertItem(str, _iconDirs, _iconDirs);
		_nResId++;
		m_treeResourceDirBottom.SetItemData(_treeRoot, _nResId);

		_pResourceLvL2 = std::get<4>(iterRoot);
		for (auto& iterLvL2 : std::get<1>(_pResourceLvL2))
		{
			_pResDirEntry = &std::get<0>(iterLvL2);
			_nResId++;

			_pResourceLvL3 = std::get<4>(iterLvL2);
			for (auto& iterLvL3 : std::get<1>(_pResourceLvL3))
			{
				_pResDirEntry = &std::get<0>(iterLvL3);
				if (_pResDirEntry->NameIsString)
					swprintf(str, MAX_PATH, L"«%s» - lang: %i", std::get<1>(iterLvL2).c_str(), _pResDirEntry->Id);
				else
					swprintf(str, MAX_PATH, L"%u - lang: %i", std::get<0>(iterLvL2).Id, _pResDirEntry->Id);

				m_treeResourceDirBottom.SetItemData(m_treeResourceDirBottom.InsertItem(str, _treeRoot), _nResId);
				_nResId++;
			}
		}
	}

	return 0;
}

