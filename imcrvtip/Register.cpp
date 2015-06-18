
#include "imcrvtip.h"

#define CLSID_STRLEN 38
#define TEXTSERVICE_MODEL   L"Apartment"
#define TEXTSERVICE_LANGID  MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT)
#ifndef _DEBUG
#define TEXTSERVICE_ICON_INDEX  0	// see resource script file
#else
#define TEXTSERVICE_ICON_INDEX  1
#endif

static const WCHAR c_szInfoKeyPrefix[] = L"CLSID\\";
static const WCHAR c_szInProcSvr32[] = L"InProcServer32";
static const WCHAR c_szModelName[] = L"ThreadingModel";

static const GUID c_guidCategory[] =
{
	GUID_TFCAT_TIP_KEYBOARD,
	GUID_TFCAT_TIPCAP_SECUREMODE,
	GUID_TFCAT_TIPCAP_UIELEMENTENABLED,
	GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
	GUID_TFCAT_TIPCAP_COMLESS,
	GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER
};
// for Windows 8 or later
static const GUID c_guidCategory8[] =
{
	GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,
	GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT
};

BOOL RegisterProfiles()
{
	HRESULT hr = E_FAIL;
	WCHAR fileName[MAX_PATH];

	ITfInputProcessorProfileMgr *pInputProcessorProfilesMgr;
	if(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pInputProcessorProfilesMgr)) == S_OK)
	{
		GetModuleFileNameW(g_hInst, fileName, _countof(fileName));

		hr = pInputProcessorProfilesMgr->RegisterProfile(c_clsidTextService, TEXTSERVICE_LANGID, c_guidProfile,
			TextServiceDesc, (ULONG)wcslen(TextServiceDesc), fileName, (ULONG)wcslen(fileName),
			TEXTSERVICE_ICON_INDEX, NULL, 0, TRUE, 0);

		SafeRelease(&pInputProcessorProfilesMgr);
	}

	return (hr == S_OK);
}

void UnregisterProfiles()
{
	HRESULT hr = E_FAIL;

	ITfInputProcessorProfileMgr *pInputProcessorProfilesMgr;
	if(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pInputProcessorProfilesMgr)) == S_OK)
	{
		hr = pInputProcessorProfilesMgr->UnregisterProfile(c_clsidTextService, TEXTSERVICE_LANGID, c_guidProfile, TF_URP_ALLPROFILES);

		SafeRelease(&pInputProcessorProfilesMgr);
	}
}

BOOL RegisterCategories()
{
	int i;

	ITfCategoryMgr *pCategoryMgr;
	if(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr)) == S_OK)
	{
		for(i = 0; i < _countof(c_guidCategory); i++)
		{
			pCategoryMgr->RegisterCategory(c_clsidTextService, c_guidCategory[i], c_clsidTextService);
		}
		// for Windows 8 or later
		if(IsWindowsVersion62OrLater())
		{
			for(i = 0; i < _countof(c_guidCategory8); i++)
			{
				pCategoryMgr->RegisterCategory(c_clsidTextService, c_guidCategory8[i], c_clsidTextService);
			}
		}

		SafeRelease(&pCategoryMgr);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

void UnregisterCategories()
{
	int i;

	ITfCategoryMgr *pCategoryMgr;
	if(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr)) == S_OK)
	{
		for(i = 0; i < _countof(c_guidCategory); i++)
		{
			pCategoryMgr->UnregisterCategory(c_clsidTextService, c_guidCategory[i], c_clsidTextService);
		}
		// for Windows 8 or later
		if(IsWindowsVersion62OrLater())
		{
			for(i = 0; i < _countof(c_guidCategory8); i++)
			{
				pCategoryMgr->UnregisterCategory(c_clsidTextService, c_guidCategory8[i], c_clsidTextService);
			}
		}

		SafeRelease(&pCategoryMgr);
	}
}

