
#include "configxml.h"
#include "parseskkdic.h"
#include "imcrvcnf.h"
#include "resource.h"

#define BUFSIZE 0x2000

typedef std::vector< std::wstring > ANNOTATIONS;
typedef std::pair< std::wstring, ANNOTATIONS > CANDIDATE;
typedef std::vector< CANDIDATE > CANDIDATES;
typedef std::pair< std::wstring, CANDIDATES > SKKDICENTRY;
typedef std::map< std::wstring, CANDIDATES > SKKDIC;

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

void LoadSKKDicAdd(SKKDIC &skkdic, const std::wstring &key, const std::wstring &candidate, const std::wstring &annotation)
{
	SKKDIC::iterator skkdic_itr;
	SKKDICENTRY skkdicentry;
	CANDIDATES::iterator candidates_itr;
	ANNOTATIONS::iterator annotations_itr;
	ANNOTATIONS annotations;

	skkdic_itr = skkdic.find(key);
	if(skkdic_itr == skkdic.end())
	{
		skkdicentry.first = key;
		if(!annotation.empty())
		{
			annotations.push_back(annotation);
		}
		skkdicentry.second.push_back(CANDIDATE(candidate, annotations));
		skkdic.insert(skkdicentry);
	}
	else
	{
		for(candidates_itr = skkdic_itr->second.begin(); candidates_itr != skkdic_itr->second.end(); candidates_itr++)
		{
			if(candidates_itr->first == candidate)
			{
				for(annotations_itr = candidates_itr->second.begin(); annotations_itr != candidates_itr->second.end(); annotations_itr++)
				{
					if(*annotations_itr == annotation)
					{
						break;
					}
				}
				if(annotations_itr == candidates_itr->second.end())
				{
					if(!annotation.empty())
					{
						candidates_itr->second.push_back(annotation);
					}
				}
				break;
			}
		}
		if(candidates_itr == skkdic_itr->second.end())
		{
			if(!annotation.empty())
			{
				annotations.push_back(annotation);
			}
			skkdic_itr->second.push_back(CANDIDATE(candidate, annotations));
		}
	}
}

void LoadSKKDic(HWND hwnd, SKKDIC &skkdic)
{
	HWND hWndList;
	WCHAR path[MAX_PATH];
	size_t count, ic;
	FILE *fp;
	WCHAR bom;
	std::wstring key;
	int okuri;
	SKKDICCANDIDATES sc;
	SKKDICCANDIDATES::iterator sc_itr;
	int rl;

	hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	count = ListView_GetItemCount(hWndList);

	for(ic=0; ic<count; ic++)
	{
		ListView_GetItemText(hWndList, ic, 0, path, _countof(path));
		okuri = -1;

		_wfopen_s(&fp, path, L"rb");
		if(fp == NULL)
		{
			continue;
		}

		bom = L'\0';
		fread(&bom, 2, 1, fp);
		if(bom == 0xBBEF)
		{
			bom = L'\0';
			fread(&bom, 2, 1, fp);
			if((bom & 0xFF) == 0xBF)
			{
				bom = 0xFEFF;
			}
		}

		switch(bom)
		{
		case 0xFEFF:
			fclose(fp);
			_wfopen_s(&fp, path, RccsUNICODE);
			if(fp == NULL)
			{
				continue;
			}
			break;
		default:
			fseek(fp, SEEK_SET, 0);
			break;
		}

		while(true)
		{
			rl = ReadSKKDicLine(fp, bom, okuri, key, sc);
			if(rl == -1)
			{
				break;
			}
			else if(rl == 1)
			{
				continue;
			}

			for(sc_itr = sc.begin(); sc_itr != sc.end(); sc_itr++)
			{
				LoadSKKDicAdd(skkdic, key, sc_itr->first, sc_itr->second);
			}
		}

		fclose(fp);
	}
}

HRESULT WriteSKKDicXml(SKKDIC &skkdic)
{
	HRESULT hr;
	FILE *fpidx;
	IXmlWriter *pWriter;
	IStream *pFileStream;
	SKKDIC::iterator skkdic_itr;
	CANDIDATES::iterator candidates_itr;
	ANNOTATIONS::iterator annotations_itr;
	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
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

	for(skkdic_itr = skkdic.begin(); skkdic_itr != skkdic.end(); skkdic_itr++)
	{
		list.clear();
		for(candidates_itr = skkdic_itr->second.begin(); candidates_itr != skkdic_itr->second.end(); candidates_itr++)
		{
			row.clear();
			attr.first = AttributeCandidate;
			attr.second = candidates_itr->first;
			row.push_back(attr);
			attr.first = AttributeAnnotation;
			attr.second = L"";
			for(annotations_itr = candidates_itr->second.begin(); annotations_itr != candidates_itr->second.end(); annotations_itr++)
			{
				if(annotations_itr != candidates_itr->second.begin())
				{
					attr.second += L",";
				}
				attr.second += *annotations_itr;
			}
			row.push_back(attr);
			list.push_back(row);
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

		hr = WriterAttribute(pWriter, TagKey, skkdic_itr->first.c_str());	//見出し
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
	SKKDIC skkdic;

	LoadSKKDic(SkkDicInfo.parent, skkdic);
	SkkDicInfo.hr = WriteSKKDicXml(skkdic);
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
		return TRUE;
	case WM_TIMER:
		SetDlgItemTextW(hDlg, IDC_STATIC_DIC_PW, pw[ipw]);
		ipw++;
		if(ipw >= _countof(pw))
		{
			ipw = 0;
		}
		return TRUE;
	case WM_DESTROY:
		KillTimer(hDlg, IDC_STATIC_DIC_PW);
		return TRUE;
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
		return TRUE;
	case WM_TIMER:
		SetDlgItemTextW(hDlg, IDC_STATIC_DIC_PW, pw[ipw]);
		ipw++;
		if(ipw >= _countof(pw))
		{
			ipw = 0;
		}
		return TRUE;
	case WM_DESTROY:
		KillTimer(hDlg, IDC_STATIC_DIC_PW);
		return TRUE;
	default:
		break;
	}
	return FALSE;
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
