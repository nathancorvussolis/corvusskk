
#include "common.h"
#include "eucjis2004.h"
#include "eucjp.h"
#include "parseskkdic.h"

LPCWSTR EntriesAri = L";; okuri-ari entries.\n";
LPCWSTR EntriesNasi = L";; okuri-nasi entries.\n";

int ReadSKKDicLine(FILE *fp, SKKDICENCODING encoding, int &okuri, std::wstring &key,
	SKKDICCANDIDATES &c, SKKDICOKURIBLOCKS &o)
{
	CHAR buf[READBUFSIZE];
	WCHAR wbuf[READBUFSIZE / sizeof(WCHAR)];
	std::string strbuf;
	std::wstring wstrbuf;

	c.clear();
	o.clear();

	switch (encoding)
	{
	case enc_utf_8:		//UTF-8
	case enc_utf_16:	//UTF-16LE
		while (fgetws(wbuf, _countof(wbuf), fp) != nullptr)
		{
			wstrbuf += wbuf;

			if (!wstrbuf.empty() && wstrbuf.back() == L'\n')
			{
				break;
			}
		}
		break;
	case enc_euc_jis_2004:		//EUC-JIS-2004
	case enc_euc_jp:		//EUC-JP
		while (fgets(buf, _countof(buf), fp) != nullptr)
		{
			strbuf += buf;

			if (!strbuf.empty() && strbuf.back() == '\n')
			{
				break;
			}
		}
		break;
	default:
		return -1;
	}

	if (ferror(fp) != 0)
	{
		return -1;
	}

	switch (encoding)
	{
	case enc_utf_8:		//UTF-8
	case enc_utf_16:	//UTF-16LE
		break;
	case enc_euc_jis_2004:		//EUC-JIS-2004
		wstrbuf = eucjis2004_string_to_wstring(strbuf);
		break;
	case enc_euc_jp:		//EUC-JP
		wstrbuf = eucjp_string_to_wstring(strbuf);
		break;
	default:
		break;
	}

	if (wstrbuf.empty())
	{
		return -1;
	}

	if (wstrbuf.compare(EntriesAri) == 0)
	{
		okuri = 1;
		return 1;
	}
	else if (wstrbuf.compare(EntriesNasi) == 0)
	{
		okuri = 0;
		return 1;
	}

	if (okuri == -1)
	{
		return 1;
	}

	std::wstring s = wstrbuf;

	static const std::wregex rectrl(L"[\\x00-\\x19]");
	s = std::regex_replace(s, rectrl, L"");

	if (okuri == 1)
	{
		//送りありエントリのブロック
		ParseSKKDicOkuriBlock(s, o);

		//送りありエントリのブロックを除去
		static const std::wregex reblock(L"\\[[^\\[\\]]+?/[^\\[\\]]+?/\\]/");
		s = std::regex_replace(s, reblock, L"");
	}

	size_t is = s.find(L"\x20/");
	if (is == std::wstring::npos)
	{
		return 1;
	}

	size_t ie = s.find_last_not_of(L'\x20', is);
	if (ie == std::wstring::npos)
	{
		return 1;
	}

	key = s.substr(0, ie + 1);

	s = s.substr(is + 1);

	ParseSKKDicCandiate(s, c);

	return 0;
}

void ParseSKKDicCandiate(const std::wstring &s, SKKDICCANDIDATES &c)
{
	size_t i, is, ie, ia;
	std::wstring candidate, annotation;

	i = 0;
	while (i < s.size())
	{
		is = s.find_first_of(L'/', i);
		ie = s.find_first_of(L'/', is + 1);
		if (ie == std::wstring::npos)
		{
			break;
		}

		candidate = s.substr(i + 1, ie - is - 1);
		i = ie;

		ia = candidate.find_first_of(L';');

		if (ia == std::wstring::npos)
		{
			annotation.clear();
		}
		else
		{
			annotation = candidate.substr(ia + 1);
			candidate = candidate.substr(0, ia);
		}

		if (!candidate.empty())
		{
			c.push_back(std::make_pair(candidate, annotation));
		}
	}
}

void ParseSKKDicOkuriBlock(const std::wstring &s, SKKDICOKURIBLOCKS &o)
{
	std::wstring so, okurik, okuric;
	std::wsmatch m;
	SKKDICCANDIDATES okurics;

	so = s;

	static const std::wregex reblock(L"\\[([^\\[\\]]+?)(/[^\\[\\]]+?/)\\]/");

	while (std::regex_search(so, m, reblock))
	{
		okurics.clear();

		okurik = std::regex_replace(m.str(), reblock, L"$1");
		okuric = std::regex_replace(m.str(), reblock, L"$2");

		ParseSKKDicCandiate(okuric, okurics);

		std::reverse(okurics.begin(), okurics.end());

		o.insert(o.begin(), std::make_pair(okurik, okurics));

		so = m.suffix().str();
	}
}

