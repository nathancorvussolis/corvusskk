
#include "eucjis2004.h"
#include "eucjp.h"
#include "parseskkdic.h"
#include "configxml.h"
#include "utf8.h"
#include "zlib.h"
#include "imcrvcnf.h"
#include "resource.h"

#define E_MAKESKKDIC_OK			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0)
#define E_MAKESKKDIC_DOWNLOAD	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 1)
#define E_MAKESKKDIC_FILEIO		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 2)
#define E_MAKESKKDIC_ENCODING	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 3)
#define E_MAKESKKDIC_UNGZIP		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 4)
#define E_MAKESKKDIC_UNTAR		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 5)

#define GZBUFSIZE		65536
#define TARBLOCKSIZE	512

BOOL IsMakeSKKDicCanceled(HANDLE hCancelEvent)
{
	return (WaitForSingleObject(hCancelEvent, 0) == WAIT_OBJECT_0);
}

HRESULT DownloadMakePath(LPCWSTR url, LPWSTR path, size_t len)
{
	WCHAR dir[MAX_PATH] = {};
	WCHAR fname[MAX_PATH] = {};

	DWORD temppathlen = GetTempPathW(_countof(dir), dir);
	if (temppathlen == 0 || temppathlen > _countof(dir))
	{
		return E_MAKESKKDIC_DOWNLOAD;
	}
	wcsncat_s(dir, TEXTSERVICE_NAME, _TRUNCATE);

	CreateDirectoryW(dir, nullptr);

	LPCWSTR fnurl = wcsrchr(url, L'/');
	if (fnurl == nullptr || *(fnurl + 1) == L'\0')
	{
		return E_MAKESKKDIC_DOWNLOAD;
	}
	wcsncpy_s(fname, fnurl + 1, _TRUNCATE);

	for (int i = 0; i < _countof(fname) && fname[i] != L'\0'; i++)
	{
		UINT type = PathGetCharTypeW(fname[i]);

		if ((type & (GCT_LFNCHAR | GCT_SHORTCHAR)) == 0)
		{
			fname[i] = L'_';
		}
	}

	_snwprintf_s(path, len, _TRUNCATE, L"%s\\%s", dir, fname);

	return S_OK;
}

HRESULT DownloadSKKDic(HANDLE hCancelEvent, LPCWSTR url, LPWSTR path, size_t len)
{
	if (FAILED(DownloadMakePath(url, path, len)))
	{
		return E_MAKESKKDIC_DOWNLOAD;
	}

	HINTERNET hInet = InternetOpenW(TEXTSERVICE_NAME L"/" TEXTSERVICE_VER, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if (hInet == nullptr)
	{
		return E_MAKESKKDIC_DOWNLOAD;
	}

	HINTERNET hUrl = InternetOpenUrlW(hInet, url, nullptr, 0, 0, 0);
	if (hUrl == nullptr)
	{
		InternetCloseHandle(hInet);

		return E_MAKESKKDIC_DOWNLOAD;
	}

	DWORD dwStatusCode = 0;
	DWORD dwQueryLength = sizeof(dwStatusCode);
	if (HttpQueryInfoW(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwQueryLength, 0) == FALSE)
	{
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInet);

		return E_MAKESKKDIC_DOWNLOAD;
	}

	switch (dwStatusCode)
	{
	case HTTP_STATUS_OK:
		break;
	default:
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInet);

		return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_HTTP, dwStatusCode);
		break;
	}

	CHAR rbuf[RECVBUFSIZE];
	BOOL retRead;
	DWORD bytesRead = 0;
	FILE *fp = nullptr;

	_wfopen_s(&fp, path, modeWB);
	if (fp == nullptr)
	{
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInet);

		return E_MAKESKKDIC_FILEIO;
	}

	while (true)
	{
		if (IsMakeSKKDicCanceled(hCancelEvent))
		{
			fclose(fp);
			DeleteFileW(path);

			InternetCloseHandle(hUrl);
			InternetCloseHandle(hInet);

			return E_ABORT;
		}

		ZeroMemory(rbuf, sizeof(rbuf));
		retRead = InternetReadFile(hUrl, rbuf, sizeof(rbuf), &bytesRead);
		if (retRead)
		{
			if (bytesRead == 0)
			{
				break;
			}
		}
		else
		{
			fclose(fp);
			DeleteFileW(path);

			InternetCloseHandle(hUrl);
			InternetCloseHandle(hInet);

			return E_MAKESKKDIC_DOWNLOAD;
		}

		fwrite(rbuf, bytesRead, 1, fp);
	}

	fclose(fp);

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInet);

	return S_OK;
}

