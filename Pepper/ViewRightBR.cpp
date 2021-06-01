/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#include "stdafx.h"
#include "MainFrm.h"
#include "ViewRightBR.h"
#include "Utility.h"

BEGIN_MESSAGE_MAP(CWndSampledlg, CWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CWndSampledlg::Attach(CImageList* pImgList, CChildFrame* pChildFrame)
{
	m_pImgRes = pImgList;
	m_pChildFrame = pChildFrame;
}

void CWndSampledlg::SetDlgVisible(bool fVisible)
{
	if (m_pChildFrame == nullptr)
		return;

	ShowWindow(fVisible ? SW_SHOW : SW_HIDE);
	m_pChildFrame->SetWindowStatus(m_hWnd, fVisible);
}

void CWndSampledlg::OnPaint()
{
	CPaintDC dc(this);
	if (m_pImgRes)
		m_pImgRes->Draw(&dc, 0, POINT { 0, 0 }, ILD_NORMAL);
}

void CWndSampledlg::OnClose()
{
	SetDlgVisible(false);
	CWnd::OnClose();
}


IMPLEMENT_DYNCREATE(CViewRightBR, CScrollView)

BEGIN_MESSAGE_MAP(CViewRightBR, CScrollView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CViewRightBR::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	SetScrollSizes(MM_TEXT, CSize(0, 0));

	m_pChildFrame = static_cast<CChildFrame*>(GetParentFrame());
	m_pMainDoc = static_cast<CPepperDoc*>(GetDocument());
	m_pLibpe = m_pMainDoc->m_pLibpe;

	m_EditBRB.Create(WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL
		| ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, 0x01);

	m_wndSampledlg.Attach(&m_stImgRes, m_pChildFrame);

	LOGFONTW lf { };
	StringCchCopyW(lf.lfFaceName, 9, L"Consolas");
	lf.lfHeight = 18;
	if (!m_fontEditRes.CreateFontIndirectW(&lf))
	{
		NONCLIENTMETRICSW ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
		m_fontEditRes.CreateFontIndirectW(&ncm.lfMessageFont);
	}
	m_EditBRB.SetFont(&m_fontEditRes);

	m_stlcs.stColor.clrTooltipText = RGB(255, 255, 255);
	m_stlcs.stColor.clrTooltipBk = RGB(0, 132, 132);
	m_stlcs.stColor.clrHdrText = RGB(255, 255, 255);
	m_stlcs.stColor.clrHdrBk = RGB(0, 132, 132);
	m_stlcs.stColor.clrHdrHglInact = RGB(0, 112, 112);
	m_stlcs.stColor.clrHdrHglAct = RGB(0, 92, 92);
	m_stlcs.pParent = this;
	m_stlcs.dwHdrHeight = 35;

	m_lf.lfHeight = 16;
	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stlcs.pListLogFont = &m_lf;
	m_hdrlf.lfHeight = 17;
	m_hdrlf.lfWeight = FW_BOLD;
	StringCchCopyW(m_hdrlf.lfFaceName, 16, L"Times New Roman");
	m_stlcs.pHdrLogFont = &m_hdrlf;

	CreateListTLSCallbacks();
}

void CViewRightBR::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
	//If it's UpdateAllViews call for top right Hex (IDC_HEX_RIGHT_TR), (from top left Resource Tree) we do nothing.
	if (!m_pChildFrame || LOWORD(lHint) == IDC_HEX_RIGHT_TR || LOWORD(lHint) == ID_DOC_EDITMODE)
		return;

	//If any but Resources Update we destroy m_DlgSampleWnd, if it's currently created.
	if (LOWORD(lHint) != IDC_SHOW_RESOURCE_RBR)
	{
		if (m_wndSampledlg.m_hWnd)
			m_wndSampledlg.DestroyWindow();
	}

	if (m_hwndActive)
		::ShowWindow(m_hwndActive, SW_HIDE);

	CRect rcParent, rcClient;
	GetParent()->GetWindowRect(&rcParent);
	GetClientRect(&rcClient);
	m_fDrawRes = false;

	switch (LOWORD(lHint))
	{
	case IDC_LIST_TLS:
		m_stListTLSCallbacks->SetWindowPos(this, 0, 0, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_hwndActive = m_stListTLSCallbacks->m_hWnd;
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rcParent.Width() / 2, 0);
		break;
	case IDC_TREE_RESOURCE:
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rcParent.Width() / 3, 0);
		break;
	case IDC_SHOW_RESOURCE_RBR:
		ShowResource(reinterpret_cast<SRESHELPER*>(pHint));
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rcParent.Width() / 3, 0);
		break;
	case IDC_LIST_DEBUG_ENTRY:
		CreateDebugEntry(HIWORD(lHint));
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rcParent.Width() / 2, 0);
		break;
	default:
		m_pChildFrame->m_stSplitterRightBottom.HideCol(1);
	}

	m_pChildFrame->m_stSplitterRightBottom.RecalcLayout();
}

void CViewRightBR::OnDraw(CDC* pDC)
{
	CRect rcClipBox;
	pDC->GetClipBox(rcClipBox);

	if (!m_fDrawRes)
	{
		pDC->FillSolidRect(rcClipBox, RGB(255, 255, 255));
		return;
	}

	const auto sizeScroll = GetTotalSize();
	CPoint ptDrawAt;
	int x, y;

	switch (m_iResTypeToDraw)
	{
	case 1: //RT_CURSOR
	case 2: //RT_BITMAP
	case 3: //RT_ICON
	{
		//Drawing in the center, independently from scroll pos.
		pDC->FillSolidRect(rcClipBox, m_clrBkIcons);

		if (sizeScroll.cx > rcClipBox.Width())
			x = sizeScroll.cx / 2 - (m_stBmp.bmWidth / 2);
		else
			x = rcClipBox.Width() / 2 - (m_stBmp.bmWidth / 2);
	
		if (sizeScroll.cy > rcClipBox.Height())
			y = sizeScroll.cy / 2 - (m_stBmp.bmHeight / 2);
		else
			y = rcClipBox.Height() / 2 - (m_stBmp.bmHeight / 2);

		ptDrawAt.SetPoint(x, y);
		m_stImgRes.Draw(pDC, 0, ptDrawAt, ILD_NORMAL);
		break;
	}
	case 5: //RT_DIALOG
		break;
	case 12: //RT_GROUP_CURSOR
	case 14: //RT_GROUP_ICON
	{
		pDC->FillSolidRect(rcClipBox, m_clrBkIcons);

		if (sizeScroll.cx > rcClipBox.Width())
			x = sizeScroll.cx / 2 - (m_iImgResWidth / 2);
		else
			x = rcClipBox.Width() / 2 - (m_iImgResWidth / 2);

		for (const auto& iter : m_vecImgRes)
		{
			IMAGEINFO imginfo;
			iter->GetImageInfo(0, &imginfo);
			int iImgHeight = imginfo.rcImage.bottom - imginfo.rcImage.top;
			if (sizeScroll.cy > rcClipBox.Height())
				y = sizeScroll.cy / 2 - (iImgHeight / 2);
			else
				y = rcClipBox.Height() / 2 - (iImgHeight / 2);

			ptDrawAt.SetPoint(x, y);
			iter->Draw(pDC, 0, ptDrawAt, ILD_NORMAL);
			x += imginfo.rcImage.right - imginfo.rcImage.left;
		}
		break;
	}
	case 0xFF:
		pDC->FillSolidRect(rcClipBox, RGB(255, 255, 255));
		pDC->SetTextColor(RGB(255, 0, 0));
		pDC->TextOutW(0, 0, L"Unable to load resource! It's either damaged, packed or zero-length.");
		break;
	default:
		pDC->TextOutW(0, 0, L"This Resource type is not supported.");
	}
}

