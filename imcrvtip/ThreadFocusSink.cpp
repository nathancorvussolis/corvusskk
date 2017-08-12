
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "InputModeWindow.h"

STDAPI CTextService::OnSetThreadFocus()
{
	if(_pCandidateList != nullptr)
	{
		_pCandidateList->_Show(TRUE);
	}

	if(_pInputModeWindow != nullptr)
	{
		_pInputModeWindow->_Show(TRUE);
	}

	return S_OK;
}

STDAPI CTextService::OnKillThreadFocus()
{
	_SaveUserDic();

	if(_pCandidateList != nullptr)
	{
		_pCandidateList->_Show(FALSE);
	}

	if(_pInputModeWindow != nullptr)
	{
		_pInputModeWindow->_Show(FALSE);
	}

	return S_OK;
}

BOOL CTextService::_InitThreadFocusSink()
{
	BOOL fRet = FALSE;

	ITfSource *pSource = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
	{
		if(SUCCEEDED(pSource->AdviseSink(IID_IUNK_ARGS((ITfThreadFocusSink *)this), &_dwThreadFocusSinkCookie)))
		{
			fRet = TRUE;
		}
		else
		{
			_dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
		}
		SafeRelease(&pSource);
	}

	return fRet;
}

void CTextService::_UninitThreadFocusSink()
{
	ITfSource *pSource = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
	{
		pSource->UnadviseSink(_dwThreadFocusSinkCookie);
		SafeRelease(&pSource);
	}
}
