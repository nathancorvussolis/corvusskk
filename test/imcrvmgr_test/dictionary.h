
#pragma once

// okuri block
typedef struct _OKURI_BLOCK {
	WCHAR r;
	WCHAR d[6];
} OKURI_BLOCK;

#define OKURI_BLOCK_NUM 32
extern OKURI_BLOCK okuri_block[OKURI_BLOCK_NUM];

extern WCHAR mgrpipename[MAX_KRNLOBJNAME];		//名前付きパイプ

void _CreateIpcName();
BOOL _ConnectDic();
void _DisconnectDic();
BOOL _SearchDic(WCHAR command, CANDIDATES &candidates, const std::wstring &searchkey, const std::wstring &searchkey_org, const std::wstring &okurikey);
BOOL _AddUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate, const std::wstring &annotation, const std::wstring &okurikey);
BOOL _DelUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate);
BOOL _SaveUserDic();
BOOL _CommandDic(WCHAR command);

#define E_MAKESKKDIC_OK			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0)
//#define E_MAKESKKDIC_DOWNLOAD	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 1)
#define E_MAKESKKDIC_FILEIO		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 2)
#define E_MAKESKKDIC_ENCODING	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 3)
//#define E_MAKESKKDIC_UNGZIP		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 4)
//#define E_MAKESKKDIC_UNTAR		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 5)

HRESULT CheckMultiByteFile(LPCWSTR path, int encoding);
HRESULT CheckWideCharFile(LPCWSTR path);
