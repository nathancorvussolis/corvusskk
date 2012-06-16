
#include "corvustip.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hinstDLL;

		ZeroMemory(&g_ovi, sizeof(g_ovi));
		g_ovi.dwOSVersionInfoSize = sizeof(g_ovi);
		GetVersionEx(&g_ovi);
		break;

	case DLL_PROCESS_DETACH:
		break;

	default:
		break;
	}

	return TRUE;
}
