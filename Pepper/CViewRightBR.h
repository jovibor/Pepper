/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once
#include <memory>
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "ListEx/ListEx.h"
#include <span>
import Utility;
using namespace util;
using namespace LISTEX;

class CWndSampleDlg final : public CWnd
{
public:
	void Attach(CImageList* pImgList, CChildFrame* pChildFrame);
	void CreatedForMenu(bool fMenu);  //Created for show RT_MENU, not RT_DIALOG.
	void SetDlgVisible(bool fVisible);
private:
	afx_msg void OnPaint();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
private:
	CImageList* m_pImgRes { };
	CChildFrame* m_pChildFrame { };
	bool m_fMenu { false }; //Dialog is created only for showing RT_MENU.
};

class CViewRightBR final : public CScrollView
{
private:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint pt);
	BOOL OnCommand(WPARAM wParam, LPARAM lParam)override;
	void OnDraw(CDC* pDC)override;
	void OnInitialUpdate()override;
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)override;
	void CreateIconCursor(const SRESDATA& stResData);
	void CreateBitmap(const SRESDATA& stResData);
	void CreatePNG(const SRESDATA& stResData);
	void CreateMenu(const SRESDATA& stResData);
	void CreateDebugEntry(DWORD dwEntry);
	void CreateDlg(const SRESDATA& stResData);
	void CreateListTLSCallbacks();
	void CreateStrings(const SRESDATA& stResData);
	void CreateAccel(const SRESDATA& stResData);
	void CreateGroupIconCursor(const SRESDATA& stResData);
	void CreateVersion(const SRESDATA& stResData);
	void CreateManifest(const SRESDATA& stResData);
	void CreateToolbar(const SRESDATA& stResData);
	void ResLoadError();
	void ShowResource(const SRESDATA* pResData);
	static auto ParceDlgTemplate(std::span<std::byte> spnData)->std::optional<std::wstring>;
	static void PremultiplyBitmapAlpha(HDC hDC, HBITMAP hBmp);
	DECLARE_MESSAGE_MAP();
	DECLARE_DYNCREATE(CViewRightBR);
private:
	HWND m_hwndActive { };
	Ilibpe* m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	SRESDATA* m_pResData { }; //Current resource's data pointer.
	CImageList m_stImgRes;
	CImage m_imgPng; //Image for .PNG, with transparency.
	CWndSampleDlg m_wndSampleDlg;
	CMenu m_menuSample;
	LISTEXCREATE m_stlcs;
	IListExPtr m_stListTLSCallbacks { CreateListEx() };
	LOGFONTW m_lf { };
	LOGFONTW m_hdrlf { };
	COLORREF m_clrBkIcons { RGB(230, 230, 230) };
	COLORREF m_clrBkImgList { RGB(250, 250, 250) };
	EResType m_eResTypeToDraw { };
	int m_iImgResWidth { };  //Width of the whole image to draw.
	int m_iImgResHeight { }; //Height of the whole image to draw.
	std::vector<std::unique_ptr<CImageList>> m_vecImgRes { }; //Vector for RT_GROUP_ICON/CURSOR.
	CEdit m_EditBRB;     //Edit control for RT_STRING, RT_VERSION, RT_MANIFEST, Debug additional info
	CFont m_fontEditRes; //Font for m_EditBRB.
};