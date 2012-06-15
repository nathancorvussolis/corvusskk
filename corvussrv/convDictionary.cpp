
#include "common.h"
#include "configxml.h"
#include "corvussrv.h"

void ConvUserDic(const std::wstring &searchkey, CANDIDATES &candidates);
void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates);
void AddComplement(const std::wstring &searchkey);
void DelComplement(const std::wstring &searchkey);
void LoadComplement();

#define BUFSIZE 0x1000

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

		xmldic.clear();
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
				for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
				{
					if(r_itr->first == AttributeCandidate)
					{
						candidate = r_itr->second;
					}
					else if(r_itr->first == AttributeAnnotation)
					{
						annotation = r_itr->second;
					}
				}

				if(!candidate.empty())
				{
					candidates.push_back(CANDIDATE(candidate, annotation));
				}
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
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itrs;
	USERDICSPAIR userdic;
	CANDIDATES::iterator candidates_itrp;
	std::wstring candidate;
	std::wstring annotation;
	APPDATAXMLDIC xmldic;
	APPDATAXMLDIC::iterator d_itr;;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;

	userdics.clear();

	ReadDicList(pathuserdicxml, SectionDictionary, xmldic);

	for(d_itr = xmldic.begin(); d_itr != xmldic.end(); d_itr++)
	{
		userdic.first = d_itr->first;
		userdic.second.clear();

		for(l_itr = d_itr->second.begin(); l_itr != d_itr->second.end(); l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				if(r_itr->first == AttributeCandidate)
				{
					candidate = r_itr->second;
				}
				else if(r_itr->first == AttributeCandidate)
				{
					annotation = r_itr->second;
				}
			}

			if(!candidate.empty())
			{
				userdic.second.push_back(CANDIDATE(candidate, annotation));
			}
		}

		if(userdic.second.size() != 0)
		{
			userdics_itr = userdics.find(userdic.first);
			if(userdics_itr == userdics.end())
			{
				userdics.insert(userdic);
			}
			else
			{
				for(candidates_itrp = userdic.second.begin(); candidates_itrp != userdic.second.end(); candidates_itrp++)
				{
					for(candidates_itrs = userdics_itr->second.begin(); candidates_itrs != userdics_itr->second.end(); candidates_itrs++)
					{
						if(candidates_itrp->first == candidates_itrs->first)
						{
							break;
						}
					}
					if(candidates_itrs == userdics_itr->second.end())
					{
						userdics_itr->second.push_back(*candidates_itrp);
					}
				}
			}
		}

	}
	
	LoadComplement();
}

unsigned int __stdcall SaveUserDicThreadEx(void *p)
{
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

	WriterInit(pathuserdicxml, &pWriter, &pFileStream);

	//ユーザ辞書
	WriterStartSection(pWriter, SectionDictionary);

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
		
		WriterStartElement(pWriter, L"entry");

		WriterAttribute(pWriter, L"key", u_itr->first.c_str());

		WriterList(pWriter, list);

		WriterEndElement(pWriter);
	}

	WriterEndSection(pWriter);

	//補完
	WriterStartSection(pWriter, SectionComplement);

	list.clear();

	for(complements_itr = userdata->complements.begin(); complements_itr != userdata->complements.end(); complements_itr++)
	{
		row.clear();

		attr.first = AttributeKey;
		attr.second = *complements_itr;
		row.push_back(attr);

		list.push_back(row);
	}

	WriterList(pWriter, list);

	WriterEndSection(pWriter);


	WriterFinal(&pWriter, &pFileStream);

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
	COMPLEMENTS::iterator complements_itr;
	APPDATAXMLLIST xmllist;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;

	complements.clear();

	ReadList(pathuserdicxml, SectionComplement, xmllist);

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
					complements.push_back(r_itr->second);
				}
			}
		}
	}
}
