
#include "configxml.h"
#include "convtype.h"
#include "imcrvtip.h"
#include "TextService.h"

static const struct {
	BYTE skkfunc;
	LPCWSTR keyname;
} configkeymap[] =
{
	{SKK_KANA,		ValueKeyMapKana},
	{SKK_CONV_CHAR,	ValueKeyMapConvChar},
	{SKK_JLATIN,	ValueKeyMapJLatin},
	{SKK_ASCII,		ValueKeyMapAscii},
	{SKK_JMODE,		ValueKeyMapJMode},
	{SKK_ABBREV,	ValueKeyMapAbbrev},
	{SKK_AFFIX,		ValueKeyMapAffix},
	{SKK_NEXT_CAND,	ValueKeyMapNextCand},
	{SKK_PREV_CAND,	ValueKeyMapPrevCand},
	{SKK_PURGE_DIC,	ValueKeyMapPurgeDic},
	{SKK_NEXT_COMP,	ValueKeyMapNextComp},
	{SKK_PREV_COMP,	ValueKeyMapPrevComp},
	{SKK_HINT,		ValueKeyMapHint},
	{SKK_CONV_POINT,ValueKeyMapConvPoint},
	{SKK_DIRECT,	ValueKeyMapDirect},
	{SKK_ENTER,		ValueKeyMapEnter},
	{SKK_CANCEL,	ValueKeyMapCancel},
	{SKK_BACK,		ValueKeyMapBack},
	{SKK_DELETE,	ValueKeyMapDelete},
	{SKK_VOID,		ValueKeyMapVoid},
	{SKK_LEFT,		ValueKeyMapLeft},
	{SKK_UP,		ValueKeyMapUp},
	{SKK_RIGHT,		ValueKeyMapRight},
	{SKK_DOWN,		ValueKeyMapDown},
	{SKK_PASTE,		ValueKeyMapPaste},
	{SKK_NULL,		L""}
};

static const TF_PRESERVEDKEY configpreservedkey[] =
{
	{VK_OEM_3		/*0xC0*/, TF_MOD_ALT},
	{VK_KANJI		/*0x19*/, TF_MOD_IGNORE_ALL_MODIFIER},
	{VK_OEM_AUTO	/*0xF3*/, TF_MOD_IGNORE_ALL_MODIFIER},
	{VK_OEM_ENLW	/*0xF4*/, TF_MOD_IGNORE_ALL_MODIFIER}
};

static const struct {
	LPCWSTR value;
	COLORREF color;
} colorsxmlvalue[8] =
{
	{ValueColorBG, RGB(0xFF,0xFF,0xFF)},
	{ValueColorFR, RGB(0x00,0x00,0x00)},
	{ValueColorSE, RGB(0x00,0x00,0xFF)},
	{ValueColorCO, RGB(0x80,0x80,0x80)},
	{ValueColorCA, RGB(0x00,0x00,0x00)},
	{ValueColorSC, RGB(0x80,0x80,0x80)},
	{ValueColorAN, RGB(0x80,0x80,0x80)},
	{ValueColorNO, RGB(0x00,0x00,0x00)}
};

LPCWSTR sectionpreservedkeyonoff[PRESERVEDKEY_NUM] = {SectionPreservedKeyON, SectionPreservedKeyOFF};

void CTextService::_CreateConfigPath()
{
	PWSTR appdatafolder = nullptr;

	ZeroMemory(pathconfigxml, sizeof(pathconfigxml));

	if(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DONT_VERIFY, nullptr, &appdatafolder) == S_OK)
	{
		_snwprintf_s(pathconfigxml, _TRUNCATE, L"%s\\%s\\%s", appdatafolder, TextServiceDesc, fnconfigxml);

		CoTaskMemFree(appdatafolder);
	}

	ZeroMemory(mgrpipename, sizeof(mgrpipename));
	ZeroMemory(mgrmutexname, sizeof(mgrmutexname));
	ZeroMemory(cnfmutexname, sizeof(cnfmutexname));

	LPWSTR pszUserUUID = nullptr;

	if(GetUserUUID(&pszUserUUID))
	{
		_snwprintf_s(mgrpipename, _TRUNCATE, L"%s%s", IMCRVMGRPIPE, pszUserUUID);
		_snwprintf_s(mgrmutexname, _TRUNCATE, L"%s%s", IMCRVMGRMUTEX, pszUserUUID);
		_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", IMCRVCNFMUTEX, pszUserUUID);

		LocalFree(pszUserUUID);
	}
}

