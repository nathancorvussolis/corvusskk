
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static const KEYMAPCNF KeyMap2[] =
{
	{IDC_EDIT_KANA,			ValueKeyMapKana,		L""},
	{IDC_EDIT_CONV_CHAR,	ValueKeyMapConvChar,	L""},
	{IDC_EDIT_JLATIN,		ValueKeyMapJLatin,		L""},
	{IDC_EDIT_ASCII,		ValueKeyMapAscii,		L""},
	{IDC_EDIT_JMODE,		ValueKeyMapJMode,		L""},
	{IDC_EDIT_ABBREV,		ValueKeyMapAbbrev,		L""},
	{IDC_EDIT_AFFIX,		ValueKeyMapAffix,		L""},
	{IDC_EDIT_NEXT_CAND,	ValueKeyMapNextCand,	L""},
	{IDC_EDIT_PREV_CAND,	ValueKeyMapPrevCand,	L""},
	{IDC_EDIT_PURGE_DIC,	ValueKeyMapPurgeDic,	L""},
	{IDC_EDIT_NEXT_COMP,	ValueKeyMapNextComp,	L""},
	{IDC_EDIT_PREV_COMP,	ValueKeyMapPrevComp,	L""},
	{IDC_EDIT_CONV_POINT,	ValueKeyMapConvPoint,	L""},
	{IDC_EDIT_DIRECT,		ValueKeyMapDirect,		L""},
	{IDC_EDIT_ENTER,		ValueKeyMapEnter,		L""},
	{IDC_EDIT_CANCEL,		ValueKeyMapCancel,		L""},
	{IDC_EDIT_BACK,			ValueKeyMapBack,		L""},
	{IDC_EDIT_DELETE,		ValueKeyMapDelete,		L"\\x2E"},
	{IDC_EDIT_VOID,			ValueKeyMapVoid,		L""},
	{IDC_EDIT_LEFT,			ValueKeyMapLeft,		L"\\x25"},
	{IDC_EDIT_UP,			ValueKeyMapUp,			L"\\x26"},
	{IDC_EDIT_RIGHT,		ValueKeyMapRight,		L"\\x27"},
	{IDC_EDIT_DOWN,			ValueKeyMapDown,		L"\\x28"},
	{IDC_EDIT_PASTE,		ValueKeyMapPaste,		L""}
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
		return TRUE;
		
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
				return TRUE;
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
