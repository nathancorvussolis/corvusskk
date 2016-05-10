
#include "utf8.h"
#include "imcrvmgr.h"

//
// request and reply commands
//
//search candidate
//  request "1\n<key>\t<key(original)>\t<okuri>\n"
//  reply   "T\n<candidate(display)>\t<candidate(register)>\t<annotation(display)>\t<annotation(register)>\n...\n":hit
//          "F\n":nothing
//search key for complement
//  request "4\n<key prefix>\t<candidate max>\t\n"
//  reply   "T\n<key>\t\t<candidates>\t\n...\n":hit
//          "F\n":nothing
//convert key
//  request "5\n<key>\t\t<okuri>\n"
//  reply   "T\n<key converted>\n...\n":hit
//          "F\n":nothing
//convert candidate
//  request "6\n<key>\t<candidate>\t<okuri>\n"
//  reply   "T\n<candidate converted>\n":hit
//          "F\n":nothing
//add candidate (complement off)
//  request "A\n<key>\t<candidate>\t<annotation>\t<okuri>\n"
//  reply   "T\n"
//add candidate (complement on)
//  request "B\n<key>\t<candidate>\t<annotation>\t\n"
//  reply   "T\n"
//delete candidate (complement off)
//  request "C\n<key>\t<candidate>\n"
//  reply   "T\n"
//delete candidate (complement on)
//  request "D\n<key>\t<candidate>\n"
//  reply   "T\n"
//save user dictionary
//  request "S\n"
//  reply   "T\n"
//create configuration dialog process
//  request "P\n"
//  reply   "T\n"
//

void SrvProc(WCHAR command, const std::wstring &argument, std::wstring &result)
{
	SKKDICCANDIDATES sc;
	std::wstring fmt, key, keyorg, okuri, candidate, annotation, conv;
	std::wregex re;

	result.clear();

	switch(command)
	{
	case REQ_SEARCH:
		re.assign(L"(.*)\t(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$2");
		keyorg = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$3");
		okuri = std::regex_replace(argument, re, fmt);

		SearchDictionary(key, okuri, sc);

		if(!sc.empty())
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
		re.assign(L"(.*)\t(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$2");
		keyorg = std::regex_replace(argument, re, fmt);

		if(lua != nullptr)
		{
			lua_getglobal(lua, u8"lua_skk_complement");
			lua_pushstring(lua, WCTOU8(key));
			if(lua_pcall(lua, 1, 1, 0) == LUA_OK)
			{
				if(lua_isstring(lua, -1))
				{
					candidate = U8TOWC(lua_tostring(lua, -1));
					ParseSKKDicCandiate(candidate, sc);
					FORWARD_ITERATION_I(sc_itr, sc)
					{
						sc_itr->first = ParseConcat(sc_itr->first);
					}
				}
			}
			lua_pop(lua, 1);
		}
		else
		{
			SearchComplement(key, sc);
		}

		SearchComplementSearchCandidate(sc, _wtoi(keyorg.c_str()));

		if(!sc.empty())
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
		re.assign(L"(.*)\t(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$2");
		candidate = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$3");
		okuri = std::regex_replace(argument, re, fmt);

		switch(command)
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

		if(!conv.empty())
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

	case REQ_USER_ADD_0:
	case REQ_USER_ADD_1:
		re.assign(L"(.*)\t(.*)\t(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$2");
		candidate = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$3");
		annotation = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$4");
		okuri = std::regex_replace(argument, re, fmt);

		if(lua != nullptr)
		{
			lua_getglobal(lua, u8"lua_skk_add");
			lua_pushboolean(lua, (command == REQ_USER_ADD_0 ? 1 : 0));
			lua_pushstring(lua, WCTOU8(key));
			lua_pushstring(lua, WCTOU8(candidate));
			lua_pushstring(lua, WCTOU8(annotation));
			lua_pushstring(lua, WCTOU8(okuri));
			lua_pcall(lua, 5, 1, 0);
		}
		else
		{
			AddUserDic(command, key, candidate, annotation, okuri);
		}

		result = REP_OK;
		result += L"\n";
		break;

	case REQ_USER_DEL_0:
	case REQ_USER_DEL_1:
		re.assign(L"(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$2");
		candidate = std::regex_replace(argument, re, fmt);

		if(lua != nullptr)
		{
			lua_getglobal(lua, u8"lua_skk_delete");
			lua_pushboolean(lua, (command == REQ_USER_DEL_0 ? 1 : 0));
			lua_pushstring(lua, WCTOU8(key));
			lua_pushstring(lua, WCTOU8(candidate));
			lua_pcall(lua, 3, 0, 0);
		}
		else
		{
			DelUserDic(command, key, candidate);
		}

		result = REP_OK;
		result += L"\n";
		break;

	case REQ_USER_SAVE:
		if(lua != nullptr)
		{
			lua_getglobal(lua, u8"lua_skk_save");
			lua_pcall(lua, 0, 0, 0);
		}
		else
		{
			StartSaveSKKUserDic(TRUE);
		}

		result = REP_OK;
		result += L"\n";
		break;

	case REQ_EXEC_CNF:
		StartProcess(hInst, IMCRVCNFEXE);

		result = REP_OK;
		result += L"\n";
		break;

	default:
		result = REP_FALSE;
		result += L"\n";
		break;
	}
}

