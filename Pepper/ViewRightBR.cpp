#include "stdafx.h"
#include "ViewRightBR.h"

IMPLEMENT_DYNCREATE(CViewRightBR, CScrollView)

BEGIN_MESSAGE_MAP(CViewRightBR, CScrollView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CViewRightBR::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();

	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	if (!m_pLibpe)
		return;

	m_stListInfo.clrListTextTooltip = RGB(255, 255, 255);
	m_stListInfo.clrListBkTooltip = RGB(0, 132, 132);
	m_stListInfo.clrHeaderText = RGB(255, 255, 255);
	m_stListInfo.clrHeaderBk = RGB(0, 132, 132);
	m_stListInfo.dwHeaderHeight = 35;

	m_lf.lfHeight = 16;
	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stListInfo.pListLogFont = &m_lf;
	m_hdrlf.lfHeight = 17;
	m_hdrlf.lfWeight = FW_BOLD;
	StringCchCopyW(m_hdrlf.lfFaceName, 16, L"Times New Roman");
	m_stListInfo.pHeaderLogFont = &m_hdrlf;

	CreateListTLSCallbacks();
}

void CViewRightBR::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_pChildFrame)
		return;

	if (m_pActiveList)
		m_pActiveList->ShowWindow(SW_HIDE);

	CRect rect;
	GetClientRect(&rect);
	m_pChildFrame->m_stSplitterRight.GetPane(1, 0)->GetClientRect(&rect);

	switch (LOWORD(lHint))
	{
	case IDC_LIST_TLS:
		m_stListTLSCallbacks.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_stListTLSCallbacks;
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rect.Width() / 2, 0);
		break;
	default:
		m_pChildFrame->m_stSplitterRightBottom.HideCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rect.Width(), 0);
	}

	m_pChildFrame->m_stSplitterRightBottom.RecalcLayout();
}

void CViewRightBR::OnDraw(CDC* pDC)
{
}

void CViewRightBR::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveList)
		m_pActiveList->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

int CViewRightBR::CreateListTLSCallbacks()
{
	PCLIBPE_TLS_TUP pTLS { };
	if (m_pLibpe->GetTLSTable(pTLS) != S_OK)
		return -1;

	m_stListTLSCallbacks.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_TLS_CALLBACKS, &m_stListInfo);
	m_stListTLSCallbacks.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_stListTLSCallbacks.InsertColumn(0, L"TLS Callbacks", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 300);

	int listindex { };
	WCHAR str[9] { };

	for (auto& iterCallbacks : std::get<2>(*pTLS))
	{
		swprintf_s(str, 9, L"%08X", iterCallbacks);
		m_stListTLSCallbacks.InsertItem(listindex, str);
		listindex++;
	}

	return 0;
}