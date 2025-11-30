
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

	s = std::regex_replace(s, RegExp(L"[\\x00-\\x19]"), L"");

	if (okuri == 1)
	{
		//送りありエントリのブロック
		ParseSKKDicOkuriBlock(s, o);

		//送りありエントリのブロックを除去
		s = std::regex_replace(s, RegExp(L"\\[[^\\[\\]]+?/[^\\[\\]]+?/\\]/"), L"");
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

	if (s.find_last_of(L'\x20', ie) != std::string::npos)
	{
		return 1;
	}

	key = s.substr(0, ie + 1);

	s = s.substr(is + 1);

	ParseSKKDicCandiate(s, c);

	return 0;
}

int ReadSKKDicLine(FILE *fp, int &okuri, std::wstring &key)
{
	WCHAR wbuf[READBUFSIZE / sizeof(WCHAR)];
	std::wstring wstrbuf;

	while (fgetws(wbuf, _countof(wbuf), fp) != nullptr)
	{
		wstrbuf += wbuf;

		if (!wstrbuf.empty() && wstrbuf.back() == L'\n')
		{
			break;
		}
	}

	if (ferror(fp) != 0)
	{
		return -1;
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

	if (s.find_last_of(L'\x20', ie) != std::string::npos)
	{
		return 1;
	}

	key = s.substr(0, ie + 1);

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

	const std::wregex &reblock = RegExp(L"\\[([^\\[\\]]+?)(/[^\\[\\]]+?/)\\]/");

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
	const std::wregex &reconcat = RegExp(L"^\\(\\s*concat\\s+\"(.+)\"\\s*\\)$");

	if (std::regex_search(ret, reconcat))
	{
		std::wstring numstr, numtmpstr;
		std::wsmatch res;

		ret = std::regex_replace(ret, reconcat, L"$1");

		//concat
		ret = std::regex_replace(ret, RegExp(L"\"\\s+\""), L"");

		//バックスラッシュ
		ret = std::regex_replace(ret, RegExp(L"\\\\\\\\"), bsrep);

		//二重引用符
		ret = std::regex_replace(ret, RegExp(L"\\\\\\\""), L"\\\"");

		//空白文字
		ret = std::regex_replace(ret, RegExp(L"\\\\s"), L"\x20");

		//制御文字など
		ret = std::regex_replace(ret, RegExp(L"\\\\[abtnvfred ]"), L"");

		//8進数表記の文字
		while (std::regex_search(ret, res, RegExp(L"\\\\[0-3][0-7]{2}")))
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
		ret = std::regex_replace(ret, RegExp(L"\\\\"), L"");

		//バックスラッシュ
		ret = std::regex_replace(ret, RegExp(bsrep), L"\\");
	}

	return ret;
}

std::wstring MakeConcat(const std::wstring &s)
{
	std::wstring ret = s;

	// '/' or ';'
	if (std::regex_search(ret, RegExp(L"[/;]")))
	{
		// '"' -> "\"", '\' -> "\\"
		ret = std::regex_replace(ret, RegExp(L"([\"\\\\])"), L"\\$1");

		// '/' -> "\057"
		ret = std::regex_replace(ret, RegExp(L"/"), L"\\057");

		// ';' -> "\073"
		ret = std::regex_replace(ret, RegExp(L";"), L"\\073");

		ret = L"(concat \"" + ret + L"\")";
	}

	return ret;
}

std::wstring EscapeGadgetString(const std::wstring &s)
{
	std::wstring ret = s;
	LPCWSTR bsrep = L"\uF05C";

	//実行変換もどきの文字列パラメータをエスケープ
	if (std::regex_match(s, RegExp(L"^\\(.+\\)$")))
	{
		std::wstring esc, str;
		std::wstring tmp = s;
		std::wsmatch m;

		// "\\" -> '\uF05C'
		tmp = std::regex_replace(tmp, RegExp(L"\\\\\\\\"), bsrep);

		// <SPC>"..."   ignoring escaped quotation
		while (std::regex_search(tmp, m, RegExp(L" (\"\"|\".*?[^\\\\]\")")))
		{
			esc += m.prefix().str();
			str = m.str();
			tmp = m.suffix().str();

			// '/' -> "\057"
			str = std::regex_replace(str, RegExp(L"/"), L"\\057");

			// ';' -> "\073"
			str = std::regex_replace(str, RegExp(L";"), L"\\073");

			esc += str;
		}

		esc += tmp;

		// '\uF05C' -> "\\"
		esc = std::regex_replace(esc, RegExp(bsrep), L"\\\\");

		ret = esc;
	}

	return ret;
}
