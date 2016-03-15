
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

INT_PTR CALLBACK DlgProcBehavior2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	WCHAR num[16];
	std::wstring strxmlval;
	int count;

	switch(message)
	{
	case WM_INITDIALOG:
		hwnd = GetDlgItem(hDlg, IDC_COMBO_COMPMULTINUM);
		num[1] = L'\0';
		for(int i = 1; i <= 9; i++)
		{
			num[0] = L'0' + (WCHAR)i;
			SendMessageW(hwnd, CB_ADDSTRING, 0, (LPARAM)num);
		}
		ReadValue(pathconfigxml, SectionBehavior, ValueCompMultiNum, strxmlval);
		count = strxmlval.empty() ? COMPMULTIDISP_NUM : _wtoi(strxmlval.c_str());
		if(count > MAX_SELKEY_C || count < 1)
		{
			count = COMPMULTIDISP_NUM;
		}
		SendMessageW(hwnd, CB_SETCURSEL, (WPARAM)(count - 1), 0);

		LoadCheckButton(hDlg, IDC_CHECKBOX_STACOMPMULTI, SectionBehavior, ValueStaCompMulti);
		LoadCheckButton(hDlg, IDC_CHECKBOX_DYNAMINCOMP, SectionBehavior, ValueDynamicComp);
		LoadCheckButton(hDlg, IDC_CHECKBOX_DYNCOMPMULTI, SectionBehavior, ValueDynCompMulti);
		LoadCheckButton(hDlg, IDC_CHECKBOX_COMPUSERDIC, SectionBehavior, ValueCompUserDic);

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_COMBO_COMPMULTINUM:
			switch(HIWORD(wParam))
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

			PropSheet_Changed(GetParent(hDlg), hDlg);

			return TRUE;

		default:
			break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			hwnd = GetDlgItem(hDlg, IDC_COMBO_COMPMULTINUM);
			count = 1;
			count += (int)SendMessageW(hwnd, CB_GETCURSEL, 0, 0);
			num[0] = L'0' + (WCHAR)count;
			num[1] = L'\0';
			WriterKey(pXmlWriter, ValueCompMultiNum, num);

			SaveCheckButton(hDlg, IDC_CHECKBOX_STACOMPMULTI, ValueStaCompMulti);
			SaveCheckButton(hDlg, IDC_CHECKBOX_DYNAMINCOMP, ValueDynamicComp);
			SaveCheckButton(hDlg, IDC_CHECKBOX_DYNCOMPMULTI, ValueDynCompMulti);
			SaveCheckButton(hDlg, IDC_CHECKBOX_COMPUSERDIC, ValueCompUserDic);

			WriterEndSection(pXmlWriter);	//End of SectionBehavior

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
