/*
  UTF-8 Wrapper for Windows
*/

#include "lprefix.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <windows.h>

#define lu8w_c
#include "lu8w.h"

/* from utf-8 to utf-16 */
wchar_t *u8stows(const char *s)
{
	int len;
	wchar_t *wbuf = NULL;

	len = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
	if(len > 0) {
		wbuf = (wchar_t *)calloc(len, sizeof(wchar_t));
		if(wbuf) {
			MultiByteToWideChar(CP_UTF8, 0, s, -1, wbuf, len);
		}
	}

	/* call free function to deallocate */
	return wbuf;
}

/* from utf-16 to utf-8 */
char *u8wstos(const wchar_t *s)
{
	int len;
	char *buf = NULL;

	len = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
	if(len > 0) {
		buf = (char *)calloc(len, sizeof(char));
		if(buf) {
			WideCharToMultiByte(CP_UTF8, 0, s, -1, buf, len, NULL, NULL);
		}
	}

	/* call free function to deallocate */
	return buf;
}

DWORD u8GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	wchar_t wfname[MAX_PATH];
	char *b;

	if(nSize == 0) return 0;
	lpFilename[0] = '\0';

	GetModuleFileNameW(hModule, wfname, sizeof(wfname) / sizeof(wchar_t));
	b = u8wstos(wfname);
	if(b) {
		if(strlen(b) < nSize) {
			strcpy(lpFilename, b);
		}
		free(b);
	}

	return (DWORD)strlen(lpFilename);
}

DWORD u8FormatMessage(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId,
	LPSTR lpBuffer, DWORD nSize, va_list *Arguments)
{
	DWORD len = 0;
	wchar_t *wbuf = NULL;
	char *b;

	if((lpBuffer == NULL) ||
		((dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) == 0 && nSize == 0) ||
		((dwFlags & FORMAT_MESSAGE_IGNORE_INSERTS) == 0) ||
		((dwFlags & FORMAT_MESSAGE_FROM_STRING) != 0)) {
		return 0;
	}

	FormatMessageW(dwFlags | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		lpSource, dwMessageId, dwLanguageId, (LPWSTR)&wbuf, 0, NULL);
	if(wbuf) {
		b = u8wstos(wbuf);
		if(b) {
			if((dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) != 0) {
				nSize = (DWORD)max(strlen(b) + 1, nSize);
				*((LPSTR *)lpBuffer) = (LPSTR)LocalAlloc(LPTR, nSize);
				lpBuffer = *((LPSTR *)lpBuffer);
				if(lpBuffer == NULL) {
					nSize = 0;
				}
			}
			if(strlen(b) < nSize) {
				strcpy(lpBuffer, b);
				len = (DWORD)strlen(b);
			}
			free(b);
		}
		LocalFree(wbuf);
	}

	return len;
}

HMODULE u8LoadLibraryEx(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	wchar_t *wfname;
	HMODULE lib = NULL;

	wfname = u8stows(lpLibFileName);
	if(wfname) {
		lib = LoadLibraryExW(wfname, hFile, dwFlags);
		free(wfname);
	}

	return lib;
}

FILE *u8fopen(const char *fname, const char *mode)
{
	wchar_t *wfname = u8stows(fname);
	wchar_t *wmode = u8stows(mode);
	FILE *fp = NULL;

	if(wfname && wmode)	{
		fp = _wfopen(wfname, wmode);
	}
	if(wfname) {
		free(wfname);
	}
	if(wmode) {
		free(wmode);
	}

	return fp;
}

FILE *u8freopen(const char *fname, const char *mode, FILE *oldfp)
{
	wchar_t *wfname = u8stows(fname);
	wchar_t *wmode = u8stows(mode);
	FILE *fp = NULL;

	if(wfname && wmode)	{
		fp = _wfreopen(wfname, wmode, oldfp);
	}
	if(wfname) {
		free(wfname);
	}
	if(wmode) {
		free(wmode);
	}

	return fp;
}

FILE *u8popen(const char *command, const char *mode)
{
	wchar_t *wcommand = u8stows(command);
	wchar_t *wmode = u8stows(mode);
	FILE *fp = NULL;

	if(wcommand && wmode)	{
		fp = _wpopen(wcommand, wmode);
	}
	if(wcommand) {
		free(wcommand);
	}
	if(wmode) {
		free(wmode);
	}

	return fp;
}

