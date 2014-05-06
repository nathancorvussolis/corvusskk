
#include "configxml.h"
#include "parseskkdic.h"
#include "utf8.h"
#include "imcrvmgr.h"

void SearchDictionary(const std::wstring &searchkey, const std::wstring &okuri, SKKDICCANDIDATES &sc)
{
    std::wstring candidate;
	SKKDICCANDIDATES::iterator sc_itrf;
	SKKDICCANDIDATES::iterator sc_itrb;
	std::wregex re;
	std::wstring fmt;

	if(lua != NULL)
	{
		lua_getglobal(lua,"lua_skk_search");
		lua_pushstring(lua, WCTOU8(searchkey.c_str()));
		lua_pushstring(lua, WCTOU8(okuri.c_str()));

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
		for(sc_itrf = sc.begin(); sc_itrf != sc.end(); sc_itrf++)
		{
			for(sc_itrb = sc_itrf + 1; sc_itrb != sc.end(); )
			{
				if(sc_itrf->first == sc_itrb->first)
				{
					sc_itrb = sc.erase(sc_itrb);
				}
				else
				{
					sc_itrb++;
				}
			}
		}
	}
}

std::wstring SearchSKKDic(const std::wstring &searchkey)
{
	std::wstring candidate;
	FILE *fpdic, *fpidx;
	std::wstring key;
	long pos, left, mid, right;
	int comp;
	WCHAR wbuf[DICBUFSIZE];
	WCHAR *p;

	_wfopen_s(&fpidx, pathskkidx, RB);
	if(fpidx == NULL)
	{
		return candidate;
	}
	_wfopen_s(&fpdic, pathskkdic, RB);
	if(fpdic == NULL)
	{
		fclose(fpidx);
		return candidate;
	}

	key = searchkey + L"\x20";

	left = 0;
	fseek(fpidx, 0, SEEK_END);
	right = (ftell(fpidx) / sizeof(pos)) - 1;

	while(left <= right)
	{
		mid = (left + right) / 2;
		fseek(fpidx, mid * sizeof(pos), SEEK_SET);
		fread(&pos, sizeof(pos), 1, fpidx);

		fseek(fpdic, pos, SEEK_SET);
		memset(wbuf, 0, sizeof(wbuf));
		fgetws(wbuf, _countof(wbuf), fpdic);

		comp = wcsncmp(key.c_str(), wbuf, key.size());
		if(comp == 0)
		{
			if((p = wcschr(wbuf, L'\x20')) != NULL)
			{
				if((p = wcschr(p, L'/')) != NULL)
				{
					candidate = p;
				}
			}
			break;
		}
		else if(comp > 0)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}

	fclose(fpdic);
	fclose(fpidx);

	return candidate;
}

std::wstring ConvertKey(const std::wstring &key)
{
	std::wstring ret;

	if(lua != NULL)
	{
		lua_getglobal(lua, "lua_skk_convert_key");
		lua_pushstring(lua, WCTOU8(key));

		if(lua_pcall(lua, 1, 1, 0) == LUA_OK)
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
		//数値変換のみ
		ret = std::regex_replace(key, std::wregex(L"[0-9]+"), std::wstring(L"#"));
	}

	return ret;
}

std::wstring ConvertCandidate(const std::wstring &key, const std::wstring &candidate)
{
	std::wstring ret;

	if(lua != NULL)
	{
		lua_getglobal(lua, "lua_skk_convert_candidate");
		lua_pushstring(lua, WCTOU8(key));
		lua_pushstring(lua, WCTOU8(candidate));

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
		//concatのみ
		ret = ParseConcat(candidate);
	}

	return ret;
}

int lua_search_skk_dictionary(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchSKKDic(searchkey);
	lua_pushstring(lua, WCTOU8(candidate.c_str()));

	return 1;
}

int lua_search_user_dictionary(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring okurikey = U8TOWC(lua_tostring(lua, 2));
	std::wstring candidate = SearchUserDic(searchkey, okurikey);
	lua_pushstring(lua, WCTOU8(candidate.c_str()));

	return 1;
}

int lua_search_skk_server(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchSKKServer(searchkey);
	lua_pushstring(lua, WCTOU8(candidate.c_str()));

	return 1;
}

int lua_search_unicode(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchUnicode(searchkey);
	lua_pushstring(lua, WCTOU8(candidate.c_str()));

	return 1;
}

int lua_search_jisx0213(lua_State *lua)
{
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));
	std::wstring candidate = SearchJISX0213(searchkey);
	lua_pushstring(lua, WCTOU8(candidate.c_str()));

	return 1;
}

int lua_complement(lua_State *lua)
{
	std::wstring candidate;
	SKKDICCANDIDATES sc;
	SKKDICCANDIDATES::iterator sc_itr;
	std::wstring searchkey = U8TOWC(lua_tostring(lua, 1));

	SearchComplement(searchkey, sc);

	for(sc_itr = sc.begin(); sc_itr != sc.end(); sc_itr++)
	{
		candidate += L"/" + sc_itr->first;
	}
	if(!candidate.empty())
	{
		candidate += L"/\n";
	}

	lua_pushstring(lua, WCTOU8(candidate.c_str()));

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
	StartSaveSKKUserDic();
	return 0;
}
