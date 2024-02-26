
#include "common.h"

#pragma comment(lib, "bcrypt.lib")

#define CCSUTF16 L",ccs=UTF-16LE"
#define CCSUTF8 L",ccs=UTF-8"
LPCWSTR modeRccsUTF16 = L"rt" CCSUTF16;
LPCWSTR modeWccsUTF16 = L"wt" CCSUTF16;
LPCWSTR modeRccsUTF8 = L"rt" CCSUTF8;
LPCWSTR modeWccsUTF8 = L"wt" CCSUTF8;
LPCWSTR modeRT = L"rt";
LPCWSTR modeRB = L"rb";
LPCWSTR modeWB = L"wb";

LPCWSTR fnconfigxml = L"config.xml";	//設定
LPCWSTR fnuserdic = L"userdict.txt";	//ユーザー辞書
LPCWSTR fnskkdic = L"skkdict.txt";		//取込SKK辞書
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

BOOL IsWindowsVersionOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion, DWORD dwBuildNumber)
{
	OSVERSIONINFOEXW osvi = {};
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = dwMajorVersion;
	osvi.dwMinorVersion = dwMinorVersion;
	osvi.dwBuildNumber = dwBuildNumber;

	DWORDLONG mask = 0;
	VER_SET_CONDITION(mask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(mask, VER_MINORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(mask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, mask);
}

BOOL GetDigest(LPCWSTR pszAlgId, CONST PBYTE data, DWORD datalen, PBYTE digest, DWORD digestlen)
{
	BOOL bRet = FALSE;

	if (data == nullptr || digest == nullptr)
	{
		return FALSE;
	}

	ZeroMemory(digest, digestlen);

	BCRYPT_ALG_HANDLE hAlg = nullptr;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, pszAlgId, nullptr, 0);
	if (BCRYPT_SUCCESS(status))
	{
		BCRYPT_HASH_HANDLE hHash = nullptr;
		status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
		if (BCRYPT_SUCCESS(status))
		{
			status = BCryptHashData(hHash, data, datalen, 0);
			if (BCRYPT_SUCCESS(status))
			{
				status = BCryptFinishHash(hHash, digest, digestlen, 0);
				if (BCRYPT_SUCCESS(status))
				{
					bRet = TRUE;
				}
			}
			BCryptDestroyHash(hHash);
		}
		BCryptCloseAlgorithmProvider(hAlg, 0);
	}

	return bRet;
}

BOOL IsLittleEndian()
{
	ULONG n = 1;
	return (*(UCHAR *)&n == 1);
}

ULONG bswap_l(ULONG l)
{
	return (IsLittleEndian() ? _byteswap_ulong(l) : l);
}

USHORT bswap_s(USHORT s)
{
	return (IsLittleEndian() ? _byteswap_ushort(s) : s);
}

BOOL GetUUID5(REFGUID rguid, CONST PBYTE name, DWORD namelen, LPGUID puuid)
{
	BOOL bRet = FALSE;
	GUID lguid = rguid;

	LPCWSTR ALGORITHM_ID = BCRYPT_SHA1_ALGORITHM;
	CONST DWORD DIGEST_LENGTH = 20;
	CONST USHORT MASK_VERSION = 0x5000;

	if (name == nullptr || namelen == 0 || puuid == nullptr)
	{
		return FALSE;
	}

	PBYTE pMessage = (PBYTE)LocalAlloc(LPTR, sizeof(lguid) + namelen);
	if (pMessage != nullptr)
	{
		//network byte order
		lguid.Data1 = bswap_l(lguid.Data1);
		lguid.Data2 = bswap_s(lguid.Data2);
		lguid.Data3 = bswap_s(lguid.Data3);

		memcpy_s(pMessage, sizeof(lguid), &lguid, sizeof(lguid));
		memcpy_s(pMessage + sizeof(lguid), namelen, name, namelen);

		BYTE digest[DIGEST_LENGTH] = {};
		if (GetDigest(ALGORITHM_ID, pMessage, (DWORD)LocalSize(pMessage), digest, (DWORD)sizeof(digest)))
		{
			GUID dguid = GUID_NULL;
			dguid.Data1 = *(ULONG *)&digest[0];
			dguid.Data2 = *(USHORT *)&digest[4];
			dguid.Data3 = *(USHORT *)&digest[6];
			*(ULONGLONG *)dguid.Data4 = *(ULONGLONG *)&digest[8];

			//host byte order
			dguid.Data1 = bswap_l(dguid.Data1);
			dguid.Data2 = bswap_s(dguid.Data2);
			dguid.Data3 = bswap_s(dguid.Data3);
			//version
			dguid.Data3 &= 0x0FFF;
			dguid.Data3 |= MASK_VERSION;
			//variant
			dguid.Data4[0] &= 0x3F;
			dguid.Data4[0] |= 0x80;

			*puuid = dguid;

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

	if (ppLogonInfo == nullptr)
	{
		return FALSE;
	}

	*ppLogonInfo = nullptr;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLength);
		PTOKEN_USER pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);

		if (pTokenUser != nullptr)
		{
			if (GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
			{
				dwUserSidLen = GetLengthSid(pTokenUser->User.Sid);
				*ppLogonInfo = (PBYTE)LocalAlloc(LPTR, dwUserSidLen + sizeof(LUID));
				if (*ppLogonInfo != nullptr)
				{
					bRet = CopySid(dwUserSidLen, (PSID)*ppLogonInfo, pTokenUser->User.Sid);
				}
			}

			LocalFree(pTokenUser);
		}

		if (bRet)
		{
			TOKEN_ELEVATION_TYPE tokenElevationType = TokenElevationTypeDefault;
			if (GetTokenInformation(hToken, TokenElevationType,
				&tokenElevationType, sizeof(tokenElevationType), &dwLength))
			{
				if (tokenElevationType == TokenElevationTypeFull)
				{
					TOKEN_LINKED_TOKEN tokenLinkedToken = {};
					if (GetTokenInformation(hToken, TokenLinkedToken,
						&tokenLinkedToken, sizeof(tokenLinkedToken), &dwLength))
					{
						CloseHandle(hToken);
						hToken = tokenLinkedToken.LinkedToken;
					}
				}
			}

			TOKEN_STATISTICS tokenStatistics = {};
			if (GetTokenInformation(hToken, TokenStatistics,
				&tokenStatistics, sizeof(tokenStatistics), &dwLength))
			{
				*(LUID *)(*ppLogonInfo + dwUserSidLen) = tokenStatistics.AuthenticationId;
			}
			else
			{
				bRet = FALSE;
			}
		}

		if (!bRet)
		{
			if (*ppLogonInfo != nullptr)
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
	PBYTE pLogonInfo = nullptr;
	//8c210750-6502-4a83-ae5c-88d86cb96f24
	const GUID NamespaceLogonInfo =
	{0x8c210750, 0x6502, 0x4a83, {0xae, 0x5c, 0x88, 0xd8, 0x6c, 0xb9, 0x6f, 0x24}};

	if (ppszUUID == nullptr)
	{
		return FALSE;
	}

	if (GetLogonInfo(&pLogonInfo))
	{
		GUID uuid = GUID_NULL;
		if (GetUUID5(NamespaceLogonInfo, pLogonInfo, (DWORD)LocalSize(pLogonInfo), &uuid))
		{
			*ppszUUID = (LPWSTR)LocalAlloc(LPTR, 37 * sizeof(WCHAR));
			if (*ppszUUID != nullptr)
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
	PTOKEN_USER pTokenUser = nullptr;

	if (ppszUserSid == nullptr)
	{
		return FALSE;
	}

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLength);
		pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);

		if (pTokenUser != nullptr)
		{
			if (GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
			{
				bRet = ConvertSidToStringSidW(pTokenUser->User.Sid, ppszUserSid);
			}

			LocalFree(pTokenUser);
		}

		CloseHandle(hToken);
	}

	return bRet;
}

BOOL StartProcess(HMODULE hCurrentModule, LPCWSTR lpFileName, LPCWSTR lpArgs)
{
	WCHAR path[MAX_PATH] = {};

	if (GetModuleFileNameW(hCurrentModule, path, _countof(path)) != 0)
	{
		WCHAR *pdir = wcsrchr(path, L'\\');
		if (pdir != nullptr)
		{
			*(pdir + 1) = L'\0';
			wcsncat_s(path, lpFileName, _TRUNCATE);
		}
	}

	const int cmdlen = 8192;
	WCHAR *commandline = (PWCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * cmdlen);
	if (commandline == nullptr)
	{
		return FALSE;
	}

	PROCESS_INFORMATION pi = {};
	STARTUPINFOW si = {};
	si.cb = sizeof(si);

	_snwprintf_s(commandline, cmdlen, _TRUNCATE, L"\"%s\"%s%s",
		path,
		(lpArgs == nullptr) ? L"" : L"\x20",
		(lpArgs == nullptr) ? L"" : lpArgs);

	BOOL bRet = CreateProcessW(nullptr, commandline, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
	if (bRet)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	HeapFree(GetProcessHeap(), 0, commandline);

	return bRet;
}
