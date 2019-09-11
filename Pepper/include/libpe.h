/****************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/                         *
* Windows library for reading PE (x86) and PE+ (x64) files' inner information.	        *
* Official git repository: https://github.com/jovibor/libpe                             *
* This software is available under the "MIT License".                                   *
****************************************************************************************/
#pragma once
#include <vector>     //std::vector and related.
#include <memory>     //std::shared_ptr and related.
#include <string>     //std::string and related.
#include <Windows.h>  //All standard Windows' typedefs.
#include <WinTrust.h> //WIN_CERTIFICATE struct.

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
	struct LIBPE_RICH {
		DWORD dwOffsetRich; //File's raw offset of the entry.
		WORD  wId;          //Entry Id.
		WORD  wVersion;     //Entry version.
		DWORD dwCount;      //Amount of occurrences.
	};
	using LIBPE_RICHHEADER_VEC = std::vector<LIBPE_RICH>;
	using PCLIBPE_RICHHEADER_VEC = const LIBPE_RICHHEADER_VEC*;

	//NT header.
	struct LIBPE_NTHEADER {
		DWORD dwOffsetNTHdrDesc; //File's raw offset of the header.
		union LIBPE_NTHEADER_VAR {
			IMAGE_NT_HEADERS32 stNTHdr32; //x86 Header.
			IMAGE_NT_HEADERS64 stNTHdr64; //x64 Header.
		}varHdr;
	};
	using PCLIBPE_NTHEADER = const LIBPE_NTHEADER*;

	//File header.
	using PCLIBPE_FILEHEADER = const IMAGE_FILE_HEADER*;

	//Optional header.
	union LIBPE_OPTHEADER_VAR {
		IMAGE_OPTIONAL_HEADER32 stOptHdr32; //x86 header.
		IMAGE_OPTIONAL_HEADER64 stOptHdr64; //x64 header.
	};
	using PCLIBPE_OPTHEADER_VAR = const LIBPE_OPTHEADER_VAR*;

	//Data directories.
	struct LIBPE_DATADIR {
		IMAGE_DATA_DIRECTORY stDataDir;       //Standard header.
		std::string          strSecResidesIn; //Name of the section this directory resides in (points to).
	};
	using LIBPE_DATADIRS_VEC = std::vector<LIBPE_DATADIR>;
	using PCLIBPE_DATADIRS_VEC = const LIBPE_DATADIRS_VEC*;

	//Sections headers.
	//For more info check:
	//docs.microsoft.com/en-us/windows/desktop/api/winnt/ns-winnt-_image_section_header#members
	//«An 8-byte, null-padded UTF-8 string. For longer names, this member contains a forward slash (/) 
	//followed by an ASCII representation of a decimal number that is an offset into the string table.»
	struct LIBPE_SECHEADERS {
		DWORD                 dwOffsetSecHdrDesc; //File's raw offset of the section header descriptor.
		IMAGE_SECTION_HEADER  stSecHdr;           //Standard section header.
		std::string           strSecName;         //Section full name.
	};
	using LIBPE_SECHEADERS_VEC = std::vector<LIBPE_SECHEADERS>;
	using PCLIBPE_SECHEADERS_VEC = const LIBPE_SECHEADERS_VEC*;

	//Export table.
	struct LIBPE_EXPORT_FUNC {
		DWORD       dwRVA;            //Function RVA.
		DWORD       dwOrdinal;        //Function ordinal.
		std::string strFuncName;      //Function name.
		std::string strForwarderName; //Function forwarder name.
	};
	struct LIBPE_EXPORT {
		DWORD                           dwOffsetExportDesc; //File's raw offset of the Export header descriptor.
		IMAGE_EXPORT_DIRECTORY          stExportDesc;       //Standard export header descriptor.
		std::string                     strModuleName;      //Actual module name.
		std::vector<LIBPE_EXPORT_FUNC>  vecFuncs;           //Array of the exported functions struct.	
	};
	using PCLIBPE_EXPORT = const LIBPE_EXPORT*;

	//Import table:
	struct LIBPE_IMPORT_FUNC {
		union LIBPE_IMPORT_THUNK_VAR {
			IMAGE_THUNK_DATA32 stThunk32; //x86 standard thunk.
			IMAGE_THUNK_DATA64 stThunk64; //x64 standard thunk.
		}varThunk;
		IMAGE_IMPORT_BY_NAME stImpByName; //Standard IMAGE_IMPORT_BY_NAME struct
		std::string          strFuncName; //Function name.
	};
	struct LIBPE_IMPORT_MODULE {
		DWORD                          dwOffsetImpDesc; //File's raw offset of the Import descriptor.
		IMAGE_IMPORT_DESCRIPTOR        stImportDesc;    //Standard Import descriptor.
		std::string                    strModuleName;   //Imported module name.
		std::vector<LIBPE_IMPORT_FUNC> vecImportFunc;   //Array of imported functions.
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
	* Highest (root) resource structure is LIBPE_RESOURCE_ROOT. It's a struct							*
	* that includes: an IMAGE_RESOURCE_DIRECTORY of root resource directory itself, 					*
	* and LIBPE_RESOURCE_ROOT_DATA_VEC, that is actually an std::vector that includes structs of all	*
	* IMAGE_RESOURCE_DIRECTORY_ENTRY structures of the root resource directory.							*
	* It also includes: std::wstring(Resource name), IMAGE_RESOURCE_DATA_ENTRY, 						*
	* std::vector<std::byte> (RAW resource data), and LIBPE_RESOURCE_LVL2 that is, in fact,				*
	* a struct of the next, second, resource level, that replicates struct of root resource level.		*
	* LIBPE_RESOURCE_LVL2 includes IMAGE_RESOURCE_DIRECTORY of second resource level, and 				*
	* LIBPE_RESOURCE_LVL2_DATA_VEC that includes LIBPE_RESOURCE_LVL3 that is a struct of the			*
	* last, third, level of resources.																	*
	* Like previous two, this last level's struct consist of IMAGE_RESOURCE_DIRECTORY 					*
	* and LIBPE_RESOURCE_LVL3_DATA_VEC, that is again — vector of structs of all 						*
	* IMAGE_RESOURCE_DIRECTORY_ENTRY of the last, third, level of resources. See the code below.		*
	****************************************************************************************************/

	//Level 3 (the lowest) Resources.
	struct LIBPE_RESOURCE_LVL3_DATA {
		IMAGE_RESOURCE_DIRECTORY_ENTRY stResDirEntryLvL3;   //Level 3 standard IMAGE_RESOURCE_DIRECTORY_ENTRY struct.
		std::wstring                   wstrResNameLvL3;     //Level 3 resource name.
		IMAGE_RESOURCE_DATA_ENTRY      stResDataEntryLvL3;  //Level 3 standard IMAGE_RESOURCE_DATA_ENTRY struct.
		std::vector<std::byte>         vecResRawDataLvL3;   //Level 3 resource raw data.
	};
	struct LIBPE_RESOURCE_LVL3 {
		DWORD                                 dwOffsetResLvL3; //File's raw offset of the level 3 IMAGE_RESOURCE_DIRECTORY descriptor.
		IMAGE_RESOURCE_DIRECTORY              stResDirLvL3;    //Level 3 standard IMAGE_RESOURCE_DIRECTORY header.
		std::vector<LIBPE_RESOURCE_LVL3_DATA> vecResLvL3;      //Array of level 3 resource entries.
	};
	using PCLIBPE_RESOURCE_LVL3 = const LIBPE_RESOURCE_LVL3*;

	//Level 2 Resources — Includes LVL3 Resourses.
	struct LIBPE_RESOURCE_LVL2_DATA {
		IMAGE_RESOURCE_DIRECTORY_ENTRY stResDirEntryLvL2;  //Level 2 standard IMAGE_RESOURCE_DIRECTORY_ENTRY struct.
		std::wstring                   wstrResNameLvL2;	   //Level 2 resource name.
		IMAGE_RESOURCE_DATA_ENTRY      stResDataEntryLvL2; //Level 2 standard IMAGE_RESOURCE_DATA_ENTRY struct.
		std::vector<std::byte>         vecResRawDataLvL2;  //Level 2 resource raw data.
		LIBPE_RESOURCE_LVL3            stResLvL3;          //Level 3 resource struct.
	};
	struct LIBPE_RESOURCE_LVL2 {
		DWORD                                 dwOffsetResLvL2; //File's raw offset of the level 2 IMAGE_RESOURCE_DIRECTORY descriptor.
		IMAGE_RESOURCE_DIRECTORY              stResDirLvL2;    //Level 2 standard IMAGE_RESOURCE_DIRECTORY header.
		std::vector<LIBPE_RESOURCE_LVL2_DATA> vecResLvL2;      //Array of level 2 resource entries.
	};
	using PCLIBPE_RESOURCE_LVL2 = const LIBPE_RESOURCE_LVL2*;

	//Level 1 (Root) Resources — Includes LVL2 Resources.
	struct LIBPE_RESOURCE_ROOT_DATA {
		IMAGE_RESOURCE_DIRECTORY_ENTRY stResDirEntryRoot;  //Level root standard IMAGE_RESOURCE_DIRECTORY_ENTRY struct.
		std::wstring                   wstrResNameRoot;	   //Level root resource name.
		IMAGE_RESOURCE_DATA_ENTRY      stResDataEntryRoot; //Level root standard IMAGE_RESOURCE_DATA_ENTRY struct.
		std::vector<std::byte>         vecResRawDataRoot;  //Level root resource raw data.
		LIBPE_RESOURCE_LVL2            stResLvL2;          //Level 2 resource struct.
	};
	struct LIBPE_RESOURCE_ROOT {
		DWORD                                 dwOffsetResRoot; //File's raw offset of the level 1 IMAGE_RESOURCE_DIRECTORY descriptor.
		IMAGE_RESOURCE_DIRECTORY              stResDirRoot;    //Level 1 standard IMAGE_RESOURCE_DIRECTORY header.
		std::vector<LIBPE_RESOURCE_ROOT_DATA> vecResRoot;      //Array of level 1 resource entries.
	};
	using PCLIBPE_RESOURCE_ROOT = const LIBPE_RESOURCE_ROOT*;
	/*********************************Resources End*****************************************/

	//Exception table.
	struct LIBPE_EXCEPTION {
		DWORD                         dwOffsetRuntimeFuncDesc; //File's raw offset of the exceptions descriptor.
		_IMAGE_RUNTIME_FUNCTION_ENTRY stRuntimeFuncEntry;      //Standard _IMAGE_RUNTIME_FUNCTION_ENTRY header.
	};
	using LIBPE_EXCEPTION_VEC = std::vector<LIBPE_EXCEPTION>;
	using PCLIBPE_EXCEPTION_VEC = const LIBPE_EXCEPTION_VEC*;

	//Security table.
	struct LIBPE_SECURITY {
		DWORD           dwOffsetWinCertDesc; //File's raw offset of the security descriptor.
		WIN_CERTIFICATE stWinSert;           //Standard WIN_CERTIFICATE header.
	};
	using LIBPE_SECURITY_VEC = std::vector<LIBPE_SECURITY>;
	using PCLIBPE_SECURITY_VEC = const LIBPE_SECURITY_VEC*;

	//Relocation table.
	struct LIBPE_RELOC_DATA {
		DWORD dwOffsetRelocData; //File's raw offset of the Relocation data descriptor.
		WORD  wRelocType;        //Relocation type.
		WORD  wRelocOffset;      //Relocation offset (Offset the relocation must be applied to.)
	};
	struct LIBPE_RELOCATION {
		DWORD                         dwOffsetReloc; //File's raw offset of the Relocation descriptor.
		IMAGE_BASE_RELOCATION         stBaseReloc;   //Standard IMAGE_BASE_RELOCATION header.
		std::vector<LIBPE_RELOC_DATA> vecRelocData;  //Array of the Relocation data struct.
	};
	using LIBPE_RELOCATION_VEC = std::vector<LIBPE_RELOCATION>;
	using PCLIBPE_RELOCATION_VEC = const LIBPE_RELOCATION_VEC*;

	//Debug table.
	struct LIBPE_DEBUG_DBGHDR
	{
		//dwHdr[6] is an array of the first six DWORDs of IMAGE_DEBUG_DIRECTORY::PointerToRawData data (Debug info header).
		//Their meaning varies depending on dwHdr[0] (Signature) value.
		//If dwHdr[0] == 0x53445352 (Ascii "RSDS") it's PDB 7.0 file:
		// Then dwHdr[1]-dwHdr[4] is GUID (*((GUID*)&dwHdr[1])). dwHdr[5] is Counter/Age.
		//If dwHdr[0] == 0x3031424E (Ascii "NB10") it's PDB 2.0 file:
		// Then dwHdr[1] is Offset. dwHdr[2] is Time/Signature. dwHdr[3] is Counter/Age.
		DWORD       dwHdr[6];
		std::string strPDBName; //PDB file name/path.
	};
	struct LIBPE_DEBUG {
		DWORD                 dwOffsetDebug;  //File's raw offset of the Debug descriptor.
		IMAGE_DEBUG_DIRECTORY stDebugDir;     //Standard IMAGE_DEBUG_DIRECTORY header.
		LIBPE_DEBUG_DBGHDR    stDebugHdrInfo; //Debug info header.
	};
	using LIBPE_DEBUG_VEC = std::vector<LIBPE_DEBUG>;
	using PCLIBPE_DEBUG_VEC = const LIBPE_DEBUG_VEC*;

	//TLS table.
	struct LIBPE_TLS {
		DWORD              dwOffsetTLS;       //File's raw offset of the TLS header descriptor.
		union LIBPE_TLS_VAR {
			IMAGE_TLS_DIRECTORY32 stTLSDir32; //x86 standard TLS header.
			IMAGE_TLS_DIRECTORY64 stTLSDir64; //x64 TLS header.
		}varTLS;
		std::vector<DWORD> vecTLSCallbacks;   //Array of the TLS callbacks.
	};
	using PCLIBPE_TLS = const LIBPE_TLS*;

	//LoadConfigDirectory.
	struct LIBPE_LOADCONFIG {
		DWORD dwOffsetLCD; //File's raw offset of the LCD descriptor.
		union LIBPE_LOADCONFIG_VAR {
			IMAGE_LOAD_CONFIG_DIRECTORY32 stLCD32; //x86 LCD descriptor.
			IMAGE_LOAD_CONFIG_DIRECTORY64 stLCD64; //x64 LCD descriptor.
		}varLCD;
	};
	using PCLIBPE_LOADCONFIG = const LIBPE_LOADCONFIG*;

	//Bound import table.
	struct LIBPE_BOUNDFORWARDER {
		DWORD                     dwOffsetBoundForwDesc; //File's raw offset of the Bound Forwarder descriptor.
		IMAGE_BOUND_FORWARDER_REF stBoundForwarder;      //Standard IMAGE_BOUND_FORWARDER_REF struct.
		std::string               strBoundForwarderName; //Bound forwarder name.
	};
	struct LIBPE_BOUNDIMPORT {
		DWORD                             dwOffsetBoundImpDesc; //File's raw offset of the Bound Import descriptor.
		IMAGE_BOUND_IMPORT_DESCRIPTOR     stBoundImpDesc;       //Standard IMAGE_BOUND_IMPORT_DESCRIPTOR struct.
		std::string                       strBoundName;         //Bound Import name.
		std::vector<LIBPE_BOUNDFORWARDER> vecBoundForwarder;    //Array of the Bound Forwarder structs.
	};
	using LIBPE_BOUNDIMPORT_VEC = std::vector<LIBPE_BOUNDIMPORT>;
	using PCLIBPE_BOUNDIMPORT_VEC = const LIBPE_BOUNDIMPORT_VEC*;

	//Delay import table.
	struct LIBPE_DELAYIMPORT_FUNC {
		union LIBPE_DELAYIMPORT_THUNK_VAR
		{
			struct x32 {
				IMAGE_THUNK_DATA32 stImportAddressTable;      //x86 Import Address Table struct.
				IMAGE_THUNK_DATA32 stImportNameTable;         //x86 Import Name Table struct.
				IMAGE_THUNK_DATA32 stBoundImportAddressTable; //x86 Bound Import Address Table struct.
				IMAGE_THUNK_DATA32 stUnloadInformationTable;  //x86 Unload Information Table struct.
			}st32;
			struct x64 {
				IMAGE_THUNK_DATA64 stImportAddressTable;      //x64 Import Address Table struct.
				IMAGE_THUNK_DATA64 stImportNameTable;         //x64 Import Name Table struct.
				IMAGE_THUNK_DATA64 stBoundImportAddressTable; //x64 Bound Import Address Table struct
				IMAGE_THUNK_DATA64 stUnloadInformationTable;  //x64 Unload Information Table struct.
			}st64;
		}varThunk;
		IMAGE_IMPORT_BY_NAME stImpByName; //Standard IMAGE_IMPORT_BY_NAME struct.
		std::string          strFuncName; //Function name.
	};
	struct LIBPE_DELAYIMPORT {
		DWORD                               dwOffsetDelayImpDesc; //File's raw offset of the Delay Import descriptor.
		IMAGE_DELAYLOAD_DESCRIPTOR          stDelayImpDesc;       //Standard IMAGE_DELAYLOAD_DESCRIPTOR struct.
		std::string                         strModuleName;        //Import module name.
		std::vector<LIBPE_DELAYIMPORT_FUNC> vecDelayImpFunc;      //Array of the Delay Import module functions.
	};
	using LIBPE_DELAYIMPORT_VEC = std::vector<LIBPE_DELAYIMPORT>;
	using PCLIBPE_DELAYIMPORT_VEC = const LIBPE_DELAYIMPORT_VEC*;

	//COM descriptor table.
	struct LIBPE_COMDESCRIPTOR {
		DWORD              dwOffsetComDesc; //File's raw offset of the IMAGE_COR20_HEADER descriptor.
		IMAGE_COR20_HEADER stCorHdr;        //Standard IMAGE_COR20_HEADER struct.
	};
	using PCLIBPE_COMDESCRIPTOR = const LIBPE_COMDESCRIPTOR*;

	//Pure abstract base class Ilibpe.
	class Ilibpe
	{
	public:
		virtual ~Ilibpe() = default;
		virtual HRESULT LoadPe(LPCWSTR) = 0;
		virtual HRESULT GetImageInfo(DWORD&)const noexcept = 0;
		virtual HRESULT GetImageFlag(DWORD dwFlag, bool& f)const noexcept = 0;
		virtual HRESULT GetOffsetFromRVA(ULONGLONG ullRVA, DWORD& dwOffset)const noexcept = 0;
		virtual HRESULT GetOffsetFromVA(ULONGLONG ullVA, DWORD& dwOffset)const noexcept = 0;
		virtual HRESULT GetMSDOSHeader(PCLIBPE_DOSHEADER&)const noexcept = 0;
		virtual HRESULT GetRichHeader(PCLIBPE_RICHHEADER_VEC&)const noexcept = 0;
		virtual HRESULT GetNTHeader(PCLIBPE_NTHEADER&)const noexcept = 0;
		virtual HRESULT GetFileHeader(PCLIBPE_FILEHEADER&)const noexcept = 0;
		virtual HRESULT GetOptionalHeader(PCLIBPE_OPTHEADER_VAR&)const noexcept = 0;
		virtual HRESULT GetDataDirectories(PCLIBPE_DATADIRS_VEC&)const noexcept = 0;
		virtual HRESULT GetSectionsHeaders(PCLIBPE_SECHEADERS_VEC&)const noexcept = 0;
		virtual HRESULT GetExport(PCLIBPE_EXPORT&)const noexcept = 0;
		virtual HRESULT GetImport(PCLIBPE_IMPORT_VEC&)const noexcept = 0;
		virtual HRESULT GetResources(PCLIBPE_RESOURCE_ROOT&)const noexcept = 0;
		virtual HRESULT GetExceptions(PCLIBPE_EXCEPTION_VEC&)const noexcept = 0;
		virtual HRESULT GetSecurity(PCLIBPE_SECURITY_VEC&)const noexcept = 0;
		virtual HRESULT GetRelocations(PCLIBPE_RELOCATION_VEC&)const noexcept = 0;
		virtual HRESULT GetDebug(PCLIBPE_DEBUG_VEC&)const noexcept = 0;
		virtual HRESULT GetTLS(PCLIBPE_TLS&)const noexcept = 0;
		virtual HRESULT GetLoadConfig(PCLIBPE_LOADCONFIG&)const noexcept = 0;
		virtual HRESULT GetBoundImport(PCLIBPE_BOUNDIMPORT_VEC&)const noexcept = 0;
		virtual HRESULT GetDelayImport(PCLIBPE_DELAYIMPORT_VEC&)const noexcept = 0;
		virtual HRESULT GetCOMDescriptor(PCLIBPE_COMDESCRIPTOR&)const noexcept = 0;
		virtual HRESULT Destroy() = 0;
	};

	/*************************************************
	* Return errors.                                 *
	*************************************************/

	constexpr auto E_CALL_LOADPE_FIRST = 0xFFFFu;
	constexpr auto E_FILE_CREATEFILE_FAILED = 0x0010u;
	constexpr auto E_FILE_SIZE_TOO_SMALL = 0x0011u;
	constexpr auto E_FILE_CREATEFILEMAPPING_FAILED = 0x0012u;
	constexpr auto E_FILE_MAPVIEWOFFILE_FAILED = 0x0013;
	constexpr auto E_FILE_MAPVIEWOFFILE_SECTION_FAILED = 0x0014u;
	constexpr auto E_FILE_SECTION_DATA_CORRUPTED = 0x0015u;
	constexpr auto E_IMAGE_TYPE_UNSUPPORTED = 0x0016u;
	constexpr auto E_IMAGE_HAS_NO_DOSHEADER = 0x0017u;
	constexpr auto E_IMAGE_HAS_NO_RICHHEADER = 0x0018u;
	constexpr auto E_IMAGE_HAS_NO_NTHEADER = 0x0019u;
	constexpr auto E_IMAGE_HAS_NO_FILEHEADER = 0x001Au;
	constexpr auto E_IMAGE_HAS_NO_OPTHEADER = 0x001Bu;
	constexpr auto E_IMAGE_HAS_NO_DATADIRECTORIES = 0x001Cu;
	constexpr auto E_IMAGE_HAS_NO_SECTIONS = 0x001Du;
	constexpr auto E_IMAGE_HAS_NO_EXPORT = 0x001Eu;
	constexpr auto E_IMAGE_HAS_NO_IMPORT = 0x001Fu;
	constexpr auto E_IMAGE_HAS_NO_RESOURCE = 0x0020u;
	constexpr auto E_IMAGE_HAS_NO_EXCEPTION = 0x0021u;
	constexpr auto E_IMAGE_HAS_NO_SECURITY = 0x0022u;
	constexpr auto E_IMAGE_HAS_NO_BASERELOC = 0x0023u;
	constexpr auto E_IMAGE_HAS_NO_DEBUG = 0x0024u;
	constexpr auto E_IMAGE_HAS_NO_ARCHITECTURE = 0x0025u;
	constexpr auto E_IMAGE_HAS_NO_GLOBALPTR = 0x0026u;
	constexpr auto E_IMAGE_HAS_NO_TLS = 0x0027u;
	constexpr auto E_IMAGE_HAS_NO_LOADCONFIG = 0x0028u;
	constexpr auto E_IMAGE_HAS_NO_BOUNDIMPORT = 0x0029u;
	constexpr auto E_IMAGE_HAS_NO_IAT = 0x002Au;
	constexpr auto E_IMAGE_HAS_NO_DELAYIMPORT = 0x002Bu;
	constexpr auto E_IMAGE_HAS_NO_COMDESCRIPTOR = 0x002Cu;

	/*****************************************************
	* Flags according to loaded PE file properties.      *
	*****************************************************/
	//Tiny function shows whether given DWORD has given flag.
	constexpr bool ImageHasFlag(DWORD dwFileInfo, DWORD dwFlag) { return dwFileInfo & dwFlag; };
	constexpr auto IMAGE_FLAG_PE32 = 0x00000001ul;
	constexpr auto IMAGE_FLAG_PE64 = 0x00000002ul;
	constexpr auto IMAGE_FLAG_DOSHEADER = 0x00000004ul;
	constexpr auto IMAGE_FLAG_RICHHEADER = 0x00000008ul;
	constexpr auto IMAGE_FLAG_NTHEADER = 0x00000010ul;
	constexpr auto IMAGE_FLAG_FILEHEADER = 0x00000020ul;
	constexpr auto IMAGE_FLAG_OPTHEADER = 0x00000040ul;
	constexpr auto IMAGE_FLAG_DATADIRECTORIES = 0x00000080ul;
	constexpr auto IMAGE_FLAG_SECTIONS = 0x00000100ul;
	constexpr auto IMAGE_FLAG_EXPORT = 0x00000200ul;
	constexpr auto IMAGE_FLAG_IMPORT = 0x00000400ul;
	constexpr auto IMAGE_FLAG_RESOURCE = 0x00000800ul;
	constexpr auto IMAGE_FLAG_EXCEPTION = 0x00001000ul;
	constexpr auto IMAGE_FLAG_SECURITY = 0x00002000ul;
	constexpr auto IMAGE_FLAG_BASERELOC = 0x00004000ul;
	constexpr auto IMAGE_FLAG_DEBUG = 0x00008000ul;
	constexpr auto IMAGE_FLAG_ARCHITECTURE = 0x00010000ul;
	constexpr auto IMAGE_FLAG_GLOBALPTR = 0x00020000ul;
	constexpr auto IMAGE_FLAG_TLS = 0x00040000ul;
	constexpr auto IMAGE_FLAG_LOADCONFIG = 0x00080000ul;
	constexpr auto IMAGE_FLAG_BOUNDIMPORT = 0x00100000ul;
	constexpr auto IMAGE_FLAG_IAT = 0x00200000ul;
	constexpr auto IMAGE_FLAG_DELAYIMPORT = 0x00400000ul;
	constexpr auto IMAGE_FLAG_COMDESCRIPTOR = 0x00800000ul;

	/********************************************************************************************
	* Factory function Createlibpe returns IlibpeUnPtr - unique_ptr with custom deleter.        *
	* In client code you should use libpe_ptr type which is an alias to either IlibpeUnPtr -    *
	* a unique_ptr, or IlibpeShPtr - a shared_ptr. Uncomment what serves best for you, and      *
	* comment out the other.                                                                    *
	* If you, for some reason, need a raw pointer, you can directly call CreateRawlibpe         *
	* function, which returns Ilibpe interface pointer, but in this case you will need to       *
	* call Ilibpe::Destroy method afterwards manually - to delete Ilibpe object.                *
	********************************************************************************************/
#ifdef ILIBPE_EXPORT
#define ILIBPEAPI __declspec(dllexport)
#else
#define ILIBPEAPI __declspec(dllimport)
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
	* End of .lib name macros.                              *
	********************************************************/
#pragma comment(lib, LIBNAME_PROPER("libpe"))
#endif

	extern "C" ILIBPEAPI HRESULT __cdecl CreateRawlibpe(Ilibpe*&);
	using IlibpeUnPtr = std::unique_ptr<Ilibpe, void(*)(Ilibpe*)>;
	using IlibpeShPtr = std::shared_ptr<Ilibpe>;

	inline IlibpeUnPtr Createlibpe()
	{
		Ilibpe* ptr { };
		if (CreateRawlibpe(ptr) == S_OK)
			return IlibpeUnPtr(ptr, [](Ilibpe * p) { p->Destroy(); });
		else
			return IlibpeUnPtr(nullptr, nullptr);
	};

	//using libpe_ptr = IlibpeUnPtr;
	using libpe_ptr = IlibpeShPtr;

	/********************************************
	* LIBPE_INFO: service info structure.       *
	********************************************/
	struct LIBPE_INFO
	{
		const wchar_t* pwszVersion { };        //WCHAR version string.
		union {
			unsigned long long ullVersion { }; //ULONGLONG version number.
			struct {
				short wMajor;
				short wMinor;
				short wMaintenance;
				short wRevision;
			}stVersion;
		};
	};
	using PCLIBPE_INFO = const LIBPE_INFO*;

	/*********************************************
	* Service info export/import function.       *
	* Returns pointer to LIBPE_INFO struct.      *
	*********************************************/
	extern "C" ILIBPEAPI PCLIBPE_INFO __cdecl libpeInfo();
}