
#include "eucjis2004.h"
#include "imcrvmgr.h"

std::wstring SearchUnicode(const std::wstring &searchkey)
{
	std::wstring candidate;
	//Unicode Code Point
	UCSCHAR ucp = 0;
	WCHAR utf16[3];

	// U+XXXXXX (XXXXXX : 0000-FFFF,10000-10FFFF)
	if(std::regex_match(searchkey, std::wregex(L"U\\+([1-9A-F]|10)?[0-9A-F]{4}")))
	{
		if(swscanf_s(searchkey.c_str(), L"U+%X", &ucp) != 1)
		{
			return candidate;
		}
	}
	// uxxxxxx (xxxxxx : 0000-ffff,10000-10ffff)
	else if(std::regex_match(searchkey, std::wregex(L"u([1-9a-f]|10)?[0-9a-f]{4}")))
	{
		if(swscanf_s(searchkey.c_str(), L"u%x", &ucp) != 1)
		{
			return candidate;
		}
	}
	else
	{
		return candidate;
	}

	ZeroMemory(utf16, sizeof(utf16));
	if(UcpToWideChar(ucp, &utf16[0], &utf16[1]) != 0)
	{
		candidate = L"/";
		candidate += utf16;
		candidate += L"/\n";
	}

	return candidate;
}

std::wstring SearchJISX0213(const std::wstring &searchkey)
{
	std::wstring candidate;
	//JIS X 0213 面区点番号
	int men, ku, ten;
	size_t size;
	WCHAR utf16[8];
	CHAR euc[4];
	CONST CHAR mask = 0x7F;
	CONST CHAR base = 0x20;
	UCSCHAR ucp[2];
	WCHAR sucp[32];

	if(!std::regex_match(searchkey, std::wregex(L"[12]-(0[1-9]|[1-8][0-9]|9[0-4])-(0[1-9]|[0-8][0-9]|9[0-4])")))
	{
		return candidate;
	}

	if(swscanf_s(searchkey.c_str(), L"%d-%d-%d", &men, &ku, &ten) != 3)
	{
		return candidate;
	}

	switch(men)
	{
	case 1:
		euc[0] = (ku + base) | ~mask;
		euc[1] = (ten + base) | ~mask;
		euc[2] = L'\0';
		break;
	case 2:
		euc[0] = 0x0F | ~mask;
		euc[1] = (ku + base) | ~mask;
		euc[2] = (ten + base) | ~mask;
		euc[3] = L'\0';
		break;
	default:
		return candidate;
		break;
	}

	if(EucJis2004ToUcp(euc, _countof(euc), &ucp[0], &ucp[1]) != 0)
	{
		if(ucp[1] == 0)
		{
			_snwprintf_s(sucp, _TRUNCATE, L"U+%04X", ucp[0]);
		}
		else
		{
			_snwprintf_s(sucp, _TRUNCATE, L"U+%04X+%04X", ucp[0], ucp[1]);
		}
	}

	size = _countof(utf16);
	if(EucJis2004ToWideChar(euc, NULL, utf16, &size))
	{
		candidate = L"/";
		candidate += utf16;
		candidate += L";";
		candidate += sucp;
		candidate += L"/\n";
	}

	return candidate;
}

#define SURROGATEPAIR_UCPMAX	0x10FFFF
#define SURROGATEPAIR_UCPMIN	0x10000
#define SURROGATEPAIR_MASK		0xFC00
#define SURROGATEPAIR_HIGH_MASK	0xD800
#define SURROGATEPAIR_LOW_MASK	0xDC00
#define SURROGATEPAIR_SEPBIT	10
#define SURROGATEPAIR_SEPMASK	0x3FF

std::wstring SearchCharacterCode(const std::wstring &searchkey)
{
	std::wstring candidate;
	//文字コード表記
	std::wstring e, u;
	WCHAR b[16];
	size_t len;
	UCSCHAR ucp;
	CONST CHAR as = 0x00;
	CONST CHAR ae = 0x7F;
	CONST CHAR mask = 0x7F;
	CONST CHAR ejs = 0x21;
	CONST CHAR ss2 = 0x0E;
	CONST CHAR ss3 = 0x0F;

	//ASCII, JIS X 0201 (片仮名, 8bit), JIS X 0213 面区点番号
	if(WideCharToEucJis2004(searchkey.c_str(), NULL, NULL, &len))
	{
		std::string euc = WCTOEUC(searchkey);

		for(size_t i = 0; i < euc.size(); i++)
		{
			if(as <= euc[i] && euc[i] <= ae)
			{
				_snwprintf_s(b, _TRUNCATE, L"%02X", euc[i]);
			}
			else if((euc[i] & mask) == ss3)
			{
				_snwprintf_s(b, _TRUNCATE, L"2-%02d-%02d",
					(euc[i + 1] & mask) - ejs + 1, (euc[i + 2] & mask) - ejs + 1);
				i += 2;
			}
			else if((euc[i] & mask) == ss2)
			{
				_snwprintf_s(b, _TRUNCATE, L"%02X", (UCHAR)euc[i + 1]);
				i++;
			}
			else
			{
				_snwprintf_s(b, _TRUNCATE, L"1-%02d-%02d",
					(euc[i] & mask) - ejs + 1, (euc[i + 1] & mask) - ejs + 1);
				i++;
			}

			if(!e.empty())
			{
				e += L",";
			}
			e += b;
		}

		candidate += L"/" + e;
	}

	//Unicodeコードポイント
	for(size_t i = 0; i < searchkey.size(); i++)
	{
		if((i + 1 < searchkey.size()) && IS_SURROGATE_PAIR(searchkey[i], searchkey[i + 1]))
		{
			ucp = SURROGATEPAIR_UCPMIN +
				((((UCSCHAR)searchkey[i] & SURROGATEPAIR_SEPMASK) << SURROGATEPAIR_SEPBIT) |
				((UCSCHAR)searchkey[i + 1] & SURROGATEPAIR_SEPMASK));
			i++;
		}
		else
		{
			ucp = (UCSCHAR)searchkey[i];
		}
		_snwprintf_s(b, _TRUNCATE, L"U+%04X", ucp);

		if(!u.empty())
		{
			u += L",";
		}
		u += b;
	}

	candidate += L"/" + u + L"/\n";

	return candidate;
}
