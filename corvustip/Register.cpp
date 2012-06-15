
#include "common.h"
#include "corvustip.h"

#define CLSID_STRLEN 38
#define TEXTSERVICE_MODEL   L"Apartment"
#define TEXTSERVICE_LANGID  MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT)
#ifndef _DEBUG
#define TEXTSERVICE_ICON_INDEX  0
#else
#define TEXTSERVICE_ICON_INDEX  1
#endif

static const WCHAR c_szInfoKeyPrefix[] = L"CLSID\\";
static const WCHAR c_szInProcSvr32[] = L"InProcServer32";
static const WCHAR c_szModelName[] = L"ThreadingModel";

// for Windows 8
// GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT {13A016DF-560B-46CD-947A-4C3AF1E0E35D}
static const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT =
{ 0x13A016DF, 0x560B, 0x46CD, { 0x94, 0x7A, 0x4C, 0x3A, 0xF1, 0xE0, 0xE3, 0x5D } };
//// GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT {25504FB4-7BAB-4BC1-9C69-CF81890F0EF5}
//static const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT =
//{ 0x25504FB4, 0x7BAB, 0x4BC1, { 0x9C, 0x69, 0xCF, 0x81, 0x89, 0x0F, 0x0E, 0xF5 } };

BOOL RegisterProfiles()
{
	ITfInputProcessorProfiles *pInputProcessProfiles;
	WCHAR fileName[MAX_PATH];
	HRESULT hr = E_FAIL;

	if(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
	                      IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles) != S_OK)
	{
		goto exit;
	}

	if(pInputProcessProfiles->Register(c_clsidTextService) != S_OK)
	{
		goto exit_r;
	}
	
	GetModuleFileNameW(g_hInst, fileName, _countof(fileName));
	
	hr = pInputProcessProfiles->AddLanguageProfile(c_clsidTextService, TEXTSERVICE_LANGID,
			c_guidProfile, TextServiceDesc, -1, fileName, -1, TEXTSERVICE_ICON_INDEX);

	if(!IsVersion6AndOver(g_ovi))
	{
		//XPで既定の言語に設定する為、デフォルトで存在するという仮定で
		//MS-IME2002の入力ロケール識別子「E0010411」をとりあえず使用。
		HKL hkl = LoadKeyboardLayoutW(L"E0010411", KLF_ACTIVATE);
		if(hkl != NULL)
		{
			pInputProcessProfiles->SubstituteKeyboardLayout(
				c_clsidTextService, TEXTSERVICE_LANGID, c_guidProfile, hkl);
		}
	}

exit_r:
	pInputProcessProfiles->Release();
	
exit:
	return (hr == S_OK);
}

void UnregisterProfiles()
{
	ITfInputProcessorProfiles *pInputProcessProfiles;

	if(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
	                      IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles) == S_OK)
	{
		pInputProcessProfiles->Unregister(c_clsidTextService);
		pInputProcessProfiles->Release();
	}
}

BOOL RegisterCategories()
{
	ITfCategoryMgr *pCategoryMgr;

	if(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER,
	                      IID_ITfCategoryMgr, (void**)&pCategoryMgr) == S_OK)
	{
		pCategoryMgr->RegisterCategory(c_clsidTextService, GUID_TFCAT_TIP_KEYBOARD, c_clsidTextService);
		pCategoryMgr->RegisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_SECUREMODE, c_clsidTextService);
		pCategoryMgr->RegisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_UIELEMENTENABLED, c_clsidTextService);
		pCategoryMgr->RegisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT, c_clsidTextService);
		pCategoryMgr->RegisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_COMLESS, c_clsidTextService);
		pCategoryMgr->RegisterCategory(c_clsidTextService, GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, c_clsidTextService);
		// for Windows 8
		if(IsVersion62AndOver(g_ovi))
		{
			pCategoryMgr->RegisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, c_clsidTextService);
			//pCategoryMgr->RegisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT, c_clsidTextService);
		}

		pCategoryMgr->Release();
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

void UnregisterCategories()
{
	ITfCategoryMgr *pCategoryMgr;

	if(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER,
	                      IID_ITfCategoryMgr, (void**)&pCategoryMgr) == S_OK)
	{
		pCategoryMgr->UnregisterCategory(c_clsidTextService, GUID_TFCAT_TIP_KEYBOARD, c_clsidTextService);
		pCategoryMgr->UnregisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_SECUREMODE, c_clsidTextService);
		pCategoryMgr->UnregisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_UIELEMENTENABLED, c_clsidTextService);
		pCategoryMgr->UnregisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT, c_clsidTextService);
		pCategoryMgr->UnregisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_COMLESS, c_clsidTextService);
		pCategoryMgr->UnregisterCategory(c_clsidTextService, GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, c_clsidTextService);
		// for Windows 8
		if(IsVersion62AndOver(g_ovi))
		{
			pCategoryMgr->UnregisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, c_clsidTextService);
			//pCategoryMgr->UnregisterCategory(c_clsidTextService, GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT, c_clsidTextService);
		}

		pCategoryMgr->Release();
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
