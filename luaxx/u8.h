/*
  Conversion from UTF-8 to UTF-16 and vice versa for Windows API and C runtime

  Put '#include "u8.h"' after standard header files include directive.
  Call setlocale function before using following functions.
*/

#ifndef U8_H
#define U8_H

#include <Windows.h>
#include <stdio.h>
#include <locale.h>

#ifdef U8EXT
#define u8api __declspec(dllexport)
#else
#define u8api __declspec(dllimport)
#endif

u8api DWORD u8GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
u8api DWORD u8FormatMessage(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId,
	LPSTR lpBuffer, DWORD nSize, va_list *Arguments);
u8api HMODULE u8LoadLibraryEx(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

u8api FILE* u8fopen(const char* fname, const char* mode);
u8api FILE* u8freopen(const char* fname, const char* mode, FILE* oldfp);
u8api FILE* u8popen(const char* command, const char* mode);
u8api int u8fprintf(FILE *file, const char* format, ...);
u8api int u8printf(const char* format, ...);
u8api char* u8fgets(char* buf, int len, FILE* file);
u8api int u8fputs(const char* buf, FILE* file);
u8api char* u8getenv(const char *varname);	/* needs "if (env != NULL) free((void*)env);" */
u8api char* u8tmpnam(char *buf);
u8api int u8system(const char *command);
u8api int u8remove(const char *fname);
u8api int u8rename(const char *oldfname, const char *newfname);
u8api char* u8setlocale(int category, const char *locale);

#if defined(U8EXT) || defined(lua_c)
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
#define setlocale u8setlocale
#endif /* U8EXT or lua_c */

#endif /* U8_H */
