
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"
#include "eucjis2004.h"

#define BUFSIZE 0x2000

typedef std::pair<std::wstring, std::wstring> ENTRY;
typedef std::map<std::wstring, std::wstring> ENTRYS;

struct {
	HWND parent;
	HWND child;
	HRESULT hr;
} SkkDicInfo;

struct {
	HWND parent;
	HWND child;
	HRESULT hr;
	WCHAR command;
	WCHAR path[MAX_PATH];
} SkkUserDicInfo;

void LoadDictionary(HWND hwnd)
{
	HWND hWndList;
	int i = 0;
	LVITEMW item;
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;

	if(ReadList(pathconfigxml, SectionDictionary, list) == S_OK)
	{
		hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
		for(l_itr = list.begin(); l_itr != list.end(); l_itr++)
		{
			if(l_itr->size() == 0 || (*l_itr)[0].first != AttributePath)
			{
				continue;
			}
			item.mask = LVIF_TEXT;
			item.pszText = (LPWSTR)(*l_itr)[0].second.c_str();
			item.iItem = i;
			item.iSubItem = 0;
			ListView_InsertItem(hWndList, &item);
			i++;
		}
		ListView_SetColumnWidth(hWndList, 0, LVSCW_AUTOSIZE);
	}
}

void SaveDictionary(HWND hwnd)
{
	WCHAR path[MAX_PATH];
	HWND hWndList;
	int i, count;
	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;

	hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	count = ListView_GetItemCount(hWndList);

	for(i=0; i<count; i++)
	{
		ListView_GetItemText(hWndList, i, 0, path, _countof(path));

		attr.first = AttributePath;
		attr.second = path;
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pXmlWriter, list);
}

void LoadSKKDic(HWND hwnd, ENTRYS &entrys)
{
	HWND hWndList;
	WCHAR path[MAX_PATH];
	size_t count, size, i, is;
	FILE *fpskkdic;
	WCHAR bom;
	CHAR buf[BUFSIZE*2];
	WCHAR wbuf[BUFSIZE];
	void *rp;
	std::wstring s;
	std::wregex re;
	std::wstring fmt;
	ENTRY entry;
	ENTRYS::iterator entrys_itr;

	hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	count = ListView_GetItemCount(hWndList);

	for(i=0; i<count; i++)
	{
		ListView_GetItemText(hWndList, i, 0, path, _countof(path));
		_wfopen_s(&fpskkdic, path, L"rb");
		if(fpskkdic == NULL)
		{
			continue;
		}

		bom = L'\0';
		fread(&bom, 2, 1, fpskkdic);
		if(bom == 0xBBEF)
		{
			bom = L'\0';
			fread(&bom, 2, 1, fpskkdic);
			if((bom & 0xFF) == 0xBF)
			{
				bom = 0xFEFF;
			}
		}

		switch(bom)
		{
		case 0xFEFF:
			fclose(fpskkdic);
			_wfopen_s(&fpskkdic, path, RccsUNICODE);
			if(fpskkdic == NULL)
			{
				continue;
			}
			break;
		default:
			fseek(fpskkdic, SEEK_SET, 0);
			break;
		}

		while(true)
		{
			switch(bom)
			{
			case 0xFEFF:
				rp = fgetws(wbuf, _countof(wbuf), fpskkdic);
				break;
			default:
				rp = fgets(buf, _countof(buf), fpskkdic);
				break;
			}
			if(rp == NULL)
			{
				break;
			}

			switch(bom)
			{
			case 0xFEFF:
				break;
			default:
				size = _countof(wbuf);
				if(!EucJis2004ToWideChar(buf, NULL, wbuf, &size))
				{
					continue;
				}
				break;
			}

			switch(wbuf[0])
			{
			case L'\0':
			case L'\r':
			case L'\n':
			case L';':
			case L'\x20':
				continue;
				break;
			default:
				break;
			}

			s.assign(wbuf);
			re.assign(L"\\t|\\r|\\n");
			fmt.assign(L"");
			s = std::regex_replace(s, re, fmt);
			is = s.find_first_of(L'\x20');
			if(is == std::wstring::npos)
			{
				continue;
			}

			entry.first = s.substr(0, is);
			entry.second = s.substr(is + 1);
			entrys_itr = entrys.find(entry.first);
			if(entrys_itr == entrys.end())
			{
				entrys.insert(entry);
			}
			else
			{
				entrys_itr->second += entry.second.substr(1);
			}
		}

		fclose(fpskkdic);
	}
}

