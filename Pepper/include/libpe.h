/*********************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/			 *
* PE viewer library for x86 (PE32) and x64 (PE32+) binares.			 *
* This code is provided «AS IS» without any warranty, and			 *
* can be used without any limitations for non-commercial usage.		 *
* Additional info can be found at https://github.com/jovibor/libpe	 *
*********************************************************************/
#pragma once
#include <vector>
#include <memory>
#include <variant>
#include <ImageHlp.h>

#ifndef __cpp_lib_byte
#define __cpp17_conformant 0
#elif __cpp_lib_byte < 201603
#define __cpp17_conformant 0
#else
#define __cpp17_conformant 1
#endif
static_assert(__cpp17_conformant, "C++17 conformant compiler is required (MSVS 15.7 with /std:c++17 or higher).");

namespace libpe
{
	//Constant DWORD*.
	using PCDWORD = const DWORD*;

	//Dos header.
	using PCLIBPE_DOSHEADER = const IMAGE_DOS_HEADER*;

	//Rich.
	//Vector of undocumented DOUBLE DWORDs of "Rich" structure.
	using LIBPE_RICHHEADER_VEC = std::vector<std::tuple<WORD, WORD, DWORD>>;
	using PCLIBPE_RICHHEADER_VEC = const LIBPE_RICHHEADER_VEC*;

	//NT header.
	//Depends on PE type — x86 or x64.
	using LIBPE_NTHEADER_VAR = std::variant<IMAGE_NT_HEADERS32, IMAGE_NT_HEADERS64>;
	using PCLIBPE_NTHEADER_VAR = const LIBPE_NTHEADER_VAR*;

	//File header.
	using PCLIBPE_FILEHEADER = const IMAGE_FILE_HEADER*;

	//Optional header. Depends on file type — x86 or x64.
	using LIBPE_OPTHEADER_VAR = std::variant<IMAGE_OPTIONAL_HEADER32, IMAGE_OPTIONAL_HEADER64>;
	using PCLIBPE_OPTHEADER_VAR = const LIBPE_OPTHEADER_VAR*;

	//Data directories.
	//Vector of IMAGE_DATA_DIRECTORY and section name this dir resides in.
	using LIBPE_DATADIRS_VEC = std::vector<std::tuple<IMAGE_DATA_DIRECTORY, std::string>>;
	using PCLIBPE_DATADIRS_VEC = const LIBPE_DATADIRS_VEC*;

	//Sections headers.
	//Section header and section real name, if presented. For more info check:
	//docs.microsoft.com/en-us/windows/desktop/api/winnt/ns-winnt-_image_section_header#members
	//«An 8-byte, null-padded UTF-8 string. For longer names, this member contains a forward slash (/) 
	//followed by an ASCII representation of a decimal number that is an offset into the string table.»
	using LIBPE_SECHEADERS_VEC = std::vector<std::tuple<IMAGE_SECTION_HEADER, std::string>>;
	using PCLIBPE_SECHEADERS_VEC = const LIBPE_SECHEADERS_VEC*;

	//Export table.
	//Tuple of: IMAGE_EXPORT_DIRECTORY, Actual export module name
	//and vector of exported funcs: RVA, ordinal, func name, func forwarder name.
	using LIBPE_EXPORT_TUP = std::tuple<IMAGE_EXPORT_DIRECTORY, std::string,
		std::vector<std::tuple<DWORD, DWORD, std::string, std::string>>>;
	using PCLIBPE_EXPORT_TUP = const LIBPE_EXPORT_TUP*;

