
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static struct {
	LPCWSTR key;
	BOOL se;
	TF_DISPLAYATTRIBUTE da;
} displayAttr[2][4] =
{
	{
		{ValueDisplayAttrInputMark, c_daDisplayAttributeSeries[0], c_daDisplayAttributeInputMark},
		{ValueDisplayAttrInputText, c_daDisplayAttributeSeries[1], c_daDisplayAttributeInputText},
		{ValueDisplayAttrInputOkuri, c_daDisplayAttributeSeries[2], c_daDisplayAttributeInputOkuri},
		{nullptr, FALSE, {(TF_DA_COLORTYPE)0, 0}}
	},
	{
		{ValueDisplayAttrConvMark, c_daDisplayAttributeSeries[3], c_daDisplayAttributeConvMark},
		{ValueDisplayAttrConvText, c_daDisplayAttributeSeries[4], c_daDisplayAttributeConvText},
		{ValueDisplayAttrConvOkuri, c_daDisplayAttributeSeries[5], c_daDisplayAttributeConvOkuri},
		{ValueDisplayAttrConvAnnot, c_daDisplayAttributeSeries[6], c_daDisplayAttributeConvAnnot}
	}
};

static const struct {
	int id;
	COLORREF *color;
} displayAttrColor[2][4][3] =
{
	{
		{
			{IDC_COL_FG_MARK, &displayAttr[0][0].da.crText.cr},
			{IDC_COL_BG_MARK, &displayAttr[0][0].da.crBk.cr},
			{IDC_COL_UL_MARK, &displayAttr[0][0].da.crLine.cr}
		},
		{
			{IDC_COL_FG_TEXT, &displayAttr[0][1].da.crText.cr},
			{IDC_COL_BG_TEXT, &displayAttr[0][1].da.crBk.cr},
			{IDC_COL_UL_TEXT, &displayAttr[0][1].da.crLine.cr}
		},
		{
			{IDC_COL_FG_OKURI, &displayAttr[0][2].da.crText.cr},
			{IDC_COL_BG_OKURI, &displayAttr[0][2].da.crBk.cr},
			{IDC_COL_UL_OKURI, &displayAttr[0][2].da.crLine.cr}
		},
		{
			{IDC_COL_FG_ANNOT, &displayAttr[0][3].da.crText.cr},
			{IDC_COL_BG_ANNOT, &displayAttr[0][3].da.crBk.cr},
			{IDC_COL_UL_ANNOT, &displayAttr[0][3].da.crLine.cr}
		}
	},
	{
		{
			{IDC_COL_FG_MARK, &displayAttr[1][0].da.crText.cr},
			{IDC_COL_BG_MARK, &displayAttr[1][0].da.crBk.cr},
			{IDC_COL_UL_MARK, &displayAttr[1][0].da.crLine.cr}
		},
		{
			{IDC_COL_FG_TEXT, &displayAttr[1][1].da.crText.cr},
			{IDC_COL_BG_TEXT, &displayAttr[1][1].da.crBk.cr},
			{IDC_COL_UL_TEXT, &displayAttr[1][1].da.crLine.cr}
		},
		{
			{IDC_COL_FG_OKURI, &displayAttr[1][2].da.crText.cr},
			{IDC_COL_BG_OKURI, &displayAttr[1][2].da.crBk.cr},
			{IDC_COL_UL_OKURI, &displayAttr[1][2].da.crLine.cr}
			},
		{
			{IDC_COL_FG_ANNOT, &displayAttr[1][3].da.crText.cr},
			{IDC_COL_BG_ANNOT, &displayAttr[1][3].da.crBk.cr},
			{IDC_COL_UL_ANNOT, &displayAttr[1][3].da.crLine.cr}
		}
	}
};

