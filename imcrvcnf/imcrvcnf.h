
#ifndef IMCRVCNF_H
#define IMCRVCNF_H

#include "common.h"

// imcrvcnf
void CreateProperty();
int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);

// ConfigCnf
void CreateConfigPath();
BOOL SetFileDacl(LPCWSTR path);
int GetScaledSizeX(HWND hwnd, int size);
void DrawSelectColor(HWND hDlg, int id, COLORREF col);

// PropertyConfDictionary
void LoadDictionary(HWND hwnd);
void SaveDictionary(HWND hwnd);
void MakeSKKDic(HWND hwnd);

// PropertyConfConv
void LoadCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault = L"");
void SaveCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName);
void LoadKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault);
void SaveKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName);
void LoadPreservedKey(HWND hwnd);
void SavePreservedKey(HWND hwnd);
void LoadConvPoint(HWND hwnd);
void SaveConvPoint(HWND hwnd);
void LoadKana(HWND hwnd);
void SaveKana(HWND hwnd);
void LoadJLatin(HWND hwnd);
void SaveJLatin(HWND hwnd);
void LoadConfigKanaTxt(LPCWSTR path);
void LoadKanaTxt(HWND hwnd, LPCWSTR path);
void SaveKanaTxt(HWND hwnd, LPCWSTR path);

// dialog procedures
INT_PTR CALLBACK DlgProcDictionary(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcBehavior1(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProcBehavior2(HWND, UINT, WPARAM, LPARAM);
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

extern LPCWSTR TextServiceDesc;

extern IXmlWriter *pXmlWriter;
extern IStream *pXmlFileStream;

extern HINSTANCE hInst;

extern WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];	//設定
extern WCHAR pathskkdic[MAX_PATH];	//取込SKK辞書

#endif //IMCRVCNF_H
