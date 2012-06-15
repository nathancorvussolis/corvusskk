
#include "corvuscnf.h"
#include "resource.h"

//セクション
static const WCHAR *IniSecBehavior = L"Behavior";
//キー
static const WCHAR *FontName = L"FontName";
static const WCHAR *FontStyle = L"FontStyle";
static const WCHAR *MaxWidth = L"MaxWidth";
static const WCHAR *VisualStyle = L"VisualStyle";
static const WCHAR *UntilCandList = L"UntilCandList";
static const WCHAR *DispCandNo = L"DispCandNo";
static const WCHAR *Annotation = L"Annotation";
static const WCHAR *NoModeMark = L"NoModeMark";
static const WCHAR *NoOkuriConv = L"NoOkuriConv";
static const WCHAR *DelOkuriCncl = L"DelOkuriCncl";
static const WCHAR *BackIncEnter = L"BackIncEnter";

INT_PTR CALLBACK DlgProcBehavior(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND cmbUntilCandList;
	size_t i;
	WCHAR num[16];
	WCHAR fontname[LF_FACESIZE];
	int fontpoint;
	int fontweight;
	BOOL fontitalic;
	CHOOSEFONT cf;
	LOGFONT lf;
	HDC hdcDlg;
	HFONT hFont;
	RECT rect;
	LONG w;
	FILE *fp;

	switch(message)
	{
	case WM_INITDIALOG:
		GetPrivateProfileStringW(IniSecBehavior, FontName, L"", fontname, _countof(fontname), pathconfig);
		
		GetPrivateProfileStringW(IniSecBehavior, FontStyle, L"12,400,0", num, _countof(num), pathconfig);
		if(swscanf_s(num, L"%d,%d,%d", &fontpoint, &fontweight, &fontitalic) != 3)
		{
			fontpoint = 12;
			fontweight = FW_NORMAL;
			fontitalic = FALSE;
		}
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

		GetPrivateProfileStringW(IniSecBehavior, MaxWidth, L"-1", num, _countof(num), pathconfig);
		w = _wtol(num);
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		if(w < 0 || w > rect.right)
		{
			w = rect.right;
		}
		_snwprintf_s(num, _TRUNCATE, L"%d", w);
		SetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num);

		LoadCheckButton(hDlg, IDC_CHECKBOX_VISUALSTYLE, IniSecBehavior, VisualStyle);

		cmbUntilCandList = GetDlgItem(hDlg, IDC_COMBO_UNTILCANDLIST);
		num[1] = L'\0';
		for(i=0; i<=8; i++)
		{
			num[0] = L'0' + (WCHAR)i;
			SendMessage(cmbUntilCandList, CB_ADDSTRING, 0, (LPARAM)num);
		}
		GetPrivateProfileStringW(IniSecBehavior, UntilCandList, L"4", num, 2, pathconfig);
		i = _wtoi(num);
		if(i > 8 || (i == 0 && num[0] != L'0'))
		{
			i = 4;
		}
		SendMessage(cmbUntilCandList, CB_SETCURSEL, (WPARAM)i, 0);

		LoadCheckButton(hDlg, IDC_CHECKBOX_DISPCANDNO, IniSecBehavior, DispCandNo);
		LoadCheckButton(hDlg, IDC_CHECKBOX_ANNOTATION, IniSecBehavior, Annotation);
		LoadCheckButton(hDlg, IDC_CHECKBOX_NOMODEMARK, IniSecBehavior, NoModeMark);
		LoadCheckButton(hDlg, IDC_CHECKBOX_NOOKURICONV, IniSecBehavior, NoOkuriConv);
		LoadCheckButton(hDlg, IDC_CHECKBOX_DELOKURICNCL, IniSecBehavior, DelOkuriCncl);
		LoadCheckButton(hDlg, IDC_CHECKBOX_BACKINCENTER, IniSecBehavior, BackIncEnter);

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
			cf.lStructSize = sizeof(CHOOSEFONT);
			cf.hwndOwner = hDlg;
			cf.lpLogFont = &lf;
			cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS | CF_SCREENFONTS | CF_SELECTSCRIPT;

			if(ChooseFont(&cf) == TRUE)
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
		case IDC_CHECKBOX_NOMODEMARK:
		case IDC_CHECKBOX_NOOKURICONV:
		case IDC_CHECKBOX_DELOKURICNCL:
		case IDC_CHECKBOX_BACKINCENTER:
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
			_wfopen_s(&fp, pathconfig, L"wb");
			if(fp != NULL)
			{
				fwrite("\xFF\xFE", 2, 1, fp);
				fclose(fp);
			}

			GetDlgItemTextW(hDlg, IDC_EDIT_FONTNAME, fontname, _countof(fontname));
			WritePrivateProfileStringW(IniSecBehavior, FontName, fontname, pathconfig);

			hFont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(LOGFONT), &lf);
			_snwprintf_s(num, _TRUNCATE, L"%d,%d,%d", GetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, NULL, FALSE),
				lf.lfWeight, ((lf.lfItalic == FALSE) ? FALSE : TRUE));
			WritePrivateProfileStringW(IniSecBehavior, FontStyle, num, pathconfig);

			GetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num, _countof(num));
			w = _wtol(num);
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
			if(w < 0 || w > rect.right)
			{
				w = rect.right;
			}
			_snwprintf_s(num, _TRUNCATE, L"%d", w);
			SetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num);
			WritePrivateProfileStringW(IniSecBehavior, MaxWidth, num, pathconfig);

			SaveCheckButton(hDlg, IDC_CHECKBOX_VISUALSTYLE, IniSecBehavior, VisualStyle);

			cmbUntilCandList = GetDlgItem(hDlg, IDC_COMBO_UNTILCANDLIST);
			num[0] = L'0' + (WCHAR)SendMessage(cmbUntilCandList, CB_GETCURSEL, 0, 0);
			num[1] = L'\0';
			WritePrivateProfileStringW(IniSecBehavior, UntilCandList, num, pathconfig);

			SaveCheckButton(hDlg, IDC_CHECKBOX_DISPCANDNO, IniSecBehavior, DispCandNo);
			SaveCheckButton(hDlg, IDC_CHECKBOX_ANNOTATION, IniSecBehavior, Annotation);
			SaveCheckButton(hDlg, IDC_CHECKBOX_NOMODEMARK, IniSecBehavior, NoModeMark);
			SaveCheckButton(hDlg, IDC_CHECKBOX_NOOKURICONV, IniSecBehavior, NoOkuriConv);
			SaveCheckButton(hDlg, IDC_CHECKBOX_DELOKURICNCL, IniSecBehavior, DelOkuriCncl);
			SaveCheckButton(hDlg, IDC_CHECKBOX_BACKINCENTER, IniSecBehavior, BackIncEnter);

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
