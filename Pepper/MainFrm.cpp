/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "MainFrm.h"
#include "res/resource.h"
#include "PepperDoc.h"

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_WM_DROPFILES()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_APP_EDITMODE, &CMainFrame::OnAppEditmode)
	ON_UPDATE_COMMAND_UI(ID_APP_EDITMODE, &CMainFrame::OnUpdateAppEditmode)
END_MESSAGE_MAP()

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetMenu(nullptr);

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
	mdiTabParams.m_bTabIcons = TRUE;    // set to TRUE to enable document icons on MDI taba
	mdiTabParams.m_bAutoColor = FALSE;    // set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = FALSE; // enable the document menu at the right edge of the tab area
	mdiTabParams.m_bFlatFrame = TRUE;
	mdiTabParams.m_bEnableTabSwap = TRUE;
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndToolBar.LoadToolBar(IDR_MAINFRAME_256);

	//CString strToolBarName;
	//strToolBarName.LoadStringW(IDS_TOOLBAR_STANDARD);
	//m_wndToolBar.SetWindowTextW(strToolBarName);

	//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndToolBar);      //CAUSES SLOW SIZING!!!

	// Switch the order of document name and application name on the window title bar. This
	// improves the usability of the taskbar because the document name is visible with the thumbnail.
	ModifyStyle(0, FWS_PREFIXTITLE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIFrameWndEx::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	//Closing Tab with middle mouse button.
	switch (pMsg->message)
	{
	case WM_MBUTTONDOWN:
	{
		CPoint pt = pMsg->pt;
		CWnd* pWnd = WindowFromPoint(pt);
		if (!pWnd)
			break;

		const CObList& tabGroups = GetMDITabGroups();
		if (tabGroups.GetCount() > 0)
		{
			POSITION pos = tabGroups.GetHeadPosition();
			while (pos != NULL)
			{
				CMFCTabCtrl* pTabCtrl = DYNAMIC_DOWNCAST(CMFCTabCtrl, tabGroups.GetNext(pos));
				if (pTabCtrl == pWnd) //Click on TabCtrl.
				{
					pTabCtrl->ScreenToClient(&pt);
					int iTab = pTabCtrl->GetTabFromPoint(pt);
					if (iTab != -1)
					{
						CWnd* pTab = pTabCtrl->GetTabWnd(iTab);
						if (pTab)
							pwndMBtnCurDown = pTab;
						else
							pwndMBtnCurDown = nullptr;
					}
					else
						pwndMBtnCurDown = nullptr;
				}
				else
					pwndMBtnCurDown = nullptr;
			}
		}
	}
	break;
	case WM_MBUTTONUP:
	{
		CPoint pt = pMsg->pt;
		CWnd* pWnd = WindowFromPoint(pt);
		if (!pWnd)
			return TRUE;

		const CObList& tabGroups = m_wndClientArea.GetMDITabGroups();
		if (tabGroups.GetCount() > 0)
		{
			POSITION pos = tabGroups.GetHeadPosition();
			while (pos != NULL)
			{
				CMFCTabCtrl* pTabCtrl = DYNAMIC_DOWNCAST(CMFCTabCtrl, tabGroups.GetNext(pos));
				if (pTabCtrl == pWnd) //Click on TabCtrl.
				{
					pTabCtrl->ScreenToClient(&pt);
					int iTab = pTabCtrl->GetTabFromPoint(pt);
					if (iTab != -1)
					{
						CWnd* pTab = pTabCtrl->GetTabWnd(iTab);
						if (pTab && pTab == pwndMBtnCurDown)
							pTab->SendMessage(WM_CLOSE, 0, 0);
					}
				}
			}
		}
	}
	break;
	}

	return CMDIFrameWndEx::PreTranslateMessage(pMsg);
}

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// base class does the real work
	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
		return FALSE;

	return TRUE;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	return CMDIFrameWndEx::OnCreateClient(lpcs, pContext);
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	PVOID pOldValue;
	Wow64DisableWow64FsRedirection(&pOldValue);
	CMDIFrameWndEx::OnDropFiles(hDropInfo);
	Wow64RevertWow64FsRedirection(pOldValue);
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 400;
	lpMMI->ptMinTrackSize.y = 200;

	CMDIFrameWndEx::OnGetMinMaxInfo(lpMMI);
}

void CMainFrame::OnAppEditmode()
{
	CFrameWnd* pFrame = GetActiveFrame();
	if (pFrame)
	{
		CPepperDoc* pDoc = (CPepperDoc*)pFrame->GetActiveDocument();
		if (pDoc)
		{
			if (pDoc->IsEditMode())
				pDoc->SetEditMode(false);
			else
				pDoc->SetEditMode(true);
		}
	}
}

void CMainFrame::OnUpdateAppEditmode(CCmdUI *pCmdUI)
{
	CFrameWnd* pFrame = GetActiveFrame();
	if (!pFrame)
	{
		pCmdUI->Enable(0);
		return;
	}

	CPepperDoc* pDoc = (CPepperDoc*)pFrame->GetActiveDocument();
	if (pDoc)
	{
		if (pDoc->IsEditMode())
			m_wndToolBar.SetButtonStyle(m_wndToolBar.CommandToIndex(ID_APP_EDITMODE), TBBS_PRESSED);
	}
	else
		pCmdUI->Enable(0);
}