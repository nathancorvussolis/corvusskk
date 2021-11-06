
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

#define MGR_TIMER_ID		IDD_DIALOG_DICTIONARY2

HANDLE hPipe = INVALID_HANDLE_VALUE;
BOOL bkcnfSaved = FALSE;
BOOL mgrprocRun = FALSE;

BOOL ConnectDic();
void DisconnectDic();
BOOL CommandDic(WCHAR command);

INT_PTR CALLBACK DlgProcDictionary2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	std::wstring strxmlval;
	WCHAR num[16];
	INT n;
	UINT u;

	switch (message)
	{
	case WM_INITDIALOG:
		SetTimer(hDlg, MGR_TIMER_ID, 1000, nullptr);

		ReadValue(pathconfigxml, SectionUserDict, ValueBackupGen, strxmlval);
		n = strxmlval.empty() ? -1 : _wtoi(strxmlval.c_str());
		if (n < 0)
		{
			n = DEF_BACKUPGENS;
		}
		else if (n > MAX_BACKUPGENS)
		{
			n = MAX_BACKUPGENS;
		}
		_snwprintf_s(num, _TRUNCATE, L"%d", n);
		SetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPGEN, num);

		ReadValue(pathconfigxml, SectionUserDict, ValueBackupDir, strxmlval);
		if (strxmlval.empty())
		{
			strxmlval = L"%APPDATA%\\" TEXTSERVICE_DESC;
		}
		FORWARD_ITERATION_I(s_itr, strxmlval)
		{
			UINT type = PathGetCharTypeW(*s_itr);
			if ((type & (GCT_LFNCHAR | GCT_SHORTCHAR | GCT_SEPARATOR)) == 0)
			{
				*s_itr = L'_';
			}
		}
		SetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPDIR, strxmlval.c_str());

		ReadValue(pathconfigxml, SectionUserDict, ValuePrivateOnVKey, strxmlval);
		u = (strxmlval.empty() ?
			VK_F10 : (BYTE)wcstoul(strxmlval.c_str(), nullptr, 0));
		_snwprintf_s(num, _TRUNCATE, L"0x%02X", u);
		SetDlgItemTextW(hDlg, IDC_EDIT_PRIVATEMODE_ON_VKEY, num);

		ReadValue(pathconfigxml, SectionUserDict, ValuePrivateOnMKey, strxmlval);
		u = (strxmlval.empty() ?
			(TF_MOD_CONTROL | TF_MOD_SHIFT) : wcstoul(strxmlval.c_str(), nullptr, 0));
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRIVATEMODE_ON_MKEY_ALT, ((u& TF_MOD_ALT) ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRIVATEMODE_ON_MKEY_CTRL, ((u& TF_MOD_CONTROL) ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRIVATEMODE_ON_MKEY_SHIFT, ((u& TF_MOD_SHIFT) ? BST_CHECKED : BST_UNCHECKED));

		ReadValue(pathconfigxml, SectionUserDict, ValuePrivateOffVKey, strxmlval);
		u = (strxmlval.empty() ?
			VK_F10 : (BYTE)wcstoul(strxmlval.c_str(), nullptr, 0));
		_snwprintf_s(num, _TRUNCATE, L"0x%02X", u);
		SetDlgItemTextW(hDlg, IDC_EDIT_PRIVATEMODE_OFF_VKEY, num);

		ReadValue(pathconfigxml, SectionUserDict, ValuePrivateOffMKey, strxmlval);
		u = (strxmlval.empty() ?
			(TF_MOD_CONTROL | TF_MOD_SHIFT) : wcstoul(strxmlval.c_str(), nullptr, 0));
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRIVATEMODE_OFF_MKEY_ALT, ((u& TF_MOD_ALT) ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRIVATEMODE_OFF_MKEY_CTRL, ((u& TF_MOD_CONTROL) ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRIVATEMODE_OFF_MKEY_SHIFT, ((u& TF_MOD_SHIFT) ? BST_CHECKED : BST_UNCHECKED));

		LoadCheckButton(hDlg, IDC_CHECKBOX_PRIVATEMODE_AUTO, SectionUserDict, ValuePrivateModeAuto, L"1");

		bkcnfSaved = TRUE;
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_RUN_BACKUP), (bkcnfSaved && mgrprocRun));

		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:
		break;

	case WM_TIMER:
		if (wParam == MGR_TIMER_ID)
		{
			BOOL running = PathFileExistsW(mgrpipename);

			SetDlgItemTextW(hDlg, IDC_MGR_STATUS_TEXT, (running ? L"実行中" : L"終了状態"));

			mgrprocRun = running;
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_RUN_BACKUP), (bkcnfSaved && mgrprocRun));

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
		case IDC_EDIT_PRIVATEMODE_ON_VKEY:
		case IDC_EDIT_PRIVATEMODE_OFF_VKEY:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				switch (LOWORD(wParam))
				{
				case IDC_EDIT_USERDICBACKUPGEN:
				case IDC_EDIT_USERDICBACKUPDIR:
					bkcnfSaved = FALSE;
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_RUN_BACKUP), (bkcnfSaved && mgrprocRun));
					break;
				default:
					break;
				}
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

			return TRUE;
		}
		break;

		case IDC_BUTTON_RUN_BACKUP:
			CommandDic(REQ_BACKUP);
			return TRUE;

		case IDC_CHECKBOX_PRIVATEMODE_ON_MKEY_ALT:
		case IDC_CHECKBOX_PRIVATEMODE_ON_MKEY_CTRL:
		case IDC_CHECKBOX_PRIVATEMODE_ON_MKEY_SHIFT:
		case IDC_CHECKBOX_PRIVATEMODE_OFF_MKEY_ALT:
		case IDC_CHECKBOX_PRIVATEMODE_OFF_MKEY_CTRL:
		case IDC_CHECKBOX_PRIVATEMODE_OFF_MKEY_SHIFT:
		case IDC_CHECKBOX_PRIVATEMODE_AUTO:
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return TRUE;

		default:
			break;
		}
		break;

	case WM_NOTIFY:
		if (lParam == NULL) break;
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_TRANSLATEACCELERATOR:
		{
			WCHAR vkeytext[8];
			LPMSG lpMsg = (LPMSG)((LPPSHNOTIFY)lParam)->lParam;
			if (lpMsg == NULL) break;
			switch (lpMsg->message)
			{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				switch (GetDlgCtrlID(lpMsg->hwnd))
				{
				case IDC_EDIT_DICTIONARY2_TEST_VKEY:
					_snwprintf_s(vkeytext, _TRUNCATE, L"0x%02X", (BYTE)lpMsg->wParam);
					SetDlgItemTextW(hDlg, IDC_EDIT_DICTIONARY2_TEST_VKEY, vkeytext);
					SendDlgItemMessageW(hDlg, IDC_EDIT_DICTIONARY2_TEST_VKEY, EM_SETSEL, 4, 4);
					SetWindowLongPtrW(hDlg, DWLP_MSGRESULT, PSNRET_MESSAGEHANDLED);
					return TRUE;
				default:
					break;
				}
				break;
			default:
				break;
			}
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
	INT n;
	UINT u;

	GetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPDIR, path, _countof(path));
	for (int i = 0; i < _countof(path) && path[i] != L'\0'; i++)
	{
		UINT type = PathGetCharTypeW(path[i]);
		if ((type & (GCT_LFNCHAR | GCT_SHORTCHAR | GCT_SEPARATOR)) == 0)
		{
			path[i] = L'_';
		}
	}
	SetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPDIR, path);
	WriterKey(pWriter, ValueBackupDir, path);

	GetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPGEN, num, _countof(num));
	n = _wtoi(num);
	if (n < 0)
	{
		n = DEF_BACKUPGENS;
	}
	else if (n > MAX_BACKUPGENS)
	{
		n = MAX_BACKUPGENS;
	}
	_snwprintf_s(num, _TRUNCATE, L"%d", n);
	SetDlgItemTextW(hDlg, IDC_EDIT_USERDICBACKUPGEN, num);
	WriterKey(pWriter, ValueBackupGen, num);

	GetDlgItemTextW(hDlg, IDC_EDIT_PRIVATEMODE_ON_VKEY, num, _countof(num));
	_snwprintf_s(num, _TRUNCATE, L"0x%02X", (BYTE)wcstoul(num, nullptr, 0));
	SetDlgItemTextW(hDlg, IDC_EDIT_PRIVATEMODE_ON_VKEY, num);
	WriterKey(pWriter, ValuePrivateOnVKey, num);

	u = 0;
	if (IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRIVATEMODE_ON_MKEY_ALT)) { u |= TF_MOD_ALT; }
	if (IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRIVATEMODE_ON_MKEY_CTRL)) { u |= TF_MOD_CONTROL; }
	if (IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRIVATEMODE_ON_MKEY_SHIFT)) { u |= TF_MOD_SHIFT; }
	_snwprintf_s(num, _TRUNCATE, L"%X", u);
	WriterKey(pWriter, ValuePrivateOnMKey, num);

	GetDlgItemTextW(hDlg, IDC_EDIT_PRIVATEMODE_OFF_VKEY, num, _countof(num));
	_snwprintf_s(num, _TRUNCATE, L"0x%02X", (BYTE)wcstoul(num, nullptr, 0));
	SetDlgItemTextW(hDlg, IDC_EDIT_PRIVATEMODE_OFF_VKEY, num);
	WriterKey(pWriter, ValuePrivateOffVKey, num);

	u = 0;
	if (IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRIVATEMODE_OFF_MKEY_ALT)) { u |= TF_MOD_ALT; }
	if (IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRIVATEMODE_OFF_MKEY_CTRL)) { u |= TF_MOD_CONTROL; }
	if (IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRIVATEMODE_OFF_MKEY_SHIFT)) { u |= TF_MOD_SHIFT; }
	_snwprintf_s(num, _TRUNCATE, L"%X", u);
	WriterKey(pWriter, ValuePrivateOffMKey, num);

	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_PRIVATEMODE_AUTO, ValuePrivateModeAuto);

	bkcnfSaved = TRUE;
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_RUN_BACKUP), (bkcnfSaved && mgrprocRun));
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

	WCHAR pipebuf[4];
	DWORD bytesWrite, bytesRead;

	ConnectDic();

	pipebuf[0] = command;
	pipebuf[1] = L'\n';
	pipebuf[2] = L'\0';
	pipebuf[3] = L'\0';

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf) - sizeof(WCHAR), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

	ret = TRUE;

exit:

	DisconnectDic();

	return ret;
}
