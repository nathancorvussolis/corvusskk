
#include "corvuscnf.h"
#include "resource.h"

static const KEYMAP KeyMap1[] =
{
	{IDC_EDIT_KANA,			L"Kana",		L"q"},
	{IDC_EDIT_CONV_CHAR,	L"ConvChar",	L"\\cq"},
	{IDC_EDIT_JLATIN,		L"JLatin",		L"L"},
	{IDC_EDIT_ASCII,		L"Ascii",		L"l"},
	{IDC_EDIT_JMODE,		L"JMode",		L"\\cj|\\cq"},
	{IDC_EDIT_ABBREV,		L"Abbrev",		L"/"},
	{IDC_EDIT_AFFIX,		L"Affix",		L"<|>"},
	{IDC_EDIT_DIRECT,		L"Direct",		L"[0-9]"},
	{IDC_EDIT_NEXT_CAND,	L"NextCand",	L"\\x20|\\cn"},
	{IDC_EDIT_PREV_CAND,	L"PrevCand",	L"x|\\cp"},
	{IDC_EDIT_PURGE_DIC,	L"PurgeDic",	L"X|\\cx"},
	{IDC_EDIT_NEXT_COMP,	L"NextComp",	L"\\ci"},
	{IDC_EDIT_PREV_COMP,	L"PrevComp",	L"\\cu"}
};

INT_PTR CALLBACK DlgProcKeyMap1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	size_t i;

	switch(message)
	{
	case WM_INITDIALOG:
		for(i=0; i<_countof(KeyMap1) ;i++)
		{
			LoadKeyMap(hDlg, KeyMap1[i].idd, KeyMap1[i].keyName, KeyMap1[i].defaultValue);
		}
		return (INT_PTR)TRUE;
		
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_EDIT_KANA:
		case IDC_EDIT_CONV_CHAR:
		case IDC_EDIT_JLATIN:
		case IDC_EDIT_ASCII:
		case IDC_EDIT_JMODE:
		case IDC_EDIT_ABBREV:
		case IDC_EDIT_AFFIX:
		case IDC_EDIT_DIRECT:
		case IDC_EDIT_NEXT_CAND:
		case IDC_EDIT_PREV_CAND:
		case IDC_EDIT_PURGE_DIC:
		case IDC_EDIT_NEXT_COMP:
		case IDC_EDIT_PREV_COMP:
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
			for(i=0; i<_countof(KeyMap1) ;i++)
			{
				SaveKeyMap(hDlg, KeyMap1[i].idd, KeyMap1[i].keyName);
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