void CTextService::_ReadBoolValue(LPCWSTR section, LPCWSTR key, BOOL &value, BOOL defval)
{
	std::wstring strxmlval;

	ReadValue(pathconfigxml, section, key, strxmlval, (defval ? L"1" : L"0"));
	value = _wtoi(strxmlval.c_str());
	if(value != TRUE && value != FALSE)
	{
		value = defval;
	}
}

void CTextService::_LoadBehavior()
{
	std::wstring strxmlval;

	//Behavior

	_ReadBoolValue(SectionBehavior, ValueBeginCvOkuri, cx_begincvokuri, TRUE);
	_ReadBoolValue(SectionBehavior, ValueShiftNNOkuri, cx_shiftnnokuri, TRUE);
	_ReadBoolValue(SectionBehavior, ValueSrchAllOkuri, cx_srchallokuri, FALSE);
	_ReadBoolValue(SectionBehavior, ValueDelCvPosCncl, cx_delcvposcncl, TRUE);
	_ReadBoolValue(SectionBehavior, ValueDelOkuriCncl, cx_delokuricncl, FALSE);
	_ReadBoolValue(SectionBehavior, ValueBackIncEnter, cx_backincenter, TRUE);
	_ReadBoolValue(SectionBehavior, ValueAddCandKtkn, cx_addcandktkn, FALSE);

	ReadValue(pathconfigxml, SectionBehavior, ValueCompMultiNum, strxmlval);
	cx_compmultinum = _wtoi(strxmlval.c_str());
	if(cx_compmultinum > MAX_SELKEY_C || cx_compmultinum < 1)
	{
		cx_compmultinum = COMPMULTIDISP_NUM;
	}
	_ReadBoolValue(SectionBehavior, ValueStaCompMulti, cx_stacompmulti, FALSE);
	_ReadBoolValue(SectionBehavior, ValueDynamicComp, cx_dynamiccomp, FALSE);
	_ReadBoolValue(SectionBehavior, ValueDynCompMulti, cx_dyncompmulti, FALSE);
	_ReadBoolValue(SectionBehavior, ValueCompUserDic, cx_compuserdic, FALSE);

	//Font

	ReadValue(pathconfigxml, SectionFont, ValueFontName, strxmlval);
	wcsncpy_s(cx_fontname, strxmlval.c_str(), _TRUNCATE);

	ReadValue(pathconfigxml, SectionFont, ValueFontSize, strxmlval);
	cx_fontpoint = _wtoi(strxmlval.c_str());
	ReadValue(pathconfigxml, SectionFont, ValueFontWeight, strxmlval);
	cx_fontweight = _wtoi(strxmlval.c_str());
	ReadValue(pathconfigxml, SectionFont, ValueFontItalic, strxmlval);
	cx_fontitalic = _wtoi(strxmlval.c_str());

	if(cx_fontpoint < 8 || cx_fontpoint > 72)
	{
		cx_fontpoint = 12;
	}
	if(cx_fontweight < 0 || cx_fontweight > 1000)
	{
		cx_fontweight = FW_NORMAL;
	}
	if(cx_fontitalic != FALSE)
	{
		cx_fontitalic = TRUE;
	}

	//Display

	ReadValue(pathconfigxml, SectionDisplay, ValueMaxWidth, strxmlval);
	cx_maxwidth = strxmlval.empty() ? -1 : _wtol(strxmlval.c_str());
	if(cx_maxwidth < 0)
	{
		cx_maxwidth = MAX_WIDTH_DEFAULT;
	}

	for(int i = 0; i < _countof(cx_colors); i++)
	{
		cx_colors[i] = colorsxmlvalue[i].color;
		ReadValue(pathconfigxml, SectionDisplay, colorsxmlvalue[i].value, strxmlval);
		if(!strxmlval.empty())
		{
			cx_colors[i] = wcstoul(strxmlval.c_str(), nullptr, 0);
		}
	}

	_ReadBoolValue(SectionDisplay, ValueDrawAPI, cx_drawapi, FALSE);
	_ReadBoolValue(SectionDisplay, ValueColorFont, cx_colorfont, FALSE);

	ReadValue(pathconfigxml, SectionDisplay, ValueUntilCandList, strxmlval);
	cx_untilcandlist = _wtoi(strxmlval.c_str());
	if(cx_untilcandlist > 9 || strxmlval.empty())
	{
		cx_untilcandlist = 5;
	}

	_ReadBoolValue(SectionDisplay, ValueDispCandNo, cx_dispcandnum, FALSE);
	_ReadBoolValue(SectionDisplay, ValueVerticalCand, cx_verticalcand, FALSE);
	_ReadBoolValue(SectionDisplay, ValueAnnotation, cx_annotation, TRUE);
	_ReadBoolValue(SectionDisplay, ValueAnnotatLst, cx_annotatlst, FALSE);
	_ReadBoolValue(SectionDisplay, ValueShowModeInl, cx_showmodeinl, FALSE);
	_ReadBoolValue(SectionDisplay, ValueShowModeImm, cx_showmodeimm, TRUE);
	_ReadBoolValue(SectionDisplay, ValueShowModeMark, cx_showmodemark, TRUE);
	_ReadBoolValue(SectionDisplay, ValueShowRoman, cx_showroman, TRUE);
}

