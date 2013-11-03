
#ifndef COMMON_H
#define COMMON_H

#define TEXTSERVICE_NAME	L"CorvusSKK"
#define TEXTSERVICE_VER		L"1.4.2"

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

//for resource
#define RC_AUTHOR			"Nathan Corvus Solis"
#define RC_PRODUCT			"CorvusSKK"
#define RC_VERSION			"1.4.2"
#define RC_VERSION_D		1,4,2,0

#define DICBUFSIZE			0x2000
#define PIPEBUFSIZE			0x2000

#define MAX_KRNLOBJNAME		256
#define CONV_POINT_NUM		256
#define KEYRELEN			256
#define MAX_PRESERVEDKEY	8
#define MAX_SELKEY_C		9

#define MAX_SKKSERVER_HOST	(255+1)
#define MAX_SKKSERVER_PORT	(5+1)

#define MAX_WIDTH_DEFAULT	800

//request
#define REQ_SEARCH			L'1'	//辞書検索
#define REQ_COMPLEMENT		L'8'	//補完
#define REQ_CONVERSION		L'9'	//候補変換
#define REQ_USER_ADD_0		L'A'	//ユーザ辞書追加(送りあり、補完なし)
#define REQ_USER_ADD_1		L'B'	//ユーザ辞書追加(送りなし、補完あり)
#define REQ_USER_DEL_0		L'C'	//ユーザ辞書削除(送りあり、補完なし)
#define REQ_USER_DEL_1		L'D'	//ユーザ辞書削除(送りなし、補完あり)
#define REQ_USER_SAVE		L'S'	//ユーザ辞書保存
//reply
#define REP_OK				L'1'	//hit
#define REP_FALSE			L'4'	//nothing

#define CORVUSMGREXE		L"imcrvmgr.exe"
#define CORVUSCNFEXE		L"imcrvcnf.exe"
#ifndef _DEBUG
#define CORVUSKRNLOBJ		L"corvus-skk-"
#else
#define CORVUSKRNLOBJ		L"corvus-skk-debug-"
#endif
#define CORVUSMGRMUTEX		CORVUSKRNLOBJ L"mgr-"
#define CORVUSCNFMUTEX		CORVUSKRNLOBJ L"cnf-"
#define CORVUSMGRPIPE		L"\\\\.\\pipe\\" CORVUSKRNLOBJ

typedef struct {
	BYTE digest[16];
} MD5_DIGEST;

extern LPCWSTR RccsUNICODE;
extern LPCWSTR WccsUNICODE;
extern LPCWSTR RB;
extern LPCWSTR WB;

extern LPCWSTR fnconfigxml;	//設定
extern LPCWSTR fnuserdic;	//ユーザ辞書
extern LPCWSTR fnskkdic;	//取込SKK辞書
extern LPCWSTR fnskkidx;	//取込SKK辞書インデックス

BOOL IsVersion62AndOver();
BOOL GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen);
BOOL GetUserSid(LPWSTR *ppszUserSid);

#endif //COMMON_H
