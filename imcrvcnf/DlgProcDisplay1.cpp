
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

#define DISPLAY_FONTSIZE 10

static struct {
	int id;
	LPCWSTR value;
	COLORREF color;
} displayListColor[DISPLAY_LIST_COLOR_NUM] =
{
	{IDC_COL_BG, ValueColorBG, RGB(0xFF, 0xFF, 0xFF)},
	{IDC_COL_FR, ValueColorFR, RGB(0x00, 0x00, 0x00)},
	{IDC_COL_SE, ValueColorSE, RGB(0x00, 0x00, 0xFF)},
	{IDC_COL_CO, ValueColorCO, RGB(0x80, 0x80, 0x80)},
	{IDC_COL_CA, ValueColorCA, RGB(0x00, 0x00, 0x00)},
	{IDC_COL_SC, ValueColorSC, RGB(0x80, 0x80, 0x80)},
	{IDC_COL_AN, ValueColorAN, RGB(0x80, 0x80, 0x80)},
	{IDC_COL_NO, ValueColorNO, RGB(0x00, 0x00, 0x00)}
};

INT_PTR CALLBACK DlgProcDisplay1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	HDC hdc;
	PAINTSTRUCT ps;
	WCHAR num[16];
	WCHAR fontname[LF_FACESIZE];
	INT fontpoint, fontweight, count;
	BOOL fontitalic;
	CHOOSEFONTW cf = {};
	LOGFONTW lf = {};
	static HFONT hFont;
	LONG w;
	std::wstring strxmlval;
	CHOOSECOLORW cc = {};
	static COLORREF customColor[16];

	switch (message)
	{
	case WM_INITDIALOG:
		ReadValue(pathconfigxml, SectionFont, ValueFontName, strxmlval);
		wcsncpy_s(fontname, strxmlval.c_str(), _TRUNCATE);
		if (fontname[0] == L'\0')
		{
			wcsncpy_s(fontname, L"メイリオ", _TRUNCATE);
		}

		ReadValue(pathconfigxml, SectionFont, ValueFontSize, strxmlval);
		fontpoint = _wtoi(strxmlval.c_str());
		if (fontpoint < 8 || fontpoint > 72)
		{
			fontpoint = FONT_POINT_DEF;
		}

		ReadValue(pathconfigxml, SectionFont, ValueFontWeight, strxmlval);
		fontweight = _wtoi(strxmlval.c_str());
		if (fontweight <= 0 || fontweight > 1000)
		{
			fontweight = FW_NORMAL;
		}

		ReadValue(pathconfigxml, SectionFont, ValueFontItalic, strxmlval);
		fontitalic = _wtoi(strxmlval.c_str());
		if (fontitalic != FALSE)
		{
			fontitalic = TRUE;
		}

		SetDlgItemTextW(hDlg, IDC_EDIT_FONTNAME, fontname);
		hFont = CreateFontW(GetFontHeight(hDlg, DISPLAY_FONTSIZE), 0, 0, 0,
			fontweight, fontitalic, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, fontname);
		SendMessageW(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_SETFONT, (WPARAM)hFont, 0);

		SetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, fontpoint, FALSE);

		ReadValue(pathconfigxml, SectionDisplay, ValueMaxWidth, strxmlval);
		w = strxmlval.empty() ? -1 : _wtol(strxmlval.c_str());
		if (w < 0)
		{
			w = MAX_WIDTH_DEFAULT;
		}
		_snwprintf_s(num, _TRUNCATE, L"%d", w);
		SetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num);

		for (int i = 0; i < _countof(customColor); i++)
		{
			customColor[i] = RGB(0xFF, 0xFF, 0xFF);
		}

		for (int i = 0; i < _countof(displayListColor); i++)
		{
			ReadValue(pathconfigxml, SectionDisplay, displayListColor[i].value, strxmlval);
			if (!strxmlval.empty())
			{
				displayListColor[i].color = wcstoul(strxmlval.c_str(), nullptr, 0);
			}
		}

		LoadCheckButton(hDlg, IDC_RADIO_API_D2D, SectionDisplay, ValueDrawAPI, L"1");
		EnableWindow(GetDlgItem(hDlg, IDC_CHECKBOX_COLOR_FONT), TRUE);
		if (!IsDlgButtonChecked(hDlg, IDC_RADIO_API_D2D))
		{
			CheckDlgButton(hDlg, IDC_RADIO_API_GDI, BST_CHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECKBOX_COLOR_FONT), FALSE);
		}
		LoadCheckButton(hDlg, IDC_CHECKBOX_COLOR_FONT, SectionDisplay, ValueColorFont, L"1");

		hwnd = GetDlgItem(hDlg, IDC_COMBO_UNTILCANDLIST);
		num[1] = L'\0';
		for (int i = 0; i <= 9; i++)
		{
			num[0] = L'0' + (WCHAR)i;
			SendMessageW(hwnd, CB_ADDSTRING, 0, (LPARAM)num);
		}
		ReadValue(pathconfigxml, SectionDisplay, ValueUntilCandList, strxmlval);
		count = strxmlval.empty() ? UNTILCANDLIST_DEF : _wtoi(strxmlval.c_str());
		if (count > 9 || count < 0)
		{
			count = UNTILCANDLIST_DEF;
		}
		SendMessageW(hwnd, CB_SETCURSEL, (WPARAM)count, 0);

		hwnd = GetDlgItem(hDlg, IDC_COMBO_PAGECANDNUM);
		num[1] = L'\0';
		for (int i = 1; i <= MAX_SELKEY_C; i++)
		{
			num[0] = L'0' + (WCHAR)i;
			SendMessageW(hwnd, CB_ADDSTRING, 0, (LPARAM)num);
		}
		ReadValue(pathconfigxml, SectionDisplay, ValuePageCandNum, strxmlval);
		count = strxmlval.empty() ? MAX_SELKEY : _wtoi(strxmlval.c_str());
		if (count > MAX_SELKEY_C || count < 1)
		{
			count = MAX_SELKEY;
		}
		SendMessageW(hwnd, CB_SETCURSEL, (WPARAM)(count - 1), 0);

		LoadCheckButton(hDlg, IDC_CHECKBOX_DISPCANDNO, SectionDisplay, ValueDispCandNo);
		LoadCheckButton(hDlg, IDC_CHECKBOX_VERTICALCAND, SectionDisplay, ValueVerticalCand, L"1");

		LoadCheckButton(hDlg, IDC_CHECKBOX_ANNOTATION, SectionDisplay, ValueAnnotation, L"1");
		LoadCheckButton(hDlg, IDC_RADIO_ANNOTATLST, SectionDisplay, ValueAnnotatLst, L"1");
		if (!IsDlgButtonChecked(hDlg, IDC_RADIO_ANNOTATLST))
		{
			CheckDlgButton(hDlg, IDC_RADIO_ANNOTATALL, BST_CHECKED);
		}

		LoadCheckButton(hDlg, IDC_CHECKBOX_SHOWMODEMARK, SectionDisplay, ValueShowModeMark);
		LoadCheckButton(hDlg, IDC_CHECKBOX_SHOWROMAN, SectionDisplay, ValueShowRoman, L"1");
		LoadCheckButton(hDlg, IDC_RADIO_SHOWROMANJLATIN, SectionDisplay, ValueShowRomanJLat, L"1");
		if (!IsDlgButtonChecked(hDlg, IDC_RADIO_SHOWROMANJLATIN))
		{
			CheckDlgButton(hDlg, IDC_RADIO_SHOWROMANASCII, BST_CHECKED);
		}

		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:
		GetObjectW(hFont, sizeof(lf), &lf);
		lf.lfHeight = GetFontHeight(hDlg, DISPLAY_FONTSIZE);
		DeleteObject(hFont);
		hFont = CreateFontIndirectW(&lf);
		SendMessageW(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_SETFONT, (WPARAM)hFont, 0);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_CHOOSEFONT:
			GetObjectW(hFont, sizeof(lf), &lf);
			lf.lfHeight = -MulDiv(GetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, nullptr, FALSE),
				C_USER_DEFAULT_SCREEN_DPI, C_FONT_LOGICAL_HEIGHT_PPI);

			cf.lStructSize = sizeof(cf);
			cf.hwndOwner = hDlg;
			cf.lpLogFont = &lf;
			cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS | CF_SCREENFONTS | CF_SELECTSCRIPT;

			if (ChooseFontW(&cf) == TRUE)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				SetDlgItemTextW(hDlg, IDC_EDIT_FONTNAME, lf.lfFaceName);
				SetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, cf.iPointSize / 10, FALSE);

				lf.lfHeight = GetFontHeight(hDlg, DISPLAY_FONTSIZE);
				DeleteObject(hFont);
				hFont = CreateFontIndirectW(&lf);
				SendMessageW(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_SETFONT, (WPARAM)hFont, 0);
			}
			return TRUE;

		case IDC_EDIT_MAXWIDTH:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return TRUE;
			default:
				break;
			}
			break;

		case IDC_COL_BG:
		case IDC_COL_FR:
		case IDC_COL_SE:
		case IDC_COL_CO:
		case IDC_COL_CA:
		case IDC_COL_SC:
		case IDC_COL_AN:
		case IDC_COL_NO:
			switch (HIWORD(wParam))
			{
			case STN_CLICKED:
			case STN_DBLCLK:
				for (int i = 0; i < _countof(displayListColor); i++)
				{
					if (LOWORD(wParam) == displayListColor[i].id)
					{
						cc.lStructSize = sizeof(cc);
						cc.hwndOwner = hDlg;
						cc.hInstance = nullptr;
						cc.rgbResult = displayListColor[i].color;
						cc.lpCustColors = customColor;
						cc.Flags = CC_FULLOPEN | CC_RGBINIT;
						cc.lCustData = 0;
						cc.lpfnHook = nullptr;
						cc.lpTemplateName = nullptr;

						if (ChooseColorW(&cc))
						{
							DrawSelectColor(hDlg, displayListColor[i].id, cc.rgbResult);
							displayListColor[i].color = cc.rgbResult;
							PropSheet_Changed(GetParent(hDlg), hDlg);
						}
						return TRUE;
					}
				}
				break;
			default:
				break;
			}
			break;

		case IDC_COMBO_UNTILCANDLIST:
		case IDC_COMBO_PAGECANDNUM:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return TRUE;
			default:
				break;
			}
			break;

		case IDC_RADIO_API_GDI:
		case IDC_RADIO_API_D2D:
			hwnd = GetDlgItem(hDlg, IDC_CHECKBOX_COLOR_FONT);
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_API_D2D))
			{
				EnableWindow(hwnd, TRUE);
			}
			else
			{
				EnableWindow(hwnd, FALSE);
			}
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return TRUE;

		case IDC_CHECKBOX_COLOR_FONT:
		case IDC_CHECKBOX_DISPCANDNO:
		case IDC_CHECKBOX_VERTICALCAND:
		case IDC_CHECKBOX_ANNOTATION:
		case IDC_RADIO_ANNOTATALL:
		case IDC_RADIO_ANNOTATLST:
		case IDC_CHECKBOX_SHOWMODEMARK:
		case IDC_CHECKBOX_SHOWROMAN:
		case IDC_RADIO_SHOWROMANASCII:
		case IDC_RADIO_SHOWROMANJLATIN:
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return TRUE;

		default:
			break;
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		for (int i = 0; i < _countof(displayListColor); i++)
		{
			DrawSelectColor(hDlg, displayListColor[i].id, displayListColor[i].color);
		}
		EndPaint(hDlg, &ps);

		return TRUE;

	case WM_DESTROY:
		DeleteObject(hFont);
		PostQuitMessage(0);
		return TRUE;

	default:
		break;
	}

	return FALSE;
}