HRESULT WriteSKKDicXml(ENTRYS &entrys)
{
	HRESULT hr;
	FILE *fpidx;
	IXmlWriter *pWriter;
	IStream *pFileStream;
	ENTRYS::iterator entrys_itr;
	std::vector<std::wstring> es;
	size_t i, is, ie;
	std::wstring s;
	std::wregex re;
	std::wstring fmt;
	APPDATAXMLATTR attr;
	APPDATAXMLROW::iterator r_itr;
	APPDATAXMLROW row;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLLIST list;
	ULARGE_INTEGER uli1;
	LARGE_INTEGER li0 = {0};

	_wfopen_s(&fpidx, pathskkcvdicidx, L"wb");

	hr = WriterInit(pathskkcvdicxml, &pWriter, &pFileStream, FALSE);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterStartElement(pWriter, TagRoot);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterStartSection(pWriter, SectionDictionary);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	for(entrys_itr = entrys.begin(); entrys_itr != entrys.end(); entrys_itr++)
	{
		//エントリを「/」で分割
		es.clear();
		i = 0;
		s = entrys_itr->second;
		while(i < s.size())
		{
			is = s.find_first_of(L'/', i);
			ie = s.find_first_of(L'/', is + 1);
			if(ie == std::wstring::npos)
			{
				break;
			}
			es.push_back(s.substr(i + 1, ie - is - 1));
			i = ie;
		}

		//候補と注釈を分割
		list.clear();
		for(i=0; i<es.size(); i++)
		{
			row.clear();
			s = es[i];
			ie = s.find_first_of(L';');

			if(ie == std::wstring::npos)
			{
				attr.first = AttributeCandidate;
				attr.second = s;
				row.push_back(attr);
				attr.first = AttributeAnnotation;
				attr.second = L"";
				row.push_back(attr);
			}
			else
			{
				attr.first = AttributeCandidate;
				attr.second = s.substr(0, ie);
				row.push_back(attr);
				attr.first = AttributeAnnotation;
				attr.second = s.substr(ie + 1);
				row.push_back(attr);
			}

			list.push_back(row);
		}

		//concatを置換
		for(l_itr = list.begin(); l_itr != list.end(); l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				s = r_itr->second;
				re.assign(L".*\\(concat \".*(\\\\057|\\\\073).*\"\\).*");
				if(std::regex_match(s, re))
				{
					re.assign(L"(.*)\\(concat \"(.*)\"\\)(.*)");
					fmt.assign(L"$1$2$3");
					s = std::regex_replace(s, re, fmt);

					re.assign(L"\\\\057");
					fmt.assign(L"/");
					s = std::regex_replace(s, re, fmt);

					re.assign(L"\\\\073");
					fmt.assign(L";");
					s = std::regex_replace(s, re, fmt);

					r_itr->second = s;
				}
			}
		}

		//インデックスファイル書き込み
		if(fpidx != NULL)
		{
			hr = pWriter->Flush();
			EXIT_NOT_S_OK(hr);

			hr = pFileStream->Seek(li0, STREAM_SEEK_CUR, &uli1);
			EXIT_NOT_S_OK(hr);

			fwrite(&uli1.QuadPart, sizeof(uli1.QuadPart), 1, fpidx);
		}

		hr = WriterStartElement(pWriter, TagEntry);
		EXIT_NOT_S_OK(hr);

		hr = WriterAttribute(pWriter, TagKey, entrys_itr->first.c_str());	//見出し
		EXIT_NOT_S_OK(hr);
	
		hr = WriterList(pWriter, list);	//候補
		EXIT_NOT_S_OK(hr);

		hr = WriterEndElement(pWriter);	//TagEntry
		EXIT_NOT_S_OK(hr);

		hr = WriterNewLine(pWriter);
		EXIT_NOT_S_OK(hr);
	}

	hr = WriterEndSection(pWriter);	//SectionDictionary
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterEndElement(pWriter);	//TagRoot
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

NOT_S_OK:
	hr = WriterFinal(&pWriter, &pFileStream);

	if(fpidx != NULL)
	{
		fclose(fpidx);
	}

	return hr;
}

unsigned int __stdcall MakeSKKDicThread(void *p)
{
	ENTRYS entrys;

	LoadSKKDic(SkkDicInfo.parent, entrys);
	SkkDicInfo.hr = WriteSKKDicXml(entrys);
	return 0;
}

