
#include "common.h"
#include "eucjis2004.h"
#include "parseskkdic.h"

LPCWSTR EntriesAri = L";; okuri-ari entries.\n";
LPCWSTR EntriesNasi = L";; okuri-nasi entries.\n";

int ReadSKKDicLine(FILE *fp, WCHAR bom, int &okuri, std::wstring &key,
	SKKDICCANDIDATES &c, SKKDICOKURIBLOCKS &o)
{
	CHAR buf[DICBUFSIZE*2];
	WCHAR wbuf[DICBUFSIZE];
	size_t size, is;
	void *rp;
	std::wstring s;
	std::wregex re;
	std::wstring fmt;
	std::wsmatch m;
	std::wregex reb;
	std::wstring so;
	std::wstring okurik;
	std::wstring okuric;

	c.clear();
	o.clear();

	switch(bom)
	{
	case 0xFEFF:
		rp = fgetws(wbuf, _countof(wbuf), fp);
		break;
	default:
		rp = fgets(buf, _countof(buf), fp);
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
		size = _countof(wbuf);
		if(!EucJis2004ToWideChar(buf, NULL, wbuf, &size))
		{
			return 1;
		}
		break;
	}

	if(wcscmp(wbuf, EntriesAri) == 0)
	{
		okuri = 1;
		return 1;
	}
	else if(wcscmp(wbuf, EntriesNasi) == 0)
	{
		okuri = 0;
		return 1;
	}
	else
	{
		if(L'\0' <= wbuf[0] && wbuf[0] <= L'\x20')
		{
			return 1;
		}
	}

	if(okuri == -1)
	{
		return 1;
	}

	s.assign(wbuf);
	re.assign(L"[\\x00-\\x19]");
	fmt.assign(L"");
	s = std::regex_replace(s, re, fmt);

	if(okuri == 1)
	{
		//送りありエントリのブロック形式
		ParseSKKDicOkuriBlock(s, o);

		//送りありエントリのブロック形式を除去
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
	std::wstring candidate;
	std::wstring annotation;

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

		c.push_back(SKKDICCANDIDATE(candidate, annotation));
	}
}

void ParseSKKDicOkuriBlock(const std::wstring &s, SKKDICOKURIBLOCKS &o)
{
	std::wregex re, reb;
	std::wstring so, okurik, okuric;
	std::wsmatch m;
	SKKDICCANDIDATES okuricc;
	SKKDICCANDIDATES okuriccr;
	SKKDICCANDIDATES::reverse_iterator c_ritr;

	re.assign(L"\\[[^\\[\\]]+?/[^\\[\\]]+?/\\]/");
	reb.assign(L"\\[([^\\[\\]]+?)(/[^\\[\\]]+?/)\\]/");
	so = s;

	while(std::regex_search(so, m, re))
	{
		okuricc.clear();
		okuriccr.clear();

		okurik = std::regex_replace(m.str(), reb, std::wstring(L"$1"));
		okuric = std::regex_replace(m.str(), reb, std::wstring(L"$2"));

		ParseSKKDicCandiate(okuric, okuricc);

		for(c_ritr = okuricc.rbegin(); c_ritr != okuricc.rend(); c_ritr ++)
		{
			okuriccr.push_back(*c_ritr);
		}

		o.insert(o.begin(), SKKDICOKURIBLOCK(okurik, okuriccr));
		so = m.suffix();
	}
}
