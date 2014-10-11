/*
  UTF-8 Wrapper for Windows
*/

#ifndef U8W_H
#define U8W_H

#include <stdio.h>
#include <windows.h>

#include "lua.h"

/* from ansi code page to utf-16 */
LUA_API wchar_t *u8lmbtolwc(const char *s, UINT codepage);	/* call free function to deallocate */
/* from utf-16 to ansi code page */
LUA_API char *u8lwctolmb(const wchar_t *s, UINT codepage);	/* call free function to deallocate */

/* from utf-8 to utf-16 */
LUA_API wchar_t *u8stows(const char *s);	/* call free function to deallocate */
/* from utf-16 to utf-8 */
LUA_API char *u8wstos(const wchar_t *s);	/* call free function to deallocate */

LUA_API DWORD u8GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
LUA_API DWORD u8FormatMessage(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId,
	LPSTR lpBuffer, DWORD nSize, va_list *Arguments);
LUA_API HMODULE u8LoadLibraryEx(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

LUA_API FILE *u8fopen(const char *fname, const char *mode);
LUA_API FILE *u8freopen(const char *fname, const char *mode, FILE *oldfp);
LUA_API FILE *u8popen(const char *command, const char *mode);
LUA_API int u8fprintf(FILE *file, const char *format, ...);
LUA_API int u8printf(const char *format, ...);
LUA_API char *u8fgets(char *buf, int len, FILE *file);
LUA_API int u8fputs(const char *buf, FILE *file);
LUA_API char *u8getenv(const char *varname);	/* call free function to deallocate */
LUA_API char *u8tmpnam(char *str);
LUA_API int u8system(const char *command);
LUA_API int u8remove(const char *fname);
LUA_API int u8rename(const char *oldfname, const char *newfname);

#if !defined(lu8w_c)
#if defined(LUA_CORE) || defined(LUA_LIB) || defined(lua_c) || defined(luac_c)
#undef LoadString

#define GetModuleFileNameA u8GetModuleFileName
#define FormatMessageA u8FormatMessage
#define LoadLibraryExA u8LoadLibraryEx

#define fopen u8fopen
#define freopen u8freopen
#define _popen u8popen
#define fprintf u8fprintf
#define printf u8printf
#define fgets u8fgets
#define fputs u8fputs
#define getenv u8getenv
#define tmpnam u8tmpnam
#define system u8system
#define remove u8remove
#define rename u8rename
#endif /* LUA_CORE or LUA_LIB or lua_c or luac_c */
#endif /* not lu8w_c */

#endif /* U8W_H */
