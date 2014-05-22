
#include "common.h"

#define CCSUNICODE L",ccs=UNICODE"
LPCWSTR RccsUNICODE = L"r" CCSUNICODE;
LPCWSTR WccsUNICODE = L"w" CCSUNICODE;
LPCWSTR RB = L"rb";
LPCWSTR WB = L"wb";

LPCWSTR fnconfigxml = L"config.xml";	//設定
LPCWSTR fnuserdic = L"userdict.txt";	//ユーザ辞書
LPCWSTR fnskkdic = L"skkdict.dic";		//取込SKK辞書
LPCWSTR fnskkidx = L"skkdict.idx";		//取込SKK辞書インデックス
LPCWSTR fninitlua = L"init.lua";		//init.lua

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputMark =
{
	{TF_CT_NONE, RGB(0x00, 0x00, 0x00)},	// TF_DA_COLOR crText;
	{TF_CT_NONE, RGB(0xFF, 0xFF, 0xFF)},	// TF_DA_COLOR crBk;
	TF_LS_DOT,					// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_INPUT				// TF_DA_ATTR_INFO bAttr;
};

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputText =
{
	{TF_CT_NONE, RGB(0x00, 0x00, 0x00)},	// TF_DA_COLOR crText;
	{TF_CT_NONE, RGB(0xFF, 0xFF, 0xFF)},	// TF_DA_COLOR crBk;
	TF_LS_DOT,					// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_INPUT				// TF_DA_ATTR_INFO bAttr;
};

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputOkuri =
{
	{TF_CT_NONE, RGB(0x00, 0x00, 0x00)},	// TF_DA_COLOR crText;
	{TF_CT_NONE, RGB(0xFF, 0xFF, 0xFF)},	// TF_DA_COLOR crBk;
	TF_LS_DASH,					// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_INPUT				// TF_DA_ATTR_INFO bAttr;
};

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvMark =
{
	{TF_CT_NONE, RGB(0x00, 0x00, 0x00)},	// TF_DA_COLOR crText;
	{TF_CT_NONE, RGB(0xFF, 0xFF, 0xFF)},	// TF_DA_COLOR crBk;
	TF_LS_SOLID,				// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_TARGET_CONVERTED	// TF_DA_ATTR_INFO bAttr;
};

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvText =
{
	{TF_CT_COLORREF, RGB(0xFF, 0xFF, 0xFF)},	// TF_DA_COLOR crText;
	{TF_CT_COLORREF, RGB(0x00, 0x80, 0xFF)},	// TF_DA_COLOR crBk;
	TF_LS_NONE,					// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_TARGET_CONVERTED	// TF_DA_ATTR_INFO bAttr;
};

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvOkuri =
{
	{TF_CT_NONE, RGB(0x00, 0x00, 0x00)},	// TF_DA_COLOR crText;
	{TF_CT_NONE, RGB(0xFF, 0xFF, 0xFF)},	// TF_DA_COLOR crBk;
	TF_LS_SOLID,				// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_FIXEDCONVERTED		// TF_DA_ATTR_INFO bAttr;
};

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvAnnot =
{
	{TF_CT_NONE, RGB(0x00, 0x00, 0x00)},	// TF_DA_COLOR crText;
	{TF_CT_NONE, RGB(0xFF, 0xFF, 0xFF)},	// TF_DA_COLOR crBk;
	TF_LS_DASH,					// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_CONVERTED			// TF_DA_ATTR_INFO bAttr;
};

const BOOL c_daDisplayAttributeSeries[DISPLAYATTRIBUTE_INFO_NUM] =
{
	FALSE, TRUE, FALSE, FALSE, TRUE, TRUE, FALSE
};

BOOL IsVersion62AndOver()
{
	OSVERSIONINFOEXW osvi;
	DWORDLONG mask = 0;

	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 2;

	VER_SET_CONDITION(mask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(mask, VER_MINORVERSION, VER_GREATER_EQUAL);

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, mask);
}

