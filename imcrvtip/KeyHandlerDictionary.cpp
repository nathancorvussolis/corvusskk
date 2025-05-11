﻿
#include "imcrvtip.h"
#include "TextService.h"

void CTextService::_ConnectDic()
{
	DWORD dwMode;

	if (WaitNamedPipeW(mgrpipename, NMPWAIT_USE_DEFAULT_WAIT) == 0)
	{
		return;
	}

	hPipe = CreateFileW(mgrpipename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_EFFECTIVE_ONLY | SECURITY_IDENTIFICATION, nullptr);
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		return;
	}

	dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
	if (SetNamedPipeHandleState(hPipe, &dwMode, nullptr, nullptr) == FALSE)
	{
		_DisconnectDic();
		return;
	}
}

void CTextService::_DisconnectDic()
{
	if (hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}

void CTextService::_SearchDic(WCHAR command)
{
	DWORD bytesWrite, bytesRead;
	std::wstring s, se, scd, scr, sad, sar, okurikey;
	std::wsmatch m;

	const std::wregex &re = RegExp(L"(.*)\t(.*)\t(.*)\t(.*)\n");

	candidates.clear();
	candidates.shrink_to_fit();

	_StartManager();

	_ConnectDic();

	ZeroMemory(pipebuf, sizeof(pipebuf));

	if (okuriidx != 0)
	{
		okurikey = kana.substr(okuriidx + 1);
		if (okurikey.size() >= 2 &&
			IS_SURROGATE_PAIR(okurikey.c_str()[0], okurikey.c_str()[1]))
		{
			okurikey = okurikey.substr(0, 2);
		}
		else
		{
			okurikey = okurikey.substr(0, 1);
		}
	}

	_snwprintf_s(pipebuf, _TRUNCATE, L"%c\n%s\t%s\t%s\n",
		command, searchkey.c_str(), searchkeyorg.c_str(), okurikey.c_str());

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(pipebuf, sizeof(pipebuf));

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf) - sizeof(WCHAR), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

	if ((pipebuf[0] != REP_OK) || (pipebuf[1] != L'\n'))
	{
		goto exit;
	}

	s.assign(&pipebuf[2]);

	while (std::regex_search(s, m, re))
	{
		se = m.str();
		s = m.suffix().str();

		scd = std::regex_replace(se, re, L"$1");
		scr = std::regex_replace(se, re, L"$2");
		sad = std::regex_replace(se, re, L"$3");
		sar = std::regex_replace(se, re, L"$4");

		if (scd.empty())
		{
			scd = scr;
		}
		if (sad.empty())
		{
			sad = sar;
		}

		candidates.push_back(std::make_pair(
			std::make_pair(scd, sad), std::make_pair(scr, sar)));
	}

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	_DisconnectDic();
}

void CTextService::_ConvertWord(WCHAR command, const std::wstring &key, const std::wstring &candidate, const std::wstring &okuri)
{
	DWORD bytesWrite, bytesRead;
	PWCHAR pn;

	convword.clear();

	_StartManager();

	_ConnectDic();

	ZeroMemory(pipebuf, sizeof(pipebuf));

	_snwprintf_s(pipebuf, _TRUNCATE, L"%c\n%s\t%s\t%s\n",
		command, key.c_str(), candidate.c_str(), okuri.c_str());

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(pipebuf, sizeof(pipebuf));

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf) - sizeof(WCHAR), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

	if ((pipebuf[0] != REP_OK) || (pipebuf[1] != L'\n'))
	{
		goto exit;
	}

	pn = &pipebuf[wcslen(pipebuf) - 1];
	if (*pn == L'\n')
	{
		*pn = L'\0';
		convword.assign(&pipebuf[2]);
	}

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	_DisconnectDic();
}

void CTextService::_AddUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate, const std::wstring &annotation)
{
	//Private Mode
	if (_IsPrivateMode()) return;

	DWORD bytesWrite, bytesRead;
	std::wstring okurikey;

	_ConnectDic();

	ZeroMemory(pipebuf, sizeof(pipebuf));

	if (okuriidx != 0)
	{
		okurikey = kana.substr(okuriidx + 1);
		if (okurikey.size() >= 2 &&
			IS_SURROGATE_PAIR(okurikey.c_str()[0], okurikey.c_str()[1]))
		{
			okurikey = okurikey.substr(0, 2);
		}
		else
		{
			okurikey = okurikey.substr(0, 1);
		}
	}

	_snwprintf_s(pipebuf, _TRUNCATE, L"%c\n%s\t%s\t%s\t%s\n",
		command, key.c_str(), candidate.c_str(), annotation.c_str(), okurikey.c_str());

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(pipebuf, sizeof(pipebuf));

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf) - sizeof(WCHAR), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	_DisconnectDic();
}

void CTextService::_DelUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate)
{
	//Private Mode
	if (_IsPrivateMode()) return;

	DWORD bytesWrite, bytesRead;

	_ConnectDic();

	ZeroMemory(pipebuf, sizeof(pipebuf));

	_snwprintf_s(pipebuf, _TRUNCATE, L"%c\n%s\t%s\n",
		command, key.c_str(), candidate.c_str());

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(pipebuf, sizeof(pipebuf));

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf) - sizeof(WCHAR), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	_DisconnectDic();
}

void CTextService::_SaveUserDic()
{
	_CommandDic(REQ_USER_SAVE);
}

void CTextService::_CommandDic(WCHAR command)
{
	DWORD bytesWrite, bytesRead;

	_ConnectDic();

	pipebuf[0] = command;
	pipebuf[1] = L'\n';
	pipebuf[2] = L'\0';

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf) - sizeof(WCHAR), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	_DisconnectDic();
}

void CTextService::_StartManager()
{
	HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, mgrmutexname);
	if (hMutex != nullptr)
	{
		CloseHandle(hMutex);
		return;
	}

	StartProcess(g_hInst, IMCRVMGREXE);
}

void CTextService::_StartConfigure()
{
	HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, cnfmutexname);
	if (hMutex != nullptr)
	{
		CloseHandle(hMutex);
		return;
	}

	AllowSetForegroundWindow(ASFW_ANY);

	_CommandDic(REQ_EXEC_CNF);
}
