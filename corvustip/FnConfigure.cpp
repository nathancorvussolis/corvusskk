
#include "corvustip.h"
#include "TextService.h"
#include "convtype.h"

#define BUFSIZE		0x100

#define CCSUNICODE L",ccs=UNICODE"
const WCHAR *RccsUNICODE = L"r" CCSUNICODE;
const WCHAR *WccsUNICODE = L"w" CCSUNICODE;

// ファイル名
const WCHAR *fnconfig = L"config.ini";
const WCHAR *fnconfcvpt = L"confcvpt.txt";
const WCHAR *fnconfkana = L"confkana.txt";
const WCHAR *fnconfjlat = L"confjlat.txt";

// config.ini セクション
const WCHAR *IniSecBehavior = L"Behavior";
const WCHAR *IniSecKeyMap = L"KeyMap";
const WCHAR *IniSecSelKey = L"SelKey";
// config.ini キー 動作
const WCHAR *FontName = L"FontName";
const WCHAR *FontStyle = L"FontStyle";
const WCHAR *MaxWidth = L"MaxWidth";
const WCHAR *VisualStyle = L"VisualStyle";
const WCHAR *UntilCandList = L"UntilCandList";
const WCHAR *DispCandNo = L"DispCandNo";
const WCHAR *Annotation = L"Annotation";
const WCHAR *NoModeMark = L"NoModeMark";
const WCHAR *NoOkuriConv = L"NoOkuriConv";
const WCHAR *DelOkuriCncl = L"DelOkuriCncl";
const WCHAR *BackIncEnter = L"BackIncEnter";
// config.ini キー キー設定
typedef struct {
	BYTE skkfunc;
	WCHAR *keyname;
} CONFIG_KEYMAP;
const CONFIG_KEYMAP configkeymap[] =
{
	 {SKK_KANA,			L"Kana"}
	,{SKK_CONV_CHAR,	L"ConvChar"}
	,{SKK_JLATIN,		L"JLatin"}
	,{SKK_ASCII,		L"Ascii"}
	,{SKK_JMODE,		L"JMode"}
	,{SKK_ABBREV,		L"Abbrev"}
	,{SKK_AFFIX,		L"Affix"}
	,{SKK_DIRECT,		L"Direct"}
	,{SKK_NEXT_CAND,	L"NextCand"}
	,{SKK_PREV_CAND,	L"PrevCand"}
	,{SKK_PURGE_DIC,	L"PurgeDic"}
	,{SKK_NEXT_COMP,	L"NextComp"}
	,{SKK_PREV_COMP,	L"PrevComp"}
	,{SKK_ENTER,		L"Enter"}
	,{SKK_CANCEL,		L"Cancel"}
	,{SKK_BACK,			L"Back"}
	,{SKK_DELETE,		L"Delete"}
	,{SKK_VOID,			L"Void"}
	,{SKK_LEFT,			L"Left"}
	,{SKK_UP,			L"Up"}
	,{SKK_RIGHT,		L"Right"}
	,{SKK_DOWN,			L"Down"}
	,{SKK_PASTE,		L"Paste"}
	,{SKK_NULL,			L""}
};

void CTextService::_CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfig[0] = L'\0';
	pathconfcvpt[0] = L'\0';
	pathconfkana[0] = L'\0';
	pathconfjlat[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, NULL, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	wcsncpy_s(pathconfig, appdata, _TRUNCATE);
	wcsncat_s(pathconfig, fnconfig, _TRUNCATE);

	wcsncpy_s(pathconfcvpt, appdata, _TRUNCATE);
	wcsncat_s(pathconfcvpt, fnconfcvpt, _TRUNCATE);

	wcsncpy_s(pathconfkana, appdata, _TRUNCATE);
	wcsncat_s(pathconfkana, fnconfkana, _TRUNCATE);

	wcsncpy_s(pathconfjlat, appdata, _TRUNCATE);
	wcsncat_s(pathconfjlat, fnconfjlat, _TRUNCATE);

	HANDLE hToken;
	PTOKEN_USER pTokenUser;
	DWORD dwLength;
	LPWSTR pszUserSid;
	WCHAR szUserSid[256];
	WCHAR szDigest[32+1];
	MD5_DIGEST digest;

	ZeroMemory(pipename, sizeof(pipename));
	ZeroMemory(szUserSid, sizeof(szUserSid));
	ZeroMemory(szDigest, sizeof(szDigest));

	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
		pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);

		if(GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
		{
			if(ConvertSidToStringSidW(pTokenUser->User.Sid, &pszUserSid))
			{
				wcsncpy_s(szUserSid, pszUserSid, _TRUNCATE);
				LocalFree(pszUserSid);
			}
		}

		LocalFree(pTokenUser);
		CloseHandle(hToken);
	}

	if(_GetMD5(&digest, (const BYTE *)szUserSid, (DWORD)wcslen(szUserSid)*sizeof(WCHAR)))
	{
		for(int i=0; i<_countof(digest.digest); i++)
		{
			_snwprintf_s(&szDigest[i*2], _countof(szDigest)-i*2, _TRUNCATE, L"%02x", digest.digest[i]);
		}
	}

	_snwprintf_s(pipename, _TRUNCATE, L"%s%s", CORVUSSRVPIPE, szDigest);
}

