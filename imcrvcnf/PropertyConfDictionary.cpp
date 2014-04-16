
#include "configxml.h"
#include "parseskkdic.h"
#include "imcrvcnf.h"
#include "resource.h"

//候補   pair< candidate, annotation >
typedef std::pair< std::wstring, std::wstring > CANDIDATE;
typedef std::vector< CANDIDATE > CANDIDATES;

//辞書   pair< key, candidates >
typedef std::map< std::wstring, CANDIDATES > SKKDIC;
typedef std::pair< std::wstring, CANDIDATES > SKKDICENTRY;

//見出し語位置
typedef std::map< std::wstring, long > KEYPOS;

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

	if(ReadList(pathconfigxml, SectionDictionary, list) == S_OK && list.size() != 0)
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

	for(i = 0; i < count; i++)
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
	LPCWSTR seps = L",";
	std::wstring annotation_seps;

	if(!annotation.empty())
	{
		annotation_seps = seps + annotation + seps;
	}

	skkdic_itr = skkdic.find(key);
	if(skkdic_itr == skkdic.end())
	{
		skkdicentry.first = key;
		skkdicentry.second.push_back(CANDIDATE(candidate, annotation_seps));
		skkdic.insert(skkdicentry);
	}
	else
	{
		for(candidates_itr = skkdic_itr->second.begin(); candidates_itr != skkdic_itr->second.end(); candidates_itr++)
		{
			if(candidates_itr->first == candidate)
			{
				if(candidates_itr->second.find(annotation_seps) == std::wstring::npos)
				{
					if(candidates_itr->second.empty())
					{
						candidates_itr->second.append(annotation_seps);
					}
					else
					{
						candidates_itr->second.append(annotation + seps);
					}
				}
				break;
			}
		}
		if(candidates_itr == skkdic_itr->second.end())
		{
			skkdic_itr->second.push_back(CANDIDATE(candidate, annotation_seps));
		}
	}
}

void LoadSKKDic(HWND hwnd, SKKDIC &entries_a, SKKDIC &entries_n)
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
	SKKDICOKURIBLOCKS so;
	int rl;

	hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	count = ListView_GetItemCount(hWndList);

	for(ic = 0; ic < count; ic++)
	{
		ListView_GetItemText(hWndList, ic, 0, path, _countof(path));
		okuri = -1;

		_wfopen_s(&fp, path, RB);
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
			fseek(fp, 0, SEEK_SET);
			break;
		}

		while(true)
		{
			rl = ReadSKKDicLine(fp, bom, okuri, key, sc, so);
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
				LoadSKKDicAdd((okuri == 0 ? entries_n : entries_a), key, sc_itr->first, sc_itr->second);
			}
		}

		fclose(fp);
	}
}

void WriteSKKDicEntry(FILE *fp, const std::wstring &key, const CANDIDATES &candidates)
{
	CANDIDATES::const_iterator candidates_itr;
	std::wstring line;

	line = key + L" /";
	for(candidates_itr = candidates.begin(); candidates_itr != candidates.end(); candidates_itr++)
	{
		line += candidates_itr->first;
		if(candidates_itr->second.size() > 2)
		{
			line += L";" + candidates_itr->second.substr(1, candidates_itr->second.size() - 2);
		}
		line += L"/";
	}
	line += L"\r\n";

	fwrite(line.c_str(), line.size() * sizeof(WCHAR), 1, fp);
}

HRESULT WriteSKKDic(const SKKDIC &entries_a, const SKKDIC &entries_n)
{
	FILE *fp;
	SKKDIC::const_iterator entries_itr;
	SKKDIC::const_reverse_iterator entries_ritr;
	long pos;
	KEYPOS keypos;
	KEYPOS::const_iterator keypos_itr;

	_wfopen_s(&fp, pathskkdic, WB);
	if(fp == NULL)
	{
		return S_FALSE;
	}

	//BOM
	fwrite("\xFF\xFE", 2, 1, fp);

	//送りありエントリ
	fwrite(EntriesAri, wcslen(EntriesAri) * sizeof(WCHAR), 1, fp);
	fseek(fp, -2, SEEK_END);
	fwrite(L"\r\n", 4, 1, fp);

	for(entries_ritr = entries_a.rbegin(); entries_ritr != entries_a.rend(); entries_ritr++)
	{
		pos = ftell(fp);
		keypos.insert(KEYPOS::value_type(entries_ritr->first, pos));

		WriteSKKDicEntry(fp, entries_ritr->first, entries_ritr->second);
	}

	//送りなしエントリ
	fwrite(EntriesNasi, wcslen(EntriesNasi) * sizeof(WCHAR), 1, fp);
	fseek(fp, -2, SEEK_END);
	fwrite(L"\r\n", 4, 1, fp);

	for(entries_itr = entries_n.begin(); entries_itr != entries_n.end(); entries_itr++)
	{
		pos = ftell(fp);
		keypos.insert(KEYPOS::value_type(entries_itr->first, pos));

		WriteSKKDicEntry(fp, entries_itr->first, entries_itr->second);
	}

	fclose(fp);

	//インデックスファイル
	_wfopen_s(&fp, pathskkidx, WB);
	if(fp == NULL)
	{
		return S_FALSE;
	}

	for(keypos_itr = keypos.begin(); keypos_itr != keypos.end(); keypos_itr++)
	{
		fwrite(&keypos_itr->second, sizeof(keypos_itr->second), 1, fp);
	}

	fclose(fp);

	return S_OK;
}

unsigned int __stdcall MakeSKKDicThread(void *p)
{
	SKKDIC entries_a, entries_n;

	LoadSKKDic(SkkDicInfo.parent, entries_a, entries_n);
	SkkDicInfo.hr = WriteSKKDic(entries_a, entries_n);
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
		if(++ipw >= _countof(pw))
		{
			ipw = 0;
		}
		return TRUE;
	case WM_DESTROY:
		KillTimer(hDlg, IDC_STATIC_DIC_PW);
		ipw = 0;
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
