#pragma once

#include "TextService.h"
#include "CandidateList.h"
#include "InputModeWindow.h"

class CCandidateWindow : public ITfCandidateListUIElementBehavior
{
public:
	CCandidateWindow(CTextService *pTextService, CCandidateList *pCandidateList);
	~CCandidateWindow();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITfUIElement
	STDMETHODIMP GetDescription(BSTR *bstr);
	STDMETHODIMP GetGUID(GUID *pguid);
	STDMETHODIMP Show(BOOL bShow);
	STDMETHODIMP IsShown(BOOL *pbShow);

	// ITfCandidateListUIElement
	STDMETHODIMP GetUpdatedFlags(DWORD *pdwFlags);
	STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppdim);
	STDMETHODIMP GetCount(UINT *puCount);
	STDMETHODIMP GetSelection(UINT *puIndex);
	STDMETHODIMP GetString(UINT uIndex, BSTR *pstr);
	STDMETHODIMP GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt);
	STDMETHODIMP SetPageIndex(UINT *pIndex, UINT uPageCnt);
	STDMETHODIMP GetCurrentPage(UINT *puPage);

	// ITfCandidateListUIElementBehavior
	STDMETHODIMP SetSelection(UINT nIndex);
	STDMETHODIMP Finalize();
	STDMETHODIMP Abort();

	BOOL _Create(HWND hwndParent, CCandidateWindow *pCandidateWindowParent, DWORD dwUIElementId, UINT depth, int mode);
	static BOOL _InitClass();
	static void _UninitClass();
	static LRESULT CALLBACK _WindowPreProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void _Destroy();
	void _Move(LPCRECT lpr, TfEditCookie ec = TF_INVALID_EDIT_COOKIE, ITfContext *pContext = nullptr);
	void _BeginUIElement();
	void _EndUIElement();
	BOOL _CanShowUIElement();
	void _Redraw();
	void _SetText(const std::wstring &text, BOOL fixed, int mode);
	void _PreEnd();
	void _End();
	void _UpdateComp();

	//KeyHandler
	HRESULT _OnKeyDown(UINT uVKey);

private:
	LONG _cRef;

	void _InitList();
	void _UpdateUIElement();
	void _NextPage();
	void _PrevPage();
	void _NextComp();
	void _PrevComp();
	void _Update();
	void _BackUpStatus();
	void _ClearStatus();
	void _RestoreStatusReg();
	void _ClearStatusReg();
	void _PreEndReq();
	void _EndReq();
	void _CreateNext(int mode);

	//KeyHandler
	void _OnKeyDownRegword(UINT uVKey);
	void _InvokeSfHandler(BYTE sf);
	void _InvokeKeyHandler(UINT uVKey);
	void _HandleKey(WPARAM wParam, BYTE bSf);
	void _GetChSf(UINT uVKey, WCHAR &ch, BYTE &sf, BYTE vkoff = 0);

	//Paint
	void _WindowProcPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	std::wstring _MakeRegWordString();
	std::wstring _MakeDelWordString();
	void _PaintWord(HDC hdc, LPRECT lpr);
	std::wstring _MakeCandidateString(UINT page, UINT count, UINT idx, int cycle);
	void _PaintCandidate(HDC hdc, LPRECT lpr, UINT page, UINT count, UINT idx);
	void _CalcWindowRect();
	HRESULT _GetTextMetrics(LPCWSTR text, DWRITE_TEXT_METRICS *metrics);
	void _InitFont();
	void _UninitFont();
	void _WindowProcDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	DWORD _dwUIElementId;
	BOOL _bShow;
	DWORD _dwFlags;
	UINT _uShowedCount;
	UINT _uCount;
	UINT _uIndex;
	UINT _uPageCnt;
	std::vector< UINT > _CandCount;
	std::vector< UINT > _PageIndex;
	std::vector< std::wstring > _CandStr;
	UINT _uPageCandNum;

	CComPtr<CTextService> _pTextService;
	CComPtr<CCandidateList> _pCandidateList;
	CComPtr<CCandidateWindow> _pCandidateWindow;		//子
	CComPtr<CCandidateWindow> _pCandidateWindowParent;	//親
	CComPtr<CInputModeWindow> _pInputModeWindow;
	HWND _hwnd;			//自分
	HWND _hwndParent;	//親
	BOOL _preEnd;		//親に対する終了要求
	RECT _rect;			//親の位置
	UINT _depth;		//深さ
	BOOL _vertical;		//縦書き

	//候補一覧、辞書登録のウィンドウ
	std::wstring disptext;		//表示文字列
	HFONT hFont;				//フォント
	int _dpi;

	//Direct2D/DirectWrite
	CComPtr<ID2D1Factory> _pD2DFactory;
	CComPtr<ID2D1DCRenderTarget> _pD2DDCRT;
	CComPtr<ID2D1SolidColorBrush> _pD2DBrush[DISPLAY_LIST_COLOR_NUM];
	D2D1_DRAW_TEXT_OPTIONS _drawtext_option;
	CComPtr<IDWriteFactory> _pDWFactory;
	CComPtr<IDWriteTextFormat> _pDWTF;

	int _mode;		//モード
	BOOL _ulsingle;	//UILess 辞書登録/辞書削除

	//辞書登録
	BOOL _regmode;				//辞書登録モード
	BOOL _regfixed;				//未確定文字列を確定
	std::wstring _regtext;		//確定文字列
	size_t _regtextpos;			//カーソルインデックス
	std::wstring _regcomp;		//未確定文字列

	CANDIDATES candidates;		//描画用候補
	size_t candidx;				//描画用候補インデックス
	size_t candorgcnt;			//オリジナル見出し語の候補数
	std::wstring searchkey;		//描画用見出し語
	std::wstring searchkeyorg;	//描画用オリジナル見出し語

	//辞書登録前の状態バックアップ
	int inputmode_bak;
	BOOL abbrevmode_bak;
	std::wstring kana_bak;
	size_t okuriidx_bak;
	size_t cursoridx_bak;
	std::wstring searchkey_bak;
	std::wstring searchkeyorg_bak;
	CANDIDATES candidates_bak;
	size_t candidx_bak;
	size_t candorgcnt_bak;
	BOOL reconversion_bak;
	std::wstring reconvsrc_bak;
};
