
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static const KEYMAPCNF KeyMap1[] =
{
	{IDC_EDIT_KANA,			ValueKeyMapKana,		L"q"},
	{IDC_EDIT_CONV_CHAR,	ValueKeyMapConvChar,	L"\\cq"},
	{IDC_EDIT_JLATIN,		ValueKeyMapJLatin,		L"L"},
	{IDC_EDIT_ASCII,		ValueKeyMapAscii,		L"l"},
	{IDC_EDIT_JMODE,		ValueKeyMapJMode,		L"\\cj|\\cq"},
	{IDC_EDIT_ABBREV,		ValueKeyMapAbbrev,		L"/"},
	{IDC_EDIT_AFFIX,		ValueKeyMapAffix,		L"<|>"},
	{IDC_EDIT_NEXT_CAND,	ValueKeyMapNextCand,	L"\\x20|\\cn"},
	{IDC_EDIT_PREV_CAND,	ValueKeyMapPrevCand,	L"x|\\cp"},
	{IDC_EDIT_PURGE_DIC,	ValueKeyMapPurgeDic,	L"X|\\cx"},
	{IDC_EDIT_NEXT_COMP,	ValueKeyMapNextComp,	L"\\ci"},
	{IDC_EDIT_PREV_COMP,	ValueKeyMapPrevComp,	L"\\cu"},
	{IDC_EDIT_CONV_POINT,	ValueKeyMapConvPoint,	L""},
	{IDC_EDIT_DIRECT,		ValueKeyMapDirect,		L"[0-9]"},
	{IDC_EDIT_ENTER,		ValueKeyMapEnter,		L"\\cm|\\cj"},
	{IDC_EDIT_CANCEL,		ValueKeyMapCancel,		L"\\cg|\\x1B"},
	{IDC_EDIT_BACK,			ValueKeyMapBack,		L"\\ch"},
	{IDC_EDIT_DELETE,		ValueKeyMapDelete,		L"\\cd|\\x7F"},
	{IDC_EDIT_VOID,			ValueKeyMapVoid,		L"\\cj"},
	{IDC_EDIT_LEFT,			ValueKeyMapLeft,		L"\\cb"},
	{IDC_EDIT_UP,			ValueKeyMapUp,			L"\\ca"},
	{IDC_EDIT_RIGHT,		ValueKeyMapRight,		L"\\cf"},
	{IDC_EDIT_DOWN,			ValueKeyMapDown,		L"\\ce"},
	{IDC_EDIT_PASTE,		ValueKeyMapPaste,		L"\\cy|\\cv"}
};

INT_PTR CALLBACK DlgProcKeyMap1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	size_t i;

	switch(message)
	{
	case WM_INITDIALOG:
		for(i=0; i<_countof(KeyMap1) ;i++)
		{
			LoadKeyMap(hDlg, KeyMap1[i].idd, SectionKeyMap, KeyMap1[i].keyName, KeyMap1[i].defaultValue);
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
			WriterStartSection(pXmlWriter, SectionKeyMap);

			for(i=0; i<_countof(KeyMap1) ;i++)
			{
				SaveKeyMap(hDlg, KeyMap1[i].idd, KeyMap1[i].keyName);
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
