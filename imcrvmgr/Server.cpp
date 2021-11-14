
#include "utf8.h"
#include "imcrvmgr.h"

void SrvProc(WCHAR command, const std::wstring &argument, std::wstring &result)
{
	SKKDICCANDIDATES sc;
	std::wstring key, keyorg, okuri, candidate, annotation, conv;

	// search, complement, convert key, convert candidate
	static const std::wregex research(L"(.*)\t(.*)\t(.*)\n");
	// add candidate
	static const std::wregex readd(L"(.*)\t(.*)\t(.*)\t(.*)\n");
	// delete candidate
	static const std::wregex redel(L"(.*)\t(.*)\n");

	result.clear();

	switch (command)
	{
	case REQ_SEARCH:
		if (!std::regex_match(argument, research)) break;

		key = std::regex_replace(argument, research, L"$1");
		keyorg = std::regex_replace(argument, research, L"$2");
		okuri = std::regex_replace(argument, research, L"$3");

		SearchDictionary(key, okuri, sc);

		if (!sc.empty())
		{
			result = REP_OK;
			result += L"\n";
			FORWARD_ITERATION_I(sc_itr, sc)
			{
				result += ConvertCandidate(keyorg, sc_itr->first, okuri) + L"\t" +
					sc_itr->first + L"\t" +
					ConvertCandidate(keyorg, sc_itr->second, okuri) + L"\t" +
					sc_itr->second + L"\n";
			}
		}
		else
		{
			result = REP_FALSE;
			result += L"\n";
		}
		break;

	case REQ_COMPLEMENT:
		if (!std::regex_match(argument, research)) break;

		key = std::regex_replace(argument, research, L"$1");
		keyorg = std::regex_replace(argument, research, L"$2");

		if (lua != nullptr)
		{
			lua_getglobal(lua, u8"lua_skk_complement");
			lua_pushstring(lua, WCTOU8(key));

			if (lua_pcall(lua, 1, 1, 0) == LUA_OK)
			{
				if (lua_isstring(lua, -1))
				{
					candidate = U8TOWC(lua_tostring(lua, -1));
					ParseSKKDicCandiate(candidate, sc);
					FORWARD_ITERATION_I(sc_itr, sc)
					{
						sc_itr->first = ParseConcat(sc_itr->first);
					}
				}
				lua_pop(lua, 1);
			}
		}
		else
		{
			SearchComplement(key, sc);
		}

		SearchComplementSearchCandidate(sc, _wtoi(keyorg.c_str()));

		if (!sc.empty())
		{
			result = REP_OK;
			result += L"\n";
			FORWARD_ITERATION_I(sc_itr, sc)
			{
				result += sc_itr->first + L"\t\t" + sc_itr->second + L"\t\n";
			}
		}
		else
		{
			result = REP_FALSE;
			result += L"\n";
		}
		break;

	case REQ_CONVERTKEY:
	case REQ_CONVERTCND:
		if (!std::regex_match(argument, research)) break;

		key = std::regex_replace(argument, research, L"$1");
		candidate = std::regex_replace(argument, research, L"$2");
		okuri = std::regex_replace(argument, research, L"$3");

		switch (command)
		{
		case REQ_CONVERTKEY:
			conv = ConvertKey(key, okuri);
			break;
		case REQ_CONVERTCND:
			conv = ConvertCandidate(key, candidate, okuri);
			break;
		default:
			break;
		}

		if (!conv.empty())
		{
			result = REP_OK;
			result += L"\n";
			result += conv + L"\n";
		}
		else
		{
			result = REP_FALSE;
			result += L"\n";
		}
		break;

	case REQ_REVERSE:
		if (!std::regex_match(argument, research)) break;

		candidate = std::regex_replace(argument, research, L"$1");

		result = REP_OK;

		if (lua != nullptr)
		{
			lua_getglobal(lua, u8"lua_skk_reverse");
			lua_pushstring(lua, WCTOU8(candidate));

			if (lua_pcall(lua, 1, 1, 0) == LUA_OK)
			{
				if (lua_isstring(lua, -1))
				{
					key = U8TOWC(lua_tostring(lua, -1));
				}
				lua_pop(lua, 1);
			}
		}
		else
		{
			SearchReverse(candidate, key);
		}

		if (!key.empty())
		{
			result = REP_OK;
			result += L"\n";
			result += key + L"\t" + key + L"\t\t\n";
		}
		else
		{
			result = REP_FALSE;
			result += L"\n";
		}
		break;

	case REQ_USER_ADD_A:
	case REQ_USER_ADD_N:
		if (!std::regex_match(argument, readd)) break;

		key = std::regex_replace(argument, readd, L"$1");
		candidate = std::regex_replace(argument, readd, L"$2");
		annotation = std::regex_replace(argument, readd, L"$3");
		okuri = std::regex_replace(argument, readd, L"$4");

		result = REP_OK;

		if (lua != nullptr)
		{
			lua_getglobal(lua, u8"lua_skk_add");
			lua_pushboolean(lua, (command == REQ_USER_ADD_A ? 1 : 0));
			lua_pushstring(lua, WCTOU8(key));
			lua_pushstring(lua, WCTOU8(candidate));
			lua_pushstring(lua, WCTOU8(annotation));
			lua_pushstring(lua, WCTOU8(okuri));

			if (lua_pcall(lua, 5, 0, 0) != LUA_OK)
			{
				result = REP_FALSE;
			}
		}
		else
		{
			AddUserDic(command, key, candidate, annotation, okuri);
		}

		result += L"\n";
		break;

	case REQ_USER_DEL_A:
	case REQ_USER_DEL_N:
		if (!std::regex_match(argument, redel)) break;

		key = std::regex_replace(argument, redel, L"$1");
		candidate = std::regex_replace(argument, redel, L"$2");

		result = REP_OK;

		if (lua != nullptr)
		{
			lua_getglobal(lua, u8"lua_skk_delete");
			lua_pushboolean(lua, (command == REQ_USER_DEL_A ? 1 : 0));
			lua_pushstring(lua, WCTOU8(key));
			lua_pushstring(lua, WCTOU8(candidate));

			if (lua_pcall(lua, 3, 0, 0) != LUA_OK)
			{
				result = REP_FALSE;
			}
		}
		else
		{
			DelUserDic(command, key, candidate);
		}

		result += L"\n";
		break;

	case REQ_USER_SAVE:
		result = REP_OK;

		if (lua != nullptr)
		{
			lua_getglobal(lua, u8"lua_skk_save");

			if (lua_pcall(lua, 0, 0, 0) != LUA_OK)
			{
				result = REP_FALSE;
			}
		}
		else
		{
			StartSaveUserDic(TRUE);
		}

		result += L"\n";
		break;

	case REQ_EXEC_CNF:
		StartProcess(hInst, IMCRVCNFEXE);

		result = REP_OK;
		result += L"\n";
		break;

	case REQ_CAPS_LOCK:
	case REQ_KANA_LOCK:
		SendKeyboardInput(command);

		result = REP_OK;
		result += L"\n";
		break;

	case REQ_BACKUP:
		StartSaveUserDic(FALSE);
		BackUpUserDic();

		result = REP_OK;
		result += L"\n";
		break;

	case REQ_EXIT:
		SendMessageW(hWndMgr, WM_CLOSE, 0, 0);

		result = REP_OK;
		result += L"\n";
		break;

	default:
		result = REP_FALSE;
		result += L"\n";
		break;
	}
}

