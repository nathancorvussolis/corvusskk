
#include "configxml.h"
#include "imcrvcnf.h"
#include "convtable.h"
#include "resource.h"

#define CONFKANALEN 0x100

std::vector<ROMAN_KANA_CONV> roman_kana_conv;

void LoadKana(HWND hDlg);
void LoadConfigKanaTxt(LPCWSTR path);
void LoadKanaTxt(HWND hDlg, LPCWSTR path);
void SaveKanaTxt(HWND hDlg, LPCWSTR path);

INT_PTR CALLBACK DlgProcKana(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	int index, count;
	ROMAN_KANA_CONV rkc;
	ROMAN_KANA_CONV rkcBak;
	WCHAR soku[2];
	NMLISTVIEW *pListView;
	OPENFILENAMEW ofn = {};
	WCHAR path[MAX_PATH];

	switch(message)
	{
	case WM_INITDIALOG:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_KANATBL);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_CENTER;

		lvc.iSubItem = 0;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		lvc.pszText = L"ﾛｰﾏ字";
		ListView_InsertColumn(hWndListView, 0, &lvc);
		lvc.iSubItem = 1;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		lvc.pszText = L"かな";
		ListView_InsertColumn(hWndListView, 1, &lvc);
		lvc.iSubItem = 2;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		lvc.pszText = L"カナ";
		ListView_InsertColumn(hWndListView, 2, &lvc);
		lvc.iSubItem = 3;
		lvc.cx = GetScaledSizeX(hDlg, 60);
		lvc.pszText = L"ｶﾅ";
		ListView_InsertColumn(hWndListView, 3, &lvc);
		lvc.iSubItem = 4;
		lvc.cx = GetScaledSizeX(hDlg, 30);
		lvc.pszText = L"…";
		ListView_InsertColumn(hWndListView, 4, &lvc);

		SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_R, L"");
		SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_H, L"");
		SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_K, L"");
		SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_KA, L"");
		CheckDlgButton(hDlg, IDC_CHECKBOX_KANATBL_SOKU, BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKBOX_KANATBL_WAIT, BST_UNCHECKED);

		LoadKana(hDlg);

		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_KANATBL);

		ListView_SetColumnWidth(hWndListView, 0, GetScaledSizeX(hDlg, 60));
		ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 60));
		ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 60));
		ListView_SetColumnWidth(hWndListView, 3, GetScaledSizeX(hDlg, 60));
		ListView_SetColumnWidth(hWndListView, 4, GetScaledSizeX(hDlg, 30));

		return TRUE;

	case WM_COMMAND:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_KANATBL);
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_LOADKANA:
			path[0] = L'\0';

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = path;
			ofn.nMaxFile = _countof(path);
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrTitle = L"Load Kana Table File";
			ofn.lpstrFilter = L"*.txt\0*.txt\0" L"*.*\0*.*\0\0";

			if(GetOpenFileNameW(&ofn))
			{
				LoadKanaTxt(hDlg, ofn.lpstrFile);
				PropSheet_Changed(GetParent(hDlg), hDlg);
			}
			break;

		case IDC_BUTTON_SAVEKANA:
			path[0] = L'\0';

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = path;
			ofn.nMaxFile = _countof(path);
			ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
			ofn.lpstrTitle = L"Save Kana Table File";
			ofn.lpstrFilter = L"*.txt\0*.txt\0" L"*.*\0*.*\0\0";
			ofn.lpstrDefExt = L"txt";

			if(GetSaveFileNameW(&ofn))
			{
				SaveKanaTxt(hDlg, ofn.lpstrFile);
				MessageBoxW(hDlg, L"完了しました。", TextServiceDesc, MB_OK | MB_ICONINFORMATION);
			}
			break;

		case IDC_BUTTON_KANATBL_W:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if(index >= 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_R, rkc.roman, _countof(rkc.roman));
				GetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_H, rkc.hiragana, _countof(rkc.hiragana));
				GetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_K, rkc.katakana, _countof(rkc.katakana));
				GetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_KA, rkc.katakana_ank, _countof(rkc.katakana_ank));
				IsDlgButtonChecked(hDlg, IDC_CHECKBOX_KANATBL_SOKU) == BST_CHECKED ? rkc.soku = TRUE : rkc.soku = FALSE;
				IsDlgButtonChecked(hDlg, IDC_CHECKBOX_KANATBL_WAIT) == BST_CHECKED ? rkc.wait = TRUE : rkc.wait = FALSE;
				SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_R, rkc.roman);
				SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_H, rkc.hiragana);
				SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_K, rkc.katakana);
				SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_KA, rkc.katakana_ank);
				ListView_SetItemText(hWndListView, index, 0, rkc.roman);
				ListView_SetItemText(hWndListView, index, 1, rkc.hiragana);
				ListView_SetItemText(hWndListView, index, 2, rkc.katakana);
				ListView_SetItemText(hWndListView, index, 3, rkc.katakana_ank);
				soku[1] = L'\0';
				soku[0] = L'0' + (rkc.soku ? 1 : 0) + (rkc.wait ? 2 : 0);
				ListView_SetItemText(hWndListView, index, 4, soku);
			}
			else if(count < ROMAN_KANA_TBL_MAX)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_R, rkc.roman, _countof(rkc.roman));
				GetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_H, rkc.hiragana, _countof(rkc.hiragana));
				GetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_K, rkc.katakana, _countof(rkc.katakana));
				GetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_KA, rkc.katakana_ank, _countof(rkc.katakana_ank));
				IsDlgButtonChecked(hDlg, IDC_CHECKBOX_KANATBL_SOKU) == BST_CHECKED ? rkc.soku = TRUE : rkc.soku = FALSE;
				IsDlgButtonChecked(hDlg, IDC_CHECKBOX_KANATBL_WAIT) == BST_CHECKED ? rkc.wait = TRUE : rkc.wait = FALSE;
				SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_R, rkc.roman);
				SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_H, rkc.hiragana);
				SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_K, rkc.katakana);
				SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_KA, rkc.katakana_ank);
				item.mask = LVIF_TEXT;
				item.pszText = rkc.roman;
				item.iItem = count;
				item.iSubItem = 0;
				ListView_InsertItem(hWndListView, &item);
				item.pszText = rkc.hiragana;
				item.iItem = count;
				item.iSubItem = 1;
				ListView_SetItem(hWndListView, &item);
				item.pszText = rkc.katakana;
				item.iItem = count;
				item.iSubItem = 2;
				ListView_SetItem(hWndListView, &item);
				item.pszText = rkc.katakana_ank;
				item.iItem = count;
				item.iSubItem = 3;
				ListView_SetItem(hWndListView, &item);
				soku[1] = L'\0';
				soku[0] = L'0' + (rkc.soku ? 1 : 0) + (rkc.wait ? 2 : 0);
				item.pszText = soku;
				item.iItem = count;
				item.iSubItem = 4;
				ListView_SetItem(hWndListView, &item);
			}
			return TRUE;

		case IDC_BUTTON_KANATBL_D:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index != -1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_DeleteItem(hWndListView, index);
			}
			return TRUE;

		case IDC_BUTTON_KANATBL_UP:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index > 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_GetItemText(hWndListView, index - 1, 0, rkcBak.roman, _countof(rkcBak.roman));
				ListView_GetItemText(hWndListView, index - 1, 1, rkcBak.hiragana, _countof(rkcBak.hiragana));
				ListView_GetItemText(hWndListView, index - 1, 2, rkcBak.katakana, _countof(rkcBak.katakana));
				ListView_GetItemText(hWndListView, index - 1, 3, rkcBak.katakana_ank, _countof(rkcBak.katakana_ank));
				ListView_GetItemText(hWndListView, index - 1, 4, soku, _countof(soku));
				((soku[0] - L'0') & 0x1) ? rkcBak.soku = TRUE : rkcBak.soku = FALSE;
				((soku[0] - L'0') & 0x2) ? rkcBak.wait = TRUE : rkcBak.wait = FALSE;
				ListView_GetItemText(hWndListView, index, 0, rkc.roman, _countof(rkc.roman));
				ListView_GetItemText(hWndListView, index, 1, rkc.hiragana, _countof(rkc.hiragana));
				ListView_GetItemText(hWndListView, index, 2, rkc.katakana, _countof(rkc.katakana));
				ListView_GetItemText(hWndListView, index, 3, rkc.katakana_ank, _countof(rkc.katakana_ank));
				ListView_GetItemText(hWndListView, index, 4, soku, _countof(soku));
				((soku[0] - L'0') & 0x1) ? rkc.soku = TRUE : rkc.soku = FALSE;
				((soku[0] - L'0') & 0x2) ? rkc.wait = TRUE : rkc.wait = FALSE;
				ListView_SetItemText(hWndListView, index - 1, 0, rkc.roman);
				ListView_SetItemText(hWndListView, index - 1, 1, rkc.hiragana);
				ListView_SetItemText(hWndListView, index - 1, 2, rkc.katakana);
				ListView_SetItemText(hWndListView, index - 1, 3, rkc.katakana_ank);
				soku[1] = L'\0';
				soku[0] = L'0' + (rkc.soku ? 1 : 0) + (rkc.wait ? 2 : 0);
				ListView_SetItemText(hWndListView, index - 1, 4, soku);
				ListView_SetItemText(hWndListView, index, 0, rkcBak.roman);
				ListView_SetItemText(hWndListView, index, 1, rkcBak.hiragana);
				ListView_SetItemText(hWndListView, index, 2, rkcBak.katakana);
				ListView_SetItemText(hWndListView, index, 3, rkcBak.katakana_ank);
				soku[1] = L'\0';
				soku[0] = L'0' + (rkcBak.soku ? 1 : 0) + (rkcBak.wait ? 2 : 0);
				ListView_SetItemText(hWndListView, index, 4, soku);
				ListView_SetItemState(hWndListView, index - 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
			}
			return TRUE;

		case IDC_BUTTON_KANATBL_DOWN:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if(index >= 0 && index < count - 1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_GetItemText(hWndListView, index + 1, 0, rkcBak.roman, _countof(rkcBak.roman));
				ListView_GetItemText(hWndListView, index + 1, 1, rkcBak.hiragana, _countof(rkcBak.hiragana));
				ListView_GetItemText(hWndListView, index + 1, 2, rkcBak.katakana, _countof(rkcBak.katakana));
				ListView_GetItemText(hWndListView, index + 1, 3, rkcBak.katakana_ank, _countof(rkcBak.katakana_ank));
				ListView_GetItemText(hWndListView, index + 1, 4, soku, _countof(soku));
				((soku[0] - L'0') & 0x1) ? rkcBak.soku = TRUE : rkcBak.soku = FALSE;
				((soku[0] - L'0') & 0x2) ? rkcBak.wait = TRUE : rkcBak.wait = FALSE;
				ListView_GetItemText(hWndListView, index, 0, rkc.roman, _countof(rkc.roman));
				ListView_GetItemText(hWndListView, index, 1, rkc.hiragana, _countof(rkc.hiragana));
				ListView_GetItemText(hWndListView, index, 2, rkc.katakana, _countof(rkc.katakana));
				ListView_GetItemText(hWndListView, index, 3, rkc.katakana_ank, _countof(rkc.katakana_ank));
				ListView_GetItemText(hWndListView, index, 4, soku, _countof(soku));
				((soku[0] - L'0') & 0x1) ? rkc.soku = TRUE : rkc.soku = FALSE;
				((soku[0] - L'0') & 0x2) ? rkc.wait = TRUE : rkc.wait = FALSE;
				ListView_SetItemText(hWndListView, index + 1, 0, rkc.roman);
				ListView_SetItemText(hWndListView, index + 1, 1, rkc.hiragana);
				ListView_SetItemText(hWndListView, index + 1, 2, rkc.katakana);
				ListView_SetItemText(hWndListView, index + 1, 3, rkc.katakana_ank);
				soku[1] = L'\0';
				soku[0] = L'0' + (rkc.soku ? 1 : 0) + (rkc.wait ? 2 : 0);
				ListView_SetItemText(hWndListView, index + 1, 4, soku);
				ListView_SetItemText(hWndListView, index, 0, rkcBak.roman);
				ListView_SetItemText(hWndListView, index, 1, rkcBak.hiragana);
				ListView_SetItemText(hWndListView, index, 2, rkcBak.katakana);
				ListView_SetItemText(hWndListView, index, 3, rkcBak.katakana_ank);
				soku[1] = L'\0';
				soku[0] = L'0' + (rkcBak.soku ? 1 : 0) + (rkcBak.wait ? 2 : 0);
				ListView_SetItemText(hWndListView, index, 4, soku);
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
					SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_R, L"");
					SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_H, L"");
					SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_K, L"");
					SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_KA, L"");
					CheckDlgButton(hDlg, IDC_CHECKBOX_KANATBL_SOKU, BST_UNCHECKED);
					CheckDlgButton(hDlg, IDC_CHECKBOX_KANATBL_WAIT, BST_UNCHECKED);
				}
				else
				{
					ListView_GetItemText(hWndListView, index, 0, rkc.roman, _countof(rkc.roman));
					ListView_GetItemText(hWndListView, index, 1, rkc.hiragana, _countof(rkc.roman));
					ListView_GetItemText(hWndListView, index, 2, rkc.katakana, _countof(rkc.roman));
					ListView_GetItemText(hWndListView, index, 3, rkc.katakana_ank, _countof(rkc.roman));
					ListView_GetItemText(hWndListView, index, 4, soku, _countof(soku));
					((soku[0] - L'0') & 0x1) ? rkc.soku = TRUE : rkc.soku = FALSE;
					((soku[0] - L'0') & 0x2) ? rkc.wait = TRUE : rkc.wait = FALSE;
					SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_R, rkc.roman);
					SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_H, rkc.hiragana);
					SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_K, rkc.katakana);
					SetDlgItemTextW(hDlg, IDC_EDIT_KANATBL_KA, rkc.katakana_ank);
					CheckDlgButton(hDlg, IDC_CHECKBOX_KANATBL_SOKU, (rkc.soku ? BST_CHECKED : BST_UNCHECKED));
					CheckDlgButton(hDlg, IDC_CHECKBOX_KANATBL_WAIT, (rkc.wait ? BST_CHECKED : BST_UNCHECKED));
				}
				return TRUE;
			}
			break;

		default:
			break;
		}
		break;
	}

	return FALSE;
}

