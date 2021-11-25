
#include "utf8.h"
#include "imcrvmgr.h"

//エントリの行頭位置
typedef std::vector<long> POS;
POS skkdicpos_a; //送りありエントリ
POS skkdicpos_n; //送りなしエントリ

void SearchDictionary(const std::wstring &searchkey, const std::wstring &okuri, SKKDICCANDIDATES &sc)
{
	std::wstring candidate;

	if (lua != nullptr)
	{
		lua_getglobal(lua, u8"lua_skk_search");
		lua_pushstring(lua, WCTOU8(searchkey));
		lua_pushstring(lua, WCTOU8(okuri));

		if (lua_pcall(lua, 2, 1, 0) == LUA_OK)
		{
			if (lua_isstring(lua, -1))
			{
				candidate = U8TOWC(lua_tostring(lua, -1));
			}
			lua_pop(lua, 1);
		}
	}
	else
	{
		std::wstring okurik = okuri;

		//skk-search-sagyo-henkaku (anything)
		//「送りあり変換で送りなし候補も検索する」 → 送り仮名あり、送りローマ字なし
		static const std::wregex reroman(L"^.+[a-z]$");
		if (!okurik.empty() && !std::regex_match(searchkey, reroman))
		{
			//送り仮名クリア
			okurik.clear();
		}

		//ユーザー辞書
		candidate += SearchUserDic(searchkey, okurik);

		//SKK辞書
		candidate += SearchSKKDic(searchkey, okurik);

		//SKK辞書サーバー
		candidate += SearchSKKServer(searchkey);

		//Unicodeコードポイント
		candidate += SearchUnicode(searchkey);

		//JIS X 0213 面区点番号
		candidate += SearchJISX0213(searchkey);

		//JIS X 0218 区点番号
		candidate += SearchJISX0208(searchkey);

		if (searchkey.size() > 1 && searchkey[0] == L'?')
		{
			//文字コード表記変換 (ASCII, JIS X 0201(8bit), JIS X 0213 / Unicode)
			candidate += SearchCharacterCode(searchkey.substr(1));
		}

		static const std::wregex resepdic(L"/\n/");
		candidate = std::regex_replace(candidate, resepdic, L"/");
	}

	static const std::wregex rectrl(L"[\\x00-\\x19]");
	candidate = std::regex_replace(candidate, rectrl, L"");

	ParseSKKDicCandiate(candidate, sc);

	//重複候補を削除
	if (sc.size() > 1)
	{
		FORWARD_ITERATION_I(sc_itrf, sc)
		{
			for (auto sc_itrb = sc_itrf + 1; sc_itrb != sc.end(); )
			{
				if (sc_itrf->first == sc_itrb->first)
				{
					sc_itrb = sc.erase(sc_itrb);
				}
				else
				{
					++sc_itrb;
				}
			}
		}
	}
}

std::wstring SearchSKKDic(const std::wstring &searchkey, const std::wstring &okuri)
{
	FILE *fp = nullptr;
	std::wstring candidate, wstrbuf, kbuf, cbuf;
	WCHAR wbuf[READBUFSIZE / sizeof(WCHAR)];
	long pos, left, mid, right;

	_wfopen_s(&fp, pathskkdic, modeRB);
	if (fp == nullptr)
	{
		return candidate;
	}

	left = 0;
	if (okuri.empty())
	{
		right = (long)skkdicpos_n.size() - 1;
	}
	else
	{
		right = (long)skkdicpos_a.size() - 1;
	}

	while (left <= right)
	{
		mid = left + (right - left) / 2;
		if (okuri.empty())
		{
			pos = skkdicpos_n[mid];
		}
		else
		{
			pos = skkdicpos_a[mid];
		}
		fseek(fp, pos, SEEK_SET);

		wstrbuf.clear();
		kbuf.clear();
		cbuf.clear();

		while (fgetws(wbuf, _countof(wbuf), fp) != nullptr)
		{
			wstrbuf += wbuf;

			if (!wstrbuf.empty() && wstrbuf.back() == L'\n')
			{
				// CR+LF -> LF
				if (wstrbuf.size() >= 2 && wstrbuf[wstrbuf.size() - 2] == L'\r')
				{
					wstrbuf.erase(wstrbuf.size() - 2);
					wstrbuf.push_back(L'\n');
				}
				break;
			}
		}

		if (ferror(fp) != 0)
		{
			break;
		}

		if (wstrbuf.empty())
		{
			break;
		}

		size_t is = wstrbuf.find(L"\x20/");
		if (is != std::wstring::npos)
		{
			size_t ie = wstrbuf.find_last_not_of(L'\x20', is);
			if (ie != std::wstring::npos)
			{
				kbuf = wstrbuf.substr(0, ie + 1);
			}
			cbuf = wstrbuf.substr(is + 1);
		}

		int cmpkey = searchkey.compare(kbuf);
		if (cmpkey == 0)
		{
			candidate = cbuf;
			break;
		}
		else if (cmpkey > 0)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}

	fclose(fp);

	return candidate;
}

