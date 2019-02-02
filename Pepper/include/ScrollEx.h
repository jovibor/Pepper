#pragma once
#include <afxwin.h>
#include "ScrollExRes.h"

namespace HEXCTRL {

	class CScrollEx : public CWnd
	{
	public:
		CScrollEx() {}
		~CScrollEx() {}
		bool Create(CWnd* pWnd, int iScrollType, ULONGLONG ullScrolline, ULONGLONG ullScrollPage, ULONGLONG ullScrollSizeMax);
		CWnd* GetParent() { return m_pwndParent; }
		ULONGLONG SetScrollPos(ULONGLONG);
		void ScrollLineDown();
		void ScrollLineRight();
		void ScrollLineUp();
		void ScrollLineLeft();
		void ScrollPageDown();
		void ScrollPageUp();
		void ScrollHome();
		void ScrollEnd();
		void SetScrollSizes(ULONGLONG ullScrolline, ULONGLONG ullScrollPage, ULONGLONG ullScrollSizeMax);
		ULONGLONG GetScrollPos();
		LONGLONG GetScrollPosDelta(ULONGLONG& ullCurrPos, ULONGLONG& ullPrevPos);
		bool IsVisible() { return m_fVisible; }
		BOOL OnNcActivate(BOOL bActive);
		void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
		void OnNcPaint();
		void OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		void OnLButtonUp(UINT nFlags, CPoint point);
		void OnMouseMove(UINT nFlags, CPoint point);
	protected:
		DECLARE_MESSAGE_MAP()
		void SendParentScrollMsg();
		void DrawScrollBar();
		void DrawArrows(CDC* pDC);
		void DrawThumb(CDC* pDC);
		long double GetThumbScrollingSize();
		CRect GetThumbRect();
		CRect GetFirstArrowRect();
		CRect GetLastArrowRect();
		CRect GetFirstChannelRect();
		CRect GetLastChannelRect();
		CRect GetScrollRect(bool fWithNCArea = false);
		CRect GetScrollWorkAreaRect();
		UINT GetScrollSizeWH();
		CRect GetParentRect();
		UINT GetScrollWorkAreaSizeWH(); //Scroll area size (WH) without arrow buttons.
		UINT GetThumbSizeWH();
		bool IsVert();
		bool IsThumbDragging();
		void SetThumbPos(int uiPos);
		UINT GetThumbPos();
		void ResetTimers();
		afx_msg void OnTimer(UINT_PTR nIDEvent);
		enum SCROLLSTATE
		{
			FIRSTBUTTON_HOVER = 1,
			FIRSTBUTTON_CLICK = 2,
			FIRSTCHANNEL_CLICK = 3,
			THUMB_HOVER = 4,
			THUMB_CLICK = 5,
			LASTCHANNEL_CLICK = 6,
			LASTBUTTON_CLICK = 7,
			LASTBUTTON_HOVER = 8
		};
	protected:
		CWnd* m_pwndParent { };
		UINT m_uiScrollBarSizeWH { };
		int m_iScrollType { };
		int m_iScrollBarState { };
		COLORREF m_clrBkNC { GetSysColor(COLOR_3DFACE) };
		//COLORREF m_clrBkNC { RGB(0, 250, 0) };
		COLORREF m_clrBkScrollBar { RGB(241, 241, 241) };
		COLORREF m_clrThumb { RGB(192, 192, 192) };
		CPoint m_ptCursorCur { };
		ULONGLONG m_ullScrollPosCur { 0 };
		ULONGLONG m_ullScrollPosPrev { };
		ULONGLONG m_ullScrollLine { };
		ULONGLONG m_ullScrollPage { };
		ULONGLONG m_ullScrollSizeMax { };

		//Timers:
		static constexpr auto IDT_FIRSTCLICK = 0x7ff0;
		const int TIMER_TIME_FIRSTCLICK = 200;
		static constexpr auto IDT_CLICKREPEAT = 0x7ff1;
		const int TIMER_TIME_REPEAT = 50;

		//Bitmap related:
		CBitmap m_bmpScroll;
		const unsigned m_iFirstBtnOffset { 0 };
		const unsigned m_iFirstChannelOffset { 17 };
		const unsigned m_iThumbOffset { 27 };
		const unsigned m_iLastChannelOffset { 51 };
		const unsigned m_iLastBtnOffset { 61 };
		const unsigned m_iFirstBtnSize { 17 };
		const unsigned m_iFirstChannelSize { 10 };
		const unsigned m_iThumbSize { 24 };
		const unsigned m_iLastChannelSize { 10 };
		const unsigned m_iLastBtnSize { 17 };
		bool m_fCreated { false };
		bool m_fVisible { false };
	};
}