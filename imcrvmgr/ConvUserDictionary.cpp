
#include "configxml.h"
#include "parseskkdic.h"
#include "imcrvmgr.h"

void AddKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder);
void DelKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder);

void ConvUserDic(const std::wstring &searchkey, CANDIDATES &candidates)
{
	USERDIC::iterator userdic_itr;
	CANDIDATES::reverse_iterator candidates_ritr;

	userdic_itr = userdic.find(searchkey);
	if(userdic_itr != userdic.end())
	{
		for(candidates_ritr = userdic_itr->second.rbegin(); candidates_ritr != userdic_itr->second.rend(); candidates_ritr++)
		{
			if(!candidates_ritr->first.empty())
			{
				candidates.push_back(*candidates_ritr);
			}
		}
	}
}

void ConvComplement(const std::wstring &searchkey, CANDIDATES &candidates)
{
	KEYORDER::reverse_iterator keyorder_ritr;

	candidates.clear();

	if(!complements.empty())
	{
		for(keyorder_ritr = complements.rbegin(); keyorder_ritr != complements.rend(); keyorder_ritr++)
		{
			if(keyorder_ritr->compare(0, searchkey.size(), searchkey) == 0)
			{
				candidates.push_back(CANDIDATE(*keyorder_ritr, L""));
			}
		}
	}
}

void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation)
{
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itr;
	USERDICENTRY userdicentry;
	std::wregex re;
	std::wstring fmt;
	std::wstring candidate_esc;
	std::wstring annotation_esc;

	candidate_esc = candidate;

	// "/" -> \057, ";" -> \073, "[" -> 133, "]" -> 135
	re.assign(L"[/;\\[\\]]");
	if(std::regex_search(candidate_esc, re))
	{
		// "\"" -> "\\\"", "\\" -> "\\\\"
		re.assign(L"([\\\"|\\\\])");
		fmt.assign(L"\\$1");
		candidate_esc = std::regex_replace(candidate_esc, re, fmt);

		re.assign(L"/");
		fmt.assign(L"\\057");
		candidate_esc = std::regex_replace(candidate_esc, re, fmt);

		re.assign(L";");
		fmt.assign(L"\\073");
		candidate_esc = std::regex_replace(candidate_esc, re, fmt);

		re.assign(L"\\[");
		fmt.assign(L"\\133");
		candidate_esc = std::regex_replace(candidate_esc, re, fmt);

		re.assign(L"\\]");
		fmt.assign(L"\\135");
		candidate_esc = std::regex_replace(candidate_esc, re, fmt);

		candidate_esc = L"(concat \"" + candidate_esc + L"\")";
	}

	// "/" -> ""
	re.assign(L"/");
	fmt.assign(L"");
	annotation_esc = std::regex_replace(annotation, re, fmt);;

	userdic_itr = userdic.find(searchkey);
	if(userdic_itr == userdic.end())
	{
		userdicentry.first = searchkey;
		userdicentry.second.push_back(CANDIDATE(candidate_esc, annotation_esc));
		userdic.insert(userdicentry);
	}
	else
	{
		for(candidates_itr = userdic_itr->second.begin(); candidates_itr != userdic_itr->second.end(); candidates_itr++)
		{
			if(candidates_itr->first == candidate_esc)
			{
				userdic_itr->second.erase(candidates_itr);
				break;
			}
		}
		userdic_itr->second.push_back(CANDIDATE(candidate_esc, annotation_esc));
	}

	switch(command)
	{
	case REQ_USER_ADD_0:
		AddKeyOrder(searchkey, accompaniments);
		break;
	case REQ_USER_ADD_1:
		AddKeyOrder(searchkey, complements);
		break;
	default:
		break;
	}

	bUserDicChg = TRUE;
}

void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate)
{
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itr;

	userdic_itr = userdic.find(searchkey);
	if(userdic_itr != userdic.end())
	{
		for(candidates_itr = userdic_itr->second.begin(); candidates_itr != userdic_itr->second.end(); candidates_itr++)
		{
			if(candidates_itr->first == candidate)
			{
				userdic_itr->second.erase(candidates_itr);
				break;
			}
		}

		if(userdic_itr->second.empty())
		{
			userdic.erase(userdic_itr);

			switch(command)
			{
			case REQ_USER_DEL_0:
				DelKeyOrder(searchkey, accompaniments);
				break;
			case REQ_USER_DEL_1:
				DelKeyOrder(searchkey, complements);
				break;
			default:
				break;
			}
		}

		bUserDicChg = TRUE;
	}
}

void AddKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder)
{
	DelKeyOrder(searchkey, keyorder);

	keyorder.push_back(searchkey);
}

void DelKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder)
{
	KEYORDER::iterator keyorder_itr;

	if(!keyorder.empty())
	{
		for(keyorder_itr = keyorder.begin(); keyorder_itr != keyorder.end(); keyorder_itr++)
		{
			if(searchkey == *keyorder_itr)
			{
				keyorder.erase(keyorder_itr);
				break;
			}
		}
	}
}