void MakeSKKDicPos()
{
	FILE *fp = nullptr;
	WCHAR wbuf[READBUFSIZE / sizeof(WCHAR)];
	std::wstring wstrbuf;
	long pos;
	int okuri = -1;

	skkdicpos_a.clear();
	skkdicpos_a.shrink_to_fit();
	skkdicpos_n.clear();
	skkdicpos_n.shrink_to_fit();

	_wfopen_s(&fp, pathskkdic, modeRB);
	if (fp == nullptr)
	{
		return;
	}

	//check BOM
	WCHAR bom = L'\0';
	fread(&bom, 2, 1, fp);
	if (bom != BOM)
	{
		fclose(fp);
		return;
	}

	pos = ftell(fp);

	while (true)
	{
		wstrbuf.clear();

		while (fgetws(wbuf, _countof(wbuf), fp) != nullptr)
		{
			wstrbuf += wbuf;

			if (!wstrbuf.empty() && wstrbuf.back() == L'\n')
			{
				// CR+LF -> LF
				if (wstrbuf.size() >= 2 && wstrbuf[wstrbuf.size() - 2] == L'\r')
				{
					wstrbuf.erase(wstrbuf.size() - 2);
					wstrbuf.push_back(L'\n');
				}
				break;
			}
		}

		if (ferror(fp) != 0)
		{
			break;
		}

		if (wstrbuf.empty())
		{
			break;
		}

		if (wstrbuf.compare(EntriesAri) == 0)
		{
			okuri = 1;
		}
		else if (wstrbuf.compare(EntriesNasi) == 0)
		{
			okuri = 0;
		}
		else
		{
			switch (okuri)
			{
			case 1:
				skkdicpos_a.push_back(pos);
				break;
			case 0:
				skkdicpos_n.push_back(pos);
				break;
			default:
				break;
			}
		}

		pos = ftell(fp);
	}

	fclose(fp);

	std::reverse(skkdicpos_a.begin(), skkdicpos_a.end());
}

std::wstring ConvertKey(const std::wstring &searchkey, const std::wstring &okuri)
{
	std::wstring ret;

	if (lua != nullptr)
	{
		lua_getglobal(lua, u8"lua_skk_convert_key");
		lua_pushstring(lua, WCTOU8(searchkey));
		lua_pushstring(lua, WCTOU8(okuri));

		if (lua_pcall(lua, 2, 1, 0) == LUA_OK)
		{
			if (lua_isstring(lua, -1))
			{
				ret = U8TOWC(lua_tostring(lua, -1));
			}
			lua_pop(lua, 1);
		}
	}
	else
	{
		//文字コード表記変換のとき見出し語変換しない
		if (searchkey.size() > 1 && searchkey[0] == L'?')
		{
			return std::wstring(L"");
		}

		//数値変換
		static const std::wregex renum(L"[0-9]+");
		ret = std::regex_replace(searchkey, renum, L"#");
	}

	static const std::wregex rectrl(L"[\\x00-\\x19]");
	ret = std::regex_replace(ret, rectrl, L"");

	return ret;
}

std::wstring ConvertCandidate(const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &okuri)
{
	std::wstring ret;

	if (lua != nullptr)
	{
		lua_getglobal(lua, u8"lua_skk_convert_candidate");
		lua_pushstring(lua, WCTOU8(searchkey));
		lua_pushstring(lua, WCTOU8(candidate));
		lua_pushstring(lua, WCTOU8(okuri));

		if (lua_pcall(lua, 3, 1, 0) == LUA_OK)
		{
			if (lua_isstring(lua, -1))
			{
				ret = U8TOWC(lua_tostring(lua, -1));
			}
			lua_pop(lua, 1);
		}
	}
	else
	{
		//concatのみ
		ret = ParseConcat(candidate);
	}

	static const std::wregex rectrl(L"[\\x00-\\x19]");
	ret = std::regex_replace(ret, rectrl, L"");

	return ret;
}
