
#include "eucjis2004.h"
#include "parseskkdic.h"
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

//見出し語位置
typedef std::map< std::wstring, long > KEYPOS; //見出し語、ファイル位置

struct {
	HWND parent;
	HWND child;
	HRESULT hr;
	BOOL cancel;
	int error;
	WCHAR path[MAX_PATH];
} SkkDicInfo;

#define SKKDIC_OK       0
#define SKKDIC_DOWNLOAD 1
#define SKKDIC_FILEIO   2
#define SKKDIC_ENCODING 3

LPCWSTR SkkDicErrorMsg[] =
{
	L"",
	L"ダウンロード",
	L"ファイル入出力",
	L"文字コード"
};

void LoadDictionary(HWND hwnd)
{
	APPDATAXMLLIST list;
	LVITEMW item;

	HRESULT hr = ReadList(pathconfigxml, SectionDictionary, list);

	if(hr == S_OK && list.size() != 0)
	{
		HWND hWndListView = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
		int i = 0;
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(l_itr->size() == 0 || (*l_itr)[0].first != AttributePath)
			{
				continue;
			}
			item.mask = LVIF_TEXT;
			item.pszText = (LPWSTR)(*l_itr)[0].second.c_str();
			item.iItem = i;
			item.iSubItem = 0;
			ListView_InsertItem(hWndListView, &item);

			BOOL check = TRUE;
			if(l_itr->size() >= 2 && (*l_itr)[1].first == AttributeEnabled)
			{
				check = _wtoi((*l_itr)[1].second.c_str());
			}
			ListView_SetCheckState(hWndListView, i, check);

			i++;
		}
		ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
	}
}

void SaveDictionary(HWND hwnd)
{
	APPDATAXMLLIST list;
	APPDATAXMLROW row;
	APPDATAXMLATTR attr;
	WCHAR path[MAX_PATH];

	HWND hWndListView = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	int count = ListView_GetItemCount(hWndListView);

	for(int i = 0; i < count; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, path, _countof(path));

		BOOL check = ListView_GetCheckState(hWndListView, i);

		attr.first = AttributePath;
		attr.second = path;
		row.push_back(attr);

		attr.first = AttributeEnabled;
		attr.second = (check ? L"1" : L"0");
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pXmlWriter, list);
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
		skkdicentry.second.push_back(SKKDICCANDIDATE(candidate, annotation_seps));
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
			skkdic_itr->second.push_back(SKKDICCANDIDATE(candidate, MakeConcat(annotation_seps)));
		}
	}
}

HRESULT DownloadDic(LPCWSTR url, LPWSTR path, size_t len)
{
	HRESULT hr = E_FAIL;
	HINTERNET hInet;
	HINTERNET hUrl;
	CHAR rbuf[RECVBUFSIZE];
	BOOL retRead;
	DWORD bytesRead = 0;
	WCHAR dir[MAX_PATH];
	WCHAR fname[MAX_PATH];
	FILE *fp;

	DWORD temppathlen = GetTempPathW(_countof(dir), dir);
	if(temppathlen == 0 || temppathlen > _countof(dir))
	{
		return E_FAIL;
	}
	wcsncat_s(dir, TEXTSERVICE_NAME, _TRUNCATE);

	CreateDirectoryW(dir, NULL);

	LPCWSTR fnurl = wcsrchr(url, L'/');
	if(fnurl == NULL || *(fnurl + 1) == L'\0')
	{
		return E_FAIL;
	}
	wcsncpy_s(fname, fnurl + 1, _TRUNCATE);

	LPWSTR pfname = fname;
	while((pfname = wcspbrk(pfname, L"\\/:*?\"<>|")) != NULL)
	{
		*pfname = L'_';
	}

	_snwprintf_s(path, len, _TRUNCATE, L"%s\\%s", dir, fname);

	hInet = InternetOpenW(TEXTSERVICE_NAME L"/" TEXTSERVICE_VER, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(hInet == NULL)
	{
		return E_FAIL;
	}

	hUrl = InternetOpenUrlW(hInet, url, NULL, 0, 0, 0);
	if(hUrl == NULL)
	{
		goto exit_u;
	}

	_wfopen_s(&fp, path, WB);
	if(fp == NULL)
	{
		goto exit_f;
	}

	while(true)
	{
		if(SkkDicInfo.cancel)
		{
			hr = E_ABORT;
			goto exit;
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
			goto exit;
		}

		fwrite(rbuf, bytesRead, 1, fp);
	}

	hr = S_OK;

exit:
	fclose(fp);
exit_f:
	InternetCloseHandle(hUrl);
exit_u:
	InternetCloseHandle(hInet);

	return hr;
}

BOOL CheckMultiByteFile(LPCWSTR path, int encoding)
{
	BOOL bRet = TRUE;
	FILE *fp;
	CHAR buf[READBUFSIZE * sizeof(WCHAR)];
	std::string strbuf;
	size_t len;

	_wfopen_s(&fp, path, RB);
	if(fp == NULL)
	{
		return FALSE;
	}

	while(fgets(buf, _countof(buf), fp) != NULL)
	{
		strbuf += buf;

		if(!strbuf.empty() && strbuf.back() == '\n')
		{
			switch(encoding)
			{
			case 1: //EUC-JIS-2004
				if(!EucJis2004ToWideChar(strbuf.c_str(), NULL, NULL, &len))
				{
					bRet = FALSE;
				}
				break;
			case 8: //UTF-8
				if(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
					strbuf.c_str(), -1, NULL, 0) == 0)
				{
					bRet = FALSE;
				}
				break;
			default:
				bRet = FALSE;
				break;
			}

			if(bRet == FALSE)
			{
				break;
			}

			strbuf.clear();
		}
	}

	fclose(fp);

	return bRet;
}

