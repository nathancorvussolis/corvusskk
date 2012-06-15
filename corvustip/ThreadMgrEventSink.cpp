
#include "corvustip.h"
#include "TextService.h"

STDAPI CTextService::OnInitDocumentMgr(ITfDocumentMgr *pdim)
{
	return S_OK;
}

STDAPI CTextService::OnUninitDocumentMgr(ITfDocumentMgr *pdim)
{
	return S_OK;
}

STDAPI CTextService::OnSetFocus(ITfDocumentMgr *pdim, ITfDocumentMgr *pdimPrevFocus)
{
	_InitTextEditSink(pdim);

	_UpdateLanguageBar();

	return S_OK;
}

STDAPI CTextService::OnPushContext(ITfContext *pic)
{
	return S_OK;
}

STDAPI CTextService::OnPopContext(ITfContext *pic)
{
	return S_OK;
}

BOOL CTextService::_InitThreadMgrEventSink()
{
	ITfSource *pSource;
	BOOL fRet = FALSE;

	if(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
	{
		if(pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink *)this, &_dwThreadMgrEventSinkCookie) == S_OK)
		{
			fRet = TRUE;
		}
		else
		{
			_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
		}
		pSource->Release();
	}

	return fRet;
}

void CTextService::_UninitThreadMgrEventSink()
{
	ITfSource *pSource;

	if(_dwThreadMgrEventSinkCookie != TF_INVALID_COOKIE)
	{
		if(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
		{
			pSource->UnadviseSink(_dwThreadMgrEventSinkCookie);
			pSource->Release();
		}
		_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
	}
}
