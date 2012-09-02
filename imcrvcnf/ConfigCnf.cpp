
#include "configxml.h"
#include "imcrvcnf.h"

LPCWSTR TextServiceDesc = TEXTSERVICE_DESC;

IXmlWriter *pXmlWriter;
IStream *pXmlFileStream;

WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// ファイルパス
WCHAR pathconfigxml[MAX_PATH];		//設定
WCHAR pathskkcvdicxml[MAX_PATH];	//取込SKK辞書
WCHAR pathskkcvdicidx[MAX_PATH];	//取込SKK辞書インデックス

void CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfigxml[0] = L'\0';
	pathskkcvdicxml[0] = L'\0';
	pathskkcvdicidx[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	_wmkdir(appdata);

	wcsncpy_s(pathconfigxml, appdata, _TRUNCATE);
	wcsncat_s(pathconfigxml, fnconfigxml, _TRUNCATE);

	wcsncpy_s(pathskkcvdicxml, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvdicxml, fnskkcvdicxml, _TRUNCATE);

	wcsncpy_s(pathskkcvdicidx, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvdicidx, fnskkcvdicidx, _TRUNCATE);

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
