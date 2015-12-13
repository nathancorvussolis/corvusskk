
#ifndef IMCRVMGR_H
#define IMCRVMGR_H

#include "common.h"
#include "parseskkdic.h"
#include "lua.hpp"

typedef struct {
	SKKDIC userdic;
	USEROKURI userokuri;
	KEYORDER complements;
	KEYORDER accompaniments;
} USERDATA;

// ConfigMgr
void CreateConfigPath();
void LoadConfig();
BOOL IsFileUpdated(LPCWSTR path, FILETIME *ft);
void InitLua();
void UninitLua();

// ConvGadget
std::wstring ConvGadget(const std::wstring &key, const std::wstring &candidate);

// ConvNum
std::wstring ConvNum(const std::wstring &key, const std::wstring &candidate);

// SearchCharacter
std::wstring SearchUnicode(const std::wstring &searchkey);
std::wstring SearchJISX0213(const std::wstring &searchkey);
std::wstring SearchJISX0208(const std::wstring &searchkey);
std::wstring SearchCharacterCode(const std::wstring &searchkey);

// SearchDictionary
void SearchDictionary(const std::wstring &searchkey, const std::wstring &okuri, SKKDICCANDIDATES &sc);
std::wstring SearchSKKDic(const std::wstring &searchkey);
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
void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, const std::wstring &okuri);
void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate);
BOOL LoadSKKUserDic();
void SaveSKKUserDic(void *p);
void StartSaveSKKUserDic(BOOL bThread);
void BackUpSKKUserDic();

// SearchSKKServer
std::wstring SearchSKKServer(const std::wstring &searchkey);
void ConnectSKKServer();
void DisconnectSKKServer();
std::wstring GetSKKServerInfo(CHAR req);

//client
#define SKK_REQ		'1'
#define SKK_VER		'2'
#define SKK_HST		'3'
//server
#define SKK_HIT		'1'

#define BACKUP_GENS		3

extern LPCWSTR TextServiceDesc;
extern LPCWSTR DictionaryManagerClass;

extern CRITICAL_SECTION csUserDataSave;
extern BOOL bUserDicChg;

extern lua_State *lua;

extern SKKDIC userdic;
extern USEROKURI userokuri;
extern KEYORDER complements;
extern KEYORDER accompaniments;

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];	//設定
extern WCHAR pathuserdic[MAX_PATH];		//ユーザー辞書
extern WCHAR pathuserbak[MAX_PATH];		//ユーザー辞書バックアッププレフィックス
extern WCHAR pathskkdic[MAX_PATH];		//取込SKK辞書
extern WCHAR pathskkidx[MAX_PATH];		//取込SKK辞書インデックス
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

#endif //IMCRVMGR_H
