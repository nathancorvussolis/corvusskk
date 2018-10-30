#pragma once

class CEnumDisplayAttributeInfo : public IEnumTfDisplayAttributeInfo
{
public:
	CEnumDisplayAttributeInfo()
	{
		DllAddRef();

		_cRef = 1;
		_nIndex = 0;
	}

	~CEnumDisplayAttributeInfo()
	{
		DllRelease();
	}

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj)
	{
		if(ppvObj == nullptr)
		{
			return E_INVALIDARG;
		}

		*ppvObj = nullptr;

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

		if(ppEnum == nullptr)
		{
			return E_INVALIDARG;
		}

		*ppEnum = nullptr;

		try
		{
			pClone = new CEnumDisplayAttributeInfo();
		}
		catch(...)
		{
			return E_OUTOFMEMORY;
		}

		pClone->_nIndex = _nIndex;

		*ppEnum = pClone;

		return S_OK;
	}

	STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched)
	{
		ULONG cFetched = 0;
		ITfDisplayAttributeInfo *pDisplayAttributeInfo;

		if(rgInfo == nullptr)
		{
			return E_INVALIDARG;
		}

		if(ulCount == 0)
		{
			return S_OK;
		}

		while(cFetched < ulCount)
		{
			if(_nIndex >= DISPLAYATTRIBUTE_INFO_NUM)
			{
				break;
			}

			try
			{
				pDisplayAttributeInfo = new CDisplayAttributeInfo(
					c_gdDisplayAttributeInfo[_nIndex].guid, &CTextService::display_attribute_info[_nIndex]);
			}
			catch(...)
			{
				for(ULONG i = 0; i < cFetched; i++)
				{
					delete *(rgInfo + i);
				}
				return E_OUTOFMEMORY;
			}

			*(rgInfo + cFetched) = pDisplayAttributeInfo;
			cFetched++;
			_nIndex++;
		}

		if(pcFetched != nullptr)
		{
			*pcFetched = cFetched;
		}

		return (cFetched == ulCount) ? S_OK : S_FALSE;
	}

	STDMETHODIMP Reset()
	{
		_nIndex = 0;
		return S_OK;
	}

	STDMETHODIMP Skip(ULONG ulCount)
	{
		if((_nIndex + ulCount) >= DISPLAYATTRIBUTE_INFO_NUM)
		{
			_nIndex = DISPLAYATTRIBUTE_INFO_NUM;
			return S_FALSE;
		}

		_nIndex += ulCount;
		return S_OK;
	}

private:
	LONG _cRef;
	ULONG _nIndex;
};