unsigned __stdcall SrvThread(void *p)
{
	HANDLE hPipe = (HANDLE)p;
	DWORD bytesRead, bytesWrite;
	BOOL bRet;
	WCHAR command;
	std::wstring argument;
	std::wstring wspipebuf;

#ifdef _DEBUG
	std::wstring dedit, tedit;
	std::wregex re;
	std::wstring fmt;
#endif

	PWCHAR pipebuf = (PWCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * PIPEBUFSIZE);
	if (pipebuf == nullptr)
	{
		return -1;
	}

	while (true)
	{
		if (ConnectNamedPipe(hPipe, nullptr) == FALSE)
		{
			DisconnectNamedPipe(hPipe);
			break;
		}

		if (bSrvThreadExit)
		{
			DisconnectNamedPipe(hPipe);
			break;
		}

		UpdateConfigPath();

		if (IsFileModified(pathconfigxml, &ftConfig))
		{
			LoadConfig();
		}

		if (IsFileModified(pathskkdic, &ftSKKDic))
		{
			MakeSKKDicPos();
		}

		ZeroMemory(pipebuf, sizeof(WCHAR) * PIPEBUFSIZE);

		bytesRead = 0;
		bRet = ReadFile(hPipe, pipebuf, sizeof(WCHAR) * (PIPEBUFSIZE - 1), &bytesRead, nullptr);
		if (bRet == FALSE || bytesRead == 0)
		{
			DisconnectNamedPipe(hPipe);
			continue;
		}

		command = pipebuf[0];
		if (pipebuf[1] != L'\n') command = L'\0';
		argument.assign(&pipebuf[2]);

		wspipebuf.clear();

#ifdef _DEBUG
		tedit.assign(pipebuf);
		re.assign(L"\n");
		fmt.assign(L"↲\r\n");
		tedit = std::regex_replace(tedit, re, fmt);
		re.assign(L"\t");
		fmt.assign(L"»\u00A0");
		tedit = std::regex_replace(tedit, re, fmt);

		EnterCriticalSection(&csEdit);	// !

		if (dedit.size() > SHRT_MAX) dedit.clear();

		switch (command)
		{
		case REQ_USER_SAVE:
			dedit.clear();
			break;
		default:
			break;
		}

		dedit.append(tedit);

		LeaveCriticalSection(&csEdit);	// !

		PostMessageW(hWndMgr, WM_USER_SETTEXT, (WPARAM)hWndEdit, (LPARAM)dedit.c_str());
#endif

		SrvProc(command, argument, wspipebuf);

		wcsncpy_s(pipebuf, PIPEBUFSIZE, wspipebuf.c_str(), _TRUNCATE);

#ifdef _DEBUG
		tedit.assign(pipebuf);
		re.assign(L"\n");
		fmt.assign(L"↲\r\n");
		tedit = std::regex_replace(tedit, re, fmt);
		re.assign(L"\t");
		fmt.assign(L"»\u00A0");
		tedit = std::regex_replace(tedit, re, fmt);

		EnterCriticalSection(&csEdit);	// !

		dedit.append(tedit);

		LeaveCriticalSection(&csEdit);	// !

		PostMessageW(hWndMgr, WM_USER_SETTEXT, (WPARAM)hWndEdit, (LPARAM)dedit.c_str());
#endif

		bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
		bRet = WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr);
		if (bRet)
		{
			FlushFileBuffers(hPipe);
		}

		DisconnectNamedPipe(hPipe);
	}

	CloseHandle(hPipe);

	HeapFree(GetProcessHeap(), 0, pipebuf);

	return 0;
}

