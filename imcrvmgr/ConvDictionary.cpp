
#include "configxml.h"
#include "imcrvmgr.h"

void ConvUserDic(const std::wstring &searchkey, CANDIDATES &candidates);
void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates);
void AddComplement(const std::wstring &searchkey);
void DelComplement(const std::wstring &searchkey);
void LoadComplement();

//ユーザ辞書
USERDICS userdics;
//補完
COMPLEMENTS complements;

void ConvDictionary(const std::wstring &searchkey, CANDIDATES &candidates, WCHAR command)
{
	CANDIDATES::iterator candidates_itrf;
	CANDIDATES::iterator candidates_itrb;

	candidates.clear();

	//ユーザ辞書
	ConvUserDic(searchkey, candidates);

	//Unicodeコードポイント
	ConvUnicode(searchkey, candidates);

	//JIS X 0213 面区点番号
	ConvJISX0213(searchkey, candidates);

	//SKK辞書
	ConvSKKDic(searchkey, candidates);

	//SKK辞書サーバ
	if(serv)
	{
		ConvSKKServer(searchkey, candidates);
	}

	//重複候補を削除
	if(candidates.size() > 1)
	{
		for(candidates_itrf = candidates.begin(); candidates_itrf != candidates.end(); candidates_itrf++)
		{
			for(candidates_itrb = candidates_itrf + 1; candidates_itrb != candidates.end(); )
			{
				if(candidates_itrf->first == candidates_itrb->first)
				{
					candidates_itrb = candidates.erase(candidates_itrb);
				}
				else
				{
					candidates_itrb++;
				}
			}
		}
	}
}

void ConvUserDic(const std::wstring &searchkey, CANDIDATES &candidates)
{
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itr;

	userdics_itr = userdics.find(searchkey);
	if(userdics_itr != userdics.end())
	{
		for(candidates_itr = userdics_itr->second.begin(); candidates_itr != userdics_itr->second.end(); candidates_itr++)
		{
			if(!candidates_itr->first.empty())
			{
				candidates.push_back(*candidates_itr);
			}
		}
	}
}

void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates)
{
	FILE *fpidx;
	ULONGLONG pos;
	long left, mid, right;
	int comp;
	APPDATAXMLDIC xmldic;
	APPDATAXMLDIC::iterator d_itr;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	std::wstring candidate;
	std::wstring annotation;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");;

	_wfopen_s(&fpidx, pathskkcvdicidx, L"rb");
	if(fpidx == NULL)
	{
		return;
	}

	left = 0;
	fseek(fpidx, 0, SEEK_END);
	right = (ftell(fpidx) / sizeof(pos)) - 1;

	while(left <= right)
	{
		mid = (left + right) / 2;
		fseek(fpidx, mid*sizeof(pos), SEEK_SET);
		fread(&pos, sizeof(pos), 1, fpidx);

		if(ReadDicList(pathskkcvdicxml, SectionDictionary, xmldic, pos) != S_OK)
		{
			break;
		}

		d_itr = xmldic.begin();
		if(d_itr != xmldic.end())
		{
			comp = searchkey.compare(d_itr->first);
		}
		else
		{
			break;
		}

		if(comp == 0)
		{
			for(l_itr = d_itr->second.begin(); l_itr != d_itr->second.end(); l_itr++)
			{
				candidate.clear();
				annotation.clear();

				for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
				{
					if(r_itr->first == AttributeCandidate)
					{
						candidate = std::regex_replace(r_itr->second, re, fmt);
					}
					else if(r_itr->first == AttributeAnnotation)
					{
						annotation = std::regex_replace(r_itr->second, re, fmt);
					}
				}

				if(candidate.empty())
				{
					continue;
				}

				candidates.push_back(CANDIDATE(candidate, annotation));
			}
			break;
		}
		else if(comp > 0)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}

	fclose(fpidx);
}