void LoadConfigKanaTxt(LPCWSTR path)
{
	FILE *fp;
	wchar_t b[CONFKANALEN];
	const wchar_t seps[] = L"\t\n\0";
	size_t sidx, eidx;
	WCHAR soku[2];

	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();

	_wfopen_s(&fp, path, RccsUTF8);
	if(fp == nullptr)
	{
		return;
	}

	ZeroMemory(b, sizeof(b));

	while(fgetws(b, CONFKANALEN, fp) != nullptr)
	{
		if(roman_kana_conv.size() >= ROMAN_KANA_TBL_MAX)
		{
			break;
		}

		ROMAN_KANA_CONV rkc = {};

		sidx = 0;
		eidx = wcscspn(&b[sidx], seps);

		for(int i = 0; i <= 4; i++)
		{
			if(sidx + eidx >= _countof(b))
			{
				break;
			}
			b[sidx + eidx] = L'\0';

			switch(i)
			{
			case 0:
				_snwprintf_s(rkc.roman, _TRUNCATE, L"%s", &b[sidx]);
				break;
			case 1:
				_snwprintf_s(rkc.hiragana, _TRUNCATE, L"%s", &b[sidx]);
				break;
			case 2:
				_snwprintf_s(rkc.katakana, _TRUNCATE, L"%s", &b[sidx]);
				break;
			case 3:
				_snwprintf_s(rkc.katakana_ank, _TRUNCATE, L"%s", &b[sidx]);
				break;
			case 4:
				_snwprintf_s(soku, _TRUNCATE, L"%s", &b[sidx]);
				rkc.soku = (_wtoi(soku) & 0x1) ? TRUE : FALSE;
				rkc.wait = (_wtoi(soku) & 0x2) ? TRUE : FALSE;
				break;
			default:
				break;
			}

			sidx += eidx + 1;
			if(sidx >= _countof(b))
			{
				break;
			}
			eidx = wcscspn(&b[sidx], seps);
		}

		ZeroMemory(b, sizeof(b));

		if(rkc.roman[0] == L'\0' &&
			rkc.hiragana[0] == L'\0' &&
			rkc.katakana[0] == L'\0' &&
			rkc.katakana_ank[0] == L'\0')
		{
			continue;
		}

		roman_kana_conv.push_back(rkc);
	}

	fclose(fp);
}

