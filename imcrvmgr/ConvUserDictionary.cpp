
#include "configxml.h"
#include "parseskkdic.h"
#include "imcrvmgr.h"

void AddKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder);
void DelKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder);

void ConvUserDic(const std::wstring &searchkey,  const std::wstring &okuri, CANDIDATES &candidates)
{
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itr;
	CANDIDATES::reverse_iterator candidates_ritr;
	CANDIDATES::iterator okuri_itr;
	CANDIDATE okuric;
	USEROKURI::iterator userokuri_itr;
	OKURIBLOCKV::reverse_iterator okuriblockv_ritr;

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

	//送り仮名が一致した候補を優先する
	if(precedeokuri && !okuri.empty())
	{
		userokuri_itr = userokuri.find(searchkey);
		if(userokuri_itr != userokuri.end())
		{
			for(okuriblockv_ritr = userokuri_itr->second.o.rbegin(); okuriblockv_ritr !=  userokuri_itr->second.o.rend(); okuriblockv_ritr++)
			{
				if(okuriblockv_ritr->first == okuri)
				{
					for(okuri_itr = okuriblockv_ritr->second.begin(); okuri_itr != okuriblockv_ritr->second.end(); okuri_itr++)
					{
						for(candidates_itr = candidates.begin(); candidates_itr != candidates.end(); candidates_itr++)
						{
							if(candidates_itr->first == okuri_itr->first)
							{
								okuric = *candidates_itr;
								candidates.erase(candidates_itr);
								candidates.insert(candidates.begin(), okuric);
								break;
							}
						}
					}
					break;
				}
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

void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, const std::wstring &okuri)
{
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itr;
	USERDICENTRY userdicentry;
	std::wregex re;
	std::wstring fmt;
	std::wstring candidate_esc;
	std::wstring annotation_esc;
	USEROKURI::iterator userokuri_itr;
	USEROKURIENTRY userokurientry;
	CANDIDATES okuric;
	OKURIBLOCKV::iterator okuriblockv_itr;

	candidate_esc = candidate;

	// "/" -> \057, ";" -> \073
	re.assign(L"[/;]");
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

		candidate_esc = L"(concat \"" + candidate_esc + L"\")";
	}

	// "/" -> ""
	re.assign(L"/");
	fmt.assign(L"");
	annotation_esc = std::regex_replace(annotation, re, fmt);

	//ユーザー辞書
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

	//見出し語順序
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

	//ユーザー辞書送りブロック
	re.assign(L"[\\[\\]]"); //角括弧を含む候補を除外
	if(command == REQ_USER_ADD_0 && !okuri.empty() && !std::regex_search(candidate_esc, re))
	{
		userokuri_itr = userokuri.find(searchkey);
		if(userokuri_itr == userokuri.end())
		{
			okuric.push_back(CANDIDATE(candidate_esc, L""));
			userokurientry.first = searchkey;
			userokurientry.second.o.push_back(OKURIBLOCK(okuri, okuric));
			userokuri.insert(userokurientry);
		}
		else
		{
			for(okuriblockv_itr = userokuri_itr->second.o.begin(); okuriblockv_itr != userokuri_itr->second.o.end(); okuriblockv_itr++)
			{
				if(okuriblockv_itr->first == okuri)
				{
					for(candidates_itr = okuriblockv_itr->second.begin(); candidates_itr != okuriblockv_itr->second.end(); candidates_itr++)
					{
						if(candidates_itr->first == candidate_esc)
						{
							candidates_itr = okuriblockv_itr->second.erase(candidates_itr);
							okuriblockv_itr->second.push_back(CANDIDATE(candidate_esc, L""));
							candidates_itr = okuriblockv_itr->second.begin();
							break;
						}
					}
					if(candidates_itr == okuriblockv_itr->second.end())
					{
						okuriblockv_itr->second.push_back(CANDIDATE(candidate_esc, L""));
					}

					okuric = okuriblockv_itr->second;
					userokuri_itr->second.o.erase(okuriblockv_itr);
					userokuri_itr->second.o.push_back(OKURIBLOCK(okuri, okuric));
					okuriblockv_itr = userokuri_itr->second.o.begin();
					break;
				}
			}
			if(okuriblockv_itr == userokuri_itr->second.o.end())
			{
				okuric.push_back(CANDIDATE(candidate_esc, L""));
				userokuri_itr->second.o.push_back(OKURIBLOCK(okuri, okuric));
			}
		}
	}

	bUserDicChg = TRUE;
}

void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate)
{
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itr;
	USEROKURI::iterator userokuri_itr;
	OKURIBLOCKV::iterator okuriblockv_itr;

	//ユーザー辞書
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

			//見出し語順序
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

	//ユーザー辞書送りブロック
	userokuri_itr = userokuri.find(searchkey);
	if(userokuri_itr != userokuri.end())
	{
		for(okuriblockv_itr = userokuri_itr->second.o.begin(); okuriblockv_itr != userokuri_itr->second.o.end(); )
		{
			for(candidates_itr = okuriblockv_itr->second.begin(); candidates_itr != okuriblockv_itr->second.end(); )
			{
				if(candidates_itr->first == candidate)
				{
					candidates_itr = okuriblockv_itr->second.erase(candidates_itr);
				}
				else
				{
					candidates_itr++;
				}
			}
			if(okuriblockv_itr->second.empty())
			{
				okuriblockv_itr = userokuri_itr->second.o.erase(okuriblockv_itr);
			}
			else
			{
				okuriblockv_itr++;
			}
		}
		if(userokuri_itr->second.o.empty())
		{
			userokuri.erase(userokuri_itr);
		}
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
	std::wstring key, empty;
	KEYORDER complements_tmp;
	KEYORDER accompaniments_tmp;
	KEYORDER::reverse_iterator keyorder_ritr;
	int okuri = -1;
	int rl;
	SKKDICCANDIDATES sc;
	SKKDICCANDIDATES::iterator sc_itr;
	SKKDICCANDIDATES::reverse_iterator sc_ritr;
	SKKDICOKURIBLOCKS so;
	SKKDICOKURIBLOCKS::iterator so_itr;
	SKKDICCANDIDATES::iterator sc_itr1;
	SKKDICOKURIBLOCKS::iterator so_itr1;
	USEROKURI::iterator userokuri_itr;
	USEROKURIENTRY userokurientry;

	_wfopen_s(&fp, pathuserdic, RccsUNICODE);
	if(fp == NULL)
	{
		return FALSE;
	}

	while(true)
	{
		rl = ReadSKKDicLine(fp, 0xFEFF, okuri, key, sc, so);
		if(rl == -1)
		{
			break;
		}
		else if(rl == 1)
		{
			continue;
		}

		//ユーザー辞書
		for(sc_ritr = sc.rbegin(); sc_ritr != sc.rend(); sc_ritr++)
		{
			AddUserDic(WCHAR_MAX, key, sc_ritr->first, sc_ritr->second, empty);
		}

		//見出し語順序
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

		if(okuri == 1)
		{
			//ユーザー辞書送りブロック
			userokuri_itr = userokuri.find(key);
			if(userokuri_itr == userokuri.end())
			{
				//送り仮名を1文字に限定する
				for(so_itr = so.begin(); so_itr != so.end(); so_itr++)
				{
					if(so_itr->first.size() >= 2 &&
						IS_SURROGATE_PAIR(so_itr->first.c_str()[0], so_itr->first.c_str()[1]))
					{
						so_itr->first = so_itr->first.substr(0, 2);
					}
					else
					{
						so_itr->first = so_itr->first.substr(0, 1);
					}
				}

				//送り仮名が重複したら1つにまとめる
				for(so_itr = so.begin(); so_itr != so.end(); so_itr++)
				{
					for(so_itr1 = so_itr + 1; so_itr1 != so.end(); )
					{
						if(so_itr->first == so_itr1->first)
						{
							for(sc_itr1 = so_itr1->second.begin(); sc_itr1 != so_itr1->second.end(); sc_itr1++)
							{
								for(sc_itr = so_itr->second.begin(); sc_itr != so_itr->second.end(); sc_itr++)
								{
									if(sc_itr->first == sc_itr1->first)
									{
										break;
									}
								}
								if(sc_itr == so_itr->second.end())
								{
									so_itr->second.push_back(*sc_itr1);
								}
							}
							so_itr1 = so.erase(so_itr1);
						}
						else
						{
							so_itr1++;
						}
					}
				}

				//候補にない送りブロックの候補を除外
				for(so_itr = so.begin(); so_itr != so.end(); )
				{
					for(sc_itr = so_itr->second.begin(); sc_itr != so_itr->second.end(); )
					{
						for(sc_ritr = sc.rbegin(); sc_ritr != sc.rend(); sc_ritr++)
						{
							if(sc_itr->first == sc_ritr->first)
							{
								break;
							}
						}
						if(sc_ritr == sc.rend())
						{
							sc_itr = so_itr->second.erase(sc_itr);
						}
						else
						{
							sc_itr++;
						}
					}
					if(so_itr->second.empty())
					{
						so_itr = so.erase(so_itr);
					}
					else
					{
						so_itr++;
					}
				}

				if(!so.empty())
				{
					userokurientry.first = key;
					userokurientry.second.o = so;
					userokuri.insert(userokurientry);
				}
			}
		}
	}

	fclose(fp);

	//見出し語順序 末尾を最新とする
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

void WriteSKKUserDicEntry(FILE *fp, const std::wstring &key,
	const CANDIDATES &candidates, const OKURIBLOCKS &okuriblocks)
{
	CANDIDATES::const_reverse_iterator candidates_ritr;
	OKURIBLOCKV::const_reverse_iterator okuriblockv_ritr;
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

	for(okuriblockv_ritr = okuriblocks.o.rbegin(); okuriblockv_ritr != okuriblocks.o.rend(); okuriblockv_ritr++)
	{
		line += L"[" + okuriblockv_ritr->first + L"/";
		for(candidates_ritr = okuriblockv_ritr->second.rbegin(); candidates_ritr !=  okuriblockv_ritr->second.rend(); candidates_ritr++)
		{
			line += candidates_ritr->first + L"/";
		}
		line += L"]/";
	}

	fwprintf(fp, L"%s\n", line.c_str());
}

BOOL SaveSKKUserDic(USERDATA* userdata)
{
	FILE *fp;
	USERDIC::iterator userdic_itr;
	KEYORDER::reverse_iterator keyorder_ritr;
	USEROKURI::iterator userokuri_itr;
	OKURIBLOCKS okuriblocks;

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
			okuriblocks.o.clear();
			userokuri_itr = userdata->userokuri.find(*keyorder_ritr);
			if(userokuri_itr != userdata->userokuri.end())
			{
				okuriblocks = userokuri_itr->second;
			}
			WriteSKKUserDicEntry(fp, userdic_itr->first, userdic_itr->second, okuriblocks);
		}
	}

	okuriblocks.o.clear();

	//送りなしエントリ
	fwprintf(fp, L"%s", EntriesNasi);

	for(keyorder_ritr = userdata->complements.rbegin(); keyorder_ritr != userdata->complements.rend(); keyorder_ritr++)
	{
		userdic_itr = userdata->userdic.find(*keyorder_ritr);
		if(userdic_itr != userdata->userdic.end())
		{
			WriteSKKUserDicEntry(fp, userdic_itr->first, userdic_itr->second, okuriblocks);
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
		userdata->userokuri = userokuri;
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
