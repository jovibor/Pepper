/****************************************************************************************************
* Copyright © 2018-2024 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#include "stdafx.h"
#include "CViewRightTR.h"

import Utility;
using namespace Utility;

IMPLEMENT_DYNCREATE(CViewRightTR, CView)

BEGIN_MESSAGE_MAP(CViewRightTR, CView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CViewRightTR::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPCHILDREN;

	return CView::PreCreateWindow(cs);
}

void CViewRightTR::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_pChildFrame = static_cast<CChildFrame*>(GetParentFrame());
	m_pMainDoc = static_cast<CPepperDoc*>(GetDocument());
	m_pFileLoader = &m_pMainDoc->GetFileLoader();

	//Hex control for Resources raw.
	m_hcs.hWndParent = m_hWnd;
	m_hcs.uID = IDC_HEX_RIGHT_TR;
	m_hcs.dwStyle = WS_CHILD;
	m_stHexEdit->Create(m_hcs);
}

void CViewRightTR::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
	const auto iMsg = LOWORD(lHint);
	if (iMsg == MSG_MDITAB_ACTIVATE || iMsg == MSG_MDITAB_DISACTIVATE) {
		OnMDITabActivate(iMsg == MSG_MDITAB_ACTIVATE);
		return; //No further handling if it's tab Activate/Disactivate messages.
	}

	if (!m_pChildFrame || iMsg == IDC_SHOW_RESOURCE_RBR)
		return;

	CRect rcParent;
	GetParent()->GetWindowRect(&rcParent);
	CRect rcClient;
	GetClientRect(&rcClient);

	switch (iMsg) {
	case IDC_TREE_RESOURCE:
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_stHexEdit->ClearData();
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
		m_pChildFrame->GetSplitRightTop().ShowCol(1);
		m_pChildFrame->GetSplitRightTop().SetColumnInfo(0, rcParent.Width() / 3, 0);
		::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	case IDC_HEX_RIGHT_TR:
		CreateHexResources(reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY>(pHint));
		m_pChildFrame->GetSplitRightTop().ShowCol(1);
		m_pChildFrame->GetSplitRightTop().SetColumnInfo(0, rcParent.Width() / 3, 0);
		break;
	case ID_DOC_EDITMODE:
		OnDocEditMode();
		break;
	default:
		if (m_hwndActive) {
			::ShowWindow(m_hwndActive, SW_HIDE);
		}
		m_pChildFrame->GetSplitRightTop().HideCol(1);
		break;
	}

	m_pChildFrame->GetSplitRightTop().RecalcLayout();
}

void CViewRightTR::OnDraw(CDC* /*pDC*/)
{
}

void CViewRightTR::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_hwndActive)
		::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightTR::OnDocEditMode()
{
	if (m_stHexEdit->IsDataSet())
		m_stHexEdit->SetMutable(m_pMainDoc->IsEditMode());
}

void CViewRightTR::OnMDITabActivate(bool fActivate)
{
	if (fActivate) { //Show all windows from the vector, when tab is activated.
		for (const auto hWnd : m_vecHWNDVisible) {
			if (::IsWindow(hWnd)) {
				::ShowWindow(hWnd, SW_SHOW);
			}
		}
		m_vecHWNDVisible.clear();
	}
	else { //Hide all opened HexCtrl dialog windows and add them to the vector, when tab is deactivated.
		for (const auto eWnd : g_arrHexDlgs) {
			const auto hWnd = m_stHexEdit->GetWndHandle(eWnd, false);
			if (::IsWindow(hWnd) && ::IsWindowVisible(hWnd)) {
				m_vecHWNDVisible.emplace_back(hWnd);
				::ShowWindow(hWnd, SW_HIDE);
			}
		}
	}
}

void CViewRightTR::CreateHexResources(const IMAGE_RESOURCE_DATA_ENTRY* pRes)
{
	if (m_hwndActive)
		::ShowWindow(m_hwndActive, SW_HIDE);

	//Resource offset must be higher than zero, otherwise it's somehow damaged.
	if (const auto dwOffset = m_pMainDoc->GetOffsetFromRVA(pRes->OffsetToData); dwOffset != 0) {
		m_pFileLoader->ShowFilePiece(dwOffset, pRes->Size, m_stHexEdit.get());
	}
	else {
		m_stHexEdit->ClearData(); //In case of empty resource just clear the data.
	}

	m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	CRect rcClient;
	GetClientRect(&rcClient);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
}