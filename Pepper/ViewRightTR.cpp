/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
* This software is available under the "MIT License modified with The Commons Clause".				*
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "ViewRightTR.h"
#include "constants.h"

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

	//Hex control for Resources raw.
	m_hcs.pwndParent = this;
	m_hcs.uId = IDC_HEX_RIGHT_TR;
	m_stHexEdit.Create(m_hcs);
	m_stHexEdit.ShowWindow(SW_HIDE);
}

void CViewRightTR::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
	if (!m_pChildFrame)
		return;
	if (LOWORD(lHint) == IDC_SHOW_RESOURCE_RBR)
		return;

	if (m_pActiveWnd)
		m_pActiveWnd->ShowWindow(SW_HIDE);

	CRect rcParent, rcClient;
	GetParent()->GetWindowRect(&rcParent);
	GetClientRect(&rcClient);

	switch (LOWORD(lHint))
	{
	case IDC_TREE_RESOURCE:
		m_stHexEdit.ClearData();
		m_pActiveWnd = &m_stHexEdit;
		m_pChildFrame->m_stSplitterRightTop.ShowCol(1);
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width() / 3, 0);
		m_stHexEdit.SetWindowPos(this, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	case IDC_HEX_RIGHT_TR:
	{
		const auto& hexData = (std::vector<std::byte>*)pHint;
		HEXDATASTRUCT hds;
		hds.pData = (PBYTE)hexData->data();
		hds.ullDataSize = hexData->size();
		m_stHexEdit.SetData(hds);
		m_pActiveWnd = &m_stHexEdit;
		m_pChildFrame->m_stSplitterRightTop.ShowCol(1);
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width() / 3, 0);
		m_stHexEdit.SetWindowPos(this, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	}
	default:
		m_pChildFrame->m_stSplitterRightTop.HideCol(1);
	}

	m_pChildFrame->m_stSplitterRightTop.RecalcLayout();
}

void CViewRightTR::OnDraw(CDC* pDC)
{
}

void CViewRightTR::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_pActiveWnd)
		m_pActiveWnd->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}