void CTextService::_LoadDisplayAttr()
{
	std::wstring strxmlval;
	BOOL se;
	TF_DISPLAYATTRIBUTE da;

	for(int i = 0; i < DISPLAYATTRIBUTE_INFO_NUM; i++)
	{
		display_attribute_series[i] = c_gdDisplayAttributeInfo[i].se;
		display_attribute_info[i] = c_gdDisplayAttributeInfo[i].da;

		ReadValue(pathconfigxml, SectionDisplayAttr, c_gdDisplayAttributeInfo[i].key, strxmlval);
		if(!strxmlval.empty())
		{
			if(swscanf_s(strxmlval.c_str(), L"%d,%d,0x%06X,%d,0x%06X,%d,%d,%d,0x%06X,%d",
				&se, &da.crText.type, &da.crText.cr, &da.crBk.type, &da.crBk.cr,
				&da.lsStyle, &da.fBoldLine, &da.crLine.type, &da.crLine.cr, &da.bAttr) == 10)
			{
				display_attribute_series[i] = se;
				display_attribute_info[i] = da;
			}
		}
	}
}

void CTextService::_LoadSelKey()
{
	WCHAR num[2];
	WCHAR key[4];
	std::wstring strxmlval;

	ZeroMemory(selkey, sizeof(selkey));

	for(int i = 0; i < MAX_SELKEY_C; i++)
	{
		num[0] = L'0' + i + 1;
		num[1] = L'\0';
		ReadValue(pathconfigxml, SectionSelKey, num, strxmlval);
		wcsncpy_s(key, strxmlval.c_str(), _TRUNCATE);
		selkey[i][0][0] = key[0];
		selkey[i][1][0] = key[1];
	}
}

void CTextService::_SetPreservedKeyONOFF(int onoff, const APPDATAXMLLIST &list)
{
	if(onoff != 0 && onoff != 1)
	{
		return;
	}

	ZeroMemory(preservedkey[onoff], sizeof(preservedkey[onoff]));

	if(list.size() != 0)
	{
		int i = 0;
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= MAX_PRESERVEDKEY)
			{
				break;
			}

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				if(r_itr->first == AttributeVKey)
				{
					preservedkey[onoff][i].uVKey = wcstoul(r_itr->second.c_str(), nullptr, 0);
				}
				else if(r_itr->first == AttributeMKey)
				{
					preservedkey[onoff][i].uModifiers =
						wcstoul(r_itr->second.c_str(), nullptr, 0) & (TF_MOD_ALT | TF_MOD_CONTROL | TF_MOD_SHIFT);
					if((preservedkey[onoff][i].uModifiers & (TF_MOD_ALT | TF_MOD_CONTROL | TF_MOD_SHIFT)) == 0)
					{
						preservedkey[onoff][i].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
					}
				}
			}

			i++;
		}
	}
	else
	{
		for(int i = 0; i < _countof(configpreservedkey); i++)
		{
			preservedkey[onoff][i] = configpreservedkey[i];
		}
	}
}

