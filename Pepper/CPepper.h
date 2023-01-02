/****************************************************************************************************
* Copyright © 2018-2023 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once

class CPepperApp : public CWinAppEx
{
public:
	CPepperApp();
	void OpenNewFile();
private:
	DECLARE_MESSAGE_MAP();
	BOOL InitInstance()override;
	int ExitInstance()override;
	afx_msg void OnAppAbout();
	void OnFileOpen();
	afx_msg void OnUpdateHelpAbout(CCmdUI *pCmdUI);
private:
	BOOL m_bHiColorIcons;
};