unsigned int __stdcall SrvThread(void *p)
{
	HANDLE hPipe = (HANDLE)p;
	WCHAR pipebuf[PIPEBUFSIZE];
	DWORD bytesRead, bytesWrite;
	BOOL bRet;
	std::wstring wspipebuf;
	WCHAR command;
#ifdef _DEBUG
	std::wstring dedit, tedit;
	std::wregex re;
	std::wstring fmt;
#endif

	while(true)
	{
		if(ConnectNamedPipe(hPipe, nullptr) == FALSE)
		{
			DisconnectNamedPipe(hPipe);
			break;
		}

		if(bSrvThreadExit)
		{
			DisconnectNamedPipe(hPipe);
			break;
		}

		UpdateConfigPath();

		if(IsFileUpdated(pathconfigxml, &ftConfig))
		{
			LoadConfig();
		}

		if(IsFileUpdated(pathskkdic, &ftSKKDic))
		{
			MakeSKKDicPos();
		}

		ZeroMemory(pipebuf, sizeof(pipebuf));

		bytesRead = 0;
		bRet = ReadFile(hPipe, pipebuf, sizeof(pipebuf), &bytesRead, nullptr);
		if(bRet == FALSE || bytesRead == 0)
		{
			DisconnectNamedPipe(hPipe);
			continue;
		}

		command = pipebuf[0];

#ifdef _DEBUG
		tedit.assign(pipebuf);
		re.assign(L"\n");
		fmt.assign(L"↲\r\n");
		tedit = std::regex_replace(tedit, re, fmt);
		re.assign(L"\t");
		fmt.assign(L"»\u00A0");
		tedit = std::regex_replace(tedit, re, fmt);

		dedit.append(tedit);
		SetWindowTextW(hwndEdit, dedit.c_str());
#endif

		SrvProc(command, &pipebuf[2], wspipebuf);
		wcsncpy_s(pipebuf, wspipebuf.c_str(), _TRUNCATE);

#ifdef _DEBUG
		tedit.assign(pipebuf);
		re.assign(L"\n");
		fmt.assign(L"↲\r\n");
		tedit = std::regex_replace(tedit, re, fmt);
		re.assign(L"\t");
		fmt.assign(L"»\u00A0");
		tedit = std::regex_replace(tedit, re, fmt);

		dedit.append(tedit);
		SetWindowTextW(hwndEdit, dedit.c_str());
		SendMessageW(hwndEdit, WM_VSCROLL, SB_BOTTOM, 0);

		switch(command)
		{
		case REQ_USER_SAVE:
			dedit.clear();
			break;
		default:
			break;
		}
#endif

		bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
		bRet = WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr);
		if(bRet)
		{
			FlushFileBuffers(hPipe);
		}

		DisconnectNamedPipe(hPipe);
	}

	CloseHandle(hPipe);

	return 0;
}

HANDLE SrvStart()
{
	PSECURITY_DESCRIPTOR psd;
	SECURITY_ATTRIBUTES sa;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	HANDLE hThread = nullptr;

	if(ConvertStringSecurityDescriptorToSecurityDescriptorW(krnlobjsddl, SDDL_REVISION_1, &psd, nullptr))
	{
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = psd;
		sa.bInheritHandle = FALSE;

		hPipe = CreateNamedPipeW(mgrpipename, PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
			1, PIPEBUFSIZE * sizeof(WCHAR), PIPEBUFSIZE * sizeof(WCHAR), 0, &sa);

		LocalFree(psd);
	}

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		hThread = (HANDLE)_beginthreadex(nullptr, 0, SrvThread, hPipe, 0, nullptr);
	}

	if(hThread == nullptr)
	{
		CloseHandle(hPipe);
	}

	return hThread;
}
