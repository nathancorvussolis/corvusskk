
#include "imcrvtip.h"
#include "TextService.h"
#include "EditSession.h"
#include "CandidateList.h"
#include "CandidateWindow.h"

class CCandidateListGetTextExtEditSession : public CEditSessionBase
{
public:
	CCandidateListGetTextExtEditSession(CTextService *pTextService, ITfContext *pContext,
		ITfContextView *pContextView, ITfRange *pRange, CCandidateWindow *pCandidateWindow) : CEditSessionBase(pTextService, pContext)
	{
		_pCandidateWindow = pCandidateWindow;
		_pRangeComposition = pRange;
		_pContextView = pContextView;
	}

	~CCandidateListGetTextExtEditSession()
	{
		_pCandidateWindow.Release();
		_pRangeComposition.Release();
		_pContextView.Release();
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		RECT rc = {};
		BOOL fClipped;
		if (SUCCEEDED(_pContextView->GetTextExt(ec, _pRangeComposition, &rc, &fClipped)))
		{
			_pCandidateWindow->_Move(&rc, ec, _pContext);
		}

		return S_OK;
	}

private:
	CComPtr<ITfContextView> _pContextView;
	CComPtr<ITfRange> _pRangeComposition;
	CComPtr<CCandidateWindow> _pCandidateWindow;
};

CCandidateList::CCandidateList(CTextService *pTextService)
{
	DllAddRef();

	_cRef = 1;

	_pTextService = pTextService;

	_pCandidateWindow = nullptr;
	_pRangeComposition = nullptr;
	_pContextCandidateWindow = nullptr;
	_pContextDocument = nullptr;
	_pDocumentMgr = nullptr;

	_dwCookieContextKeyEventSink = TF_INVALID_COOKIE;
	_dwCookieTextLayoutSink = TF_INVALID_COOKIE;

	_ec = TF_INVALID_EDIT_COOKIE;
}

CCandidateList::~CCandidateList()
{
	_EndCandidateList();

	_pTextService.Release();

	DllRelease();
}

STDAPI CCandidateList::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_INVALIDARG;
	}

	*ppvObj = nullptr;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfContextKeyEventSink))
	{
		*ppvObj = static_cast<ITfContextKeyEventSink *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfTextLayoutSink))
	{
		*ppvObj = static_cast<ITfTextLayoutSink *>(this);
	}

	if (*ppvObj)
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
	if (--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CCandidateList::OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if (pfEaten == nullptr)
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
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		_pTextService->_ResetStatus();
		_pTextService->_ClearComposition();
	}

#endif
	return S_OK;
}

STDAPI CCandidateList::OnKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if (pfEaten == nullptr)
	{
		return E_INVALIDARG;
	}

	*pfEaten = TRUE;

	return S_OK;
}

STDAPI CCandidateList::OnTestKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if (pfEaten == nullptr)
	{
		return E_INVALIDARG;
	}

	*pfEaten = TRUE;

	return S_OK;
}

STDAPI CCandidateList::OnTestKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if (pfEaten == nullptr)
	{
		return E_INVALIDARG;
	}

	*pfEaten = TRUE;

	return S_OK;
}

