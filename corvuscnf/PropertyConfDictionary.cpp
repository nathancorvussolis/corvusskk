
#include "common.h"
#include "configxml.h"
#include "corvuscnf.h"
#include "resource.h"
#include "eucjis2004.h"

#define BUFSIZE 0x1000

typedef std::pair<std::wstring, std::wstring> ENTRY;
typedef std::map<std::wstring, std::wstring> ENTRYS;

void LoadDictionary(HWND hwnd)
{
	HWND hWndList;
	int i;
	LVITEMW item;
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;

	if(ReadList(pathconfigxml, SectionDictionary, list) == S_OK)
	{
		hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
		i = 0;
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
			++i;
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
	std::vector<std::wstring> apath;
	size_t size, i;
	FILE *fpskkdic;
	WCHAR bom;
	CHAR buf[BUFSIZE];
	WCHAR wbuf[BUFSIZE];
	void *rp;
	std::wstring s;
	std::wregex re;
	std::wstring fmt;
	ENTRY entry;
	ENTRYS::iterator entrys_itr;

	hWndList = GetDlgItem(hwnd, IDC_LIST_SKK_DIC);
	size = ListView_GetItemCount(hWndList);

	for(i=0; i<size; i++)
	{
		ListView_GetItemText(hWndList, i, 0, path, _countof(path));
		apath.push_back(path);
	}

	for(i=0; i<apath.size(); i++)
	{
		_wfopen_s(&fpskkdic, apath[i].c_str(), L"rb");
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
}

void WriteSKKDicXml(ENTRYS &entrys)
{
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
	APPDATAXMLROW::iterator r_itr;;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;
	ULARGE_INTEGER uli1;
	LARGE_INTEGER li0 = {0};

	_wfopen_s(&fpidx, pathskkcvdicidx, L"wb");

	WriterInit(pathskkcvdicxml, &pWriter, &pFileStream);

	WriterStartSection(pWriter, SectionDictionary);

	for(entrys_itr = entrys.begin(); entrys_itr != entrys.end(); entrys_itr++)
	{
		//見出し
		//line = entrys_itr->first;

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

		// concatを置換
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

		for(r_itr = row.begin(); r_itr != row.end(); r_itr++)
		{
			s = r_itr->second;

			re.assign(L".*\\(concat \".*\"\\).*");
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
			}

			r_itr->second = s;
		}

		//インデックスファイル書き込み
		if(fpidx != NULL)
		{
			pWriter->Flush();

			pFileStream->Seek(li0, STREAM_SEEK_CUR, &uli1);

			uli1.QuadPart += 1;
			fwrite(&uli1.QuadPart, sizeof(uli1.QuadPart), 1, fpidx);
		}

		//辞書ファイル書き込み
		WriterStartElement(pWriter, L"entry");

		WriterAttribute(pWriter, L"key", entrys_itr->first.c_str());

		WriterList(pWriter, list);

		WriterEndElement(pWriter);
	}

	WriterEndSection(pWriter);

	WriterFinal(&pWriter, &pFileStream);

	fclose(fpidx);
}

void MakeSKKDic(HWND hwnd)
{
	ENTRYS entrys;

	LoadSKKDic(hwnd, entrys);

	WriteSKKDicXml(entrys);
}