BOOL CViewRightBR::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CViewRightBR::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_hwndActive)
		::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightBR::CreateIconCursor(const SRESHELPER* pResHelper)
{
	HICON hIcon;
	ICONINFO iconInfo;

	hIcon = CreateIconFromResourceEx(reinterpret_cast<PBYTE>(pResHelper->pData->data()), static_cast<DWORD>(pResHelper->pData->size()),
		(pResHelper->IdResType == 3) ? TRUE : FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
	if (!hIcon)
		return ResLoadError();
	if (!GetIconInfo(hIcon, &iconInfo))
		return ResLoadError();
	if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
		return ResLoadError();
	DeleteObject(iconInfo.hbmColor);
	DeleteObject(iconInfo.hbmMask);

	m_stImgRes.Create(m_stBmp.bmWidth,
		(pResHelper->IdResType == 3) ? m_stBmp.bmHeight : m_stBmp.bmWidth, ILC_COLORDDB, 0, 1);
	m_stImgRes.SetBkColor(m_clrBkImgList);
	if (m_stImgRes.Add(hIcon) == -1)
		return ResLoadError();

	DestroyIcon(hIcon);
	SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth,
		(pResHelper->IdResType == 3) ? m_stBmp.bmHeight : m_stBmp.bmWidth));

	m_iResTypeToDraw = pResHelper->IdResType;
	m_fDrawRes = true;
}