STDAPI CCandidateList::OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pContextView)
{
	HRESULT hr;

	if (pContext != _pContextDocument)
	{
		return S_OK;
	}

	switch (lcode)
	{
	case TF_LC_CREATE:
		break;

	case TF_LC_CHANGE:
		if (_pCandidateWindow != nullptr)
		{
			try
			{
				CComPtr<ITfEditSession> pEditSession;
				pEditSession.Attach(
					new CCandidateListGetTextExtEditSession(_pTextService, pContext, pContextView, _pRangeComposition, _pCandidateWindow));
				pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
			}
			catch (...)
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

class CCandidateWindowEditSession : public CEditSessionBase
{
public:
	CCandidateWindowEditSession(CTextService *pTextService, ITfContext *pContext,
		ITfRange *pRange, CCandidateWindow *pCandidateWindow) : CEditSessionBase(pTextService, pContext)
	{
		_pCandidateWindow = pCandidateWindow;
		_pRangeComposition = pRange;
	}

	~CCandidateWindowEditSession()
	{
		_pCandidateWindow.Release();
		_pRangeComposition.Release();
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		HRESULT hr;

		CComPtr<ITfContextView> pContextView;
		if (SUCCEEDED(_pContext->GetActiveView(&pContextView)) && (pContextView != nullptr))
		{
			try
			{
				CComPtr<ITfEditSession> pEditSession;
				pEditSession.Attach(
					new CCandidateListGetTextExtEditSession(_pTextService, _pContext, pContextView, _pRangeComposition, _pCandidateWindow));
				_pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
			}
			catch (...)
			{
			}
		}

		_pCandidateWindow->_BeginUIElement();
		_pCandidateWindow->_Redraw();

		return S_OK;
	}

private:
	CComPtr<ITfRange> _pRangeComposition;
	CComPtr<CCandidateWindow> _pCandidateWindow;
};

HRESULT CCandidateList::_StartCandidateList(TfClientId tfClientId, ITfDocumentMgr *pDocumentMgr,
	ITfContext *pContext, TfEditCookie ec, ITfRange *pRange, int mode)
{
	HRESULT hrRet = E_FAIL;
	TfEditCookie ecTextStore;

	_EndCandidateList();

	if (FAILED(pDocumentMgr->CreateContext(tfClientId, 0, nullptr, &_pContextCandidateWindow, &ecTextStore)))
	{
		return E_FAIL;
	}

	if (FAILED(pDocumentMgr->Push(_pContextCandidateWindow)))
	{
		goto exit;
	}

	_pDocumentMgr = pDocumentMgr;
	_pContextDocument = pContext;
	_pRangeComposition = pRange;
	_ec = ec;

	if (FAILED(_AdviseContextKeyEventSink()))
	{
		goto exit;
	}

	if (FAILED(_AdviseTextLayoutSink()))
	{
		goto exit;
	}

	try
	{
		_pCandidateWindow.Attach(new CCandidateWindow(_pTextService, this));

		HWND hwnd = nullptr;
		CComPtr<ITfContextView> pContextView;
		if (SUCCEEDED(pContext->GetActiveView(&pContextView)) && (pContextView != nullptr))
		{
			if (!_pTextService->_UILessMode && _pCandidateWindow->_CanShowUIElement())
			{
				if (FAILED(pContextView->GetWnd(&hwnd)) || hwnd == nullptr)
				{
					hwnd = GetFocus();
				}
			}
		}

		if (!_pCandidateWindow->_Create(hwnd, nullptr, 0, 0, mode))
		{
			goto exit;
		}
	}
	catch (...)
	{
		goto exit;
	}

	try
	{
		HRESULT hr = E_FAIL;
		HRESULT hrSession = E_FAIL;

		CComPtr<ITfEditSession> pEditSession;
		pEditSession.Attach(
			new CCandidateWindowEditSession(_pTextService, _pContextDocument, _pRangeComposition, _pCandidateWindow));
		hr = pContext->RequestEditSession(tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READ, &hrSession);

		if (FAILED(hr) || FAILED(hrSession))
		{
			goto exit;
		}
	}
	catch (...)
	{
		goto exit;
	}

	hrRet = S_OK;

exit:
	if (FAILED(hrRet))
	{
		_EndCandidateList();
	}
	return hrRet;
}

void CCandidateList::_InvokeKeyHandler(WPARAM key)
{
	if (_pTextService != nullptr && _pContextDocument != nullptr)
	{
		_pTextService->_InvokeKeyHandler(_pContextDocument, (WPARAM)key, (LPARAM)0, 0);
	}
}

void CCandidateList::_InvokeSfHandler(BYTE sf)
{
	if (_pTextService != nullptr && _pContextDocument != nullptr)
	{
		_pTextService->_InvokeKeyHandler(_pContextDocument, (WPARAM)0, (LPARAM)0, sf);
	}
}

void CCandidateList::_EndCandidateList()
{
	_UnadviseTextLayoutSink();

	_UnadviseContextKeyEventSink();

	if (_pDocumentMgr != nullptr)
	{
		_pDocumentMgr->Pop(0);
	}

	_pContextCandidateWindow.Release();
	_pDocumentMgr.Release();

	if (_pCandidateWindow != nullptr)
	{
		_pCandidateWindow->_EndUIElement();
		_pCandidateWindow->_Destroy();
	}
	_pCandidateWindow.Release();

	_pRangeComposition.Release();
	_pContextDocument.Release();
}

BOOL CCandidateList::_IsShowCandidateWindow()
{
	return (_pCandidateWindow != nullptr) ? TRUE : FALSE;
}

BOOL CCandidateList::_IsContextCandidateWindow(ITfContext *pContext)
{
	return (_pContextCandidateWindow == pContext) ? TRUE : FALSE;
}

HRESULT CCandidateList::_AdviseContextKeyEventSink()
{
	HRESULT hr = E_FAIL;

	CComPtr<ITfSource> pSource;
	if (SUCCEEDED(_pContextCandidateWindow->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
	{
		hr = pSource->AdviseSink(IID_IUNK_ARGS(static_cast<ITfContextKeyEventSink *>(this)), &_dwCookieContextKeyEventSink);
	}

	return hr;
}

HRESULT CCandidateList::_UnadviseContextKeyEventSink()
{
	HRESULT hr = E_FAIL;

	if (_pContextCandidateWindow != nullptr)
	{
		CComPtr<ITfSource> pSource;
		if (SUCCEEDED(_pContextCandidateWindow->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
		{
			hr = pSource->UnadviseSink(_dwCookieContextKeyEventSink);
		}
	}

	return hr;
}

HRESULT CCandidateList::_AdviseTextLayoutSink()
{
	HRESULT hr = E_FAIL;

	CComPtr<ITfSource> pSource;
	if (SUCCEEDED(_pContextDocument->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
	{
		hr = pSource->AdviseSink(IID_IUNK_ARGS(static_cast<ITfTextLayoutSink *>(this)), &_dwCookieTextLayoutSink);
	}

	return hr;
}

HRESULT CCandidateList::_UnadviseTextLayoutSink()
{
	HRESULT hr = E_FAIL;

	if (_pContextDocument != nullptr)
	{
		CComPtr<ITfSource> pSource;
		if (SUCCEEDED(_pContextDocument->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
		{
			hr = pSource->UnadviseSink(_dwCookieTextLayoutSink);
		}
	}

	return hr;
}

void CCandidateList::_Show(BOOL bShow)
{
	if (_pCandidateWindow != nullptr)
	{
		_pCandidateWindow->Show(bShow);
	}
}

void CCandidateList::_Redraw()
{
	if (_pCandidateWindow != nullptr)
	{
		_pCandidateWindow->_Redraw();
	}
}

void CCandidateList::_SetText(const std::wstring &text, BOOL fixed, int mode)
{
	if (_pCandidateWindow != nullptr)
	{
		_pCandidateWindow->_SetText(text, fixed, mode);
	}
}

void CCandidateList::_Move(LPRECT lpr, TfEditCookie ec, ITfContext *pContext)
{
	if (_pCandidateWindow != nullptr)
	{
		_pCandidateWindow->_Move(lpr, ec, pContext);
	}
}

void CCandidateList::_UpdateComp()
{
	if (_pCandidateWindow != nullptr)
	{
		_pCandidateWindow->_UpdateComp();
	}
}
