#pragma once

#include "version.h"

#define MAX_SECURITYDESC	256		//string security descriptor
#define MAX_PIPENAME		256		//named pipe name
#define PIPEBUFSIZE			0x2000	//named pipe, 16KB with wchar_t
#define READBUFSIZE			0x200	//read skk dictionary, 512B with wchar_t/char
#define RECVBUFSIZE			0x800	//receive from skk server, 2KB with char

#define MAX_SKKSERVER_HOST	(255 + 1)	//SKKサーバー ホスト 読み込みバッファ
#define MAX_SKKSERVER_PORT	(5 + 1)		//SKKサーバー ポート 読み込みバッファ
#define COMPMULTIDISP_DEF	3		//複数補完/複数動的補完の表示数
#define FONT_POINT_DEF		12		//フォントポイントサイズ
#define MAX_WIDTH_DEFAULT	320		//候補一覧の最大幅
#define UNTILCANDLIST_DEF	5		//候補一覧表示に要する変換回数
#define SHOWMODEINLTM_DEF	3000	//入力モードを表示するミリ秒数
#define MAX_SELKEY_C		9		//候補一覧選択キー最大数
#define MAX_SELKEY			7		//候補一覧選択キー数
#define PRESERVEDKEY_NUM	2		//キー設定ON/OFF
#define MAX_PRESERVEDKEY	8		//キー設定ON/OFF最大数
#define MAX_KEYRE			256		//キー１/２ 読み込みバッファ
#define MAX_CONV_POINT		256		//変換位置指定最大数

//request
#define REQ_SEARCH			L'1'	//辞書検索
#define REQ_COMPLEMENT		L'4'	//補完
#define REQ_CONVERTKEY		L'5'	//見出し語変換
#define REQ_CONVERTCND		L'6'	//候補変換
#define REQ_USER_ADD_A		L'A'	//ユーザー辞書追加(送りあり、補完なし)
#define REQ_USER_ADD_N		L'B'	//ユーザー辞書追加(送りなし、補完あり)
#define REQ_USER_DEL_A		L'C'	//ユーザー辞書削除(送りあり、補完なし)
#define REQ_USER_DEL_N		L'D'	//ユーザー辞書削除(送りなし、補完あり)
#define REQ_USER_SAVE		L'S'	//ユーザー辞書保存
#define REQ_EXEC_CNF		L'P'	//設定ダイアログ起動
#define REQ_CAPS_LOCK		L'I'	//Caps Lock
#define REQ_KANA_LOCK		L'J'	//Kana Lock
#define REQ_WATCHDOG		L'W'	//Watchdog
#define REQ_EXIT			L'X'	//Exit
//reply
#define REP_OK				L'T'	//hit
#define REP_FALSE			L'F'	//nothing

#define SYSTEMROOT_IME_DIR	L"IME"

#define IMCRVMGREXE			L"imcrvmgr.exe"
#define IMCRVCNFEXE			L"imcrvcnf.exe"
#ifndef _DEBUG
#define IMCRVKRNLOBJ		L"corvusskk-"
#else
#define IMCRVKRNLOBJ		L"corvusskk-debug-"
#endif
#define IMCRVMGRMUTEX		IMCRVKRNLOBJ L"mgr-"
#define IMCRVCNFMUTEX		IMCRVKRNLOBJ L"cnf-"
#define IMCRVMGRPIPE		L"\\\\.\\pipe\\" IMCRVKRNLOBJ

#define BOM L'\uFEFF'

extern LPCWSTR modeRccsUTF16;
extern LPCWSTR modeWccsUTF16;
extern LPCWSTR modeRccsUTF8;
extern LPCWSTR modeWccsUTF8;
extern LPCWSTR modeRT;
extern LPCWSTR modeRB;
extern LPCWSTR modeWB;

extern LPCWSTR fnconfigxml;	//設定
extern LPCWSTR fnuserdic;	//ユーザー辞書
extern LPCWSTR fnuserbak;	//ユーザー辞書バックアッププレフィックス
extern LPCWSTR fnskkdic;	//取込SKK辞書
extern LPCWSTR fninitlua;	//init.lua

