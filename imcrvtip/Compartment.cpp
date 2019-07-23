
#include "imcrvtip.h"
#include "TextService.h"

HRESULT CTextService::_SetCompartment(REFGUID rguid, const VARIANT *pvar)
{
	HRESULT hr = E_FAIL;

	CComPtr<ITfCompartmentMgr> pCompartmentMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		CComPtr<ITfCompartment> pCompartment;
		if (SUCCEEDED(pCompartmentMgr->GetCompartment(rguid, &pCompartment)) && (pCompartment != nullptr))
		{
			hr = pCompartment->SetValue(_ClientId, pvar);
		}
	}

	return hr;
}

HRESULT CTextService::_GetCompartment(REFGUID rguid, VARIANT *pvar)
{
	HRESULT hr = E_FAIL;

	if (pvar == nullptr)
	{
		return hr;
	}

	CComPtr<ITfCompartmentMgr> pCompartmentMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		CComPtr<ITfCompartment> pCompartment;
		if (SUCCEEDED(pCompartmentMgr->GetCompartment(rguid, &pCompartment)) && (pCompartment != nullptr))
		{
			CComVariant var;
			if (SUCCEEDED(pCompartment->GetValue(&var)))
			{
				if (V_VT(&var) == VT_I4)
				{
					V_VT(pvar) = V_VT(&var);
					V_I4(pvar) = V_I4(&var);

					hr = S_OK;
				}
			}
		}
	}

	return hr;
}

BOOL CTextService::_IsKeyboardDisabled()
{
	BOOL fDisabled = FALSE;

	CComPtr<ITfDocumentMgr> pDocumentMgr;
	if (SUCCEEDED(_pThreadMgr->GetFocus(&pDocumentMgr)) && (pDocumentMgr != nullptr))
	{
		CComPtr<ITfContext> pContext;
		if (SUCCEEDED(pDocumentMgr->GetTop(&pContext)) && (pContext != nullptr))
		{
			CComPtr<ITfCompartmentMgr> pCompartmentMgr;
			if (SUCCEEDED(pContext->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
			{
				{
					CComPtr<ITfCompartment> pCompartment;
					if (SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_DISABLED, &pCompartment)) && (pCompartment != nullptr))
					{
						CComVariant var;
						if (SUCCEEDED(pCompartment->GetValue(&var)))
						{
							if (V_VT(&var) == VT_I4)
							{
								fDisabled = (V_I4(&var) == 0 ? FALSE : TRUE);
							}
						}
					}
				}
				{
					CComPtr<ITfCompartment> pCompartment;
					if (SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_EMPTYCONTEXT, &pCompartment)) && (pCompartment != nullptr))
					{
						CComVariant var;
						if (SUCCEEDED(pCompartment->GetValue(&var)))
						{
							if (V_VT(&var) == VT_I4)
							{
								fDisabled = (V_I4(&var) == 0 ? FALSE : TRUE);
							}
						}
					}
				}
			}
		}
		else
		{
			fDisabled = TRUE;
		}
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

	CComPtr<ITfCompartmentMgr> pCompartmentMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		CComPtr<ITfCompartment> pCompartment;
		if (SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment)) && (pCompartment != nullptr))
		{
			CComVariant var;
			if (SUCCEEDED(pCompartment->GetValue(&var)))
			{
				if (V_VT(&var) == VT_I4)
				{
					fOpen = (V_I4(&var) == 0 ? FALSE : TRUE);
				}
			}
		}
	}

	return fOpen;
}

HRESULT CTextService::_SetKeyboardOpen(BOOL fOpen)
{
	HRESULT hr = E_FAIL;

	CComPtr<ITfCompartmentMgr> pCompartmentMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		CComPtr<ITfCompartment> pCompartment;
		if (SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment)) && (pCompartment != nullptr))
		{
			CComVariant var;
			V_VT(&var) = VT_I4;
			V_I4(&var) = fOpen;
			hr = pCompartment->SetValue(_ClientId, &var);
		}
	}

	return hr;
}
