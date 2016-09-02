
#ifndef IMCRVTIP_H
#define IMCRVTIP_H

#include <tsattrs.h>

#include "common.h"
#include "convtype.h"

//入力モード
enum InputMode
{
	im_disable = -1,	//無効
	im_default,			//デフォルト
	im_hiragana,		//ひらがな
	im_katakana,		//カタカナ
	im_katakana_ank,	//半角ｶﾀｶﾅ
	im_jlatin,			//全英
	im_ascii			//ASCII
};

#define CKEYMAPNUM		0x80	// 0x00-0x7F
#define VKEYMAPNUM		0x100	// 0x00-0xFF

//skk function code
#define SKK_NULL		0x00	// NUL

#define SKK_KANA		0x71	// かな／カナ	q
#define SKK_CONV_CHAR	0x11	// ｶﾅ全英変換	c-q
#define SKK_JLATIN		0x4C	// 全英			L
#define SKK_ASCII		0x6C	// アスキー		l
#define SKK_JMODE		0x0A	// ひらがな		c-j(LF)
#define SKK_ABBREV		0x2F	// abbrev		/
#define SKK_AFFIX		0x3E	// 接辞			> <
#define SKK_NEXT_CAND	0x20	// 次候補		SP	c-n
#define SKK_PREV_CAND	0x78	// 前候補		x	c-p
#define SKK_PURGE_DIC	0x58	// 辞書削除		X
#define SKK_NEXT_COMP	0x09	// 次補完		c-i(HT)
#define SKK_PREV_COMP	0x15	// 前補完		c-u
#define SKK_HINT		0x3B	// 絞り込み		;

#define SKK_CONV_POINT	0x51	// 変換位置		Q ;
#define SKK_DIRECT		0x30	// 直接入力		0-9
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

typedef struct {	//キー設定(文字)
	BYTE keylatin[CKEYMAPNUM];	//全英/アスキー
	BYTE keyjmode[CKEYMAPNUM];	//ひらがな/カタカナ
	BYTE keyvoid[CKEYMAPNUM];	//無効
} CKEYMAP;

typedef struct {	//キー設定(仮想キー)
	BYTE keylatin[VKEYMAPNUM];	//全英/アスキー
	BYTE keyjmode[VKEYMAPNUM];	//ひらがな/カタカナ
	BYTE keyvoid[VKEYMAPNUM];	//無効
} VKEYMAP;

typedef struct {	//変換位置指定(0:開始,1:代替,2:送り)
	WCHAR ch[3];
} CONV_POINT;

//ローマ字仮名ノード
typedef struct ROMAN_KANA_NODE {
	//探索対象、ローマ字1文字分、ルートノードはL'\0'
	WCHAR ch;
	//ローマ字仮名変換
	// ルートノードの各メンバーは空文字列
	// 仮名があるとき、最短で探索したchとconv.romanは等しい
	// 仮名がないとき、各メンバーは空文字列
	ROMAN_KANA_CONV conv;
	//子ノード、メンバーchで昇順ソート
	std::vector<ROMAN_KANA_NODE> nodes;
} ROMAN_KANA_NODE;

#define CHAR_SKK_HINT	L'\x20'		//絞り込みの区切り
#define CHAR_SKK_OKURI	L'\x20'		//送り仮名がまだ無い時点での送りローマ字

#define TKB_NEXT_PAGE	L'\uF003'	//next page key on touch-optimized keyboard
#define TKB_PREV_PAGE	L'\uF004'	//previous page key on touch-optimized keyboard

//候補一覧の色 cx_colors のインデックス
#define CL_COLOR_BG		0	//背景
#define CL_COLOR_FR		1	//枠
#define CL_COLOR_SE		2	//選択
#define CL_COLOR_CO		3	//:
#define CL_COLOR_CA		4	//候補
#define CL_COLOR_SC		5	//;
#define CL_COLOR_AN		6	//注釈
#define CL_COLOR_NO		7	//番号

//候補ウィンドウモード
enum WindowMode {
	wm_none = 0,	//なし
	wm_candidate,	//候補一覧
	wm_register,	//辞書登録
	wm_complement,	//補完一覧
	wm_delete		//候補削除
};

extern LPCWSTR TextServiceDesc;
extern LPCWSTR LangbarItemDesc;
extern LPCWSTR CandidateWindowClass;
extern LPCWSTR InputModeWindowClass;

extern HINSTANCE g_hInst;

