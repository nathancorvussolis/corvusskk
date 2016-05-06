
#include "imcrvtip.h"
#include "CandidateWindow.h"
#include "InputModeWindow.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hinstDLL;
		if(!CCandidateWindow::_InitClass())
		{
			return FALSE;
		}
		if(!CInputModeWindow::_InitClass())
		{
			return FALSE;
		}
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
