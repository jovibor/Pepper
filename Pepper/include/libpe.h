/****************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/			 				*
* This software is available under the "MIT License modified with The Commons Clause."	*
* Windows library for reading PE (x86) and PE+ (x64) files inner structure information.	*
* https://github.com/jovibor/libpe	 													*
****************************************************************************************/
#pragma once
#include <vector>		//atd::vector and related.
#include <memory>		//std::shared_ptr and related.
#include <string>		//std::string and related.
#include <Windows.h>	//All standard Windows types.
#include <ImageHlp.h>	//LPWIN_CERTIFICATE struct.

#ifndef __cpp_lib_byte
#define __cpp17_conformant 0
#elif __cpp_lib_byte < 201603
#define __cpp17_conformant 0
#else
#define __cpp17_conformant 1
#endif
static_assert(__cpp17_conformant, "C++17 conformant compiler is required (MSVS 15.7 with /std:c++17, or higher).");

namespace libpe {
	//Standard DOS header struct.
	using PCLIBPE_DOSHEADER = const IMAGE_DOS_HEADER*;

	//Rich.
	//Vector of undocumented «Rich» struct.
	//Struct: 1. Offset of the entry. 2. WORD - Id 3. WORD - version 4. DWORD - count of occurrences.
	struct LIBPE_RICH { DWORD dwOffsetRich; WORD wId; WORD wVersion; DWORD dwCount; };
	using LIBPE_RICHHEADER_VEC = std::vector<LIBPE_RICH>;
	using PCLIBPE_RICHHEADER_VEC = const LIBPE_RICHHEADER_VEC*;

	//NT header.
	//Struct: 1. Offset of the header 2. Header itself (whether x86 or x64).
	struct LIBPE_NTHEADER {
		DWORD dwOffsetNTHdrDesc;
		union LIBPE_NTHEADER_VAR { IMAGE_NT_HEADERS32 stNTHdr32; IMAGE_NT_HEADERS64 stNTHdr64; } varHdr;
	};
	using PCLIBPE_NTHEADER = const LIBPE_NTHEADER*;

	//File header.
	//Standard File header struct.
	using PCLIBPE_FILEHEADER = const IMAGE_FILE_HEADER*;

	//Optional header.
	//Union of standard Optional header struct, depends on file type — x86 or x64.
	union LIBPE_OPTHEADER_VAR { IMAGE_OPTIONAL_HEADER32 stOptHdr32; IMAGE_OPTIONAL_HEADER64 stOptHdr64; };
	using PCLIBPE_OPTHEADER_VAR = const LIBPE_OPTHEADER_VAR*;

	//Data directories.
	//Vector of LIBPE_DATADIR struct.
	//Struct: 1. Standard IMAGE_DATA_DIRECTORY 2. String with the name of the section this dir resides in (points to).
	struct LIBPE_DATADIR { IMAGE_DATA_DIRECTORY stDataDir; std::string strSecResidesIn; };
	using LIBPE_DATADIRS_VEC = std::vector<LIBPE_DATADIR>;
	using PCLIBPE_DATADIRS_VEC = const LIBPE_DATADIRS_VEC*;

	//Sections headers.
	//Vector of LIBPE_SECHEADERS struct.
	//Struct: 1. Section header offset 2. Standard section header struct 3. String with the section full name
	//For more info check:
	//docs.microsoft.com/en-us/windows/desktop/api/winnt/ns-winnt-_image_section_header#members
	//«An 8-byte, null-padded UTF-8 string. For longer names, this member contains a forward slash (/) 
	//followed by an ASCII representation of a decimal number that is an offset into the string table.»
	struct LIBPE_SECHEADERS { DWORD dwOffsetSecHdrDesc; IMAGE_SECTION_HEADER stSecHdr; std::string strSecName; };
	using LIBPE_SECHEADERS_VEC = std::vector<LIBPE_SECHEADERS>;
	using PCLIBPE_SECHEADERS_VEC = const LIBPE_SECHEADERS_VEC*;

