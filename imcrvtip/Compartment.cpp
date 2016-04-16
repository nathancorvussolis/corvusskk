
#include "imcrvtip.h"
#include "TextService.h"

HRESULT CTextService::_SetCompartment(REFGUID rguid, const VARIANT *pvar)
{
	HRESULT hr = E_FAIL;

	ITfCompartmentMgr *pCompartmentMgr;
	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		ITfCompartment *pCompartment;
		if(pCompartmentMgr->GetCompartment(rguid, &pCompartment) == S_OK)
		{
			hr = pCompartment->SetValue(_ClientId, pvar);
			SafeRelease(&pCompartment);
		}
		SafeRelease(&pCompartmentMgr);
	}

	return hr;
}

HRESULT CTextService::_GetCompartment(REFGUID rguid, VARIANT *pvar)
{
	HRESULT hr = E_FAIL;

	if(pvar == nullptr)
	{
		return hr;
	}

	ITfCompartmentMgr *pCompartmentMgr;
	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		ITfCompartment *pCompartment;
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
			SafeRelease(&pCompartment);
		}
		SafeRelease(&pCompartmentMgr);
	}

	return hr;
}

BOOL CTextService::_IsKeyboardDisabled()
{
	BOOL fDisabled = FALSE;

	ITfDocumentMgr *pDocumentMgr;
	if((_pThreadMgr->GetFocus(&pDocumentMgr) == S_OK) && (pDocumentMgr != nullptr))
	{
		ITfContext *pContext;
		if((pDocumentMgr->GetTop(&pContext) == S_OK) && (pContext != nullptr))
		{
			ITfCompartmentMgr *pCompartmentMgr;
			if(pContext->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
			{
				ITfCompartment *pCompartment;
				if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_DISABLED, &pCompartment) == S_OK)
				{
					VARIANT var;
					if(pCompartment->GetValue(&var) == S_OK)
					{
						if(var.vt == VT_I4)
						{
							fDisabled = (BOOL)var.lVal;
						}
					}
					SafeRelease(&pCompartment);
				}
				if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_EMPTYCONTEXT, &pCompartment) == S_OK)
				{
					VARIANT var;
					if(pCompartment->GetValue(&var) == S_OK)
					{
						if(var.vt == VT_I4)
						{
							fDisabled = (BOOL)var.lVal;
						}
					}
					SafeRelease(&pCompartment);
				}
				SafeRelease(&pCompartmentMgr);
			}
			SafeRelease(&pContext);
		}
		else
		{
			fDisabled = TRUE;
		}
		SafeRelease(&pDocumentMgr);
	}
	else
	{
		fDisabled = TRUE;
	}

	return fDisabled;
}

BOOL CTextService::_IsKeyboardOpen()
{
	BOOL fOpen = FALSE;

	ITfCompartmentMgr *pCompartmentMgr;
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
			SafeRelease(&pCompartment);
		}
		SafeRelease(&pCompartmentMgr);
	}

	return fOpen;
}

HRESULT CTextService::_SetKeyboardOpen(BOOL fOpen)
{
	HRESULT hr = E_FAIL;

	ITfCompartmentMgr *pCompartmentMgr;
	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		ITfCompartment *pCompartment;
		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
		{
			VARIANT var;
			var.vt = VT_I4;
			var.lVal = fOpen;
			hr = pCompartment->SetValue(_ClientId, &var);
			SafeRelease(&pCompartment);
		}
		SafeRelease(&pCompartmentMgr);
	}

	return hr;
}
