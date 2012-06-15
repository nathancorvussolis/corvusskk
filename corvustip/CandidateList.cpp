
#include "corvustip.h"
#include "TextService.h"
#include "EditSession.h"
#include "CandidateWindow.h"
#include "CandidateList.h"

class CGetTextExtEditSession : public CEditSessionBase
{
public:
	CGetTextExtEditSession(CTextService *pTextService, ITfContext *pContext, ITfContextView *pContextView, ITfRange *pRangeComposition, CCandidateWindow *pCandidateWindow) : CEditSessionBase(pTextService, pContext)
	{
		_pContextView = pContextView;
		_pRangeComposition = pRangeComposition;
		_pCandidateWindow = pCandidateWindow;
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		RECT rc;
		BOOL fClipped;

		if(_pContextView->GetTextExt(ec, _pRangeComposition, &rc, &fClipped) == S_OK)
		{
			_pCandidateWindow->_Move(rc.left, rc.bottom);
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
	_pTextService = pTextService;

	_hwndParent = NULL;
	_pCandidateWindow = NULL;
	_pRangeComposition = NULL;
	_pContextCandidateWindow = NULL;
	_pContextDocument = NULL;
	_pDocumentMgr = NULL;

	_dwCookieContextKeyEventSink = TF_INVALID_COOKIE;
	_dwCookieTextLayoutSink = TF_INVALID_COOKIE;

	_cRef = 1;

	DllAddRef();
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
	_pCandidateWindow->_OnKeyDown((UINT)wParam);

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
	CGetTextExtEditSession *pEditSession;
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
			pEditSession = new CGetTextExtEditSession(_pTextService, pContext, pContextView, _pRangeComposition, _pCandidateWindow);
			if(pEditSession != NULL)
			{
				pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
				pEditSession->Release();
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
	ITfContext *pContextDocument, TfEditCookie ec, ITfRange *pRangeComposition, BOOL reg)
{
	HRESULT hr = E_FAIL;
	TfEditCookie ecTextStore;
	ITfContextView *pContextView;
	BOOL fClipped;
	RECT rc;
	HWND hwnd;

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

	_pContextDocument = pContextDocument;
	_pContextDocument->AddRef();

	_pRangeComposition = pRangeComposition;
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

	_pCandidateWindow = new CCandidateWindow(_pTextService);
	if(_pCandidateWindow != NULL)
	{
		if(reg && _pCandidateWindow->_CanShowUI() == FALSE)
		{
			goto exit;
		}

		if(pContextDocument->GetActiveView(&pContextView) != S_OK)
		{
			goto exit;
		}

		// 多分コマンドプロンプト
		if(pContextView->GetWnd(&hwnd) != S_OK)
		{
			_pCandidateWindow->_BeginUIElement();
			pContextView->Release();
			hr = S_OK;
			goto exit;
		}

		if(pContextView->GetTextExt(ec, pRangeComposition, &rc, &fClipped) != S_OK)
		{
			goto exit;
		}

		pContextView->Release();

		if(!_pCandidateWindow->_Create(hwnd, reg))
		{
			goto exit;
		}

		_pCandidateWindow->_Move(rc.left, rc.bottom);
		_pCandidateWindow->_BeginUIElement();

		hr = S_OK;
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
	if(_pCandidateWindow)
	{
		_pCandidateWindow->_EndUIElement();
		_pCandidateWindow->_Destroy();
		_pCandidateWindow->Release();
		_pCandidateWindow = NULL;
	}

	if(_pRangeComposition)
	{
		_pRangeComposition->Release();
		_pRangeComposition = NULL;
	}

	if(_pContextCandidateWindow)
	{
		_UnadviseContextKeyEventSink();
		_pContextCandidateWindow->Release();
		_pContextCandidateWindow = NULL;
	}

	if(_pContextDocument)
	{
		_UnadviseTextLayoutSink();
		_pContextDocument->Release();
		_pContextDocument = NULL;
	}

	if(_pDocumentMgr)
	{
		_pDocumentMgr->Pop(0);
		_pDocumentMgr->Release();
		_pDocumentMgr = NULL;
	}
}

BOOL CCandidateList::_IsContextCandidateWindow(ITfContext *pContext)
{
	return (_pContextCandidateWindow == pContext) ? TRUE : FALSE;
}

HRESULT CCandidateList::_AdviseContextKeyEventSink()
{
	ITfSource *pSource;
	HRESULT hr = E_FAIL;

	if(_pContextCandidateWindow->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
	{
		hr = pSource->AdviseSink(IID_ITfContextKeyEventSink, (ITfContextKeyEventSink *)this, &_dwCookieContextKeyEventSink);
		pSource->Release();
	}

	return hr;
}

HRESULT CCandidateList::_UnadviseContextKeyEventSink()
{
	ITfSource *pSource;
	HRESULT hr = E_FAIL;

	if(_pContextCandidateWindow != NULL)
	{
		if(_pContextCandidateWindow->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
		{
			hr = pSource->UnadviseSink(_dwCookieContextKeyEventSink);
			pSource->Release();
		}
	}

	return hr;
}

HRESULT CCandidateList::_AdviseTextLayoutSink()
{
	ITfSource *pSource;
	HRESULT hr = E_FAIL;

	if(_pContextDocument->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
	{
		hr = pSource->AdviseSink(IID_ITfTextLayoutSink, (ITfTextLayoutSink *)this, &_dwCookieTextLayoutSink);
		pSource->Release();
	}

	return hr;
}

HRESULT CCandidateList::_UnadviseTextLayoutSink()
{
	ITfSource *pSource;
	HRESULT hr = E_FAIL;

	if(_pContextDocument != NULL)
	{
		if(_pContextDocument->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
		{
			hr = pSource->UnadviseSink(_dwCookieTextLayoutSink);
			pSource->Release();
		}
	}

	return hr;
}

void CCandidateList::_Show(BOOL bShow)
{
	if(_pCandidateWindow)
	{
		_pCandidateWindow->Show(bShow);
	}
}

void CCandidateList::_SetTextRegword(const std::wstring &text, BOOL fixed, BOOL showcandlist)
{
	if(_pCandidateWindow)
	{
		_pCandidateWindow->_SetTextRegword(text, fixed, showcandlist);
	}
}