	//Export table.
	//LIBPE_EXPORT struct: 1 Export descriptor offset 2. Standard export header struct. 3 String of actual module name
	//4. Vector of LIBPE_EXPORT_FUNC struct - exported functions struct.
	//LIBPE_EXPORT_FUNC struct:
	//1. RVA 2. Ordinal 3. Func name 4. Func forwarder name.
	struct LIBPE_EXPORT_FUNC { DWORD dwRVA; DWORD dwOrdinal; std::string strFuncName; std::string strForwarderName; };
	struct LIBPE_EXPORT {
		DWORD dwOffsetExportDesc; IMAGE_EXPORT_DIRECTORY stExportDesc;
		std::string strModuleName; std::vector<LIBPE_EXPORT_FUNC> vecFuncs;
	};
	using PCLIBPE_EXPORT = const LIBPE_EXPORT*;

	//Import table:
	//Vector of LIBPE_IMPORT_MODULE struct.
	//Struct: 1. Offset of import descriptor 2. Standard import descriptor 3. Import module name 4. Vector of LIBPE_IMPORT_FUNC struct
	//LIBPE_IMPORT_FUNC struct - import module funcs struct:
	//1. Union of standard IMAGE_THUNK_DATA (x86 or x64) 2. Standard IMAGE_IMPORT_BY_NAME struct 3. String with function name.
	struct LIBPE_IMPORT_FUNC {
		union LIBPE_IMPORT_THUNK_VAR {
			IMAGE_THUNK_DATA32 stThunk32;
			IMAGE_THUNK_DATA64 stThunk64;
		}varThunk;
		IMAGE_IMPORT_BY_NAME stImpByName;
		std::string strFuncName;
	};
	struct LIBPE_IMPORT_MODULE {
		DWORD dwOffsetImpDesc; IMAGE_IMPORT_DESCRIPTOR stImportDesc;
		std::string strModuleName; std::vector<LIBPE_IMPORT_FUNC> vecImportFunc;
	};
	using LIBPE_IMPORT_VEC = std::vector<LIBPE_IMPORT_MODULE>;
	using PCLIBPE_IMPORT_VEC = const LIBPE_IMPORT_VEC*;

	/**************************************Resources by Levels*******************************************
	* There are 3 levels of resources: 1. Type 2. Name 3. Language.										*
	* https://docs.microsoft.com/en-us/windows/desktop/Debug/pe-format#the-rsrc-section					*
	* «Each directory table is followed by a series of directory entries that give the name				*
	* or identifier (ID) for that level (Type, Name, or Language level) and an address of either a data *
	* description or another directory table. If the address points to a data description, then 		*
	* the data is a leaf in the tree. If the address points to another directory table, then that table *
	* lists directory entries at the next level down.													*
	* A leaf's Type, Name, and Language IDs are determined by the path that is taken through directory 	*
	* tables to reach the leaf. The first table determines Type ID, the second table (pointed to by 	*
	* the directory entry in the first table) determines Name ID, and the third table determines 		*
	* Language ID.»																						*
	* Highest (root) resource structure is LIBPE_RESOURCE_ROOT. It's, in fact, an std::tuple		*
	* that includes: an IMAGE_RESOURCE_DIRECTORY of root resource directory itself, 					*
	* and LIBPE_RESOURCE_ROOT_DATA_VEC, that is actually an std::vector that includes std::tuple of all		*
	* IMAGE_RESOURCE_DIRECTORY_ENTRY structures of the root resource directory.							*
	* It also includes: std::wstring(Resource name), IMAGE_RESOURCE_DATA_ENTRY, 						*
	* std::vector<std::byte> (RAW resource data), and LIBPE_RESOURCE_LVL2 that is, in fact,			*
	* a tuple of the next, second, resource level, that replicates tuple of root resource level.		*
	* LIBPE_RESOURCE_LVL2 includes IMAGE_RESOURCE_DIRECTORY of second resource level, and 			*
	* LIBPE_RESOURCE_LVL2_DATA_VEC that includes LIBPE_RESOURCE_LVL3	that is an std::tuple of the last,	*
	* third, level of resources.																		*
	* Like previous two, this last level's tuple consist of IMAGE_RESOURCE_DIRECTORY 					*
	* and LIBPE_RESOURCE_LVL3_DATA_VEC, that is again — vector of tuples of all 								*
	* IMAGE_RESOURCE_DIRECTORY_ENTRY of the last, third, level of resources. See code below.			*
	****************************************************************************************************/
	//Level 3 (the lowest) Resources.
	struct LIBPE_RESOURCE_LVL3_DATA {
		IMAGE_RESOURCE_DIRECTORY_ENTRY stResDirEntryLvL3; std::wstring wstrResNameLvL3;
		IMAGE_RESOURCE_DATA_ENTRY stResDataEntryLvL3; std::vector<std::byte> vecResRawDataLvL3;
	};
	struct LIBPE_RESOURCE_LVL3 { IMAGE_RESOURCE_DIRECTORY stResDirLvL3; std::vector<LIBPE_RESOURCE_LVL3_DATA> vecResLvL3; };
	using PCLIBPE_RESOURCE_LVL3 = const LIBPE_RESOURCE_LVL3*;

