
#include "imcrvtip.h"
#include "input.h"

#define CLSID_STRLEN	38
#define TEXTSERVICE_MODEL	L"Apartment"
#define TEXTSERVICE_LANGID	MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN)
#define TEXTSERVICE_ICON_INDEX	0

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

	ITfInputProcessorProfileMgr *pInputProcessorProfileMgr = nullptr;
	hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pInputProcessorProfileMgr));
	if(SUCCEEDED(hr) & (pInputProcessorProfileMgr != nullptr))
	{
		ZeroMemory(fileName, sizeof(fileName));
		GetModuleFileNameW(g_hInst, fileName, _countof(fileName));

		hr = pInputProcessorProfileMgr->RegisterProfile(c_clsidTextService, TEXTSERVICE_LANGID, c_guidProfile,
			TextServiceDesc, (ULONG)wcslen(TextServiceDesc), fileName, (ULONG)wcslen(fileName), TEXTSERVICE_ICON_INDEX,
			nullptr, 0, TRUE, 0);

		SafeRelease(&pInputProcessorProfileMgr);
	}

	return SUCCEEDED(hr);
}

void UnregisterProfiles()
{
	HRESULT hr = E_FAIL;

	ITfInputProcessorProfileMgr *pInputProcessorProfileMgr = nullptr;
	hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pInputProcessorProfileMgr));
	if(SUCCEEDED(hr) && (pInputProcessorProfileMgr != nullptr))
	{
		hr = pInputProcessorProfileMgr->UnregisterProfile(c_clsidTextService, TEXTSERVICE_LANGID, c_guidProfile, TF_URP_ALLPROFILES);

		SafeRelease(&pInputProcessorProfileMgr);
	}
}

BOOL RegisterCategories()
{
	BOOL fRet = TRUE;
	HRESULT hr;

	ITfCategoryMgr *pCategoryMgr = nullptr;
	hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr));
	if(SUCCEEDED(hr) && (pCategoryMgr != nullptr))
	{
		for(int i = 0; i < _countof(c_guidCategory); i++)
		{
			hr = pCategoryMgr->RegisterCategory(c_clsidTextService, c_guidCategory[i], c_clsidTextService);

			if(FAILED(hr))
			{
				fRet = FALSE;
			}
		}

		// for Windows 8 or later
		if(IsWindowsVersion62OrLater())
		{
			for(int i = 0; i < _countof(c_guidCategory8); i++)
			{
				hr = pCategoryMgr->RegisterCategory(c_clsidTextService, c_guidCategory8[i], c_clsidTextService);

				if(FAILED(hr))
				{
					fRet = FALSE;
				}
			}
		}

		SafeRelease(&pCategoryMgr);
	}
	else
	{
		fRet = FALSE;
	}

	return fRet;
}

void UnregisterCategories()
{
	HRESULT hr;

	ITfCategoryMgr *pCategoryMgr = nullptr;
	hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr));
	if(SUCCEEDED(hr) && (pCategoryMgr != nullptr))
	{
		for(int i = 0; i < _countof(c_guidCategory); i++)
		{
			hr = pCategoryMgr->UnregisterCategory(c_clsidTextService, c_guidCategory[i], c_clsidTextService);
		}

		// for Windows 8 or later
		if(IsWindowsVersion62OrLater())
		{
			for(int i = 0; i < _countof(c_guidCategory8); i++)
			{
				hr = pCategoryMgr->UnregisterCategory(c_clsidTextService, c_guidCategory8[i], c_clsidTextService);
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

	if(RegCreateKeyExW(HKEY_CLASSES_ROOT, szInfoKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	if(RegSetValueExW(hKey, nullptr, 0, REG_SZ, (BYTE *)TextServiceDesc, (DWORD)(wcslen(TextServiceDesc) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
	{
		goto exit;
	}

	if(RegCreateKeyExW(hKey, c_szInProcSvr32, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hSubKey, nullptr) != ERROR_SUCCESS)
	{
		goto exit;
	}

	ZeroMemory(fileName, sizeof(fileName));
	GetModuleFileNameW(g_hInst, fileName, _countof(fileName));

	if(RegSetValueExW(hSubKey, nullptr, 0, REG_SZ, (BYTE *)fileName, (DWORD)(wcslen(fileName) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
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

BOOL InstallLayoutOrTipProfileList(DWORD dwFlags)
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

	//try delay load
	__try
	{
		return InstallLayoutOrTip(profilelist, dwFlags);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return FALSE;
}

BOOL EnableTextService()
{
	return InstallLayoutOrTipProfileList(ILOT_INSTALL);
}

void DisableTextService()
{
	InstallLayoutOrTipProfileList(ILOT_UNINSTALL);
}
