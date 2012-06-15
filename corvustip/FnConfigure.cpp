
#include "common.h"
#include "configxml.h"
#include "corvustip.h"
#include "TextService.h"
#include "convtype.h"

#define BUFSIZE		0x100

// キー キー設定
typedef struct {
	BYTE skkfunc;
	LPCWSTR keyname;
} CONFIG_KEYMAP;
const CONFIG_KEYMAP configkeymap[] =
{
	 {SKK_KANA,			KeyMapKana}
	,{SKK_CONV_CHAR,	KeyMapConvChar}
	,{SKK_JLATIN,		KeyMapJLatin}
	,{SKK_ASCII,		KeyMapAscii}
	,{SKK_JMODE,		KeyMapJMode}
	,{SKK_ABBREV,		KeyMapAbbrev}
	,{SKK_AFFIX,		KeyMapAffix}
	,{SKK_DIRECT,		KeyMapDirect}
	,{SKK_NEXT_CAND,	KeyMapNextCand}
	,{SKK_PREV_CAND,	KeyMapPrevCand}
	,{SKK_PURGE_DIC,	KeyMapPurgeDic}
	,{SKK_NEXT_COMP,	KeyMapNextComp}
	,{SKK_PREV_COMP,	KeyMapPrevComp}
	,{SKK_ENTER,		KeyMapEnter}
	,{SKK_CANCEL,		KeyMapCancel}
	,{SKK_BACK,			KeyMapBack}
	,{SKK_DELETE,		KeyMapDelete}
	,{SKK_VOID,			KeyMapVoid}
	,{SKK_LEFT,			KeyMapLeft}
	,{SKK_UP,			KeyMapUp}
	,{SKK_RIGHT,		KeyMapRight}
	,{SKK_DOWN,			KeyMapDown}
	,{SKK_PASTE,		KeyMapPaste}
	,{SKK_NULL,			L""}
};

void CTextService::_CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfigxml[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, NULL, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	wcsncpy_s(pathconfigxml, appdata, _TRUNCATE);
	wcsncat_s(pathconfigxml, fnconfigxml, _TRUNCATE);

	HANDLE hToken;
	PTOKEN_USER pTokenUser;
	DWORD dwLength;
	LPWSTR pszUserSid = L"";
	WCHAR szDigest[32+1];
	MD5_DIGEST digest;

	ZeroMemory(pipename, sizeof(pipename));
	ZeroMemory(szDigest, sizeof(szDigest));

	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
		pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);

		if(GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
		{
			ConvertSidToStringSidW(pTokenUser->User.Sid, &pszUserSid);
		}

		LocalFree(pTokenUser);
		CloseHandle(hToken);
	}

	if(GetMD5(&digest, (const BYTE *)pszUserSid, (DWORD)wcslen(pszUserSid)*sizeof(WCHAR)))
	{
		for(int i=0; i<_countof(digest.digest); i++)
		{
			_snwprintf_s(&szDigest[i*2], _countof(szDigest)-i*2, _TRUNCATE, L"%02x", digest.digest[i]);
		}
	}

	_snwprintf_s(pipename, _TRUNCATE, L"%s%s", CORVUSSRVPIPE, szDigest);
	_snwprintf_s(srvmutexname, _TRUNCATE, L"%s%s", CORVUSSRVMUTEX, szDigest);
	_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", CORVUSCNFMUTEX, szDigest);

	LocalFree(pszUserSid);
}

