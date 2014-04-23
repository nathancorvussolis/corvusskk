
#include "configxml.h"
#include "parseskkdic.h"
#include "imcrvmgr.h"

void AddKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder);
void DelKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder);

std::wstring SearchUserDic(const std::wstring &searchkey,  const std::wstring &okuri)
{
	std::wstring candidate;
	SKKDICCANDIDATES sc;
	SKKDIC::iterator userdic_itr;
	SKKDICCANDIDATES::iterator sc_itr;
	SKKDICCANDIDATES::reverse_iterator sc_ritr;
	SKKDICCANDIDATES::iterator okuri_itr;
	SKKDICCANDIDATE okuric;
	USEROKURI::iterator userokuri_itr;
	SKKDICOKURIBLOCKS::reverse_iterator so_ritr;

	userdic_itr = userdic.find(searchkey);
	if(userdic_itr != userdic.end())
	{
		for(sc_ritr = userdic_itr->second.rbegin(); sc_ritr != userdic_itr->second.rend(); sc_ritr++)
		{
			if(!sc_ritr->first.empty())
			{
				sc.push_back(*sc_ritr);
			}
		}
	}

	//送り仮名が一致した候補を優先する
	if(precedeokuri && !okuri.empty())
	{
		userokuri_itr = userokuri.find(searchkey);
		if(userokuri_itr != userokuri.end())
		{
			for(so_ritr = userokuri_itr->second.o.rbegin(); so_ritr !=  userokuri_itr->second.o.rend(); so_ritr++)
			{
				if(so_ritr->first == okuri)
				{
					for(okuri_itr = so_ritr->second.begin(); okuri_itr != so_ritr->second.end(); okuri_itr++)
					{
						for(sc_itr = sc.begin(); sc_itr != sc.end(); sc_itr++)
						{
							if(sc_itr->first == okuri_itr->first)
							{
								okuric = *sc_itr;
								sc.erase(sc_itr);
								sc.insert(sc.begin(), okuric);
								break;
							}
						}
					}
					break;
				}
			}
		}
	}
	
	for(sc_itr = sc.begin(); sc_itr != sc.end(); sc_itr++)
	{
		if(!sc_itr->first.empty())
		{
			candidate.append(L"/" + sc_itr->first);
		}
	}
	if(!candidate.empty())
	{
		candidate += L"/\n";
	}

	return candidate;
}

void SearchComplement(const std::wstring &searchkey, SKKDICCANDIDATES &sc)
{
	KEYORDER::reverse_iterator keyorder_ritr;

	if(!complements.empty())
	{
		for(keyorder_ritr = complements.rbegin(); keyorder_ritr != complements.rend(); keyorder_ritr++)
		{
			if(keyorder_ritr->compare(0, searchkey.size(), searchkey) == 0)
			{
				sc.push_back(SKKDICCANDIDATE(*keyorder_ritr, L""));
			}
		}
	}
}

std::wstring EscapeDicChar(const std::wstring &s)
{
	std::wstring ret = s;
	std::wregex re;
	std::wstring fmt;

	// "/" -> \057, ";" -> \073
	re.assign(L"[/;]");
	if(std::regex_search(ret, re))
	{
		// "\"" -> "\\\"", "\\" -> "\\\\"
		re.assign(L"([\\\"|\\\\])");
		fmt.assign(L"\\$1");
		ret = std::regex_replace(ret, re, fmt);

		re.assign(L"/");
		fmt.assign(L"\\057");
		ret = std::regex_replace(ret, re, fmt);

		re.assign(L";");
		fmt.assign(L"\\073");
		ret = std::regex_replace(ret, re, fmt);

		ret = L"(concat \"" + ret + L"\")";
	}

	return ret;
}

