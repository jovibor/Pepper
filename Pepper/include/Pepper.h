#pragma once

class CPepperApp : public CWinAppEx
{	
public:
	CPepperApp();
private:
	BOOL InitInstance() override;
	int ExitInstance() override;
	void PreLoadState() override;
	afx_msg void OnAppAbout();
	void OnFileOpen();
	BOOL m_bHiColorIcons;
	DECLARE_MESSAGE_MAP()
};