void CTextService::_LoadPreservedKey()
{
	APPDATAXMLLIST list;

	//for compatibility
	HRESULT hr = ReadList(pathconfigxml, SectionPreservedKey, list);

	if(hr == S_OK && list.size() != 0)
	{
		for(int k = 0; k < PRESERVEDKEY_NUM; k++)
		{
			_SetPreservedKeyONOFF(k, list);
		}
	}
	else
	{
		for(int k = 0; k < PRESERVEDKEY_NUM; k++)
		{
			list.clear();
			hr = ReadList(pathconfigxml, sectionpreservedkeyonoff[k], list);
			_SetPreservedKeyONOFF(k, list);
		}
	}
}

void CTextService::_LoadCKeyMap()
{
	WCHAR key[2];
	WCHAR keyre[MAX_KEYRE];
	std::wstring s;
	std::wregex re;
	std::wstring strxmlval;

	ZeroMemory(&ckeymap, sizeof(ckeymap));
	key[1] = L'\0';

	for(int i = 0; i < _countof(configkeymap); i++)
	{
		if(configkeymap[i].skkfunc == SKK_NULL)
		{
			break;
		}
		ReadValue(pathconfigxml, SectionKeyMap, configkeymap[i].keyname, strxmlval);
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
			for(WCHAR ch = 0x01; ch < CKEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);

				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						if(ckeymap.keylatin[ch] != SKK_JMODE)	//「ひらがな」が優先
						{
							ckeymap.keylatin[ch] = configkeymap[i].skkfunc;
						}
					}
				}
				catch(...)
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
			for(WCHAR ch = 0x01; ch < CKEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);

				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						ckeymap.keyjmode[ch] = configkeymap[i].skkfunc;
					}
				}
				catch(...)
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
			for(WCHAR ch = 0x01; ch < CKEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);

				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						ckeymap.keyvoid[ch] = configkeymap[i].skkfunc;
					}
				}
				catch(...)
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

