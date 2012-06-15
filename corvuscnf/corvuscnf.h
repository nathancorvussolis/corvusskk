
#ifndef CORVUSCNF_H
#define CORVUSCNF_H

//for resource
#define RC_FILE				"corvuscnf"

#define MAX_KRNLOBJNAME		256
#define CONV_POINT_NUM		32
#define KEYRELEN			256

#define MAX_SKKSERVER_HOST	(255+1)
#define MAX_SKKSERVER_PORT	(5+1)

// corvuscnf
void CreateProperty(HINSTANCE hInst);

// ConfigCnf
void CreateConfigPath();

// PropertyConfDictionary
void LoadDictionary(HWND hwnd);
void SaveDictionary(HWND hwnd);
void MakeSKKDic(HWND hwnd);

// PropertyConfConv
void LoadCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName);
void SaveCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName);
void LoadKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName, LPCWSTR lpDefault);
void SaveKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName);
void LoadConvPoint(HWND hwnd);
void SaveConvPoint(HWND hwnd);
void LoadKana(HWND hwnd);
void SaveKana(HWND hwnd);
void LoadJLatin(HWND hwnd);
void SaveJLatin(HWND hwnd);

extern LPCWSTR TextServiceDesc;

extern IXmlWriter *pXmlWriter;
extern IStream *pXmlFileStream;

extern OSVERSIONINFOW ovi;

extern WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];	//設定
extern WCHAR pathskkcvdicxml[MAX_PATH];	//取込SKK辞書
extern WCHAR pathskkcvdicidx[MAX_PATH];	//取込SKK辞書インデックス

//キー設定
typedef struct {
	int idd;
	LPCWSTR keyName;
	LPCWSTR defaultValue;
} KEYMAP;

#endif
