#pragma once

#include "TextService.h"
#include "FnCandidateString.h"
#include "FnEnumCandidates.h"

class CFnCandidateList : public ITfCandidateList
{
public:
	CFnCandidateList(CTextService *pTextService, const std::wstring &searchkey, const CANDIDATES &candidates)
	{
		DllAddRef();

		_cRef = 1;

		_pTextService = pTextService;

		_searchkey = searchkey;
		_candidates = candidates;
	}

	~CFnCandidateList()
	{
		_pTextService.Release();

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

		if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfCandidateList))
		{
			*ppvObj = static_cast<ITfCandidateList *>(this);
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

	// ITfCandidateList
	STDMETHODIMP EnumCandidates(IEnumTfCandidates **ppEnum)
	{
		IEnumTfCandidates *pEnumCandidates = nullptr;

		if(ppEnum == nullptr)
		{
			return E_INVALIDARG;
		}

		*ppEnum = nullptr;

		try
		{
			pEnumCandidates = new CFnEnumCandidates(_candidates);
		}
		catch(...)
		{
			return E_OUTOFMEMORY;
		}

		*ppEnum = pEnumCandidates;

		return S_OK;
	}

	STDMETHODIMP GetCandidate(ULONG nIndex, ITfCandidateString **ppCand)
	{
		ITfCandidateString *pCandidateString = nullptr;

		if(ppCand == nullptr)
		{
			return E_INVALIDARG;
		}

		*ppCand = nullptr;

		if(nIndex >= (ULONG)_candidates.size())
		{
			return E_FAIL;
		}

		try
		{
			pCandidateString = new CFnCandidateString(nIndex, _candidates[nIndex].first.first);
		}
		catch(...)
		{
			return E_OUTOFMEMORY;
		}

		*ppCand = pCandidateString;

		return S_OK;
	}

	STDMETHODIMP GetCandidateNum(ULONG *pnCnt)
	{
		if(pnCnt == nullptr)
		{
			return E_INVALIDARG;
		}

		*pnCnt = (ULONG)_candidates.size();

		return S_OK;
	}

	STDMETHODIMP SetResult(ULONG nIndex, TfCandidateResult imcr)
	{
		HRESULT hr = S_OK;
		if(imcr == CAND_FINALIZED)
		{
			hr = _pTextService->_SetResult(_searchkey, _candidates, nIndex);
		}

		return hr;
	}

private:
	LONG _cRef;
	CComPtr<CTextService> _pTextService;
	std::wstring _searchkey;
	CANDIDATES _candidates;
};
