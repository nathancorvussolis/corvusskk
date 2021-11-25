
#include "utf8.h"
#include "imcrvmgr.h"

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

int lua_reverse(lua_State *lua)
{
	std::wstring key;

	if (lua_isstring(lua, 1))
	{
		std::wstring candidate = U8TOWC(lua_tostring(lua, 1));

		SearchReverse(candidate, key);
	}

	lua_pushstring(lua, WCTOU8(key));

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
