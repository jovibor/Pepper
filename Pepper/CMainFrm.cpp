/****************************************************************************************************
* Copyright © 2018-2022 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#include "stdafx.h"
#include "CChildFrm.h"
#include "CMainFrm.h"
#include "CPepper.h"
#include "CPepperDoc.h"
#include "res/resource.h"

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_DROPFILES()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(IDM_EDIT_EDITMODE, &CMainFrame::OnAppEditmode)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, &CMainFrame::OnTabActivate)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_EDITMODE, &CMainFrame::OnUpdateAppEditmode)
END_MESSAGE_MAP()

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// base class does the real work
	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
		return FALSE;

	return TRUE;
}

int& CMainFrame::GetChildFramesCount()
{
	return m_iChildFrames;
}

void CMainFrame::SetCurrFramePtrNull()
{
	m_pCurrFrameData = nullptr;
}

void CMainFrame::OnAppEditmode()
{
	if (const auto pFrame = GetActiveFrame(); pFrame != nullptr) {
		if (const auto pDoc = reinterpret_cast<CPepperDoc*>(pFrame->GetActiveDocument()); pDoc != nullptr)
			pDoc->SetEditMode(!pDoc->IsEditMode());
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
	mdiTabParams.m_bTabIcons = TRUE;    // set to TRUE to enable document icons on MDI taba
	mdiTabParams.m_bAutoColor = FALSE;    // set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = FALSE; // enable the document menu at the right edge of the tab area
	mdiTabParams.m_bFlatFrame = TRUE;
	mdiTabParams.m_bEnableTabSwap = TRUE;
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
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

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	//Default "MdiClient" class doesn't have CS_DBLCLKS flag set.
	//We acquire class info, unregister it, set the CS_DBLCLKS flag and then register again.
	//Now we will recieve WM_LBUTTONDBLCLK messages for m_hWndMDIClient window.
	WNDCLASSEXW wndClass { };
	if (!::GetClassInfoExW(AfxGetInstanceHandle(), L"MdiClient", &wndClass))
	{
		MessageBoxW(L"GetClassInfo(MdiClient) failed");
		return FALSE;
	}
	UnregisterClassW(L"MdiClient", AfxGetInstanceHandle());
	wndClass.cbSize = sizeof(WNDCLASSEXW);
	wndClass.style |= CS_DBLCLKS;
	RegisterClassExW(&wndClass);

	if (CMDIFrameWndEx::OnCreateClient(lpcs, pContext) == FALSE)
		return FALSE;

	SetWindowSubclass(m_hWndMDIClient, MDIClientProc, 1, reinterpret_cast<DWORD_PTR>(this));

	return TRUE;
}

void CMainFrame::OnClose()
{
	m_fClosing = true;

	CMDIFrameWndEx::OnClose();
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	PVOID pOldValue;
	Wow64DisableWow64FsRedirection(&pOldValue);
	CMDIFrameWndEx::OnDropFiles(hDropInfo);
	Wow64RevertWow64FsRedirection(pOldValue);
}

BOOL CMainFrame::OnEraseMDIClientBackground(CDC* /*pDC*/)
{
	return TRUE;
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 400;
	lpMMI->ptMinTrackSize.y = 200;

	CMDIFrameWndEx::OnGetMinMaxInfo(lpMMI);
}

LRESULT CMainFrame::OnTabActivate(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_fClosing || GetChildFramesCount() == 0)
		return S_OK;

	if (m_pCurrFrameData != nullptr)
	{
		for (const auto& iter : *m_pCurrFrameData) {
			if (::IsWindow(iter.hWnd) && ::IsWindowVisible(iter.hWnd))
				::ShowWindow(iter.hWnd, SW_HIDE);
		}
	}

	if (const auto pFrame = reinterpret_cast<CChildFrame*>(MDIGetActive()); pFrame != nullptr)
	{
		auto& refVec = pFrame->GetWndStatData();
		for (const auto& iter : refVec) {
			if (::IsWindow(iter.hWnd) && iter.fVisible)
				::ShowWindow(iter.hWnd, SW_SHOW);
		}
		m_pCurrFrameData = &refVec;
	}

	return S_OK;
}