#define DISPLAY_LIST_COLOR_NUM	8
#define DISPLAY_MODE_COLOR_NUM	8
#define DISPLAYATTRIBUTE_INFO_NUM	7

extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputMark;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputText;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputOkuri;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvMark;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvText;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvOkuri;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvAnnot;
extern const BOOL c_daDisplayAttributeSeries[DISPLAYATTRIBUTE_INFO_NUM];

BOOL IsWindowsVersionOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion, DWORD dwBuildNumber);
// Windows 8
#define IsWindowsVersion62OrLater() IsWindowsVersionOrLater(6, 2, 0)
// Windows 8.1
#define IsWindowsVersion63OrLater() IsWindowsVersionOrLater(6, 3, 0)
//// Windows 10
//#define IsWindowsVersion100OrLater() IsWindowsVersionOrLater(10, 0, 0)
//// Windows 10 ver.1507 Threshold 1 Released in July 2015
//#define IsWindowsVersion100TH1OrLater() IsWindowsVersionOrLater(10, 0, 10240)
//// Windows 10 ver.1511 Threshold 2 November Update
//#define IsWindowsVersion100TH2OrLater() IsWindowsVersionOrLater(10, 0, 10586)
//// Windows 10 ver.1607 Redstone 1 Anniversary Update
//#define IsWindowsVersion100RS1OrLater() IsWindowsVersionOrLater(10, 0, 14393)
// Windows 10 ver.1703 Redstone 2 Creators Update
#define IsWindowsVersion100RS2OrLater() IsWindowsVersionOrLater(10, 0, 15063)
//// Windows 10 ver.1709 Redstone 3 Fall Creators Update
//#define IsWindowsVersion100RS3OrLater() IsWindowsVersionOrLater(10, 0, 16299)
//// Windows 10 ver.1803 Redstone 4 April 2018 Update
//#define IsWindowsVersion100RS4OrLater() IsWindowsVersionOrLater(10, 0, 17134)
//// Windows 10 ver.1809 Redstone 5 October 2018 Update
//#define IsWindowsVersion100RS5OrLater() IsWindowsVersionOrLater(10, 0, 17763)
//// Windows 10 ver.1903 19H1 May 2019 Update
//#define IsWindowsVersion10019H1OrLater() IsWindowsVersionOrLater(10, 0, 18362)
//// Windows 10 ver.1909 19H2 November 2019 Update
//#define IsWindowsVersion10019H2OrLater() IsWindowsVersionOrLater(10, 0, 18363)
//// Windows 10 ver.2004 20H1 May 2020 Update
//#define IsWindowsVersion10020H1OrLater() IsWindowsVersionOrLater(10, 0, 19041)
//// Windows 10 ver.20H2 October 2020 Update
//#define IsWindowsVersion10020H2OrLater() IsWindowsVersionOrLater(10, 0, 19042)
//// Windows 10 ver.21H1 May 2021 Update
//#define IsWindowsVersion10021H1OrLater() IsWindowsVersionOrLater(10, 0, 19043)

#define C_USER_DEFAULT_SCREEN_DPI	96
#define C_FONT_LOGICAL_HEIGHT_PPI	72

BOOL GetUserUUID(LPWSTR *ppszUUID);
BOOL GetUserSid(LPWSTR *ppszUserSid);

BOOL StartProcess(HMODULE hCurrentModule, LPCWSTR lpFileName, LPCWSTR lpArgs = nullptr);

#define FORWARD_ITERATION_I(iterator, container) \
	for (auto (iterator) = (container).begin(); (iterator) != (container).end(); ++(iterator))
#define FORWARD_ITERATION(iterator, container) \
	for (auto (iterator) = (container).begin(); (iterator) != (container).end(); )
#define REVERSE_ITERATION_I(reverse_iterator, container) \
	for (auto (reverse_iterator) = (container).rbegin(); (reverse_iterator) != (container).rend(); ++(reverse_iterator))
#define REVERSE_ITERATION(reverse_iterator, container) \
	for (auto (reverse_iterator) = (container).rbegin(); (reverse_iterator) != (container).rend(); )