BOOL IsVersion63AndOver()
{
	OSVERSIONINFOEXW osvi;
	DWORDLONG mask = 0;

	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 3;

	VER_SET_CONDITION(mask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(mask, VER_MINORVERSION, VER_GREATER_EQUAL);

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, mask);
}

BOOL GetSidMD5Digest(LPWSTR *ppszDigest)
{
	BOOL bRet = FALSE;
	LPWSTR pszUserSid = NULL;
	LPWSTR pszLogonSid = NULL;
	MD5_DIGEST digest;
	DWORD dwLSid;
	LPWSTR pLSid;
	int i;

	if(ppszDigest == NULL)
	{
		return FALSE;
	}

	if(GetUserSid(&pszUserSid) && GetLogonSid(&pszLogonSid))
	{
		dwLSid = (DWORD)(wcslen(pszUserSid) + wcslen(pszLogonSid) + 2);
		pLSid = (LPWSTR)LocalAlloc(LPTR, dwLSid * sizeof(WCHAR));

		if(pLSid != NULL)
		{
			_snwprintf_s(pLSid, dwLSid, _TRUNCATE, L"%s %s", pszUserSid, pszLogonSid);

			*ppszDigest = (LPWSTR)LocalAlloc(LPTR, (_countof(digest.digest) * 2 + 1) * sizeof(WCHAR));

			if(*ppszDigest != NULL)
			{
				if(GetMD5(&digest, (CONST BYTE *)pLSid, dwLSid * sizeof(WCHAR)))
				{
					for(i = 0; i < _countof(digest.digest); i++)
					{
						_snwprintf_s(*ppszDigest + i * 2, (_countof(digest.digest) * 2 + 1) - i * 2,
							_TRUNCATE, L"%02x", digest.digest[i]);
					}

					bRet = TRUE;
				}
				else
				{
					LocalFree(*ppszDigest);
				}
			}

			LocalFree(pLSid);
		}
	}

	if(pszUserSid != NULL)
	{
		LocalFree(pszUserSid);
	}

	if(pszLogonSid != NULL)
	{
		LocalFree(pszLogonSid);
	}

	return bRet;
}

BOOL GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen)
{
	BOOL bRet = FALSE;
	HCRYPTPROV hProv = NULL;
	HCRYPTHASH hHash = NULL;
	BYTE *pbData;
	DWORD dwDataLen;

	if(digest == NULL || data == NULL)
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

BOOL GetUserSid(LPWSTR *ppszUserSid)
{
	BOOL bRet = FALSE;
	HANDLE hToken;
	PTOKEN_USER pTokenUser;
	DWORD dwLength;

	if(ppszUserSid == NULL)
	{
		return FALSE;
	}

	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
		pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);
		if(GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
		{
			if(ConvertSidToStringSidW(pTokenUser->User.Sid, ppszUserSid))
			{
				bRet = TRUE;
			}
		}
		LocalFree(pTokenUser);
		CloseHandle(hToken);
	}

	return bRet;
}

BOOL GetLogonSid(LPWSTR *ppszLogonSid)
{
	BOOL bRet = FALSE;
	HANDLE hToken;
	PTOKEN_GROUPS pTokenGroup;
	DWORD dwLength;
	DWORD dwIndex;

	if(ppszLogonSid == NULL)
	{
		return FALSE;
	}

	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenGroups, NULL, 0, &dwLength);
		pTokenGroup = (PTOKEN_GROUPS)LocalAlloc(LPTR, dwLength);
		if(GetTokenInformation(hToken, TokenGroups, pTokenGroup, dwLength, &dwLength))
		{
			for(dwIndex = 0; dwIndex < pTokenGroup->GroupCount; dwIndex++)
			{
				if((pTokenGroup->Groups[dwIndex].Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID)
				{
					if(ConvertSidToStringSidW(pTokenGroup->Groups[dwIndex].Sid, ppszLogonSid))
					{
						bRet = TRUE;
					}
					break;
				}
			}
		}
		LocalFree(pTokenGroup);
		CloseHandle(hToken);
	}

	return bRet;
}