std::wstring ParseConcat(const std::wstring &s)
{
	std::wstring ret = s;
	LPCWSTR bsrep = L"\uF05C";

	// (concat "*")
	static const std::wregex reconcat(L"^\\(\\s*concat\\s+\"(.+)\"\\s*\\)$");

	if (std::regex_search(ret, reconcat))
	{
		std::wstring fmt, numstr, numtmpstr;
		std::wregex re;
		std::wsmatch res;

		fmt.assign(L"$1");
		ret = std::regex_replace(ret, reconcat, fmt);

		re.assign(L"\"\\s+\"");
		fmt.assign(L"");
		ret = std::regex_replace(ret, re, fmt);

		//バックスラッシュ
		re.assign(L"\\\\\\\\");
		fmt.assign(bsrep);
		ret = std::regex_replace(ret, re, fmt);

		//二重引用符
		re.assign(L"\\\\\\\"");
		fmt.assign(L"\\\"");
		ret = std::regex_replace(ret, re, fmt);

		//空白文字
		re.assign(L"\\\\s");
		fmt.assign(L"\x20");
		ret = std::regex_replace(ret, re, fmt);

		//制御文字など
		re.assign(L"\\\\[abtnvfred ]");
		fmt.assign(L"");
		ret = std::regex_replace(ret, re, fmt);

		//8進数表記の文字
		re.assign(L"\\\\[0-3][0-7]{2}");
		while (std::regex_search(ret, res, re))
		{
			numstr += res.prefix();
			numtmpstr = res.str();
			numtmpstr[0] = L'0';
			wchar_t u = (wchar_t)wcstoul(numtmpstr.c_str(), nullptr, 0);
			if (u >= L'\x20' && u <= L'\x7E')
			{
				numstr.append(1, u);
			}
			ret = res.suffix().str();
		}
		ret = numstr + ret;

		//意味なしエスケープ
		re.assign(L"\\\\");
		fmt.assign(L"");
		ret = std::regex_replace(ret, re, fmt);

		//バックスラッシュ
		re.assign(bsrep);
		fmt.assign(L"\\");
		ret = std::regex_replace(ret, re, fmt);
	}

	return ret;
}

std::wstring MakeConcat(const std::wstring &s)
{
	std::wstring ret = s;

	// '/' or ';'
	static const std::wregex respcch(L"[/;]");

	if (std::regex_search(ret, respcch))
	{
		std::wstring fmt;
		std::wregex re;

		// '"' -> "\"", '\' -> "\\"
		re.assign(L"([\"\\\\])");
		fmt.assign(L"\\$1");
		ret = std::regex_replace(ret, re, fmt);

		// '/' -> "\057"
		re.assign(L"/");
		fmt.assign(L"\\057");
		ret = std::regex_replace(ret, re, fmt);

		// ';' -> "\073"
		re.assign(L";");
		fmt.assign(L"\\073");
		ret = std::regex_replace(ret, re, fmt);

		ret = L"(concat \"" + ret + L"\")";
	}

	return ret;
}

std::wstring EscapeGadgetString(const std::wstring &s)
{
	std::wstring ret = s;
	LPCWSTR bsrep = L"\uF05C";

	//実行変換もどきの文字列パラメータをエスケープ
	static const std::wregex regadget(L"^\\(.+\\)$");

	if (std::regex_match(s, regadget))
	{
		std::wstring esc, str;
		std::wstring tmp = s;
		std::wsmatch m;

		std::wstring fmt;
		std::wregex re;

		// "\\" -> '\uF05C'
		re.assign(L"\\\\\\\\");
		fmt.assign(bsrep);
		tmp = std::regex_replace(tmp, re, fmt);

		// <SPC>"..."   ignoring escaped quotation
		static const std::wregex restring(L" (\"\"|\".*?[^\\\\]\")");

		while (std::regex_search(tmp, m, restring))
		{
			esc += m.prefix().str();
			str = m.str();
			tmp = m.suffix().str();

			// '/' -> "\057"
			re.assign(L"/");
			fmt.assign(L"\\057");
			str = std::regex_replace(str, re, fmt);

			// ';' -> "\073"
			re.assign(L";");
			fmt.assign(L"\\073");
			str = std::regex_replace(str, re, fmt);

			esc += str;
		}

		esc += tmp;

		// '\uF05C' -> "\\"
		re.assign(bsrep);
		fmt.assign(L"\\\\");
		esc = std::regex_replace(esc, re, fmt);

		ret = esc;
	}

	return ret;
}
