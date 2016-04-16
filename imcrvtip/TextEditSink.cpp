
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"

STDAPI CTextService::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
	ITfRange *pRange;
	if(_IsComposing() && _pComposition->GetRange(&pRange) == S_OK)
	{
		// clear when auto completion
		BOOL fEmpty = FALSE;
		if(pRange->IsEmpty(ecReadOnly, &fEmpty) == S_OK && fEmpty)
		{
			if(!roman.empty() || !kana.empty())
			{
				_ResetStatus();
				_EndComposition(pic);
			}
		}

		// reposition candidate window
		if(_pCandidateList != nullptr)
		{
			ITfContextView *pContextView;
			if(pic->GetActiveView(&pContextView) == S_OK)
			{
				RECT rc;
				BOOL fClipped;
				if(pContextView->GetTextExt(ecReadOnly, pRange, &rc, &fClipped) == S_OK)
				{
					_pCandidateList->_Move(&rc, ecReadOnly, pic);
				}

				SafeRelease(&pContextView);
			}
		}

		SafeRelease(&pRange);
	}

	return S_OK;
}

BOOL CTextService::_InitTextEditSink(ITfDocumentMgr *pDocumentMgr)
{
	BOOL fRet;
	ITfSource *pSource;

	if(_pTextEditSinkContext != nullptr && _dwTextEditSinkCookie != TF_INVALID_COOKIE)
	{
		if(_pTextEditSinkContext->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
		{
			pSource->UnadviseSink(_dwTextEditSinkCookie);
			SafeRelease(&pSource);
		}

		SafeRelease(&_pTextEditSinkContext);
		_dwTextEditSinkCookie = TF_INVALID_COOKIE;
	}

	if(pDocumentMgr == nullptr)
	{
		return TRUE;
	}

	if(pDocumentMgr->GetTop(&_pTextEditSinkContext) != S_OK)
	{
		return FALSE;
	}

	if(_pTextEditSinkContext == nullptr)
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
		SafeRelease(&pSource);
	}

	if(fRet == FALSE)
	{
		SafeRelease(&_pTextEditSinkContext);
	}

	return fRet;
}
