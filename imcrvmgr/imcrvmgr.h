#pragma once

#include "common.h"
#include "parseskkdic.h"
#include "lua.hpp"

typedef struct {
	SKKDIC userdic;
	USEROKURI userokuri;
	KEYORDER keyorder_n;
	KEYORDER keyorder_a;
} USERDATA;

// ConfigMgr
void CreateConfigPath();
void UpdateConfigPath();
void CreateIpcName();
void LoadConfig();
BOOL IsFileModified(LPCWSTR path, FILETIME *ft);
void InitLua();
void UninitLua();

// SearchCharacter
std::wstring SearchUnicode(const std::wstring &searchkey);
std::wstring SearchJISX0213(const std::wstring &searchkey);
std::wstring SearchJISX0208(const std::wstring &searchkey);
std::wstring SearchCharacterCode(const std::wstring &searchkey);
void SendKeyboardInput(WCHAR command);

// SearchDictionary
void SearchDictionary(const std::wstring &searchkey, const std::wstring &okuri, SKKDICCANDIDATES &sc);
std::wstring SearchSKKDic(const std::wstring &searchkey, const std::wstring &okuri);
void MakeSKKDicPos();
std::wstring ConvertKey(const std::wstring &searchkey, const std::wstring &okuri);
std::wstring ConvertCandidate(const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &okuri);
int lua_search_skk_dictionary(lua_State *lua);
int lua_search_user_dictionary(lua_State *lua);
int lua_search_skk_server(lua_State *lua);
int lua_search_skk_server_info(lua_State *lua);
int lua_search_unicode(lua_State *lua);
int lua_search_jisx0213(lua_State *lua);
int lua_search_jisx0208(lua_State *lua);
int lua_search_character_code(lua_State *lua);
int lua_complement(lua_State *lua);
int lua_add(lua_State *lua);
int lua_delete(lua_State *lua);
int lua_save(lua_State *lua);

// SearchUserDictionary
std::wstring SearchUserDic(const std::wstring &searchkey, const std::wstring &okuri);
void SearchComplement(const std::wstring &searchkey, SKKDICCANDIDATES &sc);
void SearchComplementSearchCandidate(SKKDICCANDIDATES &sc, int max);
void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, const std::wstring &okuri);
void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate);
BOOL LoadUserDic();
void StartSaveUserDic(BOOL bThread);
void BackUpUserDic();

// SearchSKKServer
std::wstring SearchSKKServer(const std::wstring &searchkey);
void StartConnectSKKServer();
void DisconnectSKKServer();
void CleanUpSKKServer();
std::wstring GetSKKServerInfo(CHAR req);

// Server
void SrvProc(WCHAR command, const std::wstring &argument, std::wstring &result);
unsigned int __stdcall SrvThread(void *p);
HANDLE SrvStart();

//client
#define SKK_REQ		'1'
#define SKK_VER		'2'
#define SKK_HST		'3'
//server
#define SKK_HIT		'1'

#define BACKUP_GENS		7

extern CRITICAL_SECTION csUserDict;
extern CRITICAL_SECTION csSaveUserDic;
extern CRITICAL_SECTION csSKKSocket;
extern BOOL bUserDicChg;
extern FILETIME ftConfig;
extern FILETIME ftSKKDic;
#ifdef _DEBUG
extern HWND hWndEdit;
extern HFONT hFont;
#endif
extern HINSTANCE hInst;
extern HANDLE hMutex;
extern HANDLE hThreadSrv;
extern BOOL bSrvThreadExit;
extern lua_State *lua;

extern SKKDIC userdic;
extern USEROKURI userokuri;
extern KEYORDER keyorder_n;
extern KEYORDER keyorder_a;

extern LPCWSTR TextServiceDesc;
extern LPCWSTR DictionaryManagerClass;

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];	//設定
extern WCHAR pathuserdic[MAX_PATH];		//ユーザー辞書
extern WCHAR pathuserbak[MAX_PATH];		//ユーザー辞書バックアッププレフィックス
extern WCHAR pathskkdic[MAX_PATH];		//取込SKK辞書
extern WCHAR pathinitlua[MAX_PATH];		//init.lua

extern WCHAR krnlobjsddl[MAX_KRNLOBJNAME];	//SDDL
extern WCHAR mgrpipename[MAX_KRNLOBJNAME];	//名前付きパイプ
extern WCHAR mgrmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// 辞書サーバー設定
extern BOOL serv;		//SKK辞書サーバーを使用する
extern WCHAR host[MAX_SKKSERVER_HOST];	//ホスト
extern WCHAR port[MAX_SKKSERVER_PORT];	//ポート
extern DWORD encoding;	//エンコーディング
extern DWORD timeout;	//タイムアウト

extern BOOL precedeokuri;	//送り仮名が一致した候補を優先する
extern BOOL compincback;	//前方一致と後方一致で補完する
