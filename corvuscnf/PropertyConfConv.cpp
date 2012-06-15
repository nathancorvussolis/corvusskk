
#include "corvuscnf.h"
#include "convtable.h"
#include "resource.h"

//セクション
static const WCHAR *IniSecKeyMap = L"KeyMap";

#define BUFSIZE 0x100

WCHAR conv_point[CONV_POINT_NUM][3][2];
ROMAN_KANA_CONV roman_kana_conv[ROMAN_KANA_TBL_NUM];
ASCII_JLATIN_CONV ascii_jlatin_conv[ASCII_JLATIN_TBL_NUM];

void LoadCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName)
{
	WCHAR num[2];

	GetPrivateProfileStringW(lpAppName, lpKeyName, L"0", num, _countof(num), pathconfig);
	CheckDlgButton(hDlg, nIDDlgItem, (_wtoi(num) == TRUE ? BST_CHECKED : BST_UNCHECKED));
}

void SaveCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName)
{
	WCHAR num[2];

	num[0] = L'0' + IsDlgButtonChecked(hDlg, nIDDlgItem);
	num[1] = L'\0';
	WritePrivateProfileStringW(lpAppName, lpKeyName, num, pathconfig);
}

void LoadKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName, LPCWSTR lpDefault)
{
	WCHAR keyre[KEYRELEN];

	GetPrivateProfileStringW(IniSecKeyMap, lpKeyName, lpDefault, keyre, _countof(keyre), pathconfig);
	SetDlgItemTextW(hDlg, nIDDlgItem, keyre);
}

void SaveKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName)
{
	WCHAR keyre[KEYRELEN];

	GetDlgItemTextW(hDlg, nIDDlgItem, keyre, _countof(keyre));
	WritePrivateProfileStringW(IniSecKeyMap, lpKeyName, keyre, pathconfig);
}

