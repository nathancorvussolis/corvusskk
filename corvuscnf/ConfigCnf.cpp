
#include "corvuscnf.h"
#include "convtable.h"

const WCHAR *TextServiceDesc = TEXTSERVICE_DESC;

#define CCSUNICODE L",ccs=UNICODE"
const WCHAR *RccsUNICODE = L"r" CCSUNICODE;
const WCHAR *WccsUNICODE = L"w" CCSUNICODE;

WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// ファイルパス
WCHAR pathconfig[MAX_PATH];		//設定
WCHAR pathconfdic[MAX_PATH];	//SKK辞書リスト
WCHAR pathconfcvpt[MAX_PATH];	//変換位置指定
WCHAR pathconfkana[MAX_PATH];	//ローマ字仮名変換表
WCHAR pathconfjlat[MAX_PATH];	//ASCII全英変換表
WCHAR pathskkcvdic[MAX_PATH];	//取込SKK辞書
WCHAR pathskkcvidx[MAX_PATH];	//取込SKK辞書インデックス
// ファイル名
const WCHAR *fnconfig = L"config.ini";
const WCHAR *fnconfdic = L"confdic.txt";
const WCHAR *fnconfcvpt = L"confcvpt.txt";
const WCHAR *fnconfkana = L"confkana.txt";
const WCHAR *fnconfjlat = L"confjlat.txt";
const WCHAR *fnskkcvdic = L"skkcv.dic";
const WCHAR *fnskkcvidx = L"skkcv.idx";

void CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfig[0] = L'\0';
	pathconfdic[0] = L'\0';
	pathconfcvpt[0] = L'\0';
	pathconfkana[0] = L'\0';
	pathconfjlat[0] = L'\0';
	pathskkcvdic[0] = L'\0';
	pathskkcvidx[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, NULL, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	_wmkdir(appdata);

	wcsncpy_s(pathconfig, appdata, _TRUNCATE);
	wcsncat_s(pathconfig, fnconfig, _TRUNCATE);

	wcsncpy_s(pathconfdic, appdata, _TRUNCATE);
	wcsncat_s(pathconfdic, fnconfdic, _TRUNCATE);

	wcsncpy_s(pathconfcvpt, appdata, _TRUNCATE);
	wcsncat_s(pathconfcvpt, fnconfcvpt, _TRUNCATE);

	wcsncpy_s(pathconfkana, appdata, _TRUNCATE);
	wcsncat_s(pathconfkana, fnconfkana, _TRUNCATE);

	wcsncpy_s(pathconfjlat, appdata, _TRUNCATE);
	wcsncat_s(pathconfjlat, fnconfjlat, _TRUNCATE);

	wcsncpy_s(pathskkcvdic, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvdic, fnskkcvdic, _TRUNCATE);

	wcsncpy_s(pathskkcvidx, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvidx, fnskkcvidx, _TRUNCATE);

	HANDLE hToken;
	PTOKEN_USER pTokenUser;
	DWORD dwLength;
	LPWSTR pszUserSid = L"";
	WCHAR szDigest[32+1];
	MD5_DIGEST digest;

	ZeroMemory(cnfmutexname, sizeof(cnfmutexname));
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

	if(GetMD5(&digest, (const BYTE *)pszUserSid, (DWORD)wcslen(pszUserSid)*sizeof(WCHAR)))
	{
		for(int i=0; i<_countof(digest.digest); i++)
		{
			_snwprintf_s(&szDigest[i*2], _countof(szDigest)-i*2, _TRUNCATE, L"%02x", digest.digest[i]);
		}
	}

	_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", CORVUSCNFMUTEX, szDigest);

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