BOOL CTextService::_GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen)
{
    BOOL bRet = FALSE;
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;
    BYTE *pbData;
	DWORD dwDataLen;

	if(digest == NULL)
	{
		return FALSE;
	}

    ZeroMemory(digest, sizeof(digest));
    pbData = digest->digest;
	dwDataLen = sizeof(digest->digest);
    
    if(CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET))
	{
        if(CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
		{
            if(CryptHashData(hHash, data, datalen, 0))
			{
                if(CryptGetHashParam(hHash, HP_HASHVAL, pbData, &dwDataLen, 0))
				{
                    bRet = TRUE;
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    return bRet;
}

void CTextService::_LoadBehavior()
{
	WCHAR num[16];
	RECT rect;

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

	GetPrivateProfileStringW(IniSecBehavior, MaxWidth, L"-1", num, _countof(num), pathconfig);
	maxwidth = _wtol(num);
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	if(maxwidth < 0 || maxwidth > rect.right)
	{
		maxwidth = rect.right;
	}

	GetPrivateProfileStringW(IniSecBehavior, VisualStyle, L"0", num, 2, pathconfig);
	visualstyle = _wtoi(num);
	if(visualstyle != TRUE && visualstyle != FALSE)
	{
		visualstyle = FALSE;
	}

	GetPrivateProfileStringW(IniSecBehavior, UntilCandList, L"4", num, 2, pathconfig);
	untilcandlist = _wtoi(num);
	if(untilcandlist > 8 || (untilcandlist == 0 && num[0] != L'0'))
	{
		untilcandlist = 4;
	}

	GetPrivateProfileStringW(IniSecBehavior, DispCandNo, L"0", num, 2, pathconfig);
	dispcandnum = _wtoi(num);
	if(dispcandnum != TRUE && dispcandnum != FALSE)
	{
		dispcandnum = FALSE;
	}

	GetPrivateProfileStringW(IniSecBehavior, Annotation, L"0", num, 2, pathconfig);
	annotation = _wtoi(num);
	if(annotation != TRUE && annotation != FALSE)
	{
		annotation = FALSE;
	}

	GetPrivateProfileStringW(IniSecBehavior, NoModeMark, L"0", num, 2, pathconfig);
	nomodemark = _wtoi(num);
	if(nomodemark != TRUE && nomodemark != FALSE)
	{
		nomodemark = FALSE;
	}

	GetPrivateProfileStringW(IniSecBehavior, NoOkuriConv, L"0", num, 2, pathconfig);
	nookuriconv = _wtoi(num);
	if(nookuriconv != TRUE && nookuriconv != FALSE)
	{
		nookuriconv = FALSE;
	}

	GetPrivateProfileStringW(IniSecBehavior, DelOkuriCncl, L"0", num, 2, pathconfig);
	delokuricncl = _wtoi(num);
	if(delokuricncl != TRUE && delokuricncl != FALSE)
	{
		delokuricncl = FALSE;
	}

	GetPrivateProfileStringW(IniSecBehavior, BackIncEnter, L"0", num, 2, pathconfig);
	backincenter = _wtoi(num);
	if(backincenter != TRUE && backincenter != FALSE)
	{
		backincenter = FALSE;
	}
}

void CTextService::_LoadSelKey()
{
	WCHAR num[2];
	WCHAR key[4];
	int i;

	ZeroMemory(selkey, sizeof(selkey));

	for(i=0; i<MAX_SELKEY_C; i++)
	{
		_snwprintf_s(num, _TRUNCATE, L"%d", i+1);
		GetPrivateProfileStringW(IniSecSelKey, num, num, key, _countof(key), pathconfig);
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
		GetPrivateProfileStringW(IniSecKeyMap, configkeymap[i].keyname,
			L"", keyre, _countof(keyre), pathconfig);
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
	FILE *fp;
	size_t t;
	wchar_t b[BUFSIZE];
	const wchar_t seps[] = L"\t\n\0";
	size_t sidx, eidx;
	wchar_t key[3][2];

	ZeroMemory(conv_point, sizeof(conv_point));

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

		conv_point[t][0] = key[0][0];
		conv_point[t][1] = key[1][0];
		conv_point[t][2] = key[2][0];
		++t;
	}
	if(t <CONV_POINT_NUM)
	{
		conv_point[t][0] = L'\0';
		conv_point[t][1] = L'\0';
		conv_point[t][2] = L'\0';
	}

	fclose(fp);
}

void CTextService::_LoadKana()
{
	FILE *fp;
	size_t t;
	wchar_t b[BUFSIZE];
	const wchar_t seps[] = L"\t\n\0";
	size_t sidx, eidx;
	ROMAN_KANA_CONV conv;
	wchar_t soku[2];

	ZeroMemory(roman_kana_conv, sizeof(roman_kana_conv));

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

void CTextService::_LoadJLatin()
{
	FILE *fp;
	size_t t;
	wchar_t b[BUFSIZE];
	const wchar_t seps[] = L"\t\n\0";
	size_t sidx, eidx;
	ASCII_JLATIN_CONV conv;

	ZeroMemory(ascii_jlatin_conv, sizeof(ascii_jlatin_conv));

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
	if(t <ASCII_JLATIN_TBL_NUM)
	{
		ascii_jlatin_conv[t].ascii[0] = L'\0';
		ascii_jlatin_conv[t].jlatin[0] = L'\0';
	}

	fclose(fp);
}
