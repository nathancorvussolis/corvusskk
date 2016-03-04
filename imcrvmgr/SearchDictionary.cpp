﻿
#include "parseskkdic.h"
#include "configxml.h"
#include "utf8.h"
#include "imcrvmgr.h"

typedef std::map<std::wstring, long> MAP;
typedef std::vector<long> POS;
POS skkdicpos;

void SearchDictionary(const std::wstring &searchkey, const std::wstring &okuri, SKKDICCANDIDATES &sc)
{
    std::wstring candidate;
	std::wregex re;
	std::wstring fmt;

	if(lua != NULL)
	{
		lua_getglobal(lua,"lua_skk_search");
		lua_pushstring(lua, WCTOU8(searchkey));
		lua_pushstring(lua, WCTOU8(okuri));

		if(lua_pcall(lua, 2, 1, 0) == LUA_OK)
		{
			if(lua_isstring(lua, -1))
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
		candidate += SearchSKKDic(searchkey);

		//SKK辞書サーバー
		candidate += SearchSKKServer(searchkey);

		//Unicodeコードポイント
		candidate += SearchUnicode(searchkey);

		//JIS X 0213 面区点番号
		candidate += SearchJISX0213(searchkey);

		//JIS X 0218 区点番号
		candidate += SearchJISX0208(searchkey);

		if(searchkey.size() > 1 && searchkey[0] == L'?')
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
	if(sc.size() > 1)
	{
		FORWARD_ITERATION_I(sc_itrf, sc)
		{
			for(auto sc_itrb = sc_itrf + 1; sc_itrb != sc.end(); )
			{
				if(sc_itrf->first == sc_itrb->first)
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

std::wstring SearchSKKDic(const std::wstring &searchkey)
{
	FILE *fp;
	std::wstring key, candidate, wsbuf, kbuf, cbuf;
	WCHAR wbuf[READBUFSIZE];
	PWCHAR pwb = NULL;
	long pos, left, mid, right;

	_wfopen_s(&fp, pathskkdic, RB);
	if(fp == NULL)
	{
		return candidate;
	}

	left = 0;
	right = (long)skkdicpos.size() - 1;

	while(left <= right)
	{
		mid = left + (right - left) / 2;
		pos = skkdicpos[mid];
		fseek(fp, pos, SEEK_SET);

		wsbuf.clear();
		kbuf.clear();
		cbuf.clear();

		while((pwb = fgetws(wbuf, _countof(wbuf), fp)) != NULL)
		{
			wsbuf += wbuf;

			if(!wsbuf.empty() && wsbuf.back() == L'\n')
			{
				break;
			}
		}

		if(pwb == NULL)
		{
			break;
		}

		size_t ridx = wsbuf.find(L"\r\n");
		if(ridx != std::wstring::npos && ridx <= wsbuf.size())
		{
			wsbuf.erase(ridx);
			wsbuf.push_back(L'\n');
		}

		size_t cidx = wsbuf.find(L"\x20/");
		if(cidx != std::wstring::npos && cidx < wsbuf.size())
		{
			kbuf = wsbuf.substr(0, cidx);
			cbuf = wsbuf.substr(cidx + 1);
		}

		int cmpkey = searchkey.compare(kbuf);
		if(cmpkey == 0)
		{
			candidate = cbuf;
			break;
		}
		else if(cmpkey > 0)
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

void LoadSKKDic()
{
	FILE *fp;
	std::wstring wsbuf, key;
	WCHAR wbuf[READBUFSIZE];
	PWCHAR pwb = NULL;
	long pos;
	int okuri = -1;
	MAP map;

	skkdicpos.clear();
	skkdicpos.shrink_to_fit();

	_wfopen_s(&fp, pathskkdic, RB);
	if(fp == NULL)
	{
		return;
	}

	fseek(fp, 2, SEEK_SET); //BOM
	pos = ftell(fp);

	while(true)
	{
		wsbuf.clear();

		while((pwb = fgetws(wbuf, _countof(wbuf), fp)) != NULL)
		{
			wsbuf += wbuf;

			if(!wsbuf.empty() && wsbuf.back() == L'\n')
			{
				break;
			}
		}

		if(pwb == NULL)
		{
			break;
		}

		size_t ridx = wsbuf.find(L"\r\n");
		if(ridx != std::wstring::npos && ridx <= wsbuf.size())
		{
			wsbuf.erase(ridx);
			wsbuf.push_back(L'\n');
		}

		size_t cidx = wsbuf.find(L"\x20/");
		if(cidx != std::wstring::npos && cidx <= wsbuf.size())
		{
			key = wsbuf.substr(0, cidx);
		}

		if(wsbuf.compare(EntriesAri) == 0)
		{
			okuri = 1;
		}
		else if(wsbuf.compare(EntriesNasi) == 0)
		{
			okuri = 0;
		}
		else if(okuri != -1)
		{
			map.insert(MAP::value_type(key, pos));
		}

		pos = ftell(fp);
	}

	fclose(fp);

	FORWARD_ITERATION_I(map_itr, map)
	{
		skkdicpos.push_back(map_itr->second);
	}
}

std::wstring ConvertKey(const std::wstring &searchkey, const std::wstring &okuri)
{
	std::wstring ret;
	std::wregex re;
	std::wstring fmt;

	if(lua != NULL)
	{
		lua_getglobal(lua, "lua_skk_convert_key");
		lua_pushstring(lua, WCTOU8(searchkey));
		lua_pushstring(lua, WCTOU8(okuri));

		if(lua_pcall(lua, 2, 1, 0) == LUA_OK)
		{
			if(lua_isstring(lua, -1))
			{
				ret = U8TOWC(lua_tostring(lua, -1));
			}
			lua_pop(lua, 1);
		}
	}
	else
	{
		//文字コード表記変換のとき見出し語変換しない
		if(searchkey.size() > 1 && searchkey[0] == L'?')
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

	if(lua != NULL)
	{
		lua_getglobal(lua, "lua_skk_convert_candidate");
		lua_pushstring(lua, WCTOU8(searchkey));
		lua_pushstring(lua, WCTOU8(candidate));
		lua_pushstring(lua, WCTOU8(okuri));

		if(lua_pcall(lua, 3, 1, 0) == LUA_OK)
		{
			if(lua_isstring(lua, -1))
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
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchSKKDic(searchkey);

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_user_dictionary(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring okurikey = U8TOWC(lua_tostring(lua, 2));
	std::wstring candidate = SearchUserDic(searchkey, okurikey);

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_skk_server(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchSKKServer(searchkey);

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
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchUnicode(searchkey);

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_jisx0213(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchJISX0213(searchkey);

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_jisx0208(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchJISX0208(searchkey);

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_search_character_code(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchCharacterCode(searchkey);

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_complement(lua_State *lua)
{
	std::wstring candidate;
	SKKDICCANDIDATES sc;
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));

	SearchComplement(searchkey, sc);

	FORWARD_ITERATION_I(sc_itr, sc)
	{
		candidate += L"/" + MakeConcat(sc_itr->first);
	}
	if(!candidate.empty())
	{
		candidate += L"/\n";
	}

	lua_pushstring(lua, WCTOU8(candidate));

	return 1;
}

int lua_add(lua_State *lua)
{
	int okuriari = lua_toboolean(lua, 1);
	WCHAR command = (okuriari ? REQ_USER_ADD_0 : REQ_USER_ADD_1);
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 2));
	std::wstring candidate = U8TOWC(lua_tostring(lua, 3));
	std::wstring annotation = U8TOWC(lua_tostring(lua, 4));
	std::wstring okuri = U8TOWC(lua_tostring(lua, 5));

	AddUserDic(command, searchkey, candidate, annotation, okuri);

	return 0;
}

int lua_delete(lua_State *lua)
{
	int okuriari = lua_toboolean(lua, 1);
	WCHAR command = (okuriari ? REQ_USER_DEL_0 : REQ_USER_DEL_1);
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 2));
	std::wstring candidate = U8TOWC(lua_tostring(lua, 3));

	DelUserDic(command, searchkey, candidate);

	return 0;
}

int lua_save(lua_State *lua)
{
	StartSaveSKKUserDic(TRUE);

	return 0;
}
