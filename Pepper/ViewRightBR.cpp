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

	m_stEditResStrings.Create(WS_VISIBLE | WS_CHILD | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, CRect(0, 0, 0, 0), this, 0x01);

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
	if (m_fJustOneTime)
	{
		CRect rcParent;
		GetParent()->GetWindowRect(&rcParent);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rcParent.Width() / 2, 0);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(1, rcParent.Width() / 2, 0);
		m_pChildFrame->m_stSplitterRightBottom.RecalcLayout();
		m_fJustOneTime = false;
	}

	if (m_pActiveWnd)
		m_pActiveWnd->ShowWindow(SW_HIDE);

	m_fDrawRes = false;

	CRect rect;
	GetClientRect(&rect);
	m_pChildFrame->m_stSplitterRight.GetPane(1, 0)->GetClientRect(&rect);

	switch (LOWORD(lHint))
	{
	case IDC_LIST_TLS:
		m_stListTLSCallbacks.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveWnd = &m_stListTLSCallbacks;
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		break;
	case IDC_TREE_RESOURCE:
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		break;
	case IDC_SHOW_RESOURCE:
		ShowResource((std::vector<std::byte>*)pHint, HIWORD(lHint));
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		break;
	default:
		m_pChildFrame->m_stSplitterRightBottom.HideCol(1);
	}

	m_pChildFrame->m_stSplitterRightBottom.RecalcLayout();
}

