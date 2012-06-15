
#include "corvuscnf.h"
#include "resource.h"

static const KEYMAP KeyMap2[] =
{
	{IDC_EDIT_ENTER,	L"Enter",	L"\\cm|\\cj"},
	{IDC_EDIT_CANCEL,	L"Cancel",	L"\\cg|\\x1B"},
	{IDC_EDIT_BACK,		L"Back",	L"\\ch"},
	{IDC_EDIT_DELETE,	L"Delete",	L"\\x7F"},
	{IDC_EDIT_VOID,		L"Void",	L""},
	{IDC_EDIT_LEFT,		L"Left",	L"\\cb"},
	{IDC_EDIT_UP,		L"Up",		L"\\ca"},
	{IDC_EDIT_RIGHT,	L"Right",	L"\\cf"},
	{IDC_EDIT_DOWN,		L"Down",	L"\\ce"},
	{IDC_EDIT_PASTE,	L"Paste",	L"\\cy|\\cv"}
};

INT_PTR CALLBACK DlgProcKeyMap2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	size_t i;

	switch(message)
	{
	case WM_INITDIALOG:
		for(i=0; i<_countof(KeyMap2) ;i++)
		{
			LoadKeyMap(hDlg, KeyMap2[i].idd, KeyMap2[i].keyName, KeyMap2[i].defaultValue);
		}
		return (INT_PTR)TRUE;
		
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_EDIT_ENTER:
		case IDC_EDIT_CANCEL:
		case IDC_EDIT_BACK:
		case IDC_EDIT_DELETE:
		case IDC_EDIT_VOID:
		case IDC_EDIT_LEFT:
		case IDC_EDIT_UP:
		case IDC_EDIT_RIGHT:
		case IDC_EDIT_DOWN:
		case IDC_EDIT_PASTE:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return (INT_PTR)TRUE;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			for(i=0; i<_countof(KeyMap2) ;i++)
			{
				SaveKeyMap(hDlg, KeyMap2[i].idd, KeyMap2[i].keyName);
			}
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
