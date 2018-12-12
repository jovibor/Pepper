#pragma once
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#define WINVER 0x0600

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include <map>
#include "strsafe.h"
#include "constants.h"