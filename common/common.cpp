
#include "common.h"

#define CCSUNICODE L",ccs=UNICODE"
LPCWSTR RccsUNICODE = L"r" CCSUNICODE;
LPCWSTR WccsUNICODE = L"w" CCSUNICODE;

LPCWSTR fnconfigxml = L"config.xml";
LPCWSTR fnuserdicxml = L"userdic.xml";
LPCWSTR fnskkcvdicxml = L"skkcvdic.xml";
LPCWSTR fnskkcvdicidx = L"skkcvdic.idx";

// for Windows 8
#if 1
const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT =
{ 0x13A016DF, 0x560B, 0x46CD, { 0x94, 0x7A, 0x4C, 0x3A, 0xF1, 0xE0, 0xE3, 0x5D } };
const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT =
{ 0x25504FB4, 0x7BAB, 0x4BC1, { 0x9C, 0x69, 0xCF, 0x81, 0x89, 0x0F, 0x0E, 0xF5 } };
const GUID GUID_COMPARTMENT_TKB_THIRDPARTYIME_MODE_ONOFF =
{ 0x4BAA5390, 0x280B, 0x4817, { 0xBB, 0x8C, 0x62, 0xFC, 0xEB, 0x36, 0xD2, 0x0F } };
const GUID GUID_LBI_INPUTMODE =
{ 0x2C77A81E, 0x41CC, 0x4178, { 0xA3, 0xA7, 0x5F, 0x8A, 0x98, 0x75, 0x68, 0xE6 } };
const GUID GUID_INTEGRATIONSTYLE_SEARCHBOX =
{ 0xe6d1bd11, 0x82f7, 0x4903, { 0xae, 0x21, 0x1a, 0x63, 0x97, 0xcd, 0xe2, 0xeb } };
#endif

void debugout(LPCWSTR format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	INT len = _vscwprintf(format, argptr) + 1;
	WCHAR* str = (WCHAR*)alloca(len * sizeof(WCHAR));
	vswprintf_s(str, len, format, argptr);
	va_end(argptr);
	OutputDebugStringW(str);
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

BOOL IsVersion6AndOver(OSVERSIONINFOW ovi)
{
	BOOL bRet = FALSE;
	if(ovi.dwMajorVersion >= 6)
	{
		bRet = TRUE;
	}
	return bRet;
}

BOOL IsVersion62AndOver(OSVERSIONINFOW ovi)
{
	BOOL bRet = FALSE;
	if((ovi.dwMajorVersion == 6 && ovi.dwMinorVersion >= 2) || ovi.dwMajorVersion > 6)
	{
		bRet = TRUE;
	}
	return bRet;
}

BOOL SetFileDaclAC(LPCWSTR path)
{
	BOOL bRet = FALSE;
	WCHAR krnlobjsddl[MAX_KRNLOBJNAME] = {L'\0'};
	PSECURITY_DESCRIPTOR pSD = NULL;
	DWORD dwLength;
	LPWSTR pszSD;
	LPCWSTR pcszDaclAC = L"(A;;FA;;;AC)"; // for Windows 8 SDDL_ALL_APP_PACKAGES

	GetFileSecurityW(path, DACL_SECURITY_INFORMATION, NULL, 0, &dwLength);
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwLength);
	if(GetFileSecurityW(path, DACL_SECURITY_INFORMATION, pSD, dwLength, &dwLength))
	{
		if(ConvertSecurityDescriptorToStringSecurityDescriptorW(pSD, SDDL_REVISION_1,
			DACL_SECURITY_INFORMATION, &pszSD, NULL))
		{
			wcsncpy_s(krnlobjsddl, pszSD, _TRUNCATE);
			LocalFree(pszSD);
		}
	}
	LocalFree(pSD);
	
	if(wcslen(krnlobjsddl) == 0 || wcsstr(krnlobjsddl, pcszDaclAC) != NULL)
	{
		goto end;
	}

	wcsncat_s(krnlobjsddl, pcszDaclAC, _TRUNCATE);

	if(ConvertStringSecurityDescriptorToSecurityDescriptorW(krnlobjsddl, SDDL_REVISION_1, &pSD, NULL))
	{
		if(SetFileSecurityW(path, DACL_SECURITY_INFORMATION, pSD))
		{
			bRet = TRUE;
		}
		LocalFree(pSD);
	}

end:
	return bRet;
}
