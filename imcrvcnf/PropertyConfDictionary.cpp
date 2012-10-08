
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"
#include "eucjis2004.h"

#define BUFSIZE 0x1000

typedef std::pair<std::wstring, std::wstring> ENTRY;
typedef std::map<std::wstring, std::wstring> ENTRYS;

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
	CHAR buf[BUFSIZE];
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
	APPDATAXMLROW::iterator r_itr;;
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

		// 候補と注釈を分割
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

		// concatを置換
		for(l_itr = list.begin(); l_itr != list.end(); l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
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

HRESULT MakeSKKDic(HWND hwnd)
{
	HRESULT hr;
	ENTRYS entrys;

	LoadSKKDic(hwnd, entrys);

	hr = WriteSKKDicXml(entrys);

	return hr;
}