void CMainFrame::OnUpdateAppEditmode(CCmdUI* pCmdUI)
{
	const auto pFrame = GetActiveFrame();
	if (pFrame == nullptr) {
		pCmdUI->Enable(0);
		return;
	}

	const auto pDoc = reinterpret_cast<CPepperDoc*>(pFrame->GetActiveDocument());
	if (pDoc == nullptr) {
		pCmdUI->Enable(0);
		return;
	}

	pCmdUI->SetCheck(pDoc->IsEditMode());
}

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIFrameWndEx::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_MBUTTONDOWN: //Closing tabs with middle mouse button.
	{
		pWndMBtnCurrDown = nullptr;
		CPoint pt = pMsg->pt;
		auto pWnd = WindowFromPoint(pt);
		if (pWnd == nullptr)
			break;

		const auto& tabGroups = GetMDITabGroups();
		if (tabGroups.GetCount() <= 0)
			break;

		auto pos = tabGroups.GetHeadPosition();
		while (pos != nullptr)
		{
			if (auto pTabCtrl = DYNAMIC_DOWNCAST(CMFCTabCtrl, tabGroups.GetNext(pos)); pTabCtrl == pWnd) //Click on TabCtrl.
			{
				pTabCtrl->ScreenToClient(&pt);
				if (int iTab = pTabCtrl->GetTabFromPoint(pt); iTab != -1)
				{
					if (auto pTab = pTabCtrl->GetTabWnd(iTab); pTab != nullptr)
						pWndMBtnCurrDown = pTab;
					break;
				}
			}
		}
	}
	break;
	case WM_MBUTTONUP:
	{
		CPoint pt = pMsg->pt;
		auto pWnd = WindowFromPoint(pt);
		if (pWnd == nullptr)
			break;

		const auto& tabGroups = m_wndClientArea.GetMDITabGroups();
		if (tabGroups.GetCount() <= 0)
			break;

		auto pos = tabGroups.GetHeadPosition();
		while (pos != nullptr)
		{
			if (auto pTabCtrl = DYNAMIC_DOWNCAST(CMFCTabCtrl, tabGroups.GetNext(pos)); pTabCtrl == pWnd) //Click on TabCtrl.
			{
				pTabCtrl->ScreenToClient(&pt);
				if (const auto iTab = pTabCtrl->GetTabFromPoint(pt); iTab != -1) {
					if (const auto pTab = pTabCtrl->GetTabWnd(iTab); pTab != nullptr && pTab == pWndMBtnCurrDown)
						pTab->SendMessageW(WM_CLOSE);
				}
			}
		}
	}
	break;
	}

	return CMDIFrameWndEx::PreTranslateMessage(pMsg);
}

LRESULT CMainFrame::MDIClientProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwData)
{
	const auto pMainFrame = reinterpret_cast<CMainFrame*>(dwData);
	if (pMainFrame->GetChildFramesCount() != 0)
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_PAINT:
	{
		CPaintDC dc(FromHandle(hWnd));
		CRect rc;
		::GetClientRect(hWnd, rc);
		CMemDC dcMem(dc, rc);
		auto pDC = &dcMem.GetDC();
		constexpr auto iLengthText = static_cast<int>(std::size(WSTR_PEPPER_PRODUCT_NAME)) - 1;

		pDC->FillSolidRect(rc, RGB(190, 190, 190));
		pDC->SelectObject(m_fontMDIClient);
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(205, 205, 205)); //Shadow color.
		pDC->DrawTextW(WSTR_PEPPER_PRODUCT_NAME, iLengthText, rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		pDC->SetTextColor(RGB(193, 193, 193)); //Text color.
		rc.OffsetRect(-3, 2);
		pDC->DrawTextW(WSTR_PEPPER_PRODUCT_NAME, iLengthText, rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	}
	break;
	case WM_SIZE:
		MDIClientSize(hWnd, wParam, lParam);
		break;
	case WM_LBUTTONDBLCLK:
		static_cast<CPepperApp*>(AfxGetApp())->OpenNewFile();
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void CMainFrame::MDIClientSize(HWND hWnd, WPARAM /*wParam*/, LPARAM lParam)
{
	constexpr auto iLengthText = static_cast<int>(std::size(WSTR_PEPPER_PRODUCT_NAME)) - 1;
	const auto pDC = CDC::FromHandle(::GetDC(hWnd));
	const auto iWidthNew = LOWORD(lParam);
	const auto iLOGPIXELSY = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY);
	auto iFontSizeMin = 10;
	LOGFONTW lf { .lfHeight { -MulDiv(iFontSizeMin, iLOGPIXELSY, 72) }, .lfPitchAndFamily { FIXED_PITCH }, .lfFaceName { L"Consolas" } };

	m_fontMDIClient.DeleteObject();
	m_fontMDIClient.CreateFontIndirectW(&lf);
	CSize stSizeText { };
	while (stSizeText.cx < (iWidthNew - 20)) //Until the text size not big enough to fill the window's width.
	{
		m_fontMDIClient.DeleteObject();
		lf.lfHeight = -MulDiv(++iFontSizeMin, iLOGPIXELSY, 72);
		m_fontMDIClient.CreateFontIndirectW(&lf);
		pDC->SelectObject(m_fontMDIClient);
		stSizeText = pDC->GetTextExtent(WSTR_PEPPER_PRODUCT_NAME, iLengthText);
	}
	::ReleaseDC(hWnd, pDC->m_hDC);
	::RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}