
#include "configxml.h"
#include "imcrvcnf.h"
#include "convtable.h"
#include "resource.h"

#define CONFKANALEN 0x100

WCHAR conv_point[CONV_POINT_NUM][3][2];
std::vector<ROMAN_KANA_CONV> roman_kana_conv;
ASCII_JLATIN_CONV ascii_jlatin_conv[ASCII_JLATIN_TBL_NUM];
TF_PRESERVEDKEY preservedkey[MAX_PRESERVEDKEY];

void LoadCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName)
{
	std::wstring strxmlval;

	ReadValue(pathconfigxml, lpAppName, lpKeyName, strxmlval);
	CheckDlgButton(hDlg, nIDDlgItem, (_wtoi(strxmlval.c_str()) == TRUE ? BST_CHECKED : BST_UNCHECKED));
}

void SaveCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName)
{
	WCHAR num[2];

	num[0] = L'0' + IsDlgButtonChecked(hDlg, nIDDlgItem);
	num[1] = L'\0';
	WriterKey(pXmlWriter, lpKeyName, num);
}

void LoadKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault)
{
	std::wstring strxmlval;
	LPCWSTR lpDefVal = L"\\";

	ReadValue(pathconfigxml, lpAppName, lpKeyName, strxmlval, lpDefVal);
	if(strxmlval == lpDefVal) strxmlval = lpDefault;
	SetDlgItemTextW(hDlg, nIDDlgItem, strxmlval.c_str());
}

void SaveKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName)
{
	WCHAR keyre[KEYRELEN];

	GetDlgItemTextW(hDlg, nIDDlgItem, keyre, _countof(keyre));
	WriterKey(pXmlWriter, lpKeyName, keyre);
}

void LoadConfigPreservedKey()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;

	ZeroMemory(preservedkey, sizeof(preservedkey));

	if(ReadList(pathconfigxml, SectionPreservedKey, list) == S_OK && list.size() != 0)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < MAX_PRESERVEDKEY; l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				if(r_itr->first == AttributeVKey)
				{
					preservedkey[i].uVKey = wcstoul(r_itr->second.c_str(), NULL, 0);
				}
				else if(r_itr->first == AttributeMKey)
				{
					preservedkey[i].uModifiers = wcstoul(r_itr->second.c_str(), NULL, 0);
					if(preservedkey[i].uModifiers == 0)
					{
						preservedkey[i].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
					}
				}
			}

			i++;
		}
	}
	else
	{
		preservedkey[0].uVKey = VK_OEM_3/*0xC0*/;
		preservedkey[0].uModifiers = TF_MOD_ALT;
		preservedkey[1].uVKey = VK_KANJI/*0x19*/;
		preservedkey[1].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
		preservedkey[2].uVKey = VK_OEM_AUTO/*0xF3*/;
		preservedkey[2].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
		preservedkey[3].uVKey = VK_OEM_ENLW/*0xF4*/;
		preservedkey[3].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
	}
}

void LoadPreservedKey(HWND hwnd)
{
	HWND hWndList;
	int i;
	LVITEMW item;
	WCHAR num[8];

	LoadConfigPreservedKey();

	hWndList = GetDlgItem(hwnd, IDC_LIST_PRSRVKEY);

	for(i=0; i<MAX_PRESERVEDKEY; i++)
	{
		if(preservedkey[i].uVKey == 0 &&
			preservedkey[i].uModifiers == 0)
		{
			break;
		}

		item.mask = LVIF_TEXT;
		_snwprintf_s(num, _TRUNCATE, L"0x%02X", preservedkey[i].uVKey);
		item.pszText = num;
		item.iItem = i;
		item.iSubItem = 0;
		ListView_InsertItem(hWndList, &item);
		_snwprintf_s(num, _TRUNCATE, L"%d", preservedkey[i].uModifiers & TF_MOD_ALT ? 1 : 0);
		item.pszText = num;
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem(hWndList, &item);
		_snwprintf_s(num, _TRUNCATE, L"%d", preservedkey[i].uModifiers & TF_MOD_CONTROL ? 1 : 0);
		item.pszText = num;
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem(hWndList, &item);
		_snwprintf_s(num, _TRUNCATE, L"%d", preservedkey[i].uModifiers & TF_MOD_SHIFT ? 1 : 0);
		item.pszText = num;
		item.iItem = i;
		item.iSubItem = 3;
		ListView_SetItem(hWndList, &item);
	}
}

