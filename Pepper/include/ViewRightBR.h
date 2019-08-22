/****************************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/										*
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
#include "constants.h"

using namespace LISTEX;

class CWndDlgSample : public CWnd
{
public:
	CWndDlgSample(CImageList* pImgList) { m_pImgRes = pImgList; }
	virtual	~CWndDlgSample() {}
	DECLARE_MESSAGE_MAP()
private:
	afx_msg void OnPaint();
private:
	CImageList* m_pImgRes { };
};

class CViewRightBR : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightBR)
private:
	/****************************************************************
	* Struct for RT_GROUP_ICON/CURSOR.								*
	****************************************************************/
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
#pragma pack(push, 4)
	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	};
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

protected:
	CViewRightBR() :m_wndDlgSample(&m_stImgRes) {} //Initializing Dialog Sample Window's image list.
	virtual ~CViewRightBR() {}
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	void ShowResource(RESHELPER*);
	void ParceDlgTemplate(PBYTE pDataDlgRes, size_t nSize);
	int CreateListTLSCallbacks();
	void ResLoadError();
	void CreateDebugEntry(DWORD dwEntry);
	DECLARE_MESSAGE_MAP()
private:
	HWND m_hwndActive { };
	libpe_ptr m_pLibpe;
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	DWORD m_dwStyles { WS_POPUP | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX };
	DWORD m_dwExStyles { WS_EX_APPWINDOW };
	CImageList m_stImgRes;
	CWndDlgSample m_wndDlgSample;
	LISTEXCREATESTRUCT m_stlcs;
	IListExPtr m_stListTLSCallbacks { CreateListEx() };
	LOGFONT m_lf { }, m_hdrlf { };
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