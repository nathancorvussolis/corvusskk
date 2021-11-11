
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

INT_PTR CALLBACK DlgProcBehavior2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	WCHAR num[2];
	std::wstring strxmlval;
	int count;

	switch (message)
	{
	case WM_INITDIALOG:
		hwnd = GetDlgItem(hDlg, IDC_COMBO_COMPMULTINUM);
		num[1] = L'\0';
		for (int i = 1; i <= MAX_SELKEY_C; i++)
		{
			num[0] = L'0' + (WCHAR)i;
			SendMessageW(hwnd, CB_ADDSTRING, 0, (LPARAM)num);
		}
		ReadValue(pathconfigxml, SectionBehavior, ValueCompMultiNum, strxmlval);
		count = strxmlval.empty() ? COMPMULTIDISP_DEF : _wtoi(strxmlval.c_str());
		if (count > MAX_SELKEY_C || count < 1)
		{
			count = COMPMULTIDISP_DEF;
		}
		SendMessageW(hwnd, CB_SETCURSEL, (WPARAM)(count - 1), 0);

		LoadCheckButton(hDlg, IDC_CHECKBOX_STACOMPMULTI, SectionBehavior, ValueStaCompMulti);
		LoadCheckButton(hDlg, IDC_CHECKBOX_DYNAMINCOMP, SectionBehavior, ValueDynamicComp);
		LoadCheckButton(hDlg, IDC_CHECKBOX_DYNCOMPMULTI, SectionBehavior, ValueDynCompMulti);
		LoadCheckButton(hDlg, IDC_CHECKBOX_COMPUSERDIC, SectionBehavior, ValueCompUserDic);
		LoadCheckButton(hDlg, IDC_CHECKBOX_COMPINCBACK, SectionBehavior, ValueCompIncBack);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_COMBO_COMPMULTINUM:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return TRUE;
			default:
				break;
			}
			break;

		case IDC_CHECKBOX_STACOMPMULTI:
		case IDC_CHECKBOX_DYNAMINCOMP:
		case IDC_CHECKBOX_DYNCOMPMULTI:
		case IDC_CHECKBOX_COMPUSERDIC:
		case IDC_CHECKBOX_COMPINCBACK:

			PropSheet_Changed(GetParent(hDlg), hDlg);

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

void SaveBehavior2(IXmlWriter *pWriter, HWND hDlg)
{
	WCHAR num[2];

	HWND hwnd = GetDlgItem(hDlg, IDC_COMBO_COMPMULTINUM);
	int count = 1 + (int)SendMessageW(hwnd, CB_GETCURSEL, 0, 0);
	if (count > MAX_SELKEY_C || count < 1)
	{
		count = COMPMULTIDISP_DEF;
	}
	num[0] = L'0' + (WCHAR)count;
	num[1] = L'\0';
	WriterKey(pWriter, ValueCompMultiNum, num);

	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_STACOMPMULTI, ValueStaCompMulti);
	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_DYNAMINCOMP, ValueDynamicComp);
	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_DYNCOMPMULTI, ValueDynCompMulti);
	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_COMPUSERDIC, ValueCompUserDic);
	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_COMPINCBACK, ValueCompIncBack);
}
