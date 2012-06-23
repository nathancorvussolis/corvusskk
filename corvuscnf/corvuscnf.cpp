
#include "common.h"
#include "corvuscnf.h"
#include "resource.h"

OSVERSIONINFOW ovi;

// static dialog procedure
INT_PTR CALLBACK DlgProcBehavior(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcDictionary(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcSelKey(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcPreservedKey(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcKeyMap1(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcKeyMap2(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcConvPoint(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcKana(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcJLatin(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	HANDLE hMutex;
	INITCOMMONCONTROLSEX icex;

	_wsetlocale(LC_ALL, L"japanese");

	ZeroMemory(&ovi, sizeof(ovi));
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);

	CreateConfigPath();

	hMutex = CreateMutexW(NULL, FALSE, cnfmutexname);
	if(hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icex);

	CreateProperty(hInstance);
	
	return 0;
}

void CreateProperty(HINSTANCE hInst)
{
	PROPSHEETPAGEW psp;
	PROPSHEETHEADERW psh;
	HPROPSHEETPAGE hpsp[9];

	ZeroMemory(&psp, sizeof(PROPSHEETPAGEW));
	psp.dwSize = sizeof(PROPSHEETPAGEW);
	psp.dwFlags = PSP_PREMATURE;
	psp.hInstance = hInst;

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_BEHAVIOR);
	psp.pfnDlgProc = (DLGPROC)DlgProcBehavior;
	hpsp[0] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_DICTIONARY);
	psp.pfnDlgProc = (DLGPROC)DlgProcDictionary;
	hpsp[1] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_SELKEY);
	psp.pfnDlgProc = (DLGPROC)DlgProcSelKey;
	hpsp[2] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_PRSRVKEY);
	psp.pfnDlgProc = (DLGPROC)DlgProcPreservedKey;
	hpsp[3] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_KEYMAP1);
	psp.pfnDlgProc = (DLGPROC)DlgProcKeyMap1;
	hpsp[4] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_KEYMAP2);
	psp.pfnDlgProc = (DLGPROC)DlgProcKeyMap2;
	hpsp[5] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_CONVPOINT);
	psp.pfnDlgProc = (DLGPROC)DlgProcConvPoint;
	hpsp[6] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_KANATBL);
	psp.pfnDlgProc = (DLGPROC)DlgProcKana;
	hpsp[7] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_JLATTBL);
	psp.pfnDlgProc = (DLGPROC)DlgProcJLatin;
	hpsp[8] = CreatePropertySheetPageW(&psp);

	ZeroMemory(&psh, sizeof(PROPSHEETHEADERW));
	psh.dwSize = sizeof(PROPSHEETHEADERW);
	psh.dwFlags = PSH_DEFAULT;
	psh.hInstance = hInst;
	psh.hwndParent = NULL;
	psh.nPages = 9;
	psh.phpage = hpsp;
	psh.pszCaption = TEXTSERVICE_DESC L" (ver. " TEXTSERVICE_VER L")";
	PropertySheetW(&psh);

	return;
}