BOOL CheckWideCharFile(LPCWSTR path)
{
	BOOL bRet = TRUE;
	FILE *fp;
	WCHAR wbuf[READBUFSIZE];
	std::wstring wstrbuf;

	_wfopen_s(&fp, path, RB);
	if(fp == NULL)
	{
		return FALSE;
	}

	while(fgetws(wbuf, _countof(wbuf), fp) != NULL)
	{
		wstrbuf += wbuf;

		if(!wstrbuf.empty() && wstrbuf.back() == L'\n')
		{
			if(WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
				wstrbuf.c_str(), -1, NULL, 0, NULL, NULL) == 0)
			{
				bRet = FALSE;
				break;
			}

			wstrbuf.clear();
		}
	}

	fclose(fp);

	return bRet;
}

HRESULT LoadSKKDic(HWND hwnd, SKKDIC &entries_a, SKKDIC &entries_n)
{
	WCHAR path[MAX_PATH];
	WCHAR url[INTERNET_MAX_URL_LENGTH];
	URL_COMPONENTSW urlc;
	FILE *fp;
	std::wstring key;
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;

	HWND hWndListView = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	int count = ListView_GetItemCount(hWndListView);

	for(int i = 0; i < count; i++)
	{
		if(SkkDicInfo.cancel)
		{
			return E_ABORT;
		}

		BOOL check = ListView_GetCheckState(hWndListView, i);
		if(check == FALSE)
		{
			continue;
		}

		ListView_GetItemText(hWndListView, i, 0, path, _countof(path));

		//download
		ZeroMemory(&urlc, sizeof(urlc));
		urlc.dwStructSize = sizeof(urlc);
		if(InternetCrackUrlW(path, 0, 0, &urlc))
		{
			switch(urlc.nScheme)
			{
			case INTERNET_SCHEME_HTTP:
			case INTERNET_SCHEME_HTTPS:
				{
					wcsncpy_s(url, path, _TRUNCATE);

					HRESULT hrd = DownloadDic(url, path, _countof(path));
					if(hrd != S_OK)
					{
						SkkDicInfo.error = SKKDIC_DOWNLOAD;
						_snwprintf_s(SkkDicInfo.path, _TRUNCATE, L"%s", url);
						return hrd;
					}
				}
				break;
			default:
				break;
			}
		}

		SkkDicInfo.error = SKKDIC_FILEIO;
		wcscpy_s(SkkDicInfo.path, path);
		int encoding = 0;

		//check BOM
		_wfopen_s(&fp, path, RB);
		if(fp == NULL)
		{
			return E_FAIL;
		}
		WCHAR bom = L'\0';
		fread(&bom, 2, 1, fp);
		fclose(fp);
		if(bom == BOM)
		{
			//UTF-16LE
			encoding = 16;

			if(!CheckWideCharFile(path))
			{
				//Error
				encoding = -1;
			}
		}

		//UTF-8 ?
		if(encoding == 0)
		{
			if(CheckMultiByteFile(path, 8))
			{
				encoding = 8;
			}
		}

		//EUC-JIS-2004 ?
		if(encoding == 0)
		{
			if(CheckMultiByteFile(path, 1))
			{
				encoding = 1;
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
			SkkDicInfo.error = SKKDIC_ENCODING;
			fp = NULL;
			break;
		}
		if(fp == NULL)
		{
			return E_FAIL;
		}

		int okuri = -1;	//-1:header / 1:okuri-ari entries. / 0:okuri-nasi entries.

		while(true)
		{
			if(SkkDicInfo.cancel)
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

			FORWARD_ITERATION_I(sc_itr, sc)
			{
				if(SkkDicInfo.cancel)
				{
					fclose(fp);
					return E_ABORT;
				}

				LoadSKKDicAdd((okuri == 0 ? entries_n : entries_a), key, sc_itr->first, sc_itr->second);
			}
		}

		fclose(fp);
	}

	SkkDicInfo.error = SKKDIC_OK;

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

HRESULT WriteSKKDic(const SKKDIC &entries_a, const SKKDIC &entries_n)
{
	FILE *fp;
	long pos;
	KEYPOS keypos;

	_wfopen_s(&fp, pathskkdic, WB);
	if(fp == NULL)
	{
		SkkDicInfo.error = SKKDIC_FILEIO;
		wcscpy_s(SkkDicInfo.path, pathskkdic);
		return E_FAIL;
	}

	//BOM
	fwrite("\xFF\xFE", 2, 1, fp);

	//送りありエントリ
	fwrite(EntriesAri, wcslen(EntriesAri) * sizeof(WCHAR), 1, fp);
	fseek(fp, -2, SEEK_CUR);
	fwrite(L"\r\n", 4, 1, fp);

	REVERSE_ITERATION_I(entries_ritr, entries_a)
	{
		if(SkkDicInfo.cancel)
		{
			fclose(fp);
			return E_ABORT;
		}

		pos = ftell(fp);
		keypos.insert(KEYPOS::value_type(entries_ritr->first, pos));

		WriteSKKDicEntry(fp, entries_ritr->first, entries_ritr->second);
	}

	//送りなしエントリ
	fwrite(EntriesNasi, wcslen(EntriesNasi) * sizeof(WCHAR), 1, fp);
	fseek(fp, -2, SEEK_CUR);
	fwrite(L"\r\n", 4, 1, fp);

	FORWARD_ITERATION_I(entries_itr, entries_n)
	{
		if(SkkDicInfo.cancel)
		{
			fclose(fp);
			return E_ABORT;
		}

		pos = ftell(fp);
		keypos.insert(KEYPOS::value_type(entries_itr->first, pos));

		WriteSKKDicEntry(fp, entries_itr->first, entries_itr->second);
	}

	fclose(fp);

	//インデックスファイル
	_wfopen_s(&fp, pathskkidx, WB);
	if(fp == NULL)
	{
		SkkDicInfo.error = SKKDIC_FILEIO;
		wcscpy_s(SkkDicInfo.path, pathskkidx);
		return E_FAIL;
	}

	FORWARD_ITERATION_I(keypos_itr, keypos)
	{
		if(SkkDicInfo.cancel)
		{
			fclose(fp);
			return E_ABORT;
		}

		fwrite(&keypos_itr->second, sizeof(keypos_itr->second), 1, fp);
	}

	fclose(fp);

	SkkDicInfo.error = SKKDIC_OK;

	return S_OK;
}

unsigned int __stdcall MakeSKKDicThread(void *p)
{
	SKKDIC entries_a, entries_n;

	SkkDicInfo.hr = LoadSKKDic(SkkDicInfo.parent, entries_a, entries_n);
	if(SkkDicInfo.hr == S_OK)
	{
		SkkDicInfo.hr = WriteSKKDic(entries_a, entries_n);
	}

	return 0;
}

void MakeSKKDicWaitThread(void *p)
{
	WCHAR msg[512];
	HANDLE hThread;

	hThread = (HANDLE)_beginthreadex(NULL, 0, MakeSKKDicThread, NULL, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	EndDialog(SkkDicInfo.child, TRUE);

	if(SkkDicInfo.hr == S_OK)
	{
		MessageBoxW(SkkDicInfo.parent, L"完了しました。", TextServiceDesc, MB_OK | MB_ICONINFORMATION);
	}
	else if(SkkDicInfo.hr == E_ABORT)
	{
		_wremove(pathskkidx);
		_wremove(pathskkdic);

		MessageBoxW(SkkDicInfo.parent, L"中断しました。", TextServiceDesc, MB_OK | MB_ICONWARNING);
	}
	else
	{
		_wremove(pathskkidx);
		_wremove(pathskkdic);

		LPCWSTR errmsg = L"";
		switch(SkkDicInfo.error)
		{
		case SKKDIC_DOWNLOAD:
		case SKKDIC_FILEIO:
		case SKKDIC_ENCODING:
			errmsg = SkkDicErrorMsg[SkkDicInfo.error];
			break;
		default:
			break;
		}
		_snwprintf_s(msg, _TRUNCATE, L"失敗しました。(%s)\n\n%s", errmsg, SkkDicInfo.path);
		MessageBoxW(SkkDicInfo.parent, msg, TextServiceDesc, MB_OK | MB_ICONERROR);
	}
	return;
}

INT_PTR CALLBACK DlgProcSKKDic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		SkkDicInfo.cancel = FALSE;
		SkkDicInfo.hr = S_OK;
		SkkDicInfo.child = hDlg;
		SkkDicInfo.path[0] = L'\0';
		_beginthread(MakeSKKDicWaitThread, 0, NULL);
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_DIC_MAKE), PBM_SETMARQUEE, TRUE, 0);
		return TRUE;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		SetBkMode((HDC)wParam, TRANSPARENT);
		SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	case WM_COMMAND:
		if(LOWORD(wParam) == IDC_BUTTON_ABORT_DIC_MAKE)
		{
			SkkDicInfo.cancel = TRUE;
		}
		break;
	default:
		break;
	}
	return FALSE;
}

void MakeSKKDic(HWND hwnd)
{
	SkkDicInfo.parent = hwnd;
	DialogBoxW(hInst, MAKEINTRESOURCE(IDD_DIALOG_SKK_DIC_MAKE), hwnd, DlgProcSKKDic);
}