BOOL RegisterServer()
{
	WCHAR szInfoKey[_countof(c_szInfoKeyPrefix) + CLSID_STRLEN];
	HKEY hKey;
	HKEY hSubKey;
	BOOL fRet = FALSE;
	WCHAR fileName[MAX_PATH];

	if(StringFromGUID2(c_clsidTextService, szInfoKey + _countof(c_szInfoKeyPrefix) - 1, CLSID_STRLEN + 1) == 0)
	{
		return FALSE;
	}

	wmemcpy_s(szInfoKey, _countof(szInfoKey), c_szInfoKeyPrefix, _countof(c_szInfoKeyPrefix) - 1);

	if(RegCreateKeyExW(HKEY_CLASSES_ROOT, szInfoKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	if(RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE *)TextServiceDesc, (DWORD)(wcslen(TextServiceDesc) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
	{
		goto exit;
	}

	if(RegCreateKeyExW(hKey, c_szInProcSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL) != ERROR_SUCCESS)
	{
		goto exit;
	}

	GetModuleFileNameW(g_hInst, fileName, _countof(fileName));

	if(RegSetValueExW(hSubKey, NULL, 0, REG_SZ, (BYTE *)fileName, (DWORD)(wcslen(fileName) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
	{
		goto exit_sub;
	}

	if(RegSetValueExW(hSubKey, c_szModelName, 0, REG_SZ, (BYTE *)TEXTSERVICE_MODEL, (DWORD)(wcslen(TEXTSERVICE_MODEL) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
	{
		goto exit_sub;
	}

	fRet = TRUE;

exit_sub:
	RegCloseKey(hSubKey);

exit:
	RegCloseKey(hKey);

	return fRet;
}

void UnregisterServer()
{
	WCHAR szInfoKey[_countof(c_szInfoKeyPrefix) + CLSID_STRLEN];

	if(StringFromGUID2(c_clsidTextService, szInfoKey + _countof(c_szInfoKeyPrefix) - 1, CLSID_STRLEN + 1) == 0)
	{
		return;
	}

	wmemcpy_s(szInfoKey, _countof(szInfoKey), c_szInfoKeyPrefix, _countof(c_szInfoKeyPrefix) - 1);

	SHDeleteKeyW(HKEY_CLASSES_ROOT, szInfoKey);
}

BOOL InstallLayoutOrTip(DWORD dwFlags)
{
	typedef BOOL (WINAPI *PTF_INSTALLLAYOUTORTIP)(LPCWSTR psz, DWORD dwFlags);

	BOOL bRet = FALSE;
	WCHAR fileNameInputDLL[MAX_PATH];

	if(SHGetFolderPathW(NULL, CSIDL_SYSTEM, NULL, SHGFP_TYPE_CURRENT, fileNameInputDLL) != S_OK)
	{
		return FALSE;
	}

	wcsncat_s(fileNameInputDLL, L"\\input.dll", _TRUNCATE);

	HMODULE hInputDLL = LoadLibraryW(fileNameInputDLL);

	if(hInputDLL != NULL)
	{
		PTF_INSTALLLAYOUTORTIP pfnInstallLayoutOrTip =
			(PTF_INSTALLLAYOUTORTIP)GetProcAddress(hInputDLL, "InstallLayoutOrTip");

		if(pfnInstallLayoutOrTip != NULL)
		{
			WCHAR clsid[CLSID_STRLEN + 1];
			WCHAR guidprofile[CLSID_STRLEN + 1];
			WCHAR profilelist[7 + CLSID_STRLEN * 2 + 1];

			if(StringFromGUID2(c_clsidTextService, clsid, _countof(clsid)) == 0)
			{
				return FALSE;
			}

			if(StringFromGUID2(c_guidProfile, guidprofile, _countof(guidprofile)) == 0)
			{
				return FALSE;
			}

			_snwprintf_s(profilelist, _TRUNCATE, L"0x%04X:%s%s", TEXTSERVICE_LANGID, clsid, guidprofile);

			bRet = (*pfnInstallLayoutOrTip)(profilelist, dwFlags);
		}

		FreeLibrary(hInputDLL);
	}

	return bRet;
}
