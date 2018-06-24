
#ifndef IMCRVCNF_H
#define IMCRVCNF_H

#include "common.h"

// imcrvcnf
void CreateProperty();
int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
// ConfigCnf
void CreateConfigPath();
void CreateIpcName();
BOOL SetFileDacl(LPWSTR path);
int GetScaledSizeX(HWND hwnd, int size);
int GetFontHeight(HWND hwnd, int size);
void DrawSelectColor(HWND hDlg, int id, COLORREF col);
void LoadCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault = L"");
void SaveCheckButton(IXmlWriter *pWriter, HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName);
void SaveConfigXml(HWND hDlg);
// DlgDicMake
void MakeSKKDic(HWND hDlg);
// DlgDicAddUrl
INT_PTR CALLBACK DlgProcSKKDicAddUrl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
// DlgProcDictionary
INT_PTR CALLBACK DlgProcDictionary(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveDictionary(IXmlWriter *pWriter, HWND hDlg);
void SaveServer(IXmlWriter *pWriter, HWND hDlg);
// DlgProcBehavior1
INT_PTR CALLBACK DlgProcBehavior1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveFont(IXmlWriter *pWriter, HWND hDlg);
void SaveBehavior1(IXmlWriter *pWriter, HWND hDlg);
// DlgProcBehavior2
INT_PTR CALLBACK DlgProcBehavior2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveBehavior2(IXmlWriter *pWriter, HWND hDlg);
// DlgProcDisplay1
INT_PTR CALLBACK DlgProcDisplay1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveDisplay1(IXmlWriter *pWriter, HWND hDlg);
// DlgProcDisplay2
INT_PTR CALLBACK DlgProcDisplay2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveDisplay2(IXmlWriter *pWriter, HWND hDlg);
// DlgProcDisplayAttr
INT_PTR CALLBACK DlgProcDisplayAttr1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcDisplayAttr2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveDisplayAttr1(IXmlWriter *pWriter, HWND hDlg);
void SaveDisplayAttr2(IXmlWriter *pWriter, HWND hDlg);
// DlgProcSelKey
INT_PTR CALLBACK DlgProcSelKey(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveSelKey(IXmlWriter *pWriter, HWND hDlg);
// DlgProcPreservedKey
INT_PTR CALLBACK DlgProcPreservedKey(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SavePreservedKeyON(IXmlWriter *pWriter, HWND hDlg);
void SavePreservedKeyOFF(IXmlWriter *pWriter, HWND hDlg);
// DlgProcKeyMap
INT_PTR CALLBACK DlgProcKeyMap1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcKeyMap2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveCKeyMap(IXmlWriter *pWriter, HWND hDlg);
void SaveVKeyMap(IXmlWriter *pWriter, HWND hDlg);
// DlgProcConvPoint
INT_PTR CALLBACK DlgProcConvPoint(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveConvPoint(IXmlWriter *pWriter, HWND hDlg);
// DlgProcKana
INT_PTR CALLBACK DlgProcKana(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveKana(IXmlWriter *pWriter, HWND hDlg);
// DlgProcJLatin
INT_PTR CALLBACK DlgProcJLatin(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveJLatin(IXmlWriter *pWriter, HWND hDlg);

extern LPCWSTR TextServiceDesc;
extern HINSTANCE hInst;
extern WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス
extern WCHAR pathconfigxml[MAX_PATH];		//設定
extern WCHAR pathskkdic[MAX_PATH];			//取込SKK辞書
extern WCHAR urlskkdic[INTERNET_MAX_URL_LENGTH];

// Per-Monitor DPI Awareness V2
#ifndef NTDDI_WIN10_RS2
#if (WINVER < 0x0605)
#define WM_DPICHANGED_BEFOREPARENT      0x02E2
#define WM_DPICHANGED_AFTERPARENT       0x02E3
#define WM_GETDPISCALEDSIZE             0x02E4
#endif //(WINVER < 0x0605)
#endif //NTDDI_WIN10_RS2

#endif //IMCRVCNF_H
