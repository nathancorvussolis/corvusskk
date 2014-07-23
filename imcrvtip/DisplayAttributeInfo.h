
#ifndef DISPLAYATTRIBUTEINFO_H
#define DISPLAYATTRIBUTEINFO_H

class CDisplayAttributeInfo : public ITfDisplayAttributeInfo
{
public:
	CDisplayAttributeInfo(const GUID &guid, const TF_DISPLAYATTRIBUTE *pDisplayAttribute)
	{
		DllAddRef();

		_cRef = 1;

		_pguid = &guid;
		_pDisplayAttribute = pDisplayAttribute;
	}
	~CDisplayAttributeInfo()
	{
		DllRelease();
	}

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj)
	{
		if(ppvObj == NULL)
		{
			return E_INVALIDARG;
		}

		*ppvObj = NULL;

		if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfDisplayAttributeInfo))
		{
			*ppvObj = (ITfDisplayAttributeInfo *)this;
		}

		if(*ppvObj)
		{
			AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG) AddRef(void)
	{
		return ++_cRef;
	}
	STDMETHODIMP_(ULONG) Release(void)
	{
		if(--_cRef == 0)
		{
			delete this;
			return 0;
		}

		return _cRef;
	}

	// ITfDisplayAttributeInfo
	STDMETHODIMP GetGUID(GUID *pguid)
	{
		if(pguid == NULL)
		{
			return E_INVALIDARG;
		}

		if(_pguid == NULL)
		{
			return E_FAIL;
		}

		*pguid = *_pguid;

		return S_OK;
	}
	STDMETHODIMP GetDescription(BSTR *pbstrDesc)
	{
		BSTR bstrDesc;

		if(pbstrDesc == NULL)
		{
			return E_INVALIDARG;
		}

		*pbstrDesc = NULL;

		bstrDesc = SysAllocString(TextServiceDesc);

		if(bstrDesc == NULL)
		{
			return E_OUTOFMEMORY;
		}

		*pbstrDesc = bstrDesc;

		return S_OK;
	}
	STDMETHODIMP GetAttributeInfo(TF_DISPLAYATTRIBUTE *pda)
	{
		if(pda == NULL)
		{
			return E_INVALIDARG;
		}

		*pda = *_pDisplayAttribute;

		return S_OK;
	}
	STDMETHODIMP SetAttributeInfo(const TF_DISPLAYATTRIBUTE *pda)
	{
		return S_OK;
	}
	STDMETHODIMP Reset()
	{
		return SetAttributeInfo(_pDisplayAttribute);
	}

protected:
	const GUID *_pguid;
	const TF_DISPLAYATTRIBUTE *_pDisplayAttribute;

private:
	LONG _cRef;
};

#endif //DISPLAYATTRIBUTEINFO_H