void MakeSKKDicWaitThread(void *p)
{
	WCHAR num[32];
	HANDLE hThread;
	
	hThread = (HANDLE)_beginthreadex(NULL, 0, MakeSKKDicThread, NULL, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	EndDialog(SkkDicInfo.child, TRUE);

	if(SkkDicInfo.hr == S_OK)
	{
		MessageBoxW(SkkDicInfo.parent, L"完了しました。", TextServiceDesc, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		_snwprintf_s(num, _countof(num), L"失敗しました。0x%08X", SkkDicInfo.hr);
		MessageBoxW(SkkDicInfo.parent, num, TextServiceDesc, MB_OK | MB_ICONERROR);
	}
	return;
}

INT_PTR CALLBACK DlgProcSKKDic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPCWSTR pw[] = {L"／",L"─",L"＼",L"│"};
	static int ipw = 0;

	switch(message)
	{
	case WM_INITDIALOG:
		SkkDicInfo.child = hDlg;
		_beginthread(MakeSKKDicWaitThread, 0, NULL);
		SetTimer(hDlg, IDC_STATIC_DIC_PW, 1000, NULL);
		return (INT_PTR)TRUE;
	case WM_TIMER:
		SetDlgItemTextW(hDlg, IDC_STATIC_DIC_PW, pw[ipw]);
		ipw++;
		if(ipw >= _countof(pw))
		{
			ipw = 0;
		}
		return (INT_PTR)TRUE;
	case WM_DESTROY:
		KillTimer(hDlg, IDC_STATIC_DIC_PW);
		return (INT_PTR)TRUE;
	default:
		break;
	}
	return (INT_PTR)FALSE;
}

void MakeSKKDic(HWND hwnd)
{
	SkkDicInfo.parent = hwnd;
	DialogBoxW(hInst, MAKEINTRESOURCE(IDD_DIALOG_SKK_DIC_MAKE), hwnd, DlgProcSKKDic);
}

unsigned int __stdcall ReqSKKUserDicThread(void *p)
{
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	DWORD dwMode;
	WCHAR wbuf[BUFSIZE];
	DWORD bytesWrite, bytesRead;
	WCHAR mgrpipename[MAX_KRNLOBJNAME];
	LPWSTR pszUserSid;
	WCHAR szDigest[32+1];
	MD5_DIGEST digest;
	int i;

	SkkUserDicInfo.hr = S_FALSE;

	ZeroMemory(mgrpipename, sizeof(mgrpipename));
	ZeroMemory(szDigest, sizeof(szDigest));

	if(GetUserSid(&pszUserSid))
	{
		if(GetMD5(&digest, (CONST BYTE *)pszUserSid, (DWORD)wcslen(pszUserSid)*sizeof(WCHAR)))
		{
			for(i=0; i<_countof(digest.digest); i++)
			{
				_snwprintf_s(&szDigest[i*2], _countof(szDigest)-i*2, _TRUNCATE, L"%02x", digest.digest[i]);
			}
		}

		LocalFree(pszUserSid);
	}

	_snwprintf_s(mgrpipename, _TRUNCATE, L"%s%s", CORVUSMGRPIPE, szDigest);

	if(WaitNamedPipeW(mgrpipename, NMPWAIT_USE_DEFAULT_WAIT) == 0)
	{
		goto exit;
	}

	hPipe = CreateFileW(mgrpipename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_EFFECTIVE_ONLY | SECURITY_IDENTIFICATION, NULL);
	if(hPipe == INVALID_HANDLE_VALUE)
	{
		goto exit;
	}

	dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
	SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);

	_snwprintf_s(wbuf, _TRUNCATE, L"%c\n%s\n", SkkUserDicInfo.command, SkkUserDicInfo.path);

	if(WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(wbuf, sizeof(wbuf));

	if(ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL) == FALSE)
	{
		goto exit;
	}

	if(wbuf[0] != REP_OK)
	{
		goto exit;
	}

	SkkUserDicInfo.hr = S_OK;

exit:
	if(hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPipe);
	}
	return 0;
}

void ReqSKKUserDicWaitThread(void *p)
{
	WCHAR num[32];
	HANDLE hThread;
	
	hThread = (HANDLE)_beginthreadex(NULL, 0, ReqSKKUserDicThread, NULL, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	EndDialog(SkkUserDicInfo.child, TRUE);

	if(SkkUserDicInfo.hr == S_OK)
	{
		MessageBoxW(SkkUserDicInfo.parent, L"完了しました。", TextServiceDesc, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		_snwprintf_s(num, _countof(num), L"失敗しました。0x%08X", SkkUserDicInfo.hr);
		MessageBoxW(SkkUserDicInfo.parent, num, TextServiceDesc, MB_OK | MB_ICONERROR);
	}
	return;
}

INT_PTR CALLBACK DlgProcSKKUserDic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPCWSTR pw[] = {L"／",L"─",L"＼",L"│"};
	static int ipw = 0;

	switch(message)
	{
	case WM_INITDIALOG:
		SkkUserDicInfo.child = hDlg;
		_beginthread(ReqSKKUserDicWaitThread, 0, NULL);
		SetTimer(hDlg, IDC_STATIC_DIC_PW, 1000, NULL);
		return (INT_PTR)TRUE;
	case WM_TIMER:
		SetDlgItemTextW(hDlg, IDC_STATIC_DIC_PW, pw[ipw]);
		ipw++;
		if(ipw >= _countof(pw))
		{
			ipw = 0;
		}
		return (INT_PTR)TRUE;
	case WM_DESTROY:
		KillTimer(hDlg, IDC_STATIC_DIC_PW);
		return (INT_PTR)TRUE;
	default:
		break;
	}
	return (INT_PTR)FALSE;
}

void ReqSKKUserDic(HWND hwnd, WCHAR command, LPCWSTR path)
{
	SkkUserDicInfo.parent = hwnd;
	SkkUserDicInfo.command = command;
	wcscpy_s(SkkUserDicInfo.path, path);
	
	switch(command)
	{
	case REQ_SKK_LOAD:
		DialogBoxW(hInst, MAKEINTRESOURCE(IDD_DIALOG_SKK_USER_DIC), hwnd, DlgProcSKKUserDic);
		break;
	case REQ_SKK_SAVE:
		SkkUserDicInfo.child = NULL;
		ReqSKKUserDicWaitThread(NULL);
		break;
	default:
		break;
	}
}
