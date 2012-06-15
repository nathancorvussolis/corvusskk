
#ifndef CORVUSTIP_H
#define CORVUSTIP_H

#define TEXTSERVICE_NAME	L"CorvusSKK"
#define TEXTSERVICE_VER		L"0.6.3"

//for resource
#define RC_AUTHOR			"Corvus Solis"
#define RC_PRODUCT			"CorvusSKK"
#define RC_FILE				"corvustip"
#define RC_VERSION			"0.6.3"
#define RC_VERSION_D		0,6,3,0

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

#define LANGBAR_ITEM_DESC   L"ver. " TEXTSERVICE_VER
#define LANGBAR_FUNC_DESC	TEXTSERVICE_DESC L" " TEXTSERVICE_VER

#define MAX_KRNLOBJNAME 256
#define CORVUSSRVEXE	L"corvussrv.exe"
#define CORVUSCNFEXE	L"corvuscnf.exe"
#ifndef _DEBUG
#define CORVUSKRNLOBJ	L"corvus-skk-"
#else
#define CORVUSKRNLOBJ	L"corvus-skk-debug-"
#endif
#define CORVUSSRVMUTEX	CORVUSKRNLOBJ L"srv-"
#define CORVUSCNFMUTEX	CORVUSKRNLOBJ L"cnf-"
#define CORVUSSRVPIPE	L"\\\\.\\pipe\\" CORVUSKRNLOBJ

//request to corvussrv
#define REQ_SEARCH		L'1'	//辞書検索
#define REQ_COMPLEMENT	L'8'	//補完
#define REQ_USER_ADD_0	L'A'	//ユーザ辞書追加(補完なし)
#define REQ_USER_ADD_1	L'B'	//ユーザ辞書追加(補完あり)
#define REQ_USER_DEL	L'D'	//ユーザ辞書削除
#define REQ_USER_SAVE	L'S'	//ユーザ辞書書き込み
//reply from corvussrv
#define REP_OK			L'1'	//hit
#define REP_FALSE		L'4'	//nothig

//入力モード
enum
{
    im_default = 0,		//デフォルト
    im_hiragana = 1,	//ひらがな
    im_katakana,		//カタカナ
    im_katakana_ank,	//半角ｶﾀｶﾅ (仮名変換のみ)
    im_jlatin,			//全英
    im_ascii			//ASCII
};

//候補   pair< candidate, annotation >
typedef std::pair< std::wstring, std::wstring > CANDIDATEBASE;
//		pair< CANDIDATEBASE(表示用), CANDIDATEBASE(辞書登録用) >
//		例）数値変換の場合 < < "明治四五年", "年号(1868-1912)" >, < "明治#2年", "年号(1868-1912)" > >
typedef std::pair< CANDIDATEBASE, CANDIDATEBASE > CANDIDATE;
typedef std::vector< CANDIDATE > CANDIDATES;

#define CONV_POINT_NUM	32

#define KEYRELEN		256
#define KEYMAPNUM		0x80

//skk function code
#define SKK_NULL		0x00	// NUL
#define SKK_CONV_POINT	0x51	// 変換位置		Q ;

#define SKK_KANA		0x71	// かな／カナ	q
#define SKK_CONV_CHAR	0x11	// ｶﾅ全英変換	c-q
#define SKK_JLATIN		0x4C	// 全英			L
#define SKK_ASCII		0x6C	// アスキー		l
#define SKK_JMODE		0x0A	// ひらがな		c-j(LF)	(c-q)	(ASCII/全英モード)
#define SKK_ABBREV		0x2F	// abbrev		/
#define SKK_AFFIX		0x3E	// 接辞			> <

#define SKK_DIRECT		0x30	// 直接入力		0-9
#define SKK_NEXT_CAND	0x20	// 次候補		SP	c-n
#define SKK_PREV_CAND	0x78	// 前候補		x	c-p
#define SKK_PURGE_DIC	0x58	// 辞書削除		X
#define SKK_NEXT_COMP	0x09	// 次補完		c-i(HT)
#define SKK_PREV_COMP	0x15	// 前補完		c-u

#define SKK_ENTER		0x0D	// 確定			c-m(CR)	c-j(LF)
#define SKK_CANCEL		0x07	// 取消			c-g	(c-[)
#define SKK_BACK		0x08	// 後退			c-h(BS)	VK_BACK
#define SKK_DELETE		0x7F	// 削除			DEL	VK_DELETE
#define SKK_VOID		0xFF	// 無効

#define SKK_LEFT		0x02	// 左移動		c-b	VK_LEFT
#define SKK_UP			0x01	// 先頭移動		c-a	VK_UP
#define SKK_RIGHT		0x06	// 右移動		c-f	VK_RIGHT
#define SKK_DOWN		0x05	// 末尾移動		c-e	VK_DOWN
#define SKK_PASTE		0x19	// 貼付			c-y	(c-v)

//候補一覧選択キー数
#define MAX_SELKEY		7
#define MAX_SELKEY_C	9

extern const WCHAR *TextServiceDesc;
extern const WCHAR *LangbarItemDesc;
extern const WCHAR *LangbarFuncDesc;

extern HINSTANCE g_hInst;

extern OSVERSIONINFO g_ovi;

extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInput;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConverted;

extern const CLSID c_clsidTextService;
extern const GUID c_guidProfile;
extern const GUID c_guidPreservedKeyOnOff;
extern const GUID c_guidLangBarItemButton;
extern const GUID c_guidDisplayAttributeInput;
extern const GUID c_guidDisplayAttributeConverted;
extern const GUID c_guidCandidateListUIElement;

typedef struct {
	BYTE digest[16];
} MD5_DIGEST;

LONG DllAddRef();
LONG DllRelease();

#endif // CORVUSTIP_H
