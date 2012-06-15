
#ifndef CORVUSSRV_H
#define CORVUSSRV_H

#define TEXTSERVICE_NAME	L"CorvusSKK"
#define TEXTSERVICE_VER		L"0.6.0"

//for resource
#define RC_AUTHOR			"Corvus Solis"
#define RC_PRODUCT			"CorvusSKK"
#define RC_FILE				"corvussrv"
#define RC_VERSION			"0.6.0"
#define RC_VERSION_D		0,6,0,0

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

#define CORVUSSRVMUTEX	TEXTSERVICE_DESC L"_SRV_Mutex"
#define CORVUSSRVPIPE	L"\\\\.\\pipe\\" TEXTSERVICE_DESC L"_"

#define MAX_SKKSERVER_HOST	(255+1)
#define MAX_SKKSERVER_PORT	(5+1)

//request from corvustip
#define REQ_SEARCH		L'1'	//辞書検索
#define REQ_COMPLEMENT	L'8'	//補完
#define REQ_USER_ADD_0	L'A'	//ユーザ辞書追加(補完なし)
#define REQ_USER_ADD_1	L'B'	//ユーザ辞書追加(補完あり)
#define REQ_USER_DEL	L'D'	//ユーザ辞書削除
#define REQ_USER_SAVE	L'S'	//ユーザ辞書書き込み
#define REQ_CHECK_ALIVE	L'?'	//check alive
//reply to corvustip
#define REP_OK			L'1'	//hit
#define REP_FALSE		L'4'	//nothig
#define REP_CHECK_ALIVE	L'!'	//alive

//候補   pair< candidate, annotation >
typedef std::pair< std::wstring, std::wstring > CANDIDATE;
typedef std::vector< CANDIDATE > CANDIDATES;

//ユーザ辞書   pair< key , candidates >
typedef std::map< std::wstring, CANDIDATES > USERDICS;
typedef std::pair< std::wstring, CANDIDATES > USERDICSPAIR;

//補完
typedef std::vector< std::wstring > COMPLEMENTS;

// configure
void CreateConfigPath();
void LoadConfig();

// convCharacter
void ConvUnicode(const std::wstring &text, CANDIDATES &candidates);
void ConvJISX0213(const std::wstring &text, CANDIDATES &candidates);

// convDictionary
void ConvDictionary(const std::wstring &searchkey, CANDIDATES &candidates, WCHAR command);
void ConvComplement(const std::wstring &searchkey, CANDIDATES &candidates);
void AddUserDic(const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, WCHAR command);
void DelUserDic(const std::wstring &searchkey, const std::wstring &candidate);
void LoadUserDic();
HANDLE StartSaveUserDic();

// convSKKServer
void ConvSKKServer(const std::wstring &text, CANDIDATES &candidates);
void ConnectSKKServer();
void DisconnectSKKServer();

extern const WCHAR *TextServiceDesc;

extern const WCHAR *RccsUNICODE;
extern const WCHAR *WccsUNICODE;

// critical section objects
extern CRITICAL_SECTION csUserDic;
extern CRITICAL_SECTION csUserDicS;
extern CRITICAL_SECTION csSKKServ;

extern BOOL bUserDicChg;

// ファイルパス
extern WCHAR pathconf[MAX_PATH];		//設定
extern WCHAR pathskkcvdic[MAX_PATH];	//取込SKK辞書
extern WCHAR pathskkcvidx[MAX_PATH];	//取込SKK辞書インデックス
extern WCHAR pathuserdic[MAX_PATH];		//ユーザ辞書
extern WCHAR pathusercmp[MAX_PATH];		//補完見出し語

extern WCHAR pipename[_MAX_FNAME];		//名前付きパイプ

// 辞書サーバ設定
extern BOOL serv;		//SKK辞書サーバを使用する
extern WCHAR host[MAX_SKKSERVER_HOST];	//ホスト
extern WCHAR port[MAX_SKKSERVER_PORT];	//ポート
extern DWORD timeout;	//タイムアウト

#endif //CORVUSSRV_H
