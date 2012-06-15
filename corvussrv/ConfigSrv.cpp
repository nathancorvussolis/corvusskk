
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

WCHAR pipename[MAX_KRNLOBJNAME];	//名前付きパイプ
WCHAR pipesddl[MAX_KRNLOBJNAME];	//名前付きパイプSDDL
WCHAR srvmutexname[MAX_KRNLOBJNAME];	//ミューテックス

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

	HANDLE hToken;
	PTOKEN_USER pTokenUser;
	DWORD dwLength;
	LPWSTR pszUserSid = L"";
	WCHAR szDigest[32+1];
	MD5_DIGEST digest;

	ZeroMemory(pipename, sizeof(pipename));
	ZeroMemory(pipesddl, sizeof(pipesddl));
	ZeroMemory(szDigest, sizeof(szDigest));

	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
		pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);

		if(GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
		{
			ConvertSidToStringSidW(pTokenUser->User.Sid, &pszUserSid);
		}

		LocalFree(pTokenUser);
		CloseHandle(hToken);
	}

	_snwprintf_s(pipesddl, _TRUNCATE, L"D:(A;;FA;;;RC)(A;;FA;;;SY)(A;;FA;;;BA)(A;;FA;;;%s)S:(ML;;NW;;;LW)", pszUserSid);

	if(GetMD5(&digest, (const BYTE *)pszUserSid, (DWORD)wcslen(pszUserSid)*sizeof(WCHAR)))
	{
		for(int i=0; i<_countof(digest.digest); i++)
		{
			_snwprintf_s(&szDigest[i*2], _countof(szDigest)-i*2, _TRUNCATE, L"%02x", digest.digest[i]);
		}
	}

	_snwprintf_s(pipename, _TRUNCATE, L"%s%s", CORVUSSRVPIPE, szDigest);
	_snwprintf_s(srvmutexname, _TRUNCATE, L"%s%s", CORVUSSRVMUTEX, szDigest);

	LocalFree(pszUserSid);
}

BOOL GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen)
{
	BOOL bRet = FALSE;
	HCRYPTPROV hProv = NULL;
	HCRYPTHASH hHash = NULL;
	BYTE *pbData;
	DWORD dwDataLen;

	if(digest == NULL)
	{
		return FALSE;
	}

	ZeroMemory(digest, sizeof(digest));
	pbData = digest->digest;
	dwDataLen = sizeof(digest->digest);

	if(CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if(CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
		{
			if(CryptHashData(hHash, data, datalen, 0))
			{
				if(CryptGetHashParam(hHash, HP_HASHVAL, pbData, &dwDataLen, 0))
				{
					bRet = TRUE;
				}
			}
			CryptDestroyHash(hHash);
		}
		CryptReleaseContext(hProv, 0);
	}

	return bRet;
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
