#include "stdafx.h"
#include "ViewRightTR.h"

IMPLEMENT_DYNCREATE(CViewRightTR, CScrollView)

BEGIN_MESSAGE_MAP(CViewRightTR, CScrollView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CViewRightTR::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPCHILDREN;

	return CScrollView::PreCreateWindow(cs);
}

void CViewRightTR::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();
	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;

	//Hex control for Resources raw.
	m_stHexEdit.Create(this, IDC_HEX_RIGHT_TR);
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
		m_stHexEdit.SetData((PBYTE)hexData->data(), hexData->size());
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
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveWnd)
		m_pActiveWnd->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}