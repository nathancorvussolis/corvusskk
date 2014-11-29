
#include "imcrvtip.h"
#include "TextService.h"
#include "EditSession.h"
#include "CandidateWindow.h"
#include "CandidateList.h"

class CGetTextExtEditSession : public CEditSessionBase
{
public:
	CGetTextExtEditSession(CTextService *pTextService, ITfContext *pContext, ITfContextView *pContextView, ITfRange *pRange, CCandidateWindow *pCandidateWindow) : CEditSessionBase(pTextService, pContext)
	{
		_pContextView = pContextView;
		_pRangeComposition = pRange;
		_pCandidateWindow = pCandidateWindow;
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		RECT rc;
		BOOL fClipped;

		if(_pContextView->GetTextExt(ec, _pRangeComposition, &rc, &fClipped) == S_OK)
		{
			_pCandidateWindow->_Move(&rc);
		}
		return S_OK;
	}

private:
	ITfContextView *_pContextView;
	ITfRange *_pRangeComposition;
	CCandidateWindow *_pCandidateWindow;
};

CCandidateList::CCandidateList(CTextService *pTextService)
{
	DllAddRef();

	_cRef = 1;

	_pTextService = pTextService;

	_hwndParent = NULL;
	_pCandidateWindow = NULL;
	_pRangeComposition = NULL;
	_pContextCandidateWindow = NULL;
	_pContextDocument = NULL;
	_pDocumentMgr = NULL;

	_dwCookieContextKeyEventSink = TF_INVALID_COOKIE;
	_dwCookieTextLayoutSink = TF_INVALID_COOKIE;
}

CCandidateList::~CCandidateList()
{
	_EndCandidateList();

	DllRelease();
}

STDAPI CCandidateList::QueryInterface(REFIID riid, void **ppvObj)
{
	if(ppvObj == NULL)
	{
		return E_INVALIDARG;
	}

	*ppvObj = NULL;

	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfContextKeyEventSink))
	{
		*ppvObj = (ITfContextKeyEventSink *)this;
	}
	else if(IsEqualIID(riid, IID_ITfTextLayoutSink))
	{
		*ppvObj = (ITfTextLayoutSink *)this;
	}

	if(*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) CCandidateList::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) CCandidateList::Release()
{
	if(--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CCandidateList::OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if(pfEaten == NULL)
	{
		return E_INVALIDARG;
	}

	*pfEaten = TRUE;
#ifdef _DEBUG
	_pCandidateWindow->_OnKeyDown((UINT)wParam);
#else
	__try
	{
		_pCandidateWindow->_OnKeyDown((UINT)wParam);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		_pTextService->_ResetStatus();
		_pTextService->_ClearComposition();
	}

#endif
	return S_OK;
}

STDAPI CCandidateList::OnKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if(pfEaten == NULL)
	{
		return E_INVALIDARG;
	}

	*pfEaten = TRUE;

	return S_OK;
}

STDAPI CCandidateList::OnTestKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if(pfEaten == NULL)
	{
		return E_INVALIDARG;
	}

	*pfEaten = TRUE;

	return S_OK;
}

STDAPI CCandidateList::OnTestKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if(pfEaten == NULL)
	{
		return E_INVALIDARG;
	}

	*pfEaten = TRUE;

	return S_OK;
}

STDAPI CCandidateList::OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pContextView)
{
	HRESULT hr;

	if(pContext != _pContextDocument)
	{
		return S_OK;
	}

	switch(lcode)
	{
	case TF_LC_CREATE:
		break;

	case TF_LC_CHANGE:
		if(_pCandidateWindow != NULL)
		{
			try
			{
				CGetTextExtEditSession *pEditSession =
					new CGetTextExtEditSession(_pTextService, pContext, pContextView, _pRangeComposition, _pCandidateWindow);
				pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
				SafeRelease(&pEditSession);
			}
			catch(...)
			{
			}
		}
		break;

	case TF_LC_DESTROY:
		_EndCandidateList();
		break;

	default:
		break;
	}

	return S_OK;
}

