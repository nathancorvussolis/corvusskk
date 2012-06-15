
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

	BOOL _Create(HWND hwndParent, BOOL regdic);
	void _Destroy();

	void _Move(int x, int y);
	void _BeginUIElement();
	void _EndUIElement();

	HRESULT _OnKeyDown(UINT uVKey);
	HRESULT _OnKeyUp(UINT uVKey);

	void _SetTextRegword(const std::wstring &text, BOOL fixed, BOOL showcandlist);
	BOOL _CanShowUIElement();

private:
	LONG _cRef;

	void _InitList();
	void _UpdateUIElement();
	void _NextPage();
	void _PrevPage();
	void _OnKeyDownRegword(UINT uVKey, BYTE sf);
	std::wstring _EscapeTags(const std::wstring &text);
	void _UpdateTT();
	void _BackUpStatus();
	void _ClearStatus();
	void _RestoreStatusReg();
	void _ClearStatusReg();

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

	HWND _hwnd;
	CTextService *_pTextService;
	CCandidateList *_pCandidateList;

	//候補一覧、辞書登録のウィンドウ
	TOOLINFOW ti;
	std::wstring strTT;
	HFONT hFont;

	BOOL _reg;		//初期表示から辞書登録

	//辞書登録
	BOOL regword;				//モード
	BOOL regwordfixed;			//未確定文字列を確定
	BOOL regwordshowcandlist;	//辞書登録モードで候補一覧表示中
	std::wstring regwordstr;	//確定文字列
	size_t regwordstrpos;		//カーソルインデックス
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
