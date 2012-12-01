
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

INT_PTR CALLBACK DlgProcConvPoint(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	int index, count;
	WCHAR key[2];
	WCHAR keyBak[2];
	NMLISTVIEW *pListView;

	switch(message)
	{
	case WM_INITDIALOG:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_CONVPOINT);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_CENTER;

		lvc.iSubItem = 0;
		lvc.cx = 60;
		lvc.pszText = L"開始";
		ListView_InsertColumn(hWndListView, 0, &lvc);
		lvc.iSubItem = 1;
		lvc.cx = 60;
		lvc.pszText = L"代替";
		ListView_InsertColumn(hWndListView, 1, &lvc);
		lvc.iSubItem = 2;
		lvc.cx = 60;
		lvc.pszText = L"送り";
		ListView_InsertColumn(hWndListView, 2, &lvc);

		SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_ST, L"");
		SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_AL, L"");
		SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_OK, L"");

		LoadConvPoint(hDlg);

		return TRUE;

	case WM_COMMAND:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_CONVPOINT);
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_CONVPOINT_W:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if(index >= 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_ST, key, _countof(key));
				SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_ST, key);
				ListView_SetItemText(hWndListView, index, 0, key);
				GetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_AL, key, _countof(key));
				SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_AL, key);
				ListView_SetItemText(hWndListView, index, 1, key);
				GetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_OK, key, _countof(key));
				SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_OK, key);
				ListView_SetItemText(hWndListView, index, 2, key);
			}
			else if(count < CONV_POINT_NUM)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				item.mask = LVIF_TEXT;
				GetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_ST, key, _countof(key));
				SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_ST, key);
				item.pszText = key;
				item.iItem = count;
				item.iSubItem = 0;
				ListView_InsertItem(hWndListView, &item);
				GetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_AL, key, _countof(key));
				SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_AL, key);
				item.pszText = key;
				item.iItem = count;
				item.iSubItem = 1;
				ListView_SetItem(hWndListView, &item);
				GetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_OK, key, _countof(key));
				SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_OK, key);
				item.pszText = key;
				item.iItem = count;
				item.iSubItem = 2;
				ListView_SetItem(hWndListView, &item);
			}
			return TRUE;

		case IDC_BUTTON_CONVPOINT_D:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index != -1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_DeleteItem(hWndListView, index);
			}
			return TRUE;

		case IDC_BUTTON_CONVPOINT_UP:
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

				ListView_SetItemState(hWndListView, index - 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
			}
			return TRUE;

		case IDC_BUTTON_CONVPOINT_DOWN:
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

				ListView_SetItemState(hWndListView, index + 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
			}
			return TRUE;

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
					SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_ST, L"");
					SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_AL, L"");
					SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_OK, L"");
				}
				else
				{
					ListView_GetItemText(hWndListView, index, 0, key, _countof(key));
					SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_ST, key);
					ListView_GetItemText(hWndListView, index, 1, key, _countof(key));
					SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_AL, key);
					ListView_GetItemText(hWndListView, index, 2, key, _countof(key));
					SetDlgItemText(hDlg, IDC_EDIT_CONVPOINT_OK, key);
				}
				return TRUE;
			}
			break;

		case PSN_APPLY:
			SaveConvPoint(hDlg);
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
