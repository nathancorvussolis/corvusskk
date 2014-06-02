/*
  Conversion from UTF-8 to UTF-16 and vice versa for Windows API and C runtime

  Put '#include "u8.h"' after standard header files include directive.
  Call setlocale function before using following functions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include <Windows.h>

#define U8EXT
#include "u8.h"

static wchar_t* u8wstr(const char* s)
{
	int len;
	wchar_t *wbuf = NULL;

	len = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
	if(len) {
		wbuf = (wchar_t*)calloc(len, sizeof(wchar_t));
		if(wbuf) {
			MultiByteToWideChar(CP_UTF8, 0, s, -1, wbuf, len);
		}
	}

	return wbuf;
}

static char* u8str(const wchar_t* s)
{
	int len;
	char *buf = NULL;

	len = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
	if(len) {
		buf = (char*)calloc(len, sizeof(char));
		if(buf) {
			WideCharToMultiByte(CP_UTF8, 0, s, -1, buf, len, NULL, NULL);
		}
	}

	return buf;
}

u8api DWORD u8GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	wchar_t wfname[MAX_PATH];
	char *b;

	if(nSize == 0) return 0;
	lpFilename[0] = '\0';

	GetModuleFileNameW(hModule, wfname, _countof(wfname));
	b = u8str(wfname);
	if(b) {
		strcpy_s(lpFilename, nSize, b);
		free(b);
	}

	return (DWORD)strlen(lpFilename);
}

u8api DWORD u8FormatMessage(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId,
	LPSTR lpBuffer, DWORD nSize, va_list *Arguments)
{
	wchar_t *wbuf;
	char *b;

	if(nSize == 0) return 0;
	lpBuffer[0] = '\0';

	wbuf = (wchar_t*)calloc(nSize, sizeof(wchar_t));
	if(wbuf){
		FormatMessageW(dwFlags, lpSource, dwMessageId, dwLanguageId, wbuf, nSize, NULL);
		b = u8str(wbuf);
		if(b) {
			strcpy_s(lpBuffer, nSize, b);
			free(b);
		}
		free(wbuf);
	}

	return (DWORD)strlen(lpBuffer);
}

u8api HMODULE u8LoadLibraryEx(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	wchar_t *wfname;
	HMODULE lib = NULL;

	wfname = u8wstr(lpLibFileName);
	if(wfname) {
		lib = LoadLibraryExW(wfname, hFile, dwFlags);
		free(wfname);
	}

	return lib;
}

u8api FILE* u8fopen(const char* fname, const char* mode)
{
	wchar_t *wfname = u8wstr(fname);
	wchar_t *wmode = u8wstr(mode);
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

u8api FILE* u8freopen(const char* fname, const char* mode, FILE* oldfp)
{
	wchar_t *wfname = u8wstr(fname);
	wchar_t *wmode = u8wstr(mode);
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

u8api FILE* u8popen(const char* command, const char* mode)
{
	wchar_t *wcommand = u8wstr(command);
	wchar_t *wmode = u8wstr(mode);
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

u8api int u8fprintf(FILE *file, const char* format, ...)
{
	int ret = 0;
	va_list argptr;
	char *buf = NULL;
	wchar_t *wbuf;
	int i, clen, wlen;

	va_start(argptr, format);

	for(i = 1; i <= 32; i++) {
		buf = (char *)calloc(i * i * 1024, sizeof(char)); //MAX 1MB
		if(buf == NULL) {
			return -1;
		}
		clen = vsnprintf_s(buf, i * i * 1024, _TRUNCATE, format, argptr);
		if(clen == -1) {
			free(buf);
			buf = NULL;
		}
		else {
			break;
		}
	}

	if(buf == NULL) {
		return -1;
	}

	if(file == stdout || file == stderr) {
		wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
		wbuf = (wchar_t*)calloc(wlen, sizeof(wchar_t));
		if(wbuf != NULL)
		{
			MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, wlen);
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

u8api int u8printf(const char* format, ...)
{
	int ret = 0;
	va_list argptr;
	char *buf = NULL;
	wchar_t *wbuf;
	int i, clen, wlen;

	va_start(argptr, format);

	for(i = 1; i <= 32; i++) {
		buf = (char *)calloc(i * i * 1024, sizeof(char)); //MAX 1MB
		if(buf == NULL) {
			return -1;
		}
		clen = vsnprintf_s(buf, i * i * 1024, _TRUNCATE, format, argptr);
		if(clen == -1) {
			free(buf);
			buf = NULL;
		}
		else {
			break;
		}
	}

	wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	wbuf = (wchar_t*)calloc(wlen, sizeof(wchar_t));
	if(wbuf != NULL)
	{
		MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, wlen);
		ret = fwprintf(stdout, L"%s", wbuf);
		free(wbuf);
	}

	free(buf);

	return ret;
}

u8api char* u8fgets(char* buf, int len, FILE* file)
{
	wchar_t wc, ws[2 + 1];
	char cc[4 + 1];
	char* dst = NULL;

	if(file == stdin) {
		if(buf == NULL || len <= 0) return NULL;

		buf[0] = '\0';
		while(1) {
			wc = fgetwc(file);
			if(wc == WEOF) break;
			if(wc == L'\r') continue;

			ws[0] = wc;
			if(IS_HIGH_SURROGATE(wc)) {
				wc = fgetwc(file);
				if(IS_LOW_SURROGATE(wc)) {
					ws[1] = wc;
					ws[2] = L'\0';
				}
				else {
					ungetwc(wc, file);
					continue;
				}
			}
			else {
				ws[1] = L'\0';
				ws[2] = L'\0';
			}

			WideCharToMultiByte(CP_UTF8, 0, ws, -1, cc, _countof(cc), NULL, NULL);
			if(len > (int)(strlen(buf) + strlen(cc))) {
				strcat_s(buf, len, cc);
			}
			else {
				if(ws[1] != L'\0') ungetwc(ws[1], file);
				if(ws[0] != L'\0') ungetwc(ws[0], file);
				break;
			}

			if(wc == L'\n') break;
		}
		dst = buf;
	}
	else {
		dst = fgets(buf, len, file);
	}

	return dst;
}

u8api int u8fputs(const char* buf, FILE* file)
{
	wchar_t *wbuf;
	int ret = 0;

	if(file == stdout || file == stderr) {
		wbuf = u8wstr(buf);
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

u8api char* u8getenv(const char *varname)
{
	wchar_t *wvarname;
	wchar_t *wenv;
	char *env = NULL;

	wvarname = u8wstr(varname);
	if(wvarname) {
		wenv = _wgetenv(wvarname);
		if(wenv) {
			env = u8str(wenv);
		}
		free(wvarname);
	}

	return env;	/* need "if (env != NULL) free((void*)env);" */
}

