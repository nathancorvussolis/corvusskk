
#include "configxml.h"
#include "imcrvtip.h"
#include "TextService.h"
#include "convtype.h"

// キー キー設定
typedef struct {
	BYTE skkfunc;
	LPCWSTR keyname;
} CONFIG_KEYMAP;
const CONFIG_KEYMAP configkeymap[] =
{
	 {SKK_KANA,			ValueKeyMapKana}
	,{SKK_CONV_CHAR,	ValueKeyMapConvChar}
	,{SKK_JLATIN,		ValueKeyMapJLatin}
	,{SKK_ASCII,		ValueKeyMapAscii}
	,{SKK_JMODE,		ValueKeyMapJMode}
	,{SKK_ABBREV,		ValueKeyMapAbbrev}
	,{SKK_AFFIX,		ValueKeyMapAffix}
	,{SKK_NEXT_CAND,	ValueKeyMapNextCand}
	,{SKK_PREV_CAND,	ValueKeyMapPrevCand}
	,{SKK_PURGE_DIC,	ValueKeyMapPurgeDic}
	,{SKK_NEXT_COMP,	ValueKeyMapNextComp}
	,{SKK_PREV_COMP,	ValueKeyMapPrevComp}
	,{SKK_CONV_POINT,	ValueKeyMapConvPoint}
	,{SKK_DIRECT,		ValueKeyMapDirect}
	,{SKK_ENTER,		ValueKeyMapEnter}
	,{SKK_CANCEL,		ValueKeyMapCancel}
	,{SKK_BACK,			ValueKeyMapBack}
	,{SKK_DELETE,		ValueKeyMapDelete}
	,{SKK_VOID,			ValueKeyMapVoid}
	,{SKK_LEFT,			ValueKeyMapLeft}
	,{SKK_UP,			ValueKeyMapUp}
	,{SKK_RIGHT,		ValueKeyMapRight}
	,{SKK_DOWN,			ValueKeyMapDown}
	,{SKK_PASTE,		ValueKeyMapPaste}
	,{SKK_NULL,			L""}
};

static const TF_PRESERVEDKEY c_PreservedKey[] =
{
	 { VK_OEM_3/*0xC0*/, TF_MOD_ALT }
	,{ VK_KANJI/*0x19*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ VK_OEM_AUTO/*0xF3*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ VK_OEM_ENLW/*0xF4*/, TF_MOD_IGNORE_ALL_MODIFIER }
};

static struct {
	LPCWSTR value;
	COLORREF col;
} colorsxmlvalue[8] = {
	{ValueColorBG, RGB(0xFF,0xFF,0xFF)},
	{ValueColorFR, RGB(0x00,0x00,0x00)},
	{ValueColorSE, RGB(0x00,0x00,0xFF)},
	{ValueColorCO, RGB(0x80,0x80,0x80)},
	{ValueColorCA, RGB(0x00,0x00,0x00)},
	{ValueColorSC, RGB(0x80,0x80,0x80)},
	{ValueColorAN, RGB(0x80,0x80,0x80)},
	{ValueColorNO, RGB(0x00,0x00,0x00)}
};

void CTextService::_CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfigxml[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	wcsncpy_s(pathconfigxml, appdata, _TRUNCATE);
	wcsncat_s(pathconfigxml, fnconfigxml, _TRUNCATE);

	LPWSTR pszUserSid;
	WCHAR szDigest[32+1];
	MD5_DIGEST digest;
	int i;

	ZeroMemory(mgrpipename, sizeof(mgrpipename));
	ZeroMemory(szDigest, sizeof(szDigest));

	if(GetUserSid(&pszUserSid))
	{
		if(GetMD5(&digest, (CONST BYTE *)pszUserSid, (DWORD)wcslen(pszUserSid)*sizeof(WCHAR)))
		{
			for(i=0; i<_countof(digest.digest); i++)
			{
				_snwprintf_s(&szDigest[i*2], _countof(szDigest)-i*2, _TRUNCATE, L"%02x", digest.digest[i]);
			}
		}

		LocalFree(pszUserSid);
	}

	_snwprintf_s(mgrpipename, _TRUNCATE, L"%s%s", CORVUSMGRPIPE, szDigest);
	_snwprintf_s(mgrmutexname, _TRUNCATE, L"%s%s", CORVUSMGRMUTEX, szDigest);
	_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", CORVUSCNFMUTEX, szDigest);
}

