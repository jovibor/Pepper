#include "stdafx.h"
#include "PepperDoc.h"

IMPLEMENT_DYNCREATE(CPepperDoc, CDocument)

BEGIN_MESSAGE_MAP(CPepperDoc, CDocument)
END_MESSAGE_MAP()

BOOL CPepperDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (Getlibpe(&m_pLibpe) != S_OK)
	{
		MessageBoxW(0, L"Getlibpe() failed", L"Error", MB_ICONERROR);
		return FALSE;
	}

	HRESULT hr;
	if ((hr = m_pLibpe->LoadPe(lpszPathName)) != S_OK)
	{
		TCHAR str[MAX_PATH] { };
		swprintf_s(str, L"File load failed with error code: 0x0%X", hr);
		MessageBoxW(0, str, L"File Load Failed", MB_ICONERROR);
		return FALSE;
	}

	UpdateAllViews(0);

	return TRUE;
}

void CPepperDoc::OnCloseDocument()
{
	if (m_pLibpe)
		m_pLibpe->Release();

	CDocument::OnCloseDocument();
}