
#ifndef CANDIDATELIST_H
#define CANDIDATELIST_H

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
		ITfContext *pContext, TfEditCookie ec, ITfRange *pRange, BOOL reg, BOOL comp);
	void _EndCandidateList();

	BOOL _IsShowCandidateWindow();
	BOOL _IsContextCandidateWindow(ITfContext *pContext);

	void _InvokeKeyHandler(WPARAM key);
	void _InvokeSfHandler(BYTE sf);
	void _Show(BOOL bShow);
	void _SetText(const std::wstring &text, BOOL fixed, BOOL showcandlist, BOOL showreg);
	void _Move(LPRECT lpr, TfEditCookie ec = TF_INVALID_EDIT_COOKIE, ITfContext *pContext = NULL);
	void _UpdateComp();

private:
	LONG _cRef;

	HRESULT _AdviseContextKeyEventSink();
	HRESULT _UnadviseContextKeyEventSink();
	HRESULT _AdviseTextLayoutSink();
	HRESULT _UnadviseTextLayoutSink();

	DWORD _dwCookieContextKeyEventSink;
	DWORD _dwCookieTextLayoutSink;

	CTextService *_pTextService;
	ITfRange *_pRangeComposition;
	ITfContext *_pContextCandidateWindow;
	ITfContext *_pContextDocument;
	ITfDocumentMgr *_pDocumentMgr;
	TfEditCookie _ec;

	CCandidateWindow *_pCandidateWindow;

	BOOL _comp;		//複数補完/複数動的補完
};

#endif //CANDIDATELIST_H