void LoadKanaTxt(HWND hDlg, LPCWSTR path)
{
	LVITEMW item;
	WCHAR soku[2];

	LoadConfigKanaTxt(path);

	HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_KANATBL);
	ListView_DeleteAllItems(hWndListView);
	int count = (int)roman_kana_conv.size();

	for(int i = 0; i < count; i++)
	{
		item.mask = LVIF_TEXT;
		item.pszText = roman_kana_conv[i].roman;
		item.iItem = i;
		item.iSubItem = 0;
		ListView_InsertItem(hWndListView, &item);
		item.pszText = roman_kana_conv[i].hiragana;
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem(hWndListView, &item);
		item.pszText = roman_kana_conv[i].katakana;
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem(hWndListView, &item);
		item.pszText = roman_kana_conv[i].katakana_ank;
		item.iItem = i;
		item.iSubItem = 3;
		ListView_SetItem(hWndListView, &item);
		soku[0] = L'0' + (roman_kana_conv[i].soku ? 1 : 0) + (roman_kana_conv[i].wait ? 2 : 0);
		soku[1] = L'\0';
		item.pszText = soku;
		item.iItem = i;
		item.iSubItem = 4;
		ListView_SetItem(hWndListView, &item);
	}
}

void SaveKanaTxt(HWND hDlg, LPCWSTR path)
{
	ROMAN_KANA_CONV rkc;
	WCHAR soku[2];
	FILE *fp;

	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();

	HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_KANATBL);
	int count = ListView_GetItemCount(hWndListView);

	for(int i = 0; i < count && i < ROMAN_KANA_TBL_MAX; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, rkc.roman, _countof(rkc.roman));
		ListView_GetItemText(hWndListView, i, 1, rkc.hiragana, _countof(rkc.hiragana));
		ListView_GetItemText(hWndListView, i, 2, rkc.katakana, _countof(rkc.katakana));
		ListView_GetItemText(hWndListView, i, 3, rkc.katakana_ank, _countof(rkc.katakana_ank));
		ListView_GetItemText(hWndListView, i, 4, soku, _countof(soku));
		((soku[0] - L'0') & 1) ? rkc.soku = TRUE : rkc.soku = FALSE;
		((soku[0] - L'0') & 2) ? rkc.wait = TRUE : rkc.wait = FALSE;

		roman_kana_conv.push_back(rkc);
	}

	_wfopen_s(&fp, path, WccsUTF8);
	if(fp != nullptr)
	{
		count = (int)roman_kana_conv.size();

		for(int i = 0; i < count; i++)
		{
			if(roman_kana_conv[i].roman[0] == L'\0' &&
				roman_kana_conv[i].hiragana[0] == L'\0' &&
				roman_kana_conv[i].katakana[0] == L'\0' &&
				roman_kana_conv[i].katakana_ank[0] == L'\0')
			{
				continue;
			}
			fwprintf(fp, L"%s\t%s\t%s\t%s\t%d\n",
				roman_kana_conv[i].roman,
				roman_kana_conv[i].hiragana,
				roman_kana_conv[i].katakana,
				roman_kana_conv[i].katakana_ank,
				roman_kana_conv[i].soku | (roman_kana_conv[i].wait << 1));
		}

		fclose(fp);
	}
}

