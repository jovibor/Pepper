/****************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/                         *
* Windows library for reading PE (x86) and PE+ (x64) files' inner information.	        *
* Official git repository: https://github.com/jovibor/libpe                             *
* This software is available under the "MIT License".                                   *
****************************************************************************************/
#pragma once
#include <WinTrust.h> //WIN_CERTIFICATE struct.
#include <Windows.h>  //All standard Windows' typedefs.
#include <memory>     //std::shared_ptr and related.
#include <string>     //std::string and related.
#include <vector>     //std::vector and related.

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
	using PLIBPE_DOSHEADER = IMAGE_DOS_HEADER*;

	//Rich.
	struct LIBPE_RICH {
		DWORD dwOffsetRich; //File's raw offset of the entry.
		WORD  wId;          //Entry Id.
		WORD  wVersion;     //Entry version.
		DWORD dwCount;      //Amount of occurrences.
	};
	using LIBPE_RICHHEADER_VEC = std::vector<LIBPE_RICH>;
	using PLIBPE_RICHHEADER_VEC = LIBPE_RICHHEADER_VEC*;

	//NT header.
	struct LIBPE_NTHEADER {
		DWORD dwOffsetNTHdrDesc; //File's raw offset of the header.
		union LIBPE_NTHEADER_VAR {
			IMAGE_NT_HEADERS32 stNTHdr32; //x86 Header.
			IMAGE_NT_HEADERS64 stNTHdr64; //x64 Header.
		}varHdr;
	};
	using PLIBPE_NTHEADER = LIBPE_NTHEADER*;

	//File header.
	using PLIBPE_FILEHEADER = IMAGE_FILE_HEADER*;

	//Optional header.
	union LIBPE_OPTHEADER_VAR {
		IMAGE_OPTIONAL_HEADER32 stOptHdr32; //x86 header.
		IMAGE_OPTIONAL_HEADER64 stOptHdr64; //x64 header.
	};
	using PLIBPE_OPTHEADER_VAR = LIBPE_OPTHEADER_VAR*;

	//Data directories.
	struct LIBPE_DATADIR {
		IMAGE_DATA_DIRECTORY stDataDir;       //Standard header.
		std::string          strSecResidesIn; //Name of the section this directory resides in (points to).
	};
	using LIBPE_DATADIRS_VEC = std::vector<LIBPE_DATADIR>;
	using PLIBPE_DATADIRS_VEC = LIBPE_DATADIRS_VEC*;

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
	using PLIBPE_SECHEADERS_VEC = LIBPE_SECHEADERS_VEC*;

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
	using PLIBPE_EXPORT = LIBPE_EXPORT*;

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
	using PLIBPE_IMPORT_VEC = LIBPE_IMPORT_VEC*;

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
	using PLIBPE_RESOURCE_LVL3 = LIBPE_RESOURCE_LVL3*;

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
	using PLIBPE_RESOURCE_LVL2 = LIBPE_RESOURCE_LVL2*;

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
	using PLIBPE_RESOURCE_ROOT = LIBPE_RESOURCE_ROOT*;
	/*********************************Resources End*****************************************/

	//Exception table.
	struct LIBPE_EXCEPTION {
		DWORD                         dwOffsetRuntimeFuncDesc; //File's raw offset of the exceptions descriptor.
		_IMAGE_RUNTIME_FUNCTION_ENTRY stRuntimeFuncEntry;      //Standard _IMAGE_RUNTIME_FUNCTION_ENTRY header.
	};
	using LIBPE_EXCEPTION_VEC = std::vector<LIBPE_EXCEPTION>;
	using PLIBPE_EXCEPTION_VEC = LIBPE_EXCEPTION_VEC*;

	//Security table.
	struct LIBPE_SECURITY {
		DWORD           dwOffsetWinCertDesc; //File's raw offset of the security descriptor.
		WIN_CERTIFICATE stWinSert;           //Standard WIN_CERTIFICATE header.
	};
	using LIBPE_SECURITY_VEC = std::vector<LIBPE_SECURITY>;
	using PLIBPE_SECURITY_VEC = const LIBPE_SECURITY_VEC*;

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
	using PLIBPE_RELOCATION_VEC = LIBPE_RELOCATION_VEC*;

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
	using PLIBPE_DEBUG_VEC = LIBPE_DEBUG_VEC*;

	//TLS table.
	struct LIBPE_TLS {
		DWORD              dwOffsetTLS;       //File's raw offset of the TLS header descriptor.
		union LIBPE_TLS_VAR {
			IMAGE_TLS_DIRECTORY32 stTLSDir32; //x86 standard TLS header.
			IMAGE_TLS_DIRECTORY64 stTLSDir64; //x64 TLS header.
		}varTLS;
		std::vector<DWORD> vecTLSCallbacks;   //Array of the TLS callbacks.
	};
	using PLIBPE_TLS = LIBPE_TLS*;

	//LoadConfigDirectory.
	struct LIBPE_LOADCONFIG {
		DWORD dwOffsetLCD; //File's raw offset of the LCD descriptor.
		union LIBPE_LOADCONFIG_VAR {
			IMAGE_LOAD_CONFIG_DIRECTORY32 stLCD32; //x86 LCD descriptor.
			IMAGE_LOAD_CONFIG_DIRECTORY64 stLCD64; //x64 LCD descriptor.
		}varLCD;
	};
	using PLIBPE_LOADCONFIG = const LIBPE_LOADCONFIG*;

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
	using PLIBPE_BOUNDIMPORT_VEC = LIBPE_BOUNDIMPORT_VEC*;

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
	using PLIBPE_DELAYIMPORT_VEC = LIBPE_DELAYIMPORT_VEC*;

	//COM descriptor table.
	struct LIBPE_COMDESCRIPTOR {
		DWORD              dwOffsetComDesc; //File's raw offset of the IMAGE_COR20_HEADER descriptor.
		IMAGE_COR20_HEADER stCorHdr;        //Standard IMAGE_COR20_HEADER struct.
	};
	using PLIBPE_COMDESCRIPTOR = LIBPE_COMDESCRIPTOR*;

	//Pure abstract base class Ilibpe.
	class Ilibpe
	{
	public:
		virtual HRESULT LoadPe(LPCWSTR pwszFilePath) = 0;
		virtual HRESULT GetImageInfo(DWORD& dwInfo)noexcept = 0;
		virtual HRESULT GetImageFlag(DWORD dwFlag, bool& f)const noexcept = 0;
		virtual HRESULT GetOffsetFromRVA(ULONGLONG ullRVA, DWORD& dwOffset)noexcept = 0;
		virtual HRESULT GetOffsetFromVA(ULONGLONG ullVA, DWORD& dwOffset)noexcept = 0;
		virtual HRESULT GetMSDOSHeader(PLIBPE_DOSHEADER& pDosHeader)noexcept = 0;
		virtual HRESULT GetRichHeader(PLIBPE_RICHHEADER_VEC& pVecRich)noexcept = 0;
		virtual HRESULT GetNTHeader(PLIBPE_NTHEADER& pVarNTHdr)noexcept = 0;
		virtual HRESULT GetFileHeader(PLIBPE_FILEHEADER& pFileHeader)noexcept = 0;
		virtual HRESULT GetOptionalHeader(PLIBPE_OPTHEADER_VAR& pVarOptHeader)noexcept = 0;
		virtual HRESULT GetDataDirectories(PLIBPE_DATADIRS_VEC& pVecDataDir)noexcept = 0;
		virtual HRESULT GetSectionsHeaders(PLIBPE_SECHEADERS_VEC& pVecSections)noexcept = 0;
		virtual HRESULT GetExport(PLIBPE_EXPORT& pExport)noexcept = 0;
		virtual HRESULT GetImport(PLIBPE_IMPORT_VEC& pVecImport)noexcept = 0;
		virtual HRESULT GetResources(PLIBPE_RESOURCE_ROOT& pResRoot)noexcept = 0;
		virtual HRESULT GetExceptions(PLIBPE_EXCEPTION_VEC& pVecException)noexcept = 0;
		virtual HRESULT GetSecurity(PLIBPE_SECURITY_VEC& pVecSecurity)noexcept = 0;
		virtual HRESULT GetRelocations(PLIBPE_RELOCATION_VEC& pVecRelocs)noexcept = 0;
		virtual HRESULT GetDebug(PLIBPE_DEBUG_VEC& pVecDebug)noexcept = 0;
		virtual HRESULT GetTLS(PLIBPE_TLS& pTLS)noexcept = 0;
		virtual HRESULT GetLoadConfig(PLIBPE_LOADCONFIG& pLCD)noexcept = 0;
		virtual HRESULT GetBoundImport(PLIBPE_BOUNDIMPORT_VEC& pVecBoundImp)noexcept = 0;
		virtual HRESULT GetDelayImport(PLIBPE_DELAYIMPORT_VEC& pVecDelayImp)noexcept = 0;
		virtual HRESULT GetCOMDescriptor(PLIBPE_COMDESCRIPTOR& pCOMDesc)noexcept = 0;
		virtual HRESULT Destroy() = 0;
	};

	/*************************************************
	* Return errors.                                 *
	*************************************************/

	constexpr auto E_CALL_LOADPE_FIRST = 0xFFFFU;
	constexpr auto E_FILE_CREATEFILE_FAILED = 0x0010U;
	constexpr auto E_FILE_SIZE_TOO_SMALL = 0x0011U;
	constexpr auto E_FILE_CREATEFILEMAPPING_FAILED = 0x0012U;
	constexpr auto E_FILE_MAPVIEWOFFILE_FAILED = 0x0013;
	constexpr auto E_FILE_MAPVIEWOFFILE_SECTION_FAILED = 0x0014U;
	constexpr auto E_FILE_SECTION_DATA_CORRUPTED = 0x0015U;
	constexpr auto E_IMAGE_TYPE_UNSUPPORTED = 0x0016U;
	constexpr auto E_IMAGE_HAS_NO_DOSHEADER = 0x0017U;
	constexpr auto E_IMAGE_HAS_NO_RICHHEADER = 0x0018U;
	constexpr auto E_IMAGE_HAS_NO_NTHEADER = 0x0019U;
	constexpr auto E_IMAGE_HAS_NO_FILEHEADER = 0x001AU;
	constexpr auto E_IMAGE_HAS_NO_OPTHEADER = 0x001BU;
	constexpr auto E_IMAGE_HAS_NO_DATADIRECTORIES = 0x001CU;
	constexpr auto E_IMAGE_HAS_NO_SECTIONS = 0x001DU;
	constexpr auto E_IMAGE_HAS_NO_EXPORT = 0x001EU;
	constexpr auto E_IMAGE_HAS_NO_IMPORT = 0x001FU;
	constexpr auto E_IMAGE_HAS_NO_RESOURCE = 0x0020U;
	constexpr auto E_IMAGE_HAS_NO_EXCEPTION = 0x0021U;
	constexpr auto E_IMAGE_HAS_NO_SECURITY = 0x0022U;
	constexpr auto E_IMAGE_HAS_NO_BASERELOC = 0x0023U;
	constexpr auto E_IMAGE_HAS_NO_DEBUG = 0x0024U;
	constexpr auto E_IMAGE_HAS_NO_ARCHITECTURE = 0x0025U;
	constexpr auto E_IMAGE_HAS_NO_GLOBALPTR = 0x0026U;
	constexpr auto E_IMAGE_HAS_NO_TLS = 0x0027U;
	constexpr auto E_IMAGE_HAS_NO_LOADCONFIG = 0x0028U;
	constexpr auto E_IMAGE_HAS_NO_BOUNDIMPORT = 0x0029U;
	constexpr auto E_IMAGE_HAS_NO_IAT = 0x002AU;
	constexpr auto E_IMAGE_HAS_NO_DELAYIMPORT = 0x002BU;
	constexpr auto E_IMAGE_HAS_NO_COMDESCRIPTOR = 0x002CU;

	/*****************************************************
	* Flags according to loaded PE file properties.      *
	*****************************************************/
	//Tiny function shows whether given DWORD has given flag.
	constexpr bool ImageHasFlag(DWORD dwFileInfo, DWORD dwFlag) { return dwFileInfo & dwFlag; };
	constexpr auto IMAGE_FLAG_PE32 = 0x00000001UL;
	constexpr auto IMAGE_FLAG_PE64 = 0x00000002UL;
	constexpr auto IMAGE_FLAG_DOSHEADER = 0x00000004UL;
	constexpr auto IMAGE_FLAG_RICHHEADER = 0x00000008UL;
	constexpr auto IMAGE_FLAG_NTHEADER = 0x00000010UL;
	constexpr auto IMAGE_FLAG_FILEHEADER = 0x00000020UL;
	constexpr auto IMAGE_FLAG_OPTHEADER = 0x00000040UL;
	constexpr auto IMAGE_FLAG_DATADIRECTORIES = 0x00000080UL;
	constexpr auto IMAGE_FLAG_SECTIONS = 0x00000100UL;
	constexpr auto IMAGE_FLAG_EXPORT = 0x00000200UL;
	constexpr auto IMAGE_FLAG_IMPORT = 0x00000400UL;
	constexpr auto IMAGE_FLAG_RESOURCE = 0x00000800UL;
	constexpr auto IMAGE_FLAG_EXCEPTION = 0x00001000UL;
	constexpr auto IMAGE_FLAG_SECURITY = 0x00002000UL;
	constexpr auto IMAGE_FLAG_BASERELOC = 0x00004000UL;
	constexpr auto IMAGE_FLAG_DEBUG = 0x00008000UL;
	constexpr auto IMAGE_FLAG_ARCHITECTURE = 0x00010000UL;
	constexpr auto IMAGE_FLAG_GLOBALPTR = 0x00020000UL;
	constexpr auto IMAGE_FLAG_TLS = 0x00040000UL;
	constexpr auto IMAGE_FLAG_LOADCONFIG = 0x00080000UL;
	constexpr auto IMAGE_FLAG_BOUNDIMPORT = 0x00100000UL;
	constexpr auto IMAGE_FLAG_IAT = 0x00200000UL;
	constexpr auto IMAGE_FLAG_DELAYIMPORT = 0x00400000UL;
	constexpr auto IMAGE_FLAG_COMDESCRIPTOR = 0x00800000UL;

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

	extern "C" ILIBPEAPI HRESULT __cdecl CreateRawlibpe(Ilibpe*& pLibpe);
	using IlibpeUnPtr = std::unique_ptr<Ilibpe, void(*)(Ilibpe*)>;
	using IlibpeShPtr = std::shared_ptr<Ilibpe>;

	inline IlibpeUnPtr Createlibpe()
	{
		Ilibpe* pLibpe { };
		if (CreateRawlibpe(pLibpe) == S_OK)
			return IlibpeUnPtr(pLibpe, [](Ilibpe * p) { p->Destroy(); });

		return { nullptr, nullptr };
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
	using PLIBPE_INFO = const LIBPE_INFO*;

	/*********************************************
	* Service info export/import function.       *
	* Returns pointer to LIBPE_INFO struct.      *
	*********************************************/
	extern "C" ILIBPEAPI PLIBPE_INFO __cdecl libpeInfo();
}