void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, const std::wstring &okuri)
{
	SKKDIC::iterator userdic_itr;
	SKKDICCANDIDATES::iterator sc_itr;
	SKKDICENTRY userdicentry;
	std::wregex re;
	std::wstring candidate_esc;
	std::wstring annotation_esc;
	USEROKURI::iterator userokuri_itr;
	USEROKURIENTRY userokurientry;
	SKKDICCANDIDATES okuric;
	SKKDICOKURIBLOCKS::iterator so_itr;

	candidate_esc = EscapeDicChar(candidate);
	annotation_esc = EscapeDicChar(annotation);

	//ユーザー辞書
	userdic_itr = userdic.find(searchkey);
	if(userdic_itr == userdic.end())
	{
		userdicentry.first = searchkey;
		userdicentry.second.push_back(SKKDICCANDIDATE(candidate_esc, annotation_esc));
		userdic.insert(userdicentry);
	}
	else
	{
		for(sc_itr = userdic_itr->second.begin(); sc_itr != userdic_itr->second.end(); sc_itr++)
		{
			if(sc_itr->first == candidate_esc)
			{
				userdic_itr->second.erase(sc_itr);
				break;
			}
		}
		userdic_itr->second.push_back(SKKDICCANDIDATE(candidate_esc, annotation_esc));
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
			okuric.push_back(SKKDICCANDIDATE(candidate_esc, L""));
			userokurientry.first = searchkey;
			userokurientry.second.o.push_back(SKKDICOKURIBLOCK(okuri, okuric));
			userokuri.insert(userokurientry);
		}
		else
		{
			for(so_itr = userokuri_itr->second.o.begin(); so_itr != userokuri_itr->second.o.end(); so_itr++)
			{
				if(so_itr->first == okuri)
				{
					for(sc_itr = so_itr->second.begin(); sc_itr != so_itr->second.end(); sc_itr++)
					{
						if(sc_itr->first == candidate_esc)
						{
							sc_itr = so_itr->second.erase(sc_itr);
							so_itr->second.push_back(SKKDICCANDIDATE(candidate_esc, L""));
							sc_itr = so_itr->second.begin();
							break;
						}
					}
					if(sc_itr == so_itr->second.end())
					{
						so_itr->second.push_back(SKKDICCANDIDATE(candidate_esc, L""));
					}

					okuric = so_itr->second;
					userokuri_itr->second.o.erase(so_itr);
					userokuri_itr->second.o.push_back(SKKDICOKURIBLOCK(okuri, okuric));
					so_itr = userokuri_itr->second.o.begin();
					break;
				}
			}
			if(so_itr == userokuri_itr->second.o.end())
			{
				okuric.push_back(SKKDICCANDIDATE(candidate_esc, L""));
				userokuri_itr->second.o.push_back(SKKDICOKURIBLOCK(okuri, okuric));
			}
		}
	}

	bUserDicChg = TRUE;
}

void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate)
{
	SKKDIC::iterator userdic_itr;
	SKKDICCANDIDATES::iterator sc_itr;
	USEROKURI::iterator userokuri_itr;
	SKKDICOKURIBLOCKS::iterator so_itr;

	//ユーザー辞書
	userdic_itr = userdic.find(searchkey);
	if(userdic_itr != userdic.end())
	{
		for(sc_itr = userdic_itr->second.begin(); sc_itr != userdic_itr->second.end(); sc_itr++)
		{
			if(sc_itr->first == candidate)
			{
				userdic_itr->second.erase(sc_itr);
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
		for(so_itr = userokuri_itr->second.o.begin(); so_itr != userokuri_itr->second.o.end(); )
		{
			for(sc_itr = so_itr->second.begin(); sc_itr != so_itr->second.end(); )
			{
				if(sc_itr->first == candidate)
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
				so_itr = userokuri_itr->second.o.erase(so_itr);
			}
			else
			{
				so_itr++;
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

void WriteSKKUserDicEntry(FILE *fp, const std::wstring &key, const SKKDICCANDIDATES &sc, const SKKDICOKURIBLOCKS &so)
{
	SKKDICCANDIDATES::const_reverse_iterator sc_ritr;
	SKKDICOKURIBLOCKS::const_reverse_iterator so_ritr;
	std::wstring line;

	line = key + L" /";
	for(sc_ritr = sc.rbegin(); sc_ritr != sc.rend(); sc_ritr++)
	{
		line += sc_ritr->first;
		if(!sc_ritr->second.empty())
		{
			line += L";" + sc_ritr->second;
		}
		line += L"/";
	}

	for(so_ritr = so.rbegin(); so_ritr != so.rend(); so_ritr++)
	{
		line += L"[" + so_ritr->first + L"/";
		for(sc_ritr = so_ritr->second.rbegin(); sc_ritr !=  so_ritr->second.rend(); sc_ritr++)
		{
			line += sc_ritr->first + L"/";
		}
		line += L"]/";
	}

	fwprintf(fp, L"%s\n", line.c_str());
}

BOOL SaveSKKUserDic(USERDATA* userdata)
{
	FILE *fp;
	SKKDIC::iterator userdic_itr;
	KEYORDER::reverse_iterator keyorder_ritr;
	USEROKURI::iterator userokuri_itr;
	SKKDICOKURIBLOCKS so;

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
			so.clear();
			userokuri_itr = userdata->userokuri.find(*keyorder_ritr);
			if(userokuri_itr != userdata->userokuri.end())
			{
				so = userokuri_itr->second.o;
			}
			WriteSKKUserDicEntry(fp, userdic_itr->first, userdic_itr->second, so);
		}
	}

	so.clear();

	//送りなしエントリ
	fwprintf(fp, L"%s", EntriesNasi);

	for(keyorder_ritr = userdata->complements.rbegin(); keyorder_ritr != userdata->complements.rend(); keyorder_ritr++)
	{
		userdic_itr = userdata->userdic.find(*keyorder_ritr);
		if(userdic_itr != userdata->userdic.end())
		{
			WriteSKKUserDicEntry(fp, userdic_itr->first, userdic_itr->second, so);
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
