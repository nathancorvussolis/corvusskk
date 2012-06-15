
#ifndef CORVUSSRV_H
#define CORVUSSRV_H

//for resource
#define RC_FILE				"corvussrv"

#define MAX_KRNLOBJNAME		256

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

// ConfigSrv
void CreateConfigPath();
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
HANDLE StartSaveUserDicEx();
void StartSaveUserDic();

// ConvSKKServer
void ConvSKKServer(const std::wstring &text, CANDIDATES &candidates);
void ConnectSKKServer();
void DisconnectSKKServer();

extern LPCWSTR TextServiceDesc;

extern CRITICAL_SECTION csUserDataSave;
extern BOOL bUserDicChg;
extern OSVERSIONINFOW ovi;

extern USERDICS userdics;
extern COMPLEMENTS complements;

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];
extern WCHAR pathuserdicxml[MAX_PATH];
extern WCHAR pathskkcvdicxml[MAX_PATH];
extern WCHAR pathskkcvdicidx[MAX_PATH];

extern WCHAR krnlobjsddl[MAX_KRNLOBJNAME];	//SDDL
extern WCHAR pipename[MAX_KRNLOBJNAME];	//名前付きパイプ
extern WCHAR srvmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// 辞書サーバ設定
extern BOOL serv;		//SKK辞書サーバを使用する
extern WCHAR host[MAX_SKKSERVER_HOST];	//ホスト
extern WCHAR port[MAX_SKKSERVER_PORT];	//ポート
extern DWORD timeout;	//タイムアウト

#endif //CORVUSSRV_H
