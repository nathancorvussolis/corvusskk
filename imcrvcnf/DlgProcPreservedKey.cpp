
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

TF_PRESERVEDKEY preservedkey[PRESERVEDKEY_NUM][MAX_PRESERVEDKEY];

static const struct {
	int id;
	LPCWSTR text;
	LPCWSTR section;
} preservedkeyInfo[PRESERVEDKEY_NUM] = {
	{IDC_LIST_PRSRVKEY_ON, L"ON 仮想ｷｰ", SectionPreservedKeyON},
	{IDC_LIST_PRSRVKEY_OFF, L"OFF 仮想ｷｰ", SectionPreservedKeyOFF},
};

void LoadPreservedKey(HWND hDlg);

INT_PTR CALLBACK DlgProcPreservedKey(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	int index, count;
	WCHAR key[8];
	WCHAR keyBak[8];
	NMLISTVIEW *pListView;
	WCHAR text[16] = {};

	switch (message)
	{
	case WM_INITDIALOG:
		for (int i = 0; i < PRESERVEDKEY_NUM; i++)
		{
			hWndListView = GetDlgItem(hDlg, preservedkeyInfo[i].id);
			ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_CENTER;
			lvc.pszText = text;

			lvc.iSubItem = 0;
			lvc.cx = GetScaledSizeX(hDlg, 90);
			wcsncpy_s(text, preservedkeyInfo[i].text, _TRUNCATE);
			ListView_InsertColumn(hWndListView, 0, &lvc);
			lvc.iSubItem = 1;
			lvc.cx = GetScaledSizeX(hDlg, 60);
			wcsncpy_s(text, L"ALT", _TRUNCATE);
			ListView_InsertColumn(hWndListView, 1, &lvc);
			lvc.iSubItem = 2;
			lvc.cx = GetScaledSizeX(hDlg, 60);
			wcsncpy_s(text, L"CTRL", _TRUNCATE);
			ListView_InsertColumn(hWndListView, 2, &lvc);
			lvc.iSubItem = 3;
			lvc.cx = GetScaledSizeX(hDlg, 60);
			wcsncpy_s(text, L"SHIFT", _TRUNCATE);
			ListView_InsertColumn(hWndListView, 3, &lvc);
		}

		SetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, L"");
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_RADIO_PRSRVKEY_ON, BST_CHECKED);

		LoadPreservedKey(hDlg);

		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:
		for (int i = 0; i < PRESERVEDKEY_NUM; i++)
		{
			hWndListView = GetDlgItem(hDlg, preservedkeyInfo[i].id);

			ListView_SetColumnWidth(hWndListView, 0, GetScaledSizeX(hDlg, 90));
			ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 60));
			ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 60));
			ListView_SetColumnWidth(hWndListView, 3, GetScaledSizeX(hDlg, 60));
		}

		return TRUE;

	case WM_COMMAND:
		hWndListView = GetDlgItem(hDlg,
			IsDlgButtonChecked(hDlg, IDC_RADIO_PRSRVKEY_ON) ? IDC_LIST_PRSRVKEY_ON : IDC_LIST_PRSRVKEY_OFF);
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_PRSRVKEY_W:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if (index >= 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key, _countof(key));
				_snwprintf_s(key, _TRUNCATE, L"0x%02X", wcstoul(key, nullptr, 0));
				SetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key);
				ListView_SetItemText(hWndListView, index, 0, key);
				wcsncpy_s(text, IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT) == BST_CHECKED ? L"1" : L"0", _TRUNCATE);
				ListView_SetItemText(hWndListView, index, 1, text);
				wcsncpy_s(text, IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL) == BST_CHECKED ? L"1" : L"0", _TRUNCATE);
				ListView_SetItemText(hWndListView, index, 2, text);
				wcsncpy_s(text, IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT) == BST_CHECKED ? L"1" : L"0", _TRUNCATE);
				ListView_SetItemText(hWndListView, index, 3, text);
			}
			else if (count < MAX_PRESERVEDKEY)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				item.mask = LVIF_TEXT;

				GetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key, _countof(key));
				_snwprintf_s(key, _TRUNCATE, L"0x%02X", wcstoul(key, nullptr, 0));
				SetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key);
				item.pszText = key;
				item.iItem = count;
				item.iSubItem = 0;
				ListView_InsertItem(hWndListView, &item);

				item.pszText = text;
				wcsncpy_s(text, IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT) == BST_CHECKED ? L"1" : L"0", _TRUNCATE);
				item.iSubItem = 1;
				ListView_SetItem(hWndListView, &item);
				wcsncpy_s(text, IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL) == BST_CHECKED ? L"1" : L"0", _TRUNCATE);
				item.pszText = text;
				item.iSubItem = 2;
				ListView_SetItem(hWndListView, &item);
				wcsncpy_s(text, IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT) == BST_CHECKED ? L"1" : L"0", _TRUNCATE);
				item.iSubItem = 3;
				ListView_SetItem(hWndListView, &item);
			}
			return TRUE;

		case IDC_BUTTON_PRSRVKEY_D:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if (index != -1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_DeleteItem(hWndListView, index);
			}
			return TRUE;

		case IDC_BUTTON_PRSRVKEY_UP:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if (index > 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_GetItemText(hWndListView, index - 1, 0, keyBak, _countof(keyBak));
				ListView_GetItemText(hWndListView, index, 0, key, _countof(key));
				ListView_SetItemText(hWndListView, index - 1, 0, key);
				ListView_SetItemText(hWndListView, index, 0, keyBak);

				ListView_GetItemText(hWndListView, index - 1, 1, keyBak, _countof(keyBak));
				ListView_GetItemText(hWndListView, index, 1, key, _countof(key));
				ListView_SetItemText(hWndListView, index - 1, 1, key);
				ListView_SetItemText(hWndListView, index, 1, keyBak);

				ListView_GetItemText(hWndListView, index - 1, 2, keyBak, _countof(keyBak));
				ListView_GetItemText(hWndListView, index, 2, key, _countof(key));
				ListView_SetItemText(hWndListView, index - 1, 2, key);
				ListView_SetItemText(hWndListView, index, 2, keyBak);

				ListView_GetItemText(hWndListView, index - 1, 3, keyBak, _countof(keyBak));
				ListView_GetItemText(hWndListView, index, 3, key, _countof(key));
				ListView_SetItemText(hWndListView, index - 1, 3, key);
				ListView_SetItemText(hWndListView, index, 3, keyBak);

				ListView_SetItemState(hWndListView, index - 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
			}
			return TRUE;

		case IDC_BUTTON_PRSRVKEY_DOWN:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if (index >= 0 && index < count - 1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_GetItemText(hWndListView, index + 1, 0, keyBak, _countof(keyBak));
				ListView_GetItemText(hWndListView, index, 0, key, _countof(key));
				ListView_SetItemText(hWndListView, index + 1, 0, key);
				ListView_SetItemText(hWndListView, index, 0, keyBak);

				ListView_GetItemText(hWndListView, index + 1, 1, keyBak, _countof(keyBak));
				ListView_GetItemText(hWndListView, index, 1, key, _countof(key));
				ListView_SetItemText(hWndListView, index + 1, 1, key);
				ListView_SetItemText(hWndListView, index, 1, keyBak);

				ListView_GetItemText(hWndListView, index + 1, 2, keyBak, _countof(keyBak));
				ListView_GetItemText(hWndListView, index, 2, key, _countof(key));
				ListView_SetItemText(hWndListView, index + 1, 2, key);
				ListView_SetItemText(hWndListView, index, 2, keyBak);

				ListView_GetItemText(hWndListView, index + 1, 3, keyBak, _countof(keyBak));
				ListView_GetItemText(hWndListView, index, 3, key, _countof(key));
				ListView_SetItemText(hWndListView, index + 1, 3, key);
				ListView_SetItemText(hWndListView, index, 3, keyBak);

				ListView_SetItemState(hWndListView, index + 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
			}
			return TRUE;

		case IDC_RADIO_PRSRVKEY_ON:
			SetFocus(GetDlgItem(hDlg, IDC_LIST_PRSRVKEY_ON));
			return TRUE;

		case IDC_RADIO_PRSRVKEY_OFF:
			SetFocus(GetDlgItem(hDlg, IDC_LIST_PRSRVKEY_OFF));
			return TRUE;

		default:
			break;
		}
		break;

	case WM_NOTIFY:
		hWndListView = ((LPNMHDR)lParam)->hwndFrom;
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_TRANSLATEACCELERATOR:
			{
				LPMSG lpMsg = (LPMSG)((LPPSHNOTIFY)lParam)->lParam;
				switch (lpMsg->message)
				{
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					switch (GetDlgCtrlID(lpMsg->hwnd))
					{
					case IDC_EDIT_DISPVKEY:
						WCHAR vkeytext[8];
						_snwprintf_s(vkeytext, _TRUNCATE, L"0x%02X", (BYTE)lpMsg->wParam);
						SetDlgItemTextW(hDlg, IDC_EDIT_DISPVKEY, vkeytext);
						SendDlgItemMessageW(hDlg, IDC_EDIT_DISPVKEY, EM_SETSEL, 4, 4);
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

		case NM_SETFOCUS:
			switch (((LPNMHDR)lParam)->idFrom)
			{
			case IDC_LIST_PRSRVKEY_ON:
				CheckRadioButton(hDlg, IDC_RADIO_PRSRVKEY_ON, IDC_RADIO_PRSRVKEY_OFF, IDC_RADIO_PRSRVKEY_ON);
				break;
			case IDC_LIST_PRSRVKEY_OFF:
				CheckRadioButton(hDlg, IDC_RADIO_PRSRVKEY_ON, IDC_RADIO_PRSRVKEY_OFF, IDC_RADIO_PRSRVKEY_OFF);
				break;
			default:
				break;
			}
			switch (((LPNMHDR)lParam)->idFrom)
			{
			case IDC_LIST_PRSRVKEY_ON:
			case IDC_LIST_PRSRVKEY_OFF:
				SetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, L"");
				CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT, BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL, BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT, BST_UNCHECKED);

				index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
				ListView_SetItemState(hWndListView, index, 0, 0x000F);
				ListView_SetItemState(hWndListView, index, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
				return TRUE;
			default:
				break;
			}
			break;

		case LVN_ITEMCHANGED:
			pListView = (NMLISTVIEW*)((LPNMHDR)lParam);
			if (pListView->uChanged & LVIF_STATE)
			{
				index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
				if (index == -1)
				{
					SetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, L"");
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT, BST_UNCHECKED);
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL, BST_UNCHECKED);
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT, BST_UNCHECKED);
				}
				else
				{
					ListView_GetItemText(hWndListView, index, 0, key, _countof(key));
					SetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key);
					ListView_GetItemText(hWndListView, index, 1, key, _countof(key));
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT, key[0] == L'1' ? BST_CHECKED : BST_UNCHECKED);
					ListView_GetItemText(hWndListView, index, 2, key, _countof(key));
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL, key[0] == L'1' ? BST_CHECKED : BST_UNCHECKED);
					ListView_GetItemText(hWndListView, index, 3, key, _countof(key));
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT, key[0] == L'1' ? BST_CHECKED : BST_UNCHECKED);
				}
				return TRUE;
			}
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

