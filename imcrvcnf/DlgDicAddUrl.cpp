
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

WCHAR urlskkdic[INTERNET_MAX_URL_LENGTH];

INT_PTR CALLBACK DlgProcSKKDicAddUrl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		SetBkMode((HDC)wParam, TRANSPARENT);
		SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemTextW(hDlg, IDC_EDIT_SKK_DIC_URL, urlskkdic, _countof(urlskkdic));
			{
				// trim
				std::wstring strurl = std::regex_replace(std::wstring(urlskkdic),
					std::wregex(L"^\\s+|\\s+$"), std::wstring(L""));
				_snwprintf_s(urlskkdic, _TRUNCATE, L"%s", strurl.c_str());

				if (urlskkdic[0] == L'\0')
				{
					EndDialog(hDlg, IDCANCEL);
				}
			}
			EndDialog(hDlg, IDOK);
			return TRUE;
		case IDCANCEL:
			urlskkdic[0] = L'\0';
			EndDialog(hDlg, IDCANCEL);
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
