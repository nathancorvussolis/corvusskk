
#include "parseskkdic.h"
#include "utf8.h"
#include "imcrvmgr.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HANDLE SrvStart();

CRITICAL_SECTION csUserDataSave;
BOOL bUserDicChg;
FILETIME ftConfig, ftSKKDic;
#ifdef _DEBUG
HWND hwndEdit;
HFONT hFont;
#endif
HINSTANCE hInst;
HANDLE hMutex;
HANDLE hThreadSrv;
BOOL bSrvThreadExit;
lua_State *lua;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASSEXW wc;
	HWND hWnd;
	WSADATA wsaData;
	PSECURITY_DESCRIPTOR psd;
	SECURITY_ATTRIBUTES sa;

	_wsetlocale(LC_ALL, L"JPN");

	CreateConfigPath();

	if(ConvertStringSecurityDescriptorToSecurityDescriptorW(krnlobjsddl, SDDL_REVISION_1, &psd, nullptr))
	{
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = psd;
		sa.bInheritHandle = FALSE;

		hMutex = CreateMutexW(&sa, FALSE, mgrmutexname);
		if(hMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
		{
			LocalFree(psd);
			return 0;
		}

		LocalFree(psd);
	}
	else
	{
		return 0;
	}

	WSAStartup(WINSOCK_VERSION, &wsaData);

	ZeroMemory(&ftConfig, sizeof(ftConfig));
	if(IsFileUpdated(pathconfigxml, &ftConfig))
	{
		LoadConfig();
	}

	ZeroMemory(&ftSKKDic, sizeof(ftSKKDic));
	if(IsFileUpdated(pathskkdic, &ftSKKDic))
	{
		MakeSKKDicPos();
	}

	InitLua();

	hInst = hInstance;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = DictionaryManagerClass;
	RegisterClassExW(&wc);

#ifdef _DEBUG
	hWnd = CreateWindowW(DictionaryManagerClass, TextServiceDesc,
		WS_OVERLAPPEDWINDOW, 0, 0, 600, 800, nullptr, nullptr, hInst, nullptr);
#else
	hWnd = CreateWindowW(DictionaryManagerClass, TextServiceDesc,
		WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, hInst, nullptr);
#endif

	if(!hWnd)
	{
		UninitLua();
		WSACleanup();
		return 0;
	}

#ifdef _DEBUG
	ShowWindow(hWnd, SW_MINIMIZE);
#else
	ShowWindow(hWnd, SW_HIDE);
#endif
	UpdateWindow(hWnd);

	while(GetMessageW(&msg, nullptr, 0, 0))
	{
		if(!TranslateAcceleratorW(msg.hwnd, nullptr, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	UninitLua();
	WSACleanup();

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE hPipe;
#ifdef _DEBUG
	RECT r;
	HDC hDC;
#endif

	switch(message)
	{
	case WM_CREATE:
#ifdef _DEBUG
		GetClientRect(hWnd, &r);
		hwndEdit = CreateWindowW(L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
			0, 0, r.right, r.bottom, hWnd, nullptr, hInst, nullptr);
		hDC = GetDC(hwndEdit);
		hFont = CreateFontW(-MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH,
			L"Meiryo");
		SendMessageW(hwndEdit, WM_SETFONT, (WPARAM)hFont, 0);
		ReleaseDC(hwndEdit, hDC);
#endif
		InitializeCriticalSection(&csUserDataSave);

		bUserDicChg = FALSE;
		LoadSKKUserDic();

		bSrvThreadExit = FALSE;
		hThreadSrv = SrvStart();
		if(hThreadSrv == nullptr)
		{
			DestroyWindow(hWnd);
		}
		break;

#ifdef _DEBUG
	case WM_SIZE:
		GetClientRect(hWnd, &r);
		MoveWindow(hwndEdit, 0, 0, r.right, r.bottom, TRUE);
		break;
#endif

	case WM_POWERBROADCAST:
		if(wParam == PBT_APMSUSPEND)
		{
			StartSaveSKKUserDic(FALSE);

			BackUpSKKUserDic();
		}
		break;

	case WM_DESTROY:
	case WM_ENDSESSION:
#ifdef _DEBUG
		DeleteObject(hFont);
#endif
		bSrvThreadExit = TRUE;
		hPipe = CreateFileW(mgrpipename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_EFFECTIVE_ONLY | SECURITY_IDENTIFICATION, nullptr);
		if(hPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hPipe);
			WaitForSingleObject(hThreadSrv, INFINITE);
		}

		CloseHandle(hThreadSrv);

		StartSaveSKKUserDic(FALSE);

		if(message == WM_ENDSESSION)
		{
			BackUpSKKUserDic();
		}

		DeleteCriticalSection(&csUserDataSave);

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);

		PostQuitMessage(0);
		break;

	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

//
// request and reply commands
//
//search candidate
//  request "1\n<key>\t<key(original)>\t<okuri>\n"
//  reply   "T\n<candidate(display)>\t<candidate(register)>\t<annotation(display)>\t<annotation(register)>\n...\n":hit
//          "F\n":nothing
//search candidate (user dictionary only)
//  request "2\n<key>\t<key(original)>\t<okuri>\n"
//  reply   "T\n<candidate(display)>\t<candidate(register)>\t<annotation(display)>\t<annotation(register)>\n...\n":hit
//          "F\n":nothing
//search key for complement
//  request "4\n<key>\t\t\n"
//  reply   "T\n<key>\t\t\t\n...\n":hit
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
	case REQ_SEARCHUSER:
		re.assign(L"(.*)\t(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$2");
		keyorg = std::regex_replace(argument, re, fmt);
		fmt.assign(L"$3");
		okuri = std::regex_replace(argument, re, fmt);

		switch(command)
		{
		case REQ_SEARCH:
			SearchDictionary(key, okuri, sc);
			break;
		case REQ_SEARCHUSER:
			candidate = SearchUserDic(key, okuri);
			re.assign(L"[\\x00-\\x19]");
			fmt.assign(L"");
			candidate = std::regex_replace(candidate, re, fmt);
			ParseSKKDicCandiate(candidate, sc);
			break;
		default:
			break;
		}

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

		if(!sc.empty())
		{
			result = REP_OK;
			result += L"\n";
			FORWARD_ITERATION_I(sc_itr, sc)
			{
				result += sc_itr->first + L"\t\t\t\n";
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

#ifdef _DEBUG
	case REQ_DEBUGOUT_ON:
	case REQ_DEBUGOUT_OFF:
		result = REP_OK;
		result += L"\n";
		break;
#endif

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
	static BOOL debugout = TRUE;
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
		switch(command)
		{
		case REQ_DEBUGOUT_ON:
			debugout = TRUE;
			break;
		default:
			break;
		}

		if(debugout)
		{
			tedit.assign(pipebuf);
			re.assign(L"\n");
			fmt.assign(L"\r\n");
			tedit = std::regex_replace(tedit, re, fmt);
			re.assign(L"\t");
			fmt.assign(L"»");
			tedit = std::regex_replace(tedit, re, fmt);

			dedit.append(tedit);
			SetWindowTextW(hwndEdit, dedit.c_str());
		}
#endif

		SrvProc(command, &pipebuf[2], wspipebuf);
		wcsncpy_s(pipebuf, wspipebuf.c_str(), _TRUNCATE);

#ifdef _DEBUG
		if(debugout)
		{
			tedit.assign(pipebuf);
			re.assign(L"\n");
			fmt.assign(L"\r\n");
			tedit = std::regex_replace(tedit, re, fmt);
			re.assign(L"\t");
			fmt.assign(L"»");
			tedit = std::regex_replace(tedit, re, fmt);

			dedit.append(tedit);
			SetWindowTextW(hwndEdit, dedit.c_str());
			SendMessageW(hwndEdit, WM_VSCROLL, SB_BOTTOM, 0);
		}

		switch(command)
		{
		case REQ_USER_SAVE:
			dedit.clear();
			break;
		case REQ_DEBUGOUT_OFF:
			debugout = FALSE;
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