void SavePreservedKey(HWND hwnd)
{
	int i, count;
	HWND hWndListView;
	WCHAR key[8];
	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;

	hWndListView = GetDlgItem(hwnd, IDC_LIST_PRSRVKEY);
	count = ListView_GetItemCount(hWndListView);
	for(i=0; i<count && i<MAX_PRESERVEDKEY; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, key, _countof(key));
		preservedkey[i].uVKey = wcstoul(key, NULL, 0);
		preservedkey[i].uModifiers = 0;
		ListView_GetItemText(hWndListView, i, 1, key, _countof(key));
		if(key[0] == L'1')
		{
			preservedkey[i].uModifiers |= TF_MOD_ALT;
		}
		ListView_GetItemText(hWndListView, i, 2, key, _countof(key));
		if(key[0] == L'1')
		{
			preservedkey[i].uModifiers |= TF_MOD_CONTROL;
		}
		ListView_GetItemText(hWndListView, i, 3, key, _countof(key));
		if(key[0] == L'1')
		{
			preservedkey[i].uModifiers |= TF_MOD_SHIFT;
		}
	}
	if(count < MAX_PRESERVEDKEY)
	{
		preservedkey[count].uVKey = 0;
		preservedkey[count].uModifiers = 0;
	}

	WriterStartSection(pXmlWriter, SectionPreservedKey);

	for(i=0; i<MAX_PRESERVEDKEY; i++)
	{
		if(preservedkey[i].uVKey == 0 &&
			preservedkey[i].uModifiers == 0)
		{
			break;
		}

		attr.first = AttributeVKey;
		_snwprintf_s(key, _TRUNCATE, L"0x%02X", preservedkey[i].uVKey);
		attr.second = key;
		row.push_back(attr);

		attr.first = AttributeMKey;
		_snwprintf_s(key, _TRUNCATE, L"%X", preservedkey[i].uModifiers);
		attr.second = key;
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pXmlWriter, list);

	WriterEndSection(pXmlWriter);
}

void LoadConfigConvPoint()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;

	ZeroMemory(conv_point, sizeof(conv_point));

	if(ReadList(pathconfigxml, SectionConvPoint, list) == S_OK && list.size() != 0)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < CONV_POINT_NUM; l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				if(r_itr->first == AttributeCPStart)
				{
					conv_point[i][0][0] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPAlter)
				{
					conv_point[i][1][0] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPOkuri)
				{
					conv_point[i][2][0] = r_itr->second.c_str()[0];
				}
			}

			i++;
		}
	}
	else
	{
		for(i=0; i<26; i++)
		{
			conv_point[i][0][0] = L'A' + (WCHAR)i;
			conv_point[i][1][0] = L'a' + (WCHAR)i;
			conv_point[i][2][0] = L'a' + (WCHAR)i;
		}
	}
}

void LoadConvPoint(HWND hwnd)
{
	HWND hWndList;
	int i;
	LVITEMW item;

	LoadConfigConvPoint();

	hWndList = GetDlgItem(hwnd, IDC_LIST_CONVPOINT);

	for(i=0; i<CONV_POINT_NUM; i++)
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
	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;

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

	WriterStartSection(pXmlWriter, SectionConvPoint);

	for(i=0; i<CONV_POINT_NUM; i++)
	{
		if(conv_point[i][0][0] == L'\0' &&
			conv_point[i][1][0] == L'\0' &&
			conv_point[i][2][0] == L'\0')
		{
			break;
		}

		attr.first = AttributeCPStart;
		attr.second = conv_point[i][0];
		row.push_back(attr);

		attr.first = AttributeCPAlter;
		attr.second = conv_point[i][1];
		row.push_back(attr);

		attr.first = AttributeCPOkuri;
		attr.second = conv_point[i][2];
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pXmlWriter, list);

	WriterEndSection(pXmlWriter);
}

