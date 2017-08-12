
#include "imcrvtip.h"
#include "TextService.h"

STDAPI CTextService::OnChange(REFGUID rguid)
{
	if(IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
	{
		_KeyboardOpenCloseChanged();
	}
	else if(IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
	{
		_KeyboardInputConversionChanged();
	}

	return S_OK;
}

BOOL CTextService::_InitCompartmentEventSink()
{
	ITfCompartmentMgr *pCompartmentMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		{
			ITfCompartment *pCompartment = nullptr;
			if(SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment)) && (pCompartment != nullptr))
			{
				ITfSource *pSource = nullptr;
				if(SUCCEEDED(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
				{
					if(FAILED(pSource->AdviseSink(IID_IUNK_ARGS((ITfCompartmentEventSink *)this), &_dwCompartmentEventSinkOpenCloseCookie)))
					{
						_dwCompartmentEventSinkOpenCloseCookie = TF_INVALID_COOKIE;
					}
					SafeRelease(&pSource);
				}
				SafeRelease(&pCompartment);
			}
		}
		{
			ITfCompartment *pCompartment = nullptr;
			if(SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &pCompartment)) && (pCompartment != nullptr))
			{
				ITfSource *pSource = nullptr;
				if(SUCCEEDED(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
				{
					if(FAILED(pSource->AdviseSink(IID_IUNK_ARGS((ITfCompartmentEventSink *)this), &_dwCompartmentEventSinkInputmodeConversionCookie)))
					{
						_dwCompartmentEventSinkInputmodeConversionCookie = TF_INVALID_COOKIE;
					}
					SafeRelease(&pSource);
				}
				SafeRelease(&pCompartment);
			}
		}
		SafeRelease(&pCompartmentMgr);
	}

	return (_dwCompartmentEventSinkOpenCloseCookie != TF_INVALID_COOKIE && _dwCompartmentEventSinkInputmodeConversionCookie != TF_INVALID_COOKIE);
}

void CTextService::_UninitCompartmentEventSink()
{
	ITfCompartmentMgr *pCompartmentMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr))) && (pCompartmentMgr != nullptr))
	{
		{
			ITfCompartment *pCompartment = nullptr;
			if(SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment)) && (pCompartment != nullptr))
			{
				ITfSource *pSource = nullptr;
				if(SUCCEEDED(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
				{
					pSource->UnadviseSink(_dwCompartmentEventSinkOpenCloseCookie);
					SafeRelease(&pSource);
				}
				SafeRelease(&pCompartment);
			}
		}
		{
			ITfCompartment *pCompartment = nullptr;
			if(SUCCEEDED(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &pCompartment)) && (pCompartment != nullptr))
			{
				ITfSource *pSource = nullptr;
				if(SUCCEEDED(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
				{
					pSource->UnadviseSink(_dwCompartmentEventSinkInputmodeConversionCookie);
					SafeRelease(&pSource);
				}
				SafeRelease(&pCompartment);
			}
		}
		SafeRelease(&pCompartmentMgr);
	}
}
