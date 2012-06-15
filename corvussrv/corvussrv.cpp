
#include "corvussrv.h"

#define BUFSIZE 0x2000	// -> KeyHandlerDictionary.cpp

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void SrvStart();
void RegisterRun();

#ifdef _DEBUG
HWND hwndEdit;
HFONT hFont;
#endif
HINSTANCE hInst;
HANDLE hMutex;
BOOL bUserDicChg;
CRITICAL_SECTION csUserDic;
CRITICAL_SECTION csUserDicS;
CRITICAL_SECTION csSKKServ;

int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	WNDCLASSEX wcex;
	HWND hWnd;
	WSADATA wsaData;

	hMutex = CreateMutex(NULL, FALSE, CORVUSSRVMUTEX);
	if(hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}

	setlocale(LC_ALL, "japanese");

	WSAStartup(WINSOCK_VERSION, &wsaData);

	CreateConfigPath();

	LoadConfig();

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
		0, 0, 400, 800, NULL, NULL, hInstance, NULL);
#else
	hWnd = CreateWindowW(TextServiceDesc, TextServiceDesc, WS_POPUP,
		0, 0, 0, 0, NULL, NULL, hInstance, NULL);
#endif

	if(!hWnd)
	{
		return FALSE;
	}

#ifdef _DEBUG
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
#endif

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!TranslateAccelerator(msg.hwnd, NULL, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	WSACleanup();

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE hThread;
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
		hFont = CreateFontW(-MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH,
			L"Meiryo");
		SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hFont, 0);
		ReleaseDC(hwndEdit, hDC);
#endif
		InitializeCriticalSection(&csUserDic);
		InitializeCriticalSection(&csUserDicS);
		InitializeCriticalSection(&csSKKServ);

		bUserDicChg = FALSE;

		LoadUserDic();

		SrvStart();

		RegisterRun();
		break;

	case WM_DESTROY:
	case WM_ENDSESSION:
#ifdef _DEBUG
		DeleteObject(hFont);
#endif
		hThread = StartSaveUserDic();
		WaitForSingleObject(hThread, INFINITE);

		DeleteCriticalSection(&csSKKServ);
		DeleteCriticalSection(&csUserDicS);
		DeleteCriticalSection(&csUserDic);

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);

		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

//search candidate
//	request	"1\n<key>\n"
//	reply	"1\n<candidate>\t<annotation>\n...\n":hit, "4":nothing
//search key for complement
//	request	"8\n<key>\n"
//	reply	"1\n<key>\t\n...\n":hit, "4":nothing
//add candidate (complement off)
//	request	"A\n<key>\t<candidate>\t<annotation>\n"
//	reply	"1"
//add candidate (complement on)
//	request	"B\n<key>\t<candidate>\t<annotation>\n"
//	reply	"1"
//delete candidate
//	request	"D\n<key>\t<candidate>\n"
//	reply	"1"
//save user dictionary
//	request	"S"
//	reply	"1"
//check alive
//	request	"?"
//	reply	"!"