HRESULT CCandidateList::_StartCandidateList(TfClientId tfClientId, ITfDocumentMgr *pDocumentMgr,
	ITfContext *pContext, TfEditCookie ec, ITfRange *pRange, BOOL reg)
{
	HRESULT hr = E_FAIL;
	TfEditCookie ecTextStore;
	ITfContextView *pContextView;
	BOOL fClipped;
	RECT rc;
	HWND hwnd = NULL;

	_EndCandidateList();

	if(pDocumentMgr->CreateContext(tfClientId, 0, NULL, &_pContextCandidateWindow, &ecTextStore) != S_OK)
	{
		return E_FAIL;
	}

	if(pDocumentMgr->Push(_pContextCandidateWindow) != S_OK)
	{
		goto exit;
	}

	_pDocumentMgr = pDocumentMgr;
	_pDocumentMgr->AddRef();

	_pContextDocument = pContext;
	_pContextDocument->AddRef();

	_pRangeComposition = pRange;
	_pRangeComposition->AddRef();

	_ec = ec;

	if(_AdviseContextKeyEventSink() != S_OK)
	{
		goto exit;
	}

	if(_AdviseTextLayoutSink() != S_OK)
	{
		goto exit;
	}

	try
	{
		_pCandidateWindow = new CCandidateWindow(_pTextService);

		if(pContext->GetActiveView(&pContextView) != S_OK)
		{
			goto exit;
		}

		if(!_pTextService->_UILessMode && _pCandidateWindow->_CanShowUIElement())
		{
			if(FAILED(pContextView->GetWnd(&hwnd)) || hwnd == NULL)
			{
				hwnd = GetFocus();
			}
		}

		_hwndParent = hwnd;
		if(_hwndParent)
		{
			// for LibreOffice, to get position in screen
			SendMessageW(_hwndParent, WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
		}

		pContextView->GetTextExt(ec, pRange, &rc, &fClipped);

		SafeRelease(&pContextView);

		if(!_pCandidateWindow->_Create(hwnd, NULL, 0, 0, reg))
		{
			goto exit;
		}

		_pCandidateWindow->_Move(&rc);
		_pCandidateWindow->_BeginUIElement();
		_pCandidateWindow->_Redraw();

		hr = S_OK;
	}
	catch(...)
	{
	}

exit:
	if(hr != S_OK)
	{
		_EndCandidateList();
	}
	return hr;
}

void CCandidateList::_InvokeSfHandler(BYTE sf)
{
	if(_pTextService && _pContextDocument)
	{
		_pTextService->_InvokeKeyHandler(_pContextDocument, (WPARAM)0, (LPARAM)0, sf);
		_pTextService->showcandlist = FALSE;
	}
}

void CCandidateList::_EndCandidateList()
{
	if(_hwndParent)
	{
		// for LibreOffice, to get position in screen
		SendMessageW(_hwndParent, WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
		_hwndParent = NULL;
	}

	if(_pCandidateWindow != NULL)
	{
		_pCandidateWindow->_EndUIElement();
		_pCandidateWindow->_Destroy();
	}
	SafeRelease(&_pCandidateWindow);

	SafeRelease(&_pRangeComposition);

	_UnadviseContextKeyEventSink();
	SafeRelease(&_pContextCandidateWindow);

	_UnadviseTextLayoutSink();
	SafeRelease(&_pContextDocument);

	if(_pDocumentMgr != NULL)
	{
		_pDocumentMgr->Pop(0);
	}
	SafeRelease(&_pDocumentMgr);
}

BOOL CCandidateList::_IsShowCandidateWindow()
{
	return (_pCandidateWindow != NULL) ? TRUE : FALSE;
}

BOOL CCandidateList::_IsContextCandidateWindow(ITfContext *pContext)
{
	return (_pContextCandidateWindow == pContext) ? TRUE : FALSE;
}

HRESULT CCandidateList::_AdviseContextKeyEventSink()
{
	HRESULT hr = E_FAIL;

	ITfSource *pSource;
	if(_pContextCandidateWindow->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
	{
		hr = pSource->AdviseSink(IID_IUNK_ARGS((ITfContextKeyEventSink *)this), &_dwCookieContextKeyEventSink);
		SafeRelease(&pSource);
	}

	return hr;
}

HRESULT CCandidateList::_UnadviseContextKeyEventSink()
{
	HRESULT hr = E_FAIL;

	ITfSource *pSource;
	if(_pContextCandidateWindow != NULL)
	{
		if(_pContextCandidateWindow->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
		{
			hr = pSource->UnadviseSink(_dwCookieContextKeyEventSink);
			SafeRelease(&pSource);
		}
	}

	return hr;
}

HRESULT CCandidateList::_AdviseTextLayoutSink()
{
	HRESULT hr = E_FAIL;

	ITfSource *pSource;
	if(_pContextDocument->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
	{
		hr = pSource->AdviseSink(IID_IUNK_ARGS((ITfTextLayoutSink *)this), &_dwCookieTextLayoutSink);
		SafeRelease(&pSource);
	}

	return hr;
}

HRESULT CCandidateList::_UnadviseTextLayoutSink()
{
	HRESULT hr = E_FAIL;

	if(_pContextDocument != NULL)
	{
		ITfSource *pSource;
		if(_pContextDocument->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
		{
			hr = pSource->UnadviseSink(_dwCookieTextLayoutSink);
			SafeRelease(&pSource);
		}
	}

	return hr;
}

void CCandidateList::_Show(BOOL bShow)
{
	if(_pCandidateWindow != NULL)
	{
		_pCandidateWindow->Show(bShow);
	}
}

void CCandidateList::_SetText(const std::wstring &text, BOOL fixed, BOOL showcandlist, BOOL showreg)
{
	if(_pCandidateWindow != NULL)
	{
		_pCandidateWindow->_SetText(text, fixed, showcandlist, showreg);
	}
}

void CCandidateList::_Move(LPRECT lpr)
{
	if(_pCandidateWindow != NULL)
	{
		_pCandidateWindow->_Move(lpr);
	}
}
