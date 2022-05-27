/****************************************************************************************
* Copyright © 2018-2022, Jovibor: https://github.com/jovibor/                           *
* libpe is a library for obtaining PE (x86) and PE+ (x64) files' inner structure.       *
* Official git repository: https://github.com/jovibor/libpe                             *
* This software is available under the "MIT License".                                   *
****************************************************************************************/
#pragma once
#include <Windows.h>
#include <WinTrust.h> //WIN_CERTIFICATE struct.
#include <memory>
#include <span>
#include <string>
#include <vector>

#ifndef __cpp_lib_span
static_assert(false, "C++20 compiler is required for libpe, MSVS 16.11.14 or higher.");
#endif

namespace libpe
{
	//Rich.
	struct PERICHHDR {
		DWORD dwOffset; //File's raw offset of this entry.
		WORD  wId;      //Entry Id.
		WORD  wVersion; //Entry version.
		DWORD dwCount;  //Amount of occurrences.
	};
	using PERICHHDR_VEC = std::vector<PERICHHDR>;

	//NT header.
	struct PENTHDR {
		DWORD dwOffset;   //File's raw offset of this header.
		union UNPENTHDR { //Union of either x86 or x64 NT header.
			IMAGE_NT_HEADERS32 stNTHdr32; //x86 Header.
			IMAGE_NT_HEADERS64 stNTHdr64; //x64 Header.
		} unHdr;
	};

	//Data directories.
	struct PEDATADIR {
		IMAGE_DATA_DIRECTORY stDataDir;  //Standard header.
		std::string          strSection; //Name of the section this directory resides in (points to).
	};
	using PEDATADIR_VEC = std::vector<PEDATADIR>;

	//Sections headers.
	//For more info check:
	//docs.microsoft.com/en-us/windows/desktop/api/winnt/ns-winnt-_image_section_header#members
	//«An 8-byte, null-padded UTF-8 string. For longer names, this member contains a forward slash (/) 
	//followed by an ASCII representation of a decimal number that is an offset into the string table.»
	struct PESECHDR {
		DWORD                dwOffset;   //File's raw offset of this section header descriptor.
		IMAGE_SECTION_HEADER stSecHdr;   //Standard section header.
		std::string          strSecName; //Section full name.
	};
	using PESECHDR_VEC = std::vector<PESECHDR>;

	//Export table.
	struct PEEXPORTFUNC {
		DWORD       dwRVA;            //Function RVA.
		DWORD       dwOrdinal;        //Function ordinal.
		std::string strFuncName;      //Function name.
		std::string strForwarderName; //Function forwarder name.
	};
	struct PEEXPORT {
		DWORD                     dwOffset;      //File's raw offset of the Export header descriptor.
		IMAGE_EXPORT_DIRECTORY    stExportDesc;  //Standard export header descriptor.
		std::string               strModuleName; //Actual module name.
		std::vector<PEEXPORTFUNC> vecFuncs;      //Array of the exported functions struct.	
	};

	//Import table:
	struct PEIMPORTFUNC {
		union UNPEIMPORTTHUNK {
			IMAGE_THUNK_DATA32 stThunk32; //x86 standard thunk.
			IMAGE_THUNK_DATA64 stThunk64; //x64 standard thunk.
		} unThunk;
		IMAGE_IMPORT_BY_NAME stImpByName; //Standard IMAGE_IMPORT_BY_NAME struct
		std::string          strFuncName; //Function name.
	};
	struct PEIMPORT {
		DWORD                     dwOffset;      //File's raw offset of this Import descriptor.
		IMAGE_IMPORT_DESCRIPTOR   stImportDesc;  //Standard Import descriptor.
		std::string               strModuleName; //Imported module name.
		std::vector<PEIMPORTFUNC> vecImportFunc; //Array of imported functions.
	};
	using PEIMPORT_VEC = std::vector<PEIMPORT>;

