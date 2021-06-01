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

using namespace LISTEX;

class CWndSampledlg final : public CWnd
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
	DECLARE_DYNCREATE(CViewRightBR)
	virtual void OnDraw(CDC* pDC);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();     // first time after construct
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	void CreateIconCursor(const SRESHELPER* pResHelper);
	void CreateBitmap(const SRESHELPER* pResHelper);
	void CreateDebugEntry(DWORD dwEntry);
	void CreateDlg(const SRESHELPER* pResHelper);
	void CreateStrings(const SRESHELPER* pResHelper);
	void CreateGroupIconCursor(const SRESHELPER* pResHelper);
	void CreateVersion(const SRESHELPER* pResHelper);
	void CreateManifest(const SRESHELPER* pResHelper);
	void CreateToolbar(const SRESHELPER* pResHelper);
	int CreateListTLSCallbacks();
	void ParceDlgTemplate(PBYTE pDataDlgRes, size_t nSize, std::wstring& wstrData);
	void ResLoadError();
	void ShowResource(const SRESHELPER*);
	DECLARE_MESSAGE_MAP()
private:
	HWND m_hwndActive { };
	libpe_ptr m_pLibpe;
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CImageList m_stImgRes;
	CWndSampledlg m_wndSampledlg;
	LISTEXCREATESTRUCT m_stlcs;
	IListExPtr m_stListTLSCallbacks { CreateListEx() };
	LOGFONTW m_lf { }, m_hdrlf { };
	bool m_fDrawRes { false };
	COLORREF m_clrBkIcons { RGB(230, 230, 230) };
	COLORREF m_clrBkImgList { RGB(250, 250, 250) };
	BITMAP m_stBmp { };
	int m_iResTypeToDraw { };
	int m_iImgResWidth { }, m_iImgResHeight { };              //Width and height of whole image to draw.
	std::vector<std::unique_ptr<CImageList>> m_vecImgRes { }; //Vector for RT_GROUP_ICON/CURSOR.
	std::wstring m_wstrEditBRB; //WString for m_EditBRB.
	CEdit m_EditBRB;            //Edit control for RT_STRING, RT_VERSION, RT_MANIFEST, Debug additional info
	CFont m_fontEditRes;        //Font for m_EditBRB.
};