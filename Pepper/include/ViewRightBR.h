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
	int ShowResource(std::vector<std::byte>* pData, UINT uResType);
	DECLARE_MESSAGE_MAP()
private:
	CWnd* m_pActiveList { };
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
	HWND m_hwndRes { };
	BITMAP m_stBmp { };
	int m_iResTypeToDraw { };
	//Width and height of whole image to draw.
	int m_iImgResWidth { }, m_iImgResHeight { };
	//Vector for RT_GROUP_ICON/CURSOR.
	std::vector<std::unique_ptr<CImageList>> m_vecImgRes { };
private:
	int CreateListTLSCallbacks();
};

/****************************************************************
* Struct for RT_GROUP_ICON/CURSOR.								*
****************************************************************/
#pragma pack( push, 2 )
typedef struct
{
	BYTE   bWidth;               // Width, in pixels, of the image
	BYTE   bHeight;              // Height, in pixels, of the image
	BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
	BYTE   bReserved;            // Reserved
	WORD   wPlanes;              // Color Planes
	WORD   wBitCount;            // Bits per pixel
	DWORD  dwBytesInRes;         // how many bytes in this resource?
	WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;
typedef struct
{
	WORD			  idReserved;   // Reserved (must be 0)
	WORD			  idType;	    // Resource type (1 for icons)
	WORD			  idCount;	    // How many images?
	GRPICONDIRENTRY   idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;
#pragma pack( pop )