	/**************************************Resources by Levels*******************************************
	* There are 3 levels of resources: 1. Type 2. Name 3. Language.										*
	* https://docs.microsoft.com/en-us/windows/desktop/Debug/pe-format#the-rsrc-section					*
	* Highest (root) resource structure is PERESROOT. It's a struct	that includes:                      *
	* an IMAGE_RESOURCE_DIRECTORY of root resource directory itself and vector<PERESROOTDATA>,          *
	* that contains structs of all IMAGE_RESOURCE_DIRECTORY_ENTRY structures of the root resource       *
	* directory. It also includes: wstring(Resource name), IMAGE_RESOURCE_DATA_ENTRY,                   *
	* vector<byte> (RAW resource data), and PERESLVL2 that is a struct of the next, second, resource    *
	* level, that replicates struct of root resource level.	PERESLVL2 includes IMAGE_RESOURCE_DIRECTORY *
	* of the second resource level, and vector<PERESLVL2DATA> that includes PERESLVL3 that is a struct  *
	* of the last, third, level of resources. Like previous two, this last level's struct consist of    *
	* IMAGE_RESOURCE_DIRECTORY and vector<PERESLVL3DATA>, that is again a vector of structs of all      *
	* IMAGE_RESOURCE_DIRECTORY_ENTRY of the last, third, level of resources.                            *
	****************************************************************************************************/

	//Level 3 (the lowest) Resources.
	struct PERESLVL3DATA {
		IMAGE_RESOURCE_DIRECTORY_ENTRY stResDirEntry;  //Level 3 standard IMAGE_RESOURCE_DIRECTORY_ENTRY struct.
		std::wstring                   wstrResName;    //Level 3 resource name.
		IMAGE_RESOURCE_DATA_ENTRY      stResDataEntry; //Level 3 standard IMAGE_RESOURCE_DATA_ENTRY struct.
		std::vector<std::byte>         vecRawResData;  //Level 3 resource raw data.
	};
	struct PERESLVL3 {
		DWORD                      dwOffset;   //File's raw offset of this level 3 IMAGE_RESOURCE_DIRECTORY descriptor.
		IMAGE_RESOURCE_DIRECTORY   stResDir;   //Level 3 standard IMAGE_RESOURCE_DIRECTORY header.
		std::vector<PERESLVL3DATA> vecResData; //Array of level 3 resource entries.
	};

	//Level 2 Resources — Includes LVL3 Resourses.
	struct PERESLVL2DATA {
		IMAGE_RESOURCE_DIRECTORY_ENTRY stResDirEntry;  //Level 2 standard IMAGE_RESOURCE_DIRECTORY_ENTRY struct.
		std::wstring                   wstrResName;	   //Level 2 resource name.
		IMAGE_RESOURCE_DATA_ENTRY      stResDataEntry; //Level 2 standard IMAGE_RESOURCE_DATA_ENTRY struct.
		std::vector<std::byte>         vecRawResData;  //Level 2 resource raw data.
		PERESLVL3                      stResLvL3;      //Level 3 resource struct.
	};
	struct PERESLVL2 {
		DWORD                      dwOffset;   //File's raw offset of this level 2 IMAGE_RESOURCE_DIRECTORY descriptor.
		IMAGE_RESOURCE_DIRECTORY   stResDir;   //Level 2 standard IMAGE_RESOURCE_DIRECTORY header.
		std::vector<PERESLVL2DATA> vecResData; //Array of level 2 resource entries.
	};

	//Level 1 (Root) Resources — Includes LVL2 Resources.
	struct PERESROOTDATA {
		IMAGE_RESOURCE_DIRECTORY_ENTRY stResDirEntry;  //Level root standard IMAGE_RESOURCE_DIRECTORY_ENTRY struct.
		std::wstring                   wstrResName;	   //Level root resource name.
		IMAGE_RESOURCE_DATA_ENTRY      stResDataEntry; //Level root standard IMAGE_RESOURCE_DATA_ENTRY struct.
		std::vector<std::byte>         vecRawResData;  //Level root resource raw data.
		PERESLVL2                      stResLvL2;      //Level 2 resource struct.
	};
	struct PERESROOT {
		DWORD                      dwOffset;   //File's raw offset of this level 1 IMAGE_RESOURCE_DIRECTORY descriptor.
		IMAGE_RESOURCE_DIRECTORY   stResDir;   //Level 1 standard IMAGE_RESOURCE_DIRECTORY header.
		std::vector<PERESROOTDATA> vecResData; //Array of level 1 resource entries.
	};

