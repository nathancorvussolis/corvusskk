
#include "imcrvcnf.h"
#include "configxml.h"
#include "resource.h"

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
		hWndListView = GetDlgItem(hDlg, IDC_LIST_PRSRVKEY);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_CENTER;

		lvc.iSubItem = 0;
		lvc.cx = 60;
		lvc.pszText = L"仮想ｷｰ";
		ListView_InsertColumn(hWndListView, 0, &lvc);
		lvc.iSubItem = 1;
		lvc.cx = 60;
		lvc.pszText = L"ALT";
		ListView_InsertColumn(hWndListView, 1, &lvc);
		lvc.iSubItem = 2;
		lvc.cx = 60;
		lvc.pszText = L"CTRL";
		ListView_InsertColumn(hWndListView, 2, &lvc);
		lvc.iSubItem = 3;
		lvc.cx = 60;
		lvc.pszText = L"SHIFT";
		ListView_InsertColumn(hWndListView, 3, &lvc);

		SetDlgItemText(hDlg, IDC_EDIT_PRSRVKEY_VKEY, L"");
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT, BST_UNCHECKED);

		LoadPreservedKey(hDlg);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_PRSRVKEY);
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_PRSRVKEY_W:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if(index >= 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetDlgItemText(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key, _countof(key));
				_snwprintf_s(key, _TRUNCATE, L"0x%02X", wcstoul(key, NULL, 0));
				SetDlgItemText(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key);
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
				GetDlgItemText(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key, _countof(key));
				_snwprintf_s(key, _TRUNCATE, L"0x%02X", wcstoul(key, NULL, 0));
				SetDlgItemText(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key);
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
			return (INT_PTR)TRUE;

		case IDC_BUTTON_PRSRVKEY_D:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index != -1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_DeleteItem(hWndListView, index);
			}
			return (INT_PTR)TRUE;

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
			return (INT_PTR)TRUE;

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
			return (INT_PTR)TRUE;

		default:
			break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case LVN_ITEMCHANGED:
			pListView = (NMLISTVIEW*)((LPNMHDR)lParam);
			if(pListView->uChanged & LVIF_STATE)
			{
				hWndListView = ((LPNMHDR)lParam)->hwndFrom;
				index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
				if(index == -1)
				{
					SetDlgItemText(hDlg, IDC_EDIT_PRSRVKEY_VKEY, L"");
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT, BST_UNCHECKED);
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL, BST_UNCHECKED);
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT, BST_UNCHECKED);
				}
				else
				{
					ListView_GetItemText(hWndListView, index, 0, key, _countof(key));
					SetDlgItemText(hDlg, IDC_EDIT_PRSRVKEY_VKEY, key);
					ListView_GetItemText(hWndListView, index, 1, key, _countof(key));
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_ALT, key[0] == L'1' ? BST_CHECKED : BST_UNCHECKED);
					ListView_GetItemText(hWndListView, index, 2, key, _countof(key));
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL, key[0] == L'1' ? BST_CHECKED : BST_UNCHECKED);
					ListView_GetItemText(hWndListView, index, 3, key, _countof(key));
					CheckDlgButton(hDlg, IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT, key[0] == L'1' ? BST_CHECKED : BST_UNCHECKED);					
				}
				return (INT_PTR)TRUE;
			}
			break;

		case PSN_APPLY:
			SavePreservedKey(hDlg);
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