static const int displayAttrID[4][13] =
{
	{
		IDC_CHECKBOX_SERIES_MARK,
		IDC_RADIO_FG_STD_MARK, IDC_RADIO_FG_SEL_MARK, IDC_COL_FG_MARK,
		IDC_RADIO_BG_STD_MARK, IDC_RADIO_BG_SEL_MARK, IDC_COL_BG_MARK,
		IDC_COMBO_UL_ATTR_MARK, IDC_CHECKBOX_UL_BOLD_MARK,
		IDC_RADIO_UL_STD_MARK, IDC_RADIO_UL_SEL_MARK, IDC_COL_UL_MARK,
		IDC_COMBO_ATTR_MARK
	},
	{
		IDC_CHECKBOX_SERIES_TEXT,
		IDC_RADIO_FG_STD_TEXT, IDC_RADIO_FG_SEL_TEXT, IDC_COL_FG_TEXT,
		IDC_RADIO_BG_STD_TEXT, IDC_RADIO_BG_SEL_TEXT, IDC_COL_BG_TEXT,
		IDC_COMBO_UL_ATTR_TEXT, IDC_CHECKBOX_UL_BOLD_TEXT,
		IDC_RADIO_UL_STD_TEXT, IDC_RADIO_UL_SEL_TEXT, IDC_COL_UL_TEXT,
		IDC_COMBO_ATTR_TEXT
	},
		{
		IDC_CHECKBOX_SERIES_OKURI,
		IDC_RADIO_FG_STD_OKURI, IDC_RADIO_FG_SEL_OKURI, IDC_COL_FG_OKURI,
		IDC_RADIO_BG_STD_OKURI, IDC_RADIO_BG_SEL_OKURI, IDC_COL_BG_OKURI,
		IDC_COMBO_UL_ATTR_OKURI, IDC_CHECKBOX_UL_BOLD_OKURI,
		IDC_RADIO_UL_STD_OKURI, IDC_RADIO_UL_SEL_OKURI, IDC_COL_UL_OKURI,
		IDC_COMBO_ATTR_OKURI
	},
		{
		IDC_CHECKBOX_SERIES_ANNOT,
		IDC_RADIO_FG_STD_ANNOT, IDC_RADIO_FG_SEL_ANNOT, IDC_COL_FG_ANNOT,
		IDC_RADIO_BG_STD_ANNOT, IDC_RADIO_BG_SEL_ANNOT, IDC_COL_BG_ANNOT,
		IDC_COMBO_UL_ATTR_ANNOT, IDC_CHECKBOX_UL_BOLD_ANNOT,
		IDC_RADIO_UL_STD_ANNOT, IDC_RADIO_UL_SEL_ANNOT, IDC_COL_UL_ANNOT,
		IDC_COMBO_ATTR_ANNOT
	}
};
static const int cbAttrID[] =
{
	IDC_COMBO_ATTR_MARK, IDC_COMBO_ATTR_TEXT, IDC_COMBO_ATTR_OKURI, IDC_COMBO_ATTR_ANNOT
};
static LPCWSTR cbAttrText[] =
{
	L"入力", L"変換中", L"変換済", L"未変換", L"エラー", L"無変換"
};
static const int cbULAttrID[] =
{
	IDC_COMBO_UL_ATTR_MARK, IDC_COMBO_UL_ATTR_TEXT, IDC_COMBO_UL_ATTR_OKURI, IDC_COMBO_UL_ATTR_ANNOT
};
static LPCWSTR cbULAttrText[] =
{
	L"なし", L"直線", L"点線", L"破線", L"波線"
};
//%d,%d,0x%06X,%d,0x%06X,%d,%d,%d,0x%06X,%d
static LPCWSTR displayAttrFormat = L"%d,%d,0x%06X,%d,0x%06X,%d,%d,%d,0x%06X,%d";

INT_PTR CALLBACK DlgProcDisplayAttr(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, int no);
void DisplayAttrSeriesChecked(HWND hDlg, int id);
void LoadConfigDisplayAttr(int no);
void SaveConfigDisplayAttr(int no);

INT_PTR CALLBACK DlgProcDisplayAttrInput(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DlgProcDisplayAttr(hDlg, message, wParam, lParam, 0);
}

