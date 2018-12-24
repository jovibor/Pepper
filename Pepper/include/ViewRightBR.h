#pragma once
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "ListEx.h"
#include <memory>

class CViewRightBR : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightBR)
protected:
	CViewRightBR() {}
	virtual ~CViewRightBR() {}
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	void ShowResource(RESHELPER*);
	DECLARE_MESSAGE_MAP()
private:
	CWnd* m_pActiveWnd { };
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEXINFO m_stListInfo;
	CListEx m_stListTLSCallbacks;
	LOGFONT m_lf { }, m_hdrlf { };
	CImageList m_stImgRes;
	bool m_fDrawRes { false };
	COLORREF m_clrBk { RGB(230, 230, 230) };
	COLORREF m_clrBkImgList { RGB(250, 250, 250) };
	//HWND for RT_DIALOG.
	BITMAP m_stBmp { };
	int m_iResTypeToDraw { };
	//Width and height of whole image to draw.
	int m_iImgResWidth { }, m_iImgResHeight { };
	//Vector for RT_GROUP_ICON/CURSOR.
	std::vector<std::unique_ptr<CImageList>> m_vecImgRes { };
	std::wstring m_strRes;
	std::wstring m_strResLoadError { L"Unable to load resource! It's either damaged, packed or zero-length." };
	CEdit m_stEditResStrings; //Edit control for RT_STRING, RT_VERSION
	CFont m_fontEditRes; //Font for m_stEditResStrings.
	SIZE m_sizeLetter, m_sizeStrStyles;
	int m_iResDlgIndentToDrawX { 10 };
	int m_iResDlgIndentToDrawY { 2 };
	std::map<int, std::wstring> m_mapVerInfoStrings {
		{ 0, L"FileDescription" },
	{ 1, L"FileVersion" },
	{ 2, L"InternalName" },
	{ 3, L"CompanyName" },
	{ 4, L"LegalCopyright" },
	{ 5, L"OriginalFilename" },
	{ 6, L"ProductName" },
	{ 7, L"ProductVersion" }
	};
	const std::map<DWORD, std::wstring> m_mapDlgStyles {
		{ DS_ABSALIGN, L"DS_ABSALIGN" },
	{ DS_SYSMODAL, L"DS_SYSMODAL" },
	{ DS_LOCALEDIT, L"DS_LOCALEDIT" },
	{ DS_SETFONT, L"DS_SETFONT" },
	{ DS_MODALFRAME, L"DS_MODALFRAME" },
	{ DS_NOIDLEMSG, L"DS_NOIDLEMSG" },
	{ DS_SETFOREGROUND, L"DS_SETFOREGROUND" },
	{ DS_3DLOOK, L"DS_3DLOOK" },
	{ DS_FIXEDSYS, L"DS_FIXEDSYS" },
	{ DS_NOFAILCREATE, L"DS_NOFAILCREATE" },
	{ DS_CONTROL, L"DS_CONTROL" },
	{ DS_CENTER, L"DS_CENTER" },
	{ DS_CENTERMOUSE, L"DS_CENTERMOUSE" },
	{ DS_CONTEXTHELP, L"DS_CONTEXTHELP" },
	{ 0x8000L, L"DS_USEPIXELS" },
	{ WS_OVERLAPPED, L"WS_OVERLAPPED" },
	{ WS_POPUP, L"WS_POPUP" },
	{ WS_CHILD, L"WS_CHILD" },
	{ WS_MINIMIZE, L"WS_MINIMIZE" },
	{ WS_VISIBLE, L"WS_VISIBLE" },
	{ WS_DISABLED, L"WS_DISABLED" },
	{ WS_CLIPSIBLINGS, L"WS_CLIPSIBLINGS" },
	{ WS_CLIPCHILDREN, L"WS_CLIPCHILDREN" },
	{ WS_MAXIMIZE, L"WS_MAXIMIZE" },
	{ WS_CAPTION, L"WS_CAPTION" },
	{ WS_BORDER, L"WS_BORDER" },
	{ WS_DLGFRAME, L"WS_DLGFRAME" },
	{ WS_VSCROLL, L"WS_VSCROLL" },
	{ WS_HSCROLL, L"WS_HSCROLL" },
	{ WS_SYSMENU, L"WS_SYSMENU" },
	{ WS_THICKFRAME, L"WS_THICKFRAME" },
	{ WS_GROUP, L"WS_GROUP" },
	{ WS_TABSTOP, L"WS_TABSTOP" },
	{ WS_MINIMIZEBOX, L"WS_MINIMIZEBOX" },
	{ WS_MAXIMIZEBOX, L"WS_MAXIMIZEBOX" },
	{ WS_TILED, L"WS_TILED" },
	{ WS_ICONIC, L"WS_ICONIC" },
	{ WS_SIZEBOX, L"WS_SIZEBOX" },
	{ WS_TILEDWINDOW, L"WS_TILEDWINDOW" }
	};
private:
	int CreateListTLSCallbacks();
	void ResLoadError();
};

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