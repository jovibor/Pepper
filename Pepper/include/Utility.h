#pragma once
#include <map>
#include "libpe.h"

#define PRODUCT_NAME			L"Pepper"
#define PRODUCT_DESC			L"PE files viewer, github.com/jovibor/Pepper"
#define COPYRIGHT_NAME  		L"(C) Jovibor 2019-2021"
#define MAJOR_VERSION			1
#define MINOR_VERSION			2
#define MAINTENANCE_VERSION		6

#define TO_WSTR_HELPER(x) L## #x
#define TO_WSTR(x) TO_WSTR_HELPER(x)
#define VERSION_WSTR PRODUCT_NAME L" - PE/PE+ binaries viewer v" TO_WSTR(MAJOR_VERSION) L"."\
		TO_WSTR(MINOR_VERSION) L"." TO_WSTR(MAINTENANCE_VERSION)
#ifdef _WIN64
#define PEPPER_VERSION_WSTR VERSION_WSTR L" (x64)"
#else
#define PEPPER_VERSION_WSTR VERSION_WSTR
#endif

#define TO_WSTR_MAP(x) {x, L## #x}

using namespace libpe;

//Errors, that might come from libpe.
inline const std::map<DWORD, std::wstring> g_mapLibpeErrors {
	TO_WSTR_MAP(E_CALL_LOADPE_FIRST),
	TO_WSTR_MAP(E_FILE_CREATEFILE_FAILED),
	TO_WSTR_MAP(E_FILE_SIZE_TOO_SMALL),
	TO_WSTR_MAP(E_FILE_CREATEFILEMAPPING_FAILED),
	TO_WSTR_MAP(E_FILE_MAPVIEWOFFILE_FAILED),
	TO_WSTR_MAP(E_FILE_MAPVIEWOFFILE_SECTION_FAILED),
	TO_WSTR_MAP(E_FILE_SECTION_DATA_CORRUPTED),
	TO_WSTR_MAP(E_IMAGE_TYPE_UNSUPPORTED),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_DOSHEADER),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_RICHHEADER),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_NTHEADER),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_FILEHEADER),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_OPTHEADER),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_DATADIRECTORIES),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_SECTIONS),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_EXPORT),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_IMPORT),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_RESOURCE),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_EXCEPTION),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_SECURITY),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_BASERELOC),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_DEBUG),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_ARCHITECTURE),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_GLOBALPTR),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_TLS),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_LOADCONFIG),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_BOUNDIMPORT),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_IAT),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_DELAYIMPORT),
	TO_WSTR_MAP(E_IMAGE_HAS_NO_COMDESCRIPTOR)
};

//All possible PE Resource types.
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

//Helper struct for PE structs' fields offsets and sizes.
//Reflection kind of.
struct SPEREFLECTION
{
	DWORD dwSize;			//Struct's field size.
	DWORD dwOffset;			//Field offset.
	std::wstring wstrName;	//Field name.
};
using map_hdr = std::map<DWORD, SPEREFLECTION>;