void CTextService::_LoadVKeyMap()
{
	WCHAR key[3];
	WCHAR keyre[MAX_KEYRE];
	std::wstring s;
	std::wregex re;
	std::wstring strxmlval;
	VKEYMAP *pkeymaps[] = {&vkeymap, &vkeymap_shift, &vkeymap_ctrl};

	for(int i = 0; i < _countof(pkeymaps); i++)
	{
		ZeroMemory(pkeymaps[i], sizeof(*pkeymaps[i]));
	}

	for(int i = 0; i < _countof(configkeymap); i++)
	{
		if(configkeymap[i].skkfunc == SKK_NULL)
		{
			break;
		}
		ReadValue(pathconfigxml, SectionVKeyMap, configkeymap[i].keyname, strxmlval);
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
			for(int j = 0; j < _countof(pkeymaps); j++)
			{
				for(WCHAR ch = 0x01; ch < VKEYMAPNUM; ch++)
				{
					switch(j)
					{
					case 0:
						key[0] = ch;
						key[1] = L'\0';
						break;
					case 1:
						key[0] = L'S';
						key[1] = ch;
						key[2] = L'\0';
						break;
					case 2:
						key[0] = L'C';
						key[1] = ch;
						key[2] = L'\0';
						break;
					}

					s.assign(key);

					try
					{
						re.assign(keyre);
						if(std::regex_match(s, re))
						{
							if(pkeymaps[j]->keylatin[ch] != SKK_JMODE)	//「ひらがな」が優先
							{
								pkeymaps[j]->keylatin[ch] = configkeymap[i].skkfunc;
							}
						}
					}
					catch(...)
					{
						break;
					}
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
			for(int j = 0; j < _countof(pkeymaps); j++)
			{
				for(WCHAR ch = 0x01; ch < VKEYMAPNUM; ch++)
				{
					switch(j)
					{
					case 0:
						key[0] = ch;
						key[1] = L'\0';
						break;
					case 1:
						key[0] = L'S';
						key[1] = ch;
						key[2] = L'\0';
						break;
					case 2:
						key[0] = L'C';
						key[1] = ch;
						key[2] = L'\0';
						break;
					}

					s.assign(key);

					try
					{
						re.assign(keyre);
						if(std::regex_match(s, re))
						{
							pkeymaps[j]->keyjmode[ch] = configkeymap[i].skkfunc;
						}
					}
					catch(...)
					{
						break;
					}
				}
			}
			break;
		}

		//無効
		switch(configkeymap[i].skkfunc)
		{
		case SKK_VOID:
			for(int j = 0; j < _countof(pkeymaps); j++)
			{
				for(WCHAR ch = 0x01; ch < VKEYMAPNUM; ch++)
				{
					switch(j)
					{
					case 0:
						key[0] = ch;
						key[1] = L'\0';
						break;
					case 1:
						key[0] = L'S';
						key[1] = ch;
						key[2] = L'\0';
						break;
					case 2:
						key[0] = L'C';
						key[1] = ch;
						key[2] = L'\0';
						break;
					}

					s.assign(key);

					try
					{
						re.assign(keyre);
						if(std::regex_match(s, re))
						{
							pkeymaps[j]->keyvoid[ch] = configkeymap[i].skkfunc;
						}
					}
					catch(...)
					{
						break;
					}
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
	CONV_POINT cp;

	conv_point_s.clear();
	conv_point_s.shrink_to_fit();
	conv_point_a.clear();
	conv_point_a.shrink_to_fit();

	HRESULT hr = ReadList(pathconfigxml, SectionConvPoint, list);

	if(hr == S_OK && list.size() != 0)
	{
		int i = 0;
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= MAX_CONV_POINT)
			{
				break;
			}

			ZeroMemory(&cp, sizeof(cp));

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				if(r_itr->first == AttributeCPStart)
				{
					cp.ch[0] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPAlter)
				{
					cp.ch[1] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPOkuri)
				{
					cp.ch[2] = r_itr->second.c_str()[0];
				}
			}

			auto vs_itr = std::lower_bound(conv_point_s.begin(), conv_point_s.end(),
				cp.ch[0], [] (CONV_POINT m, WCHAR v) { return (m.ch[0] < v); });

			if(vs_itr == conv_point_s.end() || cp.ch[0] != vs_itr->ch[0])
			{
				conv_point_s.insert(vs_itr, cp);
			}

			auto va_itr = std::lower_bound(conv_point_a.begin(), conv_point_a.end(),
				cp.ch[1], [] (CONV_POINT m, WCHAR v) { return (m.ch[1] < v); });

			if(va_itr == conv_point_a.end() || cp.ch[1] != va_itr->ch[1])
			{
				conv_point_a.insert(va_itr, cp);
			}

			i++;
		}
	}
}

void CTextService::_LoadKana()
{
	APPDATAXMLLIST list;
	ROMAN_KANA_CONV rkc;
	std::wregex re(L"[\\x00-\\x19]");
	std::wstring fmt(L"");

	roman_kana_tree.ch = L'\0';
	ZeroMemory(&roman_kana_tree.conv, sizeof(roman_kana_tree.conv));
	roman_kana_tree.nodes.clear();
	roman_kana_tree.nodes.shrink_to_fit();

	HRESULT hr = ReadList(pathconfigxml, SectionKana, list);

	if(hr == S_OK && list.size() != 0)
	{
		int i = 0;
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= ROMAN_KANA_TBL_MAX)
			{
				break;
			}

			ZeroMemory(&rkc, sizeof(rkc));

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				WCHAR *pszb = nullptr;
				size_t blen = 0;

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

				if(pszb != nullptr)
				{
					wcsncpy_s(pszb, blen, std::regex_replace(r_itr->second, re, fmt).c_str(), _TRUNCATE);
				}
			}

			_AddKanaTree(roman_kana_tree, rkc, 0);

			i++;
		}
	}
	else if(hr != S_OK)
	{
		ZeroMemory(&rkc, sizeof(rkc));

		for(WCHAR ch = 0x20; ch <= 0x7E; ch++)
		{
			rkc.roman[0] = ch;
			rkc.hiragana[0] = ch;
			rkc.katakana[0] = ch;
			rkc.katakana_ank[0] = ch;

			_AddKanaTree(roman_kana_tree, rkc, 0);
		}
	}
}

