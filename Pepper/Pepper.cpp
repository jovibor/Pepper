/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "Pepper.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "PepperDoc.h"
#include "resource.h"
#include "constants.h"

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {};
protected:
	BOOL OnInitDialog() override;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	bool m_fGithubLink { true };
	HCURSOR m_curHand { };
	HCURSOR m_curArrow { };
	CFont m_fontDefault;
	CFont m_fontUnderline;
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

	GetDlgItem(IDC_STATIC_VERSION)->SetWindowTextW(PEPPER_VERSION_WSTR);

	//to prevent cursor from blinking
	SetClassLongPtrW(m_hWnd, GCLP_HCURSOR, 0);

	m_fontDefault.CreateStockObject(DEFAULT_GUI_FONT);
	LOGFONTW lf;
	m_fontDefault.GetLogFont(&lf);
	lf.lfUnderline = TRUE;
	m_fontUnderline.CreateFontIndirectW(&lf);

	m_curHand = LoadCursor(nullptr, IDC_HAND);
	m_curArrow = LoadCursor(nullptr, IDC_ARROW);

	return TRUE;
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC_HTTP_GITHUB:
		pDC->SetTextColor(RGB(0, 255, 50));
		pDC->SelectObject(m_fGithubLink ? &m_fontDefault : &m_fontUnderline);
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
		GetDlgItem(IDC_STATIC_HTTP_GITHUB)->RedrawWindow();
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
	CFileDialog stFD(TRUE, NULL, NULL,
		OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ALLOWMULTISELECT |
		OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, L"All files (*.*)|*.*||");

	if (stFD.DoModal() == IDOK)
	{
		CComPtr<IFileOpenDialog> pIFOD = stFD.GetIFileOpenDialog();
		CComPtr<IShellItemArray> pResults;
		pIFOD->GetResults(&pResults);

		DWORD dwCount { };
		pResults->GetCount(&dwCount);
		for (unsigned i = 0; i < dwCount; i++)
		{
			CComPtr<IShellItem> pItem;
			pResults->GetItemAt(i, &pItem);
			CComHeapPtr<wchar_t> pwstrPath;
			pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwstrPath);

			CWinAppEx::OpenDocumentFile(pwstrPath);
		}
	}
}

void CPepperApp::PreLoadState()
{
}