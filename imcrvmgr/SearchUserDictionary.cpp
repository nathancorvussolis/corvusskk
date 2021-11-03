
#include "configxml.h"
#include "imcrvmgr.h"

#define MAX_COMPLEMENT_RESULT 256

//ユーザー辞書
SKKDIC userdic;
USEROKURI userokuri;
//送りなし、補完あり
KEYORDER keyorder_n;
//送りあり、補完なし
KEYORDER keyorder_a;

std::wstring SearchUserDic(const std::wstring &searchkey, const std::wstring &okuri)
{
	std::wstring candidate;
	SKKDICCANDIDATES sc;
	SKKDICCANDIDATE okuric;

	EnterCriticalSection(&csUserData);	// !

	auto userdic_itr = userdic.find(searchkey);
	if (userdic_itr != userdic.end())
	{
		sc = userdic_itr->second;
		std::reverse(sc.begin(), sc.end());
	}

	//送り仮名が一致した候補を優先する
	if (precedeokuri && !okuri.empty())
	{
		auto userokuri_itr = userokuri.find(searchkey);
		if (userokuri_itr != userokuri.end())
		{
			REVERSE_ITERATION_I(so_ritr, userokuri_itr->second.o)
			{
				if (so_ritr->first == okuri)
				{
					FORWARD_ITERATION_I(okuri_itr, so_ritr->second)
					{
						FORWARD_ITERATION_I(sc_itr, sc)
						{
							if (sc_itr->first == okuri_itr->first)
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

	LeaveCriticalSection(&csUserData);	// !

	FORWARD_ITERATION_I(sc_itr, sc)
	{
		candidate.append(L"/" + sc_itr->first);
		if (!sc_itr->second.empty())
		{
			candidate.append(L";" + sc_itr->second);
		}
	}
	if (!candidate.empty())
	{
		candidate += L"/\n";
	}

	return candidate;
}

void SearchComplement(const std::wstring &searchkey, SKKDICCANDIDATES &sc)
{
	size_t count = 0;

	EnterCriticalSection(&csUserData);	// !

	if (!keyorder_n.empty())
	{
		REVERSE_ITERATION_I(keyorder_ritr, keyorder_n)
		{
			if (count >= MAX_COMPLEMENT_RESULT)
			{
				break;
			}

			if (searchkey.size() < keyorder_ritr->size())
			{
				if (keyorder_ritr->compare(0, searchkey.size(), searchkey) == 0)
				{
					//前方一致
					sc.push_back(std::make_pair(*keyorder_ritr, std::wstring(L"")));
					++count;
				}
				else if (compincback &&
					keyorder_ritr->compare((keyorder_ritr->size() - searchkey.size()),
					searchkey.size(), searchkey) == 0)
				{
					//後方一致
					sc.push_back(std::make_pair(*keyorder_ritr, std::wstring(L"")));
					++count;
				}
			}
		}
	}

	LeaveCriticalSection(&csUserData);	// !
}

void SearchComplementSearchCandidate(SKKDICCANDIDATES &sc, int max)
{
	std::wstring candidate, conv;
	SKKDICCANDIDATES scc;

	if (max >= 9 || max <= 0)
	{
		return;
	}

	FORWARD_ITERATION_I(sc_itr, sc)
	{
		candidate = SearchUserDic(sc_itr->first, L"");

		static const std::wregex rectrl(L"[\\x00-\\x19]");
		candidate = std::regex_replace(candidate, rectrl, L"");

		scc.clear();

		ParseSKKDicCandiate(candidate, scc);

		int i = 0;
		FORWARD_ITERATION_I(scc_itr, scc)
		{
			if (max <= i++)
			{
				break;
			}

			conv = ConvertCandidate(sc_itr->first, scc_itr->first, L"");
			if (conv.empty())
			{
				conv = scc_itr->first;
			}
			sc_itr->second += L"/" + conv;
		}

		if (!sc_itr->second.empty())
		{
			sc_itr->second += L"/";

			if (i < (int)scc.size())
			{
				sc_itr->second += L"…";
			}
		}
	}
}

void DelKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder)
{
	EnterCriticalSection(&csUserData);	// !

	if (!keyorder.empty())
	{
		FORWARD_ITERATION_I(keyorder_itr, keyorder)
		{
			if (searchkey == *keyorder_itr)
			{
				keyorder.erase(keyorder_itr);
				break;
			}
		}
	}

	LeaveCriticalSection(&csUserData);	// !
}

void AddKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder)
{
	EnterCriticalSection(&csUserData);	// !

	DelKeyOrder(searchkey, keyorder);

	keyorder.push_back(searchkey);

	LeaveCriticalSection(&csUserData);	// !
}

void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, const std::wstring &okuri)
{
	SKKDICENTRY userdicentry;
	std::wstring candidate_esc;
	std::wstring annotation_esc;
	USEROKURIENTRY userokurientry;
	SKKDICCANDIDATES okurics;

	if (searchkey.empty() || candidate.empty())
	{
		return;
	}

	candidate_esc = MakeConcat(EscapeGadgetString(candidate));

	annotation_esc = MakeConcat(annotation);

	EnterCriticalSection(&csUserData);	// !

	//ユーザー辞書
	auto userdic_itr = userdic.find(searchkey);
	if (userdic_itr == userdic.end())
	{
		userdicentry.first = searchkey;
		userdicentry.second.push_back(std::make_pair(candidate_esc, annotation_esc));
		userdic.insert(userdicentry);
	}
	else
	{
		FORWARD_ITERATION_I(sc_itr, userdic_itr->second)
		{
			if (sc_itr->first == candidate_esc)
			{
				userdic_itr->second.erase(sc_itr);
				break;
			}
		}
		userdic_itr->second.push_back(std::make_pair(candidate_esc, annotation_esc));
	}

	//見出し語順序
	switch (command)
	{
	case REQ_USER_ADD_A:
		AddKeyOrder(searchkey, keyorder_a);
		break;
	case REQ_USER_ADD_N:
		AddKeyOrder(searchkey, keyorder_n);
		break;
	default:
		break;
	}

	//ユーザー辞書送りブロック
	static const std::wregex reblock(L"[\\[\\]]"); //角括弧を含む候補を除外
	if (command == REQ_USER_ADD_A && !okuri.empty() && !std::regex_search(candidate_esc, reblock))
	{
		auto userokuri_itr = userokuri.find(searchkey);
		if (userokuri_itr == userokuri.end())
		{
			okurics.push_back(std::make_pair(candidate_esc, std::wstring(L"")));
			userokurientry.first = searchkey;
			userokurientry.second.o.push_back(std::make_pair(okuri, okurics));
			userokuri.insert(userokurientry);
		}
		else
		{
			bool hit_okuri = false;
			FORWARD_ITERATION_I(so_itr, userokuri_itr->second.o)
			{
				if (so_itr->first == okuri)
				{
					bool hit_candidate = false;
					FORWARD_ITERATION_I(sc_itr, so_itr->second)
					{
						if (sc_itr->first == candidate_esc)
						{
							sc_itr = so_itr->second.erase(sc_itr);
							so_itr->second.push_back(std::make_pair(candidate_esc, std::wstring(L"")));
							sc_itr = so_itr->second.begin();
							hit_candidate = true;
							break;
						}
					}
					if (!hit_candidate)
					{
						so_itr->second.push_back(std::make_pair(candidate_esc, std::wstring(L"")));
					}

					okurics = so_itr->second;
					userokuri_itr->second.o.erase(so_itr);
					userokuri_itr->second.o.push_back(std::make_pair(okuri, okurics));
					so_itr = userokuri_itr->second.o.begin();
					hit_okuri = true;
					break;
				}
			}
			if (!hit_okuri)
			{
				okurics.push_back(std::make_pair(candidate_esc, std::wstring(L"")));
				userokuri_itr->second.o.push_back(std::make_pair(okuri, okurics));
			}
		}
	}

	LeaveCriticalSection(&csUserData);	// !

	bUserDicChg = TRUE;
}

void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate)
{
	EnterCriticalSection(&csUserData);	// !

	//ユーザー辞書
	auto userdic_itr = userdic.find(searchkey);
	if (userdic_itr != userdic.end())
	{
		FORWARD_ITERATION_I(sc_itr, userdic_itr->second)
		{
			if (sc_itr->first == candidate)
			{
				userdic_itr->second.erase(sc_itr);
				break;
			}
		}

		if (userdic_itr->second.empty())
		{
			userdic.erase(userdic_itr);

			//見出し語順序
			switch (command)
			{
			case REQ_USER_DEL_A:
				DelKeyOrder(searchkey, keyorder_a);
				break;
			case REQ_USER_DEL_N:
				DelKeyOrder(searchkey, keyorder_n);
				break;
			default:
				break;
			}
		}

		bUserDicChg = TRUE;
	}

	//ユーザー辞書送りブロック
	auto userokuri_itr = userokuri.find(searchkey);
	if (userokuri_itr != userokuri.end())
	{
		FORWARD_ITERATION(so_itr, userokuri_itr->second.o)
		{
			FORWARD_ITERATION(sc_itr, so_itr->second)
			{
				if (sc_itr->first == candidate)
				{
					sc_itr = so_itr->second.erase(sc_itr);
				}
				else
				{
					++sc_itr;
				}
			}
			if (so_itr->second.empty())
			{
				so_itr = userokuri_itr->second.o.erase(so_itr);
			}
			else
			{
				++so_itr;
			}
		}
		if (userokuri_itr->second.o.empty())
		{
			userokuri.erase(userokuri_itr);
		}
	}

	LeaveCriticalSection(&csUserData);	// !
}

BOOL LoadUserDic()
{
	BOOL ret = FALSE;
	FILE *fp = nullptr;
	std::wstring key, empty;
	int okuri = -1;	//-1:header / 1:okuri-ari entries. / 0:okuri-nasi entries.
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;
	USEROKURIENTRY userokurientry;

	EnterCriticalSection(&csUserData);	// !

	userdic.clear();
	userokuri.clear();
	keyorder_n.clear();
	keyorder_a.clear();

	EnterCriticalSection(&csUserDict);	// !

	_wfopen_s(&fp, pathuserdic, modeRccsUTF8);	//UTF-8 or UTF-16LE(with BOM)
	if (fp == nullptr)
	{
		goto exit;
	}

	while (true)
	{
		int rl = ReadSKKDicLine(fp, enc_utf_8, okuri, key, sc, so);
		if (rl == -1)
		{
			//EOF
			break;
		}
		else if (rl == 1)
		{
			//comment
			continue;
		}

		if (sc.empty())
		{
			continue;
		}

		//見出し語順序
		auto userdic_itr = userdic.find(key);
		if (userdic_itr == userdic.end())
		{
			switch (okuri)
			{
			case 0:
				keyorder_n.push_back(key);
				break;
			case 1:
				keyorder_a.push_back(key);
				break;
			default:
				break;
			}
		}

		//ユーザー辞書
		REVERSE_ITERATION_I(sc_ritr, sc)
		{
			AddUserDic(WCHAR_MAX, key, sc_ritr->first, sc_ritr->second, empty);
		}

		if (okuri == 1)
		{
			//ユーザー辞書送りブロック
			auto userokuri_itr = userokuri.find(key);
			if (userokuri_itr == userokuri.end())
			{
				//送り仮名を1文字に限定する
				FORWARD_ITERATION_I(so_itr, so)
				{
					if (so_itr->first.size() >= 2)
					{
						if (IS_SURROGATE_PAIR(so_itr->first[0], so_itr->first[1]))
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
					for (auto so_itr1 = so_itr + 1; so_itr1 != so.end(); )
					{
						if (so_itr->first == so_itr1->first)
						{
							FORWARD_ITERATION_I(sc_itr1, so_itr1->second)
							{
								bool exist = false;
								FORWARD_ITERATION_I(sc_itr, so_itr->second)
								{
									if (sc_itr->first == sc_itr1->first)
									{
										exist = true;
										break;
									}
								}
								if (!exist)
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
							if (sc_itr->first == sc_ritr->first)
							{
								exist = true;
								break;
							}
						}
						if (!exist)
						{
							sc_itr = so_itr->second.erase(sc_itr);
						}
						else
						{
							++sc_itr;
						}
					}
					if (so_itr->second.empty())
					{
						so_itr = so.erase(so_itr);
					}
					else
					{
						++so_itr;
					}
				}

				if (!so.empty())
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
	std::reverse(keyorder_n.begin(), keyorder_n.end());
	std::reverse(keyorder_a.begin(), keyorder_a.end());

	ret = TRUE;

exit:
	LeaveCriticalSection(&csUserDict);	// !

	LeaveCriticalSection(&csUserData);	// !

	return ret;
}

void WriteUserDicEntry(FILE *fp, const std::wstring &key, const SKKDICCANDIDATES &sc, const SKKDICOKURIBLOCKS &so)
{
	std::wstring line;

	line = key + L" /";
	REVERSE_ITERATION_I(sc_ritr, sc)
	{
		line += sc_ritr->first;
		if (!sc_ritr->second.empty())
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

void SaveUserDic(USERDATA *userdata)
{
	FILE *fp = nullptr;
	SKKDICOKURIBLOCKS so;

	if (userdata == nullptr)
	{
		return;
	}

	EnterCriticalSection(&csUserDict);	// !

	_wfopen_s(&fp, pathuserdic, modeWccsUTF16);
	if (fp == nullptr)
	{
		goto exit;
	}

	//送りありエントリ
	fwprintf(fp, L"%s", EntriesAri);

	REVERSE_ITERATION_I(keyorder_ritr, userdata->keyorder_a)
	{
		auto userdic_itr = userdata->userdic.find(*keyorder_ritr);
		if (userdic_itr != userdata->userdic.end())
		{
			so.clear();
			auto userokuri_itr = userdata->userokuri.find(*keyorder_ritr);
			if (userokuri_itr != userdata->userokuri.end())
			{
				so = userokuri_itr->second.o;
			}
			WriteUserDicEntry(fp, userdic_itr->first, userdic_itr->second, so);
		}
	}

	so.clear();

	//送りなしエントリ
	fwprintf(fp, L"%s", EntriesNasi);

	REVERSE_ITERATION_I(keyorder_ritr, userdata->keyorder_n)
	{
		auto userdic_itr = userdata->userdic.find(*keyorder_ritr);
		if (userdic_itr != userdata->userdic.end())
		{
			WriteUserDicEntry(fp, userdic_itr->first, userdic_itr->second, so);
		}
	}

	fclose(fp);

exit:
	LeaveCriticalSection(&csUserDict);	// !
}

unsigned __stdcall SaveUserDicThread(void *p)
{
	USERDATA *userdata = (USERDATA *)p;

	if (userdata != nullptr)
	{
		if (TryEnterCriticalSection(&csSaveUserDic))	// !
		{
			SaveUserDic(userdata);

			LeaveCriticalSection(&csSaveUserDic);	// !
		}

		delete userdata;
	}

	return 0;
}

void StartSaveUserDic(BOOL bThread)
{
	if (bUserDicChg)
	{
		USERDATA *userdata = nullptr;

		EnterCriticalSection(&csUserData);	// !

		try
		{
			userdata = new USERDATA();
			userdata->userdic = userdic;
			userdata->userokuri = userokuri;
			userdata->keyorder_n = keyorder_n;
			userdata->keyorder_a = keyorder_a;
		}
		catch (...)
		{
			if (userdata != nullptr)
			{
				delete userdata;
				userdata = nullptr;
			}
		}

		LeaveCriticalSection(&csUserData);	// !

		if (userdata == nullptr)
		{
			return;
		}

		if (bThread)
		{
			HANDLE h = reinterpret_cast<HANDLE>(
				_beginthreadex(nullptr, 0, SaveUserDicThread, userdata, 0, nullptr));

			if (h != nullptr)
			{
				CloseHandle(h);

				bUserDicChg = FALSE;
			}
			else
			{
				delete userdata;
			}
		}
		else
		{
			SaveUserDic(userdata);

			bUserDicChg = FALSE;

			delete userdata;
		}
	}
}

void BackUpUserDic()
{
	WCHAR path[MAX_PATH];
	LPCWSTR ext = L"bak";

	EnterCriticalSection(&csUserDict);	// !

	// バックアップディレクトリ作成

	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	_wsplitpath_s(pathbackup, drive, _countof(drive), dir, _countof(dir), nullptr, 0, nullptr, 0);
	_wmakepath_s(path, drive, dir, nullptr, nullptr);
	SHCreateDirectoryExW(nullptr, path, nullptr);

	// バックアップ

	SYSTEMTIME st = {};
	GetSystemTime(&st);

	_snwprintf_s(path, _TRUNCATE, L"%s.%04d%02d%02dT%02d%02d%02dZ.%s",
		pathbackup,
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond,
		ext);

	CopyFileW(pathuserdic, path, FALSE);

	// バックアップファイル検索

	std::vector<std::wstring> filenames;
	WIN32_FIND_DATAW fd;

	_snwprintf_s(path, _TRUNCATE, L"%s.*.%s", pathbackup, ext);

	HANDLE hFind = FindFirstFileW(path, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		filenames.push_back(fd.cFileName);

		while (FindNextFileW(hFind, &fd))
		{
			filenames.push_back(fd.cFileName);
		}

		FindClose(hFind);
	}

	// 昇順ソート
	std::sort(filenames.begin(), filenames.end());

	// 古いバックアップファイルを削除
	int len = (int)filenames.size() - generation;
	for (int i = 0; i < len; i++)
	{
		_wmakepath_s(path, drive, dir, filenames[i].c_str(), nullptr);

		DeleteFileW(path);
	}

	LeaveCriticalSection(&csUserDict);	// !
}
