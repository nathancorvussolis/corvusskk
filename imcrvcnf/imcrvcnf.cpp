
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
	CreateIpcName();

	hMutex = CreateMutexW(nullptr, FALSE, cnfmutexname);
	if(hMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if(hMutex != nullptr)
		{
			CloseHandle(hMutex);
		}
		return 0;
	}

	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icex);

	CreateProperty();

	return 0;
}

void CreateProperty()
{
	struct {
		int id;
		DLGPROC DlgProc;
	} DlgPage[] = {
		{IDD_DIALOG_DICTIONARY,		DlgProcDictionary},
		{IDD_DIALOG_BEHAVIOR1,		DlgProcBehavior1},
		{IDD_DIALOG_BEHAVIOR2,		DlgProcBehavior2},
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

	PROPSHEETPAGEW psp[_countof(DlgPage)];
	ZeroMemory(&psp, sizeof(psp));
	for(int i = 0; i < _countof(psp); i++)
	{
		psp[i].dwSize = sizeof(psp[i]);
		psp[i].dwFlags = PSP_PREMATURE;
		psp[i].hInstance = hInst;
		psp[i].pszTemplate = MAKEINTRESOURCE(DlgPage[i].id);
		psp[i].pfnDlgProc = DlgPage[i].DlgProc;
	}

	PROPSHEETHEADERW psh;
	ZeroMemory(&psh, sizeof(psh));
	psh.dwSize = sizeof(psh);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP | PSH_USECALLBACK;
	psh.hwndParent = nullptr;
	psh.hInstance = hInst;
	psh.pszCaption = TEXTSERVICE_DESC L" ver. " TEXTSERVICE_VER;
	psh.nPages = _countof(psp);
	psh.nStartPage = 0;
	psh.ppsp = psp;
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
