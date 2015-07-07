
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

INT_PTR CALLBACK DlgProcBehavior(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		LoadCheckButton(hDlg, IDC_CHECKBOX_DEFAULTMODE, SectionBehavior, ValueDefaultMode);
		LoadCheckButton(hDlg, IDC_RADIO_DEFMODEASCII, SectionBehavior, ValueDefModeAscii);
		if(!IsDlgButtonChecked(hDlg, IDC_RADIO_DEFMODEASCII))
		{
			CheckDlgButton(hDlg, IDC_RADIO_DEFMODEHIRA, BST_CHECKED);
		}
		LoadCheckButton(hDlg, IDC_CHECKBOX_BEGINCVOKURI, SectionBehavior, ValueBeginCvOkuri, L"1");
		LoadCheckButton(hDlg, IDC_CHECKBOX_PRECEDEOKURI, SectionBehavior, ValuePrecedeOkuri);
		LoadCheckButton(hDlg, IDC_CHECKBOX_SHIFTNNOKURI, SectionBehavior, ValueShiftNNOkuri, L"1");
		LoadCheckButton(hDlg, IDC_CHECKBOX_SRCHALLOKURI, SectionBehavior, ValueSrchAllOkuri);
		LoadCheckButton(hDlg, IDC_CHECKBOX_DELCVPOSCNCL, SectionBehavior, ValueDelCvPosCncl, L"1");
		LoadCheckButton(hDlg, IDC_CHECKBOX_DELOKURICNCL, SectionBehavior, ValueDelOkuriCncl);
		LoadCheckButton(hDlg, IDC_CHECKBOX_BACKINCENTER, SectionBehavior, ValueBackIncEnter, L"1");
		LoadCheckButton(hDlg, IDC_CHECKBOX_ADDCANDKTKN, SectionBehavior, ValueAddCandKtkn);
		LoadCheckButton(hDlg, IDC_CHECKBOX_STACOMPMULTI, SectionBehavior, ValueStaCompMulti);
		LoadCheckButton(hDlg, IDC_CHECKBOX_DYNAMINCOMP, SectionBehavior, ValueDynamicComp);
		LoadCheckButton(hDlg, IDC_CHECKBOX_DYNCOMPMULTI, SectionBehavior, ValueDynCompMulti);
		LoadCheckButton(hDlg, IDC_CHECKBOX_COMPUSERDIC, SectionBehavior, ValueCompUserDic);

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CHECKBOX_DEFAULTMODE:
		case IDC_RADIO_DEFMODEHIRA:
		case IDC_RADIO_DEFMODEASCII:
		case IDC_CHECKBOX_BEGINCVOKURI:
		case IDC_CHECKBOX_PRECEDEOKURI:
		case IDC_CHECKBOX_SHIFTNNOKURI:
		case IDC_CHECKBOX_SRCHALLOKURI:
		case IDC_CHECKBOX_DELCVPOSCNCL:
		case IDC_CHECKBOX_DELOKURICNCL:
		case IDC_CHECKBOX_BACKINCENTER:
		case IDC_CHECKBOX_ADDCANDKTKN:
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
			WriterStartSection(pXmlWriter, SectionBehavior);	//Start of SectionBehavior

			SaveCheckButton(hDlg, IDC_CHECKBOX_DEFAULTMODE, ValueDefaultMode);
			SaveCheckButton(hDlg, IDC_RADIO_DEFMODEASCII, ValueDefModeAscii);
			SaveCheckButton(hDlg, IDC_CHECKBOX_BEGINCVOKURI, ValueBeginCvOkuri);
			SaveCheckButton(hDlg, IDC_CHECKBOX_PRECEDEOKURI, ValuePrecedeOkuri);
			SaveCheckButton(hDlg, IDC_CHECKBOX_SHIFTNNOKURI, ValueShiftNNOkuri);
			SaveCheckButton(hDlg, IDC_CHECKBOX_SRCHALLOKURI, ValueSrchAllOkuri);
			SaveCheckButton(hDlg, IDC_CHECKBOX_DELCVPOSCNCL, ValueDelCvPosCncl);
			SaveCheckButton(hDlg, IDC_CHECKBOX_DELOKURICNCL, ValueDelOkuriCncl);
			SaveCheckButton(hDlg, IDC_CHECKBOX_BACKINCENTER, ValueBackIncEnter);
			SaveCheckButton(hDlg, IDC_CHECKBOX_ADDCANDKTKN, ValueAddCandKtkn);
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
