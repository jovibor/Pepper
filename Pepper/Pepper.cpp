#include "stdafx.h"
#include "Pepper.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "version.h"

BEGIN_MESSAGE_MAP(CPepperApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CPepperApp::OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, &CPepperApp::OnFileOpen)
END_MESSAGE_MAP()

CPepperApp::CPepperApp()
{
	m_bHiColorIcons = TRUE;
}

CPepperApp theApp;

BOOL CPepperApp::InitInstance()
{
	CWinAppEx::InitInstance();

	SetRegistryKey(L"Pepper - PE files viewer");

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_PepperTYPE,
		RUNTIME_CLASS(CPepperDoc), RUNTIME_CLASS(CChildFrame), 0);

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

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {};
	virtual BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

void CPepperApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CPepperApp::OnFileOpen()
{
	WCHAR _strFilePath[1024] { };

	OPENFILENAME _stOFN { };
	_stOFN.lStructSize = sizeof(_stOFN);
	_stOFN.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	_stOFN.lpstrFilter = L"All files (*.*)\0*.*\0\0";
	_stOFN.lpstrFile = _strFilePath;
	_stOFN.nMaxFile = sizeof(_strFilePath) / sizeof(WCHAR);
	_stOFN.lpstrTitle = L"Select one or more PE files";
	_stOFN.Flags = OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
		OFN_EXPLORER | OFN_ENABLESIZING | OFN_DONTADDTORECENT;

	GetOpenFileName(&_stOFN);

	//Checking for multi file selection:
	//If _strFilePath at offset [_stOFN.nFileOffset - 1] equals '\0'
	//it means that we have multiple file names following
	//path name, divided by NULLs. See OFN_ALLOWMULTISELECT description.
	if (_strFilePath[_stOFN.nFileOffset - 1] == '\0')
	{
		WCHAR* _str = _stOFN.lpstrFile;
		std::wstring _strDir = _str;
		_str += (_strDir.length() + 1);
		while (*_str)
		{
			std::wstring _strFileName = _str;
			_str += (_strFileName.length() + 1);
			std::wstring _strFullPath = _strDir + L"\\" + _strFileName;

			CWinAppEx::OpenDocumentFile(_strFullPath.c_str());
		}
	}
	else
		CWinAppEx::OpenDocumentFile(_stOFN.lpstrFile);
}

void CPepperApp::PreLoadState()
{

}

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	WCHAR _strVersion[MAX_PATH] { };
	swprintf(_strVersion, L"%S, Version: %u.%u.%u.%u", PRODUCT_NAME, MAJOR_VERSION, MINOR_VERSION, MAINTENANCE_VERSION + 1, REVISION_VERSION);

	::SetWindowTextW(GetDlgItem(IDC_STATIC_VERSION)->m_hWnd, _strVersion);

	return TRUE;
}
