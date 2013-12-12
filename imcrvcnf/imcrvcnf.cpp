
#include "imcrvcnf.h"
#include "resource.h"

HINSTANCE hInst;

// static dialog procedure
INT_PTR CALLBACK DlgProcDictionary(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcBehavior(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcDisplay(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcDisplayAttrInput(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcDisplayAttrConv(HWND, UINT, WPARAM, LPARAM);
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

	_wsetlocale(LC_ALL, L"JPN");

	hInst = hInstance;

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

void CreateProperty(HINSTANCE hInstance)
{
	PROPSHEETPAGEW psp;
	PROPSHEETHEADERW psh;
	struct {
		int id;
		DLGPROC DlgProc;
	} DlgPage[] = {
		{IDD_DIALOG_DICTIONARY,		DlgProcDictionary},
		{IDD_DIALOG_BEHAVIOR,		DlgProcBehavior},
		{IDD_DIALOG_DISPLAY,		DlgProcDisplay},
		{IDD_DIALOG_DISPLAYATTR1,	DlgProcDisplayAttrInput},
		{IDD_DIALOG_DISPLAYATTR2,	DlgProcDisplayAttrConv},
		{IDD_DIALOG_SELKEY,			DlgProcSelKey},
		{IDD_DIALOG_PRSRVKEY,		DlgProcPreservedKey},
		{IDD_DIALOG_KEYMAP1,		DlgProcKeyMap1},
		{IDD_DIALOG_KEYMAP2,		DlgProcKeyMap2},
		{IDD_DIALOG_CONVPOINT,		DlgProcConvPoint},
		{IDD_DIALOG_KANATBL,		DlgProcKana},
		{IDD_DIALOG_JLATTBL,		DlgProcJLatin}
	};
	HPROPSHEETPAGE hpsp[_countof(DlgPage)];
	int i;

	ZeroMemory(&psp, sizeof(PROPSHEETPAGEW));
	psp.dwSize = sizeof(PROPSHEETPAGEW);
	psp.dwFlags = PSP_PREMATURE;
	psp.hInstance = hInst;

	for(i = 0; i < _countof(DlgPage); i++)
	{
		psp.pszTemplate = MAKEINTRESOURCE(DlgPage[i].id);
		psp.pfnDlgProc = DlgPage[i].DlgProc;
		hpsp[i] = CreatePropertySheetPageW(&psp);
	}

	ZeroMemory(&psh, sizeof(PROPSHEETHEADERW));
	psh.dwSize = sizeof(PROPSHEETHEADERW);
	psh.dwFlags = PSH_DEFAULT | PSH_NOCONTEXTHELP;
	psh.hInstance = hInstance;
	psh.hwndParent = NULL;
	psh.nPages = _countof(DlgPage);
	psh.phpage = hpsp;
	psh.pszCaption = TEXTSERVICE_DESC L" (ver. " TEXTSERVICE_VER L")";
	PropertySheetW(&psh);

	return;
}
