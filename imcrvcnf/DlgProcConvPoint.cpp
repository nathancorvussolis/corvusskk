
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

WCHAR conv_point[MAX_CONV_POINT][3][2];

void LoadConvPoint(HWND hDlg);

INT_PTR CALLBACK DlgProcConvPoint(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	int index, count;
	WCHAR key[2];
	WCHAR keyBak[2];
	NMLISTVIEW *pListView;
	WCHAR text[16] = {};

	switch(message)
	{
	case WM_INITDIALOG:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_CONVPOINT);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_CENTER;
		lvc.pszText = text;

		lvc.iSubItem = 0;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		wcsncpy_s(text, L"開始", _TRUNCATE);
		ListView_InsertColumn(hWndListView, 0, &lvc);
		lvc.iSubItem = 1;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		wcsncpy_s(text, L"代替", _TRUNCATE);
		ListView_InsertColumn(hWndListView, 1, &lvc);
		lvc.iSubItem = 2;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		wcsncpy_s(text, L"送り", _TRUNCATE);
		ListView_InsertColumn(hWndListView, 2, &lvc);

		SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_ST, L"");
		SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_AL, L"");
		SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_OK, L"");

		LoadConvPoint(hDlg);

		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_CONVPOINT);

		ListView_SetColumnWidth(hWndListView, 0, GetScaledSizeX(hDlg, 60));
		ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 60));
		ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 60));

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

				GetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_ST, key, _countof(key));
				SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_ST, key);
				ListView_SetItemText(hWndListView, index, 0, key);
				GetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_AL, key, _countof(key));
				SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_AL, key);
				ListView_SetItemText(hWndListView, index, 1, key);
				GetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_OK, key, _countof(key));
				SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_OK, key);
				ListView_SetItemText(hWndListView, index, 2, key);
			}
			else if(count < MAX_CONV_POINT)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				item.mask = LVIF_TEXT;
				GetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_ST, key, _countof(key));
				SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_ST, key);
				item.pszText = key;
				item.iItem = count;
				item.iSubItem = 0;
				ListView_InsertItem(hWndListView, &item);
				GetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_AL, key, _countof(key));
				SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_AL, key);
				item.pszText = key;
				item.iItem = count;
				item.iSubItem = 1;
				ListView_SetItem(hWndListView, &item);
				GetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_OK, key, _countof(key));
				SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_OK, key);
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
					SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_ST, L"");
					SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_AL, L"");
					SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_OK, L"");
				}
				else
				{
					ListView_GetItemText(hWndListView, index, 0, key, _countof(key));
					SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_ST, key);
					ListView_GetItemText(hWndListView, index, 1, key, _countof(key));
					SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_AL, key);
					ListView_GetItemText(hWndListView, index, 2, key, _countof(key));
					SetDlgItemTextW(hDlg, IDC_EDIT_CONVPOINT_OK, key);
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

void LoadConfigConvPoint()
{
	APPDATAXMLLIST list;

	ZeroMemory(conv_point, sizeof(conv_point));

	HRESULT hr = ReadList(pathconfigxml, SectionConvPoint, list);

	if(SUCCEEDED(hr) && list.size() != 0)
	{
		int i = 0;
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= MAX_CONV_POINT)
			{
				break;
			}

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				if(r_itr->first == AttributeCPStart)
				{
					conv_point[i][0][0] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPAlter)
				{
					conv_point[i][1][0] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPOkuri)
				{
					conv_point[i][2][0] = r_itr->second.c_str()[0];
				}
			}

			i++;
		}
	}
	else if(FAILED(hr))
	{
		for(int i = 0; i < 26; i++)
		{
			conv_point[i][0][0] = L'A' + (WCHAR)i;
			conv_point[i][1][0] = L'a' + (WCHAR)i;
			conv_point[i][2][0] = L'a' + (WCHAR)i;
		}
	}
}

void LoadConvPoint(HWND hDlg)
{
	LVITEMW item;

	LoadConfigConvPoint();

	HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_CONVPOINT);

	for(int i = 0; i < MAX_CONV_POINT; i++)
	{
		if(conv_point[i][0][0] == L'\0' &&
			conv_point[i][1][0] == L'\0' &&
			conv_point[i][2][0] == L'\0')
		{
			break;
		}

		item.mask = LVIF_TEXT;
		item.pszText = conv_point[i][0];
		item.iItem = i;
		item.iSubItem = 0;
		ListView_InsertItem(hWndListView, &item);
		item.pszText = conv_point[i][1];
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem(hWndListView, &item);
		item.pszText = conv_point[i][2];
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem(hWndListView, &item);
	}
}

void SaveConvPoint(IXmlWriter *pWriter, HWND hDlg)
{
	APPDATAXMLLIST list;
	APPDATAXMLROW row;
	APPDATAXMLATTR attr;
	WCHAR key[2];

	HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_CONVPOINT);
	int count = ListView_GetItemCount(hWndListView);

	for(int i = 0; i < count && i < MAX_CONV_POINT; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, key, _countof(key));
		wcsncpy_s(conv_point[i][0], key, _TRUNCATE);
		ListView_GetItemText(hWndListView, i, 1, key, _countof(key));
		wcsncpy_s(conv_point[i][1], key, _TRUNCATE);
		ListView_GetItemText(hWndListView, i, 2, key, _countof(key));
		wcsncpy_s(conv_point[i][2], key, _TRUNCATE);
	}
	if(count < MAX_CONV_POINT)
	{
		conv_point[count][0][0] = L'\0';
		conv_point[count][1][0] = L'\0';
		conv_point[count][2][0] = L'\0';
	}

	for(int i = 0; i < MAX_CONV_POINT; i++)
	{
		if(conv_point[i][0][0] == L'\0' &&
			conv_point[i][1][0] == L'\0' &&
			conv_point[i][2][0] == L'\0')
		{
			break;
		}

		attr.first = AttributeCPStart;
		attr.second = conv_point[i][0];
		row.push_back(attr);

		attr.first = AttributeCPAlter;
		attr.second = conv_point[i][1];
		row.push_back(attr);

		attr.first = AttributeCPOkuri;
		attr.second = conv_point[i][2];
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pWriter, list);
}
