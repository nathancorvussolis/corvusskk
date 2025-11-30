#pragma once

#include "common.h"
#include "parseskkdic.h"
#include "lua.hpp"

// ConfigMgr
void CreateConfigPath();
void UpdateConfigPath();
void CreateIpcName();
void LoadConfig(BOOL sysexit = FALSE);
BOOL IsFileModified(LPCWSTR path, FILETIME *ft);
void InitLua();
void UninitLua();

// lcrvmgr
int lua_search_skk_dictionary(lua_State *L);
int lua_search_user_dictionary(lua_State *L);
int lua_search_skk_server(lua_State *L);
int lua_search_skk_server_info(lua_State *L);
int lua_search_unicode(lua_State *L);
int lua_search_jisx0213(lua_State *L);
int lua_search_jisx0208(lua_State *L);
int lua_search_character_code(lua_State *L);
int lua_complement(lua_State *L);
int lua_reverse(lua_State *L);
int lua_add(lua_State *L);
int lua_delete(lua_State *L);
int lua_save(lua_State *L);

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

// SearchUserDictionary
std::wstring SearchUserDic(const std::wstring &searchkey, const std::wstring &okuri);
void SearchComplement(const std::wstring &searchkey, SKKDICCANDIDATES &sc);
void SearchComplementSearchCandidate(SKKDICCANDIDATES &sc, int max);
void SearchReverse(const std::wstring &candidate, std::wstring &key);
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
std::thread ServerStart();

//client
#define SKK_REQ		'1'
#define SKK_VER		'2'
#define SKK_HST		'3'
//server
#define SKK_HIT		'1'

extern HINSTANCE hInst;
extern HWND hWndMgr;
extern CRITICAL_SECTION csUserDict;
extern CRITICAL_SECTION csUserData;
extern CRITICAL_SECTION csSaveUserDic;
extern CRITICAL_SECTION csSKKSocket;
extern BOOL bUserDicChg;
extern FILETIME ftConfig;
extern FILETIME ftSKKDic;
extern lua_State *lua;
#ifdef _DEBUG
#define WM_USER_SETTEXT (WM_USER + 1)
extern CRITICAL_SECTION csEdit;
extern HWND hWndEdit;
extern HFONT hFont;
#endif
extern std::thread tSrvThr;
extern std::atomic_bool bSrvExit;

extern LPCWSTR TextServiceDesc;
extern LPCWSTR DictionaryManagerClass;

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];	//設定
extern WCHAR pathuserdic[MAX_PATH];		//ユーザー辞書
extern WCHAR pathskkdic[MAX_PATH];		//取込SKK辞書
extern WCHAR pathinitlua[MAX_PATH];		//init.lua
extern WCHAR pathbackup[MAX_PATH];		//ユーザー辞書バックアップレフィックス

extern WCHAR radconfigxml[MAX_PATH];	//設定 FOLDERID_RoamingAppData
extern WCHAR sysconfigxml[MAX_PATH];	//設定 FOLDERID_Windows\IME
extern INT csidlconfigxml;				//設定 CSIDL_APPDATA/CSIDL_WINDOWS
extern WCHAR radskkdic[MAX_PATH];		//取込SKK辞書 FOLDERID_RoamingAppData
extern WCHAR sysskkdic[MAX_PATH];		//取込SKK辞書 FOLDERID_Windows\IME
extern INT csidlskkdic;					//取込SKK辞書 CSIDL_APPDATA/CSIDL_WINDOWS

extern WCHAR krnlobjsddl[MAX_SECURITYDESC];	//SDDL
extern WCHAR mgrpipename[MAX_PIPENAME];		//名前付きパイプ
extern WCHAR mgrmutexname[MAX_PATH];		//ミューテックス

// 辞書サーバー設定
extern BOOL serv;		//SKK辞書サーバーを使用する
extern WCHAR host[MAX_SKKSERVER_HOST];	//ホスト
extern WCHAR port[MAX_SKKSERVER_PORT];	//ポート
extern DWORD encoding;	//エンコーディング
extern DWORD timeout;	//タイムアウト

extern INT generation;		//ユーザー辞書バックアップ世代数

extern BOOL precedeokuri;	//送り仮名が一致した候補を優先する
extern BOOL compincback;	//前方一致と後方一致で補完する
extern BOOL compwithall;	//全ての辞書で補完する
