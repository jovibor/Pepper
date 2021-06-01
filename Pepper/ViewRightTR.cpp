/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "ViewRightTR.h"
#include "Utility.h"

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
	m_pLibpe = m_pMainDoc->m_pLibpe;
	m_pFileLoader = &m_pMainDoc->m_stFileLoader;

	//Hex control for Resources raw.
	m_hcs.hwndParent = m_hWnd;
	m_hcs.uID = IDC_HEX_RIGHT_TR;
	if (m_stHexEdit->Create(m_hcs))
		::ShowWindow(m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN), SW_HIDE);
}

void CViewRightTR::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
	if (!m_pChildFrame || LOWORD(lHint) == IDC_SHOW_RESOURCE_RBR)
		return;

	CRect rcParent, rcClient;
	GetParent()->GetWindowRect(&rcParent);
	GetClientRect(&rcClient);

	switch (LOWORD(lHint))
	{
	case IDC_TREE_RESOURCE:
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_stHexEdit->ClearData();
		m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
		m_pChildFrame->m_stSplitterRightTop.ShowCol(1);
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width() / 3, 0);
		::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	case IDC_HEX_RIGHT_TR:
		CreateHexResources(reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY>(pHint));
		m_pChildFrame->m_stSplitterRightTop.ShowCol(1);
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width() / 3, 0);
		break;
	case ID_DOC_EDITMODE:
		OnDocEditMode();
		break;
	default:
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);

		m_pChildFrame->m_stSplitterRightTop.HideCol(1);
	}

	m_pChildFrame->m_stSplitterRightTop.RecalcLayout();
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

void CViewRightTR::CreateHexResources(const IMAGE_RESOURCE_DATA_ENTRY* pRes)
{
	CRect rcParent, rcClient;
	GetParent()->GetWindowRect(&rcParent);
	GetClientRect(&rcClient);

	if (m_hwndActive)
		::ShowWindow(m_hwndActive, SW_HIDE);

	DWORD dwOffset { };
	m_pLibpe->GetOffsetFromRVA(pRes->OffsetToData, dwOffset);

	m_pFileLoader->ShowFilePiece(dwOffset, pRes->Size, m_stHexEdit.get());
	m_hwndActive = m_stHexEdit->GetWindowHandle(EHexWnd::WND_MAIN);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
}