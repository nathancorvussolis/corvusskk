
#include "configxml.h"
#include "imcrvcnf.h"

LPCWSTR TextServiceDesc = TEXTSERVICE_DESC;

IXmlWriter *pXmlWriter;
IStream *pXmlFileStream;

WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// ファイルパス
WCHAR pathconfigxml[MAX_PATH];	//設定
WCHAR pathskkdic[MAX_PATH];		//取込SKK辞書

void CreateConfigPath()
{
	PWSTR appdatafolder = nullptr;

	ZeroMemory(pathconfigxml, sizeof(pathconfigxml));
	ZeroMemory(pathskkdic, sizeof(pathskkdic));

	if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DONT_VERIFY, nullptr, &appdatafolder)))
	{
		WCHAR appdir[MAX_PATH];

		_snwprintf_s(appdir, _TRUNCATE, L"%s\\%s", appdatafolder, TextServiceDesc);

		CoTaskMemFree(appdatafolder);

		CreateDirectoryW(appdir, nullptr);
		SetCurrentDirectoryW(appdir);

		_snwprintf_s(pathconfigxml, _TRUNCATE, L"%s\\%s", appdir, fnconfigxml);
		_snwprintf_s(pathskkdic, _TRUNCATE, L"%s\\%s", appdir, fnskkdic);
	}
}

void CreateIpcName()
{
	ZeroMemory(cnfmutexname, sizeof(cnfmutexname));

	LPWSTR pszUserUUID = nullptr;

	if(GetUserUUID(&pszUserUUID))
	{
		_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", IMCRVCNFMUTEX, pszUserUUID);

		LocalFree(pszUserUUID);
	}
}

BOOL SetFileDacl(LPCWSTR path)
{
	BOOL bRet = FALSE;
	WCHAR sddl[MAX_KRNLOBJNAME] = {L'\0'};
	PSECURITY_DESCRIPTOR psd = nullptr;
	LPWSTR pszUserSid;

	if(GetUserSid(&pszUserSid))
	{
		// SDDL_ALL_APP_PACKAGES / SDDL_RESTRICTED_CODE / SDDL_LOCAL_SYSTEM / SDDL_BUILTIN_ADMINISTRATORS / User SID
		_snwprintf_s(sddl, _TRUNCATE, L"D:%s(A;;FR;;;RC)(A;;FA;;;SY)(A;;FA;;;BA)(A;;FA;;;%s)",
			(IsWindowsVersion62OrLater() ? L"(A;;FR;;;AC)" : L""), pszUserSid);

		LocalFree(pszUserSid);
	}

	if(ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl, SDDL_REVISION_1, &psd, nullptr))
	{
		if(SetFileSecurityW(path, DACL_SECURITY_INFORMATION, psd))
		{
			bRet = TRUE;
		}
		LocalFree(psd);
	}

	return bRet;
}

int GetDpi(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(hwnd, hdc);

	if(IsWindowsVersion100RS2OrLater())
	{
		__try
		{
			HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
			UINT dpiX, dpiY;
			if(SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
			{
				dpi = (int)dpiX;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	return dpi;
}

int GetScaledSizeX(HWND hwnd, int size)
{
	return MulDiv(size, GetDpi(hwnd), 96);
}

int GetFontHeight(HWND hwnd, int size)
{
	return -MulDiv(size, GetDpi(hwnd), 72);
}

void DrawSelectColor(HWND hDlg, int id, COLORREF col)
{
	RECT rect;

	HWND hwnd = GetDlgItem(hDlg, id);
	HDC hdc = GetDC(hwnd);

	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SetDCBrushColor(hdc, col);
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	GetClientRect(hwnd, &rect);
	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	ReleaseDC(hwnd, hdc);
}
