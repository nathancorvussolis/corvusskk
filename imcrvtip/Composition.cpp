
#include "imcrvtip.h"
#include "TextService.h"
#include "EditSession.h"
#include "CandidateList.h"
#include "InputModeWindow.h"

STDAPI CTextService::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition)
{
	if(_pCandidateList != NULL)
	{
		_pCandidateList->_EndCandidateList();
		_pCandidateList = NULL;
	}

	if(_pInputModeWindow != NULL)
	{
		_pInputModeWindow->_Destroy();
		delete _pInputModeWindow;
		_pInputModeWindow = NULL;
	}

	if(_pComposition != NULL)
	{
		ITfRange* pRangeComposition;
		if(_pComposition->GetRange(&pRangeComposition) == S_OK)
		{
			pRangeComposition->SetText(ecWrite, 0, L"", 0);
			pRangeComposition->Release();
		}
		_pComposition->EndComposition(ecWrite);
		_pComposition->Release();
		_pComposition = NULL;
	}

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
	STDMETHODIMP DoEditSession(TfEditCookie ec);
};

STDAPI CStartCompositionEditSession::DoEditSession(TfEditCookie ec)
{
	ITfInsertAtSelection *pInsertAtSelection;
	ITfRange *pRangeInsert;
	ITfContextComposition *pContextComposition;
	ITfComposition *pComposition = NULL;
	HRESULT hr = E_FAIL;

	if(_pContext->QueryInterface(IID_PPV_ARGS(&pInsertAtSelection)) == S_OK)
	{
		if(pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert) == S_OK)
		{
			if(_pContext->QueryInterface(IID_PPV_ARGS(&pContextComposition)) == S_OK)
			{
				if((pContextComposition->StartComposition(ec, pRangeInsert, _pTextService, &pComposition) == S_OK) && (pComposition != NULL))
				{
					_pTextService->_SetComposition(pComposition);

					TF_SELECTION tfSelection;
					tfSelection.range = pRangeInsert;
					tfSelection.style.ase = TF_AE_NONE;
					tfSelection.style.fInterimChar = FALSE;
					hr = _pContext->SetSelection(ec, 1, &tfSelection);
				}
				pContextComposition->Release();
			}
			pRangeInsert->Release();
		}
		pInsertAtSelection->Release();
	}

	return hr;
}

void CTextService::_StartComposition(ITfContext *pContext)
{
	CStartCompositionEditSession *pStartCompositionEditSession;
	HRESULT hr;

	pStartCompositionEditSession = new CStartCompositionEditSession(this, pContext);
	if(pStartCompositionEditSession != NULL)
	{
		pContext->RequestEditSession(_ClientId, pStartCompositionEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
		pStartCompositionEditSession->Release();
	}
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

	if(_pComposition != NULL)
	{
		_ClearCompositionDisplayAttributes(ec, pContext);

		_pComposition->EndComposition(ec);
		_pComposition->Release();
		_pComposition = NULL;
	}
}

void CTextService::_EndComposition(ITfContext *pContext)
{
	CEndCompositionEditSession *pEditSession;
	HRESULT hr;

	pEditSession = new CEndCompositionEditSession(this, pContext);
	if(pEditSession != NULL)
	{
		pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
		pEditSession->Release();
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
	ITfRange *pRangeComposition;
	ULONG cFetched;

	if(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
	{
		return;
	}

	if(_pComposition->GetRange(&pRangeComposition) == S_OK)
	{
		if(_IsRangeCovered(ec, tfSelection.range, pRangeComposition))
		{
			pRangeComposition->SetText(ec, 0, L"", 0);
			
			tfSelection.range->ShiftEndToRange(ec, pRangeComposition, TF_ANCHOR_END);
			tfSelection.range->Collapse(ec, TF_ANCHOR_END);

			pContext->SetSelection(ec, 1, &tfSelection);
		}
		pRangeComposition->Release();
	}

	tfSelection.range->Release();

	_TerminateComposition(ec, pContext);
}

void CTextService::_ClearComposition()
{
	ITfDocumentMgr *pDocumentMgrFocus = NULL;
	ITfContext *pContext = NULL;
	CClearCompositionEditSession *pEditSession;
	HRESULT hr;

	if(_pCandidateList != NULL)
	{
		_pCandidateList->_EndCandidateList();
		_pCandidateList = NULL;
	}

	if(_pInputModeWindow != NULL)
	{
		_pInputModeWindow->_Destroy();
		delete _pInputModeWindow;
		_pInputModeWindow = NULL;
	}

	if(_pComposition != NULL)
	{
		if((_pThreadMgr->GetFocus(&pDocumentMgrFocus) == S_OK) && (pDocumentMgrFocus != NULL))
		{
			if((pDocumentMgrFocus->GetTop(&pContext) == S_OK) && (pContext != NULL))
			{
				_ResetStatus();

				pEditSession = new CClearCompositionEditSession(this, pContext);
				if(pEditSession != NULL)
				{
					pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
					pEditSession->Release();
				}
				pContext->Release();
			}
			pDocumentMgrFocus->Release();
		}
	}
}
