/*****************************************************************
* Copyright © 2018-present Jovibor https://github.com/jovibor/   *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.  *
* Official git repository: https://github.com/jovibor/Pepper/    *
* This software is available under the Apache-2.0 License.       *
*****************************************************************/
#include "stdafx.h"
#include "CChildFrm.h"
#include "CMainFrm.h"
#include "CPepper.h"
#include "CPepperDoc.h"
#include "res/resource.h"
#include <format>

import Utility;

class CAboutDlg : public CDialogEx {
public:
	CAboutDlg() : CDialogEx(IDD_ABOUTBOX) { };
protected:
	BOOL OnInitDialog()override;
};

#define STR2WIDE(x) L##x
#define STRWIDER(x) STR2WIDE(x)

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	const auto wstrVerPepper = std::format(L"Pepper: PE32/PE32+ Binaries Analyzer v{}.{}.{}", ut::PEPPER_VERSION_MAJOR,
		ut::PEPPER_VERSION_MINOR, ut::PEPPER_VERSION_PATCH);
	GetDlgItem(IDC_LINK_PEPPER)->SetWindowTextW(wstrVerPepper.data());
	const auto wstrVerLibpe = std::format(L"libpe: PE32/PE32+ Binaries Parsing Library v{}.{}.{}",
		libpe::LIBPE_VERSION_MAJOR, libpe::LIBPE_VERSION_MINOR, libpe::LIBPE_VERSION_PATCH);
	GetDlgItem(IDC_LINK_LIBPE)->SetWindowTextW(wstrVerLibpe.data());
	const auto wstrVerHexCtrl = std::format(L"HexCtrl: Hex Control for MFC/Win32 v{}.{}.{}",
		HEXCTRL::HEXCTRL_VERSION_MAJOR, HEXCTRL::HEXCTRL_VERSION_MINOR, HEXCTRL::HEXCTRL_VERSION_PATCH);
	GetDlgItem(IDC_LINK_HEXCTRL)->SetWindowTextW(wstrVerHexCtrl.data());
	GetDlgItem(IDC_STATIC_TIME)->SetWindowTextW(L"Built on: " STRWIDER(__DATE__) L" "  STRWIDER(__TIME__));

	return TRUE;
}

CPepperApp theApp;

BEGIN_MESSAGE_MAP(CPepperApp, CWinAppEx)
	ON_COMMAND(IDM_HELP_ABOUT, &CPepperApp::OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, &CPepperApp::OnFileOpen)
	ON_UPDATE_COMMAND_UI(IDM_HELP_ABOUT, &CPepperApp::OnUpdateHelpAbout)
END_MESSAGE_MAP()

void CPepperApp::OpenNewFile()
{
	OnFileOpen();
}

BOOL CPepperApp::InitInstance()
{
	CWinAppEx::InitInstance();

	SetRegistryKey(L"Pepper");
	LoadStdProfileSettings(5);

	//Modern looking tooltips.
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	const auto pDocTemplate = new CMultiDocTemplate(IDR_MAINFRAME, RUNTIME_CLASS(CPepperDoc), RUNTIME_CLASS(CChildFrame), nullptr);
	AddDocTemplate(pDocTemplate);

	const auto pMainFrame = new CMainFrame;	// create main MDI Frame window
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME)) {
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	//For Drag'n Drop to work, even in elevated mode.
	//helgeklein.com/blog/2010/03/how-to-enable-drag-and-drop-for-an-elevated-mfc-application-on-vistawindows-7/
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
	DragAcceptFiles(m_pMainWnd->m_hWnd, TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew) {
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	}

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
	if (cmdInfo.m_strFileName.IsEmpty()) {
		OnFileOpen();
	}

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
	const auto lmbFOD = [this]()->bool {
		CFileDialog fd(TRUE, nullptr, nullptr, OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ALLOWMULTISELECT |
			OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, L"All files (*.*)|*.*||");

		if (fd.DoModal() == IDOK) {
			CComPtr<IFileOpenDialog> pIFOD = fd.GetIFileOpenDialog();
			CComPtr<IShellItemArray> pResults;
			pIFOD->GetResults(&pResults);

			bool fOpened { false };
			DWORD dwCount { };
			pResults->GetCount(&dwCount);
			for (auto i = 0U; i < dwCount; ++i) {
				CComPtr<IShellItem> pItem;
				pResults->GetItemAt(i, &pItem);
				CComHeapPtr<wchar_t> pwstrPath;
				pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwstrPath);
				const auto pDoc = CWinAppEx::OpenDocumentFile(pwstrPath);
				fOpened = !fOpened ? pDoc != nullptr : true;
			}
			return fOpened;
		}
		return true;
		};

	while (!lmbFOD()) { }; //If no file has been opened (if multiple selection) we show the Open File Dialog again.
}

void CPepperApp::OnUpdateHelpAbout(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(0);
}