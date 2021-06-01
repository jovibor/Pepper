/****************************************************************************************************
* Copyright © 2018-2021 Jovibor https://github.com/jovibor/   										*
* This software is available under the "MIT License".                                               *
* https://github.com/jovibor/Pepper/blob/master/LICENSE												*
* Pepper - PE (x86) and PE+ (x64) files viewer, based on libpe: https://github.com/jovibor/Pepper	*
* libpe - Windows library for reading PE (x86) and PE+ (x64) files inner structure information.		*
* https://github.com/jovibor/libpe																	*
****************************************************************************************************/
#pragma once

class CPepperApp : public CWinAppEx
{	
public:
	CPepperApp();
	void OpenNewFile();
private:
	BOOL InitInstance() override;
	int ExitInstance() override;
	void PreLoadState() override;
	afx_msg void OnAppAbout();
	void OnFileOpen();
	BOOL m_bHiColorIcons;
	DECLARE_MESSAGE_MAP()
};