//Standard headers' maps.
inline const map_hdr g_mapDOSHeader {
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

inline const map_hdr g_mapFileHeader {
	{ 0, { sizeof(WORD), 0, L"Machine" } },
	{ 1, { sizeof(WORD), 2, L"NumberOfSections" } },
	{ 2, { sizeof(DWORD), 4, L"TimeDateStamp" } },
	{ 3, { sizeof(DWORD), 8, L"PointerToSymbolTable" } },
	{ 4, { sizeof(DWORD), 12, L"NumberOfSymbols" } },
	{ 5, { sizeof(WORD), 16, L"SizeOfOptionalHeader" } },
	{ 6, { sizeof(WORD), 18, L"Characteristics" } }
};

inline const map_hdr g_mapOptHeader32 {
	{ 0, { sizeof(WORD), 0, L"Magic" } },
	{ 1, { sizeof(BYTE), 2, L"MajorLinkerVersion" } },
	{ 2, { sizeof(BYTE), 3, L"MinorLinkerVersion" } },
	{ 3, { sizeof(DWORD), 4, L"SizeOfCode" } },
	{ 4, { sizeof(DWORD), 8, L"SizeOfInitializedData" } },
	{ 5, { sizeof(DWORD), 12, L"SizeOfUninitializedData" } },
	{ 6, { sizeof(DWORD), 16, L"AddressOfEntryPoint" } },
	{ 7, { sizeof(DWORD), 20, L"BaseOfCode" } },
	{ 8, { sizeof(DWORD), 24, L"BaseOfData" } },
	{ 9, { sizeof(DWORD), 28, L"ImageBase" } },
	{ 10, { sizeof(DWORD), 32, L"SectionAlignment" } },
	{ 11, { sizeof(DWORD), 36, L"FileAlignment" } },
	{ 12, { sizeof(WORD), 40, L"MajorOperatingSystemVersion" } },
	{ 13, { sizeof(WORD), 42, L"MinorOperatingSystemVersion" } },
	{ 14, { sizeof(WORD), 44, L"MajorImageVersion" } },
	{ 15, { sizeof(WORD), 46, L"MinorImageVersion" } },
	{ 16, { sizeof(WORD), 48, L"MajorSubsystemVersion" } },
	{ 17, { sizeof(WORD), 50, L"MinorSubsystemVersion" } },
	{ 18, { sizeof(DWORD), 52, L"Win32VersionValue" } },
	{ 19, { sizeof(DWORD), 56, L"SizeOfImage" } },
	{ 20, { sizeof(DWORD), 60, L"SizeOfHeaders" } },
	{ 21, { sizeof(DWORD), 64, L"CheckSum" } },
	{ 22, { sizeof(WORD), 68, L"Subsystem" } },
	{ 23, { sizeof(WORD), 70, L"DllCharacteristics" } },
	{ 24, { sizeof(DWORD), 72, L"SizeOfStackReserve" } },
	{ 25, { sizeof(DWORD), 76, L"SizeOfStackCommit" } },
	{ 26, { sizeof(DWORD), 80, L"SizeOfHeapReserve" } },
	{ 27, { sizeof(DWORD), 84, L"SizeOfHeapCommit" } },
	{ 28, { sizeof(DWORD), 88, L"LoaderFlags" } },
	{ 29, { sizeof(DWORD), 92, L"NumberOfRvaAndSizes" } }
};

inline const map_hdr g_mapOptHeader64 {
	{ 0, { sizeof(WORD), 0, L"Magic" } },
	{ 1, { sizeof(BYTE), 2, L"MajorLinkerVersion" } },
	{ 2, { sizeof(BYTE), 3, L"MinorLinkerVersion" } },
	{ 3, { sizeof(DWORD), 4, L"SizeOfCode" } },
	{ 4, { sizeof(DWORD), 8, L"SizeOfInitializedData" } },
	{ 5, { sizeof(DWORD), 12, L"SizeOfUninitializedData" } },
	{ 6, { sizeof(DWORD), 16, L"AddressOfEntryPoint" } },
	{ 7, { sizeof(DWORD), 20, L"BaseOfCode" } },
	{ 8, { sizeof(ULONGLONG), 24, L"ImageBase" } },
	{ 9, { sizeof(DWORD), 32, L"SectionAlignment" } },
	{ 10, { sizeof(DWORD), 36, L"FileAlignment" } },
	{ 11, { sizeof(WORD), 40, L"MajorOperatingSystemVersion" } },
	{ 12, { sizeof(WORD), 42, L"MinorOperatingSystemVersion" } },
	{ 13, { sizeof(WORD), 44, L"MajorImageVersion" } },
	{ 14, { sizeof(WORD), 46, L"MinorImageVersion" } },
	{ 15, { sizeof(WORD), 48, L"MajorSubsystemVersion" } },
	{ 16, { sizeof(WORD), 50, L"MinorSubsystemVersion" } },
	{ 17, { sizeof(DWORD), 52, L"Win32VersionValue" } },
	{ 18, { sizeof(DWORD), 56, L"SizeOfImage" } },
	{ 19, { sizeof(DWORD), 60, L"SizeOfHeaders" } },
	{ 20, { sizeof(DWORD), 64, L"CheckSum" } },
	{ 21, { sizeof(WORD), 68, L"Subsystem" } },
	{ 22, { sizeof(WORD), 70, L"DllCharacteristics" } },
	{ 23, { sizeof(ULONGLONG), 72, L"SizeOfStackReserve" } },
	{ 24, { sizeof(ULONGLONG), 80, L"SizeOfStackCommit" } },
	{ 25, { sizeof(ULONGLONG), 88, L"SizeOfHeapReserve" } },
	{ 26, { sizeof(ULONGLONG), 96, L"SizeOfHeapCommit" } },
	{ 27, { sizeof(DWORD), 104, L"LoaderFlags" } },
	{ 28, { sizeof(DWORD), 108, L"NumberOfRvaAndSizes" } }
};

inline const std::map<WORD, std::wstring> g_mapDataDirs {
	{ IMAGE_DIRECTORY_ENTRY_EXPORT, L"Export Directory" },
	{ IMAGE_DIRECTORY_ENTRY_IMPORT, L"Import Directory" },
	{ IMAGE_DIRECTORY_ENTRY_RESOURCE, L"Resource Directory" },
	{ IMAGE_DIRECTORY_ENTRY_EXCEPTION, L"Exception Directory" },
	{ IMAGE_DIRECTORY_ENTRY_SECURITY, L"Security Directory" },
	{ IMAGE_DIRECTORY_ENTRY_BASERELOC, L"Relocation Directory" },
	{ IMAGE_DIRECTORY_ENTRY_DEBUG, L"Debug Directory" },
	{ IMAGE_DIRECTORY_ENTRY_ARCHITECTURE, L"Architecture Directory" },
	{ IMAGE_DIRECTORY_ENTRY_GLOBALPTR, L"Global PTR" },
	{ IMAGE_DIRECTORY_ENTRY_TLS, L"TLS Directory" },
	{ IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, L"Load Config Directory" },
	{ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, L"Bound Import Directory" },
	{ IMAGE_DIRECTORY_ENTRY_IAT, L"IAT Directory" },
	{ IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, L"Delay Import Directory" },
	{ IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR, L"COM Descriptor Directory" }
};

inline const map_hdr g_mapExport {
	{ 0, { sizeof(DWORD), 0, L"Characteristics" } },
	{ 1, { sizeof(DWORD), 4, L"TimeDateStamp" } },
	{ 2, { sizeof(WORD), 8, L"MajorVersion" } },
	{ 3, { sizeof(WORD), 10, L"MinorVersion" } },
	{ 4, { sizeof(DWORD), 12, L"Name" } },
	{ 5, { sizeof(DWORD), 16, L"Base" } },
	{ 6, { sizeof(DWORD), 20, L"NumberOfFunctions" } },
	{ 7, { sizeof(DWORD), 24, L"NumberOfNames" } },
	{ 8, { sizeof(DWORD), 28, L"AddressOfFunctions" } },
	{ 9, { sizeof(DWORD), 32, L"AddressOfNames" } },
	{ 10, { sizeof(DWORD), 36, L"AddressOfNameOrdinals" } }
};

inline const map_hdr g_mapLCD32 {
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

inline const map_hdr g_mapLCD64 {
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

inline const map_hdr g_mapTLS32 {
	{ 0, { sizeof(DWORD), 0, L"StartAddressOfRawData" } },
	{ 1, { sizeof(DWORD), 4, L"EndAddressOfRawData" } },
	{ 2, { sizeof(DWORD), 8, L"AddressOfIndex" } },
	{ 3, { sizeof(DWORD), 12, L"AddressOfCallBacks" } },
	{ 4, { sizeof(DWORD), 16, L"SizeOfZeroFill" } },
	{ 5, { sizeof(DWORD), 20, L"Characteristics" } },
};

inline const map_hdr g_mapTLS64 {
	{ 0, { sizeof(ULONGLONG), 0, L"StartAddressOfRawData" } },
	{ 1, { sizeof(ULONGLONG), 8, L"EndAddressOfRawData" } },
	{ 2, { sizeof(ULONGLONG), 16, L"AddressOfIndex" } },
	{ 3, { sizeof(ULONGLONG), 24, L"AddressOfCallBacks" } },
	{ 4, { sizeof(DWORD), 32, L"SizeOfZeroFill" } },
	{ 5, { sizeof(DWORD), 36, L"Characteristics" } },
};

inline const map_hdr g_mapComDir {
	{ 0, { sizeof(DWORD), 0, L"cb" } },
	{ 1, { sizeof(WORD), 4, L"MajorRuntimeVersion" } },
	{ 2, { sizeof(WORD), 6, L"MinorRuntimeVersion" } },
	{ 3, { sizeof(DWORD), 8, L"MetaData.VirtualAddress" } },
	{ 4, { sizeof(DWORD), 12, L"MetaData.Size" } },
	{ 5, { sizeof(DWORD), 16, L"Flags" } },
	{ 6, { sizeof(DWORD), 20, L"EntryPointToken" } },
	{ 7, { sizeof(DWORD), 24, L"Resources.VirtualAddress" } },
	{ 8, { sizeof(DWORD), 28, L"Resources.Size" } },
	{ 9, { sizeof(DWORD), 32, L"StrongNameSignature.VirtualAddress" } },
	{ 10, { sizeof(DWORD), 36, L"StrongNameSignature.Size" } },
	{ 11, { sizeof(DWORD), 40, L"CodeManagerTable.VirtualAddress" } },
	{ 12, { sizeof(DWORD), 44, L"CodeManagerTable.Size" } },
	{ 13, { sizeof(DWORD), 48, L"VTableFixups.VirtualAddress" } },
	{ 14, { sizeof(DWORD), 52, L"VTableFixups.Size" } },
	{ 15, { sizeof(DWORD), 56, L"ExportAddressTableJumps.VirtualAddress" } },
	{ 16, { sizeof(DWORD), 60, L"ExportAddressTableJumps.Size" } },
	{ 17, { sizeof(DWORD), 64, L"ManagedNativeHeader.VirtualAddress" } },
	{ 18, { sizeof(DWORD), 68, L"ManagedNativeHeader.Size" } }
};
////////////////////////////////////////////////////////////

//Helper struct for resources interchange between views.
struct SRESHELPER
{
	SRESHELPER() {}
	SRESHELPER(WORD type, WORD name, std::vector<std::byte>* data) : IdResType(type), IdResName(name), pData(data) {}
	WORD IdResType { };
	WORD IdResName { };
	std::vector<std::byte>* pData { };
};

struct SWINDOWSTATUS
{
	HWND hWnd { };
	bool fVisible { };
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
constexpr auto IDC_LIST_EXCEPTIONS = 0x0013;
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

constexpr auto ID_DOC_EDITMODE = 0x0401;

constexpr auto IDM_LIST_GOTODESCOFFSET = 0x8001;
constexpr auto IDM_LIST_GOTODATAOFFSET = 0x8002;
/********************************************************
* End of IDC.											*
********************************************************/

//Color of the list's "Offset" column
constexpr COLORREF g_clrOffset = RGB(150, 150, 150);
inline const wchar_t* const g_pwszHexMap { L"0123456789ABCDEF" };

inline void DwordToWchars(DWORD dw, wchar_t* pwsz)
{
	for (size_t i = 0; i < sizeof(DWORD); i++)
	{
		pwsz[i * 2] = g_pwszHexMap[((dw >> ((4 - 1 - i) << 3)) >> 4) & 0x0F];
		pwsz[i * 2 + 1] = g_pwszHexMap[(dw >> ((4 - 1 - i) << 3)) & 0x0F];
	}
}