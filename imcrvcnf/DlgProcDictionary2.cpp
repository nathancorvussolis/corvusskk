
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

#define MGR_TIMER_ID		IDD_DIALOG_DICTIONARY2

HANDLE hPipe = INVALID_HANDLE_VALUE;
WCHAR pipebuf[PIPEBUFSIZE];

BOOL ConnectDic();
void DisconnectDic();
BOOL CommandDic(WCHAR command);

INT_PTR CALLBACK DlgProcDictionary2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	std::wstring strxmlval;
	WCHAR num[16];
	INT g;

	switch (message)
	{
	case WM_INITDIALOG:
		SetTimer(hDlg, MGR_TIMER_ID, 1000, nullptr);

		ReadValue(pathconfigxml, SectionDictionary, ValueDictionaryBackupGen, strxmlval);
		g = strxmlval.empty() ? -1 : _wtoi(strxmlval.c_str());
		if (g < 0)
		{
			g = DEF_BACKUPGENS;
		}
		else if (g > MAX_BACKUPGENS)
		{
			g = MAX_BACKUPGENS;
		}
		_snwprintf_s(num, _TRUNCATE, L"%d", g);
		SetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPGEN, num);

		ReadValue(pathconfigxml, SectionDictionary, ValueDictionaryBackupDir, strxmlval);
		if (strxmlval.empty())
		{
			strxmlval = L"%APPDATA%\\" TEXTSERVICE_DESC;
		}
		SetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPDIR, strxmlval.c_str());

		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:
		break;

	case WM_TIMER:
		if (wParam == MGR_TIMER_ID)
		{
			BOOL r = CommandDic(REQ_WATCHDOG);
			SetDlgItemTextW(hDlg, IDC_MGR_STATUS_TEXT, (r ? L"実行中" : L"終了状態"));
			return TRUE;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_MGR_KILL:
			CommandDic(REQ_EXIT);
			return TRUE;

		case IDC_BUTTON_MGR_RUN:
			StartProcess(hInst, IMCRVMGREXE);
			return TRUE;

		case IDC_BUTTON_OPEN_USERDIR:
		{
			PWSTR knownfolderpath = nullptr;

			if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DONT_VERIFY, nullptr, &knownfolderpath)))
			{
				WCHAR appdir[MAX_PATH];

				_snwprintf_s(appdir, _TRUNCATE, L"%s\\%s", knownfolderpath, TextServiceDesc);

				CoTaskMemFree(knownfolderpath);

				ShellExecuteW(nullptr, L"open", appdir, nullptr, nullptr, SW_SHOWNORMAL);

				return TRUE;
			}
		}
		break;

		case IDC_BUTTON_OPEN_SYSTEMDIR:
		{
			PWSTR knownfolderpath = nullptr;

			if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Windows, KF_FLAG_DONT_VERIFY, nullptr, &knownfolderpath)))
			{
				WCHAR appdir[MAX_PATH];

				_snwprintf_s(appdir, _TRUNCATE, L"%s\\%s\\%s", knownfolderpath, SYSTEMROOT_IME_DIR, TEXTSERVICE_DIR);

				CoTaskMemFree(knownfolderpath);

				ShellExecuteW(nullptr, L"open", appdir, nullptr, nullptr, SW_SHOWNORMAL);

				return TRUE;
			}
		}
		break;

		case IDC_EDIT_USERDICBACKUPGEN:
		case IDC_EDIT_USERDICBACKUPDIR:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return TRUE;
			default:
				break;
			}
			break;

		case IDC_BUTTON_OPEN_BACKUPDIR:
		{
			WCHAR prepath[MAX_PATH];
			WCHAR exppath[MAX_PATH];

			GetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPDIR, prepath, _countof(prepath));

			ExpandEnvironmentStringsW(prepath, exppath, _countof(exppath));

			ShellExecuteW(nullptr, L"open", exppath, nullptr, nullptr, SW_SHOWNORMAL);
		}
		break;

		default:
			break;
		}
		break;

	case WM_DESTROY:
		KillTimer(hDlg, MGR_TIMER_ID);
		PostQuitMessage(0);
		return TRUE;

	default:
		break;
	}

	return FALSE;
}

void SaveDictionary2(IXmlWriter *pWriter, HWND hDlg)
{
	WCHAR path[MAX_PATH];
	WCHAR num[16];
	INT g;

	GetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPDIR, path, _countof(path));
	WriterKey(pWriter, ValueDictionaryBackupDir, path);

	GetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPGEN, num, _countof(num));
	g = _wtoi(num);
	if (g < 0)
	{
		g = DEF_BACKUPGENS;
	}
	else if (g > MAX_BACKUPGENS)
	{
		g = MAX_BACKUPGENS;
	}
	_snwprintf_s(num, _TRUNCATE, L"%d", g);
	SetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPGEN, num);
	WriterKey(pWriter, ValueDictionaryBackupGen, num);
}

BOOL ConnectDic()
{
	DWORD dwMode;

	if (WaitNamedPipeW(mgrpipename, NMPWAIT_USE_DEFAULT_WAIT) == 0)
	{
		return FALSE;
	}

	hPipe = CreateFileW(mgrpipename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_EFFECTIVE_ONLY | SECURITY_IDENTIFICATION, nullptr);
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
	if (SetNamedPipeHandleState(hPipe, &dwMode, nullptr, nullptr) == FALSE)
	{
		DisconnectDic();
		return FALSE;
	}

	return TRUE;
}

void DisconnectDic()
{
	if (hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}

BOOL CommandDic(WCHAR command)
{
	BOOL ret = FALSE;

	DWORD bytesWrite, bytesRead;

	ConnectDic();

	pipebuf[0] = command;
	pipebuf[1] = L'\n';
	pipebuf[2] = L'\0';

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

	ret = TRUE;

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	DisconnectDic();

	return ret;
}
