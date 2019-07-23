
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

LPCWSTR TextServiceDesc = TEXTSERVICE_DESC;
WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス
WCHAR cnfcanceldiceventname[MAX_KRNLOBJNAME];	//辞書取込キャンセルイベント
WCHAR pathconfigxml[MAX_PATH];	//設定
WCHAR pathskkdic[MAX_PATH];		//取込SKK辞書

void CreateConfigPath()
{
	PWSTR appdatafolder = nullptr;

	ZeroMemory(pathconfigxml, sizeof(pathconfigxml));
	ZeroMemory(pathskkdic, sizeof(pathskkdic));

	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DONT_VERIFY, nullptr, &appdatafolder)))
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
	ZeroMemory(cnfcanceldiceventname, sizeof(cnfcanceldiceventname));

	LPWSTR pszUserUUID = nullptr;

	if (GetUserUUID(&pszUserUUID))
	{
		_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", IMCRVCNFMUTEX, pszUserUUID);
		_snwprintf_s(cnfcanceldiceventname, _TRUNCATE, L"%s%s", IMCRVKRNLOBJ L"cnf-cancel-dic-", pszUserUUID);

		LocalFree(pszUserUUID);
	}
}

BOOL SetFileDacl(LPWSTR path)
{
	BOOL bRet = FALSE;
	WCHAR sddl[MAX_KRNLOBJNAME] = {};
	PSECURITY_DESCRIPTOR psd = nullptr;
	LPWSTR pszUserSid;

	if (GetUserSid(&pszUserSid))
	{
		// SDDL_ALL_APP_PACKAGES / SDDL_RESTRICTED_CODE / SDDL_LOCAL_SYSTEM / SDDL_BUILTIN_ADMINISTRATORS / User SID
		_snwprintf_s(sddl, _TRUNCATE, L"D:%s(A;;FR;;;RC)(A;;FA;;;SY)(A;;FA;;;BA)(A;;FA;;;%s)",
			(IsWindowsVersion62OrLater() ? L"(A;;FR;;;AC)" : L""), pszUserSid);

		LocalFree(pszUserSid);
	}

	if (ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl, SDDL_REVISION_1, &psd, nullptr))
	{
		BOOL bDaclPresent = FALSE;
		PACL pDacl = nullptr;
		BOOL bDaclDefaulted = FALSE;
		if (GetSecurityDescriptorDacl(psd, &bDaclPresent, &pDacl, &bDaclDefaulted))
		{
			if (SetNamedSecurityInfoW(path, SE_FILE_OBJECT,
				DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
				nullptr, nullptr, pDacl, nullptr) == ERROR_SUCCESS)
			{
				bRet = TRUE;
			}
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

	// Windows 10 ver.1703 supports Per-Monitor DPI Awareness V2
	if (IsWindowsVersion100RS2OrLater())
	{
		// try delay load api-ms-win-shcore-scaling-l1-1-1.dll
		__try
		{
			HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
			UINT dpiX, dpiY;
			if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
			{
				dpi = (int)dpiX;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	return dpi;
}

int GetScaledSizeX(HWND hwnd, int size)
{
	return MulDiv(size, GetDpi(hwnd), C_USER_DEFAULT_SCREEN_DPI);
}

int GetFontHeight(HWND hwnd, int size)
{
	return -MulDiv(size, GetDpi(hwnd), C_FONT_LOGICAL_HEIGHT_PPI);
}

void DrawSelectColor(HWND hDlg, int id, COLORREF col)
{
	HWND hwnd = GetDlgItem(hDlg, id);
	HDC hdc = GetDC(hwnd);

	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SetDCBrushColor(hdc, col);
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	RECT r = {};
	GetClientRect(hwnd, &r);
	Rectangle(hdc, r.left, r.top, r.right, r.bottom);

	ReleaseDC(hwnd, hdc);
}

void LoadCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault)
{
	std::wstring strxmlval;

	ReadValue(pathconfigxml, lpAppName, lpKeyName, strxmlval, lpDefault);
	CheckDlgButton(hDlg, nIDDlgItem, (_wtoi(strxmlval.c_str()) == TRUE ? BST_CHECKED : BST_UNCHECKED));
}

void SaveCheckButton(IXmlWriter *pWriter, HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName)
{
	WCHAR num[2];

	num[0] = L'0' + IsDlgButtonChecked(hDlg, nIDDlgItem);
	num[1] = L'\0';
	WriterKey(pWriter, lpKeyName, num);
}

BOOL SaveConfigXml(HWND hPropSheetDlg)
{
	CComPtr<IXmlWriter> pWriter;
	CComPtr<IStream> pFileStream;

	HRESULT hr = WriterInit(pathconfigxml, &pWriter, &pFileStream);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//skk
	{
		WriterStartElement(pWriter, TagRoot);

		//dictionary
		{
			WriterStartSection(pWriter, SectionDictionary);

			SaveDictionary(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_DICTIONARY));

			WriterEndSection(pWriter);
		}

		//server
		{
			WriterStartSection(pWriter, SectionServer);

			SaveServer(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_DICTIONARY));

			WriterEndSection(pWriter);
		}

		//behavior
		{
			WriterStartSection(pWriter, SectionBehavior);

			SaveBehavior1(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_BEHAVIOR1));

			SaveBehavior2(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_BEHAVIOR2));

			WriterEndSection(pWriter);
		}

		//font
		{
			WriterStartSection(pWriter, SectionFont);

			SaveFont(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_DISPLAY1));

			WriterEndSection(pWriter);
		}

		//display
		{
			WriterStartSection(pWriter, SectionDisplay);

			SaveDisplay1(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_DISPLAY1));

			SaveDisplay2(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_DISPLAY2));

			WriterEndSection(pWriter);
		}

		//displayAttr
		{
			WriterStartSection(pWriter, SectionDisplayAttr);

			SaveDisplayAttr1(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_DISPLAYATTR1));

			SaveDisplayAttr2(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_DISPLAYATTR2));

			WriterEndSection(pWriter);
		}

		//selKey
		{
			WriterStartSection(pWriter, SectionSelKey);

			SaveSelKey(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_SELKEY));

			WriterEndSection(pWriter);
		}

		//preservedKeyon
		{
			WriterStartSection(pWriter, SectionPreservedKeyON);

			SavePreservedKeyON(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_PRSRVKEY));

			WriterEndSection(pWriter);
		}

		//preservedKeyoff
		{
			WriterStartSection(pWriter, SectionPreservedKeyOFF);

			SavePreservedKeyOFF(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_PRSRVKEY));

			WriterEndSection(pWriter);
		}

		//keymap
		{
			WriterStartSection(pWriter, SectionKeyMap);

			SaveCKeyMap(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_KEYMAP1));

			WriterEndSection(pWriter);
		}

		//vkeymap
		{
			WriterStartSection(pWriter, SectionVKeyMap);

			SaveVKeyMap(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_KEYMAP2));

			WriterEndSection(pWriter);
		}

		//convpoint
		{
			WriterStartSection(pWriter, SectionConvPoint);

			SaveConvPoint(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_CONVPOINT));

			WriterEndSection(pWriter);
		}

		//kana
		{
			WriterStartSection(pWriter, SectionKana);

			SaveKana(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_KANATBL));

			WriterEndSection(pWriter);
		}

		//jlatin
		{
			WriterStartSection(pWriter, SectionJLatin);

			SaveJLatin(pWriter, PROPSHEET_IDTOHWND(hPropSheetDlg, IDD_DIALOG_JLATTBL));

			WriterEndSection(pWriter);
		}

		WriterEndElement(pWriter);
	}

	WriterNewLine(pWriter);

	WriterFinal(pWriter);

	BOOL ret = SetFileDacl(pathconfigxml);
	if (ret == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}