void LoadConfigKana()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	ROMAN_KANA_CONV rkc;
	int i = 0;
	WCHAR *pszb;
	size_t blen;

	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();

	if(ReadList(pathconfigxml, SectionKana, list) == S_OK && list.size() != 0)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < ROMAN_KANA_TBL_MAX; l_itr++)
		{
			ZeroMemory(&rkc, sizeof(rkc));

			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				pszb = NULL;

				if(r_itr->first == AttributeRoman)
				{
					pszb = rkc.roman;
					blen = _countof(rkc.roman);
				}
				else if(r_itr->first == AttributeHiragana)
				{
					pszb = rkc.hiragana;
					blen = _countof(rkc.hiragana);
				}
				else if(r_itr->first == AttributeKatakana)
				{
					pszb = rkc.katakana;
					blen = _countof(rkc.katakana);
				}
				else if(r_itr->first == AttributeKatakanaAnk)
				{
					pszb = rkc.katakana_ank;
					blen = _countof(rkc.katakana_ank);
				}
				else if(r_itr->first == AttributeSpOp)
				{
					rkc.soku = (_wtoi(r_itr->second.c_str()) & 0x1) ? TRUE : FALSE;
					rkc.wait = (_wtoi(r_itr->second.c_str()) & 0x2) ? TRUE : FALSE;
				}

				if(pszb != NULL)
				{
					wcsncpy_s(pszb, blen, r_itr->second.c_str(), _TRUNCATE);
				}
			}

			roman_kana_conv.push_back(rkc);
			i++;
		}
	}
	else
	{
		for(i=0; i<ROMAN_KANA_TBL_DEF_NUM; i++)
		{
			if(roman_kana_conv_default[i].roman[0] == L'\0')
			{
				break;
			}
			roman_kana_conv.push_back(roman_kana_conv_default[i]);
		}
	}
}

void LoadKana(HWND hwnd)
{
	HWND hWndList;
	int i, count;
	LVITEMW item;
	WCHAR soku[2];

	LoadConfigKana();

	hWndList = GetDlgItem(hwnd, IDC_LIST_KANATBL);
	count = (int)roman_kana_conv.size();

	for(i=0; i<count; i++)
	{
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
		soku[1] = L'\0';
		soku[0] = L'0' + (roman_kana_conv[i].soku ? 1 : 0) + (roman_kana_conv[i].wait ? 2 : 0);
		item.pszText = soku;
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
	WCHAR soku[2];
	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;

	hWndListView = GetDlgItem(hwnd, IDC_LIST_KANATBL);
	count = ListView_GetItemCount(hWndListView);
	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();

	for(i=0; i<count && i<ROMAN_KANA_TBL_MAX; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, rkc.roman, _countof(rkc.roman));
		ListView_GetItemText(hWndListView, i, 1, rkc.hiragana, _countof(rkc.hiragana));
		ListView_GetItemText(hWndListView, i, 2, rkc.katakana, _countof(rkc.katakana));
		ListView_GetItemText(hWndListView, i, 3, rkc.katakana_ank, _countof(rkc.katakana_ank));
		ListView_GetItemText(hWndListView, i, 4, soku, _countof(soku));
		((soku[0] - L'0') & 0x1) != 0 ? rkc.soku = TRUE : rkc.soku = FALSE;
		((soku[0] - L'0') & 0x2) != 0 ? rkc.wait = TRUE : rkc.wait = FALSE;

		roman_kana_conv.push_back(rkc);
	}

	WriterStartSection(pXmlWriter, SectionKana);

	for(i=0; i<count; i++)
	{
		attr.first = AttributeRoman;
		attr.second = roman_kana_conv[i].roman;
		row.push_back(attr);

		attr.first = AttributeHiragana;
		attr.second = roman_kana_conv[i].hiragana;
		row.push_back(attr);

		attr.first = AttributeKatakana;
		attr.second = roman_kana_conv[i].katakana;
		row.push_back(attr);

		attr.first = AttributeKatakanaAnk;
		attr.second = roman_kana_conv[i].katakana_ank;
		row.push_back(attr);

		attr.first = AttributeSpOp;
		soku[1] = L'\0';
		soku[0] = L'0' + (roman_kana_conv[i].soku ? 1 : 0) + (roman_kana_conv[i].wait ? 2 : 0);
		attr.second = soku;
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pXmlWriter, list);

	WriterEndSection(pXmlWriter);
}