	//Flattened resources.
	struct PERESFLAT {
		std::wstring_view    wstrTypeName { }; //Type name.
		std::wstring_view    wstrResName { };  //Resource name.
		std::wstring_view    wstrLangName { }; //Lang name.
		std::span<std::byte> spnData { };      //Resource data.
		WORD                 wTypeID { };      //Type ID, e.g. RT_CURSOR, RT_BITMAP, etc...
		WORD                 wResID { };       //Resource ID.
		WORD                 wLangID { };      //Lang ID.
	};
	using PERESFLAT_VEC = std::vector< PERESFLAT>;
	/*********************************Resources End*****************************************/

	//Exception table.
	struct PEEXCEPTION {
		DWORD                         dwOffset;           //File's raw offset of this exception's descriptor.
		_IMAGE_RUNTIME_FUNCTION_ENTRY stRuntimeFuncEntry; //Standard _IMAGE_RUNTIME_FUNCTION_ENTRY header.
	};
	using PEEXCEPTION_VEC = std::vector<PEEXCEPTION>;

	//Security table.
	struct PESECURITY {
		DWORD           dwOffset;  //File's raw offset of this security descriptor.
		WIN_CERTIFICATE stWinSert; //Standard WIN_CERTIFICATE header.
	};
	using PESECURITY_VEC = std::vector<PESECURITY>;

	//Relocation table.
	struct PERELOCDATA {
		DWORD dwOffset;     //File's raw offset of this Relocation data descriptor.
		WORD  wRelocType;   //Relocation type.
		WORD  wRelocOffset; //Relocation offset (Offset the relocation must be applied to.)
	};
	struct PERELOC {
		DWORD                    dwOffset;     //File's raw offset of this Relocation descriptor.
		IMAGE_BASE_RELOCATION    stBaseReloc;  //Standard IMAGE_BASE_RELOCATION header.
		std::vector<PERELOCDATA> vecRelocData; //Array of the Relocation data struct.
	};
	using PERELOC_VEC = std::vector<PERELOC>;

	//Debug table.
	struct PEDEBUGDBGHDR {
		//dwHdr[6] is an array of the first six DWORDs of IMAGE_DEBUG_DIRECTORY::PointerToRawData data (Debug info header).
		//Their meaning varies depending on dwHdr[0] (Signature) value.
		//If dwHdr[0] == 0x53445352 (Ascii "RSDS") it's PDB 7.0 file:
		// Then dwHdr[1]-dwHdr[4] is GUID (*((GUID*)&dwHdr[1])). dwHdr[5] is Counter/Age.
		//If dwHdr[0] == 0x3031424E (Ascii "NB10") it's PDB 2.0 file:
		// Then dwHdr[1] is Offset. dwHdr[2] is Time/Signature. dwHdr[3] is Counter/Age.
		DWORD       dwHdr[6];
		std::string strPDBName; //PDB file name/path.
	};
	struct PEDEBUG {
		DWORD                 dwOffset;       //File's raw offset of this Debug descriptor.
		IMAGE_DEBUG_DIRECTORY stDebugDir;     //Standard IMAGE_DEBUG_DIRECTORY header.
		PEDEBUGDBGHDR         stDebugHdrInfo; //Debug info header.
	};
	using PEDEBUG_VEC = std::vector<PEDEBUG>;

	//TLS table.
	struct PETLS {
		DWORD              dwOffset;          //File's raw offset of the TLS header descriptor.
		union UNPETLS {
			IMAGE_TLS_DIRECTORY32 stTLSDir32; //x86 standard TLS header.
			IMAGE_TLS_DIRECTORY64 stTLSDir64; //x64 TLS header.
		} unTLS;
		std::vector<DWORD> vecTLSCallbacks;   //Array of the TLS callbacks.
	};

	//LoadConfigDirectory.
	struct PELOADCONFIG {
		DWORD dwOffset;                            //File's raw offset of the LCD descriptor.
		union UNPELOADCONFIG {
			IMAGE_LOAD_CONFIG_DIRECTORY32 stLCD32; //x86 LCD descriptor.
			IMAGE_LOAD_CONFIG_DIRECTORY64 stLCD64; //x64 LCD descriptor.
		} unLCD;
	};