void SetConfigPreservedKeyONOFF(int onoff, const APPDATAXMLLIST &list)
{
	if (onoff != 0 && onoff != 1)
	{
		return;
	}

	ZeroMemory(preservedkey[onoff], sizeof(preservedkey[onoff]));

	if (list.size() != 0)
	{
		int i = 0;
		FORWARD_ITERATION_I(l_itr, list)
		{
			if (i >= MAX_PRESERVEDKEY)
			{
				break;
			}

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				if (r_itr->first == AttributeVKey)
				{
					preservedkey[onoff][i].uVKey = wcstoul(r_itr->second.c_str(), nullptr, 0);
				}
				else if (r_itr->first == AttributeMKey)
				{
					preservedkey[onoff][i].uModifiers = wcstoul(r_itr->second.c_str(), nullptr, 0);
					if (preservedkey[onoff][i].uModifiers == 0)
					{
						preservedkey[onoff][i].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
					}
				}
			}

			i++;
		}
	}
	else
	{
		preservedkey[onoff][0].uVKey = VK_OEM_3;	// 0xC0   Alt + `
		preservedkey[onoff][0].uModifiers = TF_MOD_ALT;
		preservedkey[onoff][1].uVKey = VK_KANJI;	// 0x19   漢字 / Alt + 半角/全角 / Alt + `
		preservedkey[onoff][1].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
		preservedkey[onoff][2].uVKey = VK_OEM_AUTO;	// 0xF3   半角/全角 / Ctrl + `
		preservedkey[onoff][2].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
		preservedkey[onoff][3].uVKey = VK_OEM_ENLW;	// 0xF4   半角/全角 / Ctrl + `
		preservedkey[onoff][3].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;

		switch (onoff)
		{
		case 0:
			preservedkey[onoff][4].uVKey = VK_IME_ON;	//0x16
			preservedkey[onoff][4].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
			break;
		case 1:
			preservedkey[onoff][4].uVKey = VK_IME_OFF;	//0x1A
			preservedkey[onoff][4].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
			break;
		default:
			break;
		}
	}
}

