
#include "imcrvmgr.h"
#include "utf8.h"
#include "parseskkdic.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HANDLE SrvStart();

CRITICAL_SECTION csUserDataSave;
BOOL bUserDicChg;
FILETIME ftConfig;
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
	WNDCLASSEX wcex;
	HWND hWnd;
	WSADATA wsaData;
	PSECURITY_DESCRIPTOR psd;
	SECURITY_ATTRIBUTES sa;

	_wsetlocale(LC_ALL, L"JPN");

	CreateConfigPath();

	ConvertStringSecurityDescriptorToSecurityDescriptorW(krnlobjsddl, SDDL_REVISION_1, &psd, NULL);
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = psd;
	sa.bInheritHandle = FALSE;

	hMutex = CreateMutexW(&sa, FALSE, mgrmutexname);
	if(hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LocalFree(psd);
		return 0;
	}

	LocalFree(psd);

	WSAStartup(WINSOCK_VERSION, &wsaData);

	ZeroMemory(&ftConfig, sizeof(ftConfig));
	if(IsFileUpdated(pathconfigxml, &ftConfig))
	{
		LoadConfig();
	}

	InitLua();

	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= hInstance;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszClassName	= TextServiceDesc;
	RegisterClassEx(&wcex);

	hInst = hInstance;

#ifdef _DEBUG
	hWnd = CreateWindowW(TextServiceDesc, TextServiceDesc,
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 600, 800, NULL, NULL, hInstance, NULL);
#else
	hWnd = CreateWindowW(TextServiceDesc, TextServiceDesc, WS_POPUP,
		0, 0, 0, 0, NULL, NULL, hInstance, NULL);
#endif

	if(!hWnd)
	{
		WSACleanup();
		return 0;
	}

#ifdef _DEBUG
	//ShowWindow(hWnd, nCmdShow);
	ShowWindow(hWnd, SW_MINIMIZE);
	UpdateWindow(hWnd);
