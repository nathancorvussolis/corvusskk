
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"

STDAPI CTextService::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
	ITfRange *pRangeComposition;
	if(_IsComposing() && _pComposition->GetRange(&pRangeComposition) == S_OK)
	{
		// clear when auto completion
		BOOL fEmpty = FALSE;
		if(pRangeComposition->IsEmpty(ecReadOnly, &fEmpty) == S_OK && fEmpty)
		{
			if(!roman.empty() || !kana.empty())
			{
				_ResetStatus();
				_EndComposition(pic);
			}
		}

		// reposition candidate window
		if(_pCandidateList != NULL)
		{
			ITfContextView *pContextView;
			if(pic->GetActiveView(&pContextView) == S_OK)
			{
				RECT rc;
				BOOL fClipped;
				if(pContextView->GetTextExt(ecReadOnly, pRangeComposition, &rc, &fClipped) == S_OK)
				{
					_pCandidateList->_Move(rc.left, rc.bottom);
				}
				pContextView->Release();
			}
		}

		pRangeComposition->Release();
	}

	return S_OK;
}

BOOL CTextService::_InitTextEditSink(ITfDocumentMgr *pDocumentMgr)
{
	ITfSource *pSource;
	BOOL fRet;

	if(_dwTextEditSinkCookie != TF_INVALID_COOKIE)
	{
		if(_pTextEditSinkContext->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
		{
			pSource->UnadviseSink(_dwTextEditSinkCookie);
			pSource->Release();
		}

		_pTextEditSinkContext->Release();
		_pTextEditSinkContext = NULL;
		_dwTextEditSinkCookie = TF_INVALID_COOKIE;
	}

	if(pDocumentMgr == NULL)
	{
		return TRUE;
	}

	if(pDocumentMgr->GetTop(&_pTextEditSinkContext) != S_OK)
	{
		return FALSE;
	}

	if(_pTextEditSinkContext == NULL)
	{
		return TRUE;
	}

	fRet = FALSE;

	if(_pTextEditSinkContext->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
	{
		if(pSource->AdviseSink(IID_IUNK_ARGS((ITfTextEditSink *)this), &_dwTextEditSinkCookie) == S_OK)
		{
			fRet = TRUE;
		}
		else
		{
			_dwTextEditSinkCookie = TF_INVALID_COOKIE;
		}
		pSource->Release();
	}

	if(fRet == FALSE)
	{
		_pTextEditSinkContext->Release();
		_pTextEditSinkContext = NULL;
	}

	return fRet;
}