INT_PTR CALLBACK DlgProcDisplayAttrConv(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DlgProcDisplayAttr(hDlg, message, wParam, lParam, 1);
}

INT_PTR CALLBACK DlgProcDisplayAttr(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, int no)
{
	HWND hwnd;
	int x, y;
	RECT rect;
	POINT pt;
	CHOOSECOLORW cc;
	PAINTSTRUCT ps;
	HDC hdc;
	static COLORREF colCust[16];

	switch(message)
	{
	case WM_INITDIALOG:
		for(int i = 0; i < _countof(cbAttrID); i++)
		{
			for(int j = 0; j < _countof(cbAttrText); j++)
			{
				SendMessageW(GetDlgItem(hDlg, cbAttrID[i]), CB_ADDSTRING, 0, (LPARAM)cbAttrText[j]);
			}
		}
		for(int i = 0; i < _countof(cbULAttrID); i++)
		{
			for(int j = 0; j < _countof(cbULAttrText); j++)
			{
				SendMessageW(GetDlgItem(hDlg, cbULAttrID[i]), CB_ADDSTRING, 0, (LPARAM)cbULAttrText[j]);
			}
		}
		for(int i = 0; i < _countof(colCust); i++)
		{
			colCust[i] = RGB(0xFF, 0xFF, 0xFF);
		}

		LoadConfigDisplayAttr(no);

		for(int i = 0; i < _countof(displayAttr[no]); i++)
		{
			if(displayAttr[no][i].se)
			{
				CheckDlgButton(hDlg, displayAttrID[i][0], BST_CHECKED);
				DisplayAttrSeriesChecked(hDlg, displayAttrID[i][0]);
			}
			switch(displayAttr[no][i].da.crText.type)
			{
			case TF_CT_NONE:
				CheckDlgButton(hDlg, displayAttrID[i][1], BST_CHECKED);
				break;
			default:
				CheckDlgButton(hDlg, displayAttrID[i][2], BST_CHECKED);
				break;
			}
			switch(displayAttr[no][i].da.crBk.type)
			{
			case TF_CT_NONE:
				CheckDlgButton(hDlg, displayAttrID[i][4], BST_CHECKED);
				break;
			default:
				CheckDlgButton(hDlg, displayAttrID[i][5], BST_CHECKED);
				break;
			}
			SendMessageW(GetDlgItem(hDlg, displayAttrID[i][7]), CB_SETCURSEL, displayAttr[no][i].da.lsStyle, 0);
			CheckDlgButton(hDlg, displayAttrID[i][8], (displayAttr[no][i].da.fBoldLine ? BST_CHECKED : BST_UNCHECKED));
			switch(displayAttr[no][i].da.crLine.type)
			{
			case TF_CT_NONE:
				CheckDlgButton(hDlg, displayAttrID[i][9], BST_CHECKED);
				break;
			default:
				CheckDlgButton(hDlg, displayAttrID[i][10], BST_CHECKED);
				break;
			}
			SendMessageW(GetDlgItem(hDlg, displayAttrID[i][12]), CB_SETCURSEL, displayAttr[no][i].da.bAttr, 0);
		}

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CHECKBOX_SERIES_MARK:
		case IDC_CHECKBOX_SERIES_TEXT:
		case IDC_CHECKBOX_SERIES_OKURI:
		case IDC_CHECKBOX_SERIES_ANNOT:
			DisplayAttrSeriesChecked(hDlg, LOWORD(wParam));
			//no break;
		case IDC_RADIO_FG_STD_MARK:
		case IDC_RADIO_FG_STD_TEXT:
		case IDC_RADIO_FG_STD_OKURI:
		case IDC_RADIO_FG_STD_ANNOT:
		case IDC_RADIO_FG_SEL_MARK:
		case IDC_RADIO_FG_SEL_TEXT:
		case IDC_RADIO_FG_SEL_OKURI:
		case IDC_RADIO_FG_SEL_ANNOT:
		case IDC_RADIO_BG_STD_MARK:
		case IDC_RADIO_BG_STD_TEXT:
		case IDC_RADIO_BG_STD_OKURI:
		case IDC_RADIO_BG_STD_ANNOT:
		case IDC_RADIO_BG_SEL_MARK:
		case IDC_RADIO_BG_SEL_TEXT:
		case IDC_RADIO_BG_SEL_OKURI:
		case IDC_RADIO_BG_SEL_ANNOT:
		case IDC_CHECKBOX_UL_BOLD_MARK:
		case IDC_CHECKBOX_UL_BOLD_TEXT:
		case IDC_CHECKBOX_UL_BOLD_OKURI:
		case IDC_CHECKBOX_UL_BOLD_ANNOT:
		case IDC_RADIO_UL_STD_MARK:
		case IDC_RADIO_UL_STD_TEXT:
		case IDC_RADIO_UL_STD_OKURI:
		case IDC_RADIO_UL_STD_ANNOT:
		case IDC_RADIO_UL_SEL_MARK:
		case IDC_RADIO_UL_SEL_TEXT:
		case IDC_RADIO_UL_SEL_OKURI:
		case IDC_RADIO_UL_SEL_ANNOT:
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return TRUE;

		case IDC_COMBO_UL_ATTR_MARK:
		case IDC_COMBO_UL_ATTR_TEXT:
		case IDC_COMBO_UL_ATTR_OKURI:
		case IDC_COMBO_UL_ATTR_ANNOT:
		case IDC_COMBO_ATTR_MARK:
		case IDC_COMBO_ATTR_TEXT:
		case IDC_COMBO_ATTR_OKURI:
		case IDC_COMBO_ATTR_ANNOT:
			switch(HIWORD(wParam))
			{
			case CBN_SELCHANGE:
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

	case WM_LBUTTONDOWN:
		for(int i = 0; i < _countof(displayAttrColor[no]); i++)
		{
			for(int j = 0; j < _countof(displayAttrColor[no][i]); j++)
			{
				hwnd = GetDlgItem(hDlg, displayAttrColor[no][i][j].id);
				if(!IsWindowEnabled(hwnd))
				{
					continue;
				}
				GetWindowRect(hwnd, &rect);
				pt.x = x = GET_X_LPARAM(lParam);
				pt.y = y = GET_Y_LPARAM(lParam);
				ClientToScreen(hDlg, &pt);

				if(rect.left <= pt.x && pt.x <= rect.right &&
					rect.top <= pt.y && pt.y <= rect.bottom)
				{
					cc.lStructSize = sizeof(cc);
					cc.hwndOwner = hDlg;
					cc.hInstance = nullptr;
					cc.rgbResult = *displayAttrColor[no][i][j].color;
					cc.lpCustColors = colCust;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					cc.lCustData = 0;
					cc.lpfnHook = nullptr;
					cc.lpTemplateName = nullptr;
					if(ChooseColorW(&cc))
					{
						DrawSelectColor(hDlg, displayAttrColor[no][i][j].id, cc.rgbResult);
						*displayAttrColor[no][i][j].color = cc.rgbResult;
						PropSheet_Changed(GetParent(hDlg), hDlg);
						return TRUE;
					}
					break;
				}
			}
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		for(int i = 0; i < _countof(displayAttrColor[no]); i++)
		{
			for(int j = 0; j < _countof(displayAttrColor[no][i]) && displayAttr[no][i].key != nullptr; j++)
			{
				DrawSelectColor(hDlg, displayAttrColor[no][i][j].id, *displayAttrColor[no][i][j].color);
			}
		}
		EndPaint(hDlg, &ps);

		return TRUE;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			for(int i = 0; i < _countof(displayAttr[no]); i++)
			{
				displayAttr[no][i].se =
					(IsDlgButtonChecked(hDlg, displayAttrID[i][0]) == BST_CHECKED ? TRUE : FALSE);
				displayAttr[no][i].da.crText.type =
					(IsDlgButtonChecked(hDlg, displayAttrID[i][2]) == BST_CHECKED ? TF_CT_COLORREF : TF_CT_NONE);
				displayAttr[no][i].da.crBk.type =
					(IsDlgButtonChecked(hDlg, displayAttrID[i][5]) == BST_CHECKED ? TF_CT_COLORREF : TF_CT_NONE);
				displayAttr[no][i].da.lsStyle =
					(TF_DA_LINESTYLE)SendMessageW(GetDlgItem(hDlg, displayAttrID[i][7]), CB_GETCURSEL, 0, 0);
				displayAttr[no][i].da.fBoldLine =
					(IsDlgButtonChecked(hDlg, displayAttrID[i][8]) == BST_CHECKED ? TRUE : FALSE);
				displayAttr[no][i].da.crLine.type =
					(IsDlgButtonChecked(hDlg, displayAttrID[i][10]) == BST_CHECKED ? TF_CT_COLORREF : TF_CT_NONE);
				displayAttr[no][i].da.bAttr =
					(TF_DA_ATTR_INFO)SendMessageW(GetDlgItem(hDlg, displayAttrID[i][12]), CB_GETCURSEL, 0, 0);
			}
			SaveConfigDisplayAttr(no);
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

void DisplayAttrSeriesChecked(HWND hDlg, int id)
{
	int c = -1;

	switch(id)
	{
		case IDC_CHECKBOX_SERIES_MARK:
			c = 0;
			break;
		case IDC_CHECKBOX_SERIES_TEXT:
			c = 1;
			break;
		case IDC_CHECKBOX_SERIES_OKURI:
			c = 2;
			break;
		case IDC_CHECKBOX_SERIES_ANNOT:
			c = 3;
			break;
		default:
			break;
	}
	if(c > -1)
	{
		for(int i = 1; i < _countof(displayAttrID[c]); i++)
		{
			EnableWindow(GetDlgItem(hDlg, displayAttrID[c][i]),
				(IsDlgButtonChecked(hDlg, id) == BST_CHECKED ? FALSE : TRUE));
		}
	}
}

void LoadConfigDisplayAttr(int no)
{
	std::wstring strxmlval;
	BOOL se;
	TF_DISPLAYATTRIBUTE da;

	for(int i = 0; i < _countof(displayAttr[no]) && displayAttr[no][i].key != nullptr; i++)
	{
		ReadValue(pathconfigxml, SectionDisplayAttr, displayAttr[no][i].key, strxmlval);
		if(!strxmlval.empty())
		{
			if(swscanf_s(strxmlval.c_str(), displayAttrFormat,
				&se, &da.crText.type, &da.crText.cr, &da.crBk.type, &da.crBk.cr,
				&da.lsStyle, &da.fBoldLine, &da.crLine.type, &da.crLine.cr, &da.bAttr) == 10)
			{
				displayAttr[no][i].se = se;
				displayAttr[no][i].da = da;
			}
		}
	}
}

void SaveConfigDisplayAttr(int no)
{
	WCHAR num[64];

	if(no == 0)
	{
		WriterStartSection(pXmlWriter, SectionDisplayAttr);	//Start of SectionDisplayAttr
	}

	for(int i = 0; i < _countof(displayAttr[no]) && displayAttr[no][i].key != nullptr; i++)
	{
		_snwprintf_s(num, _TRUNCATE, displayAttrFormat,
			displayAttr[no][i].se,
			displayAttr[no][i].da.crText.type, displayAttr[no][i].da.crText.cr,
			displayAttr[no][i].da.crBk.type, displayAttr[no][i].da.crBk.cr,
			displayAttr[no][i].da.lsStyle, displayAttr[no][i].da.fBoldLine,
			displayAttr[no][i].da.crLine.type, displayAttr[no][i].da.crLine.cr,
			displayAttr[no][i].da.bAttr);
		WriterKey(pXmlWriter, displayAttr[no][i].key, num);
	}

	if(no == 1)
	{
		WriterEndSection(pXmlWriter);	//End of SectionDisplayAttr
	}
}
