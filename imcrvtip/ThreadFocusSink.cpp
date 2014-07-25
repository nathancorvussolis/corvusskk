
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "InputModeWindow.h"

STDAPI CTextService::OnSetThreadFocus()
{
	if(_pCandidateList != NULL)
	{
		_pCandidateList->_Show(TRUE);
	}

	if(_pInputModeWindow != NULL)
	{
		_pInputModeWindow->_Show(TRUE);
	}

	return S_OK;
}

STDAPI CTextService::OnKillThreadFocus()
{
	_SaveUserDic();

	if(_pCandidateList != NULL)
	{
		_pCandidateList->_Show(FALSE);
	}

	if(_pInputModeWindow != NULL)
	{
		_pInputModeWindow->_Show(FALSE);
	}

	return S_OK;
}

BOOL CTextService::_InitThreadFocusSink()
{
	ITfSource *pSource;
	BOOL fRet = FALSE;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
	{
		if(pSource->AdviseSink(IID_IUNK_ARGS((ITfThreadFocusSink *)this), &_dwThreadFocusSinkCookie) == S_OK)
		{
			fRet = TRUE;
		}
		else
		{
			_dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
		}
		pSource->Release();
	}

	return fRet;
}

void CTextService::_UninitThreadFocusSink()
{
	ITfSource *pSource;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
	{
		pSource->UnadviseSink(_dwThreadFocusSinkCookie);
		pSource->Release();
	}
}