int u8fprintf(FILE *file, const char *format, ...)
{
	int ret = 0;
	va_list argptr;
	char *buf;
	int buflen;
	wchar_t *wbuf;

	va_start(argptr, format);

	buflen = _vscprintf(format, argptr) + 1;
	if(buflen <= 0) {
		return -1;
	}

	buf = (char *)calloc(buflen, sizeof(char));
	if(buf == NULL) {
		return -1;
	}

	vsnprintf(buf, buflen, format, argptr);

	va_end(argptr);

	if(file == stdout || file == stderr) {
		wbuf = u8stows(buf);
		if(wbuf) {
			ret = fwprintf(file, L"%s", wbuf);
			free(wbuf);
		}
	}
	else {
		ret = fprintf(file, "%s", buf);
	}

	free(buf);

	return ret;
}

int u8printf(const char *format, ...)
{
	int ret = 0;
	va_list argptr;
	char *buf;
	int buflen;
	wchar_t *wbuf;

	va_start(argptr, format);

	buflen = _vscprintf(format, argptr) + 1;
	if(buflen <= 0) {
		return -1;
	}

	buf = (char *)calloc(buflen, sizeof(char));
	if(buf == NULL) {
		return -1;
	}

	vsnprintf(buf, buflen, format, argptr);

	va_end(argptr);

	wbuf = u8stows(buf);
	if(wbuf) {
		ret = wprintf(L"%s", wbuf);
		free(wbuf);
	}

	free(buf);

	return ret;
}

char *u8fgets(char *buf, int len, FILE *file)
{
	wint_t c, c0;
	wchar_t ws[2 + 1];
	char *b, *dst = NULL;

	if(file == stdin) {
		if(buf == NULL || len <= 0) return NULL;

		buf[0] = '\0';
		c0 = L'\0';
		while((c = fgetwc(file)) != WEOF) {
			if(IS_HIGH_SURROGATE(c0)) {
				if(IS_LOW_SURROGATE(c)) {
					ws[0] = c0;
					ws[1] = c;
					ws[2] = L'\0';
				}
				else {
					ungetwc(c, file);
					ws[0] = c0;
					ws[1] = L'\0';
				}
			}
			else if(IS_HIGH_SURROGATE(c)) {
				if(len <= (int)(strlen(buf) + 4)) {
					ungetwc(c, file);
					break;
				}
				c0 = c;
				continue;
			}
			else {
				ws[0] = c;
				ws[1] = L'\0';
			}
			c0 = L'\0';

			b = u8wstos(ws);
			if(b) {
				if(len > (int)(strlen(buf) + strlen(b))) {
					strcat(buf, b);
					free(b);
				}
				else {
					ungetwc(c, file);
					free(b);
					break;
				}
			}

			if(c == L'\n') break;
		}

		if(strlen(buf) > 0) dst = buf;
	}
	else {
		dst = fgets(buf, len, file);
	}

	return dst;
}

int u8fputs(const char *buf, FILE *file)
{
	wchar_t *wbuf;
	int ret = 0;

	if(file == stdout || file == stderr) {
		wbuf = u8stows(buf);
		if(wbuf) {
			ret = fputws(wbuf, file);
			free(wbuf);
		}
	}
	else {
		ret = fputs(buf, file);
	}

	return ret;
}

char *u8getenv(const char *varname)
{
	wchar_t *wvarname;
	wchar_t *wenv;
	char *env = NULL;

	wvarname = u8stows(varname);
	if(wvarname) {
		wenv = _wgetenv(wvarname);
		if(wenv) {
			env = u8wstos(wenv);
		}
		free(wvarname);
	}

	/* call free function to deallocate */
	return env;
}

char *u8tmpnam(char *str)
{
	static char buf[L_tmpnam];
	wchar_t wbuf[L_tmpnam];
	wchar_t *w;
	char *b, *t = NULL;

	w = _wtmpnam(wbuf);
	if(w) {
		b = u8wstos(w);
		if(b) {
			if(str == NULL) {
				t = buf;
			}
			else {
				t = str;
			}
			if(strlen(b) < L_tmpnam) {
				strcpy(t, b);
			}
			free(b);
		}
	}

	return t;
}

int u8system(const char *command)
{
	wchar_t *wbuf;
	int r = -1;

	wbuf = u8stows(command);
	if(wbuf) {
		r = _wsystem(wbuf);
		free(wbuf);
	}

	return r;
}

int u8remove(const char *fname)
{
	wchar_t *wfname;
	int r = -1;

	wfname = u8stows(fname);
	if(wfname) {
		r = _wremove(wfname);
		free(wfname);
	}

	return r;
}

int u8rename(const char *oldfname, const char *newfname)
{
	wchar_t *woldfname;
	wchar_t *wnewfname;
	int r = -1;

	woldfname = u8stows(oldfname);
	wnewfname = u8stows(newfname);
	if(woldfname && wnewfname) {
		r = _wrename(woldfname, wnewfname);
	}
	if(woldfname) {
		free(woldfname);
	}
	if(wnewfname) {
		free(wnewfname);
	}

	return r;
}
