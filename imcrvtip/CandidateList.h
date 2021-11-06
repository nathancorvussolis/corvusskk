#pragma once

class CCandidateWindow;

class CCandidateList :
	public ITfContextKeyEventSink,
	public ITfTextLayoutSink
{
public:
	CCandidateList(CTextService *pTextService);
	~CCandidateList();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITfContextKeyEventSink
	STDMETHODIMP OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnTestKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnTestKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten);

	// ITfTextLayoutSink
	STDMETHODIMP OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pContextView);

	HRESULT _StartCandidateList(TfClientId tid, ITfDocumentMgr *pDocumentMgr,
		ITfContext *pContext, TfEditCookie ec, ITfRange *pRange, int mode);
	void _EndCandidateList();

	BOOL _IsShowCandidateWindow();
	BOOL _IsContextCandidateWindow(ITfContext *pContext);

	void _InvokeKeyHandler(WPARAM key);
	void _InvokeSfHandler(BYTE sf);
	void _Show(BOOL bShow);
	void _Redraw();
	void _SetText(const std::wstring &text, BOOL fixed, int mode);
	void _Move(LPRECT lpr, TfEditCookie ec = TF_INVALID_EDIT_COOKIE, ITfContext *pContext = nullptr);
	void _UpdateComp();

private:
	LONG _cRef;

	HRESULT _AdviseContextKeyEventSink();
	HRESULT _UnadviseContextKeyEventSink();
	HRESULT _AdviseTextLayoutSink();
	HRESULT _UnadviseTextLayoutSink();

	DWORD _dwCookieContextKeyEventSink;
	DWORD _dwCookieTextLayoutSink;

	CComPtr<CTextService> _pTextService;
	CComPtr<ITfRange> _pRangeComposition;
	CComPtr<ITfContext> _pContextCandidateWindow;
	CComPtr<ITfContext> _pContextDocument;
	CComPtr<ITfDocumentMgr> _pDocumentMgr;
	TfEditCookie _ec;

	CComPtr<CCandidateWindow> _pCandidateWindow;
};
