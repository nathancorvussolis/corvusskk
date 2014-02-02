
#ifndef TEXTSERVICE_H
#define TEXTSERVICE_H

#include "imcrvtip.h"
#include "convtype.h"

class CLangBarItemButton;
class CCandidateList;
class CInputModeWindow;

class CTextService :
	public ITfTextInputProcessorEx,
	public ITfThreadMgrEventSink,
	public ITfThreadFocusSink,
	public ITfCompartmentEventSink,
	public ITfTextEditSink,
	public ITfKeyEventSink,
	public ITfCompositionSink,
	public ITfDisplayAttributeProvider,
	public ITfFunctionProvider,
	public ITfFnConfigure,
	public ITfFnShowHelp,
	public ITfFnGetPreferredTouchKeyboardLayout
{
public:
	CTextService();
	~CTextService();
	
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITfTextInputProcessor
	STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid);
	STDMETHODIMP Deactivate();

	// ITfTextInputProcessorEx
	STDMETHODIMP ActivateEx(ITfThreadMgr *ptim, TfClientId tid, DWORD dwFlags);

	// ITfThreadMgrEventSink
	STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pdim);
	STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pdim);
	STDMETHODIMP OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus);
	STDMETHODIMP OnPushContext(ITfContext *pic);
	STDMETHODIMP OnPopContext(ITfContext *pic);

	// ITfTextEditSink
	STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

	// ITfThreadFocusSink
	STDMETHODIMP OnSetThreadFocus();
	STDMETHODIMP OnKillThreadFocus();

	// ItfCompartmentEventSink
	STDMETHODIMP OnChange(REFGUID rguid);

	// ITfKeyEventSink
	STDMETHODIMP OnSetFocus(BOOL fForeground);
	STDMETHODIMP OnTestKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten);

	// ITfCompositionSink
	STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition);

	// ITfDisplayAttributeProvider
	STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum);
	STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo);

	// ITfFunctionProvider
	STDMETHODIMP GetType(GUID *pguid);
	STDMETHODIMP GetDescription(BSTR *pbstrDesc);
	STDMETHODIMP GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk);

	// ITfFunction
	STDMETHODIMP GetDisplayName(BSTR *pbstrName);

	// ITfFnConfigure
	STDMETHODIMP Show(HWND hwndParent, LANGID langid, REFGUID rguidProfile);

	// ITfFnShowHelp
	STDMETHODIMP Show(HWND hwndParent);

	// ITfFnGetPreferredTouchKeyboardLayout
	STDMETHODIMP GetLayout(TKBLayoutType *pTKBLayoutType, WORD *pwPreferredLayoutId);

	ITfThreadMgr *_GetThreadMgr()
	{
		return _pThreadMgr;
	}
	TfClientId _GetClientId()
	{
		return _ClientId;
	}
	ITfComposition *_GetComposition()
	{
		return _pComposition;
	}
	CCandidateList *_GetCandidateList()
	{
		return _pCandidateList;
	}

	// Compartment
	HRESULT _SetCompartment(REFGUID rguid, const VARIANT *pvar);
	HRESULT _GetCompartment(REFGUID rguid, VARIANT *pvar);
	BOOL _IsKeyboardDisabled();
	BOOL _IsKeyboardOpen();
	HRESULT _SetKeyboardOpen(BOOL fOpen);

	// Composition
	BOOL _IsComposing();
	void _SetComposition(ITfComposition *pComposition);
	BOOL _IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover);
	void _StartComposition(ITfContext *pContext);
	void _TerminateComposition(TfEditCookie ec, ITfContext *pContext);
	void _EndComposition(ITfContext *pContext);
	void _CancelComposition(TfEditCookie ec, ITfContext *pContext);
	void _ClearComposition();

	// LanguageBar
	void _UpdateLanguageBar(BOOL showinputmode = TRUE);
	void _GetIcon(HICON *phIcon);
	
	// DisplayAttribureProvider
	void _ClearCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext);
	BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext, ITfRange *pRange, TfGuidAtom gaDisplayAttribute);

	// KeyHandler
	HRESULT _InvokeKeyHandler(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BYTE bSf);
	HRESULT _HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam, BYTE bSf);
	void _KeyboardOpenCloseChanged();
	void _KeyboardInputConversionChanged();
	BOOL _IsKeyVoid(WCHAR ch, BYTE vk);
	void _ResetStatus();

	// KeyHandlerChar
	HRESULT _HandleChar(TfEditCookie ec, ITfContext *pContext, std::wstring &composition, WPARAM wParam, WCHAR ch, WCHAR chO);
	HRESULT _HandleCharReturn(TfEditCookie ec, ITfContext *pContext, BOOL back = FALSE);
	HRESULT _HandleCharTerminate(TfEditCookie ec, ITfContext *pContext, std::wstring &composition);

	// KeyHandlerCompostion
	HRESULT _Update(TfEditCookie ec, ITfContext *pContext, BOOL fixed = FALSE, BOOL back = FALSE);
	HRESULT _Update(TfEditCookie ec, ITfContext *pContext, std::wstring &composition, BOOL fixed = FALSE, BOOL back = FALSE);
	HRESULT _SetText(TfEditCookie ec, ITfContext *pContext, const std::wstring &text, LONG cchCursor, LONG cchOkuri, BOOL fixed);
	HRESULT _ShowCandidateList(TfEditCookie ec, ITfContext *pContext, BOOL reg);

	// KeyHandlerControl
	HRESULT _HandleControl(TfEditCookie ec, ITfContext *pContext, BYTE sf, WCHAR &ch);

	// KeyHandlerConv
	WCHAR _GetCh(BYTE vk, BYTE vkoff = 0);
	BYTE _GetSf(BYTE vk, WCHAR ch);
	HRESULT _ConvRomanKana(ROMAN_KANA_CONV *pconv);
	HRESULT _ConvAsciiJLatin(ASCII_JLATIN_CONV *pconv);
	void _StartConv();
	void _StartSubConv();
	void _NextConv();
	void _PrevConv();
	void _NextComp();
	void _PrevComp();
	void _SetComp(const std::wstring &candidate);
	BOOL _ConvN(WCHAR ch);
	BOOL _ConvNN();
	void _ConvKanaToKana(std::wstring &dst, int dstmode, const std::wstring &src, int srcmode);

	// KeyHandlerDictionary
	void _ConnectDic();
	void _DisconnectDic();
	void _SearchDic(WCHAR command);
	void _ConvertCandidate(std::wstring &conv, const std::wstring &key, const std::wstring &candidate);
	void _AddUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate, const std::wstring &annotation);
	void _DelUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate);
	void _SaveUserDic();
	void _StartManager();
	void _StartConfigure();
	void _StartProcess(LPCWSTR fname);

	// FnConfigure
	void _CreateConfigPath();
	void _ReadBoolValue(LPCWSTR section, LPCWSTR key, BOOL &value, BOOL defval);
	void _LoadBehavior();
	void _LoadDisplayAttr();
	void _LoadSelKey();
	void _LoadPreservedKey();
	void _LoadKeyMap(LPCWSTR section, KEYMAP &keymap);
	void _LoadConvPoint();
	void _LoadKana();
	void _LoadJLatin();

