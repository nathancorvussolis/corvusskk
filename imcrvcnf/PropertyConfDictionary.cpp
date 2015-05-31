
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
	WCHAR path[MAX_PATH];
} SkkDicInfo;

void LoadDictionary(HWND hwnd)
{
	BOOL check;
	HWND hWndListView;
	int i = 0;
	LVITEMW item;
	APPDATAXMLLIST list;

	if(ReadList(pathconfigxml, SectionDictionary, list) == S_OK && list.size() != 0)
	{
		hWndListView = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
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

			check = TRUE;
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
	WCHAR path[MAX_PATH];
	BOOL check;
	HWND hWndListView;
	int i, count;
	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;

	hWndListView = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	count = ListView_GetItemCount(hWndListView);

	for(i = 0; i < count; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, path, _countof(path));

		check = ListView_GetCheckState(hWndListView, i);

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
	HINTERNET hInet;
	HINTERNET hUrl;
	CHAR rbuf[RECVBUFSIZE];
	BOOL retRead;
	DWORD bytesRead = 0;
	DWORD dwTimeout = 1000;
	FILE *fp;
	WCHAR dir[MAX_PATH];
	std::wstring strurl;

	DWORD temppathlen = GetTempPathW(_countof(dir), dir);
	if(temppathlen == 0 || temppathlen > _countof(dir))
	{
		return E_FAIL;
	}

	wcsncat_s(dir, TEXTSERVICE_NAME, _TRUNCATE);
	_wmkdir(dir);

	strurl.assign(url);
	strurl = std::regex_replace(strurl, std::wregex(L".+/"), std::wstring(L""));
	if(strurl.empty())
	{
		return E_FAIL;
	}
	strurl = std::regex_replace(strurl, std::wregex(L"[\\\\/:*?\"<>|]"), std::wstring(L"_"));
	_snwprintf_s(path, len, _TRUNCATE, L"%s\\%s", dir, strurl.c_str());

	hInet = InternetOpenW(TEXTSERVICE_NAME L"/" TEXTSERVICE_VER, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(hInet != NULL)
	{
		InternetSetOptionW(hInet, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeout, sizeof(dwTimeout));
		InternetSetOptionW(hInet, INTERNET_OPTION_DATA_SEND_TIMEOUT, &dwTimeout, sizeof(dwTimeout));
		InternetSetOptionW(hInet, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, &dwTimeout, sizeof(dwTimeout));

		hUrl = InternetOpenUrlW(hInet, url, NULL, 0, 0, 0);
		if(hUrl != NULL)
		{
			_wfopen_s(&fp, path, WB);
			if(fp == NULL)
			{
				return E_FAIL;
			}

			while(true)
			{
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
					InternetCloseHandle(hUrl);
					InternetCloseHandle(hInet);
					fclose(fp);
					return E_FAIL;
				}

				fwrite(rbuf, bytesRead, 1, fp);
			}

			InternetCloseHandle(hUrl);
			fclose(fp);
		}
		else
		{
			InternetCloseHandle(hInet);
			return E_FAIL;
		}

		InternetCloseHandle(hInet);
	}
	else
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT LoadSKKDic(HWND hwnd, SKKDIC &entries_a, SKKDIC &entries_n)
{
	HWND hWndListView;
	BOOL check;
	WCHAR path[MAX_PATH];
	WCHAR url[INTERNET_MAX_URL_LENGTH];
	size_t i, count;
	FILE *fp;
	int encode;
	WCHAR bom;
	BYTE rbom[3];
	CHAR buf[READBUFSIZE * sizeof(WCHAR)];
	std::string strbuf;
	std::wstring key;
	int okuri;
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;
	int rl;

	hWndListView = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	count = ListView_GetItemCount(hWndListView);

	for(i = 0; i < count; i++)
	{
		if(SkkDicInfo.cancel)
		{
			return E_ABORT;
		}

		check = ListView_GetCheckState(hWndListView, i);
		if(check == FALSE)
		{
			continue;
		}

		ListView_GetItemText(hWndListView, i, 0, path, _countof(path));

		//download
		if(std::regex_match(std::wstring(path), std::wregex(L"(ftp|http|https)://.+")))
		{
			ListView_GetItemText(hWndListView, i, 0, url, _countof(url));

			HRESULT hrd = DownloadDic(url, path, _countof(path));
			if(hrd != S_OK)
			{
				_snwprintf_s(SkkDicInfo.path, _TRUNCATE, L"%s", url);
				return E_FAIL;
			}
		}

		encode = 0;

		//check BOM
		_wfopen_s(&fp, path, RB);
		if(fp == NULL)
		{
			wcscpy_s(SkkDicInfo.path, path);
			return E_FAIL;
		}
		ZeroMemory(rbom, sizeof(rbom));
		fread(rbom, 3, 1, fp);
		if(rbom[0] == 0xFF && rbom[1] == 0xFE)
		{
			//UTF-16LE
			encode = 16;
		}
		else if(rbom[0] == 0xEF && rbom[1] == 0xBB && rbom[2] == 0xBF)
		{
			//UTF-8
			encode = 8;
		}
		fclose(fp);

		//UTF-8 ?
		if(encode == 0)
		{
			strbuf.clear();

			_wfopen_s(&fp, path, RB);
			if(fp == NULL)
			{
				wcscpy_s(SkkDicInfo.path, path);
				return E_FAIL;
			}
			while(fgets(buf, _countof(buf), fp) != NULL)
			{
				strbuf += buf;
			}
			fclose(fp);

			if(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, strbuf.c_str(), -1, NULL, 0) != 0)
			{
				encode = 8;
			}
		}

		switch(encode)
		{
		case 8:
			//UTF-8
			bom = 0xFEFF;
			_wfopen_s(&fp, path, RccsUTF8);
			break;
		case 16:
			//UTF-16LE
			bom = 0xFEFF;
			_wfopen_s(&fp, path, RccsUTF16);
			break;
		default:
			//EUC-JIS-2004
			bom = L'\0';
			_wfopen_s(&fp, path, RB);
			break;
		}
		if(fp == NULL)
		{
			wcscpy_s(SkkDicInfo.path, path);
			return E_FAIL;
		}

		okuri = -1;

		while(true)
		{
			if(SkkDicInfo.cancel)
			{
				fclose(fp);
				return E_ABORT;
			}

			rl = ReadSKKDicLine(fp, bom, okuri, key, sc, so);
			if(rl == -1)
			{
				break;
			}
			else if(rl == 1)
			{
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
		wcscpy_s(SkkDicInfo.path, pathskkdic);
		return E_FAIL;
	}

	//BOM
	fwrite("\xFF\xFE", 2, 1, fp);

	//送りありエントリ
	fwrite(EntriesAri, wcslen(EntriesAri) * sizeof(WCHAR), 1, fp);
	fseek(fp, -2, SEEK_END);
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
	fseek(fp, -2, SEEK_END);
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

		_snwprintf_s(msg, _TRUNCATE, L"失敗しました。\n\n%s", SkkDicInfo.path);
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