void AddUserDic(const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, WCHAR command)
{
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itr;
	USERDICSPAIR userdic;

	userdics_itr = userdics.find(searchkey);
	if(userdics_itr == userdics.end())
	{
		userdic.first = searchkey;
		userdic.second.push_back(CANDIDATE(candidate, annotation));
		userdics.insert(userdic);
	}
	else
	{
		for(candidates_itr = userdics_itr->second.begin(); candidates_itr != userdics_itr->second.end(); candidates_itr++)
		{
			if(candidates_itr->first == candidate)
			{
				userdics_itr->second.erase(candidates_itr);
				break;
			}
		}
		userdics_itr->second.insert(userdics_itr->second.begin(), CANDIDATE(candidate, annotation));
	}

	if(command == REQ_USER_ADD_1)
	{
		AddComplement(searchkey);
	}

	bUserDicChg = TRUE;
}

void DelUserDic(const std::wstring &searchkey, const std::wstring &candidate)
{
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itr;

	userdics_itr = userdics.find(searchkey);
	if(userdics_itr != userdics.end())
	{
		for(candidates_itr = userdics_itr->second.begin(); candidates_itr != userdics_itr->second.end(); candidates_itr++)
		{
			if(candidates_itr->first == candidate)
			{
				userdics_itr->second.erase(candidates_itr);
				break;
			}
		}

		if(userdics_itr->second.empty())	//候補が空になった
		{
			userdics.erase(userdics_itr);
			DelComplement(searchkey);
		}
	}

	bUserDicChg = TRUE;
}

void LoadUserDic()
{
	HRESULT hr;
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itrs;
	USERDICSPAIR userdic;
	APPDATAXMLDIC xmldic;
	APPDATAXMLDIC::iterator d_itr;;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	std::wstring candidate;
	std::wstring annotation;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");;

	userdics.clear();

	hr = ReadDicList(pathuserdicxml, SectionDictionary, xmldic);
	EXIT_NOT_S_OK(hr);

	for(d_itr = xmldic.begin(); d_itr != xmldic.end(); d_itr++)
	{
		userdic.first = d_itr->first;

		for(l_itr = d_itr->second.begin(); l_itr != d_itr->second.end(); l_itr++)
		{
			candidate.clear();
			annotation.clear();

			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				if(r_itr->first == AttributeCandidate)
				{
					candidate = std::regex_replace(r_itr->second, re, fmt);
				}
				else if(r_itr->first == AttributeAnnotation)
				{
					annotation = std::regex_replace(r_itr->second, re, fmt);
				}
			}

			if(candidate.empty())
			{
				continue;
			}

			userdics_itr = userdics.find(userdic.first);
			if(userdics_itr == userdics.end())
			{
				userdic.second.clear();
				userdic.second.push_back(CANDIDATE(candidate, annotation));
				userdics.insert(userdic);
			}
			else
			{
				for(candidates_itrs = userdics_itr->second.begin(); candidates_itrs != userdics_itr->second.end(); candidates_itrs++)
				{
					if(candidates_itrs->first == candidate)
					{
						break;
					}
				}
				if(candidates_itrs == userdics_itr->second.end())
				{
					userdics_itr->second.push_back(CANDIDATE(candidate, annotation));
				}
			}
		}
	}
	
NOT_S_OK:
	LoadComplement();
}

