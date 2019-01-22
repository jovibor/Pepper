#pragma once
#include "libpe.h"
#include"HexCtrl.h"

using namespace libpe;
using namespace HEXControl;

class CPepperDoc : public CDocument
{
public:	
	DECLARE_DYNCREATE(CPepperDoc)
	libpe_ptr m_pLibpe { };	
	CHexCtrl m_stHexFloat;
private:
	CPepperDoc() {}
	virtual ~CPepperDoc() {}
	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	DECLARE_MESSAGE_MAP()
};