
#include "corvustip.h"
#include "TextService.h"

#define BUFSIZE 0x2000	// -> corvussrv.cpp

void CTextService::_ConvDic(WCHAR command)
{
	WCHAR wbuf[BUFSIZE];
	DWORD bytesWrite, bytesRead;
	const WCHAR tab = L'\t';
	const WCHAR nl = L'\n';
	size_t i, ic, ia;
	std::wstring s, sc, sa;

	ZeroMemory(wbuf, sizeof(wbuf));

	_snwprintf_s(wbuf, _TRUNCATE, L"%c\n%s\n", command, searchkey.c_str());

	if(WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}

	ZeroMemory(wbuf, sizeof(wbuf));

	if(ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}

	if(wbuf[0] != REP_OK)
	{
		return;
	}

	s.assign(wbuf);
	i = 1;

	while(true)
	{
		ic = s.find_first_of(tab, i + 1);
		if(ic == std::wstring::npos)
		{
			break;
		}
		ia = s.find_first_of(nl, ic + 1);
		if(ia == std::wstring::npos)
		{
			break;
		}
		sc = s.substr(i + 1, ic - (i + 1));
		sa = s.substr(ic + 1, ia - (ic + 1));
		candidates.push_back(CANDIDATE(CANDIDATEBASE(sc, sa), CANDIDATEBASE(sc, sa)));
		i = ia;
	}
}

void CTextService::_ConnectDic()
{
	DWORD dwMode;

	if(WaitNamedPipeW(pipename, NMPWAIT_USE_DEFAULT_WAIT) == 0)
	{
		_StartDicSrv();
	}

	hPipe = CreateFileW(pipename, GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hPipe == INVALID_HANDLE_VALUE)
	{
		return;
	}

	dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
	SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
}