	//Bound import table.
	struct PEBOUNDFORWARDER {
		DWORD                     dwOffset;              //File's raw offset of this Bound Forwarder descriptor.
		IMAGE_BOUND_FORWARDER_REF stBoundForwarder;      //Standard IMAGE_BOUND_FORWARDER_REF struct.
		std::string               strBoundForwarderName; //Bound forwarder name.
	};
	struct PEBOUNDIMPORT {
		DWORD                         dwOffset;          //File's raw offset of this Bound Import descriptor.
		IMAGE_BOUND_IMPORT_DESCRIPTOR stBoundImpDesc;    //Standard IMAGE_BOUND_IMPORT_DESCRIPTOR struct.
		std::string                   strBoundName;      //Bound Import name.
		std::vector<PEBOUNDFORWARDER> vecBoundForwarder; //Array of the Bound Forwarder structs.
	};
	using PEBOUNDIMPORT_VEC = std::vector<PEBOUNDIMPORT>;

	//Delay import table.
	struct PEDELAYIMPORTFUNC {
		union UNPEDELAYIMPORTTHUNK
		{
			struct x32 {
				IMAGE_THUNK_DATA32 stImportAddressTable;      //x86 Import Address Table struct.
				IMAGE_THUNK_DATA32 stImportNameTable;         //x86 Import Name Table struct.
				IMAGE_THUNK_DATA32 stBoundImportAddressTable; //x86 Bound Import Address Table struct.
				IMAGE_THUNK_DATA32 stUnloadInformationTable;  //x86 Unload Information Table struct.
			} st32;
			struct x64 {
				IMAGE_THUNK_DATA64 stImportAddressTable;      //x64 Import Address Table struct.
				IMAGE_THUNK_DATA64 stImportNameTable;         //x64 Import Name Table struct.
				IMAGE_THUNK_DATA64 stBoundImportAddressTable; //x64 Bound Import Address Table struct
				IMAGE_THUNK_DATA64 stUnloadInformationTable;  //x64 Unload Information Table struct.
			} st64;
		} unThunk;
		IMAGE_IMPORT_BY_NAME stImpByName; //Standard IMAGE_IMPORT_BY_NAME struct.
		std::string          strFuncName; //Function name.
	};
	struct PEDELAYIMPORT {
		DWORD                          dwOffset;        //File's raw offset of this Delay Import descriptor.
		IMAGE_DELAYLOAD_DESCRIPTOR     stDelayImpDesc;  //Standard IMAGE_DELAYLOAD_DESCRIPTOR struct.
		std::string                    strModuleName;   //Import module name.
		std::vector<PEDELAYIMPORTFUNC> vecDelayImpFunc; //Array of the Delay Import module functions.
	};
	using PEDELAYIMPORT_VEC = std::vector<PEDELAYIMPORT>;

	//COM descriptor table.
	struct PECOMDESCRIPTOR {
		DWORD              dwOffset; //File's raw offset of the IMAGE_COR20_HEADER descriptor.
		IMAGE_COR20_HEADER stCorHdr; //Standard IMAGE_COR20_HEADER struct.
	};

	//File information struct.
	struct PEFILEINFO {
		bool fIsx86 : 1 {};
		bool fIsx64 : 1 {};
		bool fHasDosHdr : 1 {};
		bool fHasRichHdr : 1 {};
		bool fHasNTHdr : 1 {};
		bool fHasDataDirs : 1 {};
		bool fHasSections : 1 {};
		bool fHasExport : 1 {};
		bool fHasImport : 1 {};
		bool fHasResource : 1 {};
		bool fHasException : 1 {};
		bool fHasSecurity : 1 {};
		bool fHasReloc : 1 {};
		bool fHasDebug : 1 {};
		bool fHasArchitect : 1 {};
		bool fHasGlobalPtr : 1 {};
		bool fHasTLS : 1 {};
		bool fHasLoadCFG : 1 {};
		bool fHasBoundImp : 1 {};
		bool fHasIAT : 1 {};
		bool fHasDelayImp : 1 {};
		bool fHasCOMDescr : 1 {};
	};