void LoadConfigPreservedKey()
{
	APPDATAXMLLIST list;

	//for compatibility
	HRESULT hr = ReadList(pathconfigxml, SectionPreservedKey, list);

	if (SUCCEEDED(hr) && list.size() != 0)
	{
		for (int k = 0; k < PRESERVEDKEY_NUM; k++)
		{
			SetConfigPreservedKeyONOFF(k, list);
		}
	}
	else
	{
		for (int k = 0; k < PRESERVEDKEY_NUM; k++)
		{
			list.clear();
			hr = ReadList(pathconfigxml, preservedkeyInfo[k].section, list);
			SetConfigPreservedKeyONOFF(k, list);
		}
	}
}

void LoadPreservedKey(HWND hDlg)
{
	LVITEMW item;
	WCHAR num[16];

	LoadConfigPreservedKey();

	for (int k = 0; k < PRESERVEDKEY_NUM; k++)
	{
		HWND hWndListView = GetDlgItem(hDlg, preservedkeyInfo[k].id);

		for (int i = 0; i < MAX_PRESERVEDKEY; i++)
		{
			if (preservedkey[k][i].uVKey == 0 &&
				preservedkey[k][i].uModifiers == 0)
			{
				break;
			}

			item.mask = LVIF_TEXT;
			_snwprintf_s(num, _TRUNCATE, L"0x%02X", preservedkey[k][i].uVKey);
			item.pszText = num;
			item.iItem = i;
			item.iSubItem = 0;
			ListView_InsertItem(hWndListView, &item);
			_snwprintf_s(num, _TRUNCATE, L"%d", (preservedkey[k][i].uModifiers & TF_MOD_ALT) ? 1 : 0);
			item.pszText = num;
			item.iItem = i;
			item.iSubItem = 1;
			ListView_SetItem(hWndListView, &item);
			_snwprintf_s(num, _TRUNCATE, L"%d", (preservedkey[k][i].uModifiers & TF_MOD_CONTROL) ? 1 : 0);
			item.pszText = num;
			item.iItem = i;
			item.iSubItem = 2;
			ListView_SetItem(hWndListView, &item);
			_snwprintf_s(num, _TRUNCATE, L"%d", (preservedkey[k][i].uModifiers & TF_MOD_SHIFT) ? 1 : 0);
			item.pszText = num;
			item.iItem = i;
			item.iSubItem = 3;
			ListView_SetItem(hWndListView, &item);
		}
	}
}

