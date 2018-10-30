#pragma once

class CPepperApp : public CWinAppEx
{
public:
	CPepperApp();
	BOOL InitInstance() override;
	int ExitInstance() override;
	BOOL  m_bHiColorIcons;
	void PreLoadState() override;
	afx_msg void OnAppAbout();
	void OnFileOpen();
	DECLARE_MESSAGE_MAP()
};

extern CPepperApp theApp;
