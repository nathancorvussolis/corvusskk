#pragma once

#include "common.h"

// imcrvcnf
void CreateProperty();
int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
// ConfigCnf
void CreateConfigPath();
void CreateIpcName();
BOOL SetFileDacl(LPWSTR path);
int GetScaledSize(HWND hwnd, int size);
int GetFontHeight(HWND hwnd, int size);
void DrawSelectColor(HWND hDlg, int id, COLORREF col);
void LoadCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault = L"");
void SaveCheckButton(IXmlWriter *pWriter, HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName);
BOOL SaveConfigXml(HWND hDlg);
void ReplaceWellFormed(LPWSTR str, SIZE_T size);
// DlgDicMake
void MakeSKKDic(HWND hDlg);
// DlgDicAddUrl
INT_PTR CALLBACK DlgProcSKKDicAddUrl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
// DlgProcDictionary1
INT_PTR CALLBACK DlgProcDictionary1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveDictionary1(IXmlWriter *pWriter, HWND hDlg);
void SaveDictionary1Server(IXmlWriter *pWriter, HWND hDlg);
// DlgProcDictionary2
INT_PTR CALLBACK DlgProcDictionary2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveDictionary2(IXmlWriter *pWriter, HWND hDlg);
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
extern WCHAR cnfmutexname[MAX_PATH];	//ミューテックス
extern WCHAR cnfcanceldiceventname[MAX_PATH];	//辞書取込キャンセルイベント
extern WCHAR mgrpipename[MAX_PIPENAME];	//名前付きパイプ
extern WCHAR pathconfigxml[MAX_PATH];	//設定
extern WCHAR pathskkdic[MAX_PATH];		//取込SKK辞書
extern WCHAR urlskkdic[INTERNET_MAX_URL_LENGTH];	//URL

#define PROPSHEET_IDTOHWND(hDlg, id) PropSheet_IndexToHwnd(hDlg, PropSheet_IdToIndex(hDlg, id))

#define REPLACE_WELLFORMED(str) ReplaceWellFormed(str, _countof(str))
