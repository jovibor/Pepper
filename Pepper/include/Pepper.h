#pragma once
#include "resource.h"       // main symbols

class CPepperApp : public CWinAppEx
{
public:
	CPepperApp();
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	BOOL  m_bHiColorIcons;
	virtual void PreLoadState();
	afx_msg void OnAppAbout();
	void OnFileOpen();
	DECLARE_MESSAGE_MAP()
};

extern CPepperApp theApp;
