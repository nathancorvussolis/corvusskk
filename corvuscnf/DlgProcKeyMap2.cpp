
#include "configxml.h"
#include "corvuscnf.h"
#include "resource.h"

static const KEYMAP KeyMap2[] =
{
	{IDC_EDIT_ENTER,	KeyMapEnter,	L"\\cm|\\cj"},
	{IDC_EDIT_CANCEL,	KeyMapCancel,	L"\\cg|\\x1B"},
	{IDC_EDIT_BACK,		KeyMapBack,		L"\\ch"},
	{IDC_EDIT_DELETE,	KeyMapDelete,	L"\\x7F"},
	{IDC_EDIT_VOID,		KeyMapVoid,		L"\\cj"},
	{IDC_EDIT_LEFT,		KeyMapLeft,		L"\\cb"},
	{IDC_EDIT_UP,		KeyMapUp,		L"\\ca"},
	{IDC_EDIT_RIGHT,	KeyMapRight,	L"\\cf"},
	{IDC_EDIT_DOWN,		KeyMapDown,		L"\\ce"},
	{IDC_EDIT_PASTE,	KeyMapPaste,	L"\\cy|\\cv"}
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

			WriterEndSection(pXmlWriter);

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