void SrvProc(WCHAR *wbuf, size_t size)
{
	CANDIDATES candidates;
	CANDIDATES::iterator candidates_itr;
	const wchar_t seps[]   = L"\t\n";
	wchar_t *token = NULL;
	wchar_t *next_token = NULL;
	std::wstring key;
	std::wstring candidate;
	std::wstring annotation;

	switch(wbuf[0])
	{
	case REQ_SEARCH:
		wbuf[wcslen(wbuf) - 1] = L'\0';
		ConvDictionary(std::wstring(&wbuf[2]), candidates, wbuf[0]);
		if(!candidates.empty())
		{
			wbuf[0] = REP_OK;
			wbuf[1] = L'\n';
			wbuf[2] = L'\0';
			for(candidates_itr = candidates.begin(); candidates_itr != candidates.end(); candidates_itr++)
			{
				wcsncat_s(wbuf, size, candidates_itr->first.c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\t", _TRUNCATE);
				wcsncat_s(wbuf, size, candidates_itr->second.c_str(), _TRUNCATE);
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
		wbuf[wcslen(wbuf) - 1] = L'\0';
		ConvComplement(std::wstring(&wbuf[2]), candidates);
		if(!candidates.empty())
		{
			wbuf[0] = REP_OK;
			wbuf[1] = L'\n';
			wbuf[2] = L'\0';
			for(candidates_itr = candidates.begin(); candidates_itr != candidates.end(); candidates_itr++)
			{
				wcsncat_s(wbuf, size, candidates_itr->first.c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\t\n", _TRUNCATE);
			}
		}
		else
		{
			wbuf[0] = REP_FALSE;
			wbuf[1] = L'\0';
		}
		break;

	case REQ_USER_ADD_0:
	case REQ_USER_ADD_1:
		token = wcstok_s(&wbuf[2], seps, &next_token);
		if(token != NULL)
		{
			key.assign(token);
			token = wcstok_s(NULL, seps, &next_token);
		}
		if(token != NULL)
		{
			candidate.assign(token);
			token = wcstok_s(NULL, seps, &next_token);
		}
		if(token != NULL)
		{
			annotation.assign(token);
		}

		AddUserDic(key, candidate, annotation, wbuf[0]);

		wbuf[0] = REP_OK;
		wbuf[1] = L'\0';
		break;

	case REQ_USER_DEL:
		token = wcstok_s(&wbuf[2], seps, &next_token);
		if(token != NULL)
		{
			key.assign(token);
			token = wcstok_s(NULL, seps, &next_token);
		}
		if(token != NULL)
		{
			candidate.assign(token);
		}

		DelUserDic(key, candidate);

		wbuf[0] = REP_OK;
		wbuf[1] = L'\0';
		break;

	case REQ_USER_SAVE:
		StartSaveUserDic();

		wbuf[0] = REP_OK;
		wbuf[1] = L'\0';
		break;

	case REQ_CHECK_ALIVE:
		wbuf[0] = REP_CHECK_ALIVE;
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
	HANDLE hPipe = *(HANDLE*)p;
	WCHAR wbuf[BUFSIZE];
	DWORD bytesRead, bytesWrite;
	BOOL bRet;
#ifdef _DEBUG
	WCHAR reqcmd;
	std::wstring dedit;
	std::wregex re(L"\n");
	std::wstring fmt(L"\r\n");
#endif

	*(HANDLE*)p = INVALID_HANDLE_VALUE;

	while(true)
	{
		bytesRead = 0;
		ZeroMemory(wbuf, sizeof(wbuf));

		bRet = ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL);

		if(bRet == FALSE)
		{
			StartSaveUserDic();
			break;
		}
		if(bytesRead == 0)
		{
			continue;
		}

#ifdef _DEBUG
		reqcmd = wbuf[0];
		if(reqcmd == REQ_CHECK_ALIVE)
		{
			dedit.clear();
		}
		else
		{
			dedit.append(wbuf);
			dedit.append(L"\n");
		}
		SetWindowTextW(hwndEdit, dedit.c_str());
#endif

		SrvProc(wbuf, _countof(wbuf));

#ifdef _DEBUG
		if(reqcmd != REQ_CHECK_ALIVE)
		{
			dedit.append(wbuf);
			dedit.append(L"\n");
			dedit = std::regex_replace(dedit, re, fmt);
			SetWindowTextW(hwndEdit, dedit.c_str());
		}
#endif

		bRet = WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL);

		if(bRet == FALSE)
		{
			break;
		}

		FlushFileBuffers(hPipe);
	}

	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	_endthreadex(0);
	return 0;
}

unsigned int __stdcall SrvListenThread(void *p)
{
	HANDLE hPipe;
	DWORD dwPipeMode;
	uintptr_t tRet;
	int count;

	OSVERSIONINFO ovi;
	ZeroMemory(&ovi, sizeof(ovi));
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);
	if(ovi.dwMajorVersion < 6)
	{
		dwPipeMode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT;
	}
	else
	{
		dwPipeMode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS;
	}

	//IEの保護モード対応の為、名前付きパイプの整合性レベルを低くする
	//http://msdn.microsoft.com/en-us/library/bb250462
	PSECURITY_DESCRIPTOR pSD = NULL;
	SECURITY_ATTRIBUTES sa;
#define LOW_INTEGRITY_SDDL_SACL_W	L"S:(ML;;NW;;;LW)"
	ConvertStringSecurityDescriptorToSecurityDescriptorW(LOW_INTEGRITY_SDDL_SACL_W, SDDL_REVISION_1, &pSD, NULL);
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	while(true)
	{
		hPipe = CreateNamedPipeW(pipename, PIPE_ACCESS_DUPLEX, dwPipeMode,
			PIPE_UNLIMITED_INSTANCES, BUFSIZE*sizeof(WCHAR), BUFSIZE*sizeof(WCHAR), 1000, &sa);
		
		if(hPipe == INVALID_HANDLE_VALUE)
		{
			break;
		}

		if(ConnectNamedPipe(hPipe, NULL) == FALSE)
		{
			CloseHandle(hPipe);
			continue;
		}

		LoadConfig();

		tRet = _beginthreadex(NULL, 0, SrvThread, &hPipe, 0, NULL);

		if(tRet != 0)
		{
			count = 0;
			while(hPipe != INVALID_HANDLE_VALUE)
			{
				if(count > 10)
				{
					break;
				}
				Sleep(100);
				++count;
			}
		}
	}

	LocalFree(pSD);

	_endthreadex(0);
	return 0;
}

void SrvStart()
{
	_beginthreadex(NULL, 0, SrvListenThread, NULL, 0, NULL);
}

void RegisterRun()
{
	// HKCU\..\Run  %SystemRoot%\System32\IME\CorvusSKK\corvussrv.exe
	WCHAR path[MAX_PATH];
	WCHAR szval[_MAX_FNAME];
	WCHAR szext[_MAX_EXT];
	HKEY hkResult;
	LONG ret;

	if(GetModuleFileNameW(NULL, path, _countof(path)) == 0)
	{
		return;
	}
	if(_wsplitpath_s(path, NULL, 0, NULL, 0, szval, _countof(szval), szext, _countof(szext)) != 0)
	{
		return;
	}
	if(GetSystemDirectoryW(path, _countof(path)) == 0)
	{
		return;
	}
	wcsncat_s(path, _countof(path), L"\\IME\\", _TRUNCATE);
	wcsncat_s(path, _countof(path), TextServiceDesc, _TRUNCATE);
	wcsncat_s(path, _countof(path), L"\\", _TRUNCATE);
	
	wcsncat_s(path, _countof(path), szval, _TRUNCATE);
	wcsncat_s(path, _countof(path), szext, _TRUNCATE);

	ret = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &hkResult);
	if(ret == ERROR_SUCCESS)
	{
		ret = RegQueryValueEx(hkResult, szval, 0, NULL, NULL, NULL);
		if(ret != ERROR_SUCCESS)
		{
			RegSetValueExW(hkResult, szval, 0, REG_SZ, (BYTE*)path, (DWORD)wcslen(path)*sizeof(WCHAR));
		}
		RegCloseKey(hkResult);
	}
}
