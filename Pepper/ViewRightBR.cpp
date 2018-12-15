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
	if (LOWORD(lHint) == IDC_HEX_RIGHT_TOP_RIGHT)
		return;

	m_fDrawRes = false;

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
	case IDC_TREE_RESOURCE:
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rect.Width() / 2, 0);
		break;
	case IDC_SHOW_RESOURCE:
		ShowResource((std::vector<std::byte>*)pHint, HIWORD(lHint));
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rect.Width() / 2, 0);
		break;
	default:
		m_pChildFrame->m_stSplitterRightBottom.HideCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rect.Width(), 0);
	}

	m_pChildFrame->m_stSplitterRightBottom.RecalcLayout();
}

int CViewRightBR::ShowResource(std::vector<std::byte>* pData, UINT uResType)
{
	HICON hIcon;
	ICONINFO iconInfo;
	m_imgRes.DeleteImageList();

	switch (uResType)
	{
	case 1: //CURSOR
	{
		hIcon = CreateIconFromResourceEx((PBYTE)pData->data(), pData->size(), FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
		if (!hIcon)
			return -1;
		if (!GetIconInfo(hIcon, &iconInfo))
			return -1;
		if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
			return -1;

		m_imgRes.Create(m_stBmp.bmWidth, m_stBmp.bmWidth, ILC_COLORDDB, 0, 1);
		m_imgRes.SetBkColor(RGB(255, 255, 255));
		if (m_imgRes.Add(hIcon) == -1)
			return -1;

		SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmHeight));

		m_fDrawRes = true;
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		DestroyIcon(hIcon);
		break;
	}
	case 2: //BITMAP
	{
		BITMAPINFO* pDIBInfo = (BITMAPINFO*)pData->data();
		int iColors = pDIBInfo->bmiHeader.biClrUsed ? pDIBInfo->bmiHeader.biClrUsed : 1 << pDIBInfo->bmiHeader.biBitCount;
		LPVOID  pDIBBits;

		if (pDIBInfo->bmiHeader.biBitCount > 8)
			pDIBBits = (LPVOID)((PDWORD)(pDIBInfo->bmiColors + pDIBInfo->bmiHeader.biClrUsed) +
			((pDIBInfo->bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
		else
			pDIBBits = (LPVOID)(pDIBInfo->bmiColors + iColors);

		HDC hDC = ::GetDC(m_hWnd);
		HBITMAP hBitmap = CreateDIBitmap(hDC, &pDIBInfo->bmiHeader, CBM_INIT, pDIBBits, pDIBInfo, DIB_RGB_COLORS);
		if (!hBitmap)
			return -1;
		if (!GetObjectW(hBitmap, sizeof(BITMAP), &m_stBmp))
			return -1;

		CBitmap bmp;
		if (!bmp.Attach(hBitmap))
			return -1;
		m_imgRes.Create(m_stBmp.bmWidth, m_stBmp.bmHeight, ILC_COLORDDB, 0, 1);
		if (m_imgRes.Add(&bmp, nullptr) == -1)
			return -1;
		m_fDrawRes = true;

		SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmHeight));

		bmp.DeleteObject();
		::ReleaseDC(m_hWnd, hDC);
		break;
	}
	case 3: //ICON
	{
		hIcon = CreateIconFromResourceEx((PBYTE)pData->data(), pData->size(), TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
		if (!hIcon)
			return -1;
		if (!GetIconInfo(hIcon, &iconInfo))
			return -1;
		if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
			return -1;
		m_imgRes.Create(m_stBmp.bmWidth, m_stBmp.bmHeight, ILC_COLORDDB, 0, 1);
		m_imgRes.SetBkColor(RGB(255, 255, 255));
		if (m_imgRes.Add(hIcon) == -1)
			return -1;

		SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmHeight));

		m_fDrawRes = true;
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		DestroyIcon(hIcon);
		break;
	}
	case 4: //MENU
		break;
	case 5: //DIALOG
		break;
	case 12: //GROUP_CURSOR
		break;
	case 14: //GROUP_ICON
		break;
	}
	Invalidate();
	UpdateWindow();

	return 1;
}

void CViewRightBR::OnDraw(CDC* pDC)
{
	if (!m_fDrawRes)
		return;

	CRect rect;	
	CPoint ptDrawAt;
	int x, y;

	pDC->GetClipBox(&rect);
	pDC->FillSolidRect(rect, RGB(230, 230, 230));
	GetClientRect(&rect);
	CSize size = GetTotalSize();
	//Draw at center independing of scrolls.
	if (size.cx > rect.Width())
		x = size.cx / 2 - (m_stBmp.bmWidth / 2);
	else
		x = rect.Width() / 2 - (m_stBmp.bmWidth / 2);
	if (size.cy > rect.Height())
		y = size.cy / 2 - (m_stBmp.bmHeight / 2);
	else
		y = rect.Height() / 2 - (m_stBmp.bmHeight / 2);

	ptDrawAt.SetPoint(x, y);

	m_imgRes.Draw(pDC, 0, ptDrawAt, ILD_NORMAL);
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