HRESULT CheckMultiByteFile(HANDLE hCancelEvent, LPCWSTR path, SKKDICENCODING encoding)
{
	HRESULT hr = S_OK;
	FILE *fp = nullptr;
	CHAR buf[READBUFSIZE];
	std::string strbuf;
	size_t len;

	_wfopen_s(&fp, path, modeRB);
	if (fp == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	while (true)
	{
		strbuf.clear();

		while (fgets(buf, _countof(buf), fp) != nullptr)
		{
			if (IsMakeSKKDicCanceled(hCancelEvent))
			{
				fclose(fp);
				return E_ABORT;
			}

			strbuf += buf;

			if (!strbuf.empty() && strbuf.back() == '\n')
			{
				break;
			}
		}

		if (ferror(fp) != 0)
		{
			hr = E_MAKESKKDIC_FILEIO;
			break;
		}

		if (strbuf.empty())
		{
			break;
		}

		switch (encoding)
		{
		case enc_euc_jis_2004: //EUC-JIS-2004
			if (EucJis2004ToWideChar(strbuf.c_str(), nullptr, nullptr, &len) == FALSE)
			{
				hr = S_FALSE;
			}
			break;
		case enc_euc_jp: //EUC-JP
			if (EucJPToWideChar(strbuf.c_str(), nullptr, nullptr, &len) == FALSE)
			{
				hr = S_FALSE;
			}
			break;
		case enc_utf_8: //UTF-8
			if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
				strbuf.c_str(), -1, nullptr, 0) == 0)
			{
				hr = S_FALSE;
			}
			break;
		default:
			hr = S_FALSE;
			break;
		}

		if (hr == S_FALSE)
		{
			break;
		}
	}

	fclose(fp);

	return hr;
}

HRESULT CheckWideCharFile(HANDLE hCancelEvent, LPCWSTR path)
{
	HRESULT hr = S_OK;
	FILE *fp = nullptr;
	WCHAR wbuf[READBUFSIZE / sizeof(WCHAR)];
	std::wstring wstrbuf;

	_wfopen_s(&fp, path, modeRB);
	if (fp == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	while (true)
	{
		wstrbuf.clear();

		while (fgetws(wbuf, _countof(wbuf), fp) != nullptr)
		{
			if (IsMakeSKKDicCanceled(hCancelEvent))
			{
				fclose(fp);
				return E_ABORT;
			}

			wstrbuf += wbuf;

			if (!wstrbuf.empty() && wstrbuf.back() == L'\n')
			{
				break;
			}
		}

		if (ferror(fp) != 0)
		{
			hr = E_MAKESKKDIC_FILEIO;
			break;
		}

		if (wstrbuf.empty())
		{
			break;
		}

		if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
			wstrbuf.c_str(), -1, nullptr, 0, nullptr, nullptr) == 0)
		{
			hr = S_FALSE;
			break;
		}
	}

	fclose(fp);

	return hr;
}

