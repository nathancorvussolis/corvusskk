
#include "configxml.h"
#include "imcrvmgr.h"

#define MAX_COMPLEMENT_RESULT 1024

//ユーザー辞書
SKKDIC userdic;
USEROKURI userokuri;
//補完あり
KEYORDER complements;
//補完なし
KEYORDER accompaniments;

HANDLE hThreadUserDic = INVALID_HANDLE_VALUE;

std::wstring SearchUserDic(const std::wstring &searchkey,  const std::wstring &okuri)
{
	std::wstring candidate;
	SKKDICCANDIDATES sc;
	SKKDICCANDIDATE okuric;

	auto userdic_itr = userdic.find(searchkey);
	if(userdic_itr != userdic.end())
	{
		sc = userdic_itr->second;
		std::reverse(sc.begin(), sc.end());
	}

	//送り仮名が一致した候補を優先する
	if(precedeokuri && !okuri.empty())
	{
		auto userokuri_itr = userokuri.find(searchkey);
		if(userokuri_itr != userokuri.end())
		{
			REVERSE_ITERATION_I(so_ritr, userokuri_itr->second.o)
			{
				if(so_ritr->first == okuri)
				{
					FORWARD_ITERATION_I(okuri_itr, so_ritr->second)
					{
						FORWARD_ITERATION_I(sc_itr, sc)
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

	FORWARD_ITERATION_I(sc_itr, sc)
	{
		candidate.append(L"/" + sc_itr->first);
		if(!sc_itr->second.empty())
		{
			candidate.append(L";" + sc_itr->second);
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
	size_t count = 0;

	if(!complements.empty())
	{
		REVERSE_ITERATION_I(keyorder_ritr, complements)
		{
			if(count >= MAX_COMPLEMENT_RESULT)
			{
				break;
			}

			if(searchkey.size() < keyorder_ritr->size())
			{
				if(keyorder_ritr->compare(0, searchkey.size(), searchkey) == 0)
				{
					//前方一致
					sc.push_back(SKKDICCANDIDATE(*keyorder_ritr, L""));
					++count;
				}
				else if(compincback &&
					keyorder_ritr->compare((keyorder_ritr->size() - searchkey.size()),
					searchkey.size(), searchkey) == 0)
				{
					//後方一致
					sc.push_back(SKKDICCANDIDATE(*keyorder_ritr, L""));
					++count;
				}
			}
		}
	}
}

void SearchComplementSearchCandidate(SKKDICCANDIDATES &sc, int max)
{
	std::wstring candidate, conv;
	std::wregex re(L"[\\x00-\\x19]");
	std::wstring fmt(L"");
	SKKDICCANDIDATES scc;

	if(max >= 9 || max <= 0)
	{
		return;
	}

	FORWARD_ITERATION_I(sc_itr, sc)
	{
		candidate = SearchUserDic(sc_itr->first, L"");

		candidate = std::regex_replace(candidate, re, fmt);

		scc.clear();

		ParseSKKDicCandiate(candidate, scc);

		int i = 0;
		FORWARD_ITERATION_I(scc_itr, scc)
		{
			if(max <= i++)
			{
				break;
			}

			conv = ConvertCandidate(sc_itr->first, scc_itr->first, L"");
			if(conv.empty())
			{
				conv = scc_itr->first;
			}
			sc_itr->second += L"/" + conv;
		}

		if(!sc_itr->second.empty())
		{
			sc_itr->second += L"/";

			if(i < (int)scc.size())
			{
				sc_itr->second += L"…";
			}
		}
	}
}

void DelKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder)
{
	if(!keyorder.empty())
	{
		FORWARD_ITERATION_I(keyorder_itr, keyorder)
		{
			if(searchkey == *keyorder_itr)
			{
				keyorder.erase(keyorder_itr);
				break;
			}
		}
	}
}

void AddKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder)
{
	DelKeyOrder(searchkey, keyorder);

	keyorder.push_back(searchkey);
}

void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, const std::wstring &okuri)
{
	SKKDICENTRY userdicentry;
	std::wregex re;
	std::wstring candidate_esc;
	std::wstring annotation_esc;
	USEROKURIENTRY userokurientry;
	SKKDICCANDIDATES okurics;

	if(searchkey.empty() || candidate.empty())
	{
		return;
	}

	candidate_esc = MakeConcat(candidate);
	annotation_esc = MakeConcat(annotation);

	//ユーザー辞書
	auto userdic_itr = userdic.find(searchkey);
	if(userdic_itr == userdic.end())
	{
		userdicentry.first = searchkey;
		userdicentry.second.push_back(SKKDICCANDIDATE(candidate_esc, annotation_esc));
		userdic.insert(userdicentry);
	}
	else
	{
		FORWARD_ITERATION_I(sc_itr, userdic_itr->second)
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
		auto userokuri_itr = userokuri.find(searchkey);
		if(userokuri_itr == userokuri.end())
		{
			okurics.push_back(SKKDICCANDIDATE(candidate_esc, L""));
			userokurientry.first = searchkey;
			userokurientry.second.o.push_back(SKKDICOKURIBLOCK(okuri, okurics));
			userokuri.insert(userokurientry);
		}
		else
		{
			bool hit_okuri = false;
			FORWARD_ITERATION_I(so_itr, userokuri_itr->second.o)
			{
				if(so_itr->first == okuri)
				{
					bool hit_candidate = false;
					FORWARD_ITERATION_I(sc_itr, so_itr->second)
					{
						if(sc_itr->first == candidate_esc)
						{
							sc_itr = so_itr->second.erase(sc_itr);
							so_itr->second.push_back(SKKDICCANDIDATE(candidate_esc, L""));
							sc_itr = so_itr->second.begin();
							hit_candidate = true;
							break;
						}
					}
					if(!hit_candidate)
					{
						so_itr->second.push_back(SKKDICCANDIDATE(candidate_esc, L""));
					}

					okurics = so_itr->second;
					userokuri_itr->second.o.erase(so_itr);
					userokuri_itr->second.o.push_back(SKKDICOKURIBLOCK(okuri, okurics));
					so_itr = userokuri_itr->second.o.begin();
					hit_okuri = true;
					break;
				}
			}
			if(!hit_okuri)
			{
				okurics.push_back(SKKDICCANDIDATE(candidate_esc, L""));
				userokuri_itr->second.o.push_back(SKKDICOKURIBLOCK(okuri, okurics));
			}
		}
	}

	bUserDicChg = TRUE;
}

void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate)
{
	//ユーザー辞書
	auto userdic_itr = userdic.find(searchkey);
	if(userdic_itr != userdic.end())
	{
		FORWARD_ITERATION_I(sc_itr, userdic_itr->second)
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
	auto userokuri_itr = userokuri.find(searchkey);
	if(userokuri_itr != userokuri.end())
	{
		FORWARD_ITERATION(so_itr, userokuri_itr->second.o)
		{
			FORWARD_ITERATION(sc_itr, so_itr->second)
			{
				if(sc_itr->first == candidate)
				{
					sc_itr = so_itr->second.erase(sc_itr);
				}
				else
				{
					++sc_itr;
				}
			}
			if(so_itr->second.empty())
			{
				so_itr = userokuri_itr->second.o.erase(so_itr);
			}
			else
			{
				++so_itr;
			}
		}
		if(userokuri_itr->second.o.empty())
		{
			userokuri.erase(userokuri_itr);
		}
	}
}

BOOL LoadUserDic()
{
	FILE *fp;
	std::wstring key, empty;
	int okuri = -1;	//-1:header / 1:okuri-ari entries. / 0:okuri-nasi entries.
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;
	USEROKURIENTRY userokurientry;

	userdic.clear();
	userokuri.clear();
	complements.clear();
	accompaniments.clear();

	_wfopen_s(&fp, pathuserdic, RccsUTF8);	//UTF-8 or UTF-16LE(with BOM)
	if(fp == nullptr)
	{
		return FALSE;
	}

	while(true)
	{
		int rl = ReadSKKDicLine(fp, BOM, okuri, key, sc, so);
		if(rl == -1)
		{
			//EOF
			break;
		}
		else if(rl == 1)
		{
			//comment
			continue;
		}

		if(sc.empty())
		{
			continue;
		}

		//ユーザー辞書
		REVERSE_ITERATION_I(sc_ritr, sc)
		{
			AddUserDic(WCHAR_MAX, key, sc_ritr->first, sc_ritr->second, empty);
		}

		//見出し語順序
		switch(okuri)
		{
		case 0:
			complements.push_back(key);
			break;
		case 1:
			accompaniments.push_back(key);
			break;
		default:
			break;
		}

		if(okuri == 1)
		{
			//ユーザー辞書送りブロック
			auto userokuri_itr = userokuri.find(key);
			if(userokuri_itr == userokuri.end())
			{
				//送り仮名を1文字に限定する
				FORWARD_ITERATION_I(so_itr, so)
				{
					if(so_itr->first.size() >= 2)
					{
						if(IS_SURROGATE_PAIR(so_itr->first[0], so_itr->first[1]))
						{
							so_itr->first.erase(2);
						}
						else
						{
							so_itr->first.erase(1);
						}
					}
				}

				//送り仮名が重複したら1つにまとめる
				FORWARD_ITERATION_I(so_itr, so)
				{
					for(auto so_itr1 = so_itr + 1; so_itr1 != so.end(); )
					{
						if(so_itr->first == so_itr1->first)
						{
							FORWARD_ITERATION_I(sc_itr1, so_itr1->second)
							{
								bool exist = false;
								FORWARD_ITERATION_I(sc_itr, so_itr->second)
								{
									if(sc_itr->first == sc_itr1->first)
									{
										exist = true;
										break;
									}
								}
								if(!exist)
								{
									so_itr->second.push_back(*sc_itr1);
								}
							}
							so_itr1 = so.erase(so_itr1);
						}
						else
						{
							++so_itr1;
						}
					}
				}

				//候補にない送りブロックの候補を除外
				FORWARD_ITERATION(so_itr, so)
				{
					FORWARD_ITERATION(sc_itr, so_itr->second)
					{
						bool exist = false;
						REVERSE_ITERATION_I(sc_ritr, sc)
						{
							if(sc_itr->first == sc_ritr->first)
							{
								exist = true;
								break;
							}
						}
						if(!exist)
						{
							sc_itr = so_itr->second.erase(sc_itr);
						}
						else
						{
							++sc_itr;
						}
					}
					if(so_itr->second.empty())
					{
						so_itr = so.erase(so_itr);
					}
					else
					{
						++so_itr;
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
	std::reverse(complements.begin(), complements.end());
	std::reverse(accompaniments.begin(), accompaniments.end());

	return TRUE;
}

void WriteUserDicEntry(FILE *fp, const std::wstring &key, const SKKDICCANDIDATES &sc, const SKKDICOKURIBLOCKS &so)
{
	std::wstring line;

	line = key + L" /";
	REVERSE_ITERATION_I(sc_ritr, sc)
	{
		line += sc_ritr->first;
		if(!sc_ritr->second.empty())
		{
			line += L";" + sc_ritr->second;
		}
		line += L"/";
	}

	REVERSE_ITERATION_I(so_ritr, so)
	{
		line += L"[" + so_ritr->first + L"/";
		REVERSE_ITERATION_I(sc_ritr, so_ritr->second)
		{
			line += sc_ritr->first + L"/";
		}
		line += L"]/";
	}

	fwprintf(fp, L"%s\n", line.c_str());
}

unsigned int __stdcall SaveUserDic(void *p)
{
	USERDATA *userdata = (USERDATA *)p;
	FILE *fp;
	SKKDICOKURIBLOCKS so;

	if(userdata == nullptr)
	{
		return 0;
	}

	EnterCriticalSection(&csSaveUserDic);	// !

	_wfopen_s(&fp, pathuserdic, WccsUTF16);
	if(fp == nullptr)
	{
		goto exit;
	}

	//送りありエントリ
	fwprintf(fp, L"%s", EntriesAri);

	REVERSE_ITERATION_I(keyorder_ritr, userdata->accompaniments)
	{
		auto userdic_itr = userdata->userdic.find(*keyorder_ritr);
		if(userdic_itr != userdata->userdic.end())
		{
			so.clear();
			auto userokuri_itr = userdata->userokuri.find(*keyorder_ritr);
			if(userokuri_itr != userdata->userokuri.end())
			{
				so = userokuri_itr->second.o;
			}
			WriteUserDicEntry(fp, userdic_itr->first, userdic_itr->second, so);
		}
	}

	so.clear();

	//送りなしエントリ
	fwprintf(fp, L"%s", EntriesNasi);

	REVERSE_ITERATION_I(keyorder_ritr, userdata->complements)
	{
		auto userdic_itr = userdata->userdic.find(*keyorder_ritr);
		if(userdic_itr != userdata->userdic.end())
		{
			WriteUserDicEntry(fp, userdic_itr->first, userdic_itr->second, so);
		}
	}

	fclose(fp);

exit:
	LeaveCriticalSection(&csSaveUserDic);	// !

	delete userdata;

	return 0;
}

void StartSaveUserDic(BOOL bThread)
{
	if(hThreadUserDic != INVALID_HANDLE_VALUE)
	{
		DWORD ws = WaitForSingleObject(hThreadUserDic, (bThread ? 0 : INFINITE));
		switch(ws)
		{
		case WAIT_OBJECT_0:
			CloseHandle(hThreadUserDic);
			hThreadUserDic = INVALID_HANDLE_VALUE;
			break;
		case WAIT_TIMEOUT:
			return;
			break;
		default:
			break;
		}
	}

	if(bUserDicChg)
	{
		USERDATA *userdata = nullptr;

		try
		{
			userdata = new USERDATA();
			userdata->userdic = userdic;
			userdata->userokuri = userokuri;
			userdata->complements = complements;
			userdata->accompaniments = accompaniments;
		}
		catch(...)
		{
			if(userdata != nullptr)
			{
				delete userdata;
			}
			return;
		}

		if(bThread)
		{
			if(hThreadUserDic == INVALID_HANDLE_VALUE)
			{
				hThreadUserDic = (HANDLE)_beginthreadex(nullptr, 0, SaveUserDic, userdata, 0, nullptr);
				if(hThreadUserDic == nullptr)
				{
					delete userdata;
					hThreadUserDic = INVALID_HANDLE_VALUE;
				}
				else
				{
					bUserDicChg = FALSE;
				}
			}
		}
		else
		{
			SaveUserDic(userdata);
			bUserDicChg = FALSE;
		}
	}
}

void BackUpUserDic()
{
	WCHAR oldpath[MAX_PATH];
	WCHAR newpath[MAX_PATH];

	EnterCriticalSection(&csSaveUserDic);	// !

	for(int i = BACKUP_GENS; i > 1; i--)
	{
		_snwprintf_s(oldpath, _TRUNCATE, L"%s%d", pathuserbak, i - 1);
		_snwprintf_s(newpath, _TRUNCATE, L"%s%d", pathuserbak, i);

		MoveFileExW(oldpath, newpath, MOVEFILE_REPLACE_EXISTING);
	}

	_snwprintf_s(newpath, _TRUNCATE, L"%s%d", pathuserbak, 1);

	CopyFileW(pathuserdic, newpath, FALSE);

	LeaveCriticalSection(&csSaveUserDic);	// !
}
