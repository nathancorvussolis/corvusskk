
#include "common.h"
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

INT_PTR CALLBACK DlgProcBehavior(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND cmbUntilCandList;
	size_t i;
	WCHAR num[16];
	WCHAR fontname[LF_FACESIZE];
	int fontpoint;
	int fontweight;
	BOOL fontitalic;
	CHOOSEFONTW cf;
	LOGFONT lf;
	HDC hdcDlg;
	HFONT hFont;
	RECT rect;
	LONG w;
	FILE *fp;
	std::wstring strxmlval;

	switch(message)
	{
	case WM_INITDIALOG:
		ReadValue(pathconfigxml, SectionFont, FontName, strxmlval);
		wcsncpy_s(fontname, strxmlval.c_str(), _TRUNCATE);

		ReadValue(pathconfigxml, SectionFont, FontSize, strxmlval);
		fontpoint = _wtoi(strxmlval.c_str());
		ReadValue(pathconfigxml, SectionFont, FontWeight, strxmlval);
		fontweight = _wtoi(strxmlval.c_str());
		ReadValue(pathconfigxml, SectionFont, FontItalic, strxmlval);
		fontitalic = _wtoi(strxmlval.c_str());

		if(fontpoint < 8 || fontpoint > 72)
		{
			fontpoint = 12;
		}
		if(fontweight < 0 || fontweight > 1000)
		{
			fontweight = FW_NORMAL;
		}
		if(fontitalic != TRUE && fontitalic != FALSE)
		{
			fontitalic = FALSE;
		}

		SetDlgItemTextW(hDlg, IDC_EDIT_FONTNAME, fontname);
		hdcDlg = GetDC(hDlg);
		hFont = CreateFontW(-MulDiv(10, GetDeviceCaps(hdcDlg, LOGPIXELSY), 72), 0, 0, 0,
			fontweight, fontitalic, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, fontname);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_SETFONT, (WPARAM)hFont, 0);
		ReleaseDC(hDlg, hdcDlg);

		SetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, fontpoint, FALSE);

		ReadValue(pathconfigxml, SectionFont, MaxWidth, strxmlval);
		w = strxmlval.empty() ? -1 : _wtol(strxmlval.c_str());
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		if(w < 0 || w > rect.right)
		{
			w = rect.right;
		}
		_snwprintf_s(num, _TRUNCATE, L"%d", w);
		SetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num);

		LoadCheckButton(hDlg, IDC_CHECKBOX_VISUALSTYLE, SectionBehavior, VisualStyle);

		cmbUntilCandList = GetDlgItem(hDlg, IDC_COMBO_UNTILCANDLIST);
		num[1] = L'\0';
		for(i=0; i<=8; i++)
		{
			num[0] = L'0' + (WCHAR)i;
			SendMessage(cmbUntilCandList, CB_ADDSTRING, 0, (LPARAM)num);
		}
		ReadValue(pathconfigxml, SectionBehavior, UntilCandList, strxmlval);
		i = strxmlval.empty() ? 4 : _wtoi(strxmlval.c_str());
		if(i > 8)
		{
			i = 4;
		}
		SendMessage(cmbUntilCandList, CB_SETCURSEL, (WPARAM)i, 0);

		LoadCheckButton(hDlg, IDC_CHECKBOX_DISPCANDNO, SectionBehavior, DispCandNo);
		LoadCheckButton(hDlg, IDC_CHECKBOX_ANNOTATION, SectionBehavior, Annotation);
		LoadCheckButton(hDlg, IDC_RADIO_ANNOTATLST, SectionBehavior, AnnotatLst);
		if(!IsDlgButtonChecked(hDlg, IDC_RADIO_ANNOTATLST))
		{
			CheckDlgButton(hDlg, IDC_RADIO_ANNOTATALL, BST_CHECKED);
		}
		LoadCheckButton(hDlg, IDC_CHECKBOX_NOMODEMARK, SectionBehavior, NoModeMark);
		LoadCheckButton(hDlg, IDC_CHECKBOX_NOOKURICONV, SectionBehavior, NoOkuriConv);
		LoadCheckButton(hDlg, IDC_CHECKBOX_DELOKURICNCL, SectionBehavior, DelOkuriCncl);
		LoadCheckButton(hDlg, IDC_CHECKBOX_BACKINCENTER, SectionBehavior, BackIncEnter);
		LoadCheckButton(hDlg, IDC_CHECKBOX_ADDCANDKTKN, SectionBehavior, AddCandKtkn);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_CHOOSEFONT:
			hdcDlg = GetDC(hDlg);

			hFont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(LOGFONT), &lf);
			lf.lfHeight = -MulDiv(GetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, NULL, FALSE), GetDeviceCaps(hdcDlg, LOGPIXELSY), 72);
			lf.lfCharSet = SHIFTJIS_CHARSET;

			ZeroMemory(&cf, sizeof(cf));
			cf.lStructSize = sizeof(CHOOSEFONTW);
			cf.hwndOwner = hDlg;
			cf.lpLogFont = &lf;
			cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS | CF_SCREENFONTS | CF_SELECTSCRIPT;

			if(ChooseFontW(&cf) == TRUE)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				SetDlgItemText(hDlg, IDC_EDIT_FONTNAME, lf.lfFaceName);
				lf.lfHeight = -MulDiv(10, GetDeviceCaps(hdcDlg, LOGPIXELSY), 72);
				hFont = CreateFontIndirect(&lf);
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_SETFONT, (WPARAM)hFont, 0);
				SetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, cf.iPointSize / 10, FALSE);
			}

			ReleaseDC(hDlg, hdcDlg);
			return (INT_PTR)TRUE;

		case IDC_EDIT_MAXWIDTH:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return (INT_PTR)TRUE;
			default:
				break;
			}
			break;

		case IDC_COMBO_UNTILCANDLIST:
			switch(HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return (INT_PTR)TRUE;
			default:
				break;
			}
			break;

		case IDC_CHECKBOX_VISUALSTYLE:
		case IDC_CHECKBOX_DISPCANDNO:
		case IDC_CHECKBOX_ANNOTATION:
		case IDC_RADIO_ANNOTATALL:
		case IDC_RADIO_ANNOTATLST:
		case IDC_CHECKBOX_NOMODEMARK:
		case IDC_CHECKBOX_NOOKURICONV:
		case IDC_CHECKBOX_DELOKURICNCL:
		case IDC_CHECKBOX_BACKINCENTER:
		case IDC_CHECKBOX_ADDCANDKTKN:
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return (INT_PTR)TRUE;

		default:
			break;
		}
		break;
		
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			if(IsVersion62AndOver(ovi))
			{
				_wfopen_s(&fp, pathconfigxml, L"a");
				if(fp != NULL)
				{
					fclose(fp);
				}
				SetFileDaclAC(pathconfigxml);
			}

			WriterInit(pathconfigxml, &pXmlWriter, &pXmlFileStream);

			WriterStartElement(pXmlWriter, TagRoot);

			WriterStartSection(pXmlWriter, SectionFont);

			GetDlgItemTextW(hDlg, IDC_EDIT_FONTNAME, fontname, _countof(fontname));
			WriterKey(pXmlWriter, FontName, fontname);

			hFont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(LOGFONT), &lf);
			GetDlgItemTextW(hDlg, IDC_EDIT_FONTPOINT, num, _countof(num));
			WriterKey(pXmlWriter, FontSize, num);
			_snwprintf_s(num, _TRUNCATE, L"%d", lf.lfWeight);
			WriterKey(pXmlWriter, FontWeight, num);
			_snwprintf_s(num, _TRUNCATE, L"%d", lf.lfItalic);
			WriterKey(pXmlWriter, FontItalic, num);

			WriterEndSection(pXmlWriter);

			WriterStartSection(pXmlWriter, SectionBehavior);

			GetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num, _countof(num));
			w = _wtol(num);
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
			if(w < 0 || w > rect.right)
			{
				w = rect.right;
			}
			_snwprintf_s(num, _TRUNCATE, L"%d", w);
			SetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num);
			WriterKey(pXmlWriter, MaxWidth, num);

			SaveCheckButton(hDlg, IDC_CHECKBOX_VISUALSTYLE, SectionBehavior, VisualStyle);

			cmbUntilCandList = GetDlgItem(hDlg, IDC_COMBO_UNTILCANDLIST);
			num[0] = L'0' + (WCHAR)SendMessage(cmbUntilCandList, CB_GETCURSEL, 0, 0);
			num[1] = L'\0';
			WriterKey(pXmlWriter, UntilCandList, num);

			SaveCheckButton(hDlg, IDC_CHECKBOX_DISPCANDNO, SectionBehavior, DispCandNo);
			SaveCheckButton(hDlg, IDC_CHECKBOX_ANNOTATION, SectionBehavior, Annotation);
			SaveCheckButton(hDlg, IDC_RADIO_ANNOTATLST, SectionBehavior, AnnotatLst);
			SaveCheckButton(hDlg, IDC_CHECKBOX_NOMODEMARK, SectionBehavior, NoModeMark);
			SaveCheckButton(hDlg, IDC_CHECKBOX_NOOKURICONV, SectionBehavior, NoOkuriConv);
			SaveCheckButton(hDlg, IDC_CHECKBOX_DELOKURICNCL, SectionBehavior, DelOkuriCncl);
			SaveCheckButton(hDlg, IDC_CHECKBOX_BACKINCENTER, SectionBehavior, BackIncEnter);
			SaveCheckButton(hDlg, IDC_CHECKBOX_ADDCANDKTKN, SectionBehavior, AddCandKtkn);

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