void LoadConfigKana()
{
	APPDATAXMLLIST list;

	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();

	HRESULT hr = ReadList(pathconfigxml, SectionKana, list);

	if(SUCCEEDED(hr) && list.size() != 0)
	{
		int i = 0;
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= ROMAN_KANA_TBL_MAX)
			{
				break;
			}

			ROMAN_KANA_CONV rkc = {};

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				WCHAR *pszb = nullptr;
				size_t blen = 0;

				if(r_itr->first == AttributeRoman)
				{
					pszb = rkc.roman;
					blen = _countof(rkc.roman);
				}
				else if(r_itr->first == AttributeHiragana)
				{
					pszb = rkc.hiragana;
					blen = _countof(rkc.hiragana);
				}
				else if(r_itr->first == AttributeKatakana)
				{
					pszb = rkc.katakana;
					blen = _countof(rkc.katakana);
				}
				else if(r_itr->first == AttributeKatakanaAnk)
				{
					pszb = rkc.katakana_ank;
					blen = _countof(rkc.katakana_ank);
				}
				else if(r_itr->first == AttributeSpOp)
				{
					rkc.soku = (_wtoi(r_itr->second.c_str()) & 0x1) ? TRUE : FALSE;
					rkc.wait = (_wtoi(r_itr->second.c_str()) & 0x2) ? TRUE : FALSE;
				}

				if(pszb != nullptr)
				{
					wcsncpy_s(pszb, blen, r_itr->second.c_str(), _TRUNCATE);
				}
			}

			roman_kana_conv.push_back(rkc);
			i++;
		}
	}
	else if(FAILED(hr))
	{
		for(int i = 0; i < ROMAN_KANA_TBL_DEF_NUM; i++)
		{
			if(roman_kana_conv_default[i].roman[0] == L'\0')
			{
				break;
			}
			roman_kana_conv.push_back(roman_kana_conv_default[i]);
		}
	}
}

