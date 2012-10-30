
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

//デフォルト
static LPCWSTR defaultHost = L"localhost";
static LPCWSTR defaultPort = L"1178";
static LPCWSTR defaultTimeOut = L"1000";

INT_PTR CALLBACK DlgProcDictionary(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	int	index, count;
	OPENFILENAMEW ofn;
	WCHAR path[MAX_PATH];
	WCHAR pathBak[MAX_PATH];
	WCHAR num[32];
	WCHAR host[MAX_SKKSERVER_HOST];
	WCHAR port[MAX_SKKSERVER_PORT];
	std::wstring strxmlval;

	switch(message)
	{
	case WM_INITDIALOG:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT);
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

		return (INT_PTR)TRUE;

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
				ListView_GetItemText(hWndListView, index, 0, path, _countof(path));
				ListView_SetItemText(hWndListView, index - 1, 0, path);
				ListView_SetItemText(hWndListView, index, 0, pathBak);
				ListView_SetItemState(hWndListView, index - 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
				ListView_EnsureVisible(hWndListView, index - 1, FALSE);
			}
			return (INT_PTR)TRUE;

		case IDC_BUTTON_SKK_DIC_DOWN:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if(index >= 0 && index < count - 1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_GetItemText(hWndListView, index + 1, 0, pathBak, _countof(pathBak));
				ListView_GetItemText(hWndListView, index, 0, path, _countof(path));
				ListView_SetItemText(hWndListView, index + 1, 0, path);
				ListView_SetItemText(hWndListView, index, 0, pathBak);
				ListView_SetItemState(hWndListView, index + 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
				ListView_EnsureVisible(hWndListView, index + 1, FALSE);
			}
			return (INT_PTR)TRUE;

		case IDC_BUTTON_SKK_DIC_ADD:
			path[0] = L'\0';
			ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
			ofn.lStructSize = sizeof(OPENFILENAMEW);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = path;
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
				ListView_SetItemState(hWndListView, index, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
				ListView_EnsureVisible(hWndListView, index, FALSE);
			}
			return (INT_PTR)TRUE;

		case IDC_BUTTON_SKK_DIC_DEL:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index != -1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_DeleteItem(hWndListView, index);
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
			}
			return (INT_PTR)TRUE;

		case IDC_BUTTON_SKK_DIC_MAKE:
			if(IDOK == MessageBoxW(hDlg,
				L"SKK辞書を読み込み、独自形式辞書を作成します。\nよろしいですか？",
				TextServiceDesc, MB_OKCANCEL))
			{
				MakeSKKDic(hDlg);
			}
			return (INT_PTR)TRUE;

		case IDC_BUTTON_SKK_DIC_LOAD:
			path[0] = L'\0';
			ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
			ofn.lStructSize = sizeof(OPENFILENAMEW);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = path;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST;
			if(GetOpenFileNameW(&ofn) != 0)
			{
				if(IDOK == MessageBoxW(hDlg,
					L"SKKユーザ辞書を読み込み、ユーザ辞書を上書きします。\nよろしいですか？",
					TextServiceDesc, MB_OKCANCEL))
				{
					ReqSKKUserDic(hDlg, REQ_SKK_LOAD, path);
				}
			}
			return (INT_PTR)TRUE;

		case IDC_BUTTON_SKK_DIC_SAVE:
			path[0] = L'\0';
			ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
			ofn.lStructSize = sizeof(OPENFILENAMEW);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = path;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFilter = L"SKKユーザ辞書 (*.*)\0*.*\0\0";
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			if(GetSaveFileNameW(&ofn) != 0)
			{
				ReqSKKUserDic(hDlg, REQ_SKK_SAVE, path);
			}
			return (INT_PTR)TRUE;

		case IDC_CHECKBOX_SKKSRV:
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return (INT_PTR)TRUE;

		case IDC_EDIT_SKKSRV_HOST:
		case IDC_EDIT_SKKSRV_PORT:
		case IDC_EDIT_SKKSRV_TIMEOUT:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return (INT_PTR)TRUE;
			default:
				break;
			}
			break;

		case IDC_RADIO_EUC:
		case IDC_RADIO_UTF8:
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return (INT_PTR)TRUE;

		default:
			break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			WriterStartSection(pXmlWriter, SectionDictionary);

			SaveDictionary(hDlg);

			WriterEndSection(pXmlWriter);

			WriterStartSection(pXmlWriter, SectionServer);

			SaveCheckButton(hDlg, IDC_CHECKBOX_SKKSRV, SectionServer, ValueServerServ);

			GetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_HOST, host, _countof(host));
			WriterKey(pXmlWriter, ValueServerHost, host);

			GetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_PORT, port, _countof(port));
			WriterKey(pXmlWriter, ValueServerPort, port);

			SaveCheckButton(hDlg, IDC_RADIO_UTF8, SectionServer, ValueServerEncoding);

			GetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_TIMEOUT, num, _countof(num));
			WriterKey(pXmlWriter, ValueServerTimeOut, num);

			WriterEndSection(pXmlWriter);

			return (INT_PTR)TRUE;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return (INT_PTR)FALSE;
}
