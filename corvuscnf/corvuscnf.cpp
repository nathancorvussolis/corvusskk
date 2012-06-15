
#include "corvuscnf.h"

int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HANDLE hMutex;
	INITCOMMONCONTROLSEX icex;

	hMutex = CreateMutex(NULL, FALSE, CORVUSCNFMUTEX);
	if(hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}

	setlocale(LC_ALL, "japanese");

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icex);

	CreateConfigPath();

	CreateProperty(hInstance);
	
	return 0;
}