void LoadKana(HWND hDlg)
{
	LVITEMW item;
	WCHAR soku[2];

	LoadConfigKana();

	HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_KANATBL);
	int count = (int)roman_kana_conv.size();

	for(int i = 0; i < count; i++)
	{
		item.mask = LVIF_TEXT;
		item.pszText = roman_kana_conv[i].roman;
		item.iItem = i;
		item.iSubItem = 0;
		ListView_InsertItem(hWndListView, &item);
		item.pszText = roman_kana_conv[i].hiragana;
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem(hWndListView, &item);
		item.pszText = roman_kana_conv[i].katakana;
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem(hWndListView, &item);
		item.pszText = roman_kana_conv[i].katakana_ank;
		item.iItem = i;
		item.iSubItem = 3;
		ListView_SetItem(hWndListView, &item);
		soku[0] = L'0' + (roman_kana_conv[i].soku ? 1 : 0) + (roman_kana_conv[i].wait ? 2 : 0);
		soku[1] = L'\0';
		item.pszText = soku;
		item.iItem = i;
		item.iSubItem = 4;
		ListView_SetItem(hWndListView, &item);
	}
}

void SaveKana(IXmlWriter *pWriter, HWND hDlg)
{
	APPDATAXMLLIST list;
	APPDATAXMLROW row;
	APPDATAXMLATTR attr;
	ROMAN_KANA_CONV rkc;
	WCHAR soku[2];

	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();

	HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_KANATBL);
	int count = ListView_GetItemCount(hWndListView);

	for(int i = 0; i < count && i < ROMAN_KANA_TBL_MAX; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, rkc.roman, _countof(rkc.roman));
		ListView_GetItemText(hWndListView, i, 1, rkc.hiragana, _countof(rkc.hiragana));
		ListView_GetItemText(hWndListView, i, 2, rkc.katakana, _countof(rkc.katakana));
		ListView_GetItemText(hWndListView, i, 3, rkc.katakana_ank, _countof(rkc.katakana_ank));
		ListView_GetItemText(hWndListView, i, 4, soku, _countof(soku));
		((soku[0] - L'0') & 0x1) != 0 ? rkc.soku = TRUE : rkc.soku = FALSE;
		((soku[0] - L'0') & 0x2) != 0 ? rkc.wait = TRUE : rkc.wait = FALSE;

		roman_kana_conv.push_back(rkc);
	}

	for(int i = 0; i < count; i++)
	{
		attr.first = AttributeRoman;
		attr.second = roman_kana_conv[i].roman;
		row.push_back(attr);

		attr.first = AttributeHiragana;
		attr.second = roman_kana_conv[i].hiragana;
		row.push_back(attr);

		attr.first = AttributeKatakana;
		attr.second = roman_kana_conv[i].katakana;
		row.push_back(attr);

		attr.first = AttributeKatakanaAnk;
		attr.second = roman_kana_conv[i].katakana_ank;
		row.push_back(attr);

		attr.first = AttributeSpOp;
		soku[0] = L'0' + (roman_kana_conv[i].soku ? 1 : 0) + (roman_kana_conv[i].wait ? 2 : 0);
		soku[1] = L'\0';
		attr.second = soku;
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pWriter, list);
}
