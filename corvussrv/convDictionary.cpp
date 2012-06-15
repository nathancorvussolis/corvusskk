
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

	EnterCriticalSection(&csUserDic);	// !

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

	LeaveCriticalSection(&csUserDic);	// !
}

void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates)
{
	WCHAR wbuf[BUFSIZE];
	FILE *fpcdic, *fpcidx;
	long left, mid, right, cpfmid;
	int comp;
	size_t i, ic, ia;
	const WCHAR tab = L'\t';
	const WCHAR nl = L'\n';
	std::wstring s;
	std::wstring candidate;
	std::wstring annotation;

	_wfopen_s(&fpcdic, pathskkcvdic, RccsUNICODE);
	if(fpcdic == NULL)
	{
		return;
	}

	_wfopen_s(&fpcidx, pathskkcvidx, L"rb");
	if(fpcidx == NULL)
	{
		fclose(fpcdic);
		return;
	}

	left = 0;
	fseek(fpcidx, 0, SEEK_END);
	right = (ftell(fpcidx) / sizeof(long)) - 1;

	while(left <= right)
	{
		mid = (left + right) / 2;
		fseek(fpcidx, mid*sizeof(long), SEEK_SET);
		fread(&cpfmid, sizeof(long), 1, fpcidx);
		fseek(fpcdic, cpfmid, SEEK_SET);

		fgetws(wbuf, _countof(wbuf), fpcdic);
		s.assign(wbuf);

		i = s.find_first_of(tab);
		comp = searchkey.compare(s.substr(0, i));

		if(comp == 0)
		{
			while(true)
			{
				ic = s.find_first_of(tab, i + 1);
				if(ic == std::wstring::npos)
				{
					break;
				}
				ia = s.find_first_of(tab, ic + 1);
				if(ia == std::wstring::npos)
				{
					ia = s.find_first_of(nl, ic + 1);
					if(ia == std::wstring::npos)
					{
						break;
					}
				}

				candidate = s.substr(i + 1, ic - (i + 1));
				annotation = s.substr(ic + 1, ia - (ic + 1));

				if(!candidate.empty())
				{
					candidates.push_back(CANDIDATE(candidate, annotation));
				}

				i = ia;
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

	fclose(fpcidx);
	fclose(fpcdic);
}

void AddUserDic(const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, WCHAR command)
{
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itr;
	USERDICSPAIR userdic;

	EnterCriticalSection(&csUserDic);	// !

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

	LeaveCriticalSection(&csUserDic);	// !
}

void DelUserDic(const std::wstring &searchkey, const std::wstring &candidate)
{
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itr;

	EnterCriticalSection(&csUserDic);	// !

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

	LeaveCriticalSection(&csUserDic);	// !
}

void LoadUserDic()
{
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itrs;
	USERDICSPAIR userdic;
	CANDIDATES::iterator candidates_itrp;
	FILE *fp;
	WCHAR wbuf[BUFSIZE];
	size_t i, ic, ia;
	int stage = 0;
	const WCHAR tab = L'\t';
	const WCHAR nl = L'\n';
	std::wstring s;
	std::wstring candidate;
	std::wstring annotation;

	EnterCriticalSection(&csUserDic);	// !

	userdics.clear();

	_wfopen_s(&fp, pathuserdic, RccsUNICODE);
	if(fp == NULL)
	{
		LeaveCriticalSection(&csUserDic);	// !
		return;
	}

	while(fgetws(wbuf, _countof(wbuf), fp) != NULL)
	{
		s.assign(wbuf);

		i = s.find_first_of(tab);
		if((i == std::wstring::npos) || (i == 0))
		{
			continue;
		}

		userdic.first.assign(s.substr(0, i));
		userdic.second.clear();

		while(true)
		{
			ic = s.find_first_of(tab, i + 1);
			if(ic == std::wstring::npos)
			{
				break;
			}
			ia = s.find_first_of(tab, ic + 1);
			if(ia == std::wstring::npos)
			{
				ia = s.find_first_of(nl, ic + 1);
				if(ia == std::wstring::npos)
				{
					break;
				}
			}

			candidate = s.substr(i + 1, ic - (i + 1));
			annotation = s.substr(ic + 1, ia - (ic + 1));

			if(!candidate.empty())
			{
				userdic.second.push_back(CANDIDATE(candidate, annotation));
			}

			i = ia;
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

	fclose(fp);
	
	LoadComplement();

	LeaveCriticalSection(&csUserDic);	// !
}

unsigned int __stdcall SaveUserDicThread(void *p)
{
	FILE *fp;
	USERDICS::iterator userdics_itr;
	CANDIDATES::iterator candidates_itr;
	USERDICS userdics_tmp;
	std::wstring s;
	COMPLEMENTS::iterator complements_itr;
	COMPLEMENTS complements_tmp;

	EnterCriticalSection(&csUserDic);	// !

	userdics_tmp = userdics;
	complements_tmp = complements;
	bUserDicChg = FALSE;

	LeaveCriticalSection(&csUserDic);	// !

	EnterCriticalSection(&csUserDicS);	// !

	//ユーザ辞書
	_wfopen_s(&fp, pathuserdic, WccsUNICODE);
	if(fp != NULL)
	{
		for(userdics_itr = userdics_tmp.begin(); userdics_itr != userdics_tmp.end(); userdics_itr++)
		{
			//key
			s = userdics_itr->first;

			for(candidates_itr = userdics_itr->second.begin(); candidates_itr != userdics_itr->second.end(); candidates_itr++)
			{
				//candidate
				s += L"\t" + candidates_itr->first + L"\t";

				//annotation
				if(!candidates_itr->second.empty())
				{
					s += candidates_itr->second;
				}
			}

			fwprintf(fp, L"%s\n", s.c_str());
		}

		fclose(fp);
	}
	userdics_tmp.clear();

	//補完
	_wfopen_s(&fp, pathusercmp, WccsUNICODE);
	if(fp != NULL)
	{
		if(!complements_tmp.empty())
		{
			for(complements_itr = complements_tmp.begin(); complements_itr != complements_tmp.end(); complements_itr++)
			{
				fwprintf(fp, L"%s\n", complements_itr->c_str());
			}
		}

		fclose(fp);
	}

	LeaveCriticalSection(&csUserDicS);	// !

	_endthreadex(0);
	return 0;
}

HANDLE StartSaveUserDic()
{
	HANDLE hThread = NULL;
	BOOL bUserDicChgTmp;

	EnterCriticalSection(&csUserDic);	// !

	bUserDicChgTmp = bUserDicChg;

	LeaveCriticalSection(&csUserDic);	// !

	if(bUserDicChgTmp)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, SaveUserDicThread, NULL, 0, NULL);
	}

	return hThread;
}

void ConvComplement(const std::wstring &searchkey, CANDIDATES &candidates)
{
	COMPLEMENTS::reverse_iterator cmp_ritr;

	EnterCriticalSection(&csUserDic);	// !

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

	LeaveCriticalSection(&csUserDic);	// !
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
	FILE *fp;
	WCHAR wbuf[BUFSIZE];
	std::wstring s;
	std::wregex re;
	std::wstring fmt;
	COMPLEMENTS::iterator complements_itr;

	complements.clear();

	_wfopen_s(&fp, pathusercmp, RccsUNICODE);
	if(fp == NULL)
	{
		return;
	}

	re.assign(L"\\t|\\r|\\n");
	fmt.assign(L"");

	while(fgetws(wbuf, _countof(wbuf), fp) != NULL)
	{
		if(wbuf[0] != L'\0')
		{
			s.assign(wbuf);
			s = std::regex_replace(s, re, fmt);
			for(complements_itr = complements.begin(); complements_itr != complements.end(); complements_itr++)
			{
				if(*complements_itr == s)
				{
					complements.erase(complements_itr);
					complements_itr = complements.end();
					break;
				}
			}
			if(complements_itr == complements.end())
			{
				complements.push_back(s);
			}
		}
	}

	fclose(fp);
}