#endif

	while(GetMessageW(&msg, NULL, 0, 0))
	{
		if(!TranslateAcceleratorW(msg.hwnd, NULL, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	WSACleanup();

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE hThreadSave;
	HANDLE hPipe;
#ifdef _DEBUG
	RECT r;
	HDC hDC;
#endif

	switch (message)
	{
	case WM_CREATE:
#ifdef _DEBUG
		GetClientRect(hWnd, &r);
		hwndEdit = CreateWindowW(L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
			0, 0, r.right, r.bottom, hWnd, NULL, hInst, NULL);
		hDC = GetDC(hwndEdit);
		hFont = CreateFontW(-MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH,
			L"Meiryo");
		SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hFont, 0);
		ReleaseDC(hwndEdit, hDC);
#endif
		InitializeCriticalSection(&csUserDataSave);

		bUserDicChg = FALSE;
		LoadSKKUserDic();

		bSrvThreadExit = FALSE;
		hThreadSrv = SrvStart();
		if(hThreadSrv == NULL)
		{
			DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY:
	case WM_ENDSESSION:
#ifdef _DEBUG
		DeleteObject(hFont);
#endif
		hThreadSave = StartSaveSKKUserDicEx();
		WaitForSingleObject(hThreadSave, INFINITE);
		CloseHandle(hThreadSave);

		bSrvThreadExit = TRUE;
		hPipe = CreateFileW(mgrpipename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_EFFECTIVE_ONLY | SECURITY_IDENTIFICATION, NULL);
		if(hPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hPipe);
			WaitForSingleObject(hThreadSrv, INFINITE);
		}

		CloseHandle(hThreadSrv);

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
// request and reply commands defined at common.h
//
//search candidate
//	request	"1\n<key>\t<key(original)>\t<okuri>\n"
//	reply	"T\n<candidate(display)>\t<candidate(regist)>\t<annotation(display)>\t<annotation(regist)>\n...\n":hit, "F":nothing
//search key for complement
//	request	"4\n<key>\t\t\n"
//	reply	"T\n<key>\t\t\t\n...\n":hit, "F":nothing
//convert key
//	request	"5\n<key>\t\n"
//	reply	"T\n<key converted>\n...\n":hit, "F":nothing
//convert candidate
//	request	"6\n<key>\t<candidate>\n"
//	reply	"T\n<candidate converted>\n":hit, "F":nothing
//add candidate (complement off)
//	request	"A\n<key>\t<candidate>\t<annotation>\t<okuri>\n"
//	reply	"T"
//add candidate (complement on)
//	request	"B\n<key>\t<candidate>\t<annotation>\t\n"
//	reply	"T"
//delete candidate (complement off)
//	request	"C\n<key>\t<candidate>\n"
//	reply	"T"
//delete candidate (complement on)
//	request	"D\n<key>\t<candidate>\n"
//	reply	"T"
//save user dictionary
//	request	"S\n"
//	reply	"T"

void SrvProc(WCHAR *wbuf, size_t size)
{
	SKKDICCANDIDATES sc;
	SKKDICCANDIDATES::iterator sc_itr;
	std::wstring bufdat(&wbuf[2]);
	std::wregex re;
	std::wstring fmt;
	std::wstring keyorg;
	std::wstring key;
	std::wstring okuri;
	std::wstring candidate;
	std::wstring annotation;
	std::wstring conv;

	switch(wbuf[0])
	{
	case REQ_SEARCH:
		re.assign(L"(.*)\t(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(bufdat, re, fmt);
		fmt.assign(L"$2");
		keyorg = std::regex_replace(bufdat, re, fmt);
		fmt.assign(L"$3");
		okuri = std::regex_replace(bufdat, re, fmt);

		SearchDictionary(key, okuri, sc);

		if(!sc.empty())
		{
			wbuf[0] = REP_OK;
			wbuf[1] = L'\n';
			wbuf[2] = L'\0';
			for(sc_itr = sc.begin(); sc_itr != sc.end(); sc_itr++)
			{
				wcsncat_s(wbuf, size, ConvertCandidate(keyorg, sc_itr->first).c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\t", _TRUNCATE);
				wcsncat_s(wbuf, size, sc_itr->first.c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\t", _TRUNCATE);
				wcsncat_s(wbuf, size, ConvertCandidate(keyorg, sc_itr->second).c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\t", _TRUNCATE);
				wcsncat_s(wbuf, size, sc_itr->second.c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\n", _TRUNCATE);
			}
		}
		else
		{
			wbuf[0] = REP_FALSE;
			wbuf[1] = L'\0';
		}
		break;

	case REQ_COMPLEMENT:
		re.assign(L"(.*)\t(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(bufdat, re, fmt);

		if(lua != NULL)
		{
			lua_getglobal(lua,"lua_skk_complement");
			lua_pushstring(lua, WCTOU8(key.c_str()));
			if(lua_pcall(lua, 1, 1, 0) == LUA_OK)
			{
				if(lua_isstring(lua, -1))
				{
					candidate = U8TOWC(lua_tostring(lua, -1));
					ParseSKKDicCandiate(candidate, sc);
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
			wbuf[0] = REP_OK;
			wbuf[1] = L'\n';
			wbuf[2] = L'\0';
			for(sc_itr = sc.begin(); sc_itr != sc.end(); sc_itr++)
			{
				wcsncat_s(wbuf, size, sc_itr->first.c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\t\t\t\n", _TRUNCATE);
			}
		}
		else
		{
			wbuf[0] = REP_FALSE;
			wbuf[1] = L'\0';
		}
		break;

	case REQ_CONVERTKEY:
		re.assign(L"(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(bufdat, re, fmt);

		conv = ConvertKey(key);

		if(!conv.empty())
		{
			wbuf[0] = REP_OK;
			wbuf[1] = L'\n';
			wbuf[2] = L'\0';
			wcsncat_s(wbuf, size, conv.c_str(), _TRUNCATE);
			wcsncat_s(wbuf, size, L"\n", _TRUNCATE);
		}
		else
		{
			wbuf[0] = REP_FALSE;
			wbuf[1] = L'\0';
		}
		break;

	case REQ_CONVERTCND:
		re.assign(L"(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(bufdat, re, fmt);
		fmt.assign(L"$2");
		candidate = std::regex_replace(bufdat, re, fmt);

		conv = ConvertCandidate(key, candidate);

		if(!conv.empty())
		{
			wbuf[0] = REP_OK;
			wbuf[1] = L'\n';
			wbuf[2] = L'\0';
			wcsncat_s(wbuf, size, conv.c_str(), _TRUNCATE);
			wcsncat_s(wbuf, size, L"\n", _TRUNCATE);
		}
		else
		{
			wbuf[0] = REP_FALSE;
			wbuf[1] = L'\0';
		}
		break;

	case REQ_USER_ADD_0:
	case REQ_USER_ADD_1:
		re.assign(L"(.*)\t(.*)\t(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(bufdat, re, fmt);
		fmt.assign(L"$2");
		candidate = std::regex_replace(bufdat, re, fmt);
		fmt.assign(L"$3");
		annotation = std::regex_replace(bufdat, re, fmt);
		fmt.assign(L"$4");
		okuri = std::regex_replace(bufdat, re, fmt);

		if(lua != NULL)
		{
			lua_getglobal(lua, "lua_skk_add");
			lua_pushboolean(lua, (wbuf[0] == REQ_USER_ADD_0 ? 1 : 0));
			lua_pushstring(lua, WCTOU8(key.c_str()));
			lua_pushstring(lua, WCTOU8(candidate.c_str()));
			lua_pushstring(lua, WCTOU8(annotation.c_str()));
			lua_pushstring(lua, WCTOU8(okuri.c_str()));
			lua_pcall(lua, 5, 1, 0);
		}
		else
		{
			AddUserDic(wbuf[0], key, candidate, annotation, okuri);
		}

		wbuf[0] = REP_OK;
		wbuf[1] = L'\0';
		break;

	case REQ_USER_DEL_0:
	case REQ_USER_DEL_1:
		re.assign(L"(.*)\t(.*)\n");
		fmt.assign(L"$1");
		key = std::regex_replace(bufdat, re, fmt);
		fmt.assign(L"$2");
		candidate = std::regex_replace(bufdat, re, fmt);

		if(lua != NULL)
		{
			lua_getglobal(lua, "lua_skk_delete");
			lua_pushboolean(lua, (wbuf[0] == REQ_USER_DEL_0 ? 1 : 0));
			lua_pushstring(lua, WCTOU8(key.c_str()));
			lua_pushstring(lua, WCTOU8(candidate.c_str()));
			lua_pcall(lua, 3, 0, 0);
		}
		else
		{
			DelUserDic(wbuf[0], key, candidate);
		}

		wbuf[0] = REP_OK;
		wbuf[1] = L'\0';
		break;

	case REQ_USER_SAVE:
		if(lua != NULL)
		{
			lua_getglobal(lua, "lua_skk_save");
			lua_pcall(lua, 0, 0, 0);
		}
		else
		{
			StartSaveSKKUserDic();
		}

		wbuf[0] = REP_OK;
		wbuf[1] = L'\0';
		break;

	default:
		wbuf[0] = L'0';
		wbuf[1] = L'\0';
		break;
	}
}

unsigned int __stdcall SrvThread(void *p)
{
	HANDLE hPipe = (HANDLE)p;
	WCHAR wbuf[PIPEBUFSIZE];
	DWORD bytesRead, bytesWrite;
	BOOL bRet;
#ifdef _DEBUG
	std::wstring dedit, tedit;
	std::wregex re;
	std::wstring fmt;
#endif

	while(true)
	{
		if(ConnectNamedPipe(hPipe, NULL) == FALSE)
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

		bytesRead = 0;
		ZeroMemory(wbuf, sizeof(wbuf));

		bRet = ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL);

		if(bRet == FALSE || bytesRead == 0)
		{
			DisconnectNamedPipe(hPipe);
			continue;
		}

#ifdef _DEBUG
		if(wbuf[0] == REQ_USER_SAVE)
		{
			dedit.clear();
		}

		tedit.assign(wbuf);
		tedit.append(L"\n");
		re.assign(L"\n");
		fmt.assign(L"\r\n");
		tedit = std::regex_replace(tedit, re, fmt);
		re.assign(L"\t");
		fmt.assign(L"»");
		tedit = std::regex_replace(tedit, re, fmt);

		dedit.append(tedit);
		SetWindowTextW(hwndEdit, dedit.c_str());
#endif

		SrvProc(wbuf, _countof(wbuf));

#ifdef _DEBUG
		tedit.assign(wbuf);
		tedit.append(L"\n");
		re.assign(L"\n");
		fmt.assign(L"\r\n");
		tedit = std::regex_replace(tedit, re, fmt);
		re.assign(L"\t");
		fmt.assign(L"»");
		tedit = std::regex_replace(tedit, re, fmt);

		dedit.append(tedit);
		SetWindowTextW(hwndEdit, dedit.c_str());
		SendMessageW(hwndEdit, WM_VSCROLL, SB_BOTTOM, 0);
#endif

		bRet = WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL);

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
	HANDLE hPipe;
	HANDLE hThread = NULL;

	ConvertStringSecurityDescriptorToSecurityDescriptorW(krnlobjsddl, SDDL_REVISION_1, &psd, NULL);
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = psd;
	sa.bInheritHandle = FALSE;

	hPipe = CreateNamedPipeW(mgrpipename, PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
		1, PIPEBUFSIZE*sizeof(WCHAR), PIPEBUFSIZE*sizeof(WCHAR), 0, &sa);

	LocalFree(psd);

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, SrvThread, hPipe, 0, NULL);
	}

	if(hThread == NULL)
	{
		CloseHandle(hPipe);
	}

	return hThread;
}