private:
	LONG _cRef;

	BOOL _InitThreadMgrEventSink();
	void _UninitThreadMgrEventSink();

	BOOL _InitThreadFocusSink();
	void _UninitThreadFocusSink();

	BOOL _InitCompartmentEventSink();
	void _UninitCompartmentEventSink();

	BOOL _InitTextEditSink(ITfDocumentMgr *pDocumentMgr);

	BOOL _InitKeyEventSink();
	void _UninitKeyEventSink();

	BOOL _InitPreservedKey();
	void _UninitPreservedKey();

	BOOL _InitLanguageBar();
	void _UninitLanguageBar();

	BOOL _InitDisplayAttributeGuidAtom();

	BOOL _InitFunctionProvider();
	void _UninitFunctionProvider();

	BOOL _IsKeyEaten(ITfContext *pContext, WPARAM wParam);

	ITfThreadMgr *_pThreadMgr;
	TfClientId _ClientId;

	DWORD _dwThreadMgrEventSinkCookie;
	DWORD _dwThreadFocusSinkCookie;
	DWORD _dwCompartmentEventSinkOpenCloseCookie;
	DWORD _dwCompartmentEventSinkInputmodeConversionCookie;

	ITfContext *_pTextEditSinkContext;
	DWORD _dwTextEditSinkCookie;

	ITfComposition *_pComposition;

	CLangBarItemButton *_pLangBarItem;
	CLangBarItemButton *_pLangBarItemI;

	CCandidateList *_pCandidateList;

	CInputModeWindow *_pInputModeWindow;

	TfGuidAtom _gaDisplayAttributeInputMark;
	TfGuidAtom _gaDisplayAttributeInputText;
	TfGuidAtom _gaDisplayAttributeInputOkuri;
	TfGuidAtom _gaDisplayAttributeConvMark;
	TfGuidAtom _gaDisplayAttributeConvText;
	TfGuidAtom _gaDisplayAttributeConvOkuri;
	TfGuidAtom _gaDisplayAttributeConvAnnot;

