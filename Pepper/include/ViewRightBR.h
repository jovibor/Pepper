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

class CViewRightBR : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightBR)
private:
	/****************************************************************
	* Struct for RT_GROUP_ICON/CURSOR.								*
	****************************************************************/
#pragma pack( push, 2 )
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
#pragma pack( pop )

	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	};

	//Helper struct. Not completed.
	struct DLGTEMPLATEEX
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
	};
protected:
	CViewRightBR() {}
	virtual ~CViewRightBR() {}
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	void ShowResource(RESHELPER*);
	int CreateListTLSCallbacks();
	void ResLoadError();
	DECLARE_MESSAGE_MAP()
private:
	HWND m_hwndActive { };
	libpe_ptr m_pLibpe;
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEXCREATESTRUCT m_stlcs;
	IListExPtr m_stListTLSCallbacks { CreateListEx() };
	LOGFONT m_lf { }, m_hdrlf { };
	CImageList m_stImgRes;
	bool m_fDrawRes { false };
	COLORREF m_clrBkIcons { RGB(230, 230, 230) };
	COLORREF m_clrBkImgList { RGB(250, 250, 250) };
	BITMAP m_stBmp { };
	int m_iResTypeToDraw { };
	//Width and height of whole image to draw.
	int m_iImgResWidth { }, m_iImgResHeight { };
	//Vector for RT_GROUP_ICON/CURSOR.
	std::vector<std::unique_ptr<CImageList>> m_vecImgRes { };
	std::wstring m_wstrRes;
	CEdit m_stEditResStrings; //Edit control for RT_STRING, RT_VERSION
	CFont m_fontEditRes; //Font for m_stEditResStrings.
	SIZE m_sizeLetter { }, m_sizeStrStyles { };
	const int m_iResDlgIndentToDrawX { 10 };
	const int m_iResDlgIndentToDrawY { 2 };
};