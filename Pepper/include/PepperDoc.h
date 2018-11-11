#pragma once
using namespace libpe;

class CPepperDoc : public CDocument
{
public:	
	Ilibpe* m_pLibpe { };
private:
	DECLARE_DYNCREATE(CPepperDoc)
	CPepperDoc() {}
	virtual ~CPepperDoc() {}
	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	void OnCloseDocument() override;
	DECLARE_MESSAGE_MAP()
};
