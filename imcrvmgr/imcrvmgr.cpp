
#include "imcrvmgr.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

CRITICAL_SECTION csUserDict;
CRITICAL_SECTION csUserData;
CRITICAL_SECTION csSaveUserDic;
CRITICAL_SECTION csSKKSocket;
BOOL bUserDicChg;
FILETIME ftConfig = {};
FILETIME ftSKKDic = {};
#ifdef _DEBUG
HWND hWndEdit;
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
	HWND hWnd;

	_wsetlocale(LC_ALL, L"ja-JP");

	CreateIpcName();

	hMutex = CreateMutexW(nullptr, FALSE, mgrmutexname);
	if (hMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (hMutex != nullptr)
		{
			CloseHandle(hMutex);
		}
		return 0;
	}

	hInst = hInstance;

	WNDCLASSEXW wc = {};
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

	if (!hWnd)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return 0;
	}

#ifdef _DEBUG
	ShowWindow(hWnd, SW_MINIMIZE);
#else
	ShowWindow(hWnd, SW_HIDE);
#endif
	UpdateWindow(hWnd);

	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		if (!TranslateAcceleratorW(msg.hwnd, nullptr, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE hPipe;
	WSADATA wsaData;
#ifdef _DEBUG
	RECT r = {};
	HDC hDC;
#endif

	switch (message)
	{
	case WM_CREATE:
#ifdef _DEBUG
		GetClientRect(hWnd, &r);
		hWndEdit = CreateWindowW(L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
			0, 0, r.right, r.bottom, hWnd, nullptr, hInst, nullptr);
		hDC = GetDC(hWndEdit);
		hFont = CreateFontW(-MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), C_FONT_LOGICAL_HEIGHT_PPI), 0, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH,
			L"Meiryo");
		SendMessageW(hWndEdit, WM_SETFONT, (WPARAM)hFont, 0);
		ReleaseDC(hWndEdit, hDC);
#endif
		WSAStartup(WINSOCK_VERSION, &wsaData);

		CreateConfigPath();
		UpdateConfigPath();

		InitializeCriticalSection(&csUserDict);	// !
		InitializeCriticalSection(&csUserData);	// !
		InitializeCriticalSection(&csSaveUserDic);	// !
		InitializeCriticalSection(&csSKKSocket);	// !

		if (IsFileModified(pathconfigxml, &ftConfig))
		{
			LoadConfig();
		}

		if (IsFileModified(pathskkdic, &ftSKKDic))
		{
			MakeSKKDicPos();
		}

		bUserDicChg = FALSE;
		LoadUserDic();

		InitLua();

		bSrvThreadExit = FALSE;
		hThreadSrv = SrvStart();
		if (hThreadSrv == nullptr)
		{
			DestroyWindow(hWnd);
		}
		break;

#ifdef _DEBUG
	case WM_SIZE:
		GetClientRect(hWnd, &r);
		MoveWindow(hWndEdit, 0, 0, r.right, r.bottom, TRUE);
		break;
#endif

	case WM_POWERBROADCAST:
		if (wParam == PBT_APMRESUMESUSPEND)
		{
			StartSaveUserDic(FALSE);

			BackUpUserDic();
		}
		break;

	case WM_QUERYENDSESSION:
		{
			// block shutdown
			WCHAR reason[MAX_STR_BLOCKREASON];
			_snwprintf_s(reason, _TRUNCATE, L"A SKK user dictionary is being saved.");
			ShutdownBlockReasonCreate(hWnd, reason);
			return TRUE;
		}
		break;

	case WM_DESTROY:
	case WM_ENDSESSION:
		bSrvThreadExit = TRUE;
		hPipe = CreateFileW(mgrpipename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_EFFECTIVE_ONLY | SECURITY_IDENTIFICATION, nullptr);
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hPipe);
			WaitForSingleObject(hThreadSrv, INFINITE);
		}

		CloseHandle(hThreadSrv);

		CleanUpSKKServer();

		UninitLua();

		StartSaveUserDic(FALSE);

		if (message == WM_ENDSESSION)
		{
			BackUpUserDic();
		}

		DeleteCriticalSection(&csSKKSocket);	// !
		DeleteCriticalSection(&csSaveUserDic);	// !
		DeleteCriticalSection(&csUserData);	// !
		DeleteCriticalSection(&csUserDict);	// !

		WSACleanup();

#ifdef _DEBUG
		DeleteObject(hFont);
#endif

		if (message == WM_ENDSESSION)
		{
			// unblock shutdown
			ShutdownBlockReasonDestroy(hWnd);
		}

		if (message == WM_DESTROY)
		{
			PostQuitMessage(0);
		}
		break;

	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}
