
#include "configxml.h"
#include "utf8.h"
#include "imcrvmgr.h"

LPCWSTR TextServiceDesc = TEXTSERVICE_DESC;
LPCWSTR DictionaryManagerClass = TEXTSERVICE_NAME L"DictionaryManager";

// ファイルパス
WCHAR pathconfigxml[MAX_PATH];	//設定
WCHAR pathuserdic[MAX_PATH];	//ユーザー辞書
WCHAR pathuserbak[MAX_PATH];	//ユーザー辞書バックアッププレフィックス
WCHAR pathskkdic[MAX_PATH];		//取込SKK辞書
WCHAR pathinitlua[MAX_PATH];	//init.lua

WCHAR krnlobjsddl[MAX_KRNLOBJNAME];		//SDDL
WCHAR mgrpipename[MAX_KRNLOBJNAME];		//名前付きパイプ
WCHAR mgrmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// 辞書サーバー設定
BOOL serv = FALSE;		//SKK辞書サーバーを使用する
WCHAR host[MAX_SKKSERVER_HOST] = {L'\0'};	//ホスト
WCHAR port[MAX_SKKSERVER_PORT] = {L'\0'};	//ポート
DWORD encoding = 0;		//エンコーディング
DWORD timeout = 1000;	//タイムアウト

BOOL precedeokuri = FALSE;	//送り仮名が一致した候補を優先する

const luaL_Reg luaFuncs[] =
{
	{u8"search_skk_dictionary", lua_search_skk_dictionary},
	{u8"search_user_dictionary", lua_search_user_dictionary},
	{u8"search_skk_server", lua_search_skk_server},
	{u8"search_skk_server_info", lua_search_skk_server_info},
	{u8"search_unicode", lua_search_unicode},
	{u8"search_jisx0213", lua_search_jisx0213},
	{u8"search_jisx0208", lua_search_jisx0208},
	{u8"search_character_code", lua_search_character_code},
	{u8"complement", lua_complement},
	{u8"add", lua_add},
	{u8"delete", lua_delete},
	{u8"save", lua_save},
	{nullptr, nullptr}
};

void CreateConfigPath()
{
	PWSTR appdatafolder = nullptr;

	ZeroMemory(pathconfigxml, sizeof(pathconfigxml));
	ZeroMemory(pathuserdic, sizeof(pathuserdic));
	ZeroMemory(pathuserbak, sizeof(pathuserbak));
	ZeroMemory(pathskkdic, sizeof(pathskkdic));
	ZeroMemory(pathinitlua, sizeof(pathinitlua));

	if(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DONT_VERIFY, nullptr, &appdatafolder) == S_OK)
	{
		WCHAR appdir[MAX_PATH];

		_snwprintf_s(appdir, _TRUNCATE, L"%s\\%s", appdatafolder, TextServiceDesc);

		CoTaskMemFree(appdatafolder);

		CreateDirectoryW(appdir, nullptr);
		SetCurrentDirectoryW(appdir);

		_snwprintf_s(pathconfigxml, _TRUNCATE, L"%s\\%s", appdir, fnconfigxml);
		_snwprintf_s(pathuserdic, _TRUNCATE, L"%s\\%s", appdir, fnuserdic);
		_snwprintf_s(pathuserbak, _TRUNCATE, L"%s\\%s", appdir, fnuserbak);
		_snwprintf_s(pathskkdic, _TRUNCATE, L"%s\\%s", appdir, fnskkdic);
		_snwprintf_s(pathinitlua, _TRUNCATE, L"%s\\%s", appdir, fninitlua);

		//for compatibility
		if(GetFileAttributesW(pathskkdic) == INVALID_FILE_ATTRIBUTES)
		{
			WCHAR skkdict[MAX_PATH];
			_snwprintf_s(skkdict, _TRUNCATE, L"%s\\%s", appdir, L"skkdict.dic");
			MoveFileW(skkdict, pathskkdic);
			_snwprintf_s(skkdict, _TRUNCATE, L"%s\\%s", appdir, L"skkdict.idx");
			DeleteFileW(skkdict);
		}
	}

	ZeroMemory(krnlobjsddl, sizeof(krnlobjsddl));
	ZeroMemory(mgrpipename, sizeof(mgrpipename));
	ZeroMemory(mgrmutexname, sizeof(mgrmutexname));

	LPWSTR pszUserSid = nullptr;

	if(GetUserSid(&pszUserSid))
	{
		// SDDL_ALL_APP_PACKAGES / SDDL_RESTRICTED_CODE / SDDL_LOCAL_SYSTEM / SDDL_BUILTIN_ADMINISTRATORS / User SID
		_snwprintf_s(krnlobjsddl, _TRUNCATE, L"D:%s(A;;GA;;;RC)(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;%s)",
			(IsWindowsVersion62OrLater() ? L"(A;;GA;;;AC)" : L""), pszUserSid);

		// (SDDL_MANDATORY_LABEL, SDDL_NO_WRITE_UP, SDDL_ML_LOW)
		wcsncat_s(krnlobjsddl, L"S:(ML;;NW;;;LW)", _TRUNCATE);

		LocalFree(pszUserSid);
	}

	LPWSTR pszUserUUID = nullptr;

	if(GetUserUUID(&pszUserUUID))
	{
		_snwprintf_s(mgrpipename, _TRUNCATE, L"%s%s", IMCRVMGRPIPE, pszUserUUID);
		_snwprintf_s(mgrmutexname, _TRUNCATE, L"%s%s", IMCRVMGRMUTEX, pszUserUUID);

		LocalFree(pszUserUUID);
	}
}