	//Import table:
	//IMAGE_IMPORT_DESCRIPTOR, import module name, vector of:
	//Ordinal/Hint (depending on import type), func name, import thunk RVA.
	using LIBPE_IMPORT_VEC = std::vector<std::tuple<IMAGE_IMPORT_DESCRIPTOR, std::string,
		std::vector<std::tuple<ULONGLONG, std::string, ULONGLONG>>>>;
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
	* Highest (root) resource structure is LIBPE_RESOURCE_ROOT_TUP. It's, in fact, an std::tuple		*
	* that includes: an IMAGE_RESOURCE_DIRECTORY of root resource directory itself, 					*
	* and LIBPE_RESOURCE_ROOT_VEC, that is actually an std::vector that includes std::tuple of all		*
	* IMAGE_RESOURCE_DIRECTORY_ENTRY structures of the root resource directory.							*
	* It also includes: std::wstring(Resource name), IMAGE_RESOURCE_DATA_ENTRY, 						*
	* std::vector<std::byte> (RAW resource data), and LIBPE_RESOURCE_LVL2_TUP that is, in fact,			*
	* a tuple of the next, second, resource level, that replicates tuple of root resource level.		*
	* LIBPE_RESOURCE_LVL2_TUP includes IMAGE_RESOURCE_DIRECTORY of second resource level, and 			*
	* LIBPE_RESOURCE_LVL2_VEC that includes LIBPE_RESOURCE_LVL3_TUP	that is an std::tuple of the last,	*
	* third, level of resources.																		*
	* Like previous two, this last level's tuple consist of IMAGE_RESOURCE_DIRECTORY 					*
	* and LIBPE_RESOURCE_LVL3_VEC, that is again — vector of tuples of all 								*
	* IMAGE_RESOURCE_DIRECTORY_ENTRY of the last, third, level of resources. See code below.			*
	****************************************************************************************************/
	//Level 3 (the lowest) Resources.
	using LIBPE_RESOURCE_LVL3_VEC = std::vector<std::tuple<IMAGE_RESOURCE_DIRECTORY_ENTRY, std::wstring/*ResName*/,
		IMAGE_RESOURCE_DATA_ENTRY, std::vector<std::byte>/*resource LVL3 RAW data*/>>;
	using PLIBPE_RESOURCE_LVL3_VEC = const LIBPE_RESOURCE_LVL3_VEC*;
	using LIBPE_RESOURCE_LVL3_TUP = std::tuple<IMAGE_RESOURCE_DIRECTORY, LIBPE_RESOURCE_LVL3_VEC>;
	using PCLIBPE_RESOURCE_LVL3_TUP = const LIBPE_RESOURCE_LVL3_TUP*;

	//Level 2 Resources — Includes LVL3 Resourses.
	using LIBPE_RESOURCE_LVL2_VEC = std::vector<std::tuple<IMAGE_RESOURCE_DIRECTORY_ENTRY, std::wstring/*ResName*/,
		IMAGE_RESOURCE_DATA_ENTRY, std::vector<std::byte>/*LVL2 RAW data*/, LIBPE_RESOURCE_LVL3_TUP>>;
	using PLIBPE_RESOURCE_LVL2_VEC = const LIBPE_RESOURCE_LVL2_VEC*;
	using LIBPE_RESOURCE_LVL2_TUP = std::tuple<IMAGE_RESOURCE_DIRECTORY, LIBPE_RESOURCE_LVL2_VEC>;
	using PCLIBPE_RESOURCE_LVL2_TUP = const LIBPE_RESOURCE_LVL2_TUP*;

	//Level 1 (Root) Resources — Includes LVL2 Resources.
	using LIBPE_RESOURCE_ROOT_VEC = std::vector<std::tuple<IMAGE_RESOURCE_DIRECTORY_ENTRY, std::wstring/*ResName*/,
		IMAGE_RESOURCE_DATA_ENTRY, std::vector<std::byte>/*LVL1 RAW data*/, LIBPE_RESOURCE_LVL2_TUP>>;
	using PLIBPE_RESOURCE_ROOT_VEC = const LIBPE_RESOURCE_ROOT_VEC*;
	using LIBPE_RESOURCE_ROOT_TUP = std::tuple<IMAGE_RESOURCE_DIRECTORY, LIBPE_RESOURCE_ROOT_VEC>;
	using PCLIBPE_RESOURCE_ROOT_TUP = const LIBPE_RESOURCE_ROOT_TUP*;
	/***************************************************************************************
	****************************************************************************************
	***************************************************************************************/

	//Exception table.
	using LIBPE_EXCEPTION_VEC = std::vector<_IMAGE_RUNTIME_FUNCTION_ENTRY>;
	using PCLIBPE_EXCEPTION_VEC = const LIBPE_EXCEPTION_VEC*;

	//Security table.
	//Vector of WIN_CERTIFICATE and vector of actual data in form of std::byte.
	using LIBPE_SECURITY_VEC = std::vector<std::tuple<WIN_CERTIFICATE, std::vector<std::byte>>>;
	using PCLIBPE_SECURITY_VEC = const LIBPE_SECURITY_VEC*;

