
#include "common.h"

#define CCSUTF16 L", ccs=UTF-16LE"
#define CCSUTF8 L", ccs=UTF-8"
LPCWSTR RccsUTF16 = L"r" CCSUTF16;
LPCWSTR WccsUTF16 = L"w" CCSUTF16;
LPCWSTR RccsUTF8 = L"r" CCSUTF8;
LPCWSTR WccsUTF8 = L"w" CCSUTF8;
LPCWSTR RB = L"rb";
LPCWSTR WB = L"wb";

LPCWSTR fnconfigxml = L"config.xml";	//設定
LPCWSTR fnuserdic = L"userdict.txt";	//ユーザー辞書
LPCWSTR fnuserbak = L"userdict.bk";		//ユーザー辞書バックアッププレフィックス
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
	TF_ATTR_FIXEDCONVERTED		// TF_DA_ATTR_INFO bAttr;
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
	{TF_CT_NONE, RGB(0x00, 0x00, 0x00)},	// TF_DA_COLOR crText;
	{TF_CT_NONE, RGB(0xFF, 0xFF, 0xFF)},	// TF_DA_COLOR crBk;
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

BOOL IsWindowsVersionOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion, WORD wServicePackMajor)
{
	OSVERSIONINFOEXW osvi;
	DWORDLONG mask = 0;

	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = dwMajorVersion;
	osvi.dwMinorVersion = dwMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;

	VER_SET_CONDITION(mask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(mask, VER_MINORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(mask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, mask);
}

BOOL IsWindowsVersion62OrLater()
{
	return IsWindowsVersionOrLater(6, 2, 0);
}

BOOL IsWindowsVersion63OrLater()
{
	return IsWindowsVersionOrLater(6, 3, 0);
}

BOOL IsWindowsVersion100OrLater()
{
	return IsWindowsVersionOrLater(10, 0, 0);
}

BOOL GetDigest(DWORD dwProvType, ALG_ID AlgId, CONST PBYTE data, DWORD datalen, PBYTE digest, DWORD digestlen)
{
	BOOL bRet = FALSE;
	HCRYPTPROV hProv = NULL;
	HCRYPTHASH hHash = NULL;

	if(digest == NULL || data == NULL)
	{
		return FALSE;
	}

	ZeroMemory(digest, digestlen);

	if(CryptAcquireContextW(&hProv, NULL, NULL, dwProvType, CRYPT_VERIFYCONTEXT))
	{
		if(CryptCreateHash(hProv, AlgId, 0, 0, &hHash))
		{
			if(CryptHashData(hHash, data, datalen, 0))
			{
				bRet = CryptGetHashParam(hHash, HP_HASHVAL, digest, &digestlen, 0);
			}
			CryptDestroyHash(hHash);
		}
		CryptReleaseContext(hProv, 0);
	}

	return bRet;
}

BOOL IsLittleEndian()
{
	ULONG n = 1;
	return (*(UCHAR *)&n == 1);
}

ULONG htonlc(ULONG h)
{
	if(IsLittleEndian())
	{
		h = (h << 24) | ((h & 0x0000FF00) << 8) |
			((h >> 8) & 0x0000FF00) | (h >> 24);
	}
	return h;
}

ULONG ntohlc(ULONG n)
{
	return htonlc(n);
}

USHORT htonsc(USHORT h)
{
	if(IsLittleEndian())
	{
		h = (h << 8) | (h >> 8);
	}
	return h;
}

USHORT ntohsc(USHORT n)
{
	return htonsc(n);
}

BOOL GetUUID5(REFGUID rguid, CONST PBYTE name, DWORD namelen, LPGUID uuid)
{
	BOOL bRet = FALSE;
	CONST DWORD dwProvType = PROV_RSA_AES;
	CONST ALG_ID AlgId = CALG_SHA1;
	CONST DWORD dwDigestLen = 20;
	CONST USHORT maskVersion = 0x5000;
	GUID lguid = rguid;

	PBYTE pMessage = (PBYTE)LocalAlloc(LPTR, sizeof(lguid) + namelen);
	if(pMessage != NULL)
	{
		//network byte order
		lguid.Data1 = htonlc(lguid.Data1);
		lguid.Data2 = htonsc(lguid.Data2);
		lguid.Data3 = htonsc(lguid.Data3);

		memcpy_s(pMessage, sizeof(lguid), &lguid, sizeof(lguid));
		memcpy_s(pMessage + sizeof(lguid), namelen, name, namelen);

		BYTE digest[dwDigestLen];
		if(GetDigest(dwProvType, AlgId,
			pMessage, sizeof(lguid) + namelen, digest, dwDigestLen))
		{
			GUID dguid = *(GUID *)digest;
			//local byte order
			dguid.Data1 = ntohlc(dguid.Data1);
			dguid.Data2 = ntohsc(dguid.Data2);
			dguid.Data3 = ntohsc(dguid.Data3);
			//version
			dguid.Data3 &= 0x0FFF;
			dguid.Data3 |= maskVersion;
			//variant
			dguid.Data4[0] &= 0x3F;
			dguid.Data4[0] |= 0x80;

			*uuid = dguid;

			bRet = TRUE;
		}

		LocalFree(pMessage);
	}

	return bRet;
}

BOOL GetLogonInfo(PBYTE *ppLogonInfo)
{
	BOOL bRet = FALSE;
	HANDLE hToken = INVALID_HANDLE_VALUE;
	DWORD dwLength = 0;
	DWORD dwUserSidLen = 0;

	if(ppLogonInfo == NULL)
	{
		return FALSE;
	}

	*ppLogonInfo = NULL;

	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
		PTOKEN_USER pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);

		if(pTokenUser != NULL)
		{
			if(GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
			{
				dwUserSidLen = GetLengthSid(pTokenUser->User.Sid);
				*ppLogonInfo = (PBYTE)LocalAlloc(LPTR, dwUserSidLen + sizeof(LUID));
				if(*ppLogonInfo != NULL)
				{
					bRet = CopySid(dwUserSidLen, (PSID)*ppLogonInfo, pTokenUser->User.Sid);
				}
			}

			LocalFree(pTokenUser);
		}

		if(bRet)
		{
			TOKEN_ELEVATION_TYPE tokenElevationType;
			if(GetTokenInformation(hToken, TokenElevationType,
				&tokenElevationType, sizeof(tokenElevationType), &dwLength))
			{
				if(tokenElevationType == TokenElevationTypeFull)
				{
					TOKEN_LINKED_TOKEN tokenLinkedToken;
					if(GetTokenInformation(hToken, TokenLinkedToken,
						&tokenLinkedToken, sizeof(tokenLinkedToken), &dwLength))
					{
						CloseHandle(hToken);
						hToken = tokenLinkedToken.LinkedToken;
					}
				}
			}

			TOKEN_STATISTICS tokenStatistics;
			if(GetTokenInformation(hToken, TokenStatistics,
				&tokenStatistics, sizeof(tokenStatistics), &dwLength))
			{
				*(LUID *)(*ppLogonInfo + dwUserSidLen) = tokenStatistics.AuthenticationId;
			}
			else
			{
				bRet = FALSE;
			}
		}

		if(!bRet)
		{
			if(*ppLogonInfo != NULL)
			{
				LocalFree(*ppLogonInfo);
			}
		}

		CloseHandle(hToken);
	}

	return bRet;
}

BOOL GetUserUUID(LPWSTR *ppszUUID)
{
	BOOL bRet = FALSE;
	PBYTE pLogonInfo = NULL;
	//8c210750-6502-4a83-ae5c-88d86cb96f24
	const GUID NamespaceLogonInfo =
	{0x8c210750, 0x6502, 0x4a83, {0xae, 0x5c, 0x88, 0xd8, 0x6c, 0xb9, 0x6f, 0x24}};

	if(ppszUUID == NULL)
	{
		return FALSE;
	}

	if(GetLogonInfo(&pLogonInfo))
	{
		GUID uuid = GUID_NULL;
		if(GetUUID5(NamespaceLogonInfo, pLogonInfo, (DWORD)LocalSize(pLogonInfo), &uuid))
		{
			*ppszUUID = (LPWSTR)LocalAlloc(LPTR, 37 * sizeof(WCHAR));
			if(*ppszUUID != NULL)
			{
				_snwprintf_s(*ppszUUID, 37, _TRUNCATE,
					L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					uuid.Data1, uuid.Data2, uuid.Data3,
					uuid.Data4[0], uuid.Data4[1],
					uuid.Data4[2], uuid.Data4[3], uuid.Data4[4],
					uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);

				bRet = TRUE;
			}
		}

		LocalFree(pLogonInfo);
	}

	return bRet;
}

BOOL GetUserSid(LPWSTR *ppszUserSid)
{
	BOOL bRet = FALSE;
	HANDLE hToken = INVALID_HANDLE_VALUE;
	DWORD dwLength = 0;
	PTOKEN_USER pTokenUser = NULL;

	if(ppszUserSid == NULL)
	{
		return FALSE;
	}

	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
		pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);

		if(pTokenUser != NULL)
		{
			if(GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
			{
				bRet = ConvertSidToStringSidW(pTokenUser->User.Sid, ppszUserSid);
			}

			LocalFree(pTokenUser);
		}

		CloseHandle(hToken);
	}

	return bRet;
}

BOOL StartProcess(HMODULE hCurrentModule, LPCWSTR lpFileName)
{
	WCHAR path[MAX_PATH];
	PROCESS_INFORMATION pi;
	STARTUPINFOW si;

	if(GetModuleFileNameW(hCurrentModule, path, _countof(path)) != 0)
	{
		WCHAR *pdir = wcsrchr(path, L'\\');
		if(pdir != NULL)
		{
			*(pdir + 1) = L'\0';
			wcsncat_s(path, lpFileName, _TRUNCATE);
		}
	}

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	BOOL bRet = CreateProcessW(path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if(bRet)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return bRet;
}