void CTextService::_ReadBoolValue(LPCWSTR key, BOOL &value)
{
	std::wstring strxmlval;
	ReadValue(pathconfigxml, SectionBehavior, key, strxmlval);
	value = _wtoi(strxmlval.c_str());
	if(value != TRUE && value != FALSE)
	{
		value = FALSE;
	}
}

void CTextService::_LoadBehavior()
{
	RECT rect;
	std::wstring strxmlval;
	int i;

	ReadValue(pathconfigxml, SectionFont, ValueFontName, strxmlval);
	wcsncpy_s(fontname, strxmlval.c_str(), _TRUNCATE);
	
	ReadValue(pathconfigxml, SectionFont, ValueFontSize, strxmlval);
	fontpoint = _wtoi(strxmlval.c_str());
	ReadValue(pathconfigxml, SectionFont, ValueFontWeight, strxmlval);
	fontweight = _wtoi(strxmlval.c_str());
	ReadValue(pathconfigxml, SectionFont, ValueFontItalic, strxmlval);
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

	ReadValue(pathconfigxml, SectionBehavior, ValueMaxWidth, strxmlval);
	maxwidth = strxmlval.empty() ? -1 : _wtol(strxmlval.c_str());
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	if(maxwidth < 0 || maxwidth > rect.right)
	{
		maxwidth = rect.right;
	}

	for(i=0; i<_countof(colors); i++)
	{
		colors[i] = colorsxmlvalue[i].col;
		ReadValue(pathconfigxml, SectionBehavior, colorsxmlvalue[i].value, strxmlval);
		if(!strxmlval.empty())
		{
			colors[i] = wcstoul(strxmlval.c_str(), NULL, 0);
		}
	}

	ReadValue(pathconfigxml, SectionBehavior, ValueUntilCandList, strxmlval);
	c_untilcandlist = _wtoi(strxmlval.c_str());
	if(c_untilcandlist > 8)
	{
		c_untilcandlist = 4;
	}

	_ReadBoolValue(ValueDispCandNo, c_dispcandnum);

	_ReadBoolValue(ValueAnnotation, c_annotation);

	_ReadBoolValue(ValueAnnotatLst, c_annotatlst);

	_ReadBoolValue(ValueNoModeMark, c_nomodemark);

	_ReadBoolValue(ValueNoOkuriConv, c_nookuriconv);

	_ReadBoolValue(ValueDelOkuriCncl, c_delokuricncl);

	_ReadBoolValue(ValueBackIncEnter, c_backincenter);

	_ReadBoolValue(ValueAddCandKtkn, c_addcandktkn);
}

void CTextService::_LoadSelKey()
{
	WCHAR num[2];
	WCHAR key[4];
	int i;
	std::wstring strxmlval;

	ZeroMemory(selkey, sizeof(selkey));

	for(i=0; i<MAX_SELKEY_C; i++)
	{
		num[0] = L'0' + i + 1;
		num[1] = L'\0';
		ReadValue(pathconfigxml, SectionSelKey, num, strxmlval);
		wcsncpy_s(key, strxmlval.c_str(), _TRUNCATE);
		selkey[i][0][0] = key[0];
		selkey[i][1][0] = key[1];
	}
}

void CTextService::_LoadPreservedKey()
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
					preservedkey[i].uModifiers =
						wcstoul(r_itr->second.c_str(), NULL, 0) & (TF_MOD_ALT | TF_MOD_CONTROL | TF_MOD_SHIFT);
					if((preservedkey[i].uModifiers & (TF_MOD_ALT | TF_MOD_CONTROL | TF_MOD_SHIFT)) == 0)
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
		for(i=0; i<_countof(c_PreservedKey); i++)
		{
			preservedkey[i] = c_PreservedKey[i];
		}
	}
}