void LoadConfigConvPoint()
{
	FILE *fp;
	size_t t;
	wchar_t b[BUFSIZE];
	const wchar_t seps[] = L"\t\n\0";
	size_t sidx, eidx;
	wchar_t key[3][2];

	ZeroMemory(conv_point, sizeof(conv_point));
	for(t=0; t<26; t++)
	{
		conv_point[t][0][0] = L'A' + (WCHAR)t;
		conv_point[t][1][0] = L'a' + (WCHAR)t;
		conv_point[t][2][0] = L'a' + (WCHAR)t;
	}

	_wfopen_s(&fp, pathconfcvpt, RccsUNICODE);
	if(fp == NULL)
	{
		return;
	}
	
	ZeroMemory(b, sizeof(b));
	t = 0;
	while(fgetws(b, BUFSIZE, fp) != NULL)
	{
		if(t >= CONV_POINT_NUM)
		{
			break;
		}

		ZeroMemory(key, sizeof(key));

		sidx = 0;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(key[0], _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(key[1], _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(key[2], _TRUNCATE, L"%s", &b[sidx]);

		ZeroMemory(b, sizeof(b));

		if(key[0][0] == L'\0' &&
			key[1][0] == L'\0' &&
			key[2][0] == L'\0')
		{
			continue;
		}

		memcpy_s(conv_point[t], sizeof(conv_point[t]), key, sizeof(key));
		++t;
	}
	if(t < CONV_POINT_NUM)
	{
		conv_point[t][0][0] = L'\0';
		conv_point[t][1][0] = L'\0';
		conv_point[t][2][0] = L'\0';
	}

	fclose(fp);
}

void LoadConvPoint(HWND hwnd)
{
	HWND hWndList;
	int i;
	LVITEMW item;

	LoadConfigConvPoint();

	hWndList = GetDlgItem(hwnd, IDC_LIST_CONVPOINT);

	for(i=0; ; i++)
	{
		if(conv_point[i][0][0] == L'\0' &&
			conv_point[i][1][0] == L'\0' &&
			conv_point[i][2][0] == L'\0')
		{
			break;
		}
		item.mask = LVIF_TEXT;
		item.pszText = conv_point[i][0];
		item.iItem = i;
		item.iSubItem = 0;
		ListView_InsertItem(hWndList, &item);
		item.pszText = conv_point[i][1];
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem(hWndList, &item);
		item.pszText = conv_point[i][2];
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem(hWndList, &item);
	}
}

void SaveConvPoint(HWND hwnd)
{
	int i, count;
	HWND hWndListView;
	WCHAR key[2];
	FILE *fp;

	hWndListView = GetDlgItem(hwnd, IDC_LIST_CONVPOINT);
	count = ListView_GetItemCount(hWndListView);
	for(i=0; i<count && i<CONV_POINT_NUM; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, key, _countof(key));
		wcsncpy_s(conv_point[i][0], key, _TRUNCATE);
		ListView_GetItemText(hWndListView, i, 1, key, _countof(key));
		wcsncpy_s(conv_point[i][1], key, _TRUNCATE);
		ListView_GetItemText(hWndListView, i, 2, key, _countof(key));
		wcsncpy_s(conv_point[i][2], key, _TRUNCATE);
	}
	if(count < CONV_POINT_NUM)
	{
		conv_point[count][0][0] = L'\0';
		conv_point[count][1][0] = L'\0';
		conv_point[count][2][0] = L'\0';
	}

	_wfopen_s(&fp, pathconfcvpt, WccsUNICODE);
	if(fp != NULL)
	{
		for(i=0; i<count && i<CONV_POINT_NUM; i++)
		{
			if(conv_point[i][0][0] == L'\0' &&
				conv_point[i][1][0] == L'\0' &&
				conv_point[i][2][0] == L'\0')
			{
				continue;
			}
			fwprintf(fp, L"%s\t%s\t%s\n",
					 conv_point[i][0], conv_point[i][1], conv_point[i][2]);
		}

		fclose(fp);
	}
}

void LoadConfigKana()
{
	FILE *fp;
	size_t t;
	wchar_t b[BUFSIZE];
	const wchar_t seps[] = L"\t\n\0";
	size_t sidx, eidx;
	ROMAN_KANA_CONV conv;
	wchar_t soku[2];

	memcpy_s(roman_kana_conv, sizeof(roman_kana_conv),
		roman_kana_conv_default, sizeof(roman_kana_conv_default));

	_wfopen_s(&fp, pathconfkana, RccsUNICODE);
	if(fp == NULL)
	{
		return;
	}
	
	ZeroMemory(b, sizeof(b));
	t = 0;
	while(fgetws(b, BUFSIZE, fp) != NULL)
	{
		if(t >= ROMAN_KANA_TBL_NUM)
		{
			break;
		}

		ZeroMemory(&conv, sizeof(conv));

		sidx = 0;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(conv.roman, _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(conv.hiragana, _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(conv.katakana, _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(conv.katakana_ank, _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(soku, _TRUNCATE, L"%s", &b[sidx]);
		conv.soku = _wtoi(soku);
		if(conv.soku != TRUE && conv.soku != FALSE)
		{
			conv.soku = FALSE;
		}

		ZeroMemory(b, sizeof(b));

		if(conv.roman[0] == L'\0' &&
			conv.hiragana[0] == L'\0' &&
			conv.katakana[0] == L'\0' &&
			conv.katakana_ank[0] == L'\0')
		{
			continue;
		}

		roman_kana_conv[t] = conv;
		++t;
	}
	if(t < ROMAN_KANA_TBL_NUM)
	{
		roman_kana_conv[t].roman[0] = L'\0';
		roman_kana_conv[t].hiragana[0] = L'\0';
		roman_kana_conv[t].katakana[0] = L'\0';
		roman_kana_conv[t].katakana_ank[0] = L'\0';
		roman_kana_conv[t].soku = 0;
	}

	fclose(fp);
}

void LoadKana(HWND hwnd)
{
	HWND hWndList;
	int i;
	LVITEMW item;

	LoadConfigKana();

	hWndList = GetDlgItem(hwnd, IDC_LIST_KANATBL);

	for(i=0; ; i++)
	{
		if(roman_kana_conv[i].roman[0] == L'\0' &&
			roman_kana_conv[i].hiragana[0] == L'\0' &&
			roman_kana_conv[i].katakana[0] == L'\0' &&
			roman_kana_conv[i].katakana_ank[0] == L'\0')
		{
			break;
		}
		item.mask = LVIF_TEXT;
		item.pszText = roman_kana_conv[i].roman;
		item.iItem = i;
		item.iSubItem = 0;
		ListView_InsertItem(hWndList, &item);
		item.pszText = roman_kana_conv[i].hiragana;
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem(hWndList, &item);
		item.pszText = roman_kana_conv[i].katakana;
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem(hWndList, &item);
		item.pszText = roman_kana_conv[i].katakana_ank;
		item.iItem = i;
		item.iSubItem = 3;
		ListView_SetItem(hWndList, &item);
		item.pszText = (roman_kana_conv[i].soku == TRUE ? L"1" : L"0");
		item.iItem = i;
		item.iSubItem = 4;
		ListView_SetItem(hWndList, &item);
	}
}

void SaveKana(HWND hwnd)
{
	int i, count;
	HWND hWndListView;
	ROMAN_KANA_CONV rkc;
	WCHAR soku[8];
	FILE *fp;

	hWndListView = GetDlgItem(hwnd, IDC_LIST_KANATBL);
	count = ListView_GetItemCount(hWndListView);
	for(i=0; i<count && i<ROMAN_KANA_TBL_NUM; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, rkc.roman, _countof(rkc.roman));
		ListView_GetItemText(hWndListView, i, 1, rkc.hiragana, _countof(rkc.hiragana));
		ListView_GetItemText(hWndListView, i, 2, rkc.katakana, _countof(rkc.katakana));
		ListView_GetItemText(hWndListView, i, 3, rkc.katakana_ank, _countof(rkc.katakana_ank));
		ListView_GetItemText(hWndListView, i, 4, soku, _countof(soku));
		soku[0] == L'1' ? rkc.soku = TRUE : rkc.soku = FALSE;
		roman_kana_conv[i] = rkc;
	}
	if(count < ROMAN_KANA_TBL_NUM)
	{
		roman_kana_conv[count].roman[0] = L'\0';
		roman_kana_conv[count].hiragana[0] = L'\0';
		roman_kana_conv[count].katakana[0] = L'\0';
		roman_kana_conv[count].katakana_ank[0] = L'\0';
		roman_kana_conv[count].soku = FALSE;
	}

	_wfopen_s(&fp, pathconfkana, WccsUNICODE);
	if(fp != NULL)
	{
		for(i=0; i<count && i<ROMAN_KANA_TBL_NUM; i++)
		{
			if(roman_kana_conv[i].roman[0] == L'\0' &&
				roman_kana_conv[i].hiragana[0] == L'\0' &&
				roman_kana_conv[i].katakana[0] == L'\0' &&
				roman_kana_conv[i].katakana_ank[0] == L'\0')
			{
				continue;
			}
			fwprintf(fp, L"%s\t%s\t%s\t%s\t%d\n",
					 roman_kana_conv[i].roman,
					 roman_kana_conv[i].hiragana,
					 roman_kana_conv[i].katakana,
					 roman_kana_conv[i].katakana_ank,
					 roman_kana_conv[i].soku);
		}

		fclose(fp);
	}
}

void LoadConfigJLatin()
{
	FILE *fp;
	size_t t;
	wchar_t b[BUFSIZE];
	const wchar_t seps[] = L"\t\n\0";
	size_t sidx, eidx;
	ASCII_JLATIN_CONV conv;

	memcpy_s(ascii_jlatin_conv, sizeof(ascii_jlatin_conv),
		ascii_jlatin_conv_default, sizeof(ascii_jlatin_conv_default));

	_wfopen_s(&fp, pathconfjlat, RccsUNICODE);
	if(fp == NULL)
	{
		return;
	}

	ZeroMemory(b, sizeof(b));
	t = 0;
	while(fgetws(b, BUFSIZE, fp) != NULL)
	{
		if(t >= ASCII_JLATIN_TBL_NUM)
		{
			break;
		}

		ZeroMemory(&conv, sizeof(conv));

		sidx = 0;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(conv.ascii, _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(conv.jlatin, _TRUNCATE, L"%s", &b[sidx]);

		ZeroMemory(b, sizeof(b));

		if(conv.ascii[0] == L'\0' &&
			conv.jlatin[0] == L'\0')
		{
			continue;
		}

		ascii_jlatin_conv[t] = conv;
		++t;
	}
	if(t < ASCII_JLATIN_TBL_NUM)
	{
		ascii_jlatin_conv[t].ascii[0] = L'\0';
		ascii_jlatin_conv[t].jlatin[0] = L'\0';
	}

	fclose(fp);
}

void LoadJLatin(HWND hwnd)
{
	HWND hWndList;
	int i;
	LVITEMW item;

	LoadConfigJLatin();

	hWndList = GetDlgItem(hwnd, IDC_LIST_JLATTBL);

	for(i=0; ; i++)
	{
		if(ascii_jlatin_conv[i].ascii[0] == L'\0' &&
			ascii_jlatin_conv[i].jlatin[0] == L'\0')
		{
			break;
		}
		item.mask = LVIF_TEXT;
		item.pszText = ascii_jlatin_conv[i].ascii;
		item.iItem = i;
		item.iSubItem = 0;
		ListView_InsertItem(hWndList, &item);
		item.pszText = ascii_jlatin_conv[i].jlatin;
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem(hWndList, &item);
	}
}

void SaveJLatin(HWND hwnd)
{
	int i, count;
	HWND hWndListView;
	ASCII_JLATIN_CONV ajc;
	FILE *fp;

	hWndListView = GetDlgItem(hwnd, IDC_LIST_JLATTBL);
	count = ListView_GetItemCount(hWndListView);
	for(i=0; i<count && i<ASCII_JLATIN_TBL_NUM; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, ajc.ascii, _countof(ajc.ascii));
		ListView_GetItemText(hWndListView, i, 1, ajc.jlatin, _countof(ajc.jlatin));
		ascii_jlatin_conv[i] = ajc;
	}
	if(count < ASCII_JLATIN_TBL_NUM)
	{
		ascii_jlatin_conv[count].ascii[0] = L'\0';
		ascii_jlatin_conv[count].jlatin[0] = L'\0';
	}

	_wfopen_s(&fp, pathconfjlat, WccsUNICODE);
	if(fp != NULL)
	{
		for(i=0; i<count && i<ASCII_JLATIN_TBL_NUM; i++)
		{
			if(ascii_jlatin_conv[i].ascii[0] == L'\0' &&
				ascii_jlatin_conv[i].jlatin[0] == L'\0')
			{
				continue;
			}
			fwprintf(fp, L"%s\t%s\n",
					 ascii_jlatin_conv[i].ascii,
					 ascii_jlatin_conv[i].jlatin);
		}

		fclose(fp);
	}
}