	//Pure abstract base class Ilibpe.
	class Ilibpe
	{
	public:
		virtual auto LoadPe(LPCWSTR pwszFile)->int = 0;
		[[nodiscard]] virtual auto GetFileInfo()const->PEFILEINFO = 0;
		[[nodiscard]] virtual auto GetOffsetFromRVA(ULONGLONG ullRVA)const->DWORD = 0;
		[[nodiscard]] virtual auto GetOffsetFromVA(ULONGLONG ullVA)const->DWORD = 0;
		[[nodiscard]] virtual auto GetMSDOSHeader()->IMAGE_DOS_HEADER* = 0;
		[[nodiscard]] virtual auto GetRichHeader()->PERICHHDR_VEC* = 0;
		[[nodiscard]] virtual auto GetNTHeader()->PENTHDR* = 0;
		[[nodiscard]] virtual auto GetDataDirs()->PEDATADIR_VEC* = 0;
		[[nodiscard]] virtual auto GetSecHeaders()->PESECHDR_VEC* = 0;
		[[nodiscard]] virtual auto GetExport()->PEEXPORT* = 0;
		[[nodiscard]] virtual auto GetImport()->PEIMPORT_VEC* = 0;
		[[nodiscard]] virtual auto GetResources()->PERESROOT* = 0;
		[[nodiscard]] virtual auto FlatResources(PERESROOT& stResRoot)const->PERESFLAT_VEC = 0;
		[[nodiscard]] virtual auto GetExceptions()->PEEXCEPTION_VEC* = 0;
		[[nodiscard]] virtual auto GetSecurity()->PESECURITY_VEC* = 0;
		[[nodiscard]] virtual auto GetRelocations()->PERELOC_VEC* = 0;
		[[nodiscard]] virtual auto GetDebug()->PEDEBUG_VEC* = 0;
		[[nodiscard]] virtual auto GetTLS()->PETLS* = 0;
		[[nodiscard]] virtual auto GetLoadConfig()->PELOADCONFIG* = 0;
		[[nodiscard]] virtual auto GetBoundImport()->PEBOUNDIMPORT_VEC* = 0;
		[[nodiscard]] virtual auto GetDelayImport()->PEDELAYIMPORT_VEC* = 0;
		[[nodiscard]] virtual auto GetCOMDescriptor()->PECOMDESCRIPTOR* = 0;
		virtual void Clear() = 0; //Clear all internal structs.
		virtual void Destroy() = 0;
	};

	//Return codes.
	constexpr auto PEOK = 0;
	constexpr auto ERR_FILE_OPEN = 0x01;
	constexpr auto ERR_FILE_SIZESMALL = 0x02;
	constexpr auto ERR_FILE_MAPPING = 0x03;
	constexpr auto ERR_FILE_NODOSHDR = 0x04;

#ifdef LIBPE_SHARED_DLL
#ifdef LIBPE_SHARED_DLL_EXPORT
#define ILIBPEAPI __declspec(dllexport)
#else
#define ILIBPEAPI __declspec(dllimport)
	/********************************************************
	* Platform and configuration specific .lib name macros.	*
	********************************************************/
#ifdef _WIN64
#ifdef _DEBUG
#define LIBPE_LIB_NAME(x) x"64d.lib"
#else
#define LIBPE_LIB_NAME(x) x"64.lib"
#endif
#else
#ifdef _DEBUG
#define LIBPE_LIB_NAME(x) x"d.lib"
#else
#define LIBPE_LIB_NAME(x) x".lib"
#endif
#endif
	/********************************************************
	* End of .lib name macros.                              *
	********************************************************/
#pragma comment(lib, LIBPE_LIB_NAME("libpe"))
#endif
#else
#define	ILIBPEAPI
#endif

	/********************************************************************************************
	* Factory function Createlibpe, returns IlibpePtr - unique_ptr with custom deleter.         *
	* If you, for some reason, need a raw pointer, you can directly call CreateRawlibpe         *
	* function, which returns Ilibpe interface pointer, but in this case you will need to       *
	* call Ilibpe::Destroy method afterwards manually - to delete Ilibpe object.                *
	********************************************************************************************/
	extern "C" ILIBPEAPI Ilibpe * __cdecl CreateRawlibpe();
	using IlibpePtr = std::unique_ptr < Ilibpe, decltype([](Ilibpe* p) { p->Destroy(); }) > ;

	inline IlibpePtr Createlibpe() {
		return IlibpePtr { CreateRawlibpe() };
	};

	/********************************************
	* LIBPEINFO: service info structure.        *
	********************************************/
	struct LIBPEINFO
	{
		const wchar_t* pwszVersion { };        //wchar_t string Version.
		union {
			unsigned long long ullVersion { }; //long long number Version.
			struct {
				short wMajor;
				short wMinor;
				short wMaintenance;
				short wRevision;
			} stVersion;
		};
	};

	extern "C" ILIBPEAPI LIBPEINFO __cdecl GetLibInfo();
}