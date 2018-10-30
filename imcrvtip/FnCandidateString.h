#pragma once

class CFnCandidateString : public ITfCandidateString
{
public:
	CFnCandidateString(ULONG index, const std::wstring &candidate)
	{
		DllAddRef();

		_cRef = 1;
		_nIndex = index;
		_candidate = candidate;
	}

	~CFnCandidateString()
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

		if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfCandidateString))
		{
			*ppvObj = (ITfCandidateString *)this;
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

	// ITfCandidateString
	STDMETHODIMP GetString(BSTR *pbstr)
	{
		BSTR bstr;

		if(pbstr == nullptr)
		{
			return E_INVALIDARG;
		}

		*pbstr = nullptr;

		bstr = SysAllocString(_candidate.c_str());

		if(bstr == nullptr)
		{
			return E_OUTOFMEMORY;
		}

		*pbstr = bstr;

		return S_OK;
	}

	STDMETHODIMP GetIndex(ULONG *pnIndex)
	{
		if(pnIndex == nullptr)
		{
			return E_INVALIDARG;
		}

		*pnIndex = _nIndex;

		return S_OK;
	}

private:
	LONG _cRef;
	ULONG _nIndex;
	std::wstring _candidate;
};
