/****************************************************************************************************
* Copyright © 2018-2023 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
module;
#include "HexCtrl.h"
#include <afxdlgs.h>
#include <format>
#include <fstream>
#include <span>
#include <string>
#include <unordered_map>

export module Utility;
import libpe;

#define TO_WSTR_MAP(x) {x, L## #x}

export namespace Util
{
	constexpr auto PEPPER_VERSION_MAJOR = 1;
	constexpr auto PEPPER_VERSION_MINOR = 5;
	constexpr auto PEPPER_VERSION_PATCH = 3;

	[[nodiscard]] inline auto StrToWstr(std::string_view sv, UINT uCodePage = CP_UTF8) -> std::wstring
	{
		const auto iSize = MultiByteToWideChar(uCodePage, 0, sv.data(), static_cast<int>(sv.size()), nullptr, 0);
		std::wstring wstr(iSize, 0);
		MultiByteToWideChar(uCodePage, 0, sv.data(), static_cast<int>(sv.size()), wstr.data(), iSize);
		return wstr;
	}

	enum class EResType : DWORD {
		NO_RESOURCE = 0,
		RTYPE_CURSOR = 1, RTYPE_BITMAP = 2,
		RTYPE_ICON = 3, RTYPE_MENU = 4,
		RTYPE_DIALOG = 5, RTYPE_STRING = 6,
		RTYPE_FONTDIR = 7, RTYPE_FONT = 8,
		RTYPE_ACCELERATOR = 9, RTYPE_RCDATA = 10,
		RTYPE_MESSAGETABLE = 11, RTYPE_GROUP_CURSOR = 12,
		RTYPE_GROUP_ICON = 14, RTYPE_VERSION = 16,
		RTYPE_DLGINCLUDE = 17, RTYPE_PLUGPLAY = 19,
		RTYPE_VXD = 20, RTYPE_ANICURSOR = 21,
		RTYPE_ANIICON = 22, RTYPE_HTML = 23, RTYPE_RIBBON_XML = 28,
		RTYPE_DLGINIT = 240, RTYPE_TOOLBAR = 241, RTYPE_PNG = 0x1F001,
		RTYPE_UNSUPPORTED = 0x1FFF0, RES_LOAD_ERROR = 0x1FFFF
	};

	inline bool SaveResToFile(EResType eResType, const wchar_t* pwszPath, std::span<const std::byte> spnData)
	{
		if (spnData.empty())
			return false;

		std::ofstream ofs(pwszPath, std::ios::binary);
		if (!ofs)
			return false;

		using enum EResType;
		switch (eResType) {
		case RTYPE_CURSOR:
		case RTYPE_ICON: {
		#pragma pack(push, 2)
			struct ICONDIRENTRY {
				BYTE  bWidth { };
				BYTE  bHeight { };
				BYTE  bColorCount { };
				BYTE  bReserved { };
				WORD  wPlanes { };
				WORD  wBitCount { };
				DWORD dwBytesInRes { };
				DWORD dwImageOffset { };
			};

			struct ICONDIR {
				WORD         idReserved { };
				WORD         idType { };
				WORD         idCount { };
				ICONDIRENTRY idEntries[1];
			};
		#pragma pack(pop)

			const auto fIcon = eResType == RTYPE_ICON;
			const auto dwSize = static_cast<DWORD>(spnData.size());
			const auto hIcon = CreateIconFromResourceEx(const_cast<PBYTE>(reinterpret_cast<const BYTE*>(spnData.data())),
				dwSize, fIcon ? TRUE : FALSE, 0x00030000UL, 0, 0, LR_DEFAULTCOLOR);
			if (!hIcon)
				return false;

			ICONINFOEX iconInfo { .cbSize = sizeof(ICONINFOEX), .fIcon = fIcon };
			if (GetIconInfoExW(hIcon, &iconInfo) == FALSE)
				return false;

			const auto fBWIcon { iconInfo.hbmColor == nullptr };
			BITMAP bmp;
			if (GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &bmp) == 0)
				return false;

			constexpr auto dwCurDataOffset = 0x4UL; //Offset for cursor's actual data beginning.
			const auto dwSizeToWrite = dwSize - (fIcon ? 0 : dwCurDataOffset);

			ICONDIRENTRY iconDirEntry { };
			iconDirEntry.bWidth = static_cast<BYTE>(bmp.bmWidth);
			iconDirEntry.bHeight = static_cast<BYTE>(std::abs(bmp.bmHeight) / (fBWIcon ? 2 : 1));
			iconDirEntry.bColorCount = (bmp.bmBitsPixel < 8) ? (1 << (bmp.bmBitsPixel * bmp.bmPlanes)) : 0;
			iconDirEntry.wPlanes = fIcon ? bmp.bmPlanes : static_cast<WORD>(iconInfo.xHotspot);
			iconDirEntry.wBitCount = fIcon ? bmp.bmBitsPixel : static_cast<WORD>(iconInfo.yHotspot);
			iconDirEntry.dwBytesInRes = dwSizeToWrite;
			iconDirEntry.dwImageOffset = sizeof(ICONDIR);

			ICONDIR iconDir { .idCount = 1 };
			iconDir.idType = fIcon ? 1 : 2;
			iconDir.idEntries[0] = iconDirEntry;

			const auto pData = reinterpret_cast<const char*>(fIcon ? spnData.data() : spnData.data() + dwCurDataOffset);

			ofs.write(reinterpret_cast<const char*>(&iconDir), sizeof(ICONDIR));
			ofs.write(pData, dwSizeToWrite);

			DeleteObject(iconInfo.hbmColor);
			DeleteObject(iconInfo.hbmMask);
			DestroyIcon(hIcon);
			return true;
		}
		case RTYPE_BITMAP: {
			const auto dwSizeFile = static_cast<DWORD>(sizeof(BITMAPFILEHEADER) + spnData.size());
			const auto pBMPInfo = reinterpret_cast<const BITMAPINFO*>(spnData.data());
			const auto pBMPInfoHdr = &pBMPInfo->bmiHeader;
			const BITMAPFILEHEADER bmpFHdr { .bfType = 0x4D42/*"BM"*/, .bfSize = dwSizeFile,
				.bfOffBits = pBMPInfoHdr->biSizeImage == 0 ? static_cast<DWORD>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) :
				dwSizeFile - pBMPInfoHdr->biSizeImage };

			ofs.write(reinterpret_cast<const char*>(&bmpFHdr), sizeof(BITMAPFILEHEADER));
			ofs.write(reinterpret_cast<const char*>(spnData.data()), spnData.size());
			return true;
		}
		case RTYPE_PNG: {
			ofs.write(reinterpret_cast<const char*>(spnData.data()), spnData.size());
			return true;
		}
		default:
			return false;
		}
	}

	inline bool ExtractResToFile(EResType eResType, std::span<const std::byte> spnData)
	{
		using enum EResType;
		std::wstring_view wsvName;
		switch (eResType) {
		case RTYPE_CURSOR:
			wsvName = L"Cursor files (*.cur)|*.cur|All files (*.*)|*.*||";
			break;
		case RTYPE_BITMAP:
		case RTYPE_TOOLBAR:
			wsvName = L"Bitmap files (*.bmp)|*.bmp|All files (*.*)|*.*||";
			break;
		case RTYPE_ICON:
			wsvName = L"Icon files (*.ico)|*.ico|All files (*.*)|*.*||";
			break;
		case RTYPE_PNG:
			wsvName = L"PNG files (*.png)|*.png|All files (*.*)|*.*||";
			break;
		default:
			return false;
		}

		CFileDialog fd(FALSE, nullptr, nullptr, OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_PATHMUSTEXIST,
			wsvName.data());
		if (fd.DoModal() != IDOK)
			return false;

		if (const auto wstrPath = fd.GetPathName();	!SaveResToFile(eResType, wstrPath.GetString(), spnData)) {
			MessageBoxW(nullptr, L"Error saving the file. Check if it's writable.", L"Error", MB_ICONERROR);
			return false;
		}

		return true;
	}

	inline void ExtractAllResToFile(std::optional<libpe::PERESROOT>& pRes, EResType eResType, std::wstring_view wsvPrefix)
	{
		if (!pRes)
			return;

		CFolderPickerDialog fd;
		if (fd.DoModal() != IDOK)
			return;

		const auto cstrFolderPath = fd.GetPathName(); //Folder name.
		std::wstring wstrPathWithPrefix = cstrFolderPath.GetString() + std::wstring { L"\\" };
		if (!wsvPrefix.empty()) {
			wstrPathWithPrefix += std::wstring { wsvPrefix } + L"_";
		}

		auto dwSavedFiles { 0UL };
		bool fAllSaveOK { true };
		for (const auto & ref : libpe::FlatResources(*pRes)) {
			using enum EResType;
			std::wstring_view wsvExt;
			switch (eResType) {
			case RTYPE_CURSOR:
				if (ref.wTypeID == 1) { //RT_CURSOR
					wsvExt = L".cur";
				}
				break;
			case RTYPE_BITMAP:
				if (ref.wTypeID == 2) { //RT_BITMAP
					wsvExt = L".bmp";
				}
				break;
			case RTYPE_ICON:
				if (ref.wTypeID == 3) { //RT_ICON
					wsvExt = L".ico";
				}
				break;
			case RTYPE_PNG:
				if (ref.wsvTypeStr == L"PNG") { //PNG
					wsvExt = L".png";
				}
				break;
			default:
				break;
			}

			if (!wsvExt.empty()) {
				std::wstring wstrPathFile = wstrPathWithPrefix;
				if (!ref.wsvNameStr.empty()) {
					wstrPathFile += ref.wsvNameStr;
					wstrPathFile += L"_";
				}
				else {
					wstrPathFile += std::format(L"RESID_{}_", ref.wNameID);
				}

				if (!ref.wsvLangStr.empty()) {
					wstrPathFile += ref.wsvLangStr;
				}
				else {
					wstrPathFile += std::format(L"LANGID_{}", ref.wLangID);
				}
				wstrPathFile += wsvExt;

				if (SaveResToFile(eResType, wstrPathFile.data(), ref.spnData)) {
					++dwSavedFiles;
				}
				else {
					fAllSaveOK = false;
				}
			}
		}

		if (fAllSaveOK) {
			MessageBoxW(nullptr, std::format(L"All {} files were saved successfully.", dwSavedFiles).data(), L"Success", MB_ICONINFORMATION);
		}
		else {
			MessageBoxW(nullptr, std::format(L"Some issues occured during the save process.\r\nOnly {} files were saved successfully.", dwSavedFiles).data(),
				L"Error", MB_ICONERROR);
		}
	}

	//Errors, that might come from libpe.
	inline const std::unordered_map<DWORD, std::wstring> g_mapLibpeErrors {
		TO_WSTR_MAP(libpe::PEOK),
		TO_WSTR_MAP(libpe::ERR_FILE_OPEN),
		TO_WSTR_MAP(libpe::ERR_FILE_MAPPING),
		TO_WSTR_MAP(libpe::ERR_FILE_SIZESMALL),
		TO_WSTR_MAP(libpe::ERR_FILE_NODOSHDR)
	};

	struct PEFILEINFO {
		libpe::EFileType eFileType { };
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
		bool fHasTLS : 1 {};
		bool fHasLoadCFG : 1 {};
		bool fHasBoundImp : 1 {};
		bool fHasIAT : 1 {};
		bool fHasDelayImp : 1 {};
		bool fHasCOMDescr : 1 {};
	};

	//Helper struct for PE structs' fields offsets and sizes.
	struct SPEREFLECTION {
		DWORD dwSize;          //Struct's field size.
		DWORD dwOffset;        //Field offset.
		std::wstring wstrName; //Field name.
	};
	using map_hdr = std::unordered_map<DWORD, SPEREFLECTION>;

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

	inline const std::unordered_map<DWORD, std::wstring_view> g_mapDataDirs {
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

	//All HexCtrl dialogs' IDs for hiding/showing in Views, when tab is deactivated/activated.
	inline const std::vector<HEXCTRL::EHexWnd> g_vecHexDlgs {
		HEXCTRL::EHexWnd::DLG_BKMMANAGER, HEXCTRL::EHexWnd::DLG_DATAINTERP, HEXCTRL::EHexWnd::DLG_MODIFY,
		HEXCTRL::EHexWnd::DLG_SEARCH, HEXCTRL::EHexWnd::DLG_ENCODING,
		HEXCTRL::EHexWnd::DLG_GOTO, HEXCTRL::EHexWnd::DLG_TEMPLMGR };


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

	constexpr auto MSG_MDITAB_ACTIVATE = 0x0501;
	constexpr auto MSG_MDITAB_DISACTIVATE = 0x0502;

	constexpr auto IDM_LIST_GOTODESCOFFSET = 0x8001;
	constexpr auto IDM_LIST_GOTODATAOFFSET = 0x8002;

	constexpr auto IDM_EXTRACT_RES = 0xF0U;
	constexpr auto IDM_EXTRACT_ALLRES = 0xF1U;
	/********************************************************
	* End of IDC.											*
	********************************************************/

	//Color of the list's "Offset" column
	constexpr COLORREF g_clrOffset = RGB(150, 150, 150);
};