void LoadSKKDicAdd(SKKDIC &skkdic, const std::wstring &key, const std::wstring &candidate, const std::wstring &annotation)
{
	SKKDICENTRY skkdicentry;
	LPCWSTR seps = L",";
	std::wstring annotation_seps;
	std::wstring annotation_esc;

	if (!annotation.empty())
	{
		annotation_seps = seps + ParseConcat(annotation) + seps;
	}

	auto skkdic_itr = skkdic.find(key);
	if (skkdic_itr == skkdic.end())
	{
		skkdicentry.first = key;
		skkdicentry.second.push_back(std::make_pair(candidate, annotation_seps));
		skkdic.insert(skkdicentry);
	}
	else
	{
		bool exist = false;
		FORWARD_ITERATION_I(sc_itr, skkdic_itr->second)
		{
			if (sc_itr->first == candidate)
			{
				annotation_esc = ParseConcat(sc_itr->second);
				if (annotation_esc.find(annotation_seps) == std::wstring::npos)
				{
					if (annotation_esc.empty())
					{
						sc_itr->second.assign(MakeConcat(annotation_seps));
					}
					else
					{
						annotation_esc.append(ParseConcat(annotation) + seps);
						sc_itr->second.assign(MakeConcat(annotation_esc));
					}
				}
				exist = true;
				break;
			}
		}
		if (!exist)
		{
			skkdic_itr->second.push_back(std::make_pair(candidate, MakeConcat(annotation_seps)));
		}
	}
}

