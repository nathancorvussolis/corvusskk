
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static LPCWSTR defaultHost = L"localhost";
static LPCWSTR defaultPort = L"1178";
static LPCWSTR defaultTimeOut = L"1000";

static WCHAR urlskkdic[INTERNET_MAX_URL_LENGTH];

INT_PTR CALLBACK DlgProcSKKDicAddUrl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK DlgProcDictionary(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	int	index, count;
	OPENFILENAMEW ofn;
	WCHAR path[MAX_PATH];
	WCHAR pathBak[MAX_PATH];
	BOOL check;
	BOOL checkBak;
	WCHAR num[32];
	WCHAR host[MAX_SKKSERVER_HOST];
	WCHAR port[MAX_SKKSERVER_PORT];
	std::wstring strxmlval;
	FILE *fp;
	static HWND hEdit;

	switch(message)
	{
	case WM_INITDIALOG:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_CENTER;

		lvc.iSubItem = 0;
		lvc.cx = 220;
		lvc.pszText = L"";
		ListView_InsertColumn(hWndListView, 0, &lvc);

		LoadDictionary(hDlg);

		LoadCheckButton(hDlg, IDC_CHECKBOX_SKKSRV, SectionServer, ValueServerServ);

		ReadValue(pathconfigxml, SectionServer, ValueServerHost, strxmlval);
		if(strxmlval.empty()) strxmlval = defaultHost;
		SetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_HOST, strxmlval.c_str());

		ReadValue(pathconfigxml, SectionServer, ValueServerPort, strxmlval);
		if(strxmlval.empty()) strxmlval = defaultPort;
		SetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_PORT, strxmlval.c_str());

		LoadCheckButton(hDlg, IDC_RADIO_UTF8, SectionServer, ValueServerEncoding);
		if(!IsDlgButtonChecked(hDlg, IDC_RADIO_UTF8))
		{
			CheckDlgButton(hDlg, IDC_RADIO_EUC, BST_CHECKED);
		}

		ReadValue(pathconfigxml, SectionServer, ValueServerTimeOut, strxmlval);
		if(strxmlval.empty()) strxmlval = defaultTimeOut;
		SetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_TIMEOUT, strxmlval.c_str());

		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);

		ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);

		return TRUE;

	case WM_COMMAND:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);

		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_SKK_DIC_UP:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index > 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_GetItemText(hWndListView, index - 1, 0, pathBak, _countof(pathBak));
				checkBak = ListView_GetCheckState(hWndListView, index - 1);
				ListView_GetItemText(hWndListView, index, 0, path, _countof(path));
				check = ListView_GetCheckState(hWndListView, index);

				ListView_SetItemText(hWndListView, index - 1, 0, path);
				ListView_SetCheckState(hWndListView, index - 1, check);
				ListView_SetItemText(hWndListView, index, 0, pathBak);
				ListView_SetCheckState(hWndListView, index, checkBak);

				ListView_SetItemState(hWndListView, index - 1, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_EnsureVisible(hWndListView, index - 1, FALSE);
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_DOWN:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if(index >= 0 && index < count - 1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_GetItemText(hWndListView, index + 1, 0, pathBak, _countof(pathBak));
				checkBak = ListView_GetCheckState(hWndListView, index + 1);
				ListView_GetItemText(hWndListView, index, 0, path, _countof(path));
				check = ListView_GetCheckState(hWndListView, index);

				ListView_SetItemText(hWndListView, index + 1, 0, path);
				ListView_SetCheckState(hWndListView, index + 1, check);
				ListView_SetItemText(hWndListView, index, 0, pathBak);
				ListView_SetCheckState(hWndListView, index, checkBak);

				ListView_SetItemState(hWndListView, index + 1, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_EnsureVisible(hWndListView, index + 1, FALSE);
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_ADD_FILE:
			path[0] = L'\0';
			ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
			ofn.lStructSize = sizeof(OPENFILENAMEW);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = path;
			ofn.lpstrTitle = L"ファイル追加";
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST;
			if(GetOpenFileNameW(&ofn) != 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
				count = ListView_GetItemCount(hWndListView);
				if(index == -1)
				{
					index = count;
				}
				else
				{
					++index;
				}
				item.mask = LVIF_TEXT;
				item.pszText = path;
				item.iItem = index;
				item.iSubItem = 0;
				ListView_InsertItem(hWndListView, &item);
				ListView_SetItemState(hWndListView, index, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
				ListView_SetCheckState(hWndListView, index, TRUE);
				ListView_EnsureVisible(hWndListView, index, FALSE);
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_ADD_URL:
			urlskkdic[0] = L'\0';
			if(IDOK == DialogBoxW(hInst, MAKEINTRESOURCE(IDD_DIALOG_SKK_DIC_ADD_URL), hDlg, DlgProcSKKDicAddUrl))
			{
				if(urlskkdic[0] == L'\0')
				{
					return TRUE;
				}

				PropSheet_Changed(GetParent(hDlg), hDlg);

				index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
				count = ListView_GetItemCount(hWndListView);
				if(index == -1)
				{
					index = count;
				}
				else
				{
					++index;
				}
				item.mask = LVIF_TEXT;
				item.pszText = urlskkdic;
				item.iItem = index;
				item.iSubItem = 0;
				ListView_InsertItem(hWndListView, &item);
				ListView_SetItemState(hWndListView, index, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
				ListView_SetCheckState(hWndListView, index, TRUE);
				ListView_EnsureVisible(hWndListView, index, FALSE);
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_DEL:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index != -1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_DeleteItem(hWndListView, index);
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_MAKE:
			if(IDOK == MessageBoxW(hDlg,
				L"SKK辞書を取り込みます。\nよろしいですか？",
				TextServiceDesc, MB_OKCANCEL | MB_ICONQUESTION))
			{
				MakeSKKDic(hDlg);
			}
			return TRUE;

		case IDC_CHECKBOX_SKKSRV:
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return TRUE;

		case IDC_EDIT_SKKSRV_HOST:
		case IDC_EDIT_SKKSRV_PORT:
		case IDC_EDIT_SKKSRV_TIMEOUT:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return TRUE;
			default:
				break;
			}
			break;

		case IDC_RADIO_EUC:
		case IDC_RADIO_UTF8:
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return TRUE;

		default:
			break;
		}
		break;

	case WM_NOTIFY:
		if(wParam == IDC_LIST_SKK_DIC)
		{
			hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);
			LPNMLVDISPINFOW pdi = (LPNMLVDISPINFOW)lParam;
			switch(pdi->hdr.code)
			{
			case LVN_BEGINLABELEDIT:
				hEdit = ListView_GetEditControl(hWndListView);
				return TRUE;

			case LVN_ENDLABELEDIT:
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetWindowTextW(hEdit, urlskkdic, _countof(urlskkdic));
				if(urlskkdic[0] == L'\0')
				{
					return FALSE;
				}
				ListView_SetItemText(hWndListView, pdi->item.iItem, 0, urlskkdic);
				urlskkdic[0] = L'\0';
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
				return TRUE;

			default:
				break;
			}
		}

		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			_wfopen_s(&fp, pathconfigxml, L"ab");
			if(fp != nullptr)
			{
				fclose(fp);
			}
			SetFileDacl(pathconfigxml);

			WriterInit(pathconfigxml, &pXmlWriter, &pXmlFileStream);

			WriterStartElement(pXmlWriter, TagRoot);

			WriterStartSection(pXmlWriter, SectionDictionary);	//Start of SectionDictionary

			SaveDictionary(hDlg);

			WriterEndSection(pXmlWriter);	//End of SectionDictionary

			WriterStartSection(pXmlWriter, SectionServer);	//Start of SectionServer

			SaveCheckButton(hDlg, IDC_CHECKBOX_SKKSRV, ValueServerServ);

			GetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_HOST, host, _countof(host));
			WriterKey(pXmlWriter, ValueServerHost, host);

			GetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_PORT, port, _countof(port));
			WriterKey(pXmlWriter, ValueServerPort, port);

			SaveCheckButton(hDlg, IDC_RADIO_UTF8, ValueServerEncoding);

			GetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_TIMEOUT, num, _countof(num));
			WriterKey(pXmlWriter, ValueServerTimeOut, num);

			WriterEndSection(pXmlWriter);	//End of SectionServer

			return TRUE;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcSKKDicAddUrl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		SetBkMode((HDC)wParam, TRANSPARENT);
		SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemTextW(hDlg, IDC_EDIT_SKK_DIC_URL, urlskkdic, _countof(urlskkdic));
			{
				// trim
				std::wstring strurl = std::regex_replace(std::wstring(urlskkdic),
					std::wregex(L"^\\s+|\\s+$"), std::wstring(L""));
				_snwprintf_s(urlskkdic, _TRUNCATE, L"%s", strurl.c_str());

				if(urlskkdic[0] == L'\0')
				{
					EndDialog(hDlg, IDCANCEL);
				}
			}
			EndDialog(hDlg, IDOK);
			break;
		case IDCANCEL:
			urlskkdic[0] = L'\0';
			EndDialog(hDlg, IDCANCEL);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return FALSE;
}
