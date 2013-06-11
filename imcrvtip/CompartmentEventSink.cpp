
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "LanguageBar.h"

STDAPI CTextService::OnChange(REFGUID rguid)
{
	if(IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
	{
		_KeyboardChanged();
	}

	return S_OK;
}

BOOL CTextService::_InitCompartmentEventSink()
{
	ITfCompartmentMgr *pCompartmentMgr;
	ITfCompartment *pCompartment;
	ITfSource *pSource;
	HRESULT hr = E_FAIL;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
		{
			if(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
			{
				hr = pSource->AdviseSink(IID_IUNK_ARGS((ITfCompartmentEventSink *)this), &_dwCompartmentEventSinkCookie);
				pSource->Release();
			}
			pCompartment->Release();

			if(hr != S_OK)
			{
				_dwCompartmentEventSinkCookie = TF_INVALID_COOKIE;
			}
		}
		pCompartmentMgr->Release();
	}

	return (hr == S_OK);
}

void CTextService::_UninitCompartmentEventSink()
{
	ITfCompartmentMgr *pCompartmentMgr;
	ITfCompartment *pCompartment;
	ITfSource *pSource;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)) == S_OK)
	{
		if(pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
		{
			if(pCompartment->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
			{
				pSource->UnadviseSink(_dwCompartmentEventSinkCookie);
			}
			pCompartment->Release();
		}
		pCompartmentMgr->Release();
	}
}
