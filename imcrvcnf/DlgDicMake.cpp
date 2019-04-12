
#include "eucjis2004.h"
#include "parseskkdic.h"
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

#define E_MAKESKKDIC_OK			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0)
#define E_MAKESKKDIC_DOWNLOAD	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 1)
#define E_MAKESKKDIC_FILEIO		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 2)
#define E_MAKESKKDIC_ENCODING	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 3)

BOOL IsMakeSKKDicCanceled(HANDLE hCancelEvent)
{
	return (WaitForSingleObject(hCancelEvent, 0) == WAIT_OBJECT_0);
}

HRESULT DownloadMakePath(LPCWSTR url, LPWSTR path, size_t len)
{
	WCHAR dir[MAX_PATH];
	WCHAR fname[MAX_PATH];

	DWORD temppathlen = GetTempPathW(_countof(dir), dir);
	if(temppathlen == 0 || temppathlen > _countof(dir))
	{
		return E_MAKESKKDIC_DOWNLOAD;
	}
	wcsncat_s(dir, TEXTSERVICE_NAME, _TRUNCATE);

	CreateDirectoryW(dir, nullptr);

	LPCWSTR fnurl = wcsrchr(url, L'/');
	if(fnurl == nullptr || *(fnurl + 1) == L'\0')
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
	if(FAILED(DownloadMakePath(url, path, len)))
	{
		return E_MAKESKKDIC_DOWNLOAD;
	}

	HINTERNET hInet = InternetOpenW(TEXTSERVICE_NAME L"/" TEXTSERVICE_VER, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if(hInet == nullptr)
	{
		return E_MAKESKKDIC_DOWNLOAD;
	}

	HINTERNET hUrl = InternetOpenUrlW(hInet, url, nullptr, 0, 0, 0);
	if(hUrl == nullptr)
	{
		InternetCloseHandle(hInet);

		return E_MAKESKKDIC_DOWNLOAD;
	}

	DWORD dwStatusCode = 0;
	DWORD dwQueryLength = sizeof(dwStatusCode);
	if(HttpQueryInfoW(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwQueryLength, 0) == FALSE)
	{
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInet);

		return E_MAKESKKDIC_DOWNLOAD;
	}

	switch(dwStatusCode)
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
	FILE *fp;

	_wfopen_s(&fp, path, WB);
	if(fp == nullptr)
	{
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInet);

		return E_MAKESKKDIC_FILEIO;
	}

	while(true)
	{
		if(IsMakeSKKDicCanceled(hCancelEvent))
		{
			fclose(fp);
			DeleteFileW(path);

			InternetCloseHandle(hUrl);
			InternetCloseHandle(hInet);

			return E_ABORT;
		}

		ZeroMemory(rbuf, sizeof(rbuf));
		retRead = InternetReadFile(hUrl, rbuf, sizeof(rbuf), &bytesRead);
		if(retRead)
		{
			if(bytesRead == 0)
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

HRESULT CheckMultiByteFile(HANDLE hCancelEvent, LPCWSTR path, int encoding)
{
	HRESULT hr = S_OK;
	FILE *fp;
	CHAR buf[READBUFSIZE * sizeof(WCHAR)];
	std::string strbuf;
	size_t len;

	_wfopen_s(&fp, path, RB);
	if(fp == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	while(fgets(buf, _countof(buf), fp) != nullptr)
	{
		if(IsMakeSKKDicCanceled(hCancelEvent))
		{
			fclose(fp);
			return E_ABORT;
		}

		strbuf += buf;

		if(!strbuf.empty() && strbuf.back() == '\n')
		{
			switch(encoding)
			{
			case 1: //EUC-JIS-2004
				if(!EucJis2004ToWideChar(strbuf.c_str(), nullptr, nullptr, &len))
				{
					hr = S_FALSE;
				}
				break;
			case 8: //UTF-8
				if(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
					strbuf.c_str(), -1, nullptr, 0) == 0)
				{
					hr = S_FALSE;
				}
				break;
			default:
				hr = S_FALSE;
				break;
			}

			if(hr == S_FALSE)
			{
				break;
			}

			strbuf.clear();
		}
	}

	fclose(fp);

	return hr;
}

HRESULT CheckWideCharFile(HANDLE hCancelEvent, LPCWSTR path)
{
	HRESULT hr = S_OK;
	FILE *fp;
	WCHAR wbuf[READBUFSIZE];
	std::wstring wstrbuf;

	_wfopen_s(&fp, path, RB);
	if(fp == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	while(fgetws(wbuf, _countof(wbuf), fp) != nullptr)
	{
		if(IsMakeSKKDicCanceled(hCancelEvent))
		{
			fclose(fp);
			return E_ABORT;
		}

		wstrbuf += wbuf;

		if(!wstrbuf.empty() && wstrbuf.back() == L'\n')
		{
			if(WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
				wstrbuf.c_str(), -1, nullptr, 0, nullptr, nullptr) == 0)
			{
				hr = S_FALSE;
				break;
			}

			wstrbuf.clear();
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

	if(!annotation.empty())
	{
		annotation_seps = seps + ParseConcat(annotation) + seps;
	}

	auto skkdic_itr = skkdic.find(key);
	if(skkdic_itr == skkdic.end())
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
			if(sc_itr->first == candidate)
			{
				annotation_esc = ParseConcat(sc_itr->second);
				if(annotation_esc.find(annotation_seps) == std::wstring::npos)
				{
					if(annotation_esc.empty())
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
		if(!exist)
		{
			skkdic_itr->second.push_back(std::make_pair(candidate, MakeConcat(annotation_seps)));
		}
	}
}

HRESULT LoadSKKDic(HANDLE hCancelEvent, HWND hDlg, SKKDIC &entries_a, SKKDIC &entries_n)
{
	WCHAR path[MAX_PATH];
	FILE *fp;
	std::wstring key;
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;
	WCHAR text[16] = {};

	HWND hWndListView = GetDlgItem(hDlg, IDC_LIST_SKK_DIC);
	int count = ListView_GetItemCount(hWndListView);

	for(int i = 0; i < count; i++)
	{
		ListView_SetItemText(hWndListView, i, 1, text);
		ListView_SetItemText(hWndListView, i, 2, text);
	}

	ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(hWndListView, 1, GetScaledSizeX(hDlg, 80));
	ListView_SetColumnWidth(hWndListView, 2, GetScaledSizeX(hDlg, 80));

	for(int i = 0; i < count; i++)
	{
		if(IsMakeSKKDicCanceled(hCancelEvent))
		{
			return E_ABORT;
		}

		BOOL check = ListView_GetCheckState(hWndListView, i);
		if(check == FALSE)
		{
			continue;
		}

		ListView_SetItemState(hWndListView, i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(hWndListView, i, FALSE);

		ListView_GetItemText(hWndListView, i, 0, path, _countof(path));

		//download
		URL_COMPONENTSW urlc = {};
		urlc.dwStructSize = sizeof(urlc);
		if(InternetCrackUrlW(path, 0, 0, &urlc))
		{
			switch(urlc.nScheme)
			{
			case INTERNET_SCHEME_HTTP:
			case INTERNET_SCHEME_HTTPS:
				{
					WCHAR url[INTERNET_MAX_URL_LENGTH];
					wcsncpy_s(url, path, _TRUNCATE);

					HRESULT hrd = DownloadSKKDic(hCancelEvent, url, path, _countof(path));
					if(FAILED(hrd))
					{
						return hrd;
					}
				}
				break;
			default:
				break;
			}
		}

		int encoding = 0;

		//check BOM
		_wfopen_s(&fp, path, RB);
		if(fp == nullptr)
		{
			return E_MAKESKKDIC_FILEIO;
		}
		WCHAR bom = L'\0';
		fread(&bom, 2, 1, fp);
		fclose(fp);
		if(bom == BOM)
		{
			//UTF-16LE
			encoding = 16;

			HRESULT hr = CheckWideCharFile(hCancelEvent, path);
			switch(hr)
			{
			case S_OK:
				break;
			case E_ABORT:
			case E_MAKESKKDIC_FILEIO:
				return hr;
				break;
			default:
				//Error
				encoding = -1;
				break;
			}
		}

		//UTF-8 ?
		if(encoding == 0)
		{
			HRESULT hr = CheckMultiByteFile(hCancelEvent, path, 8);
			switch(hr)
			{
			case S_OK:
				encoding = 8;
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
		if(encoding == 0)
		{
			HRESULT hr = CheckMultiByteFile(hCancelEvent, path, 1);
			switch(hr)
			{
			case S_OK:
				encoding = 1;
				break;
			case E_ABORT:
			case E_MAKESKKDIC_FILEIO:
				return hr;
				break;
			default:
				break;
			}
		}

		switch(encoding)
		{
		case 1:
			//EUC-JIS-2004
			bom = L'\0';
			_wfopen_s(&fp, path, RB);
			break;
		case 8:
			//UTF-8
			bom = BOM;
			_wfopen_s(&fp, path, RccsUTF8);
			break;
		case 16:
			//UTF-16LE
			_wfopen_s(&fp, path, RccsUTF16);
			break;
		default:
			return E_MAKESKKDIC_ENCODING;
			break;
		}
		if(fp == nullptr)
		{
			return E_MAKESKKDIC_FILEIO;
		}

		// 「;; okuri-ari entries.」、「;; okuri-nasi entries.」がない辞書のエントリは送りなしとする
		int okuri = 0;	//-1:header / 1:okuri-ari entries. / 0:okuri-nasi entries.

		size_t count_key = 0;
		size_t count_cand = 0;

		while(true)
		{
			if(IsMakeSKKDicCanceled(hCancelEvent))
			{
				fclose(fp);
				return E_ABORT;
			}

			int rl = ReadSKKDicLine(fp, bom, okuri, key, sc, so);
			if(rl == -1)
			{
				//EOF
				break;
			}
			else if(rl == 1)
			{
				//comment
				continue;
			}

			switch(okuri)
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
				if(IsMakeSKKDicCanceled(hCancelEvent))
				{
					fclose(fp);
					return E_ABORT;
				}

				LoadSKKDicAdd((okuri == 0 ? entries_n : entries_a), key, sc_itr->first, sc_itr->second);
			}
		}

		fclose(fp);

		{
			WCHAR scount[16];
			LV_ITEM lvi = {};
			lvi.mask = LVFIF_TEXT;
			lvi.iItem = i;

			_snwprintf_s(scount, _TRUNCATE, L"%Iu", count_key);
			ListView_SetItemText(hWndListView, i, 1, scount);
			_snwprintf_s(scount, _TRUNCATE, L"%Iu", count_cand);
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
		if(sc_itr->second.size() > 2)
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
	FILE *fp;
	WCHAR bom = BOM;
	LPCWSTR crlf = L"\r\n";

	_wfopen_s(&fp, pathskkdic, WB);
	if(fp == nullptr)
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
		if(IsMakeSKKDicCanceled(hCancelEvent))
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
		if(IsMakeSKKDicCanceled(hCancelEvent))
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

void MakeSKKDicThread(void *p)
{
	WCHAR msg[1024];
	SKKDIC entries_a, entries_n;

	HWND child = (HWND)p;
	HWND parent = PROPSHEET_IDTOHWND(GetWindow(child, GW_OWNER), IDD_DIALOG_DICTIONARY);

	HANDLE hCancelEvent = OpenEventW(SYNCHRONIZE, FALSE, cnfcanceldiceventname);

	LONGLONG t = 0;

	LARGE_INTEGER qpf = {};
	BOOL bHRT = QueryPerformanceFrequency(&qpf);

	LONGLONG t0 = 0;
	if(bHRT)
	{
		LARGE_INTEGER qpt = {};
		QueryPerformanceCounter(&qpt);
		t0 = qpt.QuadPart;
	}
	else
	{
		t0 = (LONGLONG)GetTickCount64();
	}

	HRESULT hr = LoadSKKDic(hCancelEvent, parent, entries_a, entries_n);
	if(SUCCEEDED(hr))
	{
		hr = WriteSKKDic(hCancelEvent, entries_a, entries_n);
	}

	if(bHRT)
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

	if(SUCCEEDED(hr))
	{
		_snwprintf_s(msg, _TRUNCATE, L"完了しました。\r\n\r\n%lld msec", t);

		MessageBoxW(parent, msg, TextServiceDesc, MB_OK | MB_ICONINFORMATION);
	}
	else if(hr == E_ABORT)
	{
		_wremove(pathskkdic);

		MessageBoxW(parent, L"中断しました。", TextServiceDesc, MB_OK | MB_ICONWARNING);
	}
	else
	{
		WCHAR errmsg[32] = {};

		switch(hr)
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
		default:
			break;
		}

		if(HRESULT_SEVERITY(hr) == SEVERITY_ERROR &&
			HRESULT_FACILITY(hr) == FACILITY_HTTP)
		{
			_snwprintf_s(errmsg, _TRUNCATE, L"HTTP %d", HRESULT_CODE(hr));
		}

		WCHAR path[MAX_PATH] = {};
		HWND hWndListView = GetDlgItem(parent, IDC_LIST_SKK_DIC);
		int count = ListView_GetItemCount(hWndListView);
		for(int i = 0; i < count; i++)
		{
			if((ListView_GetItemState(hWndListView, i, LVIS_SELECTED) & LVIS_SELECTED) != 0)
			{
				ListView_GetItemText(hWndListView, i, 0, path, _countof(path));
				break;
			}
		}

		_snwprintf_s(msg, _TRUNCATE, L"失敗しました。(%s)\n\n%s", errmsg, path);

		MessageBoxW(parent, msg, TextServiceDesc, MB_OK | MB_ICONERROR);
	}
	return;
}

INT_PTR CALLBACK DlgProcMakeSKKDic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hCancelEvent = nullptr;

	switch(message)
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
		switch(LOWORD(wParam))
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