void SavePreservedKey(IXmlWriter *pWriter, HWND hDlg, int no)
{
	APPDATAXMLLIST list;
	APPDATAXMLROW row;
	APPDATAXMLATTR attr;
	WCHAR key[8];

	list.clear();

	HWND hWndListView = GetDlgItem(hDlg, preservedkeyInfo[no].id);
	int count = ListView_GetItemCount(hWndListView);

	for (int i = 0; i < count && i < MAX_PRESERVEDKEY; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, key, _countof(key));
		preservedkey[no][i].uVKey = wcstoul(key, nullptr, 0);
		preservedkey[no][i].uModifiers = 0;
		ListView_GetItemText(hWndListView, i, 1, key, _countof(key));
		if (key[0] == L'1')
		{
			preservedkey[no][i].uModifiers |= TF_MOD_ALT;
		}
		ListView_GetItemText(hWndListView, i, 2, key, _countof(key));
		if (key[0] == L'1')
		{
			preservedkey[no][i].uModifiers |= TF_MOD_CONTROL;
		}
		ListView_GetItemText(hWndListView, i, 3, key, _countof(key));
		if (key[0] == L'1')
		{
			preservedkey[no][i].uModifiers |= TF_MOD_SHIFT;
		}
	}
	if (count < MAX_PRESERVEDKEY)
	{
		preservedkey[no][count].uVKey = 0;
		preservedkey[no][count].uModifiers = 0;
	}

	for (int i = 0; i < MAX_PRESERVEDKEY; i++)
	{
		if (preservedkey[no][i].uVKey == 0 &&
			preservedkey[no][i].uModifiers == 0)
		{
			break;
		}

		attr.first = AttributeVKey;
		_snwprintf_s(key, _TRUNCATE, L"0x%02X", preservedkey[no][i].uVKey);
		attr.second = key;
		row.push_back(attr);

		attr.first = AttributeMKey;
		_snwprintf_s(key, _TRUNCATE, L"%X", preservedkey[no][i].uModifiers);
		attr.second = key;
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pWriter, list);
}

void SavePreservedKeyON(IXmlWriter *pWriter, HWND hDlg)
{
	SavePreservedKey(pWriter, hDlg, 0);
}

void SavePreservedKeyOFF(IXmlWriter *pWriter, HWND hDlg)
{
	SavePreservedKey(pWriter, hDlg, 1);
}
