#pragma once
#include "libpe.h"
using namespace libpe;

class CPepperDoc : public CDocument
{
public:	
	libpe_ptr m_pLibpe { };
private:
	DECLARE_DYNCREATE(CPepperDoc)
	CPepperDoc() {}
	virtual ~CPepperDoc() {}
	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	DECLARE_MESSAGE_MAP()
};