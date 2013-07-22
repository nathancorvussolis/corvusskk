
#ifndef IMCRVMGR_H
#define IMCRVMGR_H

#include "common.h"

//for resource
#define RC_FILE				"imcrvmgr"

//候補   pair< candidate, annotation >
typedef std::pair< std::wstring, std::wstring > CANDIDATE;
typedef std::vector< CANDIDATE > CANDIDATES;

//ユーザ辞書   pair< key , candidates >
typedef std::map< std::wstring, CANDIDATES > USERDIC;
typedef std::pair< std::wstring, CANDIDATES > USERDICENTRY;

//見出し語順序
typedef std::vector< std::wstring > KEYORDER;

//検索結果
typedef std::pair< std::wstring, std::wstring > CANDIDATEPAIR; //表示用候補、辞書登録用候補
typedef std::pair< CANDIDATEPAIR, std::wstring > SEARCHRESULT; //候補、注釈
typedef std::vector< SEARCHRESULT > SEARCHRESULTS;

typedef struct {
	USERDIC userdic;
	KEYORDER complements;
	KEYORDER accompaniments;
} USERDATA;

// ConfigMgr
void CreateConfigPath();
void LoadConfig();
BOOL IsFileUpdated(LPCWSTR path, FILETIME *ft);

// ConvCharacter
void ConvUnicode(const std::wstring &text, CANDIDATES &candidates);
void ConvJISX0213(const std::wstring &text, CANDIDATES &candidates);

// ConvDictionary
void ConvDictionary(const std::wstring &searchkey, const std::wstring &searchkeyorg, SEARCHRESULTS &searchresults);
void InitSKKDic();
void ConvCandidate(const std::wstring &searchkey, const std::wstring &candidate, std::wstring &conv);

// ConvGadget
std::wstring ConvGaget(const std::wstring &key, const std::wstring &candidate);

// ConvNum
std::wstring ConvNum(const std::wstring &key, const std::wstring &candidate);

// ConvUserDictionary
void ConvUserDic(const std::wstring &searchkey, CANDIDATES &candidates);
void ConvComplement(const std::wstring &searchkey, CANDIDATES &candidates);
void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation);
void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate);
BOOL LoadSKKUserDic();
BOOL SaveSKKUserDic(USERDATA* userdata);
HANDLE StartSaveSKKUserDicEx();
void StartSaveSKKUserDic();

// ConvSKKServer
void ConvSKKServer(const std::wstring &text, CANDIDATES &candidates);
void ConnectSKKServer();
void DisconnectSKKServer();
void GetSKKServerVersion();

extern LPCWSTR TextServiceDesc;

extern CRITICAL_SECTION csUserDataSave;
extern BOOL bUserDicChg;

extern USERDIC userdic;
extern KEYORDER complements;
extern KEYORDER accompaniments;

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];	//設定
extern WCHAR pathuserdic[MAX_PATH];		//ユーザ辞書
extern WCHAR pathskkdic[MAX_PATH];		//取込SKK辞書
extern WCHAR pathskkidx[MAX_PATH];		//取込SKK辞書インデックス

extern WCHAR krnlobjsddl[MAX_KRNLOBJNAME];	//SDDL
extern WCHAR mgrpipename[MAX_KRNLOBJNAME];	//名前付きパイプ
extern WCHAR mgrmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// 辞書サーバ設定
extern BOOL serv;		//SKK辞書サーバを使用する
extern WCHAR host[MAX_SKKSERVER_HOST];	//ホスト
extern WCHAR port[MAX_SKKSERVER_PORT];	//ポート
extern DWORD encoding;	//エンコーディング
extern DWORD timeout;	//タイムアウト

#endif //IMCRVMGR_H
