
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
	ITfCompartmentMgr *pCompartmentMgr;
	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		ITfCompartment *pCompartment;
		ITfSource *pSource;
		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
		{
			if(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
			{
				if(pSource->AdviseSink(IID_IUNK_ARGS((ITfCompartmentEventSink *)this),
					&_dwCompartmentEventSinkOpenCloseCookie) != S_OK)
				{
					_dwCompartmentEventSinkOpenCloseCookie = TF_INVALID_COOKIE;
				}
				SafeRelease(&pSource);
			}
			SafeRelease(&pCompartment);
		}
		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &pCompartment) == S_OK)
		{
			if(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
			{
				if(pSource->AdviseSink(IID_IUNK_ARGS((ITfCompartmentEventSink *)this),
					&_dwCompartmentEventSinkInputmodeConversionCookie) != S_OK)
				{
					_dwCompartmentEventSinkInputmodeConversionCookie = TF_INVALID_COOKIE;
				}
				SafeRelease(&pSource);
			}
			SafeRelease(&pCompartment);
		}
		SafeRelease(&pCompartmentMgr);
	}

	return (_dwCompartmentEventSinkOpenCloseCookie != TF_INVALID_COOKIE &&
		_dwCompartmentEventSinkInputmodeConversionCookie != TF_INVALID_COOKIE);
}

void CTextService::_UninitCompartmentEventSink()
{
	ITfCompartmentMgr *pCompartmentMgr;
	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		ITfCompartment *pCompartment;
		ITfSource *pSource;
		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
		{
			if(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
			{
				pSource->UnadviseSink(_dwCompartmentEventSinkOpenCloseCookie);
				SafeRelease(&pSource);
			}
			SafeRelease(&pCompartment);
		}
		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &pCompartment) == S_OK)
		{
			if(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
			{
				pSource->UnadviseSink(_dwCompartmentEventSinkInputmodeConversionCookie);
				SafeRelease(&pSource);
			}
			SafeRelease(&pCompartment);
		}
		SafeRelease(&pCompartmentMgr);
	}
}
