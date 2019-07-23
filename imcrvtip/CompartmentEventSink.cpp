
#include "imcrvtip.h"
#include "TextService.h"

STDAPI CTextService::OnChange(REFGUID rguid)
{
	if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
	{
		_KeyboardOpenCloseChanged();
	}
	else if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
	{
		_KeyboardInputConversionChanged();
	}

	return S_OK;
}

BOOL CTextService::_InitCompartmentEventSink()
{
	CComPtr<ITfCompartmentMgr> pCompartmentMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		{
			CComPtr<ITfCompartment> pCompartment;
			if (SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment)) && (pCompartment != nullptr))
			{
				CComPtr<ITfSource> pSource;
				if (SUCCEEDED(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
				{
					if (FAILED(pSource->AdviseSink(IID_IUNK_ARGS(static_cast<ITfCompartmentEventSink *>(this)), &_dwCompartmentEventSinkOpenCloseCookie)))
					{
						_dwCompartmentEventSinkOpenCloseCookie = TF_INVALID_COOKIE;
					}
				}
			}
		}
		{
			CComPtr<ITfCompartment> pCompartment;
			if (SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &pCompartment)) && (pCompartment != nullptr))
			{
				CComPtr<ITfSource> pSource;
				if (SUCCEEDED(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
				{
					if (FAILED(pSource->AdviseSink(IID_IUNK_ARGS(static_cast<ITfCompartmentEventSink *>(this)), &_dwCompartmentEventSinkInputmodeConversionCookie)))
					{
						_dwCompartmentEventSinkInputmodeConversionCookie = TF_INVALID_COOKIE;
					}
				}
			}
		}
	}

	return (_dwCompartmentEventSinkOpenCloseCookie != TF_INVALID_COOKIE && _dwCompartmentEventSinkInputmodeConversionCookie != TF_INVALID_COOKIE);
}

void CTextService::_UninitCompartmentEventSink()
{
	CComPtr<ITfCompartmentMgr> pCompartmentMgr;
	if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		{
			CComPtr<ITfCompartment> pCompartment;
			if (SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment)) && (pCompartment != nullptr))
			{
				CComPtr<ITfSource> pSource;
				if (SUCCEEDED(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
				{
					pSource->UnadviseSink(_dwCompartmentEventSinkOpenCloseCookie);
				}
			}
		}
		{
			CComPtr<ITfCompartment> pCompartment;
			if (SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &pCompartment)) && (pCompartment != nullptr))
			{
				CComPtr<ITfSource> pSource;
				if (SUCCEEDED(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
				{
					pSource->UnadviseSink(_dwCompartmentEventSinkInputmodeConversionCookie);
				}
			}
		}
	}
}