HRESULT LoadSKKDicFile(HANDLE hCancelEvent, LPCWSTR path, size_t &count_key, size_t &count_cand, SKKDIC &entries_a, SKKDIC &entries_n)
{
	FILE *fp = nullptr;
	std::wstring key;
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;

	SKKDICENCODING encoding = enc_none;

	_wfopen_s(&fp, path, modeRB);
	if (fp == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	//check BOM
	WCHAR bom = L'\0';
	fread(&bom, 2, 1, fp);
	fclose(fp);

	if (bom == BOM)
	{
		//UTF-16LE
		encoding = enc_utf_16;

		HRESULT hr = CheckWideCharFile(hCancelEvent, path);
		switch (hr)
		{
		case S_OK:
			break;
		case E_ABORT:
		case E_MAKESKKDIC_FILEIO:
			return hr;
			break;
		default:
			//Error
			encoding = enc_error;
			break;
		}
	}

	//UTF-8 ?
	if (encoding == enc_none)
	{
		HRESULT hr = CheckMultiByteFile(hCancelEvent, path, enc_utf_8);
		switch (hr)
		{
		case S_OK:
			encoding = enc_utf_8;
			break;
		case E_ABORT:
		case E_MAKESKKDIC_FILEIO:
			return hr;
			break;
		default:
			break;
		}
	}

	//EUC-JIS-2004 ?
	if (encoding == enc_none)
	{
		HRESULT hr = CheckMultiByteFile(hCancelEvent, path, enc_euc_jis_2004);
		switch (hr)
		{
		case S_OK:
			encoding = enc_euc_jis_2004;
			break;
		case E_ABORT:
		case E_MAKESKKDIC_FILEIO:
			return hr;
			break;
		default:
			break;
		}
	}

	//EUC-JP ?
	if (encoding == enc_none)
	{
		HRESULT hr = CheckMultiByteFile(hCancelEvent, path, enc_euc_jp);
		switch (hr)
		{
		case S_OK:
			encoding = enc_euc_jp;
			break;
		case E_ABORT:
		case E_MAKESKKDIC_FILEIO:
			return hr;
			break;
		default:
			break;
		}
	}

	switch (encoding)
	{
	case enc_euc_jis_2004:
		//EUC-JIS-2004
	case enc_euc_jp:
		//EUC-JP
		_wfopen_s(&fp, path, modeRT);
		break;
	case enc_utf_8:
		//UTF-8
		_wfopen_s(&fp, path, modeRccsUTF8);
		break;
	case enc_utf_16:
		//UTF-16LE
		_wfopen_s(&fp, path, modeRccsUTF16);
		break;
	default:
		return E_MAKESKKDIC_ENCODING;
		break;
	}
	if (fp == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	// 「;; okuri-ari entries.」、「;; okuri-nasi entries.」がない辞書のエントリは送りなしとする
	int okuri = 0;	//-1:header / 1:okuri-ari entries. / 0:okuri-nasi entries.

	while (true)
	{
		if (IsMakeSKKDicCanceled(hCancelEvent))
		{
			fclose(fp);
			return E_ABORT;
		}

		int rl = ReadSKKDicLine(fp, encoding, okuri, key, sc, so);
		if (rl == -1)
		{
			//EOF
			break;
		}
		else if (rl == 1)
		{
			//comment
			continue;
		}

		switch (okuri)
		{
		case 1:
		case 0:
			++count_key;
			count_cand += sc.size();
			break;
		default:
			break;
		}

		FORWARD_ITERATION_I(sc_itr, sc)
		{
			if (IsMakeSKKDicCanceled(hCancelEvent))
			{
				fclose(fp);
				return E_ABORT;
			}

			LoadSKKDicAdd((okuri == 0 ? entries_n : entries_a), key, sc_itr->first, sc_itr->second);
		}
	}

	fclose(fp);

	return S_OK;
}

HRESULT UnGzip(LPCWSTR gzpath, LPWSTR path, size_t len)
{
	HRESULT ret = E_MAKESKKDIC_UNGZIP;

	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	WCHAR fname[_MAX_FNAME];
	WCHAR ext[_MAX_EXT];

	if (_wsplitpath_s(gzpath, drive, dir, fname, ext) != 0)
	{
		return E_MAKESKKDIC_UNGZIP;
	}

	if (_wcsicmp(ext, L".tgz") == 0)
	{
		wcsncat_s(fname, L".tar", _TRUNCATE);
	}
	else if (_wcsicmp(ext, L".gz") != 0)
	{
		return S_FALSE;
	}

	WCHAR tempdir[MAX_PATH];

	DWORD temppathlen = GetTempPathW(_countof(tempdir), tempdir);
	if (temppathlen == 0 || temppathlen > _countof(tempdir))
	{
		return E_MAKESKKDIC_UNGZIP;
	}
	wcsncat_s(tempdir, TEXTSERVICE_NAME, _TRUNCATE);

	CreateDirectoryW(tempdir, nullptr);

	gzFile gzf = gzopen_w(gzpath, "rb");
	if (gzf == NULL)
	{
		return E_MAKESKKDIC_UNGZIP;
	}

	gzbuffer(gzf, GZBUFSIZE);

	_snwprintf_s(path, len, _TRUNCATE, L"%s\\%s", tempdir, fname);

	FILE *fpo = nullptr;
	_wfopen_s(&fpo, path, L"wb");
	if (fpo == nullptr)
	{
		gzclose(gzf);
		return E_MAKESKKDIC_UNGZIP;
	}

	PCHAR buf = (PCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GZBUFSIZE);

	if (buf != nullptr)
	{
		while (true)
		{
			int len = gzread(gzf, buf, GZBUFSIZE);

			if (len == 0)
			{
				ret = S_OK;
				break;
			}
			else if (len < 0)
			{
				ret = E_MAKESKKDIC_UNGZIP;
				break;
			}

			if (fwrite(buf, len, 1, fpo) != 1)
			{
				ret = E_MAKESKKDIC_UNGZIP;
				break;
			}
		}

		HeapFree(GetProcessHeap(), 0, buf);
	}

	fclose(fpo);

	if (FAILED(ret))
	{
		DeleteFileW(path);
	}

	gzclose(gzf);

	return ret;
}

int TarParseOct(const char *p, int n)
{
	int i = 0;

	while ((*p < '0' || *p > '7') && n > 0)
	{
		++p;
		--n;
	}

	while (*p >= '0' && *p <= '7' && n > 0)
	{
		i *= 8;
		i += *p - '0';
		++p;
		--n;
	}

	return i;
}

bool TarIsEnd(const char *p)
{
	for (int n = 0; n < TARBLOCKSIZE; ++n)
	{
		if (p[n] != '\0')
		{
			return false;
		}
	}

	return true;
}

bool TarVerify(const char *p)
{
	int u = 0;

	for (int n = 0; n < TARBLOCKSIZE; ++n)
	{
		if (n < 148 || n > 155)
		{
			u += ((unsigned char *)p)[n];
		}
		else
		{
			u += 0x20;
		}
	}

	return (u == TarParseOct(p + 148, 8));
}

HRESULT UnTar(HANDLE hCancelEvent, LPCWSTR tarpath, size_t &count_key, size_t &count_cand, SKKDIC &entries_a, SKKDIC &entries_n)
{
	HRESULT ret = E_MAKESKKDIC_UNTAR;

	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	WCHAR fname[_MAX_FNAME];
	WCHAR ext[_MAX_EXT];

	if (_wsplitpath_s(tarpath, drive, dir, fname, ext) != 0)
	{
		return E_MAKESKKDIC_UNTAR;
	}

	if (_wcsicmp(ext, L".tar") != 0)
	{
		return S_FALSE;
	}

	FILE *fpi = nullptr;
	_wfopen_s(&fpi, tarpath, modeRB);
	if (fpi == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	WCHAR path[MAX_PATH];
	char buff[TARBLOCKSIZE];
	FILE *fpo = nullptr;
	size_t bytes_read;
	int filesize;

	for (;;)
	{
		bytes_read = fread(buff, 1, TARBLOCKSIZE, fpi);

		if (bytes_read < TARBLOCKSIZE)
		{
			break;
		}

		if (TarIsEnd(buff))
		{
			ret = S_OK;
			break;
		}

		if (!TarVerify(buff))
		{
			break;
		}

		filesize = TarParseOct(buff + 124, 12);

		switch (buff[156]) {
		case '1':
		case '2':
		case '3':
		case '4':
		case '6':
			break;
		case '5':
			filesize = 0;
			break;
		default:
			char *p = strrchr(buff, '/');
			if (p == nullptr)
			{
				p = buff;
			}
			else
			{
				++p;
			}

			{
				WCHAR tfname[MAX_PATH];

				wcsncpy_s(tfname, U8TOWC(p), _TRUNCATE);

				for (int i = 0; i < _countof(tfname) && tfname[i] != L'\0'; i++)
				{
					UINT type = PathGetCharTypeW(tfname[i]);

					if ((type & (GCT_LFNCHAR | GCT_SHORTCHAR)) == 0)
					{
						tfname[i] = L'_';
					}
				}

				WCHAR tempdir[MAX_PATH];

				DWORD temppathlen = GetTempPathW(_countof(tempdir), tempdir);
				if (temppathlen == 0 || temppathlen > _countof(tempdir))
				{
					fclose(fpi);
					return E_MAKESKKDIC_UNTAR;
				}
				wcsncat_s(tempdir, TEXTSERVICE_NAME, _TRUNCATE);

				_snwprintf_s(path, _TRUNCATE, L"%s\\%s", tempdir, tfname);

				// extract file if tar filename without extension equals tail of filename.

				size_t tfnamelen = wcslen(tfname);
				size_t fnamelen = wcslen(fname);

				if (tfnamelen < fnamelen) break;

				if (wcscmp(tfname + tfnamelen - fnamelen, fname) != 0) break;

				// extract file if filename without extension is not empty.

				WCHAR ttfname[_MAX_FNAME];

				if (_wsplitpath_s(tfname,
					nullptr, 0, nullptr, 0, ttfname, _countof(ttfname), nullptr, 0) != 0)
				{
					fclose(fpi);
					return E_MAKESKKDIC_UNTAR;
				}

				if (wcslen(ttfname) == 0) break;
			}

			_wfopen_s(&fpo, path, modeWB);

			if (fpo == nullptr)
			{
				fclose(fpi);
				return E_MAKESKKDIC_FILEIO;
			}

			break;
		}

		while (filesize > 0)
		{
			bytes_read = fread(buff, 1, TARBLOCKSIZE, fpi);

			if (bytes_read < TARBLOCKSIZE)
			{
				if (fpo != nullptr) fclose(fpo);
				fclose(fpi);
				return E_MAKESKKDIC_UNTAR;
			}

			if (filesize < TARBLOCKSIZE)
			{
				bytes_read = filesize;
			}

			if (fpo != nullptr)
			{
				if (fwrite(buff, 1, bytes_read, fpo) != bytes_read)
				{
					fclose(fpo);
					fclose(fpi);
					return E_MAKESKKDIC_FILEIO;
				}
			}

			filesize -= (int)bytes_read;
		}

		if (fpo != nullptr)
		{
			fclose(fpo);
			fpo = nullptr;

			HRESULT hr = LoadSKKDicFile(hCancelEvent, path, count_key, count_cand, entries_a, entries_n);
			if (FAILED(hr))
			{
				fclose(fpi);
				return hr;
			}
		}
	}

	fclose(fpi);

	return ret;
}

HRESULT LoadSKKDic(HANDLE hCancelEvent, HWND hDlg, SKKDIC &entries_a, SKKDIC &entries_n)
{
	WCHAR path[MAX_PATH] = {};
	WCHAR text[16] = {};

	HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);
	int count = ListView_GetItemCount(hWndListView);

	for (int i = 0; i < count; i++)
	{
		ListView_SetItemText(hWndListView, i, 1, text);
		ListView_SetItemText(hWndListView, i, 2, text);
	}

	ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 80));
	ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 80));

	for (int i = 0; i < count; i++)
	{
		if (IsMakeSKKDicCanceled(hCancelEvent))
		{
			return E_ABORT;
		}

		BOOL check = ListView_GetCheckState(hWndListView, i);
		if (check == FALSE)
		{
			continue;
		}

		ListView_SetItemState(hWndListView, i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(hWndListView, i, FALSE);

		ListView_GetItemText(hWndListView, i, 0, path, _countof(path));

		size_t count_key = 0;
		size_t count_cand = 0;

		//download
		URL_COMPONENTSW urlc = {};
		urlc.dwStructSize = sizeof(urlc);
		if (InternetCrackUrlW(path, 0, 0, &urlc))
		{
			switch (urlc.nScheme)
			{
			case INTERNET_SCHEME_HTTP:
			case INTERNET_SCHEME_HTTPS:
				{
					WCHAR url[INTERNET_MAX_URL_LENGTH];
					wcsncpy_s(url, path, _TRUNCATE);

					HRESULT hrd = DownloadSKKDic(hCancelEvent, url, path, _countof(path));
					if (FAILED(hrd))
					{
						return hrd;
					}
				}
				break;
			default:
				break;
			}
		}

		//decompress
		{
			WCHAR gzpath[MAX_PATH];
			wcsncpy_s(gzpath, path, _TRUNCATE);

			HRESULT hrg = UnGzip(gzpath, path, _countof(path));
			if (FAILED(hrg))
			{
				return hrg;
			}
		}

		//untar
		{
			WCHAR tarpath[MAX_PATH];
			wcsncpy_s(tarpath, path, _TRUNCATE);

			HRESULT hrg = UnTar(hCancelEvent, tarpath, count_key, count_cand, entries_a, entries_n);
			if (FAILED(hrg))
			{
				return hrg;
			}

			if (hrg == S_FALSE)
			{
				HRESULT hr = LoadSKKDicFile(hCancelEvent, path, count_key, count_cand, entries_a, entries_n);
				if (FAILED(hr))
				{
					return hr;
				}
			}
		}

		{
			WCHAR scount[16];
			LV_ITEM lvi = {};
			lvi.mask = LVFIF_TEXT;
			lvi.iItem = i;

			_snwprintf_s(scount, _TRUNCATE, L"%zu", count_key);
			ListView_SetItemText(hWndListView, i, 1, scount);
			_snwprintf_s(scount, _TRUNCATE, L"%zu", count_cand);
			ListView_SetItemText(hWndListView, i, 2, scount);
		}
	}

	return S_OK;
}

