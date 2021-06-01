/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "ChildFrm.h"
#include "MainFrm.h"
#include "Pepper.h"
#include "PepperDoc.h"
#include "Utility.h"
#include "res/resource.h"

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {};
protected:
	BOOL OnInitDialog()override;
};

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetDlgItem(IDC_LINK_PEPPER)->SetWindowTextW(PEPPER_VERSION_WSTR);
	std::wstring wstrlibpeVer = L"libpe - PE/PE+ binaries library v";
	wstrlibpeVer += libpeInfo()->pwszVersion;
	GetDlgItem(IDC_LINK_LIBPE)->SetWindowTextW(wstrlibpeVer.data());
	wstrlibpeVer = L"HexCtrl - ";
	wstrlibpeVer += GetHexCtrlInfo()->pwszVersion;
	GetDlgItem(IDC_LINK_HEXCTRL)->SetWindowTextW(wstrlibpeVer.data());

	return TRUE;
}

CPepperApp theApp;

BEGIN_MESSAGE_MAP(CPepperApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CPepperApp::OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, &CPepperApp::OnFileOpen)
END_MESSAGE_MAP()

CPepperApp::CPepperApp()
{
	m_bHiColorIcons = TRUE;
}

void CPepperApp::OpenNewFile()
{
	OnFileOpen();
}

BOOL CPepperApp::InitInstance()
{
	CWinAppEx::InitInstance();

	SetRegistryKey(L"Pepper - PE files viewer");

	//Modern looking tooltips.
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_PepperTYPE,
		RUNTIME_CLASS(CPepperDoc), RUNTIME_CLASS(CChildFrame), nullptr);

	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	auto* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	//For Drag'n Drop working, even in elevated state.
	//Good explanation here:
	//helgeklein.com/blog/2010/03/how-to-enable-drag-and-drop-for-an-elevated-mfc-application-on-vistawindows-7/
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
	DragAcceptFiles(m_pMainWnd->m_hWnd, TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	PVOID pOldValue;
	Wow64DisableWow64FsRedirection(&pOldValue);

	// Dispatch commands specified on the command line. Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	//To prevent OpenFileDialog popup if app was launched by
	//dropping any file on app's shortcut 
	//(with command line arg file name to be opened).
	if (cmdInfo.m_strFileName.IsEmpty())
		OnFileOpen();

	Wow64RevertWow64FsRedirection(pOldValue);

	return TRUE;
}

int CPepperApp::ExitInstance()
{
	return CWinAppEx::ExitInstance();
}

void CPepperApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CPepperApp::OnFileOpen()
{
	CFileDialog fd(TRUE, nullptr, nullptr, OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ALLOWMULTISELECT |
		OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, L"All files (*.*)|*.*||");

	if (fd.DoModal() == IDOK)
	{
		CComPtr<IFileOpenDialog> pIFOD = fd.GetIFileOpenDialog();
		CComPtr<IShellItemArray> pResults;
		pIFOD->GetResults(&pResults);

		bool fOpened { false };
		DWORD dwCount { };
		pResults->GetCount(&dwCount);
		for (auto i = 0U; i < dwCount; ++i)
		{
			CComPtr<IShellItem> pItem;
			pResults->GetItemAt(i, &pItem);
			CComHeapPtr<wchar_t> pwstrPath;
			pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwstrPath);
			const auto pDoc = CWinAppEx::OpenDocumentFile(pwstrPath);
			fOpened = !fOpened ? pDoc != nullptr : true;
		}
		//In case no file has been opened (if multiple selection) we show the open file dialog again.
		if (!fOpened)
			OnFileOpen();
	}
}

void CPepperApp::PreLoadState()
{
}