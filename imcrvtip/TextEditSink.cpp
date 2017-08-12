
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"

STDAPI CTextService::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
	if(_IsComposing() && pic != nullptr)
	{
		ITfRange *pRange = nullptr;
		if(SUCCEEDED(_pComposition->GetRange(&pRange)) && (pRange != nullptr))
		{
			// clear when auto completion
			BOOL fEmpty = FALSE;
			if(SUCCEEDED(pRange->IsEmpty(ecReadOnly, &fEmpty)) && fEmpty)
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
				ITfContextView *pContextView = nullptr;
				if(SUCCEEDED(pic->GetActiveView(&pContextView)) && (pContextView != nullptr))
				{
					RECT rc = {};
					BOOL fClipped;
					if(SUCCEEDED(pContextView->GetTextExt(ecReadOnly, pRange, &rc, &fClipped)))
					{
						_pCandidateList->_Move(&rc, ecReadOnly, pic);
					}

					SafeRelease(&pContextView);
				}
			}

			SafeRelease(&pRange);
		}
	}

	return S_OK;
}

BOOL CTextService::_InitTextEditSink(ITfDocumentMgr *pDocumentMgr)
{
	BOOL fRet = FALSE;

	if(_pTextEditSinkContext != nullptr && _dwTextEditSinkCookie != TF_INVALID_COOKIE)
	{
		ITfSource *pSource = nullptr;
		if(SUCCEEDED(_pTextEditSinkContext->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
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

	if(FAILED(pDocumentMgr->GetTop(&_pTextEditSinkContext)))
	{
		return FALSE;
	}

	if(_pTextEditSinkContext == nullptr)
	{
		return TRUE;
	}

	{
		ITfSource *pSource = nullptr;
		if(SUCCEEDED(_pTextEditSinkContext->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
		{
			if(SUCCEEDED(pSource->AdviseSink(IID_IUNK_ARGS((ITfTextEditSink *)this), &_dwTextEditSinkCookie)))
			{
				fRet = TRUE;
			}
			else
			{
				_dwTextEditSinkCookie = TF_INVALID_COOKIE;
			}
			SafeRelease(&pSource);
		}
	}

	if(fRet == FALSE)
	{
		SafeRelease(&_pTextEditSinkContext);
	}

	return fRet;
}
