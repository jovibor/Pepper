#include "stdafx.h"
#include "Pepper.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "version.h"
#include "resource.h"

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {};
	BOOL OnInitDialog() override;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
protected:
	void DoDataExchange(CDataExchange* pDX) override;
	bool m_fGithubLink { true };
	HCURSOR m_curHand { };
	HCURSOR m_curArrow { };
	HFONT m_fontDefault { };
	HFONT m_fontUnderline { };
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	WCHAR strVersion[MAX_PATH] { };
	swprintf_s(strVersion, MAX_PATH, L"Version: %u.%u.%u", MAJOR_VERSION, MINOR_VERSION, MAINTENANCE_VERSION);
	::SetWindowTextW(GetDlgItem(IDC_STATIC_VERSION)->m_hWnd, strVersion);

	//to prevent cursor from blinking
	SetClassLongPtr(m_hWnd, GCL_HCURSOR, 0);

	m_fontDefault = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

	LOGFONT logFont { };
	GetObject(m_fontDefault, sizeof(logFont), &logFont);
	logFont.lfUnderline = TRUE;

	m_fontUnderline = CreateFontIndirect(&logFont);

	m_curHand = LoadCursor(nullptr, IDC_HAND);
	m_curArrow = LoadCursor(nullptr, IDC_ARROW);

	return TRUE;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC_HTTP_GITHUB:
		pDC->SetTextColor(RGB(0, 255, 50));
		pDC->SelectObject(m_fGithubLink ? m_fontDefault : m_fontUnderline);
		break;
	}

	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd* pWnd = ChildWindowFromPoint(point);

	if (!pWnd)
		return;

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_HTTP_GITHUB)
		ShellExecute(nullptr, L"open", L"https://github.com/jovibor/Pepper", nullptr, nullptr, NULL);

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd* pWnd = ChildWindowFromPoint(point);

	if (!pWnd)
		return;

	if (m_fGithubLink == (pWnd->GetDlgCtrlID() == IDC_STATIC_HTTP_GITHUB))
	{
		m_fGithubLink = !m_fGithubLink;
		::InvalidateRect(GetDlgItem(IDC_STATIC_HTTP_GITHUB)->m_hWnd, nullptr, FALSE);
		SetCursor(m_fGithubLink ? m_curArrow : m_curHand);
	}

	CDialogEx::OnMouseMove(nFlags, point);
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
	if (cmdInfo.m_strFileName == "")
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