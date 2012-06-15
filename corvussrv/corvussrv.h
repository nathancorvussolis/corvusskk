
#ifndef CORVUSSRV_H
#define CORVUSSRV_H

#define TEXTSERVICE_NAME	L"CorvusSKK"
#define TEXTSERVICE_VER		L"0.6.2"

//for resource
#define RC_AUTHOR			"Corvus Solis"
#define RC_PRODUCT			"CorvusSKK"
#define RC_FILE				"corvussrv"
#define RC_VERSION			"0.6.2"
#define RC_VERSION_D		0,6,2,0

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

#define MAX_PIPENAME		256
#ifndef _DEBUG
#define CORVUSKRNLOBJ	L"corvus-skk-"
#else
#define CORVUSKRNLOBJ	L"corvus-skk-debug-"
#endif
#define CORVUSSRVMUTEX		CORVUSKRNLOBJ L"srv"
#define CORVUSSRVPIPE		L"\\\\.\\pipe\\" CORVUSKRNLOBJ

#define MAX_SKKSERVER_HOST	(255+1)
#define MAX_SKKSERVER_PORT	(5+1)

//request from corvustip
#define REQ_SEARCH		L'1'	//辞書検索
#define REQ_COMPLEMENT	L'8'	//補完
#define REQ_USER_ADD_0	L'A'	//ユーザ辞書追加(補完なし)
#define REQ_USER_ADD_1	L'B'	//ユーザ辞書追加(補完あり)
#define REQ_USER_DEL	L'D'	//ユーザ辞書削除
#define REQ_USER_SAVE	L'S'	//ユーザ辞書書き込み
//reply to corvustip
#define REP_OK			L'1'	//hit
#define REP_FALSE		L'4'	//nothig

//候補   pair< candidate, annotation >
typedef std::pair< std::wstring, std::wstring > CANDIDATE;
typedef std::vector< CANDIDATE > CANDIDATES;

//ユーザ辞書   pair< key , candidates >
typedef std::map< std::wstring, CANDIDATES > USERDICS;
typedef std::pair< std::wstring, CANDIDATES > USERDICSPAIR;

//補完
typedef std::vector< std::wstring > COMPLEMENTS;

typedef struct {
	USERDICS userdics;
	COMPLEMENTS complements;
} USERDATA;

typedef struct {
	BYTE digest[16];
} MD5_DIGEST;

// ConfigSrv
void CreateConfigPath();
BOOL GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen);
void LoadConfig();

// ConvCharacter
void ConvUnicode(const std::wstring &text, CANDIDATES &candidates);
void ConvJISX0213(const std::wstring &text, CANDIDATES &candidates);

// ConvDictionary
void ConvDictionary(const std::wstring &searchkey, CANDIDATES &candidates, WCHAR command);
void ConvComplement(const std::wstring &searchkey, CANDIDATES &candidates);
void AddUserDic(const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, WCHAR command);
void DelUserDic(const std::wstring &searchkey, const std::wstring &candidate);
void LoadUserDic();
HANDLE StartSaveUserDic();

// ConvSKKServer
void ConvSKKServer(const std::wstring &text, CANDIDATES &candidates);
void ConnectSKKServer();
void DisconnectSKKServer();

extern const WCHAR *TextServiceDesc;

extern const WCHAR *RccsUNICODE;
extern const WCHAR *WccsUNICODE;

extern CRITICAL_SECTION csUserDataSave;
extern BOOL bUserDicChg;
extern USERDICS userdics;
extern COMPLEMENTS complements;

// ファイルパス
extern WCHAR pathconf[MAX_PATH];		//設定
extern WCHAR pathskkcvdic[MAX_PATH];	//取込SKK辞書
extern WCHAR pathskkcvidx[MAX_PATH];	//取込SKK辞書インデックス
extern WCHAR pathuserdic[MAX_PATH];		//ユーザ辞書
extern WCHAR pathusercmp[MAX_PATH];		//補完見出し語

extern WCHAR pipename[MAX_PIPENAME];	//名前付きパイプ
extern WCHAR pipesddl[MAX_PIPENAME];	//名前付きパイプSDDL

// 辞書サーバ設定
extern BOOL serv;		//SKK辞書サーバを使用する
extern WCHAR host[MAX_SKKSERVER_HOST];	//ホスト
extern WCHAR port[MAX_SKKSERVER_PORT];	//ポート
extern DWORD timeout;	//タイムアウト

#endif //CORVUSSRV_H