void WriteSKKDicEntry(FILE *fp, const std::wstring &key, const SKKDICCANDIDATES &sc)
{
	std::wstring line;
	std::wstring annotation_esc;

	line = key + L" /";
	FORWARD_ITERATION_I(sc_itr, sc)
	{
		line += sc_itr->first;
		if (sc_itr->second.size() > 2)
		{
			annotation_esc = ParseConcat(sc_itr->second);
			line += L";" + MakeConcat(annotation_esc.substr(1, annotation_esc.size() - 2));
		}
		line += L"/";
	}
	line += L"\r\n";

	fwrite(line.c_str(), line.size() * sizeof(WCHAR), 1, fp);
}

HRESULT WriteSKKDic(HANDLE hCancelEvent, const SKKDIC &entries_a, const SKKDIC &entries_n)
{
	FILE *fp = nullptr;
	WCHAR bom = BOM;
	LPCWSTR crlf = L"\r\n";

	_wfopen_s(&fp, pathskkdic, modeWB);
	if (fp == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	//BOM
	fwrite(&bom, sizeof(bom), 1, fp);

	//送りありエントリ
	fwrite(EntriesAri, wcslen(EntriesAri) * sizeof(WCHAR), 1, fp);
	fseek(fp, -2, SEEK_CUR);
	fwrite(crlf, wcslen(crlf) * sizeof(WCHAR), 1, fp);

	REVERSE_ITERATION_I(entries_ritr, entries_a)
	{
		if (IsMakeSKKDicCanceled(hCancelEvent))
		{
			fclose(fp);
			DeleteFileW(pathskkdic);
			return E_ABORT;
		}

		WriteSKKDicEntry(fp, entries_ritr->first, entries_ritr->second);
	}

	//送りなしエントリ
	fwrite(EntriesNasi, wcslen(EntriesNasi) * sizeof(WCHAR), 1, fp);
	fseek(fp, -2, SEEK_CUR);
	fwrite(crlf, wcslen(crlf) * sizeof(WCHAR), 1, fp);

	FORWARD_ITERATION_I(entries_itr, entries_n)
	{
		if (IsMakeSKKDicCanceled(hCancelEvent))
		{
			fclose(fp);
			DeleteFileW(pathskkdic);
			return E_ABORT;
		}

		WriteSKKDicEntry(fp, entries_itr->first, entries_itr->second);
	}

	fclose(fp);

	return S_OK;
}

void SetTaskbarListMarquee(HWND hwnd, TBPFLAG flag)
{
	CComPtr<ITaskbarList3> pTaskbarList3;
	HRESULT hr = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pTaskbarList3));
	if (SUCCEEDED(hr) && (pTaskbarList3 != nullptr))
	{
		pTaskbarList3->SetProgressState(hwnd, flag);
	}
}