BOOL CTextService::_AddKanaTree(ROMAN_KANA_NODE &tree, ROMAN_KANA_CONV rkc, int depth)
{
	BOOL added = FALSE;

	if((_countof(rkc.roman) <= (depth + 1)) || (rkc.roman[depth] == L'\0'))
	{
		return FALSE;
	}

	auto v_itr = std::lower_bound(tree.nodes.begin(), tree.nodes.end(),
		rkc.roman[depth], [] (ROMAN_KANA_NODE m, WCHAR v) { return (m.ch < v); });

	if(v_itr != tree.nodes.end() && v_itr->ch == rkc.roman[depth])
	{
		if(rkc.roman[depth + 1] == L'\0')
		{
			if(v_itr->conv.roman[0] == L'\0')
			{
				//ローマ字探索最後のノードにローマ字仮名変換がなければ更新
				v_itr->conv = rkc;
			}
			added = TRUE;
		}
		else
		{
			//子ノードを探索
			added = _AddKanaTree(*v_itr, rkc, depth + 1);
		}
	}

	if(!added)
	{
		//子ノードを追加
		_AddKanaTreeItem(tree, rkc, depth);
		added = TRUE;
	}

	return added;
}

void CTextService::_AddKanaTreeItem(ROMAN_KANA_NODE &tree, ROMAN_KANA_CONV rkc, int depth)
{
	ROMAN_KANA_NODE rkn;

	ZeroMemory(&rkn, sizeof(rkn));

	if((_countof(rkc.roman) <= (depth + 1)) || (rkc.roman[depth] == L'\0'))
	{
		return;
	}

	rkn.ch = rkc.roman[depth];

	auto v_itr = std::lower_bound(tree.nodes.begin(), tree.nodes.end(),
		rkn.ch, [] (ROMAN_KANA_NODE m, WCHAR v) { return (m.ch < v); });

	if(rkc.roman[depth + 1] == L'\0')
	{
		//ローマ字探索最後のノードにローマ字仮名変換ありの子ノードを追加
		rkn.conv = rkc;
		tree.nodes.insert(v_itr, rkn);
	}
	else
	{
		//ローマ字探索途中のノードに探索対象のローマ字のみの子ノードを追加
		v_itr = tree.nodes.insert(v_itr, rkn);
		//子ノードを探索
		_AddKanaTreeItem(*v_itr, rkc, depth + 1);
	}
}

void CTextService::_LoadJLatin()
{
	APPDATAXMLLIST list;
	ASCII_JLATIN_CONV ajc;
	std::wregex re(L"[\\x00-\\x19]");
	std::wstring fmt(L"");

	ascii_jlatin_conv.clear();
	ascii_jlatin_conv.shrink_to_fit();

	HRESULT hr = ReadList(pathconfigxml, SectionJLatin, list);

	if(hr == S_OK && list.size() != 0)
	{
		int i = 0;
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= ASCII_JLATIN_TBL_NUM)
			{
				break;
			}

			ZeroMemory(&ajc, sizeof(ajc));

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				WCHAR *pszb = nullptr;
				size_t blen = 0;

				if(r_itr->first == AttributeLatin)
				{
					pszb = ajc.ascii;
					blen = _countof(ajc.ascii);
				}
				else if(r_itr->first == AttributeJLatin)
				{
					pszb = ajc.jlatin;
					blen = _countof(ajc.jlatin);
				}

				if(pszb != nullptr)
				{
					wcsncpy_s(pszb, blen, std::regex_replace(r_itr->second, re, fmt).c_str(), _TRUNCATE);
				}
			}

			auto v_itr = std::lower_bound(ascii_jlatin_conv.begin(), ascii_jlatin_conv.end(),
				ajc.ascii[0], [] (ASCII_JLATIN_CONV m, WCHAR v) { return (m.ascii[0] < v); });

			if(v_itr == ascii_jlatin_conv.end() || ajc.ascii[0] != v_itr->ascii[0])
			{
				ascii_jlatin_conv.insert(v_itr, ajc);
			}

			i++;
		}
	}
	else if(hr != S_OK)
	{
		ZeroMemory(&ajc, sizeof(ajc));

		for(WCHAR ch = 0x20; ch <= 0x7E; ch++)
		{
			ajc.ascii[0] = ch;
			ajc.jlatin[0] = ch;

			ascii_jlatin_conv.push_back(ajc);
		}
	}
}

