
#ifndef COMMON_H
#define COMMON_H

#define TEXTSERVICE_NAME	L"CorvusSKK"
#define TEXTSERVICE_VER		L"0.8.1"

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

//for resource
#define RC_AUTHOR			"Nathan Corvus Solis"
#define RC_PRODUCT			"CorvusSKK"
#define RC_VERSION			"0.8.1"
#define RC_VERSION_D		0,8,1,0

#define MAX_KRNLOBJNAME		256
#define CONV_POINT_NUM		256
#define KEYRELEN			256
#define MAX_PRESERVEDKEY	8
#define MAX_SELKEY_C		9

#define MAX_SKKSERVER_HOST	(255+1)
#define MAX_SKKSERVER_PORT	(5+1)

//request
#define REQ_SEARCH		L'1'	//辞書検索
#define REQ_COMPLEMENT	L'8'	//補完
#define REQ_USER_ADD_0	L'A'	//ユーザ辞書追加(補完なし)
#define REQ_USER_ADD_1	L'B'	//ユーザ辞書追加(補完あり)
#define REQ_USER_DEL	L'D'	//ユーザ辞書削除
#define REQ_USER_SAVE	L'S'	//ユーザ辞書書き込み
//reply
#define REP_OK			L'1'	//hit
#define REP_FALSE		L'4'	//nothig

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

#define WCHAR_NULL	L'\0'

typedef struct {
	BYTE digest[16];
} MD5_DIGEST;

extern LPCWSTR RccsUNICODE;
extern LPCWSTR WccsUNICODE;

extern LPCWSTR fnconfigxml;
extern LPCWSTR fnuserdicxml;
extern LPCWSTR fnskkcvdicxml;
extern LPCWSTR fnskkcvdicidx;

void debugout(LPCWSTR format, ...);
BOOL GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen);
BOOL IsVersion6AndOver(OSVERSIONINFOW ovi);
BOOL IsVersion62AndOver(OSVERSIONINFOW ovi);
BOOL SetFileDaclAC(LPCWSTR path);

// for Windows 8
#if 1
#define EVENT_OBJECT_IME_SHOW               0x8027
#define EVENT_OBJECT_IME_HIDE               0x8028
#define EVENT_OBJECT_IME_CHANGE             0x8029

#define TF_TMF_IMMERSIVEMODE          0x40000000

#define TF_IPP_CAPS_IMMERSIVESUPPORT            0x00010000
#define TF_IPP_CAPS_SYSTRAYSUPPORT              0x00020000

extern const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT;
extern const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT;
extern const GUID GUID_COMPARTMENT_TKB_THIRDPARTYIME_MODE_ONOFF;
extern const GUID GUID_LBI_INPUTMODE;
extern const GUID GUID_INTEGRATIONSTYLE_SEARCHBOX;
#endif

#endif //COMMON_H
