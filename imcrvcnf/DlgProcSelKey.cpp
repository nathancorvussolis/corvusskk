
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static LPCWSTR listSelKey[MAX_SELKEY_C] = {L"Aa",L"Ss",L"Dd",L"Ff",L"Jj",L"Kk",L"Ll",L"Gg",L"Hh"};

INT_PTR CALLBACK DlgProcSelKey(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	NMLISTVIEW *pListView;
	int index;
	WCHAR num[2];
	WCHAR key[4];
	std::wstring strxmlval;

	switch(message)
	{
	case WM_INITDIALOG:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SELKEY);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_CENTER;

		lvc.iSubItem = 0;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		lvc.pszText = L"数字";
		ListView_InsertColumn(hWndListView, 0, &lvc);
		lvc.iSubItem = 1;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		lvc.pszText = L"表示";
		ListView_InsertColumn(hWndListView, 1, &lvc);
		lvc.iSubItem = 2;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		lvc.pszText = L"予備";
		ListView_InsertColumn(hWndListView, 2, &lvc);

		SetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_DISP, L"");
		SetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_SPARE, L"");

		hWndListView = GetDlgItem(hDlg, IDC_LIST_SELKEY);
		for(index = 0; index < MAX_SELKEY_C; index++)
		{
			_snwprintf_s(num, _TRUNCATE, L"%d", index + 1);

			item.mask = LVIF_TEXT;
			item.pszText = num;
			item.iItem = index;
			item.iSubItem = 0;
			ListView_InsertItem(hWndListView, &item);

			ZeroMemory(key, sizeof(key));
			ReadValue(pathconfigxml, SectionSelKey, num, strxmlval);
			if(strxmlval.empty()) strxmlval = listSelKey[index];
			wcsncpy_s(key, strxmlval.c_str(), _TRUNCATE);

			num[0] = key[0];
			item.pszText = num;
			item.iItem = index;
			item.iSubItem = 1;
			ListView_SetItem(hWndListView, &item);
			num[0] = key[1];
			item.pszText = num;
			item.iItem = index;
			item.iSubItem = 2;
			ListView_SetItem(hWndListView, &item);
		}
		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SELKEY);

		ListView_SetColumnWidth(hWndListView, 0, GetScaledSizeX(hDlg, 60));
		ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 60));
		ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 60));

		return TRUE;

	case WM_COMMAND:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_SELKEY);
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_SELKEY_W:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index >= 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_DISP, num, _countof(num));
				if(num[0] == L'\0')
				{
					num[0] = L'1' + index;
					num[1] = L'\0';
				}
				SetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_DISP, num);
				ListView_SetItemText(hWndListView, index, 1, num);
				GetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_SPARE, num, _countof(num));
				SetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_SPARE, num);
				ListView_SetItemText(hWndListView, index, 2, num);

				return TRUE;
			}
			break;
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
					SetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_DISP, L"");
					SetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_SPARE, L"");
				}
				else
				{
					ListView_GetItemText(hWndListView, index, 1, num, _countof(num));
					SetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_DISP, num);
					ListView_GetItemText(hWndListView, index, 2, num, _countof(num));
					SetDlgItemTextW(hDlg, IDC_EDIT_SELKEY_SPARE, num);
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

void SaveSelKey(IXmlWriter *pWriter, HWND hDlg)
{
	HWND hWndListView;
	int index;
	WCHAR num[2];
	WCHAR key[4];

	hWndListView = GetDlgItem(hDlg, IDC_LIST_SELKEY);
	for (index = 0; index < MAX_SELKEY_C; index++)
	{
		ListView_GetItemText(hWndListView, index, 1, num, _countof(num));
		key[0] = num[0];
		ListView_GetItemText(hWndListView, index, 2, num, _countof(num));
		key[1] = num[0];
		key[2] = L'\0';
		_snwprintf_s(num, _TRUNCATE, L"%d", index + 1);
		WriterKey(pWriter, num, key);
	}
}
