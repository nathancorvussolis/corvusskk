
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

		try
		{
			pClone = new CEnumDisplayAttributeInfo();
		}
		catch(...)
		{
			return E_OUTOFMEMORY;
		}

		pClone->_iIndex = _iIndex;

		*ppEnum = pClone;

		return S_OK;
	}
	STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched)
	{
		ULONG cFetched = 0;
		ITfDisplayAttributeInfo *pDisplayAttributeInfo;

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
			if(_iIndex >= DISPLAYATTRIBUTE_INFO_NUM)
			{
				break;
			}

			try
			{
				pDisplayAttributeInfo = new CDisplayAttributeInfo(
					c_gdDisplayAttributeInfo[_iIndex].guid, &CTextService::display_attribute_info[_iIndex]);
			}
			catch(...)
			{
				for(ULONG i = 0; i < cFetched; i++)
				{
					delete *(rgInfo + i);
				}
				cFetched = 0;
				return E_OUTOFMEMORY;
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
		if((ulCount < DISPLAYATTRIBUTE_INFO_NUM) &&
			((_iIndex + ulCount) < DISPLAYATTRIBUTE_INFO_NUM))
		{
			_iIndex += ulCount;
			return S_OK;
		}

		_iIndex = DISPLAYATTRIBUTE_INFO_NUM;
		return S_FALSE;
	}

private:
	LONG _iIndex;
	LONG _cRef;
};

#endif //ENUMDISPLAYATTRIBUTEINFO_H
