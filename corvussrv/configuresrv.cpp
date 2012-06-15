
#include "corvussrv.h"

const WCHAR *TextServiceDesc = TEXTSERVICE_DESC;

#define CCSUNICODE L",ccs=UNICODE"
const WCHAR *RccsUNICODE = L"r" CCSUNICODE;
const WCHAR *WccsUNICODE = L"w" CCSUNICODE;

// ファイルパス
WCHAR pathconf[MAX_PATH];		//設定
WCHAR pathskkcvdic[MAX_PATH];	//取込SKK辞書
WCHAR pathskkcvidx[MAX_PATH];	//取込SKK辞書インデックス
WCHAR pathuserdic[MAX_PATH];	//ユーザ辞書
WCHAR pathusercmp[MAX_PATH];	//補完見出し語
// ファイル名
const WCHAR *fnconfig = L"config.ini";
const WCHAR *fnskkcvdic = L"skkcv.dic";
const WCHAR *fnskkcvidx = L"skkcv.idx";
const WCHAR *fnuserdic = L"userdic.txt";
const WCHAR *fnusercmp = L"usercmp.txt";

WCHAR pipename[_MAX_FNAME];		//名前付きパイプ

// config.ini セクション
const WCHAR * IniSecServer = L"Server";

// config.ini キー 辞書サーバ
const WCHAR *Serv = L"Serv";
const WCHAR *Host = L"Host";
const WCHAR *Port = L"Port";
const WCHAR *TimeOut = L"TimeOut";

// 辞書サーバ設定
BOOL serv;		//SKK辞書サーバを使用する
WCHAR host[MAX_SKKSERVER_HOST] = {L'\0'};	//ホスト
WCHAR port[MAX_SKKSERVER_PORT] = {L'\0'};	//ポート
DWORD timeout;	//タイムアウト

void CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];
	WCHAR username[UNLEN + 1];
	DWORD dwSize;

	pathconf[0] = L'\0';
	pathskkcvdic[0] = L'\0';
	pathskkcvidx[0] = L'\0';
	pathuserdic[0] = L'\0';
	pathusercmp[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, NULL, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	_wmkdir(appdata);

	wcsncpy_s(pathconf, appdata, _TRUNCATE);
	wcsncat_s(pathconf, fnconfig, _TRUNCATE);

	wcsncpy_s(pathskkcvdic, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvdic, fnskkcvdic, _TRUNCATE);

	wcsncpy_s(pathskkcvidx, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvidx, fnskkcvidx, _TRUNCATE);

	wcsncpy_s(pathuserdic, appdata, _TRUNCATE);
	wcsncat_s(pathuserdic, fnuserdic, _TRUNCATE);

	wcsncpy_s(pathusercmp, appdata, _TRUNCATE);
	wcsncat_s(pathusercmp, fnusercmp, _TRUNCATE);

	//名前付きパイプ
	ZeroMemory(username, sizeof(username));
	dwSize = _countof(username) - 1;
	GetUserNameW(username, &dwSize);
	_snwprintf_s(pipename, _TRUNCATE, L"%s%s", CORVUSSRVPIPE, username);
}

void LoadConfig()
{
	WCHAR num[2];
	WCHAR hosttmp[MAX_SKKSERVER_HOST];	//ホスト
	WCHAR porttmp[MAX_SKKSERVER_PORT];	//ポート

	GetPrivateProfileStringW(IniSecServer, Serv, L"0", num, 2, pathconf);
	serv = _wtoi(num);
	if(serv != TRUE && serv != FALSE)
	{
		serv = FALSE;
	}

	//使用しないとき切断する
	if(!serv)
	{
		DisconnectSKKServer();
	}

	GetPrivateProfileStringW(IniSecServer, Host, L"", hosttmp, _countof(hosttmp), pathconf);

	GetPrivateProfileStringW(IniSecServer, Port, L"", porttmp, _countof(porttmp), pathconf);

	timeout = GetPrivateProfileInt(IniSecServer, TimeOut, 1000, pathconf);

	//変更があったら接続し直す
	if(wcscmp(hosttmp, host) != 0 || wcscmp(porttmp, port) != 0)
	{
		wcsncpy_s(host, hosttmp, _TRUNCATE);
		wcsncpy_s(port, porttmp, _TRUNCATE);

		DisconnectSKKServer();
		if(serv)
		{
			ConnectSKKServer();
		}
	}
}
