/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "ChildFrm.h"
#include "MainFrm.h"
#include "ViewLeft.h"
#include "ViewRightBL.h"
#include "ViewRightBR.h"
#include "ViewRightTL.h"
#include "ViewRightTR.h"
#include <algorithm>
#include <cmath>

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIChildWndEx::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

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

	auto pMainFrm = reinterpret_cast<CMainFrame*>(AfxGetMainWnd());
	pMainFrm->GetChildFramesCount()++;

	return TRUE;
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	if (bActivate == FALSE)
	{
		//Every child window that must be closed on tab change goes here.
	}
}

void CChildFrame::OnClose()
{
	auto pMainFrm = reinterpret_cast<CMainFrame*>(AfxGetMainWnd());
	--pMainFrm->GetChildFramesCount();
	pMainFrm->SetCurrFramePtrNull();
	m_vecWndStatus.clear();

	CMDIChildWndEx::OnClose();
}

BOOL CChildFrame::OnEraseBkgnd(CDC* pDC)
{
	return CMDIChildWndEx::OnEraseBkgnd(pDC);
}

auto CChildFrame::GetWndStatData() -> std::vector<SWINDOWSTATUS>&
{
	return m_vecWndStatus;
}

void CChildFrame::SetWindowStatus(HWND hWnd, bool fVisible)
{
	if (auto iter = std::find_if(m_vecWndStatus.begin(), m_vecWndStatus.end(),
		[hWnd](const SWINDOWSTATUS& ref) {return ref.hWnd == hWnd; }); iter != m_vecWndStatus.end())
		iter->fVisible = fVisible;
	else
		m_vecWndStatus.emplace_back(SWINDOWSTATUS { hWnd, fVisible });
}

void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
	if (m_fSplitterCreated && nType != SIZE_MINIMIZED && cx > 0 && cy > 0)
	{
		if (m_cx > 0 && m_cy > 0)
		{
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
		else
		{
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