u8api char* u8tmpnam(char *str)
{
	wchar_t *wbuf;
	wchar_t *w;
	char *b = NULL;

	wbuf = u8wstr(str);
	if(wbuf) {
		w = _wtmpnam(wbuf);
		if(w) {
			b = u8str(w);
			free(w);
		}
		free(wbuf);
	}

	return b;
}

u8api int u8system(const char *command)
{
	wchar_t *wbuf;
	int r = -1;

	wbuf = u8wstr(command);
	if(wbuf) {
		r = _wsystem(wbuf);
		free(wbuf);
	}

	return r;
}

u8api int u8remove(const char *fname)
{
	wchar_t *wfname;
	int r = -1;

	wfname = u8wstr(fname);
	if(wfname) {
		r = _wremove(wfname);
		free(wfname);
	}

	return r;
}

u8api int u8rename(const char *oldfname, const char *newfname)
{
	wchar_t *woldfname;
	wchar_t *wnewfname;
	int r = -1;

	woldfname = u8wstr(oldfname);
	wnewfname = u8wstr(newfname);
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

u8api char* u8setlocale(int category, const char *locale)
{
	wchar_t *r;
	wchar_t *wl;
	char *l = NULL;

	wl = u8wstr(locale);
	r = _wsetlocale(category, wl);
	if(r != NULL) {
		l = (char *)locale;
	}
	if(wl) {
		free(wl);
	}

	return l;
}
