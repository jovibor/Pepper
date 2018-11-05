constexpr auto HEXCTRL_SECURITY_SERTIFICATEID = 0x0001;
constexpr auto LIST_ARCHITECTURE = 0x0002;
constexpr auto LIST_BOUNDIMPORT = 0x0003;
constexpr auto LIST_COMDESCRIPTOR = 0x0004;
constexpr auto LIST_DATADIRECTORIES = 0x0005;
constexpr auto LIST_DEBUG = 0x0006;
constexpr auto LIST_DELAYIMPORT = 0x0007;
constexpr auto LIST_DELAYIMPORT_FUNCS = 0x0008;
constexpr auto LIST_DOSHEADER = 0x0009;
constexpr auto LIST_RICHHEADER = 0x000A;
constexpr auto LIST_EXCEPTION = 0x000B;
constexpr auto LIST_EXPORT = 0x000C;
constexpr auto LIST_EXPORT_FUNCS = 0x000D;
constexpr auto LIST_FILEHEADER = 0x000E;
constexpr auto PE_FILE_SUMMARY = 0x000F;
constexpr auto LIST_GLOBALPTR = 0x0010;
constexpr auto LIST_IAT = 0x0011;
constexpr auto LIST_IMPORT = 0x0012;
constexpr auto LIST_IMPORT_FUNCS = 0x0013;
constexpr auto LIST_LOADCONFIG = 0x0014;
constexpr auto LIST_NTHEADER = 0x0015;
constexpr auto LIST_OPTIONALHEADER = 0x0016;
constexpr auto LIST_RELOCATIONS = 0x0017;
constexpr auto LIST_RELOCATIONS_TOPCHANGEDMSG = 0x0018;
constexpr auto LIST_RELOCATIONS_TYPE = 0x0019;
constexpr auto LIST_RESOURCE = 0x001A;
constexpr auto LIST_SECHEADERS = 0x001B;
constexpr auto LIST_SECURITY = 0x001C;
constexpr auto LIST_TLS = 0x001D;
constexpr auto TREE_LEFT_MAIN = 0x001E;
constexpr auto TREE_RESOURCE_TOP = 0x001F;
constexpr auto TREE_RESOURCE_BOTTOM = 0x0020;

const std::map<WORD, std::wstring> g_mapResType {
	{ 1, L"CURSOR" },
{ 2, L"BITMAP" },
{ 3, L"ICON" },
{ 4, L"MENU" },
{ 5, L"DIALOG" },
{ 6, L"STRING" },
{ 7, L"FONTDIR" },
{ 8, L"FONT" },
{ 9, L"ACCELERATOR" },
{ 10, L"RCDATA" },
{ 11, L"MESSAGETABLE" },
{ 12, L"GROUP_CURSOR" },
{ 14, L"GROUP_ICON" },
{ 16, L"VERSION" },
{ 17, L"DLGINCLUDE" },
{ 19, L"PLUGPLAY" },
{ 20, L"VXD" },
{ 21, L"ANICURSOR" },
{ 22, L"ANIICON" },
{ 23, L"HTML" },
{ 24, L"MANIFEST" }
};