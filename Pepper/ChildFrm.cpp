#include "stdafx.h"
#include "ChildFrm.h"
#include "ViewLeft.h"
#include "ViewRightTop.h"
#include "ViewRightBottom.h"

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIChildWndEx::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	CRect rect;
	::GetClientRect(AfxGetMainWnd()->m_hWnd, &rect);

	m_MainSplitter.CreateStatic(this, 1, 2);
	m_MainSplitter.CreateView(0, 0, RUNTIME_CLASS(CViewLeft), CSize(rect.Width() / 5, rect.Height()), pContext);
	m_RightSplitter.CreateStatic(&m_MainSplitter, 2, 1, WS_CHILD | WS_VISIBLE, m_MainSplitter.IdFromRowCol(0, 1));
	m_RightSplitter.CreateView(0, 0, RUNTIME_CLASS(CViewRightTop), CSize(rect.Width(), rect.Height() / 2), pContext);
	m_RightSplitter.CreateView(1, 0, RUNTIME_CLASS(CViewRightBottom), CSize(rect.Width(), rect.Height() / 2), pContext);

	m_fSpliterCreated = true;

	return TRUE;
}

BOOL CChildFrame::OnEraseBkgnd(CDC* pDC)
{
	return CMDIChildWndEx::OnEraseBkgnd(pDC);
}

void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
	if (m_fSpliterCreated && nType != SIZE_MINIMIZED && cx > 0 && cy > 0)
	{
		if (m_cx > 0 && m_cy > 0)
		{
			double ratio;
			int cxCur, cyCur, min;

			m_MainSplitter.GetColumnInfo(0, cxCur, min);
			ratio = (double)m_cx / cxCur;
			m_MainSplitter.SetColumnInfo(0, int(cx / ratio + 0.5), min);

			m_RightSplitter.GetRowInfo(0, cyCur, min);
			ratio = (double)m_cy / cyCur;
			m_RightSplitter.SetRowInfo(0, int(cy / ratio + 0.5), min);
			m_RightSplitter.GetRowInfo(1, cyCur, min);
			ratio = (double)m_cy / cyCur;
			m_RightSplitter.SetRowInfo(1, int(cy / ratio + 0.5), min);
		}
		else
		{	//If it's the «first» WM_SIZE after CView fully created
			//then set splitter to default state.
			CRect rect;
			GetClientRect(&rect);

			m_MainSplitter.SetColumnInfo(0, rect.Width() / 5, 0);
			m_RightSplitter.SetRowInfo(0, rect.Height() / 2, 0);
			m_RightSplitter.SetRowInfo(1, rect.Height() / 2, 0);
		}
		m_MainSplitter.RecalcLayout();
		m_RightSplitter.RecalcLayout();

		m_cx = cx; m_cy = cy;
	}

	CMDIChildWndEx::OnSize(nType, cx, cy);
}
