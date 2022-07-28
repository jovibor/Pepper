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
#include "Utility.h"
#include <span>

using namespace LISTEX;

class CWndSampleDlg final : public CWnd
{
public:
	void Attach(CImageList* pImgList, CChildFrame* pChildFrame);
	void SetDlgVisible(bool fVisible);
	DECLARE_MESSAGE_MAP()
private:
	afx_msg void OnPaint();
	afx_msg void OnClose();
private:
	CImageList* m_pImgRes { };
	CChildFrame* m_pChildFrame { };
};

class CViewRightBR final : public CScrollView
{
private:
	void OnDraw(CDC* pDC)override;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void OnInitialUpdate()override;     // first time after construct
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)override;
	void CreateIconCursor(const SRESDATA& stResData);
	void CreateBitmap(const SRESDATA& stResData);
	void CreateDebugEntry(DWORD dwEntry);
	void CreateDlg(const SRESDATA& stResData);
	void CreateListTLSCallbacks();
	void CreateStrings(const SRESDATA& stResData);
	void CreateGroupIconCursor(const SRESDATA& stResData);
	void CreateVersion(const SRESDATA& stResData);
	void CreateManifest(const SRESDATA& stResData);
	void CreateToolbar(const SRESDATA& stResData);
	void ResLoadError();
	void ShowResource(const SRESDATA* pResData);
	static auto ParceDlgTemplate(std::span<std::byte> spnData)->std::optional<std::wstring>;
	DECLARE_DYNCREATE(CViewRightBR)
	DECLARE_MESSAGE_MAP()
private:
	HWND m_hwndActive { };
	Ilibpe* m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CImageList m_stImgRes;
	CWndSampleDlg m_wndSampledlg;
	LISTEXCREATE m_stlcs;
	IListExPtr m_stListTLSCallbacks { CreateListEx() };
	LOGFONTW m_lf { }, m_hdrlf { };
	COLORREF m_clrBkIcons { RGB(230, 230, 230) };
	COLORREF m_clrBkImgList { RGB(250, 250, 250) };
	BITMAP m_stBmp { };
	int m_iResTypeToDraw { };
	int m_iImgResWidth { }, m_iImgResHeight { };              //Width and height of whole image to draw.
	std::vector<std::unique_ptr<CImageList>> m_vecImgRes { }; //Vector for RT_GROUP_ICON/CURSOR.
	std::wstring m_wstrEditBRB; //WString for m_EditBRB.
	CEdit m_EditBRB;            //Edit control for RT_STRING, RT_VERSION, RT_MANIFEST, Debug additional info
	CFont m_fontEditRes;        //Font for m_EditBRB.
	bool m_fDrawRes { false };
};