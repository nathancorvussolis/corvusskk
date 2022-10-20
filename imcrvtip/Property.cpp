
#include <initguid.h>
#include <tsattrs.h>
#include <InputScope.h>

#include "imcrvtip.h"
#include "TextService.h"
#include "EditSession.h"

BOOL CTextService::_IsVertical(TfEditCookie ec, ITfContext *pContext)
{
	BOOL ret = FALSE;

	if (pContext == nullptr)
	{
		return FALSE;
	}

	if (_IsComposing())
	{
		CComPtr<ITfReadOnlyProperty> pReadOnlyProperty;
		if (SUCCEEDED(pContext->GetAppProperty(TSATTRID_Text_VerticalWriting, &pReadOnlyProperty)) && (pReadOnlyProperty != nullptr))
		{
			CComPtr<ITfRange> pRange;
			if (SUCCEEDED(_pComposition->GetRange(&pRange)) && (pRange != nullptr))
			{
				CComVariant var;
				if (SUCCEEDED(pReadOnlyProperty->GetValue(ec, pRange, &var)))
				{
					if (V_VT(&var) == VT_BOOL)
					{
						ret = V_BOOL(&var);
					}
				}
			}
		}
	}

	return ret;
}

BOOL CTextService::_IsAppPrivateScope(TfEditCookie ec, ITfContext *pContext)
{
	BOOL ret = FALSE;

	if (pContext == nullptr)
	{
		return FALSE;
	}

	TF_SELECTION tfSelection = {};
	ULONG cFetched = 0;
	if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched)))
	{
		return FALSE;
	}

	CComPtr<ITfRange> pRangeSelection;
	pRangeSelection.Attach(tfSelection.range);

	if (cFetched != 1)
	{
		return FALSE;
	}

	CComPtr<ITfReadOnlyProperty> pReadOnlyProperty;
	if (SUCCEEDED(pContext->GetAppProperty(GUID_PROP_INPUTSCOPE, &pReadOnlyProperty)) && (pReadOnlyProperty != nullptr))
	{
		CComVariant var;
		if (SUCCEEDED(pReadOnlyProperty->GetValue(ec, pRangeSelection, &var)) && (var.punkVal != nullptr))
		{
			CComPtr<ITfInputScope> pInputScope;
			if (SUCCEEDED(var.punkVal->QueryInterface(IID_PPV_ARGS(&pInputScope))) && (pInputScope != nullptr))
			{
				InputScope *pInputScopes = nullptr;
				UINT cCount = 0;
				if (SUCCEEDED(pInputScope->GetInputScopes(&pInputScopes, &cCount)) && (pInputScopes != nullptr))
				{
					for (UINT i = 0; i < cCount; i++)
					{
						if (pInputScopes[i] == InputScope::IS_PRIVATE)
						{
							ret = TRUE;
							break;
						}
					}
					CoTaskMemFree(pInputScopes);
				}
			}
		}
	}

	return ret;
}

class CGetAppPrivateModeEditSession : public CEditSessionBase
{
public:
	CGetAppPrivateModeEditSession(CTextService *pTextService, ITfContext *pContext, BOOL *pIsPrivate) : CEditSessionBase(pTextService, pContext)
	{
		_pIsPrivate = pIsPrivate;
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		if (_pIsPrivate != nullptr)
		{
			*_pIsPrivate = _pTextService->_IsAppPrivateScope(ec, _pContext);
		}
		return S_OK;
	}

private:
	BOOL *_pIsPrivate;
};

void CTextService::_GetAppPrivateMode()
{
	BOOL isPrivate = FALSE;

	CComPtr<ITfDocumentMgr> pDocumentMgr;
	if (SUCCEEDED(_pThreadMgr->GetFocus(&pDocumentMgr)) && (pDocumentMgr != nullptr))
	{
		CComPtr<ITfContext> pContext;
		if (SUCCEEDED(pDocumentMgr->GetTop(&pContext)) && (pContext != nullptr))
		{
			try
			{
				HRESULT hr = E_FAIL;

				CComPtr<ITfEditSession> pEditSession;
				pEditSession.Attach(
					new CGetAppPrivateModeEditSession(this, pContext, &isPrivate));
				pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
			}
			catch (...)
			{
			}
		}
	}

	_AppPrivateMode = isPrivate;
}

BOOL CTextService::_IsPrivateMode()
{
	switch (_UserPrivateMode)
	{
	case S_OK:
		//ON
		return TRUE;
	case E_FAIL:
		//未定義 自動切替とIS_PRIVATEに従ってON/OFF
		if (cx_privatemodeauto && _AppPrivateMode)
		{
			return TRUE;
		}
		break;
	case S_FALSE:
		//OFF
		break;
	default:
		break;
	}
	return FALSE;
}

void CTextService::_TogglePrivateMode()
{
	if (cx_privatemodeauto && _AppPrivateMode)
	{
		switch (_UserPrivateMode)
		{
		case S_OK:
		case E_FAIL:
			//OFF
			_UserPrivateMode = S_FALSE;
			break;
		case S_FALSE:
			//未定義 自動切替とIS_PRIVATEに従ってON
			_UserPrivateMode = E_FAIL;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (_UserPrivateMode)
		{
		case S_OK:
			//未定義 自動切替とIS_PRIVATEに従ってOFF
			_UserPrivateMode = E_FAIL;
			break;
		case E_FAIL:
		case S_FALSE:
			//ON
			_UserPrivateMode = S_OK;
			break;
		default:
			break;
		}
	}
}
