
#include "imcrvtip.h"
#include "TextService.h"

HRESULT CTextService::_SetCompartment(REFGUID rguid, const VARIANT *pvar)
{
	HRESULT hr = E_FAIL;

	ITfCompartmentMgr *pCompartmentMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		ITfCompartment *pCompartment = nullptr;
		if(SUCCEEDED(pCompartmentMgr->GetCompartment(rguid, &pCompartment)) && (pCompartment != nullptr))
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

	ITfCompartmentMgr *pCompartmentMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		ITfCompartment *pCompartment = nullptr;
		if(SUCCEEDED(pCompartmentMgr->GetCompartment(rguid, &pCompartment)) && (pCompartment != nullptr))
		{
			VARIANT var;
			VariantInit(&var);
			if(SUCCEEDED(pCompartment->GetValue(&var)))
			{
				if(V_VT(&var) == VT_I4)
				{
					VariantCopy(pvar, &var);
					hr = S_OK;
				}
			}
			VariantClear(&var);
			SafeRelease(&pCompartment);
		}
		SafeRelease(&pCompartmentMgr);
	}

	return hr;
}

BOOL CTextService::_IsKeyboardDisabled()
{
	BOOL fDisabled = FALSE;

	ITfDocumentMgr *pDocumentMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->GetFocus(&pDocumentMgr)) && (pDocumentMgr != nullptr))
	{
		ITfContext *pContext = nullptr;
		if(SUCCEEDED(pDocumentMgr->GetTop(&pContext)) && (pContext != nullptr))
		{
			ITfCompartmentMgr *pCompartmentMgr = nullptr;
			if(SUCCEEDED(pContext->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
			{
				{
					ITfCompartment *pCompartment = nullptr;
					if(SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_DISABLED, &pCompartment)) && (pCompartment != nullptr))
					{
						VARIANT var;
						VariantInit(&var);
						if(SUCCEEDED(pCompartment->GetValue(&var)))
						{
							if(V_VT(&var) == VT_I4)
							{
								fDisabled = (V_I4(&var) == 0 ? FALSE : TRUE);
							}
						}
						VariantClear(&var);
						SafeRelease(&pCompartment);
					}
				}
				{
					ITfCompartment *pCompartment = nullptr;
					if(SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_EMPTYCONTEXT, &pCompartment)) && (pCompartment != nullptr))
					{
						VARIANT var;
						VariantInit(&var);
						if(SUCCEEDED(pCompartment->GetValue(&var)))
						{
							if(V_VT(&var) == VT_I4)
							{
								fDisabled = (V_I4(&var) == 0 ? FALSE : TRUE);
							}
						}
						VariantClear(&var);
						SafeRelease(&pCompartment);
					}
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

	ITfCompartmentMgr *pCompartmentMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		ITfCompartment *pCompartment = nullptr;
		if(SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment)) && (pCompartment != nullptr))
		{
			VARIANT var;
			VariantInit(&var);
			if(SUCCEEDED(pCompartment->GetValue(&var)))
			{
				if(V_VT(&var) == VT_I4)
				{
					fOpen = (V_I4(&var) == 0 ? FALSE : TRUE);
				}
			}
			VariantClear(&var);
			SafeRelease(&pCompartment);
		}
		SafeRelease(&pCompartmentMgr);
	}

	return fOpen;
}

HRESULT CTextService::_SetKeyboardOpen(BOOL fOpen)
{
	HRESULT hr = E_FAIL;

	ITfCompartmentMgr *pCompartmentMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		ITfCompartment *pCompartment = nullptr;
		if(SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment)) && (pCompartment != nullptr))
		{
			VARIANT var;
			VariantInit(&var);
			V_VT(&var) = VT_I4;
			V_I4(&var) = fOpen;
			hr = pCompartment->SetValue(_ClientId, &var);
			VariantClear(&var);
			SafeRelease(&pCompartment);
		}
		SafeRelease(&pCompartmentMgr);
	}

	return hr;
}
