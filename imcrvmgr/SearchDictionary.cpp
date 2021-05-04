
#include "utf8.h"
#include "imcrvmgr.h"

//エントリの行頭位置
typedef std::vector<long> POS;
POS skkdicpos_a; //送りありエントリ
POS skkdicpos_n; //送りなしエントリ

void SearchDictionary(const std::wstring &searchkey, const std::wstring &okuri, SKKDICCANDIDATES &sc)
{
	std::wstring candidate;
	std::wregex re;
	std::wstring fmt;

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
		//ユーザー辞書
		candidate += SearchUserDic(searchkey, okuri);

		//SKK辞書
		candidate += SearchSKKDic(searchkey, okuri);

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

		re.assign(L"/\n/");
		fmt.assign(L"/");
		candidate = std::regex_replace(candidate, re, fmt);
	}

	re.assign(L"[\\x00-\\x19]");
	fmt.assign(L"");
	candidate = std::regex_replace(candidate, re, fmt);

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
		ret = std::regex_replace(searchkey, std::wregex(L"[0-9]+"), std::wstring(L"#"));
	}

	ret = std::regex_replace(ret, std::wregex(L"[\\x00-\\x19]"), std::wstring(L""));

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

	ret = std::regex_replace(ret, std::wregex(L"[\\x00-\\x19]"), std::wstring(L""));

	return ret;
}

int lua_search_skk_dictionary(lua_State *lua)
{
	std::wstring candidate;

	if (lua_isstring(lua, 1) &&
		lua_isstring(lua, 2))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
		std::wstring okurikey = U8TOWC(lua_tostring(lua, 2));

		candidate = SearchSKKDic(searchkey, okurikey);
	}

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_user_dictionary(lua_State *lua)
{
	std::wstring candidate;

	if (lua_isstring(lua, 1) &&
		lua_isstring(lua, 2))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
		std::wstring okurikey = U8TOWC(lua_tostring(lua, 2));

		candidate = SearchUserDic(searchkey, okurikey);
	}

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_skk_server(lua_State *lua)
{
	std::wstring candidate;

	if (lua_isstring(lua, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));

		candidate = SearchSKKServer(searchkey);
	}

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_skk_server_info(lua_State *lua)
{
	std::wstring server_ver = GetSKKServerInfo(SKK_VER);
	std::wstring server_hst = GetSKKServerInfo(SKK_HST);

	lua_pushstring(lua, WCTOU8(server_ver));
	lua_pushstring(lua, WCTOU8(server_hst));

	return 2;
}

int lua_search_unicode(lua_State *lua)
{
	std::wstring candidate;

	if (lua_isstring(lua, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));

		candidate = SearchUnicode(searchkey);
	}

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_jisx0213(lua_State *lua)
{
	std::wstring candidate;

	if (lua_isstring(lua, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));

		candidate = SearchJISX0213(searchkey);
	}

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_jisx0208(lua_State *lua)
{
	std::wstring candidate;

	if (lua_isstring(lua, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));

		candidate = SearchJISX0208(searchkey);
	}

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_character_code(lua_State *lua)
{
	std::wstring candidate;

	if (lua_isstring(lua, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));

		candidate = SearchCharacterCode(searchkey);
	}

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_complement(lua_State *lua)
{
	std::wstring candidate;
	SKKDICCANDIDATES sc;

	if (lua_isstring(lua, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));

		SearchComplement(searchkey, sc);

		FORWARD_ITERATION_I(sc_itr, sc)
		{
			candidate += L"/" + MakeConcat(sc_itr->first);
		}
		if (!candidate.empty())
		{
			candidate += L"/\n";
		}
	}

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_add(lua_State *lua)
{
	if (lua_isboolean(lua, 1) &&
		lua_isstring(lua, 2) &&
		lua_isstring(lua, 3) &&
		lua_isstring(lua, 4) &&
		lua_isstring(lua, 5))
	{
		int okuriari = lua_toboolean(lua, 1);
		WCHAR command = (okuriari ? REQ_USER_ADD_A : REQ_USER_ADD_N);
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 2));
		std::wstring candidate = U8TOWC(lua_tostring(lua, 3));
		std::wstring annotation = U8TOWC(lua_tostring(lua, 4));
		std::wstring okuri = U8TOWC(lua_tostring(lua, 5));

		AddUserDic(command, searchkey, candidate, annotation, okuri);
	}

	return 0;
}

int lua_delete(lua_State *lua)
{
	if (lua_isboolean(lua, 1) &&
		lua_isstring(lua, 2) &&
		lua_isstring(lua, 3))
	{
		int okuriari = lua_toboolean(lua, 1);
		WCHAR command = (okuriari ? REQ_USER_DEL_A : REQ_USER_DEL_N);
		std::wstring searchkey = U8TOWC(lua_tostring(lua, 2));
		std::wstring candidate = U8TOWC(lua_tostring(lua, 3));

		DelUserDic(command, searchkey, candidate);
	}

	return 0;
}

int lua_save(lua_State *lua)
{
	StartSaveUserDic(TRUE);

	return 0;
}
