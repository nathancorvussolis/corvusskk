
#include "imcrvmgr.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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
	UpdateConfigPath();
	CreateIpcName();

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
	if(IsFileModified(pathconfigxml, &ftConfig))
	{
		LoadConfig();
	}

	ZeroMemory(&ftSKKDic, sizeof(ftSKKDic));
	if(IsFileModified(pathskkdic, &ftSKKDic))
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
