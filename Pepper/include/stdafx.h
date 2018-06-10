#pragma once
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#define HAS_EXCEPTIONS 0
#include <SDKDDKVer.h>
//#define ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

#include <vector>
#include <tuple>
#include <map>

#include "..\..\..\libpe\libpe\include\libpe.h"
#include "Strsafe.h"
#include "defines.h"
