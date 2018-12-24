#pragma once
#include <map>
#include "libpe.h"
using namespace libpe;

//Errors, that might come from libpe.
inline const std::map<DWORD, std::wstring> g_mapLibpeErrors {
	{ E_CALL_LOADPE_FIRST, L"E_CALL_LOADPE_FIRST" },
{ E_FILE_OPEN_FAILED, L"E_FILE_OPEN_FAILED" },
{ E_FILE_SIZE_TOO_SMALL, L"E_FILE_SIZE_TOO_SMALL" },
{ E_FILE_CREATE_FILE_MAPPING_FAILED, L"E_FILE_CREATE_FILE_MAPPING_FAILED" },
{ E_FILE_MAP_VIEW_OF_FILE_FAILED, L"E_FILE_MAP_VIEW_OF_FILE_FAILED" },
{ E_FILE_SECTION_DATA_CORRUPTED, L"E_FILE_SECTION_DATA_CORRUPTED" },
{ E_IMAGE_TYPE_UNSUPPORTED, L"E_IMAGE_TYPE_UNSUPPORTED" },
{ E_IMAGE_HAS_NO_DOSHEADER, L"E_IMAGE_HAS_NO_DOSHEADER" },
{ E_IMAGE_HAS_NO_RICHHEADER, L"E_IMAGE_HAS_NO_RICHHEADER" },
{ E_IMAGE_HAS_NO_NTHEADER, L"E_IMAGE_HAS_NO_NTHEADER" },
{ E_IMAGE_HAS_NO_FILEHEADER, L"E_IMAGE_HAS_NO_FILEHEADER" },
{ E_IMAGE_HAS_NO_OPTHEADER, L"E_IMAGE_HAS_NO_OPTHEADER" },
{ E_IMAGE_HAS_NO_DATADIRECTORIES, L"E_IMAGE_HAS_NO_DATADIRECTORIES" },
{ E_IMAGE_HAS_NO_SECTIONS, L"E_IMAGE_HAS_NO_SECTIONS" },
{ E_IMAGE_HAS_NO_EXPORT, L"E_IMAGE_HAS_NO_EXPORT" },
{ E_IMAGE_HAS_NO_IMPORT, L"E_IMAGE_HAS_NO_IMPORT" },
{ E_IMAGE_HAS_NO_RESOURCE, L"E_IMAGE_HAS_NO_RESOURCE" },
{ E_IMAGE_HAS_NO_EXCEPTION, L"E_IMAGE_HAS_NO_EXCEPTION" },
{ E_IMAGE_HAS_NO_SECURITY, L"E_IMAGE_HAS_NO_SECURITY" },
{ E_IMAGE_HAS_NO_BASERELOC, L"E_IMAGE_HAS_NO_BASERELOC" },
{ E_IMAGE_HAS_NO_DEBUG, L"E_IMAGE_HAS_NO_DEBUG" },
{ E_IMAGE_HAS_NO_ARCHITECTURE, L"E_IMAGE_HAS_NO_ARCHITECTURE" },
{ E_IMAGE_HAS_NO_GLOBALPTR, L"E_IMAGE_HAS_NO_GLOBALPTR" },
{ E_IMAGE_HAS_NO_TLS, L"E_IMAGE_HAS_NO_TLS" },
{ E_IMAGE_HAS_NO_LOADCONFIG, L"E_IMAGE_HAS_NO_LOADCONFIG" },
{ E_IMAGE_HAS_NO_BOUNDIMPORT, L"E_IMAGE_HAS_NO_BOUNDIMPORT" },
{ E_IMAGE_HAS_NO_IAT, L"E_IMAGE_HAS_NO_IAT" },
{ E_IMAGE_HAS_NO_DELAYIMPORT, L"E_IMAGE_HAS_NO_DELAYIMPORT" },
{ E_IMAGE_HAS_NO_COMDESCRIPTOR, L"E_IMAGE_HAS_NO_COMDESCRIPTOR" }
};

