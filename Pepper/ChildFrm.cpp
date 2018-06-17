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

	m_MainSplitter.CreateStatic(this, 1, 2, WS_CHILD | WS_VISIBLE);
	m_MainSplitter.CreateView(0, 0, RUNTIME_CLASS(CViewLeft), CSize(0, 0), pContext);
	m_RightSplitter.CreateStatic(&m_MainSplitter, 2, 1, WS_CHILD | WS_VISIBLE, m_MainSplitter.IdFromRowCol(0, 1));
	m_RightSplitter.CreateView(0, 0, RUNTIME_CLASS(CViewRightTop), CSize(0, 0), pContext);
	m_RightSplitter.CreateView(1, 0, RUNTIME_CLASS(CViewRightBottom), CSize(0, 0), pContext);

	m_fSpliterCreated = true;

	return TRUE;
}

BOOL CChildFrame::OnEraseBkgnd(CDC* pDC)
{
	return CMDIChildWndEx::OnEraseBkgnd(pDC);
}

void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED && cx > 0 && cy > 0)
	{
		if (m_fSpliterCreated)
		{
			if (m_cx > 0 && m_cy > 0)
			{
				int cxCur, min;
				m_MainSplitter.GetColumnInfo(0, cxCur, min);
				double ratio = (double)m_cx / cxCur;
				m_MainSplitter.SetColumnInfo(0, cx / ratio + 0.5, min);

				m_RightSplitter.GetRowInfo(0, cxCur, min);
				ratio = (double)m_cy / cxCur;
				m_RightSplitter.SetRowInfo(0, cy / ratio + 0.5, min);
			}
			else
			{
				CRect rect;
				::GetClientRect(GetParent()->m_hWnd, &rect);

				m_MainSplitter.SetColumnInfo(0, rect.Width() / 5, 0);
				m_RightSplitter.SetRowInfo(0, rect.Height() / 2, 0);
			}

			m_cx = cx; m_cy = cy;
		}
	}

	CMDIChildWndEx::OnSize(nType, cx, cy);
}