	//Level 2 Resources — Includes LVL3 Resourses.
	struct LIBPE_RESOURCE_LVL2_DATA {
		IMAGE_RESOURCE_DIRECTORY_ENTRY stResDirEntryLvL2; std::wstring wstrResNameLvL2;
		IMAGE_RESOURCE_DATA_ENTRY stResDataEntryLvL2; std::vector<std::byte> vecResRawDataLvL2; LIBPE_RESOURCE_LVL3 stResLvL3;
	};
	struct LIBPE_RESOURCE_LVL2 { IMAGE_RESOURCE_DIRECTORY stResDirLvL2; std::vector<LIBPE_RESOURCE_LVL2_DATA> vecResLvL2; };
	using PCLIBPE_RESOURCE_LVL2 = const LIBPE_RESOURCE_LVL2*;

	//Level 1 (Root) Resources — Includes LVL2 Resources.
	struct LIBPE_RESOURCE_ROOT_DATA {
		IMAGE_RESOURCE_DIRECTORY_ENTRY stResDirEntryRoot; std::wstring wstrResNameRoot;
		IMAGE_RESOURCE_DATA_ENTRY stResDataEntryRoot; std::vector<std::byte> vecResRawDataRoot; LIBPE_RESOURCE_LVL2 stResLvL2;
	};
	struct LIBPE_RESOURCE_ROOT { DWORD dwOffsetResRoot; IMAGE_RESOURCE_DIRECTORY stResDirRoot; std::vector<LIBPE_RESOURCE_ROOT_DATA> vecResRoot; };
	using PCLIBPE_RESOURCE_ROOT = const LIBPE_RESOURCE_ROOT*;
	/***************************************************************************************
	*********************************Resources End******************************************
	***************************************************************************************/

	//Exception table.
	//Vector of LIBPE_EXCEPTION struct.
	//Struct: 1. Descriptor offset 2. Standard _IMAGE_RUNTIME_FUNCTION_ENTRY struct.
	struct LIBPE_EXCEPTION { DWORD dwOffsetRuntimeFuncDesc; _IMAGE_RUNTIME_FUNCTION_ENTRY stRuntimeFuncEntry; };
	using LIBPE_EXCEPTION_VEC = std::vector<LIBPE_EXCEPTION>;
	using PCLIBPE_EXCEPTION_VEC = const LIBPE_EXCEPTION_VEC*;