extern const CLSID c_clsidTextService;
extern const GUID c_guidProfile;
extern const GUID c_guidPreservedKeyOn;
extern const GUID c_guidPreservedKeyOff;
extern const GUID c_guidLangBarItemButton;
extern const GUID c_guidCandidateListUIElement;

extern const GUID c_guidDisplayAttributeInputMark;
extern const GUID c_guidDisplayAttributeInputText;
extern const GUID c_guidDisplayAttributeInputOkuri;
extern const GUID c_guidDisplayAttributeConvMark;
extern const GUID c_guidDisplayAttributeConvText;
extern const GUID c_guidDisplayAttributeConvOkuri;
extern const GUID c_guidDisplayAttributeConvAnnot;

extern LPCWSTR markNo;
extern LPCWSTR markAnnotation;
extern LPCWSTR markCursor;
extern LPCWSTR markDel;
extern LPCWSTR markReg;
extern LPCWSTR markRegL;
extern LPCWSTR markRegR;
extern LPCWSTR markRegKeyEnd;
extern LPCWSTR markSP;
extern LPCWSTR markNBSP;
extern LPCWSTR markHM;
extern LPCWSTR markMidashi;
extern LPCWSTR markHenkan;
extern LPCWSTR markOkuri;

typedef struct {
	LPCWSTR key;
	const GUID guid;
	const BOOL se;
	const TF_DISPLAYATTRIBUTE da;
} DISPLAYATTRIBUTE_INFO;

extern const DISPLAYATTRIBUTE_INFO c_gdDisplayAttributeInfo[DISPLAYATTRIBUTE_INFO_NUM];

#define SWAPRGB(rgb) (((rgb & 0x0000FF) << 16) | (rgb & 0x00FF00) | ((rgb >> 16) & 0x0000FF))

LONG DllAddRef();
LONG DllRelease();

#define IID_IUNK_ARGS(pType) __uuidof(*(pType)), reinterpret_cast<IUnknown*>(pType)
#define IID_PUNK_ARGS(pType) __uuidof(*(pType)), reinterpret_cast<IUnknown**>(pType)

// added in Windows 8 SDK
#ifndef _WIN32_WINNT_WIN8

#define EVENT_OBJECT_IME_SHOW               0x8027
#define EVENT_OBJECT_IME_HIDE               0x8028
#define EVENT_OBJECT_IME_CHANGE             0x8029

#define TF_TMF_IMMERSIVEMODE          0x40000000

#define TF_IPP_CAPS_IMMERSIVESUPPORT            0x00010000
#define TF_IPP_CAPS_SYSTRAYSUPPORT              0x00020000

extern const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT;
extern const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT;
extern const GUID GUID_LBI_INPUTMODE;

typedef DECLSPEC_UUID("E9967127-FB3C-4978-9008-FB3060D92730")
enum __MIDL_ITfFnGetPreferredTouchKeyboardLayout_0001
{
	TKBLT_UNDEFINED = 0,
	TKBLT_CLASSIC = 1,
	TKBLT_OPTIMIZED = 2
} TKBLayoutType;

MIDL_INTERFACE("5F309A41-590A-4ACC-A97F-D8EFFF13FDFC")
ITfFnGetPreferredTouchKeyboardLayout : public ITfFunction
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetLayout(
		TKBLayoutType *pTKBLayoutType,
		WORD *pwPreferredLayoutId) = 0;
};

#define TKBL_UNDEFINED                 0
#define TKBL_OPT_JAPANESE_ABC                       0x0411

extern const IID IID_ITfFnGetPreferredTouchKeyboardLayout;

#endif //_WIN32_WINNT_WIN8

// added in Windows 8.1 SDK
#ifndef _WIN32_WINNT_WINBLUE

#define D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT (static_cast<D2D1_DRAW_TEXT_OPTIONS>(0x00000004))

#endif //_WIN32_WINNT_WINBLUE

//InstallLayoutOrTip Flags

#define ILOT_UNINSTALL               0x00000001
#define ILOT_DEFPROFILE              0x00000002
#define ILOT_DEFUSER4                0x00000004
#define ILOT_SYSLOCALE               0x00000008
#define ILOT_NOLOCALETOENUMERATE     0x00000010
#define ILOT_NOAPPLYTOCURRENTSESSION 0x00000020
#define ILOT_CLEANINSTALL            0x00000040
#define ILOT_DISABLED                0x00000080

#endif //IMCRVTIP_H