void LoadConfig()
{
	BOOL servtmp;
	WCHAR hosttmp[MAX_SKKSERVER_HOST];	//ホスト
	WCHAR porttmp[MAX_SKKSERVER_PORT];	//ポート
	DWORD encodingtmp;
	DWORD timeouttmp;
	std::wstring strxmlval;

	ReadValue(pathconfigxml, SectionServer, ValueServerServ, strxmlval);
	servtmp = _wtoi(strxmlval.c_str());
	if(servtmp != TRUE && servtmp != FALSE)
	{
		servtmp = FALSE;
	}

	ReadValue(pathconfigxml, SectionServer, ValueServerHost, strxmlval);
	wcsncpy_s(hosttmp, strxmlval.c_str(), _TRUNCATE);

	ReadValue(pathconfigxml, SectionServer, ValueServerPort, strxmlval);
	wcsncpy_s(porttmp, strxmlval.c_str(), _TRUNCATE);

	ReadValue(pathconfigxml, SectionServer, ValueServerEncoding, strxmlval);
	encodingtmp = _wtoi(strxmlval.c_str());
	if(encodingtmp != 1)
	{
		encodingtmp = 0;
	}

	ReadValue(pathconfigxml, SectionServer, ValueServerTimeOut, strxmlval);
	timeouttmp = _wtoi(strxmlval.c_str());
	if(timeouttmp > 60000)
	{
		timeouttmp = 1000;
	}

	//変更があったら接続し直す
	if(servtmp != serv || wcscmp(hosttmp, host) != 0 || wcscmp(porttmp, port) != 0 ||
		encodingtmp != encoding || timeouttmp != timeout)
	{
		serv = servtmp;
		wcsncpy_s(host, hosttmp, _TRUNCATE);
		wcsncpy_s(port, porttmp, _TRUNCATE);
		encoding = encodingtmp;
		timeout = timeouttmp;

		DisconnectSKKServer();

		if(serv)
		{
			ConnectSKKServer();
			GetSKKServerInfo(SKK_VER);
		}
	}

	ReadValue(pathconfigxml, SectionBehavior, ValuePrecedeOkuri, strxmlval);
	precedeokuri = _wtoi(strxmlval.c_str());
	if(precedeokuri != TRUE && precedeokuri != FALSE)
	{
		precedeokuri = FALSE;
	}
}

BOOL IsFileUpdated(LPCWSTR path, FILETIME *ft)
{
	BOOL ret = FALSE;
	HANDLE hFile;
	FILETIME ftn;

	if(path != nullptr && ft != nullptr)
	{
		hFile = CreateFileW(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			if(GetFileTime(hFile, nullptr, nullptr, &ftn))
			{
				if(((ULARGE_INTEGER *)ft)->QuadPart != ((ULARGE_INTEGER *)&ftn)->QuadPart)
				{
					*ft = ftn;
					ret = TRUE;
				}
			}
			CloseHandle(hFile);
		}
	}

	return ret;
}

void InitLua()
{
	CHAR version[64];

	lua = luaL_newstate();
	if(lua == nullptr)
	{
		return;
	}

	luaL_openlibs(lua);

	luaL_newlib(lua, luaFuncs);
	lua_setglobal(lua, u8"crvmgr");

	//skk-version
	_snprintf_s(version, _TRUNCATE, "%s", WCTOU8(TEXTSERVICE_NAME L" " TEXTSERVICE_VER));
	lua_pushstring(lua, version);
	lua_setglobal(lua, u8"SKK_VERSION");

	//%AppData%\CorvusSKK\init.lua
	if(luaL_dofile(lua, WCTOU8(pathinitlua)) == LUA_OK)
	{
		return;
	}

	ZeroMemory(pathinitlua, sizeof(pathinitlua));

	if(GetModuleFileNameW(nullptr, pathinitlua, _countof(pathinitlua)) != 0)
	{
		WCHAR *pdir = wcsrchr(pathinitlua, L'\\');
		if(pdir != nullptr)
		{
			*(pdir + 1) = L'\0';
			wcsncat_s(pathinitlua, fninitlua, _TRUNCATE);
		}
	}

	//%SystemRoot%\System32\IME\IMCRVSKK\init.lua
	// or %SystemRoot%\SysWOW64\IME\IMCRVSKK\init.lua
	if(luaL_dofile(lua, WCTOU8(pathinitlua)) == LUA_OK)
	{
		return;
	}

	UninitLua();
}

void UninitLua()
{
	if(lua != nullptr)
	{
		lua_close(lua);
		lua = nullptr;
	}
}