inline const std::map<WORD, std::wstring> g_mapResType {
	{ 1, L"RT_CURSOR" },
{ 2, L"RT_BITMAP" },
{ 3, L"RT_ICON" },
{ 4, L"RT_MENU" },
{ 5, L"RT_DIALOG" },
{ 6, L"RT_STRING" },
{ 7, L"RT_FONTDIR" },
{ 8, L"RT_FONT" },
{ 9, L"RT_ACCELERATOR" },
{ 10, L"RT_RCDATA" },
{ 11, L"RT_MESSAGETABLE" },
{ 12, L"RT_GROUP_CURSOR" },
{ 14, L"RT_GROUP_ICON" },
{ 16, L"RT_VERSION" },
{ 17, L"RT_DLGINCLUDE" },
{ 19, L"RT_PLUGPLAY" },
{ 20, L"RT_VXD" },
{ 21, L"RT_ANICURSOR" },
{ 22, L"RT_ANIICON" },
{ 23, L"RT_HTML" },
{ 24, L"RT_MANIFEST" },
{ 28, L"RT_RIBBON_XML" },
{ 240, L"RT_DLGINIT" },
{ 241, L"RT_TOOLBAR" }
}; 

struct RESHELPER
{	
	RESHELPER() {}
	RESHELPER(WORD type, WORD name, std::vector<std::byte>* data) :IdResType(type), IdResName(name),pData(data) {}
	WORD IdResType;
	WORD IdResName;
	std::vector<std::byte>* pData;
};

/*****************************************************************
* These are identificators of all the controls: list, hex, tree. *
*****************************************************************/
constexpr auto IDC_LIST_DOSHEADER = 0x0001;
constexpr auto IDC_LIST_RICHHEADER = 0x0002;
constexpr auto IDC_LIST_NTHEADER = 0x0003;
constexpr auto IDC_LIST_FILEHEADER = 0x0004;
constexpr auto IDC_LIST_OPTIONALHEADER = 0x0005;
constexpr auto IDC_LIST_DATADIRECTORIES = 0x0006;
constexpr auto IDC_LIST_SECHEADERS = 0x0007;
constexpr auto IDC_LIST_EXPORT = 0x0008;
constexpr auto IDC_LIST_EXPORT_FUNCS = 0x0009;
constexpr auto IDC_LIST_IMPORT = 0x000A;
constexpr auto IDC_LIST_IMPORT_ENTRY = 0x000B;
constexpr auto IDC_LIST_EXCEPTION = 0x000C;
constexpr auto IDC_LIST_SECURITY = 0x000D;
constexpr auto IDC_LIST_SECURITY_ENTRY = 0x000E;
constexpr auto IDC_LIST_RELOCATIONS = 0x000F;
constexpr auto IDC_LIST_RELOCATIONS_ENTRY = 0x0010;
constexpr auto IDC_LIST_DEBUG = 0x0011;
constexpr auto IDC_LIST_DEBUG_ENTRY = 0x0012;
constexpr auto IDC_LIST_ARCHITECTURE = 0x0013;
constexpr auto IDC_LIST_GLOBALPTR = 0x0014;
constexpr auto IDC_LIST_TLS = 0x0015;
constexpr auto IDC_LIST_TLS_CALLBACKS = 0x0016;
constexpr auto IDC_LIST_LOADCONFIG = 0x0017;
constexpr auto IDC_LIST_BOUNDIMPORT = 0x0018;
constexpr auto IDC_LIST_IAT = 0x0019;
constexpr auto IDC_LIST_DELAYIMPORT = 0x001A;
constexpr auto IDC_LIST_DELAYIMPORT_FUNCS = 0x001B;
constexpr auto IDC_LIST_COMDESCRIPTOR = 0x001C;

constexpr auto IDC_TREE_LEFT_MAIN = 0x0100;
constexpr auto IDC_TREE_RESOURCE = 0x0101;
constexpr auto IDC_TREE_RESOURCE_TOP = 0x0102;
constexpr auto IDC_TREE_RESOURCE_BOTTOM = 0x0103;

constexpr auto IDC_HEX_RIGHT_BOTTOM_LEFT = 0x0200;
constexpr auto IDC_HEX_RIGHT_TR = 0x0201;

constexpr auto IDC_SHOW_FILE_SUMMARY = 0x0300;
constexpr auto IDC_SHOW_RESOURCE_RBR = 0x0301;
/********************************************************
* End of IDC.											*
********************************************************/