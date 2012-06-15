
#include "corvuscnf.h"
#include "convtable.h"
#include "resource.h"

// static dialog procedure
INT_PTR CALLBACK DlgProcBehavior(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcDictionary(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcSelKey(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcKeyMap1(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcKeyMap2(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcConvPoint(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcKana(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcJLatin(HWND, UINT, WPARAM, LPARAM);

const WCHAR *TextServiceDesc = TEXTSERVICE_DESC;

#define CCSUNICODE L",ccs=UNICODE"
const WCHAR *RccsUNICODE = L"r" CCSUNICODE;
const WCHAR *WccsUNICODE = L"w" CCSUNICODE;

// ファイルパス
WCHAR pathconfig[MAX_PATH];		//設定
WCHAR pathconfdic[MAX_PATH];	//SKK辞書リスト
WCHAR pathconfcvpt[MAX_PATH];	//変換位置指定
WCHAR pathconfkana[MAX_PATH];	//ローマ字仮名変換表
WCHAR pathconfjlat[MAX_PATH];	//ASCII全英変換表
WCHAR pathskkcvdic[MAX_PATH];	//取込SKK辞書
WCHAR pathskkcvidx[MAX_PATH];	//取込SKK辞書インデックス
// ファイル名
const WCHAR *fnconfig = L"config.ini";
const WCHAR *fnconfdic = L"confdic.txt";
const WCHAR *fnconfcvpt = L"confcvpt.txt";
const WCHAR *fnconfkana = L"confkana.txt";
const WCHAR *fnconfjlat = L"confjlat.txt";
const WCHAR *fnskkcvdic = L"skkcv.dic";
const WCHAR *fnskkcvidx = L"skkcv.idx";

void CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfig[0] = L'\0';
	pathconfdic[0] = L'\0';
	pathconfcvpt[0] = L'\0';
	pathconfkana[0] = L'\0';
	pathconfjlat[0] = L'\0';
	pathskkcvdic[0] = L'\0';
	pathskkcvidx[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, NULL, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	_wmkdir(appdata);

	wcsncpy_s(pathconfig, appdata, _TRUNCATE);
	wcsncat_s(pathconfig, fnconfig, _TRUNCATE);

	wcsncpy_s(pathconfdic, appdata, _TRUNCATE);
	wcsncat_s(pathconfdic, fnconfdic, _TRUNCATE);

	wcsncpy_s(pathconfcvpt, appdata, _TRUNCATE);
	wcsncat_s(pathconfcvpt, fnconfcvpt, _TRUNCATE);

	wcsncpy_s(pathconfkana, appdata, _TRUNCATE);
	wcsncat_s(pathconfkana, fnconfkana, _TRUNCATE);

	wcsncpy_s(pathconfjlat, appdata, _TRUNCATE);
	wcsncat_s(pathconfjlat, fnconfjlat, _TRUNCATE);

	wcsncpy_s(pathskkcvdic, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvdic, fnskkcvdic, _TRUNCATE);

	wcsncpy_s(pathskkcvidx, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvidx, fnskkcvidx, _TRUNCATE);
}

void CreateProperty(HINSTANCE hInst)
{
	PROPSHEETPAGEW psp;
	PROPSHEETHEADERW psh;
	HPROPSHEETPAGE hpsp[8];

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

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_KEYMAP1);
	psp.pfnDlgProc = (DLGPROC)DlgProcKeyMap1;
	hpsp[3] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_KEYMAP2);
	psp.pfnDlgProc = (DLGPROC)DlgProcKeyMap2;
	hpsp[4] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_CONVPOINT);
	psp.pfnDlgProc = (DLGPROC)DlgProcConvPoint;
	hpsp[5] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_KANATBL);
	psp.pfnDlgProc = (DLGPROC)DlgProcKana;
	hpsp[6] = CreatePropertySheetPageW(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_JLATTBL);
	psp.pfnDlgProc = (DLGPROC)DlgProcJLatin;
	hpsp[7] = CreatePropertySheetPageW(&psp);

	ZeroMemory(&psh, sizeof(PROPSHEETHEADERW));
	psh.dwSize = sizeof(PROPSHEETHEADERW);
	psh.dwFlags = PSH_DEFAULT;
	psh.hInstance = hInst;
	psh.hwndParent = NULL;
	psh.nPages = 8;
	psh.phpage = hpsp;
	psh.pszCaption = TEXTSERVICE_DESC L" (ver. " TEXTSERVICE_VER L")";
	PropertySheetW(&psh);

	return;
}
