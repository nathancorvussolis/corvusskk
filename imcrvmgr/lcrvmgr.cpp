
#include "utf8.h"
#include "imcrvmgr.h"

int lua_search_skk_dictionary(lua_State *L)
{
	std::wstring candidate;

	if (lua_isstring(L, 1) &&
		lua_isstring(L, 2))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(L, 1));
		std::wstring okurikey = U8TOWC(lua_tostring(L, 2));

		candidate = SearchSKKDic(searchkey, okurikey);
	}

	lua_pushstring(L, WCTOU8(candidate));

	return 1;
}

int lua_search_user_dictionary(lua_State *L)
{
	std::wstring candidate;

	if (lua_isstring(L, 1) &&
		lua_isstring(L, 2))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(L, 1));
		std::wstring okurikey = U8TOWC(lua_tostring(L, 2));

		candidate = SearchUserDic(searchkey, okurikey);
	}

	lua_pushstring(L, WCTOU8(candidate));

	return 1;
}

int lua_search_skk_server(lua_State *L)
{
	std::wstring candidate;

	if (lua_isstring(L, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(L, 1));

		candidate = SearchSKKServer(searchkey);
	}

	lua_pushstring(L, WCTOU8(candidate));

	return 1;
}

int lua_search_skk_server_info(lua_State *L)
{
	std::wstring server_ver = GetSKKServerInfo(SKK_VER);
	std::wstring server_hst = GetSKKServerInfo(SKK_HST);

	lua_pushstring(L, WCTOU8(server_ver));
	lua_pushstring(L, WCTOU8(server_hst));

	return 2;
}

int lua_search_unicode(lua_State *L)
{
	std::wstring candidate;

	if (lua_isstring(L, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(L, 1));

		candidate = SearchUnicode(searchkey);
	}

	lua_pushstring(L, WCTOU8(candidate));

	return 1;
}

int lua_search_jisx0213(lua_State *L)
{
	std::wstring candidate;

	if (lua_isstring(L, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(L, 1));

		candidate = SearchJISX0213(searchkey);
	}

	lua_pushstring(L, WCTOU8(candidate));

	return 1;
}

int lua_search_jisx0208(lua_State *L)
{
	std::wstring candidate;

	if (lua_isstring(L, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(L, 1));

		candidate = SearchJISX0208(searchkey);
	}

	lua_pushstring(L, WCTOU8(candidate));

	return 1;
}

int lua_search_character_code(lua_State *L)
{
	std::wstring candidate;

	if (lua_isstring(L, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(L, 1));

		candidate = SearchCharacterCode(searchkey);
	}

	lua_pushstring(L, WCTOU8(candidate));

	return 1;
}

int lua_complement(lua_State *L)
{
	std::wstring candidate;
	SKKDICCANDIDATES sc;

	if (lua_isstring(L, 1))
	{
		std::wstring searchkey = U8TOWC(lua_tostring(L, 1));

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

	lua_pushstring(L, WCTOU8(candidate));

	return 1;
}

int lua_reverse(lua_State *L)
{
	std::wstring key;

	if (lua_isstring(L, 1))
	{
		std::wstring candidate = U8TOWC(lua_tostring(L, 1));

		SearchReverse(candidate, key);
	}

	lua_pushstring(L, WCTOU8(key));

	return 1;
}

int lua_add(lua_State *L)
{
	if (lua_isboolean(L, 1) &&
		lua_isstring(L, 2) &&
		lua_isstring(L, 3) &&
		lua_isstring(L, 4) &&
		lua_isstring(L, 5))
	{
		int okuriari = lua_toboolean(L, 1);
		WCHAR command = (okuriari ? REQ_USER_ADD_A : REQ_USER_ADD_N);
		std::wstring searchkey = U8TOWC(lua_tostring(L, 2));
		std::wstring candidate = U8TOWC(lua_tostring(L, 3));
		std::wstring annotation = U8TOWC(lua_tostring(L, 4));
		std::wstring okuri = U8TOWC(lua_tostring(L, 5));

		AddUserDic(command, searchkey, candidate, annotation, okuri);
	}

	return 0;
}

int lua_delete(lua_State *L)
{
	if (lua_isboolean(L, 1) &&
		lua_isstring(L, 2) &&
		lua_isstring(L, 3))
	{
		int okuriari = lua_toboolean(L, 1);
		WCHAR command = (okuriari ? REQ_USER_DEL_A : REQ_USER_DEL_N);
		std::wstring searchkey = U8TOWC(lua_tostring(L, 2));
		std::wstring candidate = U8TOWC(lua_tostring(L, 3));

		DelUserDic(command, searchkey, candidate);
	}

	return 0;
}

int lua_save(lua_State *L)
{
	StartSaveUserDic(TRUE);

	return 0;
}
