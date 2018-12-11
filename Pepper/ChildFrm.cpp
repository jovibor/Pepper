#include "stdafx.h"
#include "ChildFrm.h"
#include "ViewLeft.h"
#include "ViewRightTopLeft.h"
#include "ViewRightBottomLeft.h"
#include "ViewRightTopRight.h"
#include "ViewRightBottomRight.h"

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

	m_stSplitterMain.CreateStatic(this, 1, 2);
	m_stSplitterMain.CreateView(0, 0, RUNTIME_CLASS(CViewLeft), CSize(rect.Width() / 5, rect.Height()), pContext);
	m_stSplitterRight.CreateStatic(&m_stSplitterMain, 2, 1, WS_CHILD | WS_VISIBLE, m_stSplitterMain.IdFromRowCol(0, 1));

	m_stSplitterRightTop.CreateStatic(&m_stSplitterRight, 1, 2, WS_CHILD | WS_VISIBLE, m_stSplitterRight.IdFromRowCol(0, 0));
	m_stSplitterRightTop.CreateView(0, 0, RUNTIME_CLASS(CViewRightTopLeft), CSize(rect.Width(), rect.Height()), pContext);
	m_stSplitterRightTop.CreateView(0, 1, RUNTIME_CLASS(CViewRightTopRight), CSize(0, rect.Height()), pContext);
	m_stSplitterRightTop.HideCol(1);
	m_stSplitterRight.AddNested(0, 0, &m_stSplitterRightTop);

	m_stSplitterRightBottom.CreateStatic(&m_stSplitterRight, 1, 2, WS_CHILD | WS_VISIBLE, m_stSplitterRight.IdFromRowCol(1, 0));
	m_stSplitterRightBottom.CreateView(0, 0, RUNTIME_CLASS(CViewRightBottomLeft), CSize(rect.Width(), rect.Height()), pContext);
	m_stSplitterRightBottom.CreateView(0, 1, RUNTIME_CLASS(CViewRightBottomRight), CSize(0, rect.Height() / 2), pContext);
//	m_stSplitterRightBottom.CreateView(0, 2, RUNTIME_CLASS(CViewRightBottomRight), CSize(0, rect.Height() / 2), pContext);
//	m_stSplitterRightBottom.CreateView(0, 3, RUNTIME_CLASS(CViewRightTopLeft), CSize(0, rect.Height() / 2), pContext);
	 
	m_stSplitterRightBottom.HideCol(1);
	m_stSplitterRight.AddNested(1, 0, &m_stSplitterRightBottom);

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
			//When resizing main window we keep ratio between splitters.
			double ratio;
			int cxCurr, cyCurr, min;

			m_stSplitterMain.GetColumnInfo(0, cxCurr, min);
			ratio = (double)m_cx / cxCurr;
			m_stSplitterMain.SetColumnInfo(0, int(cx / ratio + 0.5), min);

			m_stSplitterRight.GetRowInfo(0, cyCurr, min);
			ratio = (double)m_cy / cyCurr;
			m_stSplitterRight.SetRowInfo(0, int(cy / ratio + 0.5), min);
			m_stSplitterRight.GetRowInfo(1, cyCurr, min);
			ratio = (double)m_cy / cyCurr;
			m_stSplitterRight.SetRowInfo(1, int(cy / ratio + 0.5), min);

			m_stSplitterRightTop.GetColumnInfo(0, cxCurr, min);
			ratio = (double)m_cx / cxCurr;
			m_stSplitterRightTop.SetColumnInfo(0, int(cx / ratio + 0.5), min);
			m_stSplitterRightTop.GetColumnInfo(1, cyCurr, min);
			ratio = (double)m_cx / cxCurr;
			m_stSplitterRightTop.SetColumnInfo(1, int(cx / ratio + 0.5), min);

			m_stSplitterRightBottom.GetColumnInfo(0, cxCurr, min);
			ratio = (double)m_cx / cxCurr;
			m_stSplitterRightBottom.SetColumnInfo(0, int(cx / ratio + 0.5), min);
			m_stSplitterRightBottom.GetColumnInfo(1, cyCurr, min);
			ratio = (double)m_cx / cxCurr;
			m_stSplitterRightBottom.SetColumnInfo(1, int(cx / ratio + 0.5), min);
		}
		else
		{	//If it's the «first» WM_SIZE after CView fully created
			//then set splitter to default state.
			CRect rect;
			GetClientRect(&rect);
			m_stSplitterMain.SetColumnInfo(0, rect.Width() / 5, 0);
			m_stSplitterRight.SetRowInfo(0, rect.Height() / 2, 0);
			m_stSplitterRight.SetRowInfo(1, rect.Height() / 2, 0);
		}
		m_stSplitterMain.RecalcLayout();
		m_stSplitterRight.RecalcLayout();

		m_cx = cx; m_cy = cy;
	}

	CMDIChildWndEx::OnSize(nType, cx, cy);
}