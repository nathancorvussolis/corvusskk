
#ifndef ENUMDISPLAYATTRIBUTEINFO_H
#define ENUMDISPLAYATTRIBUTEINFO_H

class CEnumDisplayAttributeInfo : public IEnumTfDisplayAttributeInfo
{
public:
	CEnumDisplayAttributeInfo()
	{
		DllAddRef();

		_cRef = 1;
		_iIndex = 0;
	}
	~CEnumDisplayAttributeInfo()
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

		if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumTfDisplayAttributeInfo))
		{
			*ppvObj = (IEnumTfDisplayAttributeInfo *)this;
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

	// IEnumTfDisplayAttributeInfo
	STDMETHODIMP Clone(IEnumTfDisplayAttributeInfo **ppEnum)
	{
		CEnumDisplayAttributeInfo *pClone;

		if(ppEnum == NULL)
		{
			return E_INVALIDARG;
		}

		*ppEnum = NULL;

		pClone = new CEnumDisplayAttributeInfo();

		if(pClone == NULL)
		{
			return E_OUTOFMEMORY;
		}

		pClone->_iIndex = _iIndex;

		*ppEnum = pClone;

		return S_OK;
	}
	STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched)
	{
		ITfDisplayAttributeInfo *pDisplayAttributeInfo;
		ULONG cFetched = 0;

		if(rgInfo == NULL)
		{
			return E_INVALIDARG;
		}

		if(ulCount == 0)
		{
			return S_OK;
		}

		while(cFetched < ulCount)
		{
			if(_iIndex > 2)
			{
				break;
			}

			if(_iIndex == 0)
			{
				pDisplayAttributeInfo = new CDisplayAttributeInfoInput();
				if(pDisplayAttributeInfo == NULL)
				{
					return E_OUTOFMEMORY;
				}
			}
			else if(_iIndex == 1)
			{
				pDisplayAttributeInfo = new CDisplayAttributeInfoCandidate();
				if(pDisplayAttributeInfo == NULL)
				{
					return E_OUTOFMEMORY;
				}
			}
			else if(_iIndex == 2)
			{
				pDisplayAttributeInfo = new CDisplayAttributeInfoAnnotation();
				if(pDisplayAttributeInfo == NULL)
				{
					return E_OUTOFMEMORY;
				}
			}

			*(rgInfo + cFetched) = pDisplayAttributeInfo;
			cFetched++;
			_iIndex++;
		}

		if(pcFetched != NULL)
		{
			*pcFetched = cFetched;
		}

		return (cFetched == ulCount) ? S_OK : S_FALSE;
	}
	STDMETHODIMP Reset()
	{
		_iIndex = 0;
		return S_OK;
	}
	STDMETHODIMP Skip(ULONG ulCount)
	{
		if(ulCount > 0 && _iIndex == 0)
		{
			_iIndex++;
		}
		return S_OK;
	}

private:
	LONG _iIndex;
	LONG _cRef;
};

#endif //ENUMDISPLAYATTRIBUTEINFO_H
