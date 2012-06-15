
#include "common.h"


#define CCSUNICODE L",ccs=UNICODE"
LPCWSTR RccsUNICODE = L"r" CCSUNICODE;
LPCWSTR WccsUNICODE = L"w" CCSUNICODE;

LPCWSTR fnconfigxml = L"config.xml";
LPCWSTR fnuserdicxml = L"userdic.xml";
LPCWSTR fnskkcvdicxml = L"skkcvdic.xml";
LPCWSTR fnskkcvdicidx = L"skkcvdic.idx";

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