void CTextService::_DisonnectDic()
{
	if(hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}

void CTextService::_CheckAliveDic()
{
	WCHAR buf;
	DWORD bytes;

	buf = REQ_CHECK_ALIVE;
	if(WriteFile(hPipe, &buf, 2, &bytes, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}

	if(ReadFile(hPipe, &buf, 2, &bytes, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}
}

void CTextService::_AddUserDic(const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation, WCHAR command)
{
	WCHAR wbuf[BUFSIZE];
	DWORD bytesWrite, bytesRead;

	ZeroMemory(wbuf, sizeof(wbuf));

	_snwprintf_s(wbuf, _TRUNCATE, L"%c\n%s\t%s\t%s\n",
		command, searchkey.c_str(), candidate.c_str(), annotation.c_str());

	//接続確認
	_CheckAliveDic();

	if(WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}

	ZeroMemory(wbuf, sizeof(wbuf));

	if(ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}
}

void CTextService::_DelUserDic(const std::wstring &searchkey, const std::wstring &candidate)
{
	WCHAR wbuf[BUFSIZE];
	DWORD bytesWrite, bytesRead;

	ZeroMemory(wbuf, sizeof(wbuf));

	_snwprintf_s(wbuf, _TRUNCATE, L"%c\n%s\t%s\n",
		REQ_USER_DEL, searchkey.c_str(), candidate.c_str());

	//接続確認
	_CheckAliveDic();

	if(WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}

	ZeroMemory(wbuf, sizeof(wbuf));

	if(ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}
}

void CTextService::_SaveUserDic()
{
	WCHAR buf;
	DWORD bytes;

	buf = REQ_USER_SAVE;
	if(WriteFile(hPipe, &buf, 2, &bytes, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}

	if(ReadFile(hPipe, &buf, 2, &bytes, NULL) == FALSE)
	{
		_ConnectDic();
		return;
	}
}

void CTextService::_StartDicSrv()
{
	HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, CORVUSSRVMUTEX);
	if(hMutex != NULL)
	{
		CloseHandle(hMutex);
		return;
	}

	_StartProcess(CORVUSSRVEXE);
}

void CTextService::_StartConfigure()
{
	HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, CORVUSCNFMUTEX);
	if(hMutex != NULL)
	{
		CloseHandle(hMutex);
		return;
	}

	_StartProcess(CORVUSCNFEXE);
}

void CTextService::_StartProcess(const WCHAR *fname)
{
	WCHAR path[MAX_PATH];
	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	PROCESS_INFORMATION pi;
	STARTUPINFOW si;

	GetModuleFileNameW(g_hInst, path, _countof(path));
	_wsplitpath_s(path, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);
	_snwprintf_s(path, _TRUNCATE, L"%s%s%s", drive, dir, fname);
	ZeroMemory(&si,sizeof(si));
	si.cb = sizeof(si);
	CreateProcessW(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}

void CTextService::_ConvDicNum()
{
	CANDIDATES::iterator candidates_itr;

	//数値を#に置換
	searchkey = std::regex_replace(searchkey, std::wregex(L"[0-9]+"), std::wstring(L"#"));

	if(searchkey == searchkeyorg)
	{
		return;
	}

	//数値を「#」にしたキーで検索
	_ConvDic(REQ_SEARCH);

	if(!candidates.empty())
	{
		for(candidates_itr = candidates.begin(); candidates_itr != candidates.end(); candidates_itr++)
		{
			// "12がつ31にち", "#3月#3日"　→　"十二月三十一日"
			_ConvNum(candidates_itr->first.first, searchkeyorg, candidates_itr->first.first);
		}
	}
}

void CTextService::_ConvNum(std::wstring &convnum, const std::wstring &key, const std::wstring &candidate)
{
	const WCHAR *keyint[] = {L"#0", L"#1", L"#2", L"#3"};
	const WCHAR *kannum[] = {L"〇", L"一", L"二", L"三", L"四", L"五", L"六", L"七", L"八", L"九"};
	const WCHAR *kancl1[] = {L"",   L"十", L"百", L"千"};
	const WCHAR *kancl2[] = {L"",   L"万", L"億", L"兆", L"京"};
	size_t j, k, m, n, p, q, r;
	bool kancl2flg;
	std::wregex rxnum(L"[0-9]+");
	std::wregex rxint(L"#[0-3]");
	std::wstring searchkey_tmp = key;
	std::wstring candidate_tmp = candidate;
	std::wstring repcandidate;
	std::wsmatch result;
	std::vector< std::wstring > keynum;
	std::vector< std::wstring > septype3;

	while(std::regex_search(searchkey_tmp, result, rxnum))
	{
		keynum.push_back(result.str());
		searchkey_tmp = result.suffix();
	}

	j = 0;
	while(std::regex_search(candidate_tmp, result, rxint) && (j < keynum.size()))
	{
		repcandidate.append(result.prefix());
		switch(result.str().back())
		{
		case L'0':	//5500
			repcandidate.append(keynum[j]);
			break;
		case L'1':	//５５００
			for(k=0; k<keynum[j].size(); k++)
			{
				repcandidate.push_back((L'０'-L'0')/*(0xFF10-0x0030)*/ + keynum[j][k]);
			}
			break;
		case L'2':	//五五〇〇
			for(k=0; k<keynum[j].size(); k++)
			{
				repcandidate.append(kannum[(keynum[j][k] - L'0'/*0x0030*/)]);
			}
			break;
		case L'3':	//五千五百
			r = keynum[j].size();
			if(r > 20)	// 10^20 < X
			{
				repcandidate.append(keynum[j]);
				break;
			}
			septype3.clear();
			if((r % 4) != 0)
			{
				// 1234567890 -> "12" 34567890
				septype3.push_back(keynum[j].substr(0, (r % 4)));
			}
			m = ((r - (r % 4)) / 4);
			for(k=0; k<m; k++)
			{
				// 1234567890 -> 12 "3456" "7890"
				septype3.push_back(keynum[j].substr(((r % 4) + (k * 4)), 4));
			}
			m = septype3.size();
			for(p=0; p<septype3.size(); p++)
			{
				kancl2flg = false;
				n = septype3[p].size();
				for(q=0; q<n; q++)
				{
					if(septype3[p][q] != L'0' || (m == 1 && n == 1))
					{
						//十の位と百の位の「一」は表記しない。
						if((septype3[p][q] != L'1') ||		//二～九
								((n == 4) && (q == 0)) ||	//千の位
								(q == n - 1))  				//一の位
						{
							repcandidate.append(kannum[septype3[p][q] - L'0']);
						}
						repcandidate.append(kancl1[(n - q - 1) % 4]);
						kancl2flg = true;
					}
				}
				if(kancl2flg)
				{
					repcandidate.append(kancl2[m - p - 1]);
				}
			}
			break;
		default:
			repcandidate.append(keynum[j]);
			break;
		}
		candidate_tmp = result.suffix();
		++j;
	}
	repcandidate.append(candidate_tmp);
	convnum = repcandidate;
}