	//Security table.
	//Vector of LIBPE_SECURITY struct.
	//Struct: 1. Descriptor offset 2. Standard WIN_CERTIFICATE struct.
	struct LIBPE_SECURITY { DWORD dwOffsetWinCertDesc; WIN_CERTIFICATE stWinSert; };
	using LIBPE_SECURITY_VEC = std::vector<LIBPE_SECURITY>;
	using PCLIBPE_SECURITY_VEC = const LIBPE_SECURITY_VEC*;

	//Relocation table.
	//Vector of LIBPE_RELOCATION struct.
	//Struct: 1. Descritor offset 2. Standard IMAGE_BASE_RELOCATION struct 3. Vector of the LIBPE_RELOC_DATA struct
	//LIBPE_RELOC_DATA struct: 1. Relocation data offset 2. Relocation type 3. Relocation offset (offset reloc must be applied to)
	struct LIBPE_RELOC_DATA { DWORD dwOffsetRelocData; WORD wRelocType; WORD wRelocOffset; };
	struct LIBPE_RELOCATION { DWORD dwOffsetReloc; IMAGE_BASE_RELOCATION stBaseReloc; std::vector<LIBPE_RELOC_DATA> vecRelocData; };
	using LIBPE_RELOCATION_VEC = std::vector<LIBPE_RELOCATION>;
	using PCLIBPE_RELOCATION_VEC = const LIBPE_RELOCATION_VEC*;

	//Debug table.
	//Vector of LIBPE_DEBUG struct.
	//Struct: 1. Debug descriptor offset. 2. Standard IMAGE_DEBUG_DIRECTORY struct
	struct LIBPE_DEBUG { DWORD dwOffsetDebug; IMAGE_DEBUG_DIRECTORY stDebugDir; };
	using LIBPE_DEBUG_VEC = std::vector<LIBPE_DEBUG>;
	using PCLIBPE_DEBUG_VEC = const LIBPE_DEBUG_VEC*;

	//TLS table.
	//LIBPE_TLS struct: 1. TLS header offset 2. Union of standard IMAGE_TLS_DIRECTORY header (x86 or x64) 
	//3. TLS raw data offset 4. TLS raw data size 5. Vector of DWORDS of TLS Callbacks.
	struct LIBPE_TLS {
		DWORD dwOffsetTLS;
		union LIBPE_TLS_VAR { IMAGE_TLS_DIRECTORY32 stTLSDir32; IMAGE_TLS_DIRECTORY64 stTLSDir64; } varTLS;
		DWORD dwRawDataOffset; DWORD dwRawDataSize;
		std::vector<DWORD> vecTLSCallbacks;
	};
	using PCLIBPE_TLS = const LIBPE_TLS*;

	//LoadConfigDirectory.
	//LCD struct: 1. Offset of the descriptor 2. Union of standard IMAGE_LOAD_CONFIG_DIRECTORY struct (x86 or x64)
	struct LIBPE_LOADCONFIG {
		DWORD dwOffsetLCD;
		union LIBPE_LOADCONFIG_VAR { IMAGE_LOAD_CONFIG_DIRECTORY32 stLCD32; IMAGE_LOAD_CONFIG_DIRECTORY64 stLCD64; } varLCD;
	};
	using PCLIBPE_LOADCONFIG = const LIBPE_LOADCONFIG*;

	//Bound import table.
	//Vector of LIBPE_BOUNDIMPORT struct.
	//Struct: 1. Descriptor offset 2. Standard bound import descriptor 3. Bound name 4. Vector of LIBPE_BOUNDFORWARDER struct
	//LIBPE_BOUNDFORWARDER struct: 1. Descriptor offset 2. Standard IMAGE_BOUND_FORWARDER_REF struct 3. Bound forwarder name
	struct LIBPE_BOUNDFORWARDER {
		DWORD dwOffsetBoundForwDesc; IMAGE_BOUND_FORWARDER_REF stBoundForwarder;
		std::string strBoundForwarderName;
	};
	struct LIBPE_BOUNDIMPORT {
		DWORD dwOffsetBoundImpDesc;	IMAGE_BOUND_IMPORT_DESCRIPTOR stBoundImpDesc; std::string strBoundName;
		std::vector<LIBPE_BOUNDFORWARDER> vecBoundForwarder;
	};
	using LIBPE_BOUNDIMPORT_VEC = std::vector<LIBPE_BOUNDIMPORT>;
	using PCLIBPE_BOUNDIMPORT_VEC = const LIBPE_BOUNDIMPORT_VEC*;