void CViewRightBR::CreateBitmap(const SRESHELPER* pResHelper)
{
	auto* pDIBInfo = reinterpret_cast<BITMAPINFO*>(pResHelper->pData->data());
	int iColors = pDIBInfo->bmiHeader.biClrUsed ? pDIBInfo->bmiHeader.biClrUsed : 1 << pDIBInfo->bmiHeader.biBitCount;
	LPVOID pDIBBits;

	if (pDIBInfo->bmiHeader.biBitCount > 8)
		pDIBBits = static_cast<LPVOID>(reinterpret_cast<PDWORD>(pDIBInfo->bmiColors + pDIBInfo->bmiHeader.biClrUsed) +
			((pDIBInfo->bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
	else
		pDIBBits = static_cast<LPVOID>(pDIBInfo->bmiColors + iColors);

	HDC hDC = ::GetDC(m_hWnd);
	HBITMAP hBitmap = CreateDIBitmap(hDC, &pDIBInfo->bmiHeader, CBM_INIT, pDIBBits, pDIBInfo, DIB_RGB_COLORS);
	::ReleaseDC(m_hWnd, hDC);
	if (!hBitmap)
		return ResLoadError();

	if (!GetObjectW(hBitmap, sizeof(BITMAP), &m_stBmp))
		return ResLoadError();

	CBitmap bmp;
	if (!bmp.Attach(hBitmap))
		return ResLoadError();

	m_stImgRes.Create(m_stBmp.bmWidth, m_stBmp.bmHeight, ILC_COLORDDB, 0, 1);
	if (m_stImgRes.Add(&bmp, nullptr) == -1)
		return ResLoadError();

	m_iResTypeToDraw = 2;
	m_fDrawRes = true;
	SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmHeight));
	bmp.DeleteObject();
}

void CViewRightBR::CreateDlg(const SRESHELPER* pResHelper)
{
#pragma pack(push, 4)
	struct DLGTEMPLATEEX //Helper struct. Not completed.
	{
		WORD      dlgVer;
		WORD      signature;
		DWORD     helpID;
		DWORD     exStyle;
		DWORD     style;
		WORD      cDlgItems;
		short     x;
		short     y;
		short     cx;
		short     cy;
		WORD      menu;
	};
#pragma pack(pop)

	ParceDlgTemplate(reinterpret_cast<PBYTE>(pResHelper->pData->data()), pResHelper->pData->size(), m_wstrEditBRB);

	bool fDlgEx = *((reinterpret_cast<PWORD>(pResHelper->pData->data())) + 1) == 0xFFFF;

	//Pointer to 'Style' dword.
	PDWORD pStyle { fDlgEx ? &(reinterpret_cast<DLGTEMPLATEEX*>(pResHelper->pData->data()))->style :
		&(reinterpret_cast<DLGTEMPLATE*>(pResHelper->pData->data()))->style };

	bool fWS_VISIBLE { false };
	if (*pStyle & WS_VISIBLE) //Remove WS_VISIBLE flag if exists, so that dialog not steal the focus on creation.
	{
		*pStyle &= ~WS_VISIBLE;
		fWS_VISIBLE = true;
	}

	auto hwndResDlg = CreateDialogIndirectParamW(nullptr,
		reinterpret_cast<LPCDLGTEMPLATEW>(pResHelper->pData->data()), m_hWnd, nullptr, 0);

	if (!hwndResDlg)
	{
		//Trying to set dialog's Items count to 0, to avoid failing on CreateWindowEx 
		//within CreateDialogIndirectParamW, for Dialog items (controls) with CustomClassName.
		//It can be overcome though, by creating a WindowClass with that custom name, 
		//but it's a lot of overhead for every such control: Fetch CustomClassName within ParceDlgTemplate
		//then register it with AfxRegisterClass.
		//Instead, just showing empty dialog without controls.
		PWORD pWordDlgItems;
		WORD wOld;
		if (fDlgEx) //DLGTEMPLATEEX
		{
			pWordDlgItems = &(reinterpret_cast<DLGTEMPLATEEX*>(pResHelper->pData->data()))->cDlgItems;
			wOld = *pWordDlgItems;
		}
		else //DLGTEMPLATE
		{
			pWordDlgItems = &(reinterpret_cast<DLGTEMPLATE*>(pResHelper->pData->data()))->cdit;
			wOld = *pWordDlgItems;
		}
		*pWordDlgItems = 0;
		hwndResDlg = CreateDialogIndirectParamW(nullptr,
			reinterpret_cast<LPCDLGTEMPLATEW>(pResHelper->pData->data()), m_hWnd, nullptr, 0);
		*pWordDlgItems = wOld;
	}

	if (fWS_VISIBLE) //Revert style back.
		*pStyle |= WS_VISIBLE;
	if (!hwndResDlg)
		return ResLoadError();

	CRect rcDlg;
	::GetWindowRect(hwndResDlg, &rcDlg);
	int iPosX = 0, iPosY = 0;
	UINT uFlags = SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER;
	DWORD dwStyles { WS_POPUP | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX };
	DWORD dwExStyles { WS_EX_APPWINDOW };
	if (!m_wndSampledlg.m_hWnd)
	{
		if (!m_wndSampledlg.CreateEx(dwExStyles, AfxRegisterWndClass(0),
			L"Sample Dialog...", dwStyles, 0, 0, 0, 0, m_hWnd, nullptr))
		{
			MessageBoxW(L"Sample Dialog window CreateEx failed.", L"Error");
			return;
		}
		iPosX = (GetSystemMetrics(SM_CXSCREEN) / 2) - rcDlg.Width() / 2;
		iPosY = (GetSystemMetrics(SM_CYSCREEN) / 2) - rcDlg.Height() / 2;
		uFlags &= ~SWP_NOMOVE;
	}

	HDC hDC = ::GetDC(m_wndSampledlg.m_hWnd);
	HDC hDCMemory = CreateCompatibleDC(hDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDC, rcDlg.Width(), rcDlg.Height());
	::SelectObject(hDCMemory, hBitmap);

	//To avoid window pop-up removing Windows animation temporarily, then restore back.
	ANIMATIONINFO aninfo;
	aninfo.cbSize = sizeof(ANIMATIONINFO);
	SystemParametersInfoW(SPI_GETANIMATION, aninfo.cbSize, &aninfo, 0);
	int iMinAnimate = aninfo.iMinAnimate;
	if (iMinAnimate) {
		aninfo.iMinAnimate = 0;
		SystemParametersInfoW(SPI_SETANIMATION, aninfo.cbSize, &aninfo, SPIF_SENDCHANGE);
	}
	LONG_PTR lLong = GetWindowLongPtrW(hwndResDlg, GWL_EXSTYLE);
	SetWindowLongPtrW(hwndResDlg, GWL_EXSTYLE, lLong | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(hwndResDlg, 0, 1, LWA_ALPHA);

	::ShowWindow(hwndResDlg, SW_SHOWNOACTIVATE);
	::PrintWindow(hwndResDlg, hDCMemory, 0);
	::DestroyWindow(hwndResDlg);

	if (iMinAnimate) {
		aninfo.iMinAnimate = iMinAnimate;
		SystemParametersInfoW(SPI_SETANIMATION, aninfo.cbSize, &aninfo, SPIF_SENDCHANGE);
	}

	DeleteDC(hDCMemory);
	::ReleaseDC(m_wndSampledlg.m_hWnd, hDC);

	CBitmap bmp;
	if (!bmp.Attach(hBitmap))
		return ResLoadError();
	m_stImgRes.Create(rcDlg.Width(), rcDlg.Height(), ILC_COLORDDB, 0, 1);
	if (m_stImgRes.Add(&bmp, nullptr) == -1)
		return ResLoadError();
	bmp.DeleteObject();

	AdjustWindowRectEx(rcDlg, dwStyles, FALSE, dwExStyles); //Get window size with desirable client rect.
	m_wndSampledlg.SetWindowPos(this, iPosX, iPosY, rcDlg.Width(), rcDlg.Height(), uFlags);
	m_wndSampledlg.SetDlgVisible(true);
	m_wndSampledlg.RedrawWindow(); //Draw dialog bitmap.

	m_EditBRB.SetWindowTextW(m_wstrEditBRB.data()); //Set Dialog resources info to Editbox.
	CRect rcClient;
	GetClientRect(&rcClient);
	m_EditBRB.SetWindowPos(this, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	m_hwndActive = m_EditBRB.m_hWnd;
}

int CViewRightBR::CreateListTLSCallbacks()
{
	PLIBPE_TLS pTLS;
	if (m_pLibpe->GetTLS(pTLS) != S_OK)
		return -1;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_TLS_CALLBACKS;
	m_stListTLSCallbacks->Create(m_stlcs);
	m_stListTLSCallbacks->InsertColumn(0, L"TLS Callbacks", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 300);
	LVCOLUMNW stCol { LVCF_FMT, LVCFMT_CENTER };
	m_stListTLSCallbacks->SetColumn(0, &stCol);

	int listindex { };
	WCHAR wstr[9];

	for (auto& iterCallbacks : pTLS->vecTLSCallbacks)
	{
		swprintf_s(wstr, 9, L"%08X", iterCallbacks);
		m_stListTLSCallbacks->InsertItem(listindex, wstr);
		listindex++;
	}

	return 0;
}

void CViewRightBR::CreateDebugEntry(DWORD dwEntry)
{
	PLIBPE_DEBUG_VEC pDebug;
	if (m_pLibpe->GetDebug(pDebug) != S_OK)
		return;

	//At the moment only IMAGE_DEBUG_TYPE_CODEVIEW info is supported.
	if (pDebug->at(dwEntry).stDebugDir.Type != IMAGE_DEBUG_TYPE_CODEVIEW)
		return;

	m_wstrEditBRB.clear();
	auto& refDebug = pDebug->at(dwEntry);
	WCHAR warr[9] { };

	if (refDebug.stDebugHdrInfo.dwHdr[0] == 0x53445352) //"RSDS"
	{
		m_wstrEditBRB = L"Signature: RSDS\r\n";
		m_wstrEditBRB += L"GUID: ";
		LPWSTR lpwstr;
		GUID guid = *(reinterpret_cast<GUID*>(&refDebug.stDebugHdrInfo.dwHdr[1]));
		StringFromIID(guid, &lpwstr);
		m_wstrEditBRB += lpwstr;
		m_wstrEditBRB += L"\r\n";
		m_wstrEditBRB += L"Counter/Age: ";
		DwordToWchars(refDebug.stDebugHdrInfo.dwHdr[5], warr);
		m_wstrEditBRB += warr;
		m_wstrEditBRB += L"\r\n";
	}
	else if (refDebug.stDebugHdrInfo.dwHdr[0] == 0x3031424E) //"NB10"
	{
		m_wstrEditBRB = L"Signature: NB10\r\n";
		m_wstrEditBRB += L"Offset: ";
		DwordToWchars(refDebug.stDebugHdrInfo.dwHdr[1], warr);
		m_wstrEditBRB += warr;
		m_wstrEditBRB += L"\r\n";
		m_wstrEditBRB += L"Time/Signature: ";
		DwordToWchars(refDebug.stDebugHdrInfo.dwHdr[2], warr);
		m_wstrEditBRB += warr;
		m_wstrEditBRB += L"\r\n";
		m_wstrEditBRB += L"Counter/Age: ";
		DwordToWchars(refDebug.stDebugHdrInfo.dwHdr[3], warr);
		m_wstrEditBRB += warr;
		m_wstrEditBRB += L"\r\n";
	}

	if (!refDebug.stDebugHdrInfo.strPDBName.empty())
	{
		std::wstring wstr;
		wstr.resize(refDebug.stDebugHdrInfo.strPDBName.size());
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCCH>(refDebug.stDebugHdrInfo.strPDBName.data()),
			static_cast<int>(refDebug.stDebugHdrInfo.strPDBName.size()), &wstr[0],
			static_cast<int>(refDebug.stDebugHdrInfo.strPDBName.size()));
		m_wstrEditBRB += L"PDB File: ";
		m_wstrEditBRB += wstr;
		m_EditBRB.SetWindowTextW(m_wstrEditBRB.data());
	}

	CRect rc;
	GetClientRect(&rc);
	m_EditBRB.SetWindowPos(this, rc.left, rc.top, rc.right, rc.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	m_hwndActive = m_EditBRB.m_hWnd;
}

void CViewRightBR::CreateStrings(const SRESHELPER* pResHelper)
{
	auto pwszResString = reinterpret_cast<LPCWSTR>(pResHelper->pData->data());
	std::wstring wstrTmp;
	for (int i = 0; i < 16; i++)
	{
		m_wstrEditBRB += wstrTmp.assign(pwszResString + 1, static_cast<UINT>(*pwszResString));
		if (i != 15)
			m_wstrEditBRB += L"\r\n";
		pwszResString += 1 + static_cast<UINT>(*pwszResString);
	}

	m_EditBRB.SetWindowTextW(m_wstrEditBRB.data());
	CRect rcClient;
	GetClientRect(&rcClient);
	m_EditBRB.SetWindowPos(this, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_hwndActive = m_EditBRB.m_hWnd;
}

void CViewRightBR::CreateGroupIconCursor(const SRESHELPER* pResHelper)
{
	PLIBPE_RESOURCE_ROOT pstResRoot;
	if (m_pLibpe->GetResources(pstResRoot) != S_OK)
		return;

#pragma pack(push, 2)
	struct GRPICONDIRENTRY
	{
		BYTE   bWidth;               // Width, in pixels, of the image
		BYTE   bHeight;              // Height, in pixels, of the image
		BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
		BYTE   bReserved;            // Reserved
		WORD   wPlanes;              // Color Planes
		WORD   wBitCount;            // Bits per pixel
		DWORD  dwBytesInRes;         // how many bytes in this resource?
		WORD   nID;                  // the ID
	};
	struct GRPICONDIR
	{
		WORD			  idReserved;   // Reserved (must be 0)
		WORD			  idType;	    // Resource type (1 for icons)
		WORD			  idCount;	    // How many images?
		GRPICONDIRENTRY   idEntries[1]; // The entries for each image
	};
	using LPGRPICONDIR = const GRPICONDIR*;
#pragma pack(pop)

	auto pGRPIDir = reinterpret_cast<LPGRPICONDIR>(pResHelper->pData->data());
	HICON hIcon;
	ICONINFO iconInfo;

	for (int i = 0; i < pGRPIDir->idCount; i++)
	{
		auto& rootvec = pstResRoot->vecResRoot;
		for (auto& iterRoot : rootvec)
		{
			if (iterRoot.stResDirEntryRoot.Id == 3 || //RT_ICON
				iterRoot.stResDirEntryRoot.Id == 1)   //RT_CURSOR
			{
				auto& lvl2tup = iterRoot.stResLvL2;
				auto& lvl2vec = lvl2tup.vecResLvL2;

				for (auto& iterlvl2 : lvl2vec)
				{
					if (iterlvl2.stResDirEntryLvL2.Id == pGRPIDir->idEntries[i].nID)
					{
						auto& lvl3tup = iterlvl2.stResLvL3;
						auto& lvl3vec = lvl3tup.vecResLvL3;

						if (!lvl3vec.empty())
						{
							auto& data = lvl3vec.at(0).vecResRawDataLvL3;
							if (!data.empty())
							{
								hIcon = CreateIconFromResourceEx(reinterpret_cast<PBYTE>(data.data()), static_cast<DWORD>(data.size()),
									(iterRoot.stResDirEntryRoot.Id == 3) ? TRUE : FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
								if (!hIcon)
									return ResLoadError();
								if (!GetIconInfo(hIcon, &iconInfo))
									return ResLoadError();
								if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
									return ResLoadError();

								DeleteObject(iconInfo.hbmColor);
								DeleteObject(iconInfo.hbmMask);

								m_vecImgRes.emplace_back(std::make_unique<CImageList>());
								auto& vecBack = m_vecImgRes.back();
								vecBack->Create(m_stBmp.bmWidth,
									(iterRoot.stResDirEntryRoot.Id == 3) ? m_stBmp.bmHeight : m_stBmp.bmWidth, ILC_COLORDDB, 0, 1);
								vecBack->SetBkColor(m_clrBkImgList);
								m_iImgResWidth += m_stBmp.bmWidth;
								m_iImgResHeight = max(m_stBmp.bmHeight, m_iImgResHeight);

								if (vecBack->Add(hIcon) == -1)
									return ResLoadError();

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

	m_iResTypeToDraw = pResHelper->IdResType;
	m_fDrawRes = true;
}

void CViewRightBR::CreateVersion(const SRESHELPER* pResHelper)
{
#pragma pack(push, 4)
	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	};
#pragma pack(pop)

	static const std::map<int, std::wstring> mapVerInfoStrings {
		{ 0, L"FileDescription" },
		{ 1, L"FileVersion" },
		{ 2, L"InternalName" },
		{ 3, L"CompanyName" },
		{ 4, L"LegalCopyright" },
		{ 5, L"OriginalFilename" },
		{ 6, L"ProductName" },
		{ 7, L"ProductVersion" }
	};

	LANGANDCODEPAGE* pLangAndCP;
	UINT dwBytesOut;

	//Read the list of languages and code pages.
	VerQueryValueW(pResHelper->pData->data(), L"\\VarFileInfo\\Translation", reinterpret_cast<LPVOID*>(&pLangAndCP), &dwBytesOut);

	WCHAR wstrSubBlock[50];
	DWORD dwLangCount = dwBytesOut / sizeof(LANGANDCODEPAGE);
	//Read the file description for each language and code page.
	for (size_t iterCodePage = 0; iterCodePage < dwLangCount; iterCodePage++)
	{
		for (unsigned i = 0; i < mapVerInfoStrings.size(); i++) //sizeof pstrVerInfoStrings [];
		{
			swprintf_s(wstrSubBlock, 50, L"\\StringFileInfo\\%04x%04x\\%s",
				pLangAndCP[iterCodePage].wLanguage, pLangAndCP[iterCodePage].wCodePage, mapVerInfoStrings.at(i).data());

			m_wstrEditBRB += mapVerInfoStrings.at(i);
			m_wstrEditBRB += L" - ";

			WCHAR* pszBufferOut;
			if (VerQueryValueW(pResHelper->pData->data(), wstrSubBlock, reinterpret_cast<LPVOID*>(&pszBufferOut), &dwBytesOut))
				if (dwBytesOut)
					m_wstrEditBRB += pszBufferOut;
			m_wstrEditBRB += L"\r\n";
		}
	}
	m_EditBRB.SetWindowTextW(m_wstrEditBRB.data());
	CRect rcClient;
	GetClientRect(&rcClient);
	m_EditBRB.SetWindowPos(this, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	m_hwndActive = m_EditBRB.m_hWnd;
}

void CViewRightBR::CreateManifest(const SRESHELPER* pResHelper)
{
	m_wstrEditBRB.resize(pResHelper->pData->size());
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCCH>(pResHelper->pData->data()),
		static_cast<int>(pResHelper->pData->size()), &m_wstrEditBRB[0], static_cast<int>(pResHelper->pData->size()));

	m_EditBRB.SetWindowTextW(m_wstrEditBRB.data());
	CRect rcClient;
	GetClientRect(&rcClient);
	m_EditBRB.SetWindowPos(this, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	m_hwndActive = m_EditBRB.m_hWnd;
}

void CViewRightBR::CreateToolbar(const SRESHELPER* pResHelper)
{
	PLIBPE_RESOURCE_ROOT pstResRoot;
	if (m_pLibpe->GetResources(pstResRoot) != S_OK)
		return;

	auto& rootvec = pstResRoot->vecResRoot;
	for (auto& iterRoot : rootvec)
	{
		if (iterRoot.stResDirEntryRoot.Id == 2) //RT_BITMAP
		{
			auto& lvl2tup = iterRoot.stResLvL2;
			auto& lvl2vec = lvl2tup.vecResLvL2;

			for (auto& iterlvl2 : lvl2vec)
			{
				if (iterlvl2.stResDirEntryLvL2.Id == pResHelper->IdResName)
				{
					auto& lvl3tup = iterlvl2.stResLvL3;
					auto& lvl3vec = lvl3tup.vecResLvL3;

					if (!lvl3vec.empty())
					{
						auto& data = lvl3vec.at(0).vecResRawDataLvL3;
						if (!data.empty())
						{
							SRESHELPER rh(2, pResHelper->IdResName, static_cast<std::vector<std::byte>*>(&data));
							ShowResource(&rh);
						}
					}
				}
			}
		}
	}
}

void CViewRightBR::ParceDlgTemplate(PBYTE pDataDlgRes, size_t nSize, std::wstring& wstrData)
{
	//If data is from PE resources, then it is packed.
#pragma pack(push, 4)
	struct DLGTEMPLATEEX //Helper struct. Not completed.
	{
		WORD      dlgVer;
		WORD      signature;
		DWORD     helpID;
		DWORD     exStyle;
		DWORD     style;
		WORD      cDlgItems;
		short     x;
		short     y;
		short     cx;
		short     cy;
		WORD      menu;
	};
	struct DLGITEMTEMPLATEEX //Helper struct. Not completed.
	{
		DWORD     helpID;
		DWORD     exStyle;
		DWORD     style;
		short     x;
		short     y;
		short     cx;
		short     cy;
		DWORD     id;
	};
#pragma pack(pop)

	//All Window and Dialog styles except default (0).
	static const std::map<DWORD, std::wstring> mapDlgStyles {
		TO_WSTR_MAP(DS_3DLOOK),
		TO_WSTR_MAP(DS_ABSALIGN),
		TO_WSTR_MAP(DS_CENTER),
		TO_WSTR_MAP(DS_CENTERMOUSE),
		TO_WSTR_MAP(DS_CONTEXTHELP),
		TO_WSTR_MAP(DS_CONTROL),
		TO_WSTR_MAP(DS_FIXEDSYS),
		TO_WSTR_MAP(DS_LOCALEDIT),
		TO_WSTR_MAP(DS_MODALFRAME),
		TO_WSTR_MAP(DS_NOFAILCREATE),
		TO_WSTR_MAP(DS_NOIDLEMSG),
		TO_WSTR_MAP(DS_SETFONT),
		TO_WSTR_MAP(DS_SETFOREGROUND),
		TO_WSTR_MAP(DS_SHELLFONT),
		TO_WSTR_MAP(DS_SYSMODAL),
		TO_WSTR_MAP(WS_BORDER),
		TO_WSTR_MAP(WS_CAPTION),
		TO_WSTR_MAP(WS_CHILD),
		TO_WSTR_MAP(WS_CHILDWINDOW),
		TO_WSTR_MAP(WS_CLIPCHILDREN),
		TO_WSTR_MAP(WS_CLIPSIBLINGS),
		TO_WSTR_MAP(WS_DISABLED),
		TO_WSTR_MAP(WS_DLGFRAME),
		TO_WSTR_MAP(WS_HSCROLL),
		TO_WSTR_MAP(WS_MAXIMIZE),
		TO_WSTR_MAP(WS_MAXIMIZEBOX),
		TO_WSTR_MAP(WS_MINIMIZE),
		TO_WSTR_MAP(WS_MINIMIZEBOX),
		TO_WSTR_MAP(WS_POPUP),
		TO_WSTR_MAP(WS_SYSMENU),
		TO_WSTR_MAP(WS_THICKFRAME),
		TO_WSTR_MAP(WS_VISIBLE),
		TO_WSTR_MAP(WS_VSCROLL)
	};

	//All Extended Window styles except default (0).
	static const std::map<DWORD, std::wstring> mapDlgExStyles
	{
		TO_WSTR_MAP(WS_EX_ACCEPTFILES),
		TO_WSTR_MAP(WS_EX_APPWINDOW),
		TO_WSTR_MAP(WS_EX_CLIENTEDGE),
		TO_WSTR_MAP(WS_EX_COMPOSITED),
		TO_WSTR_MAP(WS_EX_CONTEXTHELP),
		TO_WSTR_MAP(WS_EX_CONTROLPARENT),
		TO_WSTR_MAP(WS_EX_DLGMODALFRAME),
		TO_WSTR_MAP(WS_EX_LAYERED),
		TO_WSTR_MAP(WS_EX_LAYOUTRTL),
		TO_WSTR_MAP(WS_EX_LEFTSCROLLBAR),
		TO_WSTR_MAP(WS_EX_MDICHILD),
		TO_WSTR_MAP(WS_EX_NOACTIVATE),
		TO_WSTR_MAP(WS_EX_NOINHERITLAYOUT),
		TO_WSTR_MAP(WS_EX_NOPARENTNOTIFY),
		TO_WSTR_MAP(WS_EX_RIGHT),
		TO_WSTR_MAP(WS_EX_RTLREADING),
		TO_WSTR_MAP(WS_EX_STATICEDGE),
		TO_WSTR_MAP(WS_EX_TOOLWINDOW),
		TO_WSTR_MAP(WS_EX_TOPMOST),
		TO_WSTR_MAP(WS_EX_TRANSPARENT),
		TO_WSTR_MAP(WS_EX_WINDOWEDGE),
	};

	PBYTE pDataHdr = pDataDlgRes;
	bool fDlgEx { false };
	if (*((reinterpret_cast<PWORD>(pDataHdr)) + 1) == 0xFFFF) //DLGTEMPLATEEX
		fDlgEx = true;

	DWORD dwStyles, dwExStyles;
	WORD wDlgItems;
	short x, y, cx, cy;
	WCHAR* pwstrMenuRes { }; size_t lengthMenuRes { }; WORD wMenuResOrdinal { };
	WCHAR* pwstrTypeFace { }; size_t lengthTypeFace { };

	if (fDlgEx) //DLGTEMPLATEEX
	{
		dwStyles = (reinterpret_cast<DLGTEMPLATEEX*>(pDataHdr))->style;
		dwExStyles = (reinterpret_cast<DLGTEMPLATEEX*>(pDataHdr))->exStyle;
		wDlgItems = (reinterpret_cast<DLGTEMPLATEEX*>(pDataHdr))->cDlgItems;
		x = (reinterpret_cast<DLGTEMPLATEEX*>(pDataHdr))->x;
		y = (reinterpret_cast<DLGTEMPLATEEX*>(pDataHdr))->y;
		cx = (reinterpret_cast<DLGTEMPLATEEX*>(pDataHdr))->cx;
		cy = (reinterpret_cast<DLGTEMPLATEEX*>(pDataHdr))->cy;

		wstrData = L"DIALOGEX ";

		//Menu.
		if ((reinterpret_cast<DLGTEMPLATEEX*>(pDataHdr))->menu == 0) //No menu.
			pDataHdr += sizeof(DLGTEMPLATEEX);
		else if ((reinterpret_cast<DLGTEMPLATEEX*>(pDataHdr))->menu == 0xFFFF) //Menu ordinal.
		{
			wMenuResOrdinal = *reinterpret_cast<PWORD>(pDataHdr + sizeof(WORD));
			pDataHdr += sizeof(WORD) * 2; //Ordinal's WORD follows ((DLGTEMPLATEEX*)pDataHdr)->menu.
		}
		else //Menu wstring.
		{
			pDataHdr += sizeof(DLGTEMPLATEEX);
			pwstrMenuRes = reinterpret_cast<WCHAR*>(pDataHdr);
			if (StringCbLengthW(pwstrMenuRes, nSize - (reinterpret_cast<DWORD_PTR>(pDataHdr) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)), &lengthMenuRes) != S_OK)
				return ResLoadError();
			pDataHdr += lengthMenuRes + sizeof(WCHAR); //Plus null terminating.
		}
	}
	else //DLGTEMPLATE
	{
		dwStyles = (reinterpret_cast<DLGTEMPLATE*>(pDataHdr))->style;
		dwExStyles = (reinterpret_cast<DLGTEMPLATE*>(pDataHdr))->dwExtendedStyle;
		wDlgItems = (reinterpret_cast<DLGTEMPLATE*>(pDataHdr))->cdit;
		x = (reinterpret_cast<DLGTEMPLATE*>(pDataHdr))->x;
		y = (reinterpret_cast<DLGTEMPLATE*>(pDataHdr))->y;
		cx = (reinterpret_cast<DLGTEMPLATE*>(pDataHdr))->cx;
		cy = (reinterpret_cast<DLGTEMPLATE*>(pDataHdr))->cy;
		pDataHdr += sizeof(DLGTEMPLATE);

		wstrData = L"DIALOG ";

		//Menu.
		if (*reinterpret_cast<PWORD>(pDataHdr) == 0) //No menu.
			pDataHdr += sizeof(WORD);
		else if (*reinterpret_cast<PWORD>(pDataHdr) == 0xFFFF) //Menu ordinal.
		{
			wMenuResOrdinal = *reinterpret_cast<PWORD>(pDataHdr + sizeof(WORD));
			pDataHdr += sizeof(WORD) * 2; //Ordinal's WORD follows menu WORD.
		}
		else //Menu wstring.
		{
			pwstrMenuRes = reinterpret_cast<WCHAR*>(&pDataHdr);
			if (StringCbLengthW(pwstrMenuRes, nSize - (reinterpret_cast<DWORD_PTR>(pDataHdr) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)), &lengthMenuRes) != S_OK)
				return ResLoadError();
			pDataHdr += lengthMenuRes + sizeof(WCHAR); //Plus null terminating.
		}
	}

	WCHAR wpsz[128];
	swprintf_s(wpsz, L"%i, %i, %i, %i\r\n", x, y, cx, cy);
	wstrData += wpsz;

	//Dialog styles stringanize.
	std::wstring wstrStyles;
	for (auto& i : mapDlgStyles)
	{
		if (i.first & dwStyles)
		{
			if (!wstrStyles.empty())
				wstrStyles += L" | " + i.second;
			else
				wstrStyles += i.second;
		}
	}

	wstrData += L"DIALOG STYLES: " + wstrStyles + L"\r\n";
	if (dwExStyles) //ExStyle sringanize.
	{
		wstrStyles.clear();
		for (auto& i : mapDlgExStyles)
		{
			if (i.first & dwExStyles)
			{
				if (!wstrStyles.empty())
					wstrStyles += L" | " + i.second;
				else
					wstrStyles += i.second;
			}
		}
		wstrData += L"DIALOG EXTENDED STYLES: " + wstrStyles + L"\r\n";
	}

	//////////////////////////////////////////////////////////////////
	//Next goes Class Name and Title that common for both templates.
	//////////////////////////////////////////////////////////////////

	//Class name.
	if (*reinterpret_cast<PWORD>(pDataHdr) == 0xFFFF) //Class name is ordinal.
	{
		WORD wClassOrdinal = *reinterpret_cast<PWORD>(pDataHdr + sizeof(WORD));
		pDataHdr += sizeof(WORD) * 2; //Class name WORD plus Ordinal WORD.

		wstrData += L"DIALOG CLASS ORDINAL: ";
		swprintf_s(wpsz, L"0x%X\r\n", wClassOrdinal);
		wstrData += wpsz;
	}
	else //Class name is WString.
	{
		if (*reinterpret_cast<PWORD>(pDataHdr) != 0x0000) //If not NULL then there is a need to align.
			pDataHdr += (sizeof(WORD) - ((reinterpret_cast<DWORD_PTR>(pDataHdr) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)) & 1)) & 1; //WORD Aligning.
		size_t lengthClassName;
		auto* pwstrClassName = reinterpret_cast<WCHAR*>(pDataHdr);
		if (StringCbLengthW(pwstrClassName, nSize - (reinterpret_cast<DWORD_PTR>(pDataHdr) -
			reinterpret_cast<DWORD_PTR>(pDataDlgRes)), &lengthClassName) != S_OK)
			return ResLoadError();
		pDataHdr += lengthClassName + sizeof(WCHAR); //Plus null terminating.

		wstrData += L"DIALOG CLASS NAME: \"";
		wstrData += pwstrClassName;
		wstrData += L"\"\r\n";
	}

	//Title (Caption)
	if (*reinterpret_cast<PWORD>(pDataHdr) != 0x0000) //If not NULL then there is a need to align.
		pDataHdr += (sizeof(WORD) - ((reinterpret_cast<DWORD_PTR>(pDataHdr) -
			reinterpret_cast<DWORD_PTR>(pDataDlgRes)) & 1)) & 1; //WORD Aligning.
	size_t lengthTitle;
	auto* pwstrTitle = reinterpret_cast<WCHAR*>(pDataHdr);
	if (StringCbLengthW(pwstrTitle, nSize - (reinterpret_cast<DWORD_PTR>(pDataHdr) -
		reinterpret_cast<DWORD_PTR>(pDataDlgRes)), &lengthTitle) != S_OK)
		return ResLoadError();
	pDataHdr += lengthTitle + sizeof(WCHAR); //Plus null terminating.

	wstrData += L"DIALOG CAPTION: \"";
	wstrData += pwstrTitle;
	wstrData += L"\"\r\n";

	//Menu.
	if (pwstrMenuRes && lengthMenuRes)
	{
		wstrData += L"DIALOG MENU RESOURCE NAME: \"";
		wstrData += pwstrMenuRes;
		wstrData += L"\"\r\n";
	}
	else if (wMenuResOrdinal)
	{
		wstrData += L"DIALOG MENU RESOURCE ORDINAL: ";
		swprintf_s(wpsz, L"0x%X\r\n", wMenuResOrdinal);
		wstrData += wpsz;
	}

	//Font related stuff has little differences between templates.
	wstrData += L"DIALOG FONT: ";
	WORD wFontPointSize { };
	if (fDlgEx && ((dwStyles & DS_SETFONT) || (dwStyles & DS_SHELLFONT))) //DLGTEMPLATEEX
	{
		//Font related. Only if DS_SETFONT or DS_SHELLFONT styles present.

		wFontPointSize = *reinterpret_cast<PWORD>(pDataHdr);
		pDataHdr += sizeof(wFontPointSize);

		WORD wFontWeight = *reinterpret_cast<PWORD>(pDataHdr);
		pDataHdr += sizeof(wFontWeight);

		BYTE bItalic = *pDataHdr;
		pDataHdr += sizeof(bItalic);

		BYTE bCharset = *pDataHdr;
		pDataHdr += sizeof(bCharset);

		if (*reinterpret_cast<PWORD>(pDataHdr) != 0x0000) //If not NULL nedd to align.
			pDataHdr += (sizeof(WORD) - ((reinterpret_cast<DWORD_PTR>(pDataHdr) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)) & 1)) & 1; //WORD Aligning.
		pwstrTypeFace = reinterpret_cast<WCHAR*>(pDataHdr);
		if (StringCbLengthW(pwstrTypeFace, nSize - (reinterpret_cast<DWORD_PTR>(pDataHdr) -
			reinterpret_cast<DWORD_PTR>(pDataDlgRes)), &lengthTypeFace) != S_OK)
			return ResLoadError();
		pDataHdr += lengthTypeFace + sizeof(WCHAR); //Plus null terminating.

		wstrData += L"NAME: \"";
		wstrData += pwstrTypeFace;
		wstrData += L"\"";
		//These Font params are only in DLGTEMPLATEEX.
		swprintf_s(wpsz, L", SIZE: %hu, WEIGHT: %hu, IS ITALIC: %hhu, CHARSET: %hhu", wFontPointSize, wFontWeight, bItalic, bCharset);
		wstrData += wpsz;
		wstrData += L"\r\n";
	}
	else if (!fDlgEx && (dwStyles & DS_SETFONT)) //DLGTEMPLATE
	{
		//Font related. Only if DS_SETFONT style present.

		wFontPointSize = *reinterpret_cast<PWORD>(pDataHdr);
		pDataHdr += sizeof(wFontPointSize);

		if (*reinterpret_cast<PWORD>(pDataHdr) != 0x0000)
			pDataHdr += (sizeof(WORD) - ((reinterpret_cast<DWORD_PTR>(pDataHdr) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)) & 1)) & 1; //WORD Aligning.
		pwstrTypeFace = reinterpret_cast<WCHAR*>(pDataHdr);
		if (StringCbLengthW(pwstrTypeFace, nSize - (reinterpret_cast<DWORD_PTR>(pDataHdr) -
			reinterpret_cast<DWORD_PTR>(pDataDlgRes)), &lengthTypeFace) != S_OK)
			return ResLoadError();
		pDataHdr += lengthTypeFace + sizeof(WCHAR); //Plus null terminating.

		wstrData += L"NAME \"";
		wstrData += pwstrTypeFace;
		wstrData += L"\"";
		swprintf_s(wpsz, L", SIZE: %hu", wFontPointSize); //Only SIZE in DLGTEMPLATE.
		wstrData += wpsz;
		wstrData += L"\r\n";
	}

	wstrData += L"DIALOG ITEMS: ";
	swprintf_s(wpsz, L"%i\r\n", wDlgItems);
	wstrData += wpsz;
	//DLGTEMPLATE(EX) end. ///////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	//Now go DLGITEMTEMPLATE(EX) data structures.
	//////////////////////////////////////////////////////////////////
	PBYTE pDataItems = pDataHdr; //Just to differentiate.

	wstrData += L"{\r\n"; //Open bracer for stringanize.
	for (WORD items = 0; items < wDlgItems; items++)
	{
		pDataItems += (sizeof(DWORD) - ((reinterpret_cast<DWORD_PTR>(pDataItems) -
			reinterpret_cast<DWORD_PTR>(pDataDlgRes)) & 3)) & 3; //DWORD Aligning.
		//Out of bounds checking.
		if (reinterpret_cast<DWORD_PTR>(pDataItems) >= reinterpret_cast<DWORD_PTR>(pDataDlgRes) + nSize)
			break;

		DWORD dwItemStyles, dwItemExStyles;
		short xItem, yItem, cxItem, cyItem;
		WCHAR wpszItemStyles[128]; //Styles, ExStyles, ItemID and Coords.
		if (fDlgEx) //DLGITEMTEMPLATEEX
		{
			dwItemStyles = (reinterpret_cast<DLGITEMTEMPLATEEX*>(pDataItems))->style;
			dwItemExStyles = (reinterpret_cast<DLGITEMTEMPLATEEX*>(pDataItems))->exStyle;
			xItem = (reinterpret_cast<DLGITEMTEMPLATEEX*>(pDataItems))->x;
			yItem = (reinterpret_cast<DLGITEMTEMPLATEEX*>(pDataItems))->y;
			cxItem = (reinterpret_cast<DLGITEMTEMPLATEEX*>(pDataItems))->cx;
			cyItem = (reinterpret_cast<DLGITEMTEMPLATEEX*>(pDataItems))->cy;
			DWORD dwIDItem = (reinterpret_cast<DLGITEMTEMPLATEEX*>(pDataItems))->id;

			pDataItems += sizeof(DLGITEMTEMPLATEEX);

			//Styles, ExStyles, ItemID and Coords. ItemID is DWORD;
			swprintf_s(wpszItemStyles, L"Styles: 0x%08lX, ExStyles: 0x%08lX, ItemID: 0x%08lX, Coords: x=%i, y=%i, cx=%i, cy=%i",
				dwItemStyles, dwItemExStyles, dwIDItem, xItem, yItem, cxItem, cyItem);
		}
		else //DLGITEMTEMPLATE
		{
			dwItemStyles = (reinterpret_cast<DLGITEMTEMPLATE*>(pDataItems))->style;
			dwItemExStyles = (reinterpret_cast<DLGITEMTEMPLATE*>(pDataItems))->dwExtendedStyle;
			xItem = (reinterpret_cast<DLGITEMTEMPLATE*>(pDataItems))->x;
			yItem = (reinterpret_cast<DLGITEMTEMPLATE*>(pDataItems))->y;
			cxItem = (reinterpret_cast<DLGITEMTEMPLATE*>(pDataItems))->cx;
			cyItem = (reinterpret_cast<DLGITEMTEMPLATE*>(pDataItems))->cy;
			WORD wIDItem = (reinterpret_cast<DLGITEMTEMPLATE*>(pDataItems))->id;

			pDataItems += sizeof(DLGITEMTEMPLATE);

			//Styles, ExStyles, ItemID and Coords. ItemID is WORD;
			swprintf_s(wpszItemStyles, L"Styles: 0x%08lX, ExStyles: 0x%08lX, ItemID: 0x%04hX, Coords: x=%i, y=%i, cx=%i, cy=%i",
				dwItemStyles, dwItemExStyles, wIDItem, xItem, yItem, cxItem, cyItem);
		}

		static const std::map<WORD, std::wstring> mapItemClassOrd {
			{ 0x0080, L"Button" },
			{ 0x0081, L"Edit" },
			{ 0x0082, L"Static" },
			{ 0x0083, L"List box" },
			{ 0x0084, L"Scroll bar" },
			{ 0x0085, L"Combo box" },
		};
		std::wstring wstrItem = L"    ";

		//Item Class name.
		if (*reinterpret_cast<PWORD>(pDataItems) == 0xFFFF) //Class is ordinal
		{
			WORD wClassOrdinalItem = *reinterpret_cast<PWORD>(pDataItems + sizeof(WORD));
			pDataItems += sizeof(WORD) * 2;

			swprintf_s(wpsz, L"Class ordinal: 0x%04X", wClassOrdinalItem);
			wstrItem += wpsz;
			auto iter = mapItemClassOrd.find(wClassOrdinalItem);
			if (iter != mapItemClassOrd.end())
				wstrItem += L" (\"" + iter->second + L"\")";
			wstrItem += L", ";
		}
		else //Class name is Wstring.
		{
			pDataItems += (sizeof(WORD) - ((reinterpret_cast<DWORD_PTR>(pDataItems) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)) & 1)) & 1; //WORD Aligning.
			size_t lengthClassNameItem;
			auto* pwstrClassNameItem = reinterpret_cast<WCHAR*>(pDataItems);
			if (StringCbLengthW(pwstrClassNameItem, nSize - (reinterpret_cast<DWORD_PTR>(pDataItems) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)), &lengthClassNameItem) != S_OK)
				return ResLoadError();
			pDataItems += lengthClassNameItem + sizeof(WCHAR); //Plus null terminating.

			wstrItem += L"Class name: \"";
			wstrItem += pwstrClassNameItem;
			wstrItem += L"\", ";
		}

		//Item Title (Caption).
		if (*reinterpret_cast<PWORD>(pDataItems) == 0xFFFF) //Item Title is ordinal
		{
			WORD wTitleOrdinalItem = *reinterpret_cast<PWORD>(pDataItems + sizeof(WORD));
			pDataItems += sizeof(WORD) * 2;

			swprintf_s(wpsz, L"Caption ordinal: 0x%04X, ", wTitleOrdinalItem);
			wstrItem += wpsz;
		}
		else //Title is wstring.
		{
			pDataItems += (sizeof(WORD) - ((reinterpret_cast<DWORD_PTR>(pDataItems) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)) & 1)) & 1; //WORD Aligning.
			size_t lengthTitleItem;
			auto* pwstrTitleItem = reinterpret_cast<WCHAR*>(pDataItems);
			if (StringCbLengthW(pwstrTitleItem, nSize - (reinterpret_cast<DWORD_PTR>(pDataItems) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)), &lengthTitleItem) != S_OK)
				return ResLoadError();
			pDataItems += lengthTitleItem + sizeof(WCHAR); //Plus null terminating.

			wstrItem += L"Caption: \"";
			wstrItem += pwstrTitleItem;
			wstrItem += L"\", ";
		}

		//Extra count Item.
		WORD wExtraCountItem = *reinterpret_cast<PWORD>(pDataItems);
		pDataItems += sizeof(WORD);
		if (wExtraCountItem)
		{
			pDataItems += (sizeof(WORD) - ((reinterpret_cast<DWORD_PTR>(pDataItems) -
				reinterpret_cast<DWORD_PTR>(pDataDlgRes)) & 1)) & 1; //WORD Aligning.
			pDataItems += wExtraCountItem;
		}

		wstrItem += wpszItemStyles;
		wstrData += wstrItem + L"\r\n";
	}
	wstrData += L"}";
}

void CViewRightBR::ShowResource(const SRESHELPER* pResHelper)
{
	m_stImgRes.DeleteImageList();
	m_iResTypeToDraw = -1;
	m_iImgResWidth = 0;
	m_iImgResHeight = 0;
	m_vecImgRes.clear();
	m_wstrEditBRB.clear();

	if (pResHelper)
	{
		//Destroy Dialog Sample window if it's any other resource type now.
		if (pResHelper->IdResType != 5 && m_wndSampledlg.m_hWnd)
		{
			m_wndSampledlg.SetDlgVisible(false);
			m_wndSampledlg.DestroyWindow();
		}
		if (pResHelper->pData->empty())
			return ResLoadError();

		switch (pResHelper->IdResType)
		{
		case 1: //RT_CURSOR
		case 3: //RT_ICON
			CreateIconCursor(pResHelper);
			break;
		case 2: //RT_BITMAP
			CreateBitmap(pResHelper);
			break;
			//case 4: //RT_MENU
			//break;
		case 5: //RT_DIALOG
			CreateDlg(pResHelper);
			break;
		case 6: //RT_STRING
			CreateStrings(pResHelper);
			break;
		case 12: //RT_GROUP_CURSOR
		case 14: //RT_GROUP_ICON
			CreateGroupIconCursor(pResHelper);
			break;
		case 16: //RT_VERSION
			CreateVersion(pResHelper);
			break;
		case 24: //RT_MANIFEST
			CreateManifest(pResHelper);
			break;
		case 241: //RT_TOOLBAR
			CreateToolbar(pResHelper);
			break;
		default:
			m_iResTypeToDraw = pResHelper->IdResType;
			m_fDrawRes = true;
		}
	}
	else
	{
		//Destroy Dialog Sample window if it's just Resource window Update.
		if (m_wndSampledlg.m_hWnd)
		{
			m_wndSampledlg.SetDlgVisible(false);
			m_wndSampledlg.DestroyWindow();
		}
	}

	RedrawWindow();
}

void CViewRightBR::ResLoadError()
{
	m_iResTypeToDraw = 0xFF;
	m_fDrawRes = true;
	RedrawWindow();
}