unsigned int __stdcall SaveUserDicThreadEx(void *p)
{
	HRESULT hr;
	IXmlWriter *pWriter;
	IStream *pFileStream;
	std::wstring s;
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itr;
	COMPLEMENTS::iterator complements_itr;

	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;
	USERDICS::iterator u_itr;

	USERDATA *userdata = (USERDATA *)p;

	EnterCriticalSection(&csUserDataSave);	// !

	hr = WriterInit(pathuserdicxml, &pWriter, &pFileStream, FALSE);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterStartElement(pWriter, TagRoot);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	//ユーザ辞書
	hr = WriterStartSection(pWriter, SectionDictionary);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	for(u_itr = userdata->userdics.begin(); u_itr != userdata->userdics.end(); u_itr++)
	{
		list.clear();

		for(candidates_itr = u_itr->second.begin(); candidates_itr != u_itr->second.end(); candidates_itr++)
		{
			row.clear();

			attr.first = AttributeCandidate;
			attr.second = candidates_itr->first;
			row.push_back(attr);
			attr.first = AttributeAnnotation;
			attr.second = candidates_itr->second;
			row.push_back(attr);

			list.push_back(row);
		}
		
		hr = WriterStartElement(pWriter, TagEntry);
		EXIT_NOT_S_OK(hr);

		hr = WriterAttribute(pWriter, TagKey, u_itr->first.c_str());	//見出し語
		EXIT_NOT_S_OK(hr);

		hr = WriterList(pWriter, list);
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

	//補完
	hr = WriterStartSection(pWriter, SectionComplement);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	list.clear();

	for(complements_itr = userdata->complements.begin(); complements_itr != userdata->complements.end(); complements_itr++)
	{
		row.clear();

		attr.first = AttributeKey;
		attr.second = *complements_itr;
		row.push_back(attr);

		list.push_back(row);
	}

	hr = WriterList(pWriter, list, TRUE);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterEndSection(pWriter);	//SectionComplement
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterEndElement(pWriter);	//TagRoot
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

NOT_S_OK:
	hr = WriterFinal(&pWriter, &pFileStream);

	LeaveCriticalSection(&csUserDataSave);	// !

	delete userdata;

	return 0;
}

HANDLE StartSaveUserDicEx()
{
	HANDLE hThread = NULL;

	if(bUserDicChg)
	{
		bUserDicChg = FALSE;
		USERDATA *userdata = new USERDATA();
		userdata->userdics = userdics; 
		userdata->complements = complements;

		hThread = (HANDLE)_beginthreadex(NULL, 0, SaveUserDicThreadEx, userdata, 0, NULL);
	}

	return hThread;
}

void SaveUserDicThreadExClose(void *p)
{
	HANDLE hThread = (HANDLE)p;
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

void StartSaveUserDic()
{
	HANDLE hThread = StartSaveUserDicEx();
	if(hThread != NULL)
	{
		_beginthread(SaveUserDicThreadExClose, 0, hThread);
	}
}

void ConvComplement(const std::wstring &searchkey, CANDIDATES &candidates)
{
	COMPLEMENTS::reverse_iterator cmp_ritr;

	candidates.clear();

	if(!complements.empty())
	{
		for(cmp_ritr=complements.rbegin(); cmp_ritr!=complements.rend(); cmp_ritr++)
		{
			if(cmp_ritr->compare(0, searchkey.size(), searchkey) == 0)
			{
				candidates.push_back(CANDIDATE(*cmp_ritr, L""));
			}
		}
	}
}

void AddComplement(const std::wstring &searchkey)
{
	DelComplement(searchkey);

	complements.push_back(searchkey);
}

void DelComplement(const std::wstring &searchkey)
{
	COMPLEMENTS::iterator cmp_itr;

	if(!complements.empty())
	{
		for(cmp_itr = complements.begin(); cmp_itr != complements.end(); cmp_itr++)
		{
			if(searchkey.compare(*cmp_itr) == 0)
			{
				complements.erase(cmp_itr);
				break;
			}
		}
	}
}

void LoadComplement()
{
	HRESULT hr;
	COMPLEMENTS::iterator complements_itr;
	APPDATAXMLLIST xmllist;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");;

	complements.clear();

	hr = ReadList(pathuserdicxml, SectionComplement, xmllist);
	EXIT_NOT_S_OK(hr);

	for(l_itr = xmllist.begin(); l_itr != xmllist.end(); l_itr++)
	{
		for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
		{
			if(r_itr->first == AttributeKey)
			{
				for(complements_itr = complements.begin(); complements_itr != complements.end(); complements_itr++)
				{
					if(*complements_itr == r_itr->second)
					{
						complements.erase(complements_itr);
						complements_itr = complements.end();
						break;
					}
				}
				if(complements_itr == complements.end())
				{
					complements.push_back(std::regex_replace(r_itr->second, re, fmt));
				}
			}
		}
	}

NOT_S_OK:
	;
}
