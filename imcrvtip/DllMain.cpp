
#include "imcrvtip.h"
#include "CandidateWindow.h"
#include "InputModeWindow.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hinstDLL;

		CCandidateWindow::_UninitClass();
		if (!CCandidateWindow::_InitClass())
		{
			return FALSE;
		}

		CInputModeWindow::_UninitClass();
		if (!CInputModeWindow::_InitClass())
		{
			return FALSE;
		}
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		CCandidateWindow::_UninitClass();
		CInputModeWindow::_UninitClass();
		break;

	default:
		break;
	}

	return TRUE;
}
