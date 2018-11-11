#include "stdafx.h"
#include "ViewRightTopRight.h"

IMPLEMENT_DYNCREATE(CViewRightTopRight, CScrollView)

CViewRightTopRight::CViewRightTopRight()
{
}

CViewRightTopRight::~CViewRightTopRight()
{
}

BEGIN_MESSAGE_MAP(CViewRightTopRight, CScrollView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CViewRightTopRight::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();
	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	if (!m_pLibpe)
		return;

	//Hex control for Resources raw.
	m_stHexEdit.Create(this, CRect(0, 0, 0, 0), IDC_HEX_RESOURCES_RAW);
	m_stHexEdit.ShowWindow(SW_HIDE);
}

void CViewRightTopRight::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
	if (!m_pChildFrame)
		return;

	if (m_pActiveList)
		m_pActiveList->ShowWindow(SW_HIDE);

	CRect rcClient, rcParent;
	GetClientRect(&rcClient);
	m_pChildFrame->m_stSplitterRight.GetPane(0, 0)->GetClientRect(&rcParent);

	switch (LOWORD(lHint))
	{
	case IDC_TREE_RESOURCE:
		m_stHexEdit.SetData(0);
		m_stHexEdit.SetWindowPos(this, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_stHexEdit;
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width() / 2, 0);
		break;
	case IDC_HEX_RESOURCES_RAW:
		m_stHexEdit.SetData((std::vector<std::byte>*)pHint);
		m_stHexEdit.SetWindowPos(this, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_stHexEdit;
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width() / 2, 0);
		break;
	default:
		m_pChildFrame->m_stSplitterRightTop.SetColumnInfo(0, rcParent.Width(), 0);
	}

	m_pChildFrame->m_stSplitterRightTop.RecalcLayout();
}

void CViewRightTopRight::OnDraw(CDC* pDC)
{
}

void CViewRightTopRight::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveList)
		m_pActiveList->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}