void CTextService::_LoadBehavior()
{
	RECT rect;
	std::wstring strxmlval;
	PACL pDacl;
	PSECURITY_DESCRIPTOR pSD;
	LPWSTR pszSD;
	ULONG ulSD;
	BOOL bAppContainer = FALSE;

	if(IsVersion62AndOver(g_ovi))
	{
		if(GetSecurityInfo(GetCurrentProcess(), SE_KERNEL_OBJECT,
			DACL_SECURITY_INFORMATION | LABEL_SECURITY_INFORMATION,
			NULL, NULL, &pDacl, NULL, &pSD) == ERROR_SUCCESS)
		{
			if(ConvertSecurityDescriptorToStringSecurityDescriptorW(pSD, SDDL_REVISION_1,
				DACL_SECURITY_INFORMATION | LABEL_SECURITY_INFORMATION, &pszSD, &ulSD))
			{
				// for Windows 8 Application Package Authority
				if(wcsstr(pszSD, L"S-1-15-2") != NULL)
				{
					bAppContainer = TRUE;
				}
				LocalFree(pszSD);
			}
			LocalFree(pSD);
		}
	}

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

	ReadValue(pathconfigxml, SectionFont, MaxWidth, strxmlval);
	maxwidth = strxmlval.empty() ? -1 : _wtol(strxmlval.c_str());
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	if(maxwidth < 0 || maxwidth > rect.right)
	{
		maxwidth = rect.right;
	}

	ReadValue(pathconfigxml, SectionBehavior, VisualStyle, strxmlval);
	visualstyle = _wtoi(strxmlval.c_str());
	if(visualstyle != TRUE && visualstyle != FALSE)
	{
		visualstyle = FALSE;
	}

	ReadValue(pathconfigxml, SectionBehavior, UntilCandList, strxmlval);
	untilcandlist = _wtoi(strxmlval.c_str());
	if(untilcandlist > 8 || (untilcandlist < 0))
	{
		untilcandlist = 4;
	}

	if(bAppContainer)
	{
		untilcandlist = 0;
	}

	ReadValue(pathconfigxml, SectionBehavior, DispCandNo, strxmlval);
	dispcandnum = _wtoi(strxmlval.c_str());
	if(dispcandnum != TRUE && dispcandnum != FALSE)
	{
		dispcandnum = FALSE;
	}

	ReadValue(pathconfigxml, SectionBehavior, Annotation, strxmlval);
	annotation = _wtoi(strxmlval.c_str());
	if(annotation != TRUE && annotation != FALSE)
	{
		annotation = FALSE;
	}

	ReadValue(pathconfigxml, SectionBehavior, NoModeMark, strxmlval);
	nomodemark = _wtoi(strxmlval.c_str());
	if(nomodemark != TRUE && nomodemark != FALSE)
	{
		nomodemark = FALSE;
	}

	ReadValue(pathconfigxml, SectionBehavior, NoOkuriConv, strxmlval);
	nookuriconv = _wtoi(strxmlval.c_str());
	if(nookuriconv != TRUE && nookuriconv != FALSE)
	{
		nookuriconv = FALSE;
	}

	ReadValue(pathconfigxml, SectionBehavior, DelOkuriCncl, strxmlval);
	delokuricncl = _wtoi(strxmlval.c_str());
	if(delokuricncl != TRUE && delokuricncl != FALSE)
	{
		delokuricncl = FALSE;
	}

	ReadValue(pathconfigxml, SectionBehavior, BackIncEnter, strxmlval);
	backincenter = _wtoi(strxmlval.c_str());
	if(backincenter != TRUE && backincenter != FALSE)
	{
		backincenter = FALSE;
	}

	ReadValue(pathconfigxml, SectionBehavior, AddCandKtkn, strxmlval);
	addcandktkn = _wtoi(strxmlval.c_str());
	if(addcandktkn != TRUE && addcandktkn != FALSE)
	{
		addcandktkn = FALSE;
	}
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

void CTextService::_LoadKeyMap()
{
	size_t i;
	WCHAR ch;
	WCHAR key[2];
	WCHAR keyre[KEYRELEN];
	std::wstring s;
	std::wregex re;
	std::wstring strxmlval;

	ZeroMemory(keymap_jmode, sizeof(keymap_jmode));
	ZeroMemory(keymap_latin, sizeof(keymap_latin));
	ZeroMemory(keymap_void, sizeof(keymap_void));
	key[1] = L'\0';

	for(i=0; i<_countof(configkeymap); i++)
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
			for(ch=0x01; ch<KEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						if(keymap_latin[ch] != SKK_JMODE)	//「ひらがな」が優先
						{
							keymap_latin[ch] = configkeymap[i].skkfunc;
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
						keymap_jmode[ch] = configkeymap[i].skkfunc;
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
						keymap_void[ch] = configkeymap[i].skkfunc;
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
	WCHAR *pszb;
	size_t blen;

	ZeroMemory(roman_kana_conv, sizeof(roman_kana_conv));

	if(ReadList(pathconfigxml, SectionKana, list) == S_OK)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < ROMAN_KANA_TBL_NUM; l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				pszb = NULL;

				if(r_itr->first == AttributeRoman)
				{
					pszb = roman_kana_conv[i].roman;
					blen = _countof(roman_kana_conv[i].roman);
				}
				else if(r_itr->first == AttributeHiragana)
				{
					pszb = roman_kana_conv[i].hiragana;
					blen = _countof(roman_kana_conv[i].hiragana);
				}
				else if(r_itr->first == AttributeKatakana)
				{
					pszb = roman_kana_conv[i].katakana;
					blen = _countof(roman_kana_conv[i].katakana);
				}
				else if(r_itr->first == AttributeKatakanaAnk)
				{
					pszb = roman_kana_conv[i].katakana_ank;
					blen = _countof(roman_kana_conv[i].katakana_ank);
				}
				else if(r_itr->first == AttributeSoku)
				{
					roman_kana_conv[i].soku = _wtoi(r_itr->second.c_str());
				}

				if(pszb != NULL)
				{
					wcsncpy_s(pszb, blen, r_itr->second.c_str(), _TRUNCATE);
				}
			}

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
					wcsncpy_s(pszb, blen, r_itr->second.c_str(), _TRUNCATE);
				}
			}

			i++;
		}
	}
}