int CViewRightBR::ShowResource(std::vector<std::byte>* pData, UINT uResType)
{
	LPGRPICONDIR pGRPIDir;
	HICON hIcon;
	ICONINFO iconInfo;
	if (m_hwndResTemplatedDlg)
	{
		::DestroyWindow(m_hwndResTemplatedDlg);
		m_hwndResTemplatedDlg = nullptr;
	}
	m_stEditResStrings.ShowWindow(SW_HIDE);
	m_stImgRes.DeleteImageList();
	m_iResTypeToDraw = -1;
	m_iImgResWidth = 0;
	m_iImgResHeight = 0;
	m_vecImgRes.clear();

	PCLIBPE_RESOURCE_ROOT_TUP pTupResRoot { };
	if (m_pLibpe->GetResourceTable(pTupResRoot) != S_OK)
		return -1;

	switch (uResType)
	{
	case 1: //RT_CURSOR
	{
		hIcon = CreateIconFromResourceEx((PBYTE)pData->data(), pData->size(), FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
		if (!hIcon)
			return -1;
		if (!GetIconInfo(hIcon, &iconInfo))
			return -1;
		if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
			return -1;

		m_stImgRes.Create(m_stBmp.bmWidth, m_stBmp.bmWidth, ILC_COLORDDB, 0, 1);
		m_stImgRes.SetBkColor(m_clrBkImgList);
		if (m_stImgRes.Add(hIcon) == -1)
			return -1;

		SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmWidth));

		m_iResTypeToDraw = 1;
		m_fDrawRes = true;
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		DestroyIcon(hIcon);
		break;
	}
	case 2: //RT_BITMAP
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
		m_stImgRes.Create(m_stBmp.bmWidth, m_stBmp.bmHeight, ILC_COLORDDB, 0, 1);
		if (m_stImgRes.Add(&bmp, nullptr) == -1)
			return -1;

		m_iResTypeToDraw = 2;
		m_fDrawRes = true;
		SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmHeight));
		bmp.DeleteObject();
		::ReleaseDC(m_hWnd, hDC);
		break;
	}
	case 3: //RT_ICON
	{
		hIcon = CreateIconFromResourceEx((PBYTE)pData->data(), pData->size(), TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
		if (!hIcon)
			return -1;
		if (!GetIconInfo(hIcon, &iconInfo))
			return -1;
		if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
			return -1;
		m_stImgRes.Create(m_stBmp.bmWidth, m_stBmp.bmHeight, ILC_COLORDDB, 0, 1);
		m_stImgRes.SetBkColor(m_clrBkImgList);
		if (m_stImgRes.Add(hIcon) == -1)
			return -1;

		SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmHeight));

		m_iResTypeToDraw = 3;
		m_fDrawRes = true;
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		DestroyIcon(hIcon);
		break;
	}
	case 4: //RT_MENU
		break;
	case 5: //RT_DIALOG
	{
		m_hwndResTemplatedDlg = CreateDialogIndirectParamW(nullptr, (LPCDLGTEMPLATEW)pData->data(), m_hWnd, nullptr, NULL);
		if (m_hwndResTemplatedDlg)
		{
			CRect rc;
			GetClientRect(&rc);
			::SetWindowPos(m_hwndResTemplatedDlg, m_hWnd, rc.left + 10, rc.top + 10, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
			m_iResTypeToDraw = 5;
			m_fDrawRes = true;
		}
		break;
	}
	case 6: //RT_STRING
	{
		m_strResStrings.clear();
		LPCWSTR pwszResString = reinterpret_cast<LPCWSTR>(pData->data());
		std::wstring strTmp;
		for (int i = 0; i < 16; i++)
		{
			m_strResStrings += strTmp.assign(pwszResString + 1, (UINT)*pwszResString);
			if (i != 15)
				m_strResStrings += L"\r\n";
			pwszResString += 1 + (UINT)*pwszResString;
		}
		CRect rc;
		GetClientRect(&rc);
		m_stEditResStrings.SetWindowTextW(m_strResStrings.data());
		m_stEditResStrings.SetWindowPos(this, rc.left, rc.top, rc.right, rc.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

		m_pActiveWnd = &m_stEditResStrings;
		break;
	}
	case 12: //RT_GROUP_CURSOR
	{
		pGRPIDir = (LPGRPICONDIR)pData->data();

		for (int i = 0; i < pGRPIDir->idCount; i++)
		{
			auto& rootvec = std::get<1>(*pTupResRoot);
			for (auto& iterRoot : rootvec)
			{
				if (std::get<0>(iterRoot).Id == 1) //RT_CURSOR
				{
					auto& lvl2tup = std::get<4>(iterRoot);
					auto& lvl2vec = std::get<1>(lvl2tup);

					for (auto& iterlvl2 : lvl2vec)
					{
						if (std::get<0>(iterlvl2).Id == pGRPIDir->idEntries[i].nID)
						{
							auto& lvl3tup = std::get<4>(iterlvl2);
							auto& lvl3vec = std::get<1>(lvl3tup);

							if (!lvl3vec.empty())
							{
								auto& data = std::get<3>(lvl3vec.at(0));
								if (!data.empty())
								{
									hIcon = CreateIconFromResourceEx((PBYTE)data.data(), data.size(), FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
									if (!hIcon)
										return -1;
									if (!GetIconInfo(hIcon, &iconInfo))
										return -1;
									if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
										return -1;

									m_vecImgRes.push_back(std::make_unique<CImageList>());
									auto& vecBack = m_vecImgRes.back();
									vecBack->Create(m_stBmp.bmWidth, m_stBmp.bmWidth, ILC_COLORDDB, 0, 1);
									vecBack->SetBkColor(m_clrBkImgList);
									m_iImgResWidth += m_stBmp.bmWidth;
									m_iImgResHeight = max(m_stBmp.bmHeight, m_iImgResHeight);

									if (vecBack->Add(hIcon) == -1)
										return -1;

									DeleteObject(iconInfo.hbmColor);
									DeleteObject(iconInfo.hbmMask);
									DestroyIcon(hIcon);
									break;
								}
							}
						}
					}
				}
			}
		}
		SetScrollSizes(MM_TEXT, CSize(m_iImgResWidth, m_iImgResHeight));

		m_iResTypeToDraw = 12;
		m_fDrawRes = true;
		break;
	}
	case 14: //RT_GROUP_ICON
	{
		pGRPIDir = (LPGRPICONDIR)pData->data();

		for (int i = 0; i < pGRPIDir->idCount; i++)
		{
			auto& rootvec = std::get<1>(*pTupResRoot);
			for (auto& iterRoot : rootvec)
			{
				if (std::get<0>(iterRoot).Id == 3) //RT_ICON
				{
					auto& lvl2tup = std::get<4>(iterRoot);
					auto& lvl2vec = std::get<1>(lvl2tup);

					for (auto& iterlvl2 : lvl2vec)
					{
						if (std::get<0>(iterlvl2).Id == pGRPIDir->idEntries[i].nID)
						{
							auto& lvl3tup = std::get<4>(iterlvl2);
							auto& lvl3vec = std::get<1>(lvl3tup);

							if (!lvl3vec.empty())
							{
								auto& data = std::get<3>(lvl3vec.at(0));
								if (!data.empty())
								{
									hIcon = CreateIconFromResourceEx((PBYTE)data.data(), data.size(), TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
									if (!hIcon)
										return -1;
									if (!GetIconInfo(hIcon, &iconInfo))
										return -1;
									if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
										return -1;

									m_vecImgRes.push_back(std::make_unique<CImageList>());
									auto& vecBack = m_vecImgRes.back();
									vecBack->Create(m_stBmp.bmWidth, m_stBmp.bmHeight, ILC_COLORDDB, 0, 1);
									vecBack->SetBkColor(m_clrBkImgList);
									m_iImgResWidth += m_stBmp.bmWidth;
									m_iImgResHeight = max(m_stBmp.bmHeight, m_iImgResHeight);

									if (vecBack->Add(hIcon) == -1)
										return -1;

									DeleteObject(iconInfo.hbmColor);
									DeleteObject(iconInfo.hbmMask);
									DestroyIcon(hIcon);
									break;
								}
							}
						}
					}
				}
			}
		}
		SetScrollSizes(MM_TEXT, CSize(m_iImgResWidth, m_iImgResHeight));

		m_iResTypeToDraw = 14;
		m_fDrawRes = true;
		break;
	}
	case 16: //RT_VERSION
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
	GetClientRect(&rect);
	CSize sizeScroll = GetTotalSize();
	CPoint ptDrawAt;
	int x, y;

	pDC->GetClipBox(&rect);
	pDC->FillSolidRect(rect, RGB(230, 230, 230));

	switch (m_iResTypeToDraw)
	{
	case 1: //RT_CURSOR
	case 2: //RT_BITMAP
	case 3: //RT_ICON
		//Draw at center independing of scrolls.
		if (sizeScroll.cx > rect.Width())
			x = sizeScroll.cx / 2 - (m_stBmp.bmWidth / 2);
		else
			x = rect.Width() / 2 - (m_stBmp.bmWidth / 2);
		if (sizeScroll.cy > rect.Height())
			y = sizeScroll.cy / 2 - (m_stBmp.bmHeight / 2);
		else
			y = rect.Height() / 2 - (m_stBmp.bmHeight / 2);

		ptDrawAt.SetPoint(x, y);
		m_stImgRes.Draw(pDC, 0, ptDrawAt, ILD_NORMAL);

		break;
	case 5: //RT_DIALOG
		break;
	case 12: //RT_GROUP_CURSOR
	case 14: //RT_GROUP_ICON
		if (sizeScroll.cx > rect.Width())
			x = sizeScroll.cx / 2 - (m_iImgResWidth / 2);
		else
			x = rect.Width() / 2 - (m_iImgResWidth / 2);

		for (int i = 0; i < (int)m_vecImgRes.size(); i++)
		{
			IMAGEINFO imginfo;
			m_vecImgRes.at(i)->GetImageInfo(0, &imginfo);
			int iImgHeight = imginfo.rcImage.bottom - imginfo.rcImage.top;
			if (sizeScroll.cy > rect.Height())
				y = sizeScroll.cy / 2 - (iImgHeight / 2);
			else
				y = rect.Height() / 2 - (iImgHeight / 2);

			ptDrawAt.SetPoint(x, y);
			m_vecImgRes.at(i)->Draw(pDC, 0, ptDrawAt, ILD_NORMAL);
			x += imginfo.rcImage.right - imginfo.rcImage.left;
		}
		break;
	}
}

void CViewRightBR::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveWnd)
		m_pActiveWnd->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
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