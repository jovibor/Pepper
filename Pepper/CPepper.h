/*****************************************************************
* Copyright © 2018-present Jovibor https://github.com/jovibor/   *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.  *
* Official git repository: https://github.com/jovibor/Pepper/    *
* This software is available under the Apache-2.0 License.       *
*****************************************************************/
#pragma once

class CPepperApp : public CWinAppEx {
public:
	void OpenNewFile();
private:
	BOOL InitInstance()override;
	int ExitInstance()override;
	afx_msg void OnAppAbout();
	void OnFileOpen();
	afx_msg void OnUpdateHelpAbout(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP();
};