private:
	//ファイルパス
	WCHAR pathconfigxml[MAX_PATH];	//設定
	FILETIME ftconfigxml;			//更新時刻

	//imcrvmgr.exe との名前付きパイプ
	WCHAR mgrpipename[MAX_KRNLOBJNAME];
	HANDLE hPipe;
	//ミューテックス
	WCHAR mgrmutexname[MAX_KRNLOBJNAME];
	WCHAR cnfmutexname[MAX_KRNLOBJNAME];

	//キーマップ
	KEYMAP ckeymap;
	KEYMAP vkeymap;

	//変換位置指定（開始,代替,送り）
	WCHAR conv_point[CONV_POINT_NUM][3];

	//変換テーブル
	std::vector<ROMAN_KANA_CONV> roman_kana_conv;
	ASCII_JLATIN_CONV ascii_jlatin_conv[ASCII_JLATIN_TBL_NUM];

public:
	DWORD _dwActiveFlags;	//ITfThreadMgrEx::GetActiveFlags()
	BOOL _ImmersiveMode;	//Immersive Mode
	BOOL _UILessMode;		//UILess Mode

	//状態
	int inputmode;			//入力モード (無し/ひらがな/カタカナ/半角ｶﾀｶﾅ/全英/アスキー)
	BOOL inputkey;			//見出し入力▽モード
	BOOL abbrevmode;		//abbrevモード
	BOOL showentry;			//候補表示▼モード
	BOOL showcandlist;		//候補リスト表示
	BOOL complement;		//補完
	BOOL purgedicmode;		//辞書削除モード
	BOOL hintmode;			//ヒントモード

	//動作設定
	WCHAR cx_fontname[LF_FACESIZE];	//候補一覧のフォント設定(フォント名)
	int cx_fontpoint;				//候補一覧のフォント設定(サイズ)
	int cx_fontweight;				//候補一覧のフォント設定(太さ)
	BOOL cx_fontitalic;				//候補一覧のフォント設定(イタリック)

	LONG cx_maxwidth;			//候補一覧の最大幅
	COLORREF cx_colors[8];		//候補一覧の色
	size_t cx_untilcandlist;	//候補一覧表示に要する変換回数(0:表示なし/1:1回目)
	BOOL cx_verticalcand;		//候補一覧を縦に表示する
	BOOL cx_dispcandnum;		//候補一覧表示なしのとき候補数を表示する
	BOOL cx_annotation;			//注釈を表示する
	BOOL cx_annotatlst;			//注釈を表示する（候補一覧のみ）
	BOOL cx_showmodeinl;		//入力モードを表示する
	BOOL cx_showmodeimm;		//入力モードを表示する（没入型のみ）
	BOOL cx_showmodemark;		//▽▼*マークを表示する
	BOOL cx_showroman;			//ローマ字を表示する

	BOOL cx_begincvokuri;		//送り仮名が決定したとき変換を開始する
	BOOL cx_keepinputnor;		//ローマ字が無いとき最後の入力を残す
	BOOL cx_delcvposcncl;		//取消のとき変換位置を削除する
	BOOL cx_delokuricncl;		//取消のとき送り仮名を削除する
	BOOL cx_backincenter;		//後退に確定を含める
	BOOL cx_addcandktkn;		//候補に片仮名変換を追加する
	BOOL cx_shiftnnokuri;		//送り仮名で撥音を送り出す

	//ローマ字・仮名
	std::wstring roman;		//ローマ字
	std::wstring kana;		//仮名
	size_t okuriidx;		//送り仮名インデックス

	//検索用見出し語
	std::wstring searchkey;		//数値変換で数値→#
	std::wstring searchkeyorg;	//オリジナル

	//候補
	CANDIDATES candidates;	//候補
	size_t candidx;			//候補インデックス
	size_t candorgcnt;		//オリジナル見出し語の候補数

	size_t cursoridx;		//カーソルインデックス

	//候補一覧選択キー
	WCHAR selkey[MAX_SELKEY_C][2][2];

	//preserved key
	TF_PRESERVEDKEY preservedkey[MAX_PRESERVEDKEY];

	//表示属性   別のインスタンスからGetDisplayAttributeInfo()が呼ばれるのでstaticで
	static BOOL display_attribute_series[DISPLAYATTRIBUTE_INFO_NUM];
	static TF_DISPLAYATTRIBUTE display_attribute_info[DISPLAYATTRIBUTE_INFO_NUM];
};

#endif //TEXTSERVICE_H
