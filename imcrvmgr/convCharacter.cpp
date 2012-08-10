
#include "corvussrv.h"
#include "eucjis2004.h"

void ConvUnicode(const std::wstring &text, CANDIDATES &candidates)
{
	//Unicode Code Point
	UCSCHAR ucp = 0;
	WCHAR utf16[3];

	// U+XXXXXX (XXXXXX : 0000-FFFF,10000-10FFFF)
	if(std::regex_match(text, std::wregex(L"U\\+([1-9A-F]|10)?[0-9A-F]{4}")))
	{
		if(swscanf_s(text.c_str(), L"U+%X", &ucp) != 1)
		{
			return;
		}
	}
	// uxxxxxx (xxxxxx : 0000-ffff,10000-10ffff)
	else if(std::regex_match(text, std::wregex(L"u([1-9a-f]|10)?[0-9a-f]{4}")))
	{
		if(swscanf_s(text.c_str(), L"u%x", &ucp) != 1)
		{
			return;
		}
	}
	else
	{
		return;
	}

	ZeroMemory(utf16, sizeof(utf16));
	if(UcpToWideChar(ucp, &utf16[0], &utf16[1]) != 0)
	{
		candidates.push_back(CANDIDATE(utf16, L""));
	}
}

void ConvJISX0213(const std::wstring &text, CANDIDATES &candidates)
{
	//JIS X 0213 面区点番号
	int men, ku, ten;
	size_t size;
	WCHAR utf16[8];
	CHAR euc[4];
	CONST CHAR mask = 0x7F;
	CONST CHAR base = 0x20;
	UCSCHAR ucp[2];
	WCHAR sucp[32];

	if(!std::regex_match(text, std::wregex(L"[12]-(0[1-9]|[1-8][0-9]|9[0-4])-(0[1-9]|[0-8][0-9]|9[0-4])")))
	{
		return;
	}

	if(swscanf_s(text.c_str(), L"%d-%d-%d", &men, &ku, &ten) != 3)
	{
		return;
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
		return;
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
		candidates.push_back(CANDIDATE(utf16, sucp));
	}
}
