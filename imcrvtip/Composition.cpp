
#include "imcrvtip.h"
#include "TextService.h"
#include "EditSession.h"
#include "InputModeWindow.h"

STDAPI CTextService::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition)
{
	_EndCandidateList();
	showcandlist = FALSE;

	if(pComposition != NULL)
	{
		ITfRange *pRange;
		if(pComposition->GetRange(&pRange) == S_OK)
		{
			pRange->SetText(ecWrite, 0, L"", 0);
			SafeRelease(&pRange);
		}
	}
	SafeRelease(&_pComposition);

	_EndInputModeWindow();

	_ResetStatus();

	return S_OK;
}

BOOL CTextService::_IsComposing()
{
	return _pComposition != NULL;
}

void CTextService::_SetComposition(ITfComposition *pComposition)
{
	_pComposition = pComposition;
}

BOOL CTextService::_IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover)
{
	LONG lResult;

	if(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult) != S_OK || lResult > 0)
	{
		return FALSE;
	}

	if(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult) != S_OK || lResult < 0)
	{
		return FALSE;
	}

	return TRUE;
}

class CStartCompositionEditSession : public CEditSessionBase
{
public:
	CStartCompositionEditSession(CTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
	{
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		HRESULT hr = E_FAIL;

		ITfInsertAtSelection *pInsertAtSelection;
		if(_pContext->QueryInterface(IID_PPV_ARGS(&pInsertAtSelection)) == S_OK)
		{
			ITfRange *pRange;
			if(pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRange) == S_OK)
			{
				ITfContextComposition *pContextComposition;
				if(_pContext->QueryInterface(IID_PPV_ARGS(&pContextComposition)) == S_OK)
				{
					ITfComposition *pComposition;
					if((pContextComposition->StartComposition(ec, pRange, _pTextService, &pComposition) == S_OK) && (pComposition != NULL))
					{
						_pTextService->_SetComposition(pComposition);

						TF_SELECTION tfSelection;
						tfSelection.range = pRange;
						tfSelection.style.ase = TF_AE_NONE;
						tfSelection.style.fInterimChar = FALSE;
						hr = _pContext->SetSelection(ec, 1, &tfSelection);
					}
					SafeRelease(&pContextComposition);
				}
				SafeRelease(&pRange);
			}
			SafeRelease(&pInsertAtSelection);
		}

		return hr;
	}
};

BOOL CTextService::_StartComposition(ITfContext *pContext)
{
	HRESULT hr = E_FAIL;

	try
	{
		CStartCompositionEditSession *pEditSession = new CStartCompositionEditSession(this, pContext);
		pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
		SafeRelease(&pEditSession);
	}
	catch(...)
	{
	}

	return (hr == S_OK);
}

class CEndCompositionEditSession : public CEditSessionBase
{
public:
	CEndCompositionEditSession(CTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
	{
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		_pTextService->_TerminateComposition(ec, _pContext);
		return S_OK;
	}
};

void CTextService::_TerminateComposition(TfEditCookie ec, ITfContext *pContext)
{
	if(pContext == NULL)	//辞書登録用
	{
		return;
	}

	if(_IsComposing())
	{
		_ClearCompositionDisplayAttributes(ec, pContext);
		_pComposition->EndComposition(ec);
	}
	SafeRelease(&_pComposition);
}

void CTextService::_EndComposition(ITfContext *pContext)
{
	HRESULT hr;

	try
	{
		CEndCompositionEditSession *pEditSession = new CEndCompositionEditSession(this, pContext);
		pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
		SafeRelease(&pEditSession);
	}
	catch(...)
	{
	}
}

class CClearCompositionEditSession : public CEditSessionBase
{
public:
	CClearCompositionEditSession(CTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
	{
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		_pTextService->_CancelComposition(ec, _pContext);
		return S_OK;
	}
};

void CTextService::_CancelComposition(TfEditCookie ec, ITfContext *pContext)
{
	TF_SELECTION tfSelection;
	ULONG cFetched = 0;

	if(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK)
	{
		return;
	}

	if(cFetched != 1)
	{
		SafeRelease(&tfSelection.range);
		return;
	}

	ITfRange *pRange;
	if(_IsComposing() && _pComposition->GetRange(&pRange) == S_OK)
	{
		if(_IsRangeCovered(ec, tfSelection.range, pRange))
		{
			pRange->SetText(ec, 0, L"", 0);

			tfSelection.range->ShiftEndToRange(ec, pRange, TF_ANCHOR_END);
			tfSelection.range->Collapse(ec, TF_ANCHOR_END);

			pContext->SetSelection(ec, 1, &tfSelection);
		}
		SafeRelease(&pRange);
	}

	SafeRelease(&tfSelection.range);

	_TerminateComposition(ec, pContext);
}

void CTextService::_ClearComposition()
{
	HRESULT hr;

	_EndCandidateList();
	showcandlist = FALSE;

	_EndInputModeWindow();

	if(_IsComposing())
	{
		ITfDocumentMgr *pDocumentMgr;
		if((_pThreadMgr->GetFocus(&pDocumentMgr) == S_OK) && (pDocumentMgr != NULL))
		{
			ITfContext *pContext;
			if((pDocumentMgr->GetTop(&pContext) == S_OK) && (pContext != NULL))
			{
				_ResetStatus();

				try
				{
					CClearCompositionEditSession *pEditSession = new CClearCompositionEditSession(this, pContext);
					pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
					SafeRelease(&pEditSession);
				}
				catch(...)
				{
				}

				SafeRelease(&pContext);
			}
			SafeRelease(&pDocumentMgr);
		}
	}
}