void SaveFont(IXmlWriter *pWriter, HWND hDlg)
{
	WCHAR fontname[LF_FACESIZE];
	HFONT hFont;
	LOGFONTW lf = {};
	WCHAR num[16];

	GetDlgItemTextW(hDlg, IDC_EDIT_FONTNAME, fontname, _countof(fontname));
	WriterKey(pWriter, ValueFontName, fontname);

	hFont = (HFONT)SendMessageW(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_GETFONT, 0, 0);
	GetObjectW(hFont, sizeof(lf), &lf);
	GetDlgItemTextW(hDlg, IDC_EDIT_FONTPOINT, num, _countof(num));
	WriterKey(pWriter, ValueFontSize, num);
	_snwprintf_s(num, _TRUNCATE, L"%d", lf.lfWeight);
	WriterKey(pWriter, ValueFontWeight, num);
	_snwprintf_s(num, _TRUNCATE, L"%d", lf.lfItalic == FALSE ? 0 : 1);
	WriterKey(pWriter, ValueFontItalic, num);
}

void SaveDisplay1(IXmlWriter *pWriter, HWND hDlg)
{
	WCHAR num[16];
	LONG w;
	HWND hwnd;
	int count;

	GetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num, _countof(num));
	w = _wtol(num);
	if (w < 0)
	{
		w = MAX_WIDTH_DEFAULT;
	}
	_snwprintf_s(num, _TRUNCATE, L"%d", w);
	SetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num);
	WriterKey(pWriter, ValueMaxWidth, num);

	for (int i = 0; i < _countof(displayListColor); i++)
	{
		_snwprintf_s(num, _TRUNCATE, L"0x%06X", displayListColor[i].color);
		WriterKey(pWriter, displayListColor[i].value, num);
	}

	SaveCheckButton(pWriter, hDlg, IDC_RADIO_API_D2D, ValueDrawAPI);
	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_COLOR_FONT, ValueColorFont);

	hwnd = GetDlgItem(hDlg, IDC_COMBO_UNTILCANDLIST);
	count = (int)SendMessageW(hwnd, CB_GETCURSEL, 0, 0);
	if (count > 9 || count < 0)
	{
		count = UNTILCANDLIST_DEF;
	}
	num[0] = L'0' + count;
	num[1] = L'\0';
	WriterKey(pWriter, ValueUntilCandList, num);

	hwnd = GetDlgItem(hDlg, IDC_COMBO_PAGECANDNUM);
	count = 1 + (int)SendMessageW(hwnd, CB_GETCURSEL, 0, 0);
	if (count > MAX_SELKEY_C || count < 1)
	{
		count = MAX_SELKEY;
	}
	num[0] = L'0' + count;
	num[1] = L'\0';
	WriterKey(pWriter, ValuePageCandNum, num);

	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_DISPCANDNO, ValueDispCandNo);
	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_VERTICALCAND, ValueVerticalCand);
	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_ANNOTATION, ValueAnnotation);
	SaveCheckButton(pWriter, hDlg, IDC_RADIO_ANNOTATLST, ValueAnnotatLst);

	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_SHOWMODEMARK, ValueShowModeMark);
	SaveCheckButton(pWriter, hDlg, IDC_CHECKBOX_SHOWROMAN, ValueShowRoman);
	SaveCheckButton(pWriter, hDlg, IDC_RADIO_SHOWROMANJLATIN, ValueShowRomanJLat);
}