	//Relocation table.
	//Vector IMAGE_BASE_RELOCATION, and vector of <Relocations type and Offset>
	using LIBPE_RELOCATION_VEC = std::vector<std::tuple<IMAGE_BASE_RELOCATION, std::vector<std::tuple<WORD, WORD>>>>;
	using PCLIBPE_RELOCATION_VEC = const LIBPE_RELOCATION_VEC*;

	//Debug table.
	//Vector of debug entries: IMAGE_DEBUG_DIRECTORY, vector of raw data.
	using LIBPE_DEBUG_VEC = std::vector<std::tuple<IMAGE_DEBUG_DIRECTORY, std::vector<std::byte>>>;
	using PCLIBPE_DEBUG_VEC = const LIBPE_DEBUG_VEC*;

	//TLS table.
	//Variant of TLS header type, depends on file type — x86 or x64.
	//Vector of std::byte — TLS Raw data, vector<std::byte> — TLS Callbacks. 
	using LIBPE_TLS_TUP = std::tuple<std::variant<IMAGE_TLS_DIRECTORY32, IMAGE_TLS_DIRECTORY64>,
		std::vector<std::byte>/*Raw Data*/, std::vector<DWORD>/*Callbacks*/>;
	using PCLIBPE_TLS_TUP = const LIBPE_TLS_TUP*;

	//LoadConfigTable. Depends on file type — x86 or x64.
	using LIBPE_LOADCONFIGTABLE_VAR = std::variant<IMAGE_LOAD_CONFIG_DIRECTORY32, IMAGE_LOAD_CONFIG_DIRECTORY64>;
	using PCLIBPE_LOADCONFIGTABLE_VAR = const LIBPE_LOADCONFIGTABLE_VAR*;

	//Bound import table.
	//Vector of: IMAGE_BOUND_IMPORT_DESCRIPTOR, import module name, 
	//vector of: IMAGE_BOUND_FORWARDER_REF, forwarder module name.
	using LIBPE_BOUNDIMPORT_VEC = std::vector<std::tuple<IMAGE_BOUND_IMPORT_DESCRIPTOR, std::string,
		std::vector<std::tuple<IMAGE_BOUND_FORWARDER_REF, std::string>>>>;
	using PCLIBPE_BOUNDIMPORT_VEC = const LIBPE_BOUNDIMPORT_VEC*;

	//Delay import table.
	//Vector of IMAGE_DELAYLOAD_DESCRIPTOR, module name, vector of:
	//Hint/Ordinal, Func name, ThunkName RVA, ThunkIAT RVA, ThunkBoundIAT RVA, ThunkUnloadedInfoIAT RVA.
	using LIBPE_DELAYIMPORT_VEC = std::vector<std::tuple<IMAGE_DELAYLOAD_DESCRIPTOR, std::string,
		std::vector<std::tuple<LONGLONG, std::string, LONGLONG, LONGLONG, LONGLONG, LONGLONG>>>>;
	using PCLIBPE_DELAYIMPORT_VEC = const LIBPE_DELAYIMPORT_VEC*;

	//COM descriptor table.
	using PCLIBPE_COMDESCRIPTOR = const IMAGE_COR20_HEADER*;

