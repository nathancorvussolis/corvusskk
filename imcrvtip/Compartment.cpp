
#include "imcrvtip.h"
#include "TextService.h"

HRESULT CTextService::_SetCompartment(REFGUID rguid, const VARIANT *pvar)
{
	ITfCompartmentMgr *pCompartmentMgr;
	ITfCompartment *pCompartment;
	HRESULT hr = E_FAIL;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		if(pCompartmentMgr->GetCompartment(rguid, &pCompartment) == S_OK)
		{
			hr = pCompartment->SetValue(_ClientId, pvar);
		}
		pCompartmentMgr->Release();
	}

	return hr;
}

HRESULT CTextService::_GetCompartment(REFGUID rguid, VARIANT *pvar)
{
	ITfCompartmentMgr *pCompartmentMgr;
	ITfCompartment *pCompartment;
	HRESULT hr = E_FAIL;

	if(pvar == NULL)
	{
		return hr;
	}

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		if(pCompartmentMgr->GetCompartment(rguid, &pCompartment) == S_OK)
		{
			VARIANT var;
			if(pCompartment->GetValue(&var) == S_OK)
			{
				if(var.vt == VT_I4)
				{
					*pvar = var;
					hr = S_OK;
				}
			}
		}
		pCompartmentMgr->Release();
	}

	return hr;
}

BOOL CTextService::_IsKeyboardDisabled()
{
	ITfDocumentMgr *pDocumentMgrFocus = NULL;
	ITfContext *pContext = NULL;
	ITfCompartmentMgr *pCompartmentMgr = NULL;
	BOOL fDisabled = FALSE;

	if((_pThreadMgr->GetFocus(&pDocumentMgrFocus) != S_OK) || (pDocumentMgrFocus == NULL))
	{
		fDisabled = TRUE;
		goto exit;
	}

	if((pDocumentMgrFocus->GetTop(&pContext) != S_OK) || (pContext == NULL))
	{
		fDisabled = TRUE;
		goto exit;
	}
	
	if(pContext->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		ITfCompartment *pCompartmentDisabled;
		ITfCompartment *pCompartmentEmptyContext;

		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_DISABLED, &pCompartmentDisabled) == S_OK)
		{
			VARIANT var;
			if(pCompartmentDisabled->GetValue(&var) == S_OK)
			{
				if(var.vt == VT_I4)
				{
					fDisabled = (BOOL)var.lVal;
				}
			}
			pCompartmentDisabled->Release();
		}

		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_EMPTYCONTEXT, &pCompartmentEmptyContext) == S_OK)
		{
			VARIANT var;
			if(pCompartmentEmptyContext->GetValue(&var) == S_OK)
			{
				if(var.vt == VT_I4)
				{
					fDisabled = (BOOL)var.lVal;
				}
			}
			pCompartmentEmptyContext->Release();
		}

		pCompartmentMgr->Release();
	}

exit:
	if(pContext)
	{
		pContext->Release();
	}

	if(pDocumentMgrFocus)
	{
		pDocumentMgrFocus->Release();
	}

	return fDisabled;
}

BOOL CTextService::_IsKeyboardOpen()
{
	ITfCompartmentMgr *pCompartmentMgr;
	BOOL fOpen = FALSE;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		ITfCompartment *pCompartment;
		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
		{
			VARIANT var;
			if(S_OK == pCompartment->GetValue(&var))
			{
				if(var.vt == VT_I4)
				{
					fOpen = (BOOL)var.lVal;
				}
			}
		}
		pCompartmentMgr->Release();
	}
	
	return fOpen;
}

HRESULT CTextService::_SetKeyboardOpen(BOOL fOpen)
{
	ITfCompartmentMgr *pCompartmentMgr;
	ITfCompartment *pCompartment;
	HRESULT hr = E_FAIL;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
		{
			VARIANT var;
			var.vt = VT_I4;
			var.lVal = fOpen;
			hr = pCompartment->SetValue(_ClientId, &var);
		}
		pCompartmentMgr->Release();
	}
	
	return hr;
}
