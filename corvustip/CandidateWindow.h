
#ifndef CANDIDATEWINDOW_H
#define CANDIDATEWINDOW_H

#include "TextService.h"

class CCandidateWindow : public ITfCandidateListUIElementBehavior
{
public:
	CCandidateWindow(CTextService *pTextService);
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

	BOOL _Create(HWND hwndParent, CCandidateWindow *pCandidateWindowParent, DWORD dwUIElementId, UINT depth, BOOL reg);
	static LRESULT CALLBACK _WindowPreProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void _Destroy();

	void _Move(int x, int y);
	void _BeginUIElement();
	void _EndUIElement();
	BOOL _CanShowUIElement();

	HRESULT _OnKeyDown(UINT uVKey);

	void _SetText(const std::wstring &text, BOOL fixed, BOOL showcandlist, BOOL showreg);
	void _PreEnd();
	void _End();

private:
	LONG _cRef;

	void _InitList();
	void _UpdateUIElement();
	void _NextPage();
	void _PrevPage();
	void _OnKeyDownRegword(UINT uVKey, BYTE sf);
	std::wstring _EscapeTags(const std::wstring &text);
	void _Update();

	void _BackUpStatus();
	void _ClearStatus();
	void _RestoreStatusReg();
	void _ClearStatusReg();

	void _PreEndReq();
	void _EndReq();
	void _CreateNext(BOOL reg);

	DWORD _dwUIElementId;
	BOOL _bShow;
	DWORD _dwFlags;
	UINT _uShowedCount;
	UINT _uCount;
	UINT _uIndex;
	UINT _uPageCnt;
	std::vector< UINT > _CandCount;
	std::vector< UINT > _PageInex;
	std::vector< std::wstring > _CandStr;

	CTextService *_pTextService;
	CCandidateList *_pCandidateList;
	CCandidateWindow *_pCandidateWindow;		//子
	CCandidateWindow *_pCandidateWindowParent;	//親
	HWND _hwnd;			//自分
	HWND _hwndParent;	//親
	BOOL _preEnd;		//親に対する終了要求
	POINT _pt;			//位置
	UINT _depth;		//深さ

	//候補一覧、辞書登録のウィンドウ
	WNDPROC WndProcDef;
	TOOLINFOW ti;
	std::wstring disptext;
	HFONT hFont;

	BOOL _reg;		//初期表示から辞書登録

	//辞書登録
	BOOL regword;				//モード
	BOOL regwordul;				//
	BOOL regwordfixed;			//未確定文字列を確定
	std::wstring regwordtext;	//確定文字列
	size_t regwordtextpos;		//カーソルインデックス
	std::wstring comptext;		//未確定文字列

	//辞書登録前の状態バックアップ
	int inputmode_bak;
	BOOL abbrevmode_bak;
	std::wstring kana_bak;
	size_t accompidx_bak;
	std::wstring searchkey_bak;
	std::wstring searchkeyorg_bak;
	CANDIDATES candidates_bak;
	size_t candidx_bak;
};

#endif // CANDIDATEWINDOW_H