void MakeSKKDicThread(void *p)
{
	WCHAR msg[1024];
	HRESULT hr = S_OK;

	HWND child = (HWND)p;
	HWND parent = PROPSHEET_IDTOHWND(GetWindow(child, GW_OWNER), IDD_DIALOG_DICTIONARY1);
	HWND pdlg = GetParent(child);

	HANDLE hCancelEvent = OpenEventW(SYNCHRONIZE, FALSE, cnfcanceldiceventname);

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	SetTaskbarListMarquee(pdlg, TBPF_INDETERMINATE);

	LONGLONG t = 0;

	LARGE_INTEGER qpf = {};
	BOOL bHRT = QueryPerformanceFrequency(&qpf);

	LONGLONG t0 = 0;
	if (bHRT)
	{
		LARGE_INTEGER qpt = {};
		QueryPerformanceCounter(&qpt);
		t0 = qpt.QuadPart;
	}
	else
	{
		t0 = (LONGLONG)GetTickCount64();
	}

	{
		SKKDIC entries_a, entries_n;

		hr = LoadSKKDic(hCancelEvent, parent, entries_a, entries_n);
		if (SUCCEEDED(hr))
		{
			hr = WriteSKKDic(hCancelEvent, entries_a, entries_n);
		}
	}

	if (bHRT)
	{
		LARGE_INTEGER qpt = {};
		QueryPerformanceCounter(&qpt);

		t = (LONGLONG)(((double)(qpt.QuadPart - t0) / (double)qpf.QuadPart) * 1000);
	}
	else
	{
		t = (LONGLONG)GetTickCount64() - t0;
	}

	CloseHandle(hCancelEvent);

	EndDialog(child, TRUE);

	if (SUCCEEDED(hr))
	{
		SetTaskbarListMarquee(pdlg, TBPF_NOPROGRESS);

		_snwprintf_s(msg, _TRUNCATE, L"完了しました。\r\n\r\n%lld msec", t);

		MessageBoxW(pdlg, msg, TextServiceDesc, MB_OK | MB_ICONINFORMATION);
	}
	else if (hr == E_ABORT)
	{
		_wremove(pathskkdic);

		SetTaskbarListMarquee(pdlg, TBPF_PAUSED);

		MessageBoxW(pdlg, L"中断しました。", TextServiceDesc, MB_OK | MB_ICONWARNING);
	}
	else
	{
		WCHAR errmsg[32] = {};

		switch (hr)
		{
		case E_MAKESKKDIC_DOWNLOAD:
			wcsncpy_s(errmsg, L"ダウンロード", _TRUNCATE);
			break;
		case E_MAKESKKDIC_FILEIO:
			wcsncpy_s(errmsg, L"ファイル入出力", _TRUNCATE);
			break;
		case E_MAKESKKDIC_ENCODING:
			wcsncpy_s(errmsg, L"文字コード", _TRUNCATE);
			break;
		case E_MAKESKKDIC_UNGZIP:
			wcsncpy_s(errmsg, L"GZIP展開", _TRUNCATE);
			break;
		case E_MAKESKKDIC_UNTAR:
			wcsncpy_s(errmsg, L"TAR展開", _TRUNCATE);
			break;
		default:
			break;
		}

		if (HRESULT_SEVERITY(hr) == SEVERITY_ERROR &&
			HRESULT_FACILITY(hr) == FACILITY_HTTP)
		{
			_snwprintf_s(errmsg, _TRUNCATE, L"HTTP %d", HRESULT_CODE(hr));
		}

		WCHAR path[MAX_PATH] = {};
		HWND hWndListView = GetDlgItem(parent, IDC_LIST_SKK_DIC);
		int count = ListView_GetItemCount(hWndListView);
		for (int i = 0; i < count; i++)
		{
			if ((ListView_GetItemState(hWndListView, i, LVIS_SELECTED) & LVIS_SELECTED) != 0)
			{
				ListView_GetItemText(hWndListView, i, 0, path, _countof(path));
				break;
			}
		}

		SetTaskbarListMarquee(pdlg, TBPF_ERROR);

		_snwprintf_s(msg, _TRUNCATE, L"失敗しました。(%s)\n\n%s", errmsg, path);

		MessageBoxW(pdlg, msg, TextServiceDesc, MB_OK | MB_ICONERROR);
	}

	SetTaskbarListMarquee(pdlg, TBPF_NOPROGRESS);

	CoUninitialize();

	return;
}

INT_PTR CALLBACK DlgProcMakeSKKDic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hCancelEvent = nullptr;

	switch (message)
	{
	case WM_INITDIALOG:
		SendMessageW(GetDlgItem(hDlg, IDC_PROGRESS_DIC_MAKE), PBM_SETMARQUEE, TRUE, 0);
		hCancelEvent = CreateEventW(nullptr, FALSE, FALSE, cnfcanceldiceventname);
		_beginthread(MakeSKKDicThread, 0, hDlg);
		return TRUE;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		SetBkMode((HDC)wParam, TRANSPARENT);
		SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_ABORT_DIC_MAKE:
			SetEvent(hCancelEvent);
			return TRUE;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		CloseHandle(hCancelEvent);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

void MakeSKKDic(HWND hDlg)
{
	DialogBoxW(hInst, MAKEINTRESOURCE(IDD_DIALOG_SKK_DIC_MAKE), hDlg, DlgProcMakeSKKDic);
}
