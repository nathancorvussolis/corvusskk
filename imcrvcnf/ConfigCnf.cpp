
#include "configxml.h"
#include "imcrvcnf.h"

LPCWSTR TextServiceDesc = TEXTSERVICE_DESC;

IXmlWriter *pXmlWriter;
IStream *pXmlFileStream;

WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// ファイルパス
WCHAR pathconfigxml[MAX_PATH];	//設定
WCHAR pathskkdic[MAX_PATH];		//取込SKK辞書
WCHAR pathskkidx[MAX_PATH];		//取込SKK辞書インデックス

void CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfigxml[0] = L'\0';
	pathskkdic[0] = L'\0';
	pathskkidx[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	_wmkdir(appdata);
	SetCurrentDirectoryW(appdata);

	_snwprintf_s(pathconfigxml, _TRUNCATE, L"%s%s", appdata, fnconfigxml);
	_snwprintf_s(pathskkdic, _TRUNCATE, L"%s%s", appdata, fnskkdic);
	_snwprintf_s(pathskkidx, _TRUNCATE, L"%s%s", appdata, fnskkidx);

	ZeroMemory(cnfmutexname, sizeof(cnfmutexname));

	LPWSTR pszDigest = NULL;

	if(GetSidMD5Digest(&pszDigest))
	{
		_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", CORVUSCNFMUTEX, pszDigest);

		LocalFree(pszDigest);
	}
}

BOOL SetFileDacl(LPCWSTR path)
{
	BOOL bRet = FALSE;
	WCHAR sddl[MAX_KRNLOBJNAME] = {L'\0'};
	PSECURITY_DESCRIPTOR psd = NULL;
	LPWSTR pszUserSid;

	if(GetUserSid(&pszUserSid))
	{
		// SDDL_ALL_APP_PACKAGES / SDDL_RESTRICTED_CODE / SDDL_LOCAL_SYSTEM / SDDL_BUILTIN_ADMINISTRATORS / User SID
		_snwprintf_s(sddl, _TRUNCATE, L"D:%s(A;;FR;;;RC)(A;;FA;;;SY)(A;;FA;;;BA)(A;;FA;;;%s)",
			(IsWindowsVersion62OrLater() ? L"(A;;FR;;;AC)" : L""), pszUserSid);
		LocalFree(pszUserSid);
	}

	if(ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl, SDDL_REVISION_1, &psd, NULL))
	{
		if(SetFileSecurityW(path, DACL_SECURITY_INFORMATION, psd))
		{
			bRet = TRUE;
		}
		LocalFree(psd);
	}

	return bRet;
}

int GetScaledSizeX(HWND hwnd, int size)
{
	HDC hdc = GetDC(hwnd);
	int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(hwnd, hdc);
	return MulDiv(size, dpi, 96);
}

void DrawSelectColor(HWND hDlg, int id, COLORREF col)
{
	RECT rect;
	HDC hdc;
	HWND hwnd;

	hwnd = GetDlgItem(hDlg, id);
	hdc = GetDC(hwnd);
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SetDCBrushColor(hdc, col);
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	GetClientRect(hwnd, &rect);
	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	ReleaseDC(hwnd, hdc);
}
