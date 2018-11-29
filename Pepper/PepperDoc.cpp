#include "stdafx.h"
#include "PepperDoc.h"

IMPLEMENT_DYNCREATE(CPepperDoc, CDocument)

BEGIN_MESSAGE_MAP(CPepperDoc, CDocument)
END_MESSAGE_MAP()

BOOL CPepperDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (Getlibpe(m_pLibpe) != S_OK)
	{
		MessageBoxW(nullptr, L"Getlibpe() failed.", L"Error.", MB_ICONERROR);
		return FALSE;
	}

	HRESULT hr;
	if ((hr = m_pLibpe->LoadPe(lpszPathName)) != S_OK)
	{
		WCHAR str[MAX_PATH] { };
		const auto it = g_mapLibpeErrors.find(hr);
		if (it != g_mapLibpeErrors.end())
			swprintf_s(str, L"File load failed with libpe error code: 0x0%X\n%s", hr, it->second.data());
		else
			swprintf_s(str, L"File load failed with libpe error code: 0x0%X", hr);

		MessageBoxW(nullptr, str, L"File load failed.", MB_ICONERROR);

		return FALSE;
	}

	UpdateAllViews(nullptr);

	return TRUE;
}