
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static struct {
	int id;
	LPCWSTR value;
	COLORREF color;
} displayModeColor[DISPLAY_MODE_COLOR_NUM] =
{
	{IDC_COL_MODE_MC, ValueColorMC, RGB(0xFF, 0xFF, 0xFF)},
	{IDC_COL_MODE_MF, ValueColorMF, RGB(0x00, 0x00, 0x00)},
	{IDC_COL_MODE_HR, ValueColorHR, RGB(0xC0, 0x00, 0x00)},
	{IDC_COL_MODE_KT, ValueColorKT, RGB(0x00, 0xC0, 0x00)},
	{IDC_COL_MODE_KA, ValueColorKA, RGB(0x00, 0xC0, 0x80)},
	{IDC_COL_MODE_JL, ValueColorJL, RGB(0x00, 0x00, 0xC0)},
	{IDC_COL_MODE_AC, ValueColorAC, RGB(0x00, 0x80, 0xC0)},
	{IDC_COL_MODE_DR, ValueColorDR, RGB(0x80, 0x80, 0x80)}
};

INT_PTR CALLBACK DlgProcDisplay2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	HDC hdc;
	PAINTSTRUCT ps;
	WCHAR num[16];
	INT count;
	RECT rect;
	POINT pt;
	std::wstring strxmlval;
	CHOOSECOLORW cc;
	static COLORREF customColor[16];

	switch(message)
	{
	case WM_INITDIALOG:
		LoadCheckButton(hDlg, IDC_CHECKBOX_SHOWMODEINL, SectionDisplay, ValueShowModeInl, L"1");
		ReadValue(pathconfigxml, SectionDisplay, ValueShowModeSec, strxmlval);
		count = strxmlval.empty() ? -1 : _wtoi(strxmlval.c_str());
		if(count > 60 || count <= 0)
		{
			count = 3;
		}
		_snwprintf_s(num, _TRUNCATE, L"%d", count);
		SetDlgItemTextW(hDlg, IDC_EDIT_SHOWMODESEC, num);

		for(int i = 0; i < _countof(customColor); i++)
		{
			customColor[i] = RGB(0xFF, 0xFF, 0xFF);
		}

		for(int i = 0; i < _countof(displayModeColor); i++)
		{
			ReadValue(pathconfigxml, SectionDisplay, displayModeColor[i].value, strxmlval);
			if(!strxmlval.empty())
			{
				displayModeColor[i].color = wcstoul(strxmlval.c_str(), nullptr, 0);
			}
		}

		return TRUE;

	case WM_DPICHANGED_AFTERPARENT:

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_EDIT_SHOWMODESEC:
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

	case WM_LBUTTONDOWN:
		for(int i = 0; i < _countof(displayModeColor); i++)
		{
			hwnd = GetDlgItem(hDlg, displayModeColor[i].id);
			GetWindowRect(hwnd, &rect);
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			ClientToScreen(hDlg, &pt);

			if(rect.left <= pt.x && pt.x <= rect.right &&
				rect.top <= pt.y && pt.y <= rect.bottom)
			{
				cc.lStructSize = sizeof(cc);
				cc.hwndOwner = hDlg;
				cc.hInstance = nullptr;
				cc.rgbResult = displayModeColor[i].color;
				cc.lpCustColors = customColor;
				cc.Flags = CC_FULLOPEN | CC_RGBINIT;
				cc.lCustData = 0;
				cc.lpfnHook = nullptr;
				cc.lpTemplateName = nullptr;
				if(ChooseColorW(&cc))
				{
					DrawSelectColor(hDlg, displayModeColor[i].id, cc.rgbResult);
					displayModeColor[i].color = cc.rgbResult;
					PropSheet_Changed(GetParent(hDlg), hDlg);
					return TRUE;
				}
				break;
			}
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		for(int i = 0; i < _countof(displayModeColor); i++)
		{
			DrawSelectColor(hDlg, displayModeColor[i].id, displayModeColor[i].color);
		}
		EndPaint(hDlg, &ps);

		return TRUE;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			SaveCheckButton(hDlg, IDC_CHECKBOX_SHOWMODEINL, ValueShowModeInl);
			GetDlgItemTextW(hDlg, IDC_EDIT_SHOWMODESEC, num, _countof(num));
			count = _wtoi(num);
			if(count <= 0 || count > 60)
			{
				count = 3;
			}
			_snwprintf_s(num, _TRUNCATE, L"%d", count);
			SetDlgItemTextW(hDlg, IDC_EDIT_SHOWMODESEC, num);
			WriterKey(pXmlWriter, ValueShowModeSec, num);

			for(int i = 0; i < _countof(displayModeColor); i++)
			{
				_snwprintf_s(num, _TRUNCATE, L"0x%06X", displayModeColor[i].color);
				WriterKey(pXmlWriter, displayModeColor[i].value, num);
			}

			WriterEndSection(pXmlWriter);	//End of SectionDisplay

			return TRUE;

		default:
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;

	default:
		break;
	}

	return FALSE;
}
