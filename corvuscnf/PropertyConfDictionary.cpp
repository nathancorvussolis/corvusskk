
#include "corvuscnf.h"
#include "convtable.h"
#include "resource.h"
#include "eucjis2004.h"

#define BUFSIZE 0x1000

void LoadDictionary(HWND hwnd)
{
	WCHAR path[MAX_PATH];
	HWND hWndList;
	int i, j;
	LVITEMW item;
	FILE *fp;

	_wfopen_s(&fp, pathconfdic, RccsUNICODE);
	if(fp != NULL)
	{
		hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
		j = 0;
		while(fgetws(path, _countof(path), fp) != NULL)
		{
			for(i=0; i<_countof(path); i++)
			{
				if(path[i] == L'\r' || path[i] == L'\n')
				{
					path[i] = L'\0';
					break;
				}
			}
			item.mask = LVIF_TEXT;
			item.pszText = path;
			item.iItem = j;
			item.iSubItem = 0;
			ListView_InsertItem(hWndList, &item);
			++j;
		}
		ListView_SetColumnWidth(hWndList, 0, LVSCW_AUTOSIZE);
		fclose(fp);
	}
}

void SaveDictionary(HWND hwnd)
{
	WCHAR path[MAX_PATH];
	HWND hWndList;
	int i, count;
	FILE *fp;

	_wfopen_s(&fp, pathconfdic, WccsUNICODE);
	if(fp != NULL)
	{
		hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
		count = ListView_GetItemCount(hWndList);
		for(i=0; i<count; i++)
		{
			ListView_GetItemText(hWndList, i, 0, path, _countof(path));
			fwprintf(fp, L"%s\n", path);
		}
		fclose(fp);
	}
}

void MakeSKKDic(void)
{
	typedef std::pair<std::wstring, std::wstring> ENTRY;
	typedef std::map<std::wstring, std::wstring> ENTRYS;
	ENTRY entry;
	ENTRYS entrys;
	ENTRYS::iterator entrys_itr;
	CHAR buf[BUFSIZE];
	WCHAR wbuf[BUFSIZE];
	size_t size;
	WCHAR path[MAX_PATH];
	FILE *fpdiclist, *fpskkdic, *fpcdictxt, *fpcdicidx;
	WCHAR bom;
	void *rp;
	long cpf;
	size_t i, is, ie;
	std::vector<std::wstring> es;
	std::wstring s, line;
	std::wregex re;
	std::wstring fmt;

	_wfopen_s(&fpdiclist, pathconfdic, RccsUNICODE);
	if(fpdiclist == NULL)
	{
		return;
	}

	while(fgetws(path, _countof(path), fpdiclist) != NULL)
	{
		if(path[0] != L'\0')
		{
			path[wcslen(path) - 1] = L'\0';
		}

		_wfopen_s(&fpskkdic, path, L"rb");
		if(fpskkdic == NULL)
		{
			continue;
		}

		fread(&bom, 2, 1, fpskkdic);

		switch(bom)
		{
		case 0xFEFF:
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
				rp = fgets((char*)buf, _countof(buf), fpskkdic);
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
				size = _countof(wbuf) - 1;
				if(EucJis2004ToWideChar(buf, NULL, wbuf, &size))
				{
					wbuf[size] = L'\0';
				}
				else
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
			i = s.find_first_of(L'\x20');
			if(i == std::wstring::npos)
			{
				continue;
			}

			entry.first = s.substr(0, i);
			entry.second = s.substr(i + 1);
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

	fclose(fpdiclist);

	if(entrys.empty()) return;

	_wfopen_s(&fpcdictxt, pathskkcvdic, WccsUNICODE);
	if(fpcdictxt == NULL)
	{
		return;
	}
	_wfopen_s(&fpcdicidx, pathskkcvidx, L"wb");
	if(fpcdicidx == NULL)
	{
		fclose(fpcdictxt);
		return;
	}

	for(entrys_itr = entrys.begin(); entrys_itr != entrys.end(); entrys_itr++)
	{
		//見出し
		line = entrys_itr->first;

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

		// 「;」→「\t」、concatを置換
		for(i=0; i<es.size(); i++)
		{
			s = es[i];

			if(s.find_first_of(L';') == std::wstring::npos)
			{
				s += L"\t";
			}
			else
			{
				re.assign(L";");
				fmt.assign(L"\t");
				s = std::regex_replace(s, re, fmt);
			}

			if(s.find_first_of(L'\t') == 0)
			{
				continue;
			}

			re.assign(L".*\\(concat \".*\"\\).*");
			if(std::regex_match(s, re))
			{
				re.assign(L"(.*)\\(concat \"(.*)\"\\)(.*)");
				fmt.assign(L"$1$2$3");
				s = std::regex_replace(s, re, fmt);	//annotation if annotation has / candidate if annotaion doesnot has
				s = std::regex_replace(s, re, fmt);	//candidate if annotaion has

				re.assign(L"\\\\057");
				fmt.assign(L"/");
				s = std::regex_replace(s, re, fmt);

				re.assign(L"\\\\073");
				fmt.assign(L";");
				s = std::regex_replace(s, re, fmt);
			}

			//候補、注釈
			line += L"\t" + s;
		}

		//インデックスファイル書き込み
		fseek(fpcdictxt, 0, SEEK_END);
		cpf = ftell(fpcdictxt);
		fwrite(&cpf, sizeof(cpf), 1, fpcdicidx);

		//辞書ファイル書き込み
		fwprintf(fpcdictxt, L"%s\n", line.c_str());
	}

	fclose(fpcdicidx);
	fclose(fpcdictxt);
}
