/*
  UTF-8 Wrapper Library for Windows
*/

#include <stdlib.h>
#include <windows.h>

#define lu8wlib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include "lu8w.h"

static int u8w_getacp (lua_State *L) {
  int codepage = GetACP();
  lua_pushinteger(L, codepage);
  return 1;
}

/* from ansi code page to utf-8 */
static int u8w_acptoutf8 (lua_State *L) {
  int res = 0;
  const char *acp = luaL_checkstring(L, 1);
  int codepage = luaL_optint(L, 2, CP_ACP);
  const wchar_t *utf16 = u8lmbtolwc(acp, codepage);
  const char *utf8 = u8wstos(utf16);
  if (utf8) res = 1;
  lua_pushstring(L, utf8);
  if (utf8) free((void *)utf8);
  if (utf16) free((void *)utf16);
  return res;
}

/* from ansi code page to utf-8 */
static int u8w_utf8toacp (lua_State *L) {
  int res = 0;
  const char *utf8 = luaL_checkstring(L, 1);
  int codepage = luaL_optint(L, 2, CP_ACP);
  const wchar_t *utf16 = u8stows(utf8);
  const char *acp = u8lwctolmb(utf16, codepage);
  if (acp) res = 1;
  lua_pushstring(L, acp);
  if (acp) free((void *)acp);
  if (utf16) free((void *)utf16);
  return res;
}

static const luaL_Reg u8wlib[] = {
  {"getacp",     u8w_getacp},
  {"acptoutf8",  u8w_acptoutf8},
  {"utf8toacp",  u8w_utf8toacp},
  {NULL, NULL}
};

LUAMOD_API int luaopen_u8w (lua_State *L) {
  luaL_newlib(L, u8wlib);
  return 1;
}
