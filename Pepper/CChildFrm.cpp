/*****************************************************************
* Copyright © 2018-present Jovibor https://github.com/jovibor/   *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.  *
* Official git repository: https://github.com/jovibor/Pepper/    *
* This software is available under the Apache-2.0 License.       *
*****************************************************************/
#include "stdafx.h"
#include "CChildFrm.h"
#include "CMainFrm.h"
#include "CViewLeft.h"
#include "CViewRightBL.h"
#include "CViewRightBR.h"
#include "CViewRightTL.h"
#include "CViewRightTR.h"
#include <algorithm>
#include <cmath>

import Utility;

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_WM_MDIACTIVATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	CRect rc;
	::GetClientRect(AfxGetMainWnd()->m_hWnd, &rc);

	m_stSplitterMain.CreateStatic(this, 1, 2);
	m_stSplitterMain.CreateView(0, 0, RUNTIME_CLASS(CViewLeft), CSize(rc.Width() / 5, rc.Height()), pContext);
	m_stSplitterRight.CreateStatic(&m_stSplitterMain, 2, 1, WS_CHILD | WS_VISIBLE, m_stSplitterMain.IdFromRowCol(0, 1));
	m_stSplitterMain.AddNested(0, 1, &m_stSplitterRight);
	m_stSplitterRightTop.CreateStatic(&m_stSplitterRight, 1, 2, WS_CHILD | WS_VISIBLE, m_stSplitterRight.IdFromRowCol(0, 0));
	m_stSplitterRightTop.CreateView(0, 0, RUNTIME_CLASS(CViewRightTL), CSize((rc.Width() - rc.Width() / 5) / 2, rc.Height()), pContext);
	m_stSplitterRightTop.CreateView(0, 1, RUNTIME_CLASS(CViewRightTR), CSize((rc.Width() - rc.Width() / 5) / 2, rc.Height()), pContext);
	m_stSplitterRightTop.HideCol(1);
	m_stSplitterRight.AddNested(0, 0, &m_stSplitterRightTop);
	m_stSplitterRightBottom.CreateStatic(&m_stSplitterRight, 1, 2, WS_CHILD | WS_VISIBLE, m_stSplitterRight.IdFromRowCol(1, 0));
	m_stSplitterRightBottom.CreateView(0, 0, RUNTIME_CLASS(CViewRightBL), CSize(rc.Width(), rc.Height()), pContext);
	m_stSplitterRightBottom.CreateView(0, 1, RUNTIME_CLASS(CViewRightBR), CSize(0, rc.Height() / 2), pContext);
	m_stSplitterRightBottom.HideCol(1);
	m_stSplitterRight.AddNested(1, 0, &m_stSplitterRightBottom);
	m_fSplitterCreated = true;

	++reinterpret_cast<CMainFrame*>(AfxGetMainWnd())->GetChildFramesCount();

	return TRUE;
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	//If tab is closing we don't need to UpdateAllViews.
	//At this moment the Document can already be destroyed in memory, so GetActiveDocument can point to a bad data.
	if (!m_fClosing) {
		GetActiveDocument()->UpdateAllViews(nullptr, bActivate == FALSE ? MSG_MDITAB_DISACTIVATE : MSG_MDITAB_ACTIVATE);
	}
}

void CChildFrame::OnClose()
{
	m_fClosing = true;

	CMDIChildWndEx::OnClose();
}

void CChildFrame::OnDestroy()
{
	CMDIChildWndEx::OnDestroy();

	--reinterpret_cast<CMainFrame*>(AfxGetMainWnd())->GetChildFramesCount();
}

auto CChildFrame::GetSplitRight()->CSplitterEx&
{
	return m_stSplitterRight;
}

auto CChildFrame::GetSplitRightTop()->CSplitterEx&
{
	return m_stSplitterRightTop;
}

auto CChildFrame::GetSplitRightBot()->CSplitterEx&
{
	return m_stSplitterRightBottom;
}

void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
	if (m_fSplitterCreated && nType != SIZE_MINIMIZED && cx > 0 && cy > 0) {
		if (m_cx > 0 && m_cy > 0) {
			//When resizing main window we keep ratio between splitters.
			double ratio;
			int cxCurr, cyCurr, min;

			m_stSplitterMain.GetColumnInfo(0, cxCurr, min);
			ratio = static_cast<double>(m_cx) / cxCurr;
			m_stSplitterMain.SetColumnInfo(0, std::lround(cx / ratio), min);

			m_stSplitterRight.GetRowInfo(0, cyCurr, min);
			ratio = static_cast<double>(m_cy) / cyCurr;
			m_stSplitterRight.SetRowInfo(0, std::lround(cy / ratio), min);
			m_stSplitterRight.GetRowInfo(1, cyCurr, min);
			ratio = static_cast<double>(m_cy) / cyCurr;
			m_stSplitterRight.SetRowInfo(1, std::lround(cy / ratio), min);

			m_stSplitterRightTop.GetColumnInfo(0, cxCurr, min);
			ratio = static_cast<double>(m_cx) / cxCurr;
			m_stSplitterRightTop.SetColumnInfo(0, std::lround(cx / ratio), min);
			m_stSplitterRightTop.GetColumnInfo(1, cyCurr, min);
			ratio = static_cast<double>(m_cx) / cxCurr;
			m_stSplitterRightTop.SetColumnInfo(1, std::lround(cx / ratio), min);

			m_stSplitterRightBottom.GetColumnInfo(0, cxCurr, min);
			ratio = static_cast<double>(m_cx) / cxCurr;
			m_stSplitterRightBottom.SetColumnInfo(0, std::lround(cx / ratio), min);
			m_stSplitterRightBottom.GetColumnInfo(1, cyCurr, min);
			ratio = static_cast<double>(m_cx) / cxCurr;
			m_stSplitterRightBottom.SetColumnInfo(1, std::lround(cx / ratio), min);
		}
		else {
			CRect rect;
			GetClientRect(&rect);

			//If it's the «first» WM_SIZE after CView fully created
			//then set splitter to default state.
			m_stSplitterMain.SetColumnInfo(0, rect.Width() / 5, 0);
			m_stSplitterRight.SetRowInfo(0, rect.Height() / 2, 0);
			m_stSplitterRight.SetRowInfo(1, rect.Height() / 2, 0);
			m_stSplitterRightTop.SetColumnInfo(0, 50, 0);
		}
		m_stSplitterMain.RecalcLayout();
		m_stSplitterRight.RecalcLayout();
		m_stSplitterRightTop.RecalcLayout();

		m_cx = cx; m_cy = cy;
	}

	CMDIChildWndEx::OnSize(nType, cx, cy);
}