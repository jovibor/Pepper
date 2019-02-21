#pragma once
#include <map>
#include "libpe.h"
using namespace libpe;

//Errors, that might come from libpe.
inline const std::map<DWORD, std::wstring> g_mapLibpeErrors {
	{ E_CALL_LOADPE_FIRST, L"E_CALL_LOADPE_FIRST" },
{ E_FILE_OPEN_FAILED, L"E_FILE_OPEN_FAILED" },
{ E_FILE_SIZE_TOO_SMALL, L"E_FILE_SIZE_TOO_SMALL" },
{ E_FILE_CREATEFILEMAPPING_FAILED, L"E_FILE_CREATEFILEMAPPING_FAILED" },
{ E_FILE_MAPVIEWOFFILE_FAILED, L"E_FILE_MAPVIEWOFFILE_FAILED" },
{ E_FILE_MAPVIEWOFFILE_SECTION_FAILED, L"E_FILE_MAPVIEWOFFILE_SECTION_FAILED" },
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

struct STRUCTHELPER
{
	DWORD dwSize;
	DWORD dwOffset;
	std::wstring strField;
};

using map_hdr = const std::map<DWORD, STRUCTHELPER>;

inline map_hdr g_mapDOS {
	{ 0, { sizeof(WORD), 0, L"e_magic" } },
{ 1, { sizeof(WORD), 2, L"e_cblp" } },
{ 2, { sizeof(WORD), 4, L"e_cp" } },
{ 3, { sizeof(WORD), 6, L"e_crlc" } },
{ 4, { sizeof(WORD), 8, L"e_cparhdr" } },
{ 5, { sizeof(WORD), 10, L"e_minalloc" } },
{ 6, { sizeof(WORD), 12, L"e_maxalloc" } },
{ 7, { sizeof(WORD), 14, L"e_ss" } },
{ 8, { sizeof(WORD), 16, L"e_sp" } },
{ 9, { sizeof(WORD), 18, L"e_csum" } },
{ 10, { sizeof(WORD), 20, L"e_ip" } },
{ 11, { sizeof(WORD), 22, L"e_cs" } },
{ 12, { sizeof(WORD), 24, L"e_lfarlc" } },
{ 13, { sizeof(WORD), 26, L"e_ovno" } },
{ 14, { sizeof(WORD), 28, L"e_res[0]" } },
{ 15, { sizeof(WORD), 30, L"   e_res[1]" } },
{ 16, { sizeof(WORD), 32, L"   e_res[2]" } },
{ 17, { sizeof(WORD), 34, L"   e_res[3]" } },
{ 18, { sizeof(WORD), 36, L"e_oemid" } },
{ 19, { sizeof(WORD), 38, L"e_oeminfo" } },
{ 20, { sizeof(WORD), 40, L"e_res2[0]" } },
{ 21, { sizeof(WORD), 42, L"   e_res2[1]" } },
{ 22, { sizeof(WORD), 44, L"   e_res2[2]" } },
{ 23, { sizeof(WORD), 46, L"   e_res2[3]" } },
{ 24, { sizeof(WORD), 48, L"   e_res2[4]" } },
{ 25, { sizeof(WORD), 50, L"   e_res2[5]" } },
{ 26, { sizeof(WORD), 52, L"   e_res2[6]" } },
{ 27, { sizeof(WORD), 54, L"   e_res2[7]" } },
{ 28, { sizeof(WORD), 56, L"   e_res2[8]" } },
{ 29, { sizeof(WORD), 58, L"   e_res2[9]" } },
{ 30, { sizeof(LONG), 60, L"e_lfanew" } }
};

inline map_hdr g_mapLCD32 {
	{ 0, { sizeof(DWORD), 0, L"Size" } },
{ 1, { sizeof(DWORD), 4, L"TimeDateStamp" } },
{ 2, { sizeof(WORD), 8, L"MajorVersion" } },
{ 3, { sizeof(WORD), 10, L"MinorVersion" } },
{ 4, { sizeof(DWORD), 12, L"GlobalFlagsClear" } },
{ 5, { sizeof(DWORD), 16, L"GlobalFlagsSet" } },
{ 6, { sizeof(DWORD), 20, L"CriticalSectionDefaultTimeout" } },
{ 7, { sizeof(DWORD), 24, L"DeCommitFreeBlockThreshold" } },
{ 8, { sizeof(DWORD), 28, L"DeCommitTotalFreeThreshold" } },
{ 9, { sizeof(DWORD), 32, L"LockPrefixTable" } },
{ 10, { sizeof(DWORD), 36, L"MaximumAllocationSize" } },
{ 11, { sizeof(DWORD), 40, L"VirtualMemoryThreshold" } },
{ 12, { sizeof(DWORD), 44, L"ProcessHeapFlags" } },
{ 13, { sizeof(DWORD), 48, L"ProcessAffinityMask" } },
{ 14, { sizeof(WORD), 52, L"CSDVersion" } },
{ 15, { sizeof(WORD), 54, L"DependentLoadFlags" } },
{ 16, { sizeof(DWORD), 56, L"EditList" } },
{ 17, { sizeof(DWORD), 60, L"SecurityCookie" } },
{ 18, { sizeof(DWORD), 64, L"SEHandlerTable" } },
{ 19, { sizeof(DWORD), 68, L"SEHandlerCount" } },
{ 20, { sizeof(DWORD), 72, L"GuardCFCheckFunctionPointer" } },
{ 21, { sizeof(DWORD), 76, L"GuardCFDispatchFunctionPointer" } },
{ 22, { sizeof(DWORD), 80, L"GuardCFFunctionTable" } },
{ 23, { sizeof(DWORD), 84, L"GuardCFFunctionCount" } },
{ 24, { sizeof(DWORD), 88, L"GuardFlags" } },
{ 25, { sizeof(WORD), 92, L"CodeIntegrity.Flags" } },
{ 26, { sizeof(WORD), 94, L"CodeIntegrity.Catalog" } },
{ 27, { sizeof(DWORD), 96, L"CodeIntegrity.CatalogOffset" } },
{ 28, { sizeof(DWORD), 100, L"CodeIntegrity.Reserved" } },
{ 29, { sizeof(DWORD), 104, L"GuardAddressTakenIatEntryTable" } },
{ 30, { sizeof(DWORD), 108, L"GuardAddressTakenIatEntryCount" } },
{ 31, { sizeof(DWORD), 112, L"GuardLongJumpTargetTable" } },
{ 32, { sizeof(DWORD), 116, L"GuardLongJumpTargetCount" } },
{ 33, { sizeof(DWORD), 120, L"DynamicValueRelocTable" } },
{ 34, { sizeof(DWORD), 124, L"CHPEMetadataPointer" } },
{ 35, { sizeof(DWORD), 128, L"GuardRFFailureRoutine" } },
{ 36, { sizeof(DWORD), 132, L"GuardRFFailureRoutineFunctionPointer" } },
{ 37, { sizeof(DWORD), 136, L"DynamicValueRelocTableOffset" } },
{ 38, { sizeof(WORD), 140, L"DynamicValueRelocTableSection" } },
{ 39, { sizeof(WORD), 142, L"Reserved2" } },
{ 40, { sizeof(DWORD), 144, L"GuardRFVerifyStackPointerFunctionPointer" } },
{ 41, { sizeof(DWORD), 148, L"HotPatchTableOffset" } },
{ 42, { sizeof(DWORD), 152, L"Reserved3" } },
{ 43, { sizeof(DWORD), 156, L"EnclaveConfigurationPointer" } },
{ 44, { sizeof(DWORD), 160, L"VolatileMetadataPointer" } },
};

inline map_hdr g_mapLCD64 {
	{ 0, { sizeof(DWORD), 0, L"Size" } },
{ 1, { sizeof(DWORD), 4, L"TimeDateStamp" } },
{ 2, { sizeof(WORD), 8, L"MajorVersion" } },
{ 3, { sizeof(WORD), 10, L"MinorVersion" } },
{ 4, { sizeof(DWORD), 12, L"GlobalFlagsClear" } },
{ 5, { sizeof(DWORD), 16, L"GlobalFlagsSet" } },
{ 6, { sizeof(DWORD), 20, L"CriticalSectionDefaultTimeout" } },
{ 7, { sizeof(ULONGLONG), 24, L"DeCommitFreeBlockThreshold" } },
{ 8, { sizeof(ULONGLONG), 32, L"DeCommitTotalFreeThreshold" } },
{ 9, { sizeof(ULONGLONG), 40, L"LockPrefixTable" } },
{ 10, { sizeof(ULONGLONG), 48, L"MaximumAllocationSize" } },
{ 11, { sizeof(ULONGLONG), 56, L"VirtualMemoryThreshold" } },
{ 12, { sizeof(ULONGLONG), 64, L"ProcessHeapFlags" } },
{ 13, { sizeof(DWORD), 72, L"ProcessAffinityMask" } },
{ 14, { sizeof(WORD), 76, L"CSDVersion" } },
{ 15, { sizeof(WORD), 78, L"DependentLoadFlags" } },
{ 16, { sizeof(ULONGLONG), 80, L"EditList" } },
{ 17, { sizeof(ULONGLONG), 88, L"SecurityCookie" } },
{ 18, { sizeof(ULONGLONG), 96, L"SEHandlerTable" } },
{ 19, { sizeof(ULONGLONG), 104, L"SEHandlerCount" } },
{ 20, { sizeof(ULONGLONG), 112, L"GuardCFCheckFunctionPointer" } },
{ 21, { sizeof(ULONGLONG), 120, L"GuardCFDispatchFunctionPointer" } },
{ 22, { sizeof(ULONGLONG), 128, L"GuardCFFunctionTable" } },
{ 23, { sizeof(ULONGLONG), 136, L"GuardCFFunctionCount" } },
{ 24, { sizeof(DWORD), 144, L"GuardFlags" } },
{ 25, { sizeof(WORD), 148, L"CodeIntegrity.Flags" } },
{ 26, { sizeof(WORD), 150, L"CodeIntegrity.Catalog" } },
{ 27, { sizeof(DWORD), 152, L"CodeIntegrity.CatalogOffset" } },
{ 28, { sizeof(DWORD), 156, L"CodeIntegrity.Reserved" } },
{ 29, { sizeof(ULONGLONG), 160, L"GuardAddressTakenIatEntryTable" } },
{ 30, { sizeof(ULONGLONG), 168, L"GuardAddressTakenIatEntryCount" } },
{ 31, { sizeof(ULONGLONG), 176, L"GuardLongJumpTargetTable" } },
{ 32, { sizeof(ULONGLONG), 184, L"GuardLongJumpTargetCount" } },
{ 33, { sizeof(ULONGLONG), 192, L"DynamicValueRelocTable" } },
{ 34, { sizeof(ULONGLONG), 200, L"CHPEMetadataPointer" } },
{ 35, { sizeof(ULONGLONG), 208, L"GuardRFFailureRoutine" } },
{ 36, { sizeof(ULONGLONG), 216, L"GuardRFFailureRoutineFunctionPointer" } },
{ 37, { sizeof(DWORD), 224, L"DynamicValueRelocTableOffset" } },
{ 38, { sizeof(WORD), 228, L"DynamicValueRelocTableSection" } },
{ 39, { sizeof(WORD), 230, L"Reserved2" } },
{ 40, { sizeof(ULONGLONG), 232, L"GuardRFVerifyStackPointerFunctionPointer" } },
{ 41, { sizeof(DWORD), 240, L"HotPatchTableOffset" } },
{ 42, { sizeof(DWORD), 244, L"Reserved3" } },
{ 43, { sizeof(ULONGLONG), 248, L"EnclaveConfigurationPointer" } },
{ 44, { sizeof(ULONGLONG), 256, L"VolatileMetadataPointer" } },
};

/////////////////////////////////////////////////////////////////////////////
struct RESHELPER
{
	RESHELPER() {}
	RESHELPER(WORD type, WORD name, std::vector<std::byte>* data) : IdResType(type), IdResName(name), pData(data) {}
	WORD IdResType { };
	WORD IdResName { };
	std::vector<std::byte>* pData { };
};

/*****************************************************************
* These are identificators of all the controls: list, hex, tree. *
*****************************************************************/
constexpr auto IDC_LIST_DOSHEADER = 0x0001;
constexpr auto IDC_LIST_DOSHEADER_ENTRY = 0x0002;
constexpr auto IDC_LIST_RICHHEADER = 0x0003;
constexpr auto IDC_LIST_RICHHEADER_ENTRY = 0x0004;
constexpr auto IDC_LIST_NTHEADER = 0x0005;
constexpr auto IDC_LIST_NTHEADER_ENTRY = 0x0006;
constexpr auto IDC_LIST_FILEHEADER = 0x0007;
constexpr auto IDC_LIST_FILEHEADER_ENTRY = 0x0008;
constexpr auto IDC_LIST_OPTIONALHEADER = 0x0009;
constexpr auto IDC_LIST_OPTIONALHEADER_ENTRY = 0x000A;
constexpr auto IDC_LIST_DATADIRECTORIES = 0x000B;
constexpr auto IDC_LIST_DATADIRECTORIES_ENTRY = 0x000C;
constexpr auto IDC_LIST_SECHEADERS = 0x000D;
constexpr auto IDC_LIST_SECHEADERS_ENTRY = 0x000E;
constexpr auto IDC_LIST_EXPORT = 0x000F;
constexpr auto IDC_LIST_EXPORT_FUNCS = 0x0010;
constexpr auto IDC_LIST_IMPORT = 0x0011;
constexpr auto IDC_LIST_IMPORT_ENTRY = 0x0012;
constexpr auto IDC_LIST_EXCEPTION = 0x0013;
constexpr auto IDC_LIST_EXCEPTION_ENTRY = 0x0014;
constexpr auto IDC_LIST_SECURITY = 0x0015;
constexpr auto IDC_LIST_SECURITY_ENTRY = 0x0016;
constexpr auto IDC_LIST_RELOCATIONS = 0x0017;
constexpr auto IDC_LIST_RELOCATIONS_ENTRY = 0x0018;
constexpr auto IDC_LIST_DEBUG = 0x0019;
constexpr auto IDC_LIST_DEBUG_ENTRY = 0x001A;
constexpr auto IDC_LIST_ARCHITECTURE = 0x001B;
constexpr auto IDC_LIST_ARCHITECTURE_ENTRY = 0x001C;
constexpr auto IDC_LIST_GLOBALPTR = 0x001D;
constexpr auto IDC_LIST_GLOBALPTR_ENTRY = 0x001E;
constexpr auto IDC_LIST_TLS = 0x001F;
constexpr auto IDC_LIST_TLS_ENTRY = 0x0020;
constexpr auto IDC_LIST_TLS_CALLBACKS = 0x0021;
constexpr auto IDC_LIST_LOADCONFIG = 0x0022;
constexpr auto IDC_LIST_LOADCONFIG_ENTRY = 0x0023;
constexpr auto IDC_LIST_BOUNDIMPORT = 0x0024;
constexpr auto IDC_LIST_BOUNDIMPORT_ENTRY = 0x0025;
constexpr auto IDC_LIST_IAT = 0x0026;
constexpr auto IDC_LIST_DELAYIMPORT = 0x0027;
constexpr auto IDC_LIST_DELAYIMPORT_ENTRY = 0x0028;
constexpr auto IDC_LIST_COMDESCRIPTOR = 0x0029;
constexpr auto IDC_LIST_COMDESCRIPTOR_ENTRY = 0x002A;

constexpr auto IDC_TREE_LEFT_MAIN = 0x0100;
constexpr auto IDC_TREE_RESOURCE = 0x0101;
constexpr auto IDC_TREE_RESOURCE_TOP = 0x0102;
constexpr auto IDC_TREE_RESOURCE_BOTTOM = 0x0103;

constexpr auto IDC_HEX_RIGHT_BL = 0x0200;
constexpr auto IDC_HEX_RIGHT_TR = 0x0201;

constexpr auto IDC_SHOW_FILE_SUMMARY = 0x0300;
constexpr auto IDC_SHOW_RESOURCE_RBR = 0x0301;

constexpr auto IDM_LIST_GOTODESCOFFSET = 0x8001;
constexpr auto IDM_LIST_GOTODATAOFFSET = 0x8002;
/********************************************************
* End of IDC.											*
********************************************************/

constexpr COLORREF g_clrOffset = RGB(150, 150, 150);