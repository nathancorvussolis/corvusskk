
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static const KEYMAPCNF KeyMap1[] =
{
	{IDC_EDIT_KANA,			KeyMapKana,		L"q"},
	{IDC_EDIT_CONV_CHAR,	KeyMapConvChar,	L"\\cq"},
	{IDC_EDIT_JLATIN,		KeyMapJLatin,	L"L"},
	{IDC_EDIT_ASCII,		KeyMapAscii,	L"l"},
	{IDC_EDIT_JMODE,		KeyMapJMode,	L"\\cj|\\cq"},
	{IDC_EDIT_ABBREV,		KeyMapAbbrev,	L"/"},
	{IDC_EDIT_AFFIX,		KeyMapAffix,	L"<|>"},
	{IDC_EDIT_NEXT_CAND,	KeyMapNextCand,	L"\\x20|\\cn"},
	{IDC_EDIT_PREV_CAND,	KeyMapPrevCand,	L"x|\\cp"},
	{IDC_EDIT_PURGE_DIC,	KeyMapPurgeDic,	L"X|\\cx"},
	{IDC_EDIT_NEXT_COMP,	KeyMapNextComp,	L"\\ci"},
	{IDC_EDIT_PREV_COMP,	KeyMapPrevComp,	L"\\cu"},
	{IDC_EDIT_CONV_POINT,	KeyMapConvPoint,L""},
	{IDC_EDIT_DIRECT,		KeyMapDirect,	L"[0-9]"},
	{IDC_EDIT_ENTER,		KeyMapEnter,	L"\\cm|\\cj"},
	{IDC_EDIT_CANCEL,		KeyMapCancel,	L"\\cg|\\x1B"},
	{IDC_EDIT_BACK,			KeyMapBack,		L"\\ch"},
	{IDC_EDIT_DELETE,		KeyMapDelete,	L"\\x7F"},
	{IDC_EDIT_VOID,			KeyMapVoid,		L"\\cj"},
	{IDC_EDIT_LEFT,			KeyMapLeft,		L"\\cb"},
	{IDC_EDIT_UP,			KeyMapUp,		L"\\ca"},
	{IDC_EDIT_RIGHT,		KeyMapRight,	L"\\cf"},
	{IDC_EDIT_DOWN,			KeyMapDown,		L"\\ce"},
	{IDC_EDIT_PASTE,		KeyMapPaste,	L"\\cy|\\cv"}
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
			WriterStartSection(pXmlWriter, SectionKeyMap);

			for(i=0; i<_countof(KeyMap1) ;i++)
			{
				SaveKeyMap(hDlg, KeyMap1[i].idd, KeyMap1[i].keyName);
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
