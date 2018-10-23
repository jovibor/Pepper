#pragma once
using namespace libpe;

class CPepperDoc : public CDocument
{
public:
	virtual ~CPepperDoc() {};
	CPepperDoc();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	Ilibpe* m_pLibpe { };
protected: // create from serialization only
	DECLARE_DYNCREATE(CPepperDoc)
	DECLARE_MESSAGE_MAP()
	std::map<DWORD, std::wstring> m_mapLibpeErrors { };
};