void CTextService::_LoadKeyMap(LPCWSTR section, KEYMAP &keymap)
{
	size_t i;
	WCHAR ch;
	WCHAR key[2];
	WCHAR keyre[KEYRELEN];
	std::wstring s;
	std::wregex re;
	std::wstring strxmlval;

	ZeroMemory(&keymap, sizeof(keymap));
	key[1] = L'\0';

	for(i=0; i<_countof(configkeymap); i++)
	{
		if(configkeymap[i].skkfunc == SKK_NULL)
		{
			break;
		}
		ReadValue(pathconfigxml, section, configkeymap[i].keyname, strxmlval);
		wcsncpy_s(keyre, strxmlval.c_str(), _TRUNCATE);
		if(keyre[0] == L'\0')
		{
			continue;
		}

		//全英/アスキーモード
		switch(configkeymap[i].skkfunc)
		{
		case SKK_JMODE:
		case SKK_ENTER:
		case SKK_CANCEL:
		case SKK_BACK:
		case SKK_DELETE:
		case SKK_LEFT:
		case SKK_UP:
		case SKK_RIGHT:
		case SKK_DOWN:
		case SKK_PASTE:
			for(ch=0x01; ch<KEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						if(keymap.keylatin[ch] != SKK_JMODE)	//「ひらがな」が優先
						{
							keymap.keylatin[ch] = configkeymap[i].skkfunc;
						}
					}
				}
				catch(std::regex_error err)
				{
					break;
				}
			}
			break;
		default:
			break;
		}

		//ひらがな/カタカナモード
		switch(configkeymap[i].skkfunc)
		{
		case SKK_JMODE:
		case SKK_VOID:
			break;
		default:
			for(ch=0x01; ch<KEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						keymap.keyjmode[ch] = configkeymap[i].skkfunc;
					}
				}
				catch(std::regex_error err)
				{
					break;
				}
			}
			break;
		}

		//無効
		switch(configkeymap[i].skkfunc)
		{
		case SKK_VOID:
			for(ch=0x01; ch<KEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						keymap.keyvoid[ch] = configkeymap[i].skkfunc;
					}
				}
				catch(std::regex_error err)
				{
					break;
				}
			}
			break;
		default:
			break;
		}
	}
}

void CTextService::_LoadConvPoint()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;

	ZeroMemory(conv_point, sizeof(conv_point));

	if(ReadList(pathconfigxml, SectionConvPoint, list) == S_OK)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < CONV_POINT_NUM; l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				if(r_itr->first == AttributeCPStart)
				{
					conv_point[i][0] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPAlter)
				{
					conv_point[i][1] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPOkuri)
				{
					conv_point[i][2] = r_itr->second.c_str()[0];
				}
			}

			i++;
		}
	}
}

void CTextService::_LoadKana()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;
	ROMAN_KANA_CONV rkc;
	WCHAR *pszb;
	size_t blen;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");

	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();

	if(ReadList(pathconfigxml, SectionKana, list) == S_OK)
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
					wcsncpy_s(pszb, blen, std::regex_replace(r_itr->second, re, fmt).c_str(), _TRUNCATE);
				}
			}

			roman_kana_conv.push_back(rkc);
			i++;
		}
	}
}

void CTextService::_LoadJLatin()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;
	WCHAR *pszb;
	size_t blen;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");

	ZeroMemory(ascii_jlatin_conv, sizeof(ascii_jlatin_conv));

	if(ReadList(pathconfigxml, SectionJLatin, list) == S_OK)
	{
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
					wcsncpy_s(pszb, blen, std::regex_replace(r_itr->second, re, fmt).c_str(), _TRUNCATE);
				}
			}

			i++;
		}
	}
}
