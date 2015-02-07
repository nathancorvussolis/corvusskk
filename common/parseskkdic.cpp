
#include "common.h"
#include "eucjis2004.h"
#include "parseskkdic.h"

LPCWSTR EntriesAri = L";; okuri-ari entries.\n";
LPCWSTR EntriesNasi = L";; okuri-nasi entries.\n";

int ReadSKKDicLine(FILE *fp, WCHAR bom, int &okuri, std::wstring &key,
	SKKDICCANDIDATES &c, SKKDICOKURIBLOCKS &o)
{
	CHAR buf[READBUFSIZE * sizeof(WCHAR)];
	std::string sbuf;
	WCHAR wbuf[READBUFSIZE];
	std::wstring wsbuf;
	size_t ds, is;
	void *rp;
	std::wstring s, fmt;
	std::wregex re;

	c.clear();
	o.clear();

	switch(bom)
	{
	case 0xFEFF:
		while((rp = fgetws(wbuf, _countof(wbuf), fp)) != NULL)
		{
			wsbuf += wbuf;

			if(!wsbuf.empty() && wsbuf.back() == L'\n')
			{
				break;
			}
		}
		break;
	default:
		while((rp = fgets(buf, _countof(buf), fp)) != NULL)
		{
			sbuf += buf;

			if(!sbuf.empty() && sbuf.back() == '\n')
			{
				break;
			}
		}
		break;
	}

	if(rp == NULL)
	{
		return -1;
	}

	switch(bom)
	{
	case 0xFEFF:
		break;
	default:
		ds = -1;
		if(!EucJis2004ToWideChar(sbuf.c_str(), NULL, NULL, &ds))
		{
			return 1;
		}
		wsbuf = eucjis2004_string_to_wstring(sbuf);
		break;
	}

	if(wsbuf.empty())
	{
		return 1;
	}

	if(wsbuf.compare(EntriesAri) == 0)
	{
		okuri = 1;
		return 1;
	}
	else if(wsbuf.compare(EntriesNasi) == 0)
	{
		okuri = 0;
		return 1;
	}
	else
	{
		if(L'\0' <= wsbuf.front() && wsbuf.front() <= L'\x20')
		{
			return 1;
		}
	}

	if(okuri == -1)
	{
		return 1;
	}

	s = wsbuf;
	re.assign(L"[\\x00-\\x19]");
	fmt.assign(L"");
	s = std::regex_replace(s, re, fmt);

	if(okuri == 1)
	{
		//送りありエントリのブロック
		ParseSKKDicOkuriBlock(s, o);

		//送りありエントリのブロックを除去
		re.assign(L"\\[[^\\[\\]]+?/[^\\[\\]]+?/\\]/");
		fmt.assign(L"");
		s = std::regex_replace(s, re, fmt);
	}

	is = s.find_first_of(L'\x20');
	if(is == std::wstring::npos)
	{
		return 1;
	}
	key = s.substr(0, is);

	is = s.find_first_of(L'/', is);
	if(is == std::wstring::npos)
	{
		return 1;
	}
	s = s.substr(is);

	ParseSKKDicCandiate(s, c);

	return 0;
}

void ParseSKKDicCandiate(const std::wstring &s, SKKDICCANDIDATES &c)
{
	size_t i, is, ie, ia;
	std::wstring candidate, annotation;

	i = 0;
	while(i < s.size())
	{
		is = s.find_first_of(L'/', i);
		ie = s.find_first_of(L'/', is + 1);
		if(ie == std::wstring::npos)
		{
			break;
		}

		candidate = s.substr(i + 1, ie - is - 1);
		i = ie;

		ia = candidate.find_first_of(L';');

		if(ia == std::wstring::npos)
		{
			annotation.clear();
		}
		else
		{
			annotation = candidate.substr(ia + 1);
			candidate = candidate.substr(0, ia);
		}

		if(!candidate.empty())
		{
			c.push_back(SKKDICCANDIDATE(candidate, annotation));
		}
	}
}

void ParseSKKDicOkuriBlock(const std::wstring &s, SKKDICOKURIBLOCKS &o)
{
	std::wstring so, okurik, okuric;
	std::wregex re, reb;
	std::wsmatch m;
	SKKDICCANDIDATES okurics;

	re.assign(L"\\[[^\\[\\]]+?/[^\\[\\]]+?/\\]/");
	reb.assign(L"\\[([^\\[\\]]+?)(/[^\\[\\]]+?/)\\]/");
	so = s;

	while(std::regex_search(so, m, re))
	{
		okurics.clear();

		okurik = std::regex_replace(m.str(), reb, std::wstring(L"$1"));
		okuric = std::regex_replace(m.str(), reb, std::wstring(L"$2"));

		ParseSKKDicCandiate(okuric, okurics);

		std::reverse(okurics.begin(), okurics.end());

		o.insert(o.begin(), SKKDICOKURIBLOCK(okurik, okurics));
		so = m.suffix();
	}
}

std::wstring ParseConcat(const std::wstring &s)
{
	std::wstring ret, numstr, tmpstr, fmt;
	std::wregex re;
	std::wsmatch res;
	wchar_t u;

	ret = s;

	tmpstr = s;
	re.assign(L"^\\(concat \".+?\"\\)$");
	if(std::regex_search(tmpstr, re))
	{
		ret.clear();
		fmt = L"$1";

		re.assign(L"\\(concat \"(.+)\"\\)");
		tmpstr = std::regex_replace(tmpstr, re, fmt);

		re.assign(L"\\\\([\\\"\\\\])");
		tmpstr = std::regex_replace(tmpstr, re, fmt);

		re.assign(L"\\\\[0-3][0-7]{2}");
		while(std::regex_search(tmpstr, res, re))
		{
			ret += res.prefix();
			numstr = res.str();
			numstr[0] = L'0';
			u = (wchar_t)wcstoul(numstr.c_str(), NULL, 0);
			if(u >= L'\x20' && u <= L'\x7E')
			{
				ret.append(1, u);
			}
			tmpstr = res.suffix();
		}
		ret += tmpstr;
	}

	return ret;
}

std::wstring MakeConcat(const std::wstring &s)
{
	std::wstring ret, fmt;
	std::wregex re;

	ret = s;

	// "/" -> \057, ";" -> \073
	re.assign(L"[/;]");
	if(std::regex_search(ret, re))
	{
		// "\"" -> "\\\"", "\\" -> "\\\\"
		re.assign(L"([\\\"\\\\])");
		fmt.assign(L"\\$1");
		ret = std::regex_replace(ret, re, fmt);

		re.assign(L"/");
		fmt.assign(L"\\057");
		ret = std::regex_replace(ret, re, fmt);

		re.assign(L";");
		fmt.assign(L"\\073");
		ret = std::regex_replace(ret, re, fmt);

		ret = L"(concat \"" + ret + L"\")";
	}

	return ret;
}
