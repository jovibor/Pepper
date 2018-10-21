#include "stdafx.h"
#include "Pepper.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "version.h"

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {};
	virtual BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	WCHAR strVersion[MAX_PATH] { };
	swprintf_s(strVersion, MAX_PATH, L"%S, Version: %u.%u.%u.%u", PRODUCT_NAME, MAJOR_VERSION, MINOR_VERSION, MAINTENANCE_VERSION, REVISION_VERSION);

	::SetWindowTextW(GetDlgItem(IDC_STATIC_VERSION)->m_hWnd, strVersion);

	return TRUE;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CPepperApp theApp;

BEGIN_MESSAGE_MAP(CPepperApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CPepperApp::OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, &CPepperApp::OnFileOpen)
END_MESSAGE_MAP()

CPepperApp::CPepperApp()
{
	m_bHiColorIcons = TRUE;
}

BOOL CPepperApp::InitInstance()
{
	CWinAppEx::InitInstance();

	SetRegistryKey(L"Pepper - PE files viewer");

	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_PepperTYPE,
		RUNTIME_CLASS(CPepperDoc), RUNTIME_CLASS(CChildFrame), nullptr);

	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	OnFileOpen();

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
	WCHAR strFilePath[2048] { };

	OPENFILENAME stOFN { };
	stOFN.lStructSize = sizeof(stOFN);
	stOFN.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	stOFN.lpstrFilter = L"All files (*.*)\0*.*\0\0";
	stOFN.lpstrFile = strFilePath;
	stOFN.nMaxFile = sizeof(strFilePath) / sizeof(WCHAR);
	stOFN.lpstrTitle = L"Select one or more PE files";
	stOFN.Flags = OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
		OFN_EXPLORER | OFN_ENABLESIZING | OFN_DONTADDTORECENT;

	GetOpenFileName(&stOFN);

	//Checking for multi file selection:
	//If strFilePath at offset [stOFN.nFileOffset - 1] equals '\0'
	//it means that we have multiple file names following path name,
	//divided with NULLs ('\0'). See OFN_ALLOWMULTISELECT description.
	if (strFilePath[stOFN.nFileOffset - 1] == '\0')
	{
		WCHAR* str = stOFN.lpstrFile;
		std::wstring strDir = str;
		str += (strDir.length() + 1);
		while (*str)
		{
			std::wstring strFileName = str;
			str += (strFileName.length() + 1);
			std::wstring strFullPath = strDir + L"\\" + strFileName;

			CWinAppEx::OpenDocumentFile(strFullPath.c_str());
		}
	}
	else
		CWinAppEx::OpenDocumentFile(stOFN.lpstrFile);
}

void CPepperApp::PreLoadState()
{
}