	//Delay import table.
	//Vector of LIBPE_DELAYIMPORT struct.
	//Struct: 1. Descriptor offset 2. Standard IMAGE_DELAYLOAD_DESCRIPTOR struct 
	//3. Module name string 4. Vector of LIBPE_DELAYIMPORT_FUNC struct
	//LIBPE_DELAYIMPORT_FUNC struct: 
	//1. Union of four IMAGE_THUNK_DATA (x86 or x64) — 1) Import addres table 2) Import name table
	//3) Bound import addres table 4) Unload information table
	//2. Standard IMAGE_IMPORT_BY_NAME struct 3. Function name string
	struct LIBPE_DELAYIMPORT_FUNC {
		union LIBPE_DELAYIMPORT_THUNK_VAR
		{
			struct x32 {
				IMAGE_THUNK_DATA32 stImportAddressTable;
				IMAGE_THUNK_DATA32 stImportNameTable;
				IMAGE_THUNK_DATA32 stBoundImportAddressTable;
				IMAGE_THUNK_DATA32 stUnloadInformationTable;
			}st32;
			struct x64 {
				IMAGE_THUNK_DATA64 stImportAddressTable;
				IMAGE_THUNK_DATA64 stImportNameTable;
				IMAGE_THUNK_DATA64 stBoundImportAddressTable;
				IMAGE_THUNK_DATA64 stUnloadInformationTable;
			}st64;
		}varThunk;
		IMAGE_IMPORT_BY_NAME stImpByName;
		std::string strFuncName;
	};
	struct LIBPE_DELAYIMPORT {
		DWORD dwOffsetDelayImpDesc;	IMAGE_DELAYLOAD_DESCRIPTOR stDelayImpDesc; std::string strModuleName;
		std::vector<LIBPE_DELAYIMPORT_FUNC> vecDelayImpFunc;
	};
	using LIBPE_DELAYIMPORT_VEC = std::vector<LIBPE_DELAYIMPORT>;
	using PCLIBPE_DELAYIMPORT_VEC = const LIBPE_DELAYIMPORT_VEC*;

	//COM descriptor table.
	//LIBPE_COMDESCRIPTOR struct: 1. Offset of the header 2. Standard IMAGE_COR20_HEADER struct
	struct LIBPE_COMDESCRIPTOR { DWORD dwOffsetComDesc; IMAGE_COR20_HEADER stCorHdr; };
	using PCLIBPE_COMDESCRIPTOR = const LIBPE_COMDESCRIPTOR*;

