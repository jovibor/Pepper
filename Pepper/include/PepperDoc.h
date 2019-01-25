#pragma once
#include "libpe.h"
#include "FileLoader.h"
using namespace libpe;

class CPepperDoc : public CDocument
{
public:	
	DECLARE_DYNCREATE(CPepperDoc)
	libpe_ptr m_pLibpe { };
	CFileLoader m_stFileLoader;
private:
	CPepperDoc() {}
	virtual ~CPepperDoc() {}
	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	DECLARE_MESSAGE_MAP()
};