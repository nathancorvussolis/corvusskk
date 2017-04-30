
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static const struct {
	int id;
	LPWSTR text;
} preservedkeyTextInfo[PRESERVEDKEY_NUM] = {
	{IDC_LIST_PRSRVKEY_ON, L"ON 仮想ｷｰ"},
	{IDC_LIST_PRSRVKEY_OFF, L"OFF 仮想ｷｰ"},
};

INT_PTR CALLBACK DlgProcPreservedKey(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	int index, count;
	WCHAR key[8];
	WCHAR keyBak[8];
	NMLISTVIEW *pListView;

	switch(message)
	{
	case WM_INITDIALOG:
		for(int i = 0; i < PRESERVEDKEY_NUM; i++)
		{
			hWndListView = GetDlgItem(hDlg, preservedkeyTextInfo[i].id);
			ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_CENTER;

			lvc.iSubItem = 0;
			lvc.cx = GetScaledSizeX(hDlg, 90);
			lvc.pszText = preservedkeyTextInfo[i].text;
			ListView_InsertColumn(hWndListView, 0, &lvc);
			lvc.iSubItem = 1;
			lvc.cx = GetScaledSizeX(hDlg, 60);
			lvc.pszText = L"ALT";
			ListView_InsertColumn(hWndListView, 1, &lvc);
			lvc.iSubItem = 2;
			lvc.cx = GetScaledSizeX(hDlg, 60);
			lvc.pszText = L"CTRL";
			ListView_InsertColumn(hWndListView, 2, &lvc);
			lvc.iSubItem = 3;
			lvc.cx = GetScaledSizeX(hDlg, 60);
			lvc.pszText = L"SHIFT";
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
		for(int i = 0; i < PRESERVEDKEY_NUM; i++)
		{
			hWndListView = GetDlgItem(hDlg, preservedkeyTextInfo[i].id);

			ListView_SetColumnWidth(hWndListView, 0, GetScaledSizeX(hDlg, 90));
			ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 60));
			ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 60));
			ListView_SetColumnWidth(hWndListView, 3, GetScaledSizeX(hDlg, 60));
		}

		return TRUE;

	case WM_COMMAND:
		hWndListView = GetDlgItem(hDlg,
			IsDlgButtonChecked(hDlg, IDC_RADIO_PRSRVKEY_ON) ? IDC_LIST_PRSRVKEY_ON : IDC_LIST_PRSRVKEY_OFF);
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_PRSRVKEY_W:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if(index >= 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key, _countof(key));
				_snwprintf_s(key, _TRUNCATE, L"0x%02X", wcstoul(key, nullptr, 0));
				SetDlgItemTextW(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key);
				ListView_SetItemText(hWndListView, index, 0, key);
				ListView_SetItemText(hWndListView, index, 1,
					IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT) == BST_CHECKED ? L"1" : L"0");
				ListView_SetItemText(hWndListView, index, 2,
					IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL) == BST_CHECKED ? L"1" : L"0");
				ListView_SetItemText(hWndListView, index, 3,
					IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT) == BST_CHECKED ? L"1" : L"0");
			}
			else if(count < MAX_PRESERVEDKEY)
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
				item.pszText = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT) == BST_CHECKED ? L"1" : L"0";
				item.iItem = count;
				item.iSubItem = 1;
				ListView_SetItem(hWndListView, &item);
				item.pszText = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL) == BST_CHECKED ? L"1" : L"0";
				item.iItem = count;
				item.iSubItem = 2;
				ListView_SetItem(hWndListView, &item);
				item.pszText = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT) == BST_CHECKED ? L"1" : L"0";
				item.iItem = count;
				item.iSubItem = 3;
				ListView_SetItem(hWndListView, &item);
			}
			return TRUE;

		case IDC_BUTTON_PRSRVKEY_D:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index != -1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_DeleteItem(hWndListView, index);
			}
			return TRUE;

		case IDC_BUTTON_PRSRVKEY_UP:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index > 0)
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
			if(index >= 0 && index < count - 1)
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
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_TRANSLATEACCELERATOR:
			{
				LPMSG lpMsg = (LPMSG)((LPPSHNOTIFY)lParam)->lParam;
				switch(lpMsg->message)
				{
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					switch(GetDlgCtrlID(lpMsg->hwnd))
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
			switch(((LPNMHDR)lParam)->idFrom)
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
			switch(((LPNMHDR)lParam)->idFrom)
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
			if(pListView->uChanged & LVIF_STATE)
			{
				index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
				if(index == -1)
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

		case PSN_APPLY:
			SavePreservedKey(hDlg);
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