	//Pure Virtual base class Ilibpe.
	class  Ilibpe
	{
	public:
		virtual HRESULT LoadPe(LPCWSTR) = 0;
		virtual HRESULT GetFileSummary(PCDWORD&) = 0;
		virtual HRESULT GetMSDOSHeader(PCLIBPE_DOSHEADER&) = 0;
		virtual HRESULT GetRichHeader(PCLIBPE_RICHHEADER_VEC&) = 0;
		virtual HRESULT GetNTHeader(PCLIBPE_NTHEADER_VAR&) = 0;
		virtual HRESULT GetFileHeader(PCLIBPE_FILEHEADER&) = 0;
		virtual HRESULT GetOptionalHeader(PCLIBPE_OPTHEADER_VAR&) = 0;
		virtual HRESULT GetDataDirectories(PCLIBPE_DATADIRS_VEC&) = 0;
		virtual HRESULT GetSectionsHeaders(PCLIBPE_SECHEADERS_VEC&) = 0;
		virtual HRESULT GetExportTable(PCLIBPE_EXPORT_TUP&) = 0;
		virtual HRESULT GetImportTable(PCLIBPE_IMPORT_VEC&) = 0;
		virtual HRESULT GetResourceTable(PCLIBPE_RESOURCE_ROOT_TUP&) = 0;
		virtual HRESULT GetExceptionTable(PCLIBPE_EXCEPTION_VEC&) = 0;
		virtual HRESULT GetSecurityTable(PCLIBPE_SECURITY_VEC&) = 0;
		virtual HRESULT GetRelocationTable(PCLIBPE_RELOCATION_VEC&) = 0;
		virtual HRESULT GetDebugTable(PCLIBPE_DEBUG_VEC&) = 0;
		virtual HRESULT GetTLSTable(PCLIBPE_TLS_TUP&) = 0;
		virtual HRESULT GetLoadConfigTable(PCLIBPE_LOADCONFIGTABLE_VAR&) = 0;
		virtual HRESULT GetBoundImportTable(PCLIBPE_BOUNDIMPORT_VEC&) = 0;
		virtual HRESULT GetDelayImportTable(PCLIBPE_DELAYIMPORT_VEC&) = 0;
		virtual HRESULT GetCOMDescriptorTable(PCLIBPE_COMDESCRIPTOR&) = 0;
	};
	using libpe_ptr = std::shared_ptr<Ilibpe>;

	/*************************************************
	* Return errors.								 *
	*************************************************/

	constexpr auto E_CALL_LOADPE_FIRST = 0xFFFF;
	constexpr auto E_FILE_OPEN_FAILED = 0x0010;
	constexpr auto E_FILE_SIZE_TOO_SMALL = 0x0011;
	constexpr auto E_FILE_CREATE_FILE_MAPPING_FAILED = 0x0012;
	constexpr auto E_FILE_MAP_VIEW_OF_FILE_FAILED = 0x0013;
	constexpr auto E_FILE_SECTION_DATA_CORRUPTED = 0x0014;
	constexpr auto E_IMAGE_TYPE_UNSUPPORTED = 0x0015;
	constexpr auto E_IMAGE_HAS_NO_DOSHEADER = 0x0016;
	constexpr auto E_IMAGE_HAS_NO_RICHHEADER = 0x0017;
	constexpr auto E_IMAGE_HAS_NO_NTHEADER = 0x0018;
	constexpr auto E_IMAGE_HAS_NO_FILEHEADER = 0x0019;
	constexpr auto E_IMAGE_HAS_NO_OPTHEADER = 0x001A;
	constexpr auto E_IMAGE_HAS_NO_DATADIRECTORIES = 0x001B;
	constexpr auto E_IMAGE_HAS_NO_SECTIONS = 0x001C;
	constexpr auto E_IMAGE_HAS_NO_EXPORT = 0x001D;
	constexpr auto E_IMAGE_HAS_NO_IMPORT = 0x001E;
	constexpr auto E_IMAGE_HAS_NO_RESOURCE = 0x001F;
	constexpr auto E_IMAGE_HAS_NO_EXCEPTION = 0x0020;
	constexpr auto E_IMAGE_HAS_NO_SECURITY = 0x0021;
	constexpr auto E_IMAGE_HAS_NO_BASERELOC = 0x0022;
	constexpr auto E_IMAGE_HAS_NO_DEBUG = 0x0023;
	constexpr auto E_IMAGE_HAS_NO_ARCHITECTURE = 0x0024;
	constexpr auto E_IMAGE_HAS_NO_GLOBALPTR = 0x0025;
	constexpr auto E_IMAGE_HAS_NO_TLS = 0x0026;
	constexpr auto E_IMAGE_HAS_NO_LOADCONFIG = 0x0027;
	constexpr auto E_IMAGE_HAS_NO_BOUNDIMPORT = 0x0028;
	constexpr auto E_IMAGE_HAS_NO_IAT = 0x0029;
	constexpr auto E_IMAGE_HAS_NO_DELAYIMPORT = 0x002A;
	constexpr auto E_IMAGE_HAS_NO_COMDESCRIPTOR = 0x002B;

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
}

#if defined(ILIBPE_EXPORT)
#define ILIBPEAPI __declspec(dllexport) __cdecl
#else 
#define ILIBPEAPI __declspec(dllimport) __cdecl
#endif

extern "C" HRESULT ILIBPEAPI Getlibpe(libpe::libpe_ptr& libpe_ptr);