void CTextService::_InitFont()
{
	HDC hdc = GetDC(nullptr);
	int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(nullptr, hdc);

	LOGFONTW logfont;
	logfont.lfHeight = -MulDiv(cx_fontpoint, dpi, 72);
	logfont.lfWidth = 0;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = cx_fontweight;
	logfont.lfItalic = cx_fontitalic;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	logfont.lfCharSet = SHIFTJIS_CHARSET;
	logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = PROOF_QUALITY;
	logfont.lfPitchAndFamily = DEFAULT_PITCH;
	wcscpy_s(logfont.lfFaceName, cx_fontname);

	if(hFont == nullptr)
	{
		hFont = CreateFontIndirectW(&logfont);
	}

	if(cx_drawapi && !_UILessMode && (_pD2DFactory == nullptr))
	{
		//try delay load
		__try
		{
			_drawtext_option = (IsWindowsVersion63OrLater() && cx_colorfont) ?
				D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT : D2D1_DRAW_TEXT_OPTIONS_NONE;

			HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_pD2DFactory);

			if(hr == S_OK)
			{
				D2D1_RENDER_TARGET_PROPERTIES d2dprops = D2D1::RenderTargetProperties(
					D2D1_RENDER_TARGET_TYPE_DEFAULT,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
					0.0F, 0.0F, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT);

				hr = _pD2DFactory->CreateDCRenderTarget(&d2dprops, &_pD2DDCRT);
			}

			if(hr == S_OK)
			{
				for(int i = 0; i < DISPLAY_COLOR_NUM; i++)
				{
					hr = _pD2DDCRT->CreateSolidColorBrush(D2D1::ColorF(SWAPRGB(cx_colors[i])), &_pD2DBrush[i]);
					if(hr != S_OK)
					{
						break;
					}
				}
			}

			if(hr == S_OK)
			{
				hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, IID_PUNK_ARGS(&_pDWFactory));
			}

			if(hr == S_OK)
			{
				hr = _pDWFactory->CreateTextFormat(cx_fontname, nullptr,
					static_cast<DWRITE_FONT_WEIGHT>(cx_fontweight),
					(cx_fontitalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL),
					DWRITE_FONT_STRETCH_NORMAL, (FLOAT)MulDiv(cx_fontpoint, dpi, 72), L"JPN", &_pDWTF);
			}

			if(hr == S_OK)
			{
				hr = _pDWTF->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
			}

			if(hr != S_OK)
			{
				_UninitFont();

				hFont = CreateFontIndirectW(&logfont);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			_UninitFont();
			//use GDI font
			hFont = CreateFontIndirectW(&logfont);
		}
	}
}

void CTextService::_UninitFont()
{
	if(hFont != nullptr)
	{
		DeleteObject(hFont);
		hFont = nullptr;
	}

	SafeRelease(&_pDWTF);
	SafeRelease(&_pDWFactory);
	for(int i = 0; i < DISPLAY_COLOR_NUM; i++)
	{
		SafeRelease(&_pD2DBrush[i]);
	}
	SafeRelease(&_pD2DDCRT);
	SafeRelease(&_pD2DFactory);
}
