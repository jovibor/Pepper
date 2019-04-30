/********************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/					*
* Github repository URL: https://github.com/jovibor/ListEx						*
* This software is available under the "MIT License".							*
* This is an extended and featured version of CMFCListCtrl class.				*
* CListEx - list control class with the ability to set tooltips on arbitrary	*
* cells, and also with a lots of other stuff to customize your control in many	*
* different aspects. For more info see official documentation on github.		*
********************************************************************************/
#pragma once
#include "../ListEx.h"
#include "CListExHdr.h"
#include <unordered_map>

namespace LISTEX {
	/********************************************
	* CListEx class definition.					*
	********************************************/
	class CListEx : public IListEx
	{
	public:
		DECLARE_DYNAMIC(CListEx)
		CListEx() {}
		virtual ~CListEx() {}
		bool Create(const LISTEXCREATESTRUCT& lcs);
		void CreateDialogCtrl();
		bool IsCreated();
		void SetColor(const LISTEXCOLORSTRUCT& lcs);
		void SetFont(const LOGFONTW* pLogFontNew);
		void SetFontSize(UINT uiSize);
		UINT GetFontSize();
		void SetCellTooltip(int iItem, int iSubitem, const wchar_t* pwszTooltip, const wchar_t* pwszCaption = nullptr);
		void SetCellMenu(int iItem, int iSubitem, CMenu* pMenu);
		void SetListMenu(CMenu* pMenu);
		void SetCellData(int iItem, int iSubitem, DWORD_PTR dwData);
		DWORD_PTR GetCellData(int iItem, int iSubitem);
		void SetHeaderHeight(DWORD dwHeight);
		void SetHeaderFont(const LOGFONT* pLogFontNew);
		void SetHeaderColumnColor(DWORD nColumn, COLORREF clr);
		void Destroy() override;
		DECLARE_MESSAGE_MAP()
	protected:
		CListExHdr& GetHeaderCtrl() override { return m_stListHeader; }
		void InitHeader() override;
		bool HasTooltip(int iItem, int iSubitem, std::wstring** ppwstrText = nullptr, std::wstring** ppwstrCaption = nullptr);
		bool HasMenu(int iItem, int iSubitem, CMenu** ppMenu = nullptr);
		void DrawItem(LPDRAWITEMSTRUCT) override;
		afx_msg void OnPaint();
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg void OnKillFocus(CWnd* pNewWnd);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint pt);
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint pt);
		virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
		afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnTimer(UINT_PTR nIDEvent);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
		afx_msg void OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnHdnTrack(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnDestroy();
	private:
		bool m_fCreated { false };
		CListExHdr m_stListHeader;
		LISTEXCOLORSTRUCT m_stColor { };
		CFont m_fontList;
		CPen m_penGrid;
		HWND m_hwndTt { };
		TOOLINFO m_stToolInfo { };
		bool m_fTtShown { false };
		LVHITTESTINFO m_stCurrCell { };
		DWORD m_dwGridWidth { 1 };		//Grid width.
		CMenu* m_pListMenu { };			//List global menu, if set.
		std::unordered_map<int, std::unordered_map<int,
			std::tuple<std::wstring/*tip text*/, std::wstring/*caption text*/>>> m_umapCellTt { }; //Cell's tooltips.
		std::unordered_map<int, std::unordered_map<int, CMenu*>> m_umapCellMenu { };			//Cell's menus.
		std::unordered_map<int, std::unordered_map<int, DWORD_PTR>> m_umapCellData { };			//Cell's custom data.
		NMITEMACTIVATE m_stNMII { };
		const ULONG_PTR ID_TIMER_TOOLTIP { 0x01 };
	};

	/*******************Setting a manifest for ComCtl32.dll version 6.***********************/
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
}