	//Pure abstract base class Ilibpe.
	class Ilibpe
	{
	public:
		virtual ~Ilibpe() = default;
		virtual HRESULT LoadPe(LPCWSTR) = 0;
		virtual HRESULT GetImageInfo(DWORD&)noexcept = 0;
		virtual HRESULT GetImageFlag(DWORD dwFlag, bool& f)noexcept = 0;
		virtual HRESULT GetOffsetFromRVA(ULONGLONG ullRVA, DWORD& dwOffset)noexcept = 0;
		virtual HRESULT GetMSDOSHeader(PCLIBPE_DOSHEADER&)noexcept = 0;
		virtual HRESULT GetRichHeader(PCLIBPE_RICHHEADER_VEC&)noexcept = 0;
		virtual HRESULT GetNTHeader(PCLIBPE_NTHEADER&)noexcept = 0;
		virtual HRESULT GetFileHeader(PCLIBPE_FILEHEADER&)noexcept = 0;
		virtual HRESULT GetOptionalHeader(PCLIBPE_OPTHEADER_VAR&)noexcept = 0;
		virtual HRESULT GetDataDirectories(PCLIBPE_DATADIRS_VEC&)noexcept = 0;
		virtual HRESULT GetSectionsHeaders(PCLIBPE_SECHEADERS_VEC&)noexcept = 0;
		virtual HRESULT GetExport(PCLIBPE_EXPORT&)noexcept = 0;
		virtual HRESULT GetImport(PCLIBPE_IMPORT_VEC&)noexcept = 0;
		virtual HRESULT GetResources(PCLIBPE_RESOURCE_ROOT&)noexcept = 0;
		virtual HRESULT GetExceptions(PCLIBPE_EXCEPTION_VEC&)noexcept = 0;
		virtual HRESULT GetSecurity(PCLIBPE_SECURITY_VEC&)noexcept = 0;
		virtual HRESULT GetRelocations(PCLIBPE_RELOCATION_VEC&)noexcept = 0;
		virtual HRESULT GetDebug(PCLIBPE_DEBUG_VEC&)noexcept = 0;
		virtual HRESULT GetTLS(PCLIBPE_TLS&)noexcept = 0;
		virtual HRESULT GetLoadConfig(PCLIBPE_LOADCONFIG&)noexcept = 0;
		virtual HRESULT GetBoundImport(PCLIBPE_BOUNDIMPORT_VEC&)noexcept = 0;
		virtual HRESULT GetDelayImport(PCLIBPE_DELAYIMPORT_VEC&)noexcept = 0;
		virtual HRESULT GetCOMDescriptor(PCLIBPE_COMDESCRIPTOR&)noexcept = 0;
	};
	using libpe_ptr = std::shared_ptr<Ilibpe>;

	/*************************************************
	* Return errors.								 *
	*************************************************/

	constexpr auto E_CALL_LOADPE_FIRST = 0xFFFF;
	constexpr auto E_FILE_CREATEFILE_FAILED = 0x0010;
	constexpr auto E_FILE_SIZE_TOO_SMALL = 0x0011;
	constexpr auto E_FILE_CREATEFILEMAPPING_FAILED = 0x0012;
	constexpr auto E_FILE_MAPVIEWOFFILE_FAILED = 0x0013;
	constexpr auto E_FILE_MAPVIEWOFFILE_SECTION_FAILED = 0x0014;
	constexpr auto E_FILE_SECTION_DATA_CORRUPTED = 0x0015;
	constexpr auto E_IMAGE_TYPE_UNSUPPORTED = 0x0016;
	constexpr auto E_IMAGE_HAS_NO_DOSHEADER = 0x0017;
	constexpr auto E_IMAGE_HAS_NO_RICHHEADER = 0x0018;
	constexpr auto E_IMAGE_HAS_NO_NTHEADER = 0x0019;
	constexpr auto E_IMAGE_HAS_NO_FILEHEADER = 0x001A;
	constexpr auto E_IMAGE_HAS_NO_OPTHEADER = 0x001B;
	constexpr auto E_IMAGE_HAS_NO_DATADIRECTORIES = 0x001C;
	constexpr auto E_IMAGE_HAS_NO_SECTIONS = 0x001D;
	constexpr auto E_IMAGE_HAS_NO_EXPORT = 0x001E;
	constexpr auto E_IMAGE_HAS_NO_IMPORT = 0x001F;
	constexpr auto E_IMAGE_HAS_NO_RESOURCE = 0x0020;
	constexpr auto E_IMAGE_HAS_NO_EXCEPTION = 0x0021;
	constexpr auto E_IMAGE_HAS_NO_SECURITY = 0x0022;
	constexpr auto E_IMAGE_HAS_NO_BASERELOC = 0x0023;
	constexpr auto E_IMAGE_HAS_NO_DEBUG = 0x0024;
	constexpr auto E_IMAGE_HAS_NO_ARCHITECTURE = 0x0025;
	constexpr auto E_IMAGE_HAS_NO_GLOBALPTR = 0x0026;
	constexpr auto E_IMAGE_HAS_NO_TLS = 0x0027;
	constexpr auto E_IMAGE_HAS_NO_LOADCONFIG = 0x0028;
	constexpr auto E_IMAGE_HAS_NO_BOUNDIMPORT = 0x0029;
	constexpr auto E_IMAGE_HAS_NO_IAT = 0x002A;
	constexpr auto E_IMAGE_HAS_NO_DELAYIMPORT = 0x002B;
	constexpr auto E_IMAGE_HAS_NO_COMDESCRIPTOR = 0x002C;

	/*****************************************************
	* Flags according to loaded PE file properties.		 *
	*****************************************************/
	//Tiny function shows whether given DWORD has given flag.
	constexpr bool ImageHasFlag(DWORD dwFileInfo, DWORD dwFlag) { return dwFileInfo & dwFlag; };
	constexpr DWORD IMAGE_FLAG_PE32 = 0x00000001;
	constexpr DWORD IMAGE_FLAG_PE64 = 0x00000002;
	constexpr DWORD IMAGE_FLAG_DOSHEADER = 0x00000004;
	constexpr DWORD IMAGE_FLAG_RICHHEADER = 0x00000008;
	constexpr DWORD IMAGE_FLAG_NTHEADER = 0x00000010;
	constexpr DWORD IMAGE_FLAG_FILEHEADER = 0x00000020;
	constexpr DWORD IMAGE_FLAG_OPTHEADER = 0x00000040;
	constexpr DWORD IMAGE_FLAG_DATADIRECTORIES = 0x00000080;
	constexpr DWORD IMAGE_FLAG_SECTIONS = 0x00000100;
	constexpr DWORD IMAGE_FLAG_EXPORT = 0x00000200;
	constexpr DWORD IMAGE_FLAG_IMPORT = 0x00000400;
	constexpr DWORD IMAGE_FLAG_RESOURCE = 0x00000800;
	constexpr DWORD IMAGE_FLAG_EXCEPTION = 0x00001000;
	constexpr DWORD IMAGE_FLAG_SECURITY = 0x00002000;
	constexpr DWORD IMAGE_FLAG_BASERELOC = 0x00004000;
	constexpr DWORD IMAGE_FLAG_DEBUG = 0x00008000;
	constexpr DWORD IMAGE_FLAG_ARCHITECTURE = 0x00010000;
	constexpr DWORD IMAGE_FLAG_GLOBALPTR = 0x00020000;
	constexpr DWORD IMAGE_FLAG_TLS = 0x00040000;
	constexpr DWORD IMAGE_FLAG_LOADCONFIG = 0x00080000;
	constexpr DWORD IMAGE_FLAG_BOUNDIMPORT = 0x00100000;
	constexpr DWORD IMAGE_FLAG_IAT = 0x00200000;
	constexpr DWORD IMAGE_FLAG_DELAYIMPORT = 0x00400000;
	constexpr DWORD IMAGE_FLAG_COMDESCRIPTOR = 0x00800000;


#ifdef ILIBPE_EXPORT
#define ILIBPEAPI __declspec(dllexport) __cdecl
#else
#define ILIBPEAPI __declspec(dllimport) __cdecl
	/********************************************************
	* Platform and configuration specific .lib name macros.	*
	********************************************************/
#ifdef _WIN64
#ifdef _DEBUG
#define LIBNAME_PROPER(x) x"64d.lib"
#else
#define LIBNAME_PROPER(x) x"64.lib"
#endif
#else
#ifdef _DEBUG
#define LIBNAME_PROPER(x) x"d.lib"
#else
#define LIBNAME_PROPER(x) x".lib"
#endif
#endif
	/********************************************************
	* End. //////////////////////////////////////////////// *
	********************************************************/
#pragma comment(lib, LIBNAME_PROPER("libpe"))
#endif

	extern "C" HRESULT ILIBPEAPI Createlibpe(libpe_ptr& plibpe);
}