BOOL LoadSKKUserDic()
{
	FILE *fp;
	std::wstring key;
	KEYORDER complements_tmp;
	KEYORDER accompaniments_tmp;
	KEYORDER::reverse_iterator keyorder_ritr;
	int okuri = -1;
	SKKDICCANDIDATES sc;
	SKKDICCANDIDATES::reverse_iterator sc_ritr;
	int rl;

	_wfopen_s(&fp, pathuserdic, RccsUNICODE);
	if(fp == NULL)
	{
		return FALSE;
	}

	while(true)
	{
		rl = ReadSKKDicLine(fp, 0xFEFF, okuri, key, sc);
		if(rl == -1)
		{
			break;
		}
		else if(rl == 1)
		{
			continue;
		}

		for(sc_ritr = sc.rbegin(); sc_ritr != sc.rend(); sc_ritr++)
		{
			AddUserDic(WCHAR_MAX, key, sc_ritr->first, sc_ritr->second);
		}

		switch(okuri)
		{
		case 0:
			complements_tmp.push_back(key);
			break;
		case 1:
			accompaniments_tmp.push_back(key);
			break;
		default:
			break;
		}
	}

	fclose(fp);

	complements.reserve(complements_tmp.capacity());
	for(keyorder_ritr = complements_tmp.rbegin(); keyorder_ritr != complements_tmp.rend(); keyorder_ritr++)
	{
		complements.push_back(*keyorder_ritr);
	}

	accompaniments.reserve(accompaniments_tmp.capacity());
	for(keyorder_ritr = accompaniments_tmp.rbegin(); keyorder_ritr != accompaniments_tmp.rend(); keyorder_ritr++)
	{
		accompaniments.push_back(*keyorder_ritr);
	}

	return TRUE;
}

void WriteSKKUserDicEntry(FILE *fp, const std::wstring &key, const CANDIDATES &candidates)
{
	CANDIDATES::const_reverse_iterator candidates_ritr;
	std::wstring line;

	line = key + L" /";
	for(candidates_ritr = candidates.rbegin(); candidates_ritr != candidates.rend(); candidates_ritr++)
	{
		line += candidates_ritr->first;
		if(!candidates_ritr->second.empty())
		{
			line += L";" + candidates_ritr->second;
		}
		line += L"/";
	}

	fwprintf(fp, L"%s\n", line.c_str());
}

BOOL SaveSKKUserDic(USERDATA* userdata)
{
	FILE *fp;
	USERDIC::iterator userdic_itr;
	KEYORDER::reverse_iterator keyorder_ritr;

	_wfopen_s(&fp, pathuserdic, WccsUNICODE);
	if(fp == NULL)
	{
		return FALSE;
	}

	//送りありエントリ
	fwprintf(fp, L"%s", EntriesAri);

	for(keyorder_ritr = userdata->accompaniments.rbegin(); keyorder_ritr != userdata->accompaniments.rend(); keyorder_ritr++)
	{
		userdic_itr = userdata->userdic.find(*keyorder_ritr);
		if(userdic_itr != userdata->userdic.end())
		{
			WriteSKKUserDicEntry(fp, userdic_itr->first, userdic_itr->second);
		}
	}

	//送りなしエントリ
	fwprintf(fp, L"%s", EntriesNasi);

	for(keyorder_ritr = userdata->complements.rbegin(); keyorder_ritr != userdata->complements.rend(); keyorder_ritr++)
	{
		userdic_itr = userdata->userdic.find(*keyorder_ritr);
		if(userdic_itr != userdata->userdic.end())
		{
			WriteSKKUserDicEntry(fp, userdic_itr->first, userdic_itr->second);
		}
	}

	fclose(fp);

	return TRUE;
}

unsigned int __stdcall SaveSKKUserDicThreadEx(void *p)
{
	USERDATA *userdata = (USERDATA *)p;
	BOOL ret;

	EnterCriticalSection(&csUserDataSave);	// !

	ret = SaveSKKUserDic(userdata);

	LeaveCriticalSection(&csUserDataSave);	// !

	delete userdata;

	return 0;
}

HANDLE StartSaveSKKUserDicEx()
{
	HANDLE hThread = NULL;

	if(bUserDicChg)
	{
		bUserDicChg = FALSE;
		USERDATA *userdata = new USERDATA();
		userdata->userdic = userdic;
		userdata->complements = complements;
		userdata->accompaniments = accompaniments;

		hThread = (HANDLE)_beginthreadex(NULL, 0, SaveSKKUserDicThreadEx, userdata, 0, NULL);
	}

	return hThread;
}

void SaveSKKUserDicThreadExWaitThread(void *p)
{
	HANDLE hThread;
	
	hThread = StartSaveSKKUserDicEx();
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

void StartSaveSKKUserDic()
{
	_beginthread(SaveSKKUserDicThreadExWaitThread, 0, NULL);
}
