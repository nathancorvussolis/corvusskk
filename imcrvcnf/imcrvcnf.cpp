
#include "imcrvcnf.h"
#include "resource.h"

#define CONFIG_RECOVERY_OPTION L"/rc"

HINSTANCE hInst;
HANDLE hMutex;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX icex;

	_wsetlocale(LC_ALL, L"ja-JP");

	hInst = hInstance;

	CreateConfigPath();
	CreateIpcName();

	hMutex = CreateMutexW(nullptr, FALSE, cnfmutexname);
	if (hMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (hMutex != nullptr)
		{
			CloseHandle(hMutex);
		}
		return 0;
	}

	CoInitialize(nullptr);

	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icex);

	// "<config path>"
	//  or
	// /rc "<config path>"
	bool rcMode = false;
	LPWSTR fileArg = nullptr;
	int numArgs = 0;
	LPWSTR *pArgs = CommandLineToArgvW(GetCommandLineW(), &numArgs);
	if (pArgs != nullptr && numArgs >= 2)
	{
		for (int i = 1; i < numArgs; i++)
		{
			if (wcscmp(pArgs[i], CONFIG_RECOVERY_OPTION) == 0)
			{
				rcMode = true;
			}
			else if (PathFileExistsW(pArgs[i]) && !PathIsDirectoryW(pArgs[i]))
			{
				fileArg = pArgs[i];
				wcsncpy_s(pathconfigxml, fileArg, _TRUNCATE);
				break;
			}
		}
	}

	CreateProperty();

	if (rcMode && fileArg != nullptr)
	{
		DeleteFileW(fileArg);
	}

	if (pArgs != nullptr)
	{
		LocalFree(pArgs);
	}

	CoUninitialize();

	if (hMutex != nullptr)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}

	return 0;
}

void CreateProperty()
{
	const struct {
		int id;
		DLGPROC DlgProc;
	} DlgPages[] = {
		{IDD_DIALOG_DICTIONARY1,	DlgProcDictionary1},
		{IDD_DIALOG_DICTIONARY2,	DlgProcDictionary2},
		{IDD_DIALOG_BEHAVIOR1,		DlgProcBehavior1},
		{IDD_DIALOG_BEHAVIOR2,		DlgProcBehavior2},
		{IDD_DIALOG_DISPLAY1,		DlgProcDisplay1},
		{IDD_DIALOG_DISPLAY2,		DlgProcDisplay2},
		{IDD_DIALOG_DISPLAYATTR1,	DlgProcDisplayAttr1},
		{IDD_DIALOG_DISPLAYATTR2,	DlgProcDisplayAttr2},
		{IDD_DIALOG_SELKEY,			DlgProcSelKey},
		{IDD_DIALOG_PRSRVKEY,		DlgProcPreservedKey},
		{IDD_DIALOG_KEYMAP1,		DlgProcKeyMap1},
		{IDD_DIALOG_KEYMAP2,		DlgProcKeyMap2},
		{IDD_DIALOG_CONVPOINT,		DlgProcConvPoint},
		{IDD_DIALOG_KANATBL,		DlgProcKana},
		{IDD_DIALOG_JLATTBL,		DlgProcJLatin}
	};

	PROPSHEETPAGEW psp[_countof(DlgPages)] = {};
	for (int i = 0; i < _countof(psp); i++)
	{
		psp[i].dwSize = sizeof(psp[i]);
		psp[i].dwFlags = PSP_PREMATURE;
		psp[i].hInstance = hInst;
		psp[i].pszTemplate = MAKEINTRESOURCE(DlgPages[i].id);
		psp[i].pfnDlgProc = DlgPages[i].DlgProc;
	}

	PROPSHEETHEADERW psh = {};
	psh.dwSize = sizeof(psh);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP | PSH_USECALLBACK;
	psh.hwndParent = nullptr;
	psh.hInstance = hInst;
	psh.pszCaption = TEXTSERVICE_DESC L" ver. " TEXTSERVICE_VER;
	psh.nPages = _countof(psp);
	psh.nStartPage = 0;
	psh.ppsp = psp;
	psh.pfnCallback = PropSheetProc;

	PropertySheetW(&psh);

	return;
}

int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
	static HWND hwndInit = nullptr;

	switch (uMsg)
	{
	case PSCB_INITIALIZED:
		hwndInit = hwndDlg;
		//imcrvmgr.exeから実行されるときimcrvtip.dllで
		//AllowSetForegroundWindow関数が実行済みのはず
		SetForegroundWindow(hwndDlg);
		break;
	case PSCB_PRECREATE:
		break;
	case PSCB_BUTTONPRESSED:
		if (lParam == PSBTN_OK || lParam == PSBTN_APPLYNOW)
		{
			// hwndDlg is NULL on Vista
			if (hwndDlg == nullptr)
			{
				hwndDlg = hwndInit;
			}

			CreateConfigPath();

			if (SaveConfigXml(hwndDlg) == FALSE)
			{
				MessageBoxW(hwndDlg, L"保存に失敗しました。", TextServiceDesc, MB_OK | MB_ICONERROR);

				if (lParam == PSBTN_OK)
				{
					ReleaseMutex(hMutex);
					CloseHandle(hMutex);
					hMutex = nullptr;

					WCHAR dir[MAX_PATH] = {};
					GetCurrentDirectoryW(_countof(dir), dir);

					WCHAR tempfilepath[MAX_PATH] = {};
					GetTempFileNameW(dir, L"cnf", 0, tempfilepath);

					wcsncpy_s(pathconfigxml, tempfilepath, _TRUNCATE);

					if (SaveConfigXml(hwndDlg) == TRUE)
					{
						WCHAR args[MAX_PATH] = {};
						_snwprintf_s(args, _TRUNCATE, L"%s \"%s\"", CONFIG_RECOVERY_OPTION, tempfilepath);

						StartProcess(hInst, IMCRVCNFEXE, args);
					}
				}
			}
		}
		break;
	default:
		break;
	}
	return 0;
}
