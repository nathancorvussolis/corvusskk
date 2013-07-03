
#include "imcrvtip.h"
#include "TextService.h"

void CTextService::_ConnectDic()
{
	DWORD dwMode;

	if(WaitNamedPipeW(mgrpipename, NMPWAIT_USE_DEFAULT_WAIT) == 0)
	{
		return;
	}

	hPipe = CreateFileW(mgrpipename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_EFFECTIVE_ONLY | SECURITY_IDENTIFICATION, NULL);
	if(hPipe == INVALID_HANDLE_VALUE)
	{
		return;
	}

	dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
	SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
}

void CTextService::_DisconnectDic()
{
	if(hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}

void CTextService::_SearchDic(WCHAR command)
{
	WCHAR wbuf[PIPEBUFSIZE];
	DWORD bytesWrite, bytesRead;
	size_t i, icd, icr, ia;
	std::wstring s, scd, scr, sa;

	_StartManager();

	_ConnectDic();

	ZeroMemory(wbuf, sizeof(wbuf));

	_snwprintf_s(wbuf, _TRUNCATE, L"%c\n%s\t%s\n",
		command, searchkey.c_str(), searchkeyorg.c_str());

	if(WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(wbuf, sizeof(wbuf));

	if(ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL) == FALSE)
	{
		goto exit;
	}

	if(wbuf[0] != REP_OK)
	{
		goto exit;
	}

	s.assign(wbuf);
	i = 1;

	while(true)
	{
		icd = s.find_first_of(L'\t', i + 1);
		if(icd == std::wstring::npos)
		{
			break;
		}
		icr = s.find_first_of(L'\t', icd + 1);
		if(icr == std::wstring::npos)
		{
			break;
		}
		ia = s.find_first_of(L'\n', icr + 1);
		if(ia == std::wstring::npos)
		{
			break;
		}
		scd = s.substr(i + 1, icd - (i + 1));
		scr = s.substr(icd + 1, icr - (icd + 1));
		sa = s.substr(icr + 1, ia - (icr + 1));
		if(scd.empty())
		{
			scd = scr;
		}

		candidates.push_back(CANDIDATE(CANDIDATEBASE(scd, sa), CANDIDATEBASE(scr, sa)));
		i = ia;
	}

exit:
	_DisconnectDic();
}

void CTextService::_ConvertCandidate(std::wstring &conv, const std::wstring &key, const std::wstring &candidate)
{
	WCHAR wbuf[PIPEBUFSIZE];
	DWORD bytesWrite, bytesRead;

	_StartManager();

	_ConnectDic();

	ZeroMemory(wbuf, sizeof(wbuf));

	_snwprintf_s(wbuf, _TRUNCATE, L"%c\n%s\t%s\n",
		REQ_CONVERSION, key.c_str(), candidate.c_str());

	if(WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(wbuf, sizeof(wbuf));

	if(ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL) == FALSE)
	{
		goto exit;
	}

	if(wbuf[0] != REP_OK)
	{
		conv = candidate;
		goto exit;
	}

	wbuf[wcslen(wbuf) - 1] = L'\0';
	conv.assign(&wbuf[2]);

exit:
	_DisconnectDic();
}

void CTextService::_AddUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate, const std::wstring &annotation)
{
	WCHAR wbuf[PIPEBUFSIZE];
	DWORD bytesWrite, bytesRead;

	_ConnectDic();

	ZeroMemory(wbuf, sizeof(wbuf));

	_snwprintf_s(wbuf, _TRUNCATE, L"%c\n%s\t%s\t%s\n",
		command, key.c_str(), candidate.c_str(), annotation.c_str());

	if(WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(wbuf, sizeof(wbuf));

	if(ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL) == FALSE)
	{
		goto exit;
	}

exit:
	_DisconnectDic();
}

void CTextService::_DelUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate)
{
	WCHAR wbuf[PIPEBUFSIZE];
	DWORD bytesWrite, bytesRead;

	_ConnectDic();

	ZeroMemory(wbuf, sizeof(wbuf));

	_snwprintf_s(wbuf, _TRUNCATE, L"%c\n%s\t%s\n",
		command, key.c_str(), candidate.c_str());

	if(WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(wbuf, sizeof(wbuf));

	if(ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL) == FALSE)
	{
		goto exit;
	}

exit:
	_DisconnectDic();
}

void CTextService::_SaveUserDic()
{
	WCHAR buf[2];
	DWORD bytes;

	_ConnectDic();

	buf[0] = REQ_USER_SAVE;
	buf[1] = L'\n';
	if(WriteFile(hPipe, &buf, 4, &bytes, NULL) == FALSE)
	{
		goto exit;
	}

	if(ReadFile(hPipe, &buf, 2, &bytes, NULL) == FALSE)
	{
		goto exit;
	}

exit:
	_DisconnectDic();
}

void CTextService::_StartManager()
{
	HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, mgrmutexname);
	if(hMutex != NULL)
	{
		CloseHandle(hMutex);
		return;
	}

	_StartProcess(CORVUSMGREXE);
}

void CTextService::_StartConfigure()
{
	HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, cnfmutexname);
	if(hMutex != NULL)
	{
		CloseHandle(hMutex);
		return;
	}

	_StartProcess(CORVUSCNFEXE);
}

void CTextService::_StartProcess(LPCWSTR fname)
{
	WCHAR path[MAX_PATH];
	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	PROCESS_INFORMATION pi;
	STARTUPINFOW si;

	GetModuleFileNameW(g_hInst, path, _countof(path));
	_wsplitpath_s(path, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);
	_snwprintf_s(path, _TRUNCATE, L"%s%s%s", drive, dir, fname);

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if(CreateProcessW(path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}
