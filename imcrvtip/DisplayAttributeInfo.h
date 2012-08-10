
#ifndef DISPLAYATTRIBUTEINFO_H
#define DISPLAYATTRIBUTEINFO_H

class CDisplayAttributeInfoBase : public ITfDisplayAttributeInfo
{
public:
	CDisplayAttributeInfoBase()
	{
		DllAddRef();

		_pguid = NULL;
		_pDisplayAttribute = NULL;

		_cRef = 1;
	}
	~CDisplayAttributeInfoBase()
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

class CDisplayAttributeInfoInput : public CDisplayAttributeInfoBase
{
public:
	CDisplayAttributeInfoInput()
	{
		_pguid = &c_guidDisplayAttributeInput;
		_pDisplayAttribute = &c_daDisplayAttributeInput;
	}
};

class CDisplayAttributeInfoCandidate : public CDisplayAttributeInfoBase
{
public:
	CDisplayAttributeInfoCandidate()
	{
		_pguid = &c_guidDisplayAttributeCandidate;
		_pDisplayAttribute = &c_daDisplayAttributeCandidate;
	}
};

class CDisplayAttributeInfoAnnotation : public CDisplayAttributeInfoBase
{
public:
	CDisplayAttributeInfoAnnotation()
	{
		_pguid = &c_guidDisplayAttributeAnnotation;
		_pDisplayAttribute = &c_daDisplayAttributeAnnotation;
	}
};

#endif //DISPLAYATTRIBUTEINFO_H
