
#include "imcrvcnf.h"
#include "resource.h"

HINSTANCE hInst;

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
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icex);

	CreateProperty();

	return 0;
}

void CreateProperty()
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

	ZeroMemory(&psp, sizeof(PROPSHEETPAGEW));
	psp.dwSize = sizeof(PROPSHEETPAGEW);
	psp.dwFlags = PSP_PREMATURE;
	psp.hInstance = hInst;

	for(int i = 0; i < _countof(DlgPage); i++)
	{
		psp.pszTemplate = MAKEINTRESOURCE(DlgPage[i].id);
		psp.pfnDlgProc = DlgPage[i].DlgProc;
		hpsp[i] = CreatePropertySheetPageW(&psp);
	}

	ZeroMemory(&psh, sizeof(PROPSHEETHEADERW));
	psh.dwSize = sizeof(PROPSHEETHEADERW);
	psh.dwFlags = PSH_DEFAULT | PSH_NOCONTEXTHELP | PSH_USECALLBACK;
	psh.hwndParent = NULL;
	psh.hInstance = hInst;
	psh.pszCaption = TEXTSERVICE_DESC L" (ver. " TEXTSERVICE_VER L")";
	psh.nPages = _countof(DlgPage);
	psh.nStartPage = 0;
	psh.phpage = hpsp;
	psh.pfnCallback = PropSheetProc;

	PropertySheetW(&psh);

	return;
}

int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
	switch(uMsg)
	{
	case PSCB_INITIALIZED:
		//imcrvmgr.exeから実行されるときimcrvtip.dllで
		//AllowSetForegroundWindow関数が実行済みのはず
		SetForegroundWindow(hwndDlg);
		break;
	case PSCB_PRECREATE:
		break;
	case PSCB_BUTTONPRESSED:
		break;
	default:
		break;
	}
	return 0;
}
