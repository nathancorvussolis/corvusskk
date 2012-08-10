
#include "corvustip.h"
#include "TextService.h"

STDAPI CTextService::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
	// clear when auto completion
	ITfRange *pRangeComposition;
	if(_IsComposing() && _pComposition->GetRange(&pRangeComposition) == S_OK)
	{
		BOOL fEmpty = FALSE;
		if(pRangeComposition->IsEmpty(ecReadOnly, &fEmpty) == S_OK && fEmpty)
		{
			if(!roman.empty() || !kana.empty())
			{
				_ResetStatus();
				_EndComposition(pic);
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
		if(_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
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

	if(_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
	{
		if(pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_dwTextEditSinkCookie) == S_OK)
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
