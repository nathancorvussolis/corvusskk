
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static const KEYMAPCNF KeyMap2[] =
{
	{IDC_EDIT_KANA,			KeyMapKana,		L""},
	{IDC_EDIT_CONV_CHAR,	KeyMapConvChar,	L""},
	{IDC_EDIT_JLATIN,		KeyMapJLatin,	L""},
	{IDC_EDIT_ASCII,		KeyMapAscii,	L""},
	{IDC_EDIT_JMODE,		KeyMapJMode,	L""},
	{IDC_EDIT_ABBREV,		KeyMapAbbrev,	L""},
	{IDC_EDIT_AFFIX,		KeyMapAffix,	L""},
	{IDC_EDIT_NEXT_CAND,	KeyMapNextCand,	L""},
	{IDC_EDIT_PREV_CAND,	KeyMapPrevCand,	L""},
	{IDC_EDIT_PURGE_DIC,	KeyMapPurgeDic,	L""},
	{IDC_EDIT_NEXT_COMP,	KeyMapNextComp,	L""},
	{IDC_EDIT_PREV_COMP,	KeyMapPrevComp,	L""},
	{IDC_EDIT_CONV_POINT,	KeyMapConvPoint,L""},
	{IDC_EDIT_DIRECT,		KeyMapDirect,	L""},
	{IDC_EDIT_ENTER,		KeyMapEnter,	L""},
	{IDC_EDIT_CANCEL,		KeyMapCancel,	L""},
	{IDC_EDIT_BACK,			KeyMapBack,		L""},
	{IDC_EDIT_DELETE,		KeyMapDelete,	L"\\x2E"},
	{IDC_EDIT_VOID,			KeyMapVoid,		L""},
	{IDC_EDIT_LEFT,			KeyMapLeft,		L"\\x25"},
	{IDC_EDIT_UP,			KeyMapUp,		L"\\x26"},
	{IDC_EDIT_RIGHT,		KeyMapRight,	L"\\x27"},
	{IDC_EDIT_DOWN,			KeyMapDown,		L"\\x28"},
	{IDC_EDIT_PASTE,		KeyMapPaste,	L""}
};

INT_PTR CALLBACK DlgProcKeyMap2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	size_t i;

	switch(message)
	{
	case WM_INITDIALOG:
		for(i=0; i<_countof(KeyMap2) ;i++)
		{
			LoadKeyMap(hDlg, KeyMap2[i].idd, SectionVKeyMap, KeyMap2[i].keyName, KeyMap2[i].defaultValue);
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
		case IDC_EDIT_NEXT_CAND:
		case IDC_EDIT_PREV_CAND:
		case IDC_EDIT_PURGE_DIC:
		case IDC_EDIT_NEXT_COMP:
		case IDC_EDIT_PREV_COMP:
		case IDC_EDIT_CONV_POINT:
		case IDC_EDIT_DIRECT:
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
			WriterStartSection(pXmlWriter, SectionVKeyMap);

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
