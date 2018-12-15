#include "stdafx.h"
#include "ViewRightTR.h"

IMPLEMENT_DYNCREATE(CViewRightTR, CScrollView)

BEGIN_MESSAGE_MAP(CViewRightTR, CScrollView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CViewRightTR::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();
	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	if (!m_pLibpe)
		return;

	//Hex control for Resources raw.
	m_stHexEdit.Create(this, CRect(0, 0, 0, 0), IDC_HEX_RIGHT_TOP_RIGHT);
	m_stHexEdit.ShowWindow(SW_HIDE);
}

void CViewRightTR::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
	if (!m_pChildFrame)
		return;
	if (LOWORD(lHint) == IDC_SHOW_RESOURCE)
		return;

	if (m_pActiveList)
		m_pActiveList->ShowWindow(SW_HIDE);

	CRect rcClient, rcParent;
	GetClientRect(&rcClient);
	m_pChildFrame->m_stSplitterRight.GetPane(0, 0)->GetClientRect(&rcParent);

	switch (LOWORD(lHint))
	{
	case IDC_TREE_RESOURCE:
		m_stHexEdit.ClearData();
		m_stHexEdit.SetWindowPos(this, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_stHexEdit;
		m_pChildFrame->m_stSplitterRightTop.ShowCol(1);
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width() / 2, 0);
		break;
	case IDC_HEX_RIGHT_TOP_RIGHT:
		m_stHexEdit.SetData(*(std::vector<std::byte>*)pHint);
		m_stHexEdit.SetWindowPos(this, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_stHexEdit;
		m_pChildFrame->m_stSplitterRightTop.ShowCol(1);
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width() / 2, 0);
		break;
	default:
		m_pChildFrame->m_stSplitterRightTop.HideCol(1);
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width(), 0);
	}
	m_pChildFrame->m_stSplitterRightTop.RecalcLayout();
}

void CViewRightTR::OnDraw(CDC* pDC)
{
}

void CViewRightTR::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveList)
		m_pActiveList->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}