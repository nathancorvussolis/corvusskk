
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
} SkkDicInfo;

void LoadDictionary(HWND hwnd)
{
	HWND hWndList;
	int i = 0;
	LVITEMW item;
	APPDATAXMLLIST list;

	if(ReadList(pathconfigxml, SectionDictionary, list) == S_OK && list.size() != 0)
	{
		hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
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
		bool hit = false;
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
				hit = true;
				break;
			}
		}
		if(!hit)
		{
			skkdic_itr->second.push_back(SKKDICCANDIDATE(candidate, MakeConcat(annotation_seps)));
		}
	}
}

HRESULT LoadSKKDic(HWND hwnd, SKKDIC &entries_a, SKKDIC &entries_n)
{
	HWND hWndList;
	WCHAR path[MAX_PATH];
	size_t count, ic;
	FILE *fp;
	WCHAR bom;
	std::wstring key;
	int okuri;
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;
	int rl;

	hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	count = ListView_GetItemCount(hWndList);

	for(ic = 0; ic < count; ic++)
	{
		if(SkkDicInfo.cancel)
		{
			return E_ABORT;
		}

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
		return S_FALSE;
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
		return S_FALSE;
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

		_snwprintf_s(num, _countof(num), L"失敗しました。0x%08X", SkkDicInfo.hr);
		MessageBoxW(SkkDicInfo.parent, num, TextServiceDesc, MB_OK | MB_ICONERROR);
	}
	return;
}

INT_PTR CALLBACK DlgProcSKKDic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPCWSTR pw[] = {L"／", L"─", L"＼", L"│"};
	static int ipw = 0;

	switch(message)
	{
	case WM_INITDIALOG:
		SkkDicInfo.cancel = FALSE;
		SkkDicInfo.hr = S_FALSE;
		SkkDicInfo.child = hDlg;
		_beginthread(MakeSKKDicWaitThread, 0, NULL);
		SetTimer(hDlg, IDC_STATIC_DIC_PW, 1000, NULL);
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
