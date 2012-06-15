
#include "corvuscnf.h"
#include "convtable.h"
#include "resource.h"

INT_PTR CALLBACK DlgProcJLatin(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndListView;
	LV_COLUMNW lvc;
	LVITEMW item;
	int index, count;
	ASCII_JLATIN_CONV ajc;
	ASCII_JLATIN_CONV ajcBak;
	NMLISTVIEW *pListView;

	switch(message)
	{
	case WM_INITDIALOG:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_JLATTBL);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_CENTER;

		lvc.iSubItem = 0;
		lvc.cx = 60;
		lvc.pszText = L"ASCII";
		ListView_InsertColumn(hWndListView, 0, &lvc);
		lvc.iSubItem = 1;
		lvc.cx = 60;
		lvc.pszText = L"全英";
		ListView_InsertColumn(hWndListView, 1, &lvc);

		SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_A, L"");
		SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_J, L"");

		LoadJLatin(hDlg);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		hWndListView = GetDlgItem(hDlg, IDC_LIST_JLATTBL);
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_JLATTBL_W:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if(index >= 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetDlgItemText(hDlg, IDC_EDIT_JLATTBL_A, ajc.ascii, _countof(ajc.ascii));
				GetDlgItemText(hDlg, IDC_EDIT_JLATTBL_J, ajc.jlatin, _countof(ajc.jlatin));
				SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_A, ajc.ascii);
				SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_J, ajc.jlatin);
				ListView_SetItemText(hWndListView, index, 0, ajc.ascii);
				ListView_SetItemText(hWndListView, index, 1, ajc.jlatin);
			}
			else if(count < ASCII_JLATIN_TBL_NUM)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				GetDlgItemText(hDlg, IDC_EDIT_JLATTBL_A, ajc.ascii, _countof(ajc.ascii));
				GetDlgItemText(hDlg, IDC_EDIT_JLATTBL_J, ajc.jlatin, _countof(ajc.jlatin));
				SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_A, ajc.ascii);
				SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_J, ajc.jlatin);
				item.mask = LVIF_TEXT;
				item.pszText = ajc.ascii;
				item.iItem = count;
				item.iSubItem = 0;
				ListView_InsertItem(hWndListView, &item);
				item.pszText = ajc.jlatin;
				item.iItem = count;
				item.iSubItem = 1;
				ListView_SetItem(hWndListView, &item);
			}
			return (INT_PTR)TRUE;

		case IDC_BUTTON_JLATTBL_D:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index != -1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_DeleteItem(hWndListView, index);
			}
			return (INT_PTR)TRUE;

		case IDC_BUTTON_JLATTBL_UP:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			if(index > 0)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_GetItemText(hWndListView, index - 1, 0, ajcBak.ascii, _countof(ajcBak.ascii));
				ListView_GetItemText(hWndListView, index - 1, 1, ajcBak.jlatin, _countof(ajcBak.jlatin));
				ListView_GetItemText(hWndListView, index, 0, ajc.ascii, _countof(ajc.ascii));
				ListView_GetItemText(hWndListView, index, 1, ajc.jlatin, _countof(ajc.jlatin));
				ListView_SetItemText(hWndListView, index - 1, 0, ajc.ascii);
				ListView_SetItemText(hWndListView, index - 1, 1, ajc.jlatin);
				ListView_SetItemText(hWndListView, index, 0, ajcBak.ascii);
				ListView_SetItemText(hWndListView, index, 1, ajcBak.jlatin);
				ListView_SetItemState(hWndListView, index - 1, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
			}
			return (INT_PTR)TRUE;

		case IDC_BUTTON_JLATTBL_DOWN:
			index = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
			count = ListView_GetItemCount(hWndListView);
			if(index >= 0 && index < count - 1)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				ListView_GetItemText(hWndListView, index + 1, 0, ajcBak.ascii, _countof(ajcBak.ascii));
				ListView_GetItemText(hWndListView, index + 1, 1, ajcBak.jlatin, _countof(ajcBak.jlatin));
				ListView_GetItemText(hWndListView, index, 0, ajc.ascii, _countof(ajc.ascii));
				ListView_GetItemText(hWndListView, index, 1, ajc.jlatin, _countof(ajc.jlatin));
				ListView_SetItemText(hWndListView, index + 1, 0, ajc.ascii);
				ListView_SetItemText(hWndListView, index + 1, 1, ajc.jlatin);
				ListView_SetItemText(hWndListView, index, 0, ajcBak.ascii);
				ListView_SetItemText(hWndListView, index, 1, ajcBak.jlatin);
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
					SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_A, L"");
					SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_J, L"");
				}
				else
				{
					ListView_GetItemText(hWndListView, index, 0, ajc.ascii, _countof(ajc.ascii));
					ListView_GetItemText(hWndListView, index, 1, ajc.jlatin, _countof(ajc.jlatin));
					SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_A, ajc.ascii);
					SetDlgItemText(hDlg, IDC_EDIT_JLATTBL_J, ajc.jlatin);
				}
				return (INT_PTR)TRUE;
			}
			break;

		case PSN_APPLY:
			SaveJLatin(hDlg);
			return (INT_PTR)TRUE;

		default:
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