void LoadConfigJLatin()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;
	WCHAR *pszb;
	size_t blen;

	if(ReadList(pathconfigxml, SectionJLatin, list) == S_OK && list.size() != 0)
	{
		ZeroMemory(ascii_jlatin_conv, sizeof(ascii_jlatin_conv));

		for(l_itr = list.begin(); l_itr != list.end() && i < ASCII_JLATIN_TBL_NUM; l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				pszb = NULL;

				if(r_itr->first == AttributeLatin)
				{
					pszb = ascii_jlatin_conv[i].ascii;
					blen = _countof(ascii_jlatin_conv[i].ascii);
				}
				else if(r_itr->first == AttributeJLatin)
				{
					pszb = ascii_jlatin_conv[i].jlatin;
					blen = _countof(ascii_jlatin_conv[i].jlatin);
				}

				if(pszb != NULL)
				{
					wcsncpy_s(pszb, blen, r_itr->second.c_str(), _TRUNCATE);
				}
			}

			i++;
		}
	}
	else
	{
		memcpy_s(ascii_jlatin_conv, sizeof(ascii_jlatin_conv),
			ascii_jlatin_conv_default, sizeof(ascii_jlatin_conv_default));
	}
}

void LoadJLatin(HWND hwnd)
{
	HWND hWndList;
	int i;
	LVITEMW item;

	LoadConfigJLatin();

	hWndList = GetDlgItem(hwnd, IDC_LIST_JLATTBL);

	for(i=0; i<ASCII_JLATIN_TBL_NUM; i++)
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
	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;

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

	WriterStartSection(pXmlWriter, SectionJLatin);

	for(i=0; ; i++)
	{
		if(ascii_jlatin_conv[i].ascii[0] == L'\0' &&
			ascii_jlatin_conv[i].jlatin[0] == L'\0')
		{
			break;
		}

		attr.first = AttributeLatin;
		attr.second = ascii_jlatin_conv[i].ascii;
		row.push_back(attr);

		attr.first = AttributeJLatin;
		attr.second = ascii_jlatin_conv[i].jlatin;
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pXmlWriter, list);

	WriterEndSection(pXmlWriter);
}


void LoadConfigKanaTxt(LPCWSTR path)
{
	FILE *fp;
	wchar_t b[CONFKANALEN];
	const wchar_t seps[] = L"\t\n\0";
	size_t sidx, eidx;
	ROMAN_KANA_CONV rkc;
	wchar_t soku[2];

	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();

	_wfopen_s(&fp, path, RccsUNICODE);
	if(fp == NULL)
	{
		return;
	}
	
	ZeroMemory(b, sizeof(b));

	while(fgetws(b, CONFKANALEN, fp) != NULL)
	{
		if(roman_kana_conv.size() >= ROMAN_KANA_TBL_MAX)
		{
			break;
		}

		ZeroMemory(&rkc, sizeof(rkc));

		sidx = 0;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(rkc.roman, _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(rkc.hiragana, _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(rkc.katakana, _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(rkc.katakana_ank, _TRUNCATE, L"%s", &b[sidx]);
		sidx += eidx + 1;
		eidx = wcscspn(&b[sidx], seps);
		b[sidx + eidx] = L'\0';
		_snwprintf_s(soku, _TRUNCATE, L"%s", &b[sidx]);
		rkc.soku = (_wtoi(soku) & 0x1) ? TRUE : FALSE;
		rkc.wait = (_wtoi(soku) & 0x2) ? TRUE : FALSE;

		ZeroMemory(b, sizeof(b));

		if(rkc.roman[0] == L'\0' &&
			rkc.hiragana[0] == L'\0' &&
			rkc.katakana[0] == L'\0' &&
			rkc.katakana_ank[0] == L'\0')
		{
			continue;
		}

		roman_kana_conv.push_back(rkc);
	}

	fclose(fp);
}

void LoadKanaTxt(HWND hwnd, LPCWSTR path)
{
	HWND hWndList;
	int i, count;
	LVITEMW item;
	WCHAR soku[2];

	LoadConfigKanaTxt(path);

	hWndList = GetDlgItem(hwnd, IDC_LIST_KANATBL);
	ListView_DeleteAllItems(hWndList);
	count = (int)roman_kana_conv.size();

	for(i=0; i<count; i++)
	{
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
		soku[1] = L'\0';
		soku[0] = L'0' + (roman_kana_conv[i].soku ? 1 : 0) + (roman_kana_conv[i].wait ? 2 : 0);
		item.pszText = soku;
		item.iItem = i;
		item.iSubItem = 4;
		ListView_SetItem(hWndList, &item);
	}
}
