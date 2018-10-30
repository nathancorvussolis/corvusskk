#pragma once

#include "FnCandidateString.h"

class CFnEnumCandidates : public IEnumTfCandidates
{
public:
	CFnEnumCandidates(const CANDIDATES &candidates)
	{
		DllAddRef();

		_cRef = 1;
		_nIndex = 0;
		_candidates = candidates;
	}

	~CFnEnumCandidates()
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

		if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumTfCandidates))
		{
			*ppvObj = (IEnumTfCandidates *)this;
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

	// IEnumTfCandidates
	STDMETHODIMP Clone(IEnumTfCandidates **ppEnum)
	{
		CFnEnumCandidates *pClone;

		if(ppEnum == nullptr)
		{
			return E_INVALIDARG;
		}

		*ppEnum = nullptr;

		try
		{
			pClone = new CFnEnumCandidates(_candidates);
		}
		catch(...)
		{
			return E_OUTOFMEMORY;
		}

		pClone->_nIndex = _nIndex;

		*ppEnum = pClone;

		return S_OK;
	}

	STDMETHODIMP Next(ULONG ulCount, ITfCandidateString **ppCand, ULONG *pcFetched)
	{
		ULONG cFetched = 0;
		ITfCandidateString *pCandidateString;

		if(ppCand == nullptr)
		{
			return E_INVALIDARG;
		}

		if(ulCount == 0)
		{
			return S_OK;
		}

		while(cFetched < ulCount)
		{
			if(_nIndex >= (ULONG)_candidates.size())
			{
				break;
			}

			try
			{
				pCandidateString = new CFnCandidateString(_nIndex, _candidates[_nIndex].first.first);
			}
			catch(...)
			{
				for(ULONG i = 0; i < cFetched; i++)
				{
					delete *(ppCand + i);
				}
				return E_OUTOFMEMORY;
			}

			*(ppCand + cFetched) = pCandidateString;
			cFetched++;
			_nIndex++;
		}

		if(pcFetched != nullptr)
		{
			*pcFetched = cFetched;
		}

		return (cFetched == ulCount) ? S_OK : S_FALSE;
	}

	STDMETHODIMP Reset(void)
	{
		_nIndex = 0;
		return S_OK;
	}

	STDMETHODIMP Skip(ULONG ulCount)
	{
		if((_nIndex + ulCount) >= (ULONG)_candidates.size())
		{
			_nIndex = (ULONG)_candidates.size();
			return S_FALSE;
		}

		_nIndex += ulCount;
		return S_OK;
	}

private:
	LONG _cRef;
	ULONG _nIndex;
	CANDIDATES _candidates;
};