HANDLE SrvStart()
{
	PSECURITY_DESCRIPTOR psd;
	SECURITY_ATTRIBUTES sa;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	HANDLE hThread = nullptr;

	if (ConvertStringSecurityDescriptorToSecurityDescriptorW(krnlobjsddl, SDDL_REVISION_1, &psd, nullptr))
	{
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = psd;
		sa.bInheritHandle = FALSE;

		hPipe = CreateNamedPipeW(mgrpipename, PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
			1, PIPEBUFSIZE * sizeof(WCHAR), PIPEBUFSIZE * sizeof(WCHAR), 0, &sa);

		LocalFree(psd);
	}

	if (hPipe != INVALID_HANDLE_VALUE)
	{
		hThread = reinterpret_cast<HANDLE>(
			_beginthreadex(nullptr, 0, SrvThread, hPipe, 0, nullptr));
		if (hThread == nullptr)
		{
			CloseHandle(hPipe);
		}
	}

	return hThread;
}

/*
	request and reply commands

	search candidate
		request
			"1\n<key>\t<key(original)>\t<okuri>\n"
		reply
			"T\n<candidate(display)>\t<candidate(register)>\t<annotation(display)>\t<annotation(register)>\n...\n":hit
			"F\n":nothing

	search key for complement
		request
			"4\n<key prefix>\t<candidate max>\t\n"
		reply
			"T\n<key>\t\t<candidates>\t\n...\n":hit
			"F\n":nothing

	convert key
		request
			"5\n<key>\t\t<okuri>\n"
		reply
			"T\n<key converted>\n...\n":hit
			"F\n":nothing

	convert candidate
		request
			"6\n<key>\t<candidate>\t<okuri>\n"
		reply
			"T\n<candidate converted>\n":hit
			"F\n":nothing

	add candidate (complement off)
		request
			"A\n<key>\t<candidate>\t<annotation>\t<okuri>\n"
		reply
			"T\n"

	add candidate (complement on)
		request
			"B\n<key>\t<candidate>\t<annotation>\t\n"
		reply
			"T\n"

	delete candidate (complement off)
		request
			"C\n<key>\t<candidate>\n"
		reply
			"T\n"

	delete candidate (complement on)
		request
			"D\n<key>\t<candidate>\n"
		reply
			"T\n"

	save user dictionary
		request
			"S\n"
		reply
			"T\n"

	create configuration dialog process
		request
			"P\n"
		reply
			"T\n"

	caps lock
		request
			"I\n"
		reply
			"T\n"

	kana lock
		request
			"J\n"
		reply
			"T\n"

	backup
		request
			"R\n"
		reply
			"T\n"

	exit
		request
			"X\n"
		reply
			"T\n"
*/
