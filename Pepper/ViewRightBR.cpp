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

	m_stEditResStrings.Create(WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL
		| ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, 0x01);

	LOGFONT lf { };
	StringCchCopyW(lf.lfFaceName, 9, L"Consolas");
	lf.lfHeight = 18;
	if (!m_fontEditRes.CreateFontIndirectW(&lf))
	{
		NONCLIENTMETRICSW ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
		m_fontEditRes.CreateFontIndirectW(&ncm.lfMessageFont);
	}
	m_stEditResStrings.SetFont(&m_fontEditRes);

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
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rcParent.Width() / 3, 0);
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
	case IDC_SHOW_RESOURCE_RBR:
		ShowResource((RESHELPER*)pHint);
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		break;
	default:
		m_pChildFrame->m_stSplitterRightBottom.HideCol(1);
	}

	m_pChildFrame->m_stSplitterRightBottom.RecalcLayout();
}

int CViewRightBR::ShowResource(RESHELPER* pRes)
{
	LPGRPICONDIR pGRPIDir;
	HICON hIcon;
	ICONINFO iconInfo;
	if (m_hwndResDlg)
	{
		::DestroyWindow(m_hwndResDlg);
		m_hwndResDlg = nullptr;
	}
	m_stEditResStrings.ShowWindow(SW_HIDE);
	m_stImgRes.DeleteImageList();
	m_iResTypeToDraw = -1;
	m_iImgResWidth = 0;
	m_iImgResHeight = 0;
	m_vecImgRes.clear();
	m_strRes.clear();

	CRect rcClient;
	GetClientRect(&rcClient);

	PCLIBPE_RESOURCE_ROOT_TUP pTupResRoot { };
	if (m_pLibpe->GetResourceTable(pTupResRoot) != S_OK)
		return -1;

	if (pRes)
	{
		switch (pRes->IdResType)
		{
		case 1: //RT_CURSOR
		{
			hIcon = CreateIconFromResourceEx((PBYTE)pRes->pData->data(), pRes->pData->size(), FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
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
			BITMAPINFO* pDIBInfo = (BITMAPINFO*)pRes->pData->data();
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
			hIcon = CreateIconFromResourceEx((PBYTE)pRes->pData->data(), pRes->pData->size(), TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
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
			m_hwndResDlg = CreateDialogIndirectParamW(nullptr, (LPCDLGTEMPLATEW)pRes->pData->data(), m_hWnd, nullptr, NULL);

			if (m_hwndResDlg)
			{
				CRect rc;
				::GetWindowRect(m_hwndResDlg, &rc);
				DWORD dwStyle;
				if (*(((PWORD)pRes->pData->data()) + 1) == 0xFFFF) //DLGTEMPLATEEX
					dwStyle = ((DLGTEMPLATEEX*)pRes->pData->data())->style;
				else //DLGTEMPLATE
					dwStyle = ((DLGTEMPLATE*)pRes->pData->data())->style;

				m_strRes = L"STYLES: ";
				for (auto& i : m_mapDlgStyles)
					if (i.first & dwStyle)
						m_strRes += i.second + L" | ";
				m_strRes = m_strRes.substr(0, m_strRes.size() - 3);

				::ShowWindow(m_hwndResDlg, SW_SHOWNOACTIVATE);
				HDC hDCDlg = ::GetDC(m_hwndResDlg);
				HDC hDCMemory = CreateCompatibleDC(hDCDlg);
				HBITMAP hBitmap = CreateCompatibleBitmap(hDCDlg, rc.Width(), rc.Height());
				::SelectObject(hDCMemory, hBitmap);
				::PrintWindow(m_hwndResDlg, hDCMemory, 0);
				::ShowWindow(m_hwndResDlg, SW_HIDE);
				GetTextExtentPoint32W(hDCDlg, m_strRes.data(), m_strRes.size(), &m_sizeLetter);
				DeleteDC(hDCMemory);
				::ReleaseDC(m_hwndResDlg, hDCDlg);

				CBitmap bmp;
				if (!bmp.Attach(hBitmap))
					return -1;
				m_stImgRes.Create(rc.Width(), rc.Height(), ILC_COLORDDB, 0, 1);
				if (m_stImgRes.Add(&bmp, nullptr) == -1)
					return -1;
				SetScrollSizes(MM_TEXT, CSize(max(m_sizeLetter.cx + m_iResDlgIndentToDrawX, rc.Width()), rc.Height() + m_sizeLetter.cy + m_iResDlgIndentToDrawY));
				bmp.DeleteObject();

				m_iResTypeToDraw = 5;
				m_fDrawRes = true;
				::DestroyWindow(m_hwndResDlg);
			}
			break;
		}
		case 6: //RT_STRING
		{
			LPCWSTR pwszResString = reinterpret_cast<LPCWSTR>(pRes->pData->data());
			std::wstring strTmp;
			for (int i = 0; i < 16; i++)
			{
				m_strRes += strTmp.assign(pwszResString + 1, (UINT)*pwszResString);
				if (i != 15)
					m_strRes += L"\r\n";
				pwszResString += 1 + (UINT)*pwszResString;
			}

			m_stEditResStrings.SetWindowTextW(m_strRes.data());
			m_stEditResStrings.SetWindowPos(this, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

			m_pActiveWnd = &m_stEditResStrings;
			break;
		}
		case 12: //RT_GROUP_CURSOR
		{
			pGRPIDir = (LPGRPICONDIR)pRes->pData->data();

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

										m_vecImgRes.emplace_back(std::make_unique<CImageList>());
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
			pGRPIDir = (LPGRPICONDIR)pRes->pData->data();

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

										m_vecImgRes.emplace_back(std::make_unique<CImageList>());
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
		{
			LANGANDCODEPAGE* pLangAndCP;
			UINT dwBytesOut;

			//Read the list of languages and code pages.
			VerQueryValueW(pRes->pData->data(), L"\\VarFileInfo\\Translation", (LPVOID*)&pLangAndCP, &dwBytesOut);

			WCHAR strSubBlock[50];
			DWORD dwLangCount = dwBytesOut / sizeof(LANGANDCODEPAGE);
			//Read the file description for each language and code page.
			for (unsigned iterCodePage = 0; iterCodePage < dwLangCount; iterCodePage++)
			{
				for (unsigned i = 0; i < m_mapVerInfoStrings.size(); i++) //sizeof pstrVerInfoStrings [];
				{
					swprintf_s(strSubBlock, 50, L"\\StringFileInfo\\%04x%04x\\%s",
						pLangAndCP[iterCodePage].wLanguage, pLangAndCP[iterCodePage].wCodePage, m_mapVerInfoStrings.at(i).data());

					m_strRes += m_mapVerInfoStrings.at(i).data();
					m_strRes += L" - ";

					WCHAR* pszBufferOut;
					if (VerQueryValueW(pRes->pData->data(), strSubBlock, (LPVOID*)&pszBufferOut, &dwBytesOut))
						if (dwBytesOut)
							m_strRes += pszBufferOut;
					m_strRes += L"\r\n";
				}
			}
			m_stEditResStrings.SetWindowTextW(m_strRes.data());
			m_stEditResStrings.SetWindowPos(this, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

			m_pActiveWnd = &m_stEditResStrings;
			break;
		}
		case 241: //RT_TOOLBAR
		{
			auto& rootvec = std::get<1>(*pTupResRoot);
			for (auto& iterRoot : rootvec)
			{
				if (std::get<0>(iterRoot).Id == 2) //RT_BITMAP
				{
					auto& lvl2tup = std::get<4>(iterRoot);
					auto& lvl2vec = std::get<1>(lvl2tup);

					for (auto& iterlvl2 : lvl2vec)
					{
						if (std::get<0>(iterlvl2).Id == pRes->IdResName)
						{
							auto& lvl3tup = std::get<4>(iterlvl2);
							auto& lvl3vec = std::get<1>(lvl3tup);

							if (!lvl3vec.empty())
							{
								auto& data = std::get<3>(lvl3vec.at(0));
								if (!data.empty())
									ShowResource(&RESHELPER(2, pRes->IdResName, (std::vector<std::byte>*)&data));
							}
						}
					}
				}
			}
			break;
		}
		}
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
		pDC->FillSolidRect(rect, RGB(255, 255, 255));
		pDC->SelectObject(&m_fontEditRes);
		pDC->TextOutW(m_iResDlgIndentToDrawX, m_iResDlgIndentToDrawY, m_strRes.data(), m_strRes.size());
		ptDrawAt.SetPoint(m_iResDlgIndentToDrawX, m_sizeLetter.cy + m_iResDlgIndentToDrawY * 3);
		m_stImgRes.Draw(pDC, 0, ptDrawAt, ILD_NORMAL);
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