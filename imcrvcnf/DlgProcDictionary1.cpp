
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static LPCWSTR defaultHost = L"localhost";
static LPCWSTR defaultPort = L"1178";
static LPCWSTR defaultTimeOut = L"1000";

#define LISTVIEW_COLUMN_COUNT 3

void LoadDictionary(HWND hDlg);

INT_PTR CALLBACK DlgProcDictionary1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	int	index, count;
	OPENFILENAMEW ofn = {};
	WCHAR path[MAX_PATH];
	WCHAR pathBak[MAX_PATH];
	BOOL check;
	BOOL checkBak;
	WCHAR text[16] = {};
	std::wstring strxmlval;
	static HWND hEdit;

	switch (message)
	{
	case WM_INITDIALOG:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.pszText = text;

		lvc.fmt = LVCFMT_LEFT;
		lvc.iSubItem = 0;
		lvc.cx = 220;
		wcsncpy_s(text, L"ファイル/URL", _TRUNCATE);
		ListView_InsertColumn(hWndListView, 0, &lvc);

		lvc.fmt = LVCFMT_RIGHT;
		lvc.iSubItem = 1;
		lvc.cx = 80;
		wcsncpy_s(text, L"　見出し", _TRUNCATE);
		ListView_InsertColumn(hWndListView, 1, &lvc);

		lvc.fmt = LVCFMT_RIGHT;
		lvc.iSubItem = 2;
		lvc.cx = 80;
		wcsncpy_s(text, L"　　候補", _TRUNCATE);
		ListView_InsertColumn(hWndListView, 2, &lvc);

		LoadDictionary(hDlg);

		LoadCheckButton(hDlg, IDC_CHECKBOX_SKKSRV, SectionServer, ValueServerServ);

		ReadValue(pathconfigxml, SectionServer, ValueServerHost, strxmlval);
		if (strxmlval.empty()) strxmlval = defaultHost;
		SetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_HOST, strxmlval.c_str());

		ReadValue(pathconfigxml, SectionServer, ValueServerPort, strxmlval);
		if (strxmlval.empty()) strxmlval = defaultPort;
		SetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_PORT, strxmlval.c_str());

		LoadCheckButton(hDlg, IDC_RADIO_UTF8, SectionServer, ValueServerEncoding);
		if (!IsDlgButtonChecked(hDlg, IDC_RADIO_UTF8))
		{
			CheckDlgButton(hDlg, IDC_RADIO_EUC, BST_CHECKED);
		}

		ReadValue(pathconfigxml, SectionServer, ValueServerTimeOut, strxmlval);
		if (strxmlval.empty()) strxmlval = defaultTimeOut;
		SetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_TIMEOUT, strxmlval.c_str());

		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);

		ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);
		ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 80));
		ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 80));

		return TRUE;

	case WM_COMMAND:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);

		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_SKK_DIC_UP:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if (index > 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				for (int i = 0; i < LISTVIEW_COLUMN_COUNT; i++)
				{
					ListView_GetItemText(hWndListView, index - 1, i, pathBak, _countof(pathBak));
					checkBak = ListView_GetCheckState(hWndListView, index - 1);
					ListView_GetItemText(hWndListView, index, i, path, _countof(path));
					check = ListView_GetCheckState(hWndListView, index);

					ListView_SetItemText(hWndListView, index - 1, i, path);
					ListView_SetCheckState(hWndListView, index - 1, check);
					ListView_SetItemText(hWndListView, index, i, pathBak);
					ListView_SetCheckState(hWndListView, index, checkBak);
				}

				ListView_SetItemState(hWndListView, index - 1, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_EnsureVisible(hWndListView, index - 1, FALSE);
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_DOWN:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if (index >= 0 && index < count - 1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				for (int i = 0; i < LISTVIEW_COLUMN_COUNT; i++)
				{
					ListView_GetItemText(hWndListView, index + 1, i, pathBak, _countof(pathBak));
					checkBak = ListView_GetCheckState(hWndListView, index + 1);
					ListView_GetItemText(hWndListView, index, i, path, _countof(path));
					check = ListView_GetCheckState(hWndListView, index);

					ListView_SetItemText(hWndListView, index + 1, i, path);
					ListView_SetCheckState(hWndListView, index + 1, check);
					ListView_SetItemText(hWndListView, index, i, pathBak);
					ListView_SetCheckState(hWndListView, index, checkBak);
				}

				ListView_SetItemState(hWndListView, index + 1, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_EnsureVisible(hWndListView, index + 1, FALSE);
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_ADD_FILE:
			path[0] = L'\0';
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = path;
			ofn.lpstrTitle = L"ファイル追加";
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST;

			if (GetOpenFileNameW(&ofn) != 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
				count = ListView_GetItemCount(hWndListView);
				if (index == -1)
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
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);
				ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 80));
				ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 80));
				ListView_SetCheckState(hWndListView, index, TRUE);
				ListView_EnsureVisible(hWndListView, index, FALSE);
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_ADD_URL:
			urlskkdic[0] = L'\0';
			if (IDOK == DialogBoxW(hInst, MAKEINTRESOURCE(IDD_DIALOG_SKK_DIC_ADD_URL), hDlg, DlgProcSKKDicAddUrl))
			{
				if (urlskkdic[0] == L'\0')
				{
					return TRUE;
				}

				PropSheet_Changed(GetParent(hDlg), hDlg);

				index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
				count = ListView_GetItemCount(hWndListView);
				if (index == -1)
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
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);
				ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 80));
				ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 80));
				ListView_SetCheckState(hWndListView, index, TRUE);
				ListView_EnsureVisible(hWndListView, index, FALSE);
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_DEL:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if (index != -1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_DeleteItem(hWndListView, index);
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);
				ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 80));
				ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 80));
			}
			return TRUE;

		case IDC_BUTTON_SKK_DIC_MAKE:
			if (IDOK == MessageBoxW(hDlg,
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
			switch (HIWORD(wParam))
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
		if (lParam == NULL) break;
		if (wParam == IDC_LIST_SKK_DIC)
		{
			hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);
			LPNMLVDISPINFOW pdi = (LPNMLVDISPINFOW)lParam;
			switch (pdi->hdr.code)
			{
			case LVN_BEGINLABELEDIT:
				hEdit = ListView_GetEditControl(hWndListView);
				return TRUE;

			case LVN_ENDLABELEDIT:
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetWindowTextW(hEdit, urlskkdic, _countof(urlskkdic));
				if (urlskkdic[0] == L'\0')
				{
					return FALSE;
				}
				ListView_SetItemText(hWndListView, pdi->item.iItem, 0, urlskkdic);
				urlskkdic[0] = L'\0';
				ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);
				ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 80));
				ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 80));
				return TRUE;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}

	return FALSE;
}

void LoadDictionary(HWND hDlg)
{
	APPDATAXMLLIST list;
	LVITEMW item;

	HRESULT hr = ReadList(pathconfigxml, SectionDictionary, list);

	if (SUCCEEDED(hr) && list.size() != 0)
	{
		HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);
		int i = 0;
		FORWARD_ITERATION_I(l_itr, list)
		{
			if (l_itr->size() == 0 || (*l_itr)[0].first != AttributePath)
			{
				continue;
			}
			item.mask = LVIF_TEXT;
			item.pszText = (LPWSTR)(*l_itr)[0].second.c_str();
			item.iItem = i;
			item.iSubItem = 0;
			ListView_InsertItem(hWndListView, &item);

			BOOL check = TRUE;
			if (l_itr->size() >= 2 && (*l_itr)[1].first == AttributeEnabled)
			{
				check = _wtoi((*l_itr)[1].second.c_str());
			}
			ListView_SetCheckState(hWndListView, i, check);

			i++;
		}
		ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);
		ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 80));
		ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 80));
	}
}

void SaveDictionary1(IXmlWriter *pWriter, HWND hDlg)
{
	APPDATAXMLLIST list;
	APPDATAXMLROW row;
	APPDATAXMLATTR attr;
	WCHAR path[MAX_PATH];

	HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);
	int count = ListView_GetItemCount(hWndListView);

	for (int i = 0; i < count; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, path, _countof(path));

		BOOL check = ListView_GetCheckState(hWndListView, i);

		attr.first = AttributePath;
		attr.second = path;
		row.push_back(attr);

		attr.first = AttributeEnabled;
		attr.second = (check ? L"1" : L"0");
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pWriter, list);
}

void SaveDictionary1Server(IXmlWriter *pWriter, HWND hDlg)
{
	WCHAR num[16];
	WCHAR host[MAX_SKKSERVER_HOST];
	WCHAR port[MAX_SKKSERVER_PORT];

	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_SKKSRV, ValueServerServ);

	GetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_HOST, host, _countof(host));
	WriterKey(pWriter, ValueServerHost, host);

	GetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_PORT, port, _countof(port));
	WriterKey(pWriter, ValueServerPort, port);

	SaveCheckButton(pWriter, hDlg, IDC_RADIO_UTF8, ValueServerEncoding);

	GetDlgItemTextW(hDlg, IDC_EDIT_SKKSRV_TIMEOUT, num, _countof(num));
	WriterKey(pWriter, ValueServerTimeOut, num);
}
