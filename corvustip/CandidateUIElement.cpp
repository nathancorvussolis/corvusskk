
#include "common.h"
#include "corvustip.h"
#include "TextService.h"
#include "CandidateWindow.h"
#include "CandidateList.h"

static LPCWSTR markNo = L":";
static LPCWSTR markAnnotation = L";";

CCandidateWindow::CCandidateWindow(CTextService *pTextService)
{
	_pTextService = pTextService;
	_pTextService->AddRef();

	_pCandidateList = _pTextService->_GetCandidateList();
	_pCandidateList->AddRef();

	_pCandidateWindow = NULL;
	_pCandidateWindowParent = NULL;

	_hwnd = NULL;
	_hwndParent = NULL;

	_preEnd = FALSE;
	_bShow = FALSE;
	_dwFlags = 0;
	_uShowedCount = 0;
	_uCount = 0;
	_uIndex = 0;
	_uPageCnt = 0;
	_CandCount.clear();
	_PageInex.clear();
	_CandStr.clear();

	hFont = NULL;

	_reg = FALSE;

	regword = FALSE;
	regwordfixed = FALSE;

	regwordtext.clear();
	regwordtextpos = 0;
	comptext.clear();

	_ClearStatusReg();

	_cRef = 1;
}

CCandidateWindow::~CCandidateWindow()
{
	if(_pCandidateWindow)
	{
		_pCandidateWindow->_EndUIElement();
		_pCandidateWindow->_Destroy();
		_pCandidateWindow->Release();
	}
	_pCandidateList->Release();
	_pTextService->Release();
}

STDAPI CCandidateWindow::QueryInterface(REFIID riid, void **ppvObj)
{
	if(ppvObj == NULL)
	{
		return E_INVALIDARG;
	}

	*ppvObj = NULL;

	if(IsEqualIID(riid, IID_IUnknown) ||
	        IsEqualIID(riid, IID_ITfUIElement) ||
	        IsEqualIID(riid, IID_ITfCandidateListUIElement) ||
	        IsEqualIID(riid, IID_ITfCandidateListUIElementBehavior))
	{
		*ppvObj = (ITfCandidateListUIElementBehavior *)this;
	}

	if(*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) CCandidateWindow::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) CCandidateWindow::Release()
{
	if(--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CCandidateWindow::GetDescription(BSTR *bstr)
{
	BSTR bstrDesc;

	if(bstr == NULL)
	{
		return E_INVALIDARG;
	}

	*bstr = NULL;

	bstrDesc = SysAllocString(TextServiceDesc);

	if(bstrDesc == NULL)
	{
		return E_OUTOFMEMORY;
	}

	*bstr = bstrDesc;

	return S_OK;
}

STDAPI CCandidateWindow::GetGUID(GUID *pguid)
{
	if(pguid == NULL)
	{
		return E_INVALIDARG;
	}

	*pguid = c_guidCandidateListUIElement;

	return S_OK;
}

STDAPI CCandidateWindow::Show(BOOL bShow)
{
	if(!_bShow)
	{
		return E_UNEXPECTED;
	}

	if(_pCandidateWindow)
	{
		_pCandidateWindow->Show(bShow);
	}
#ifndef _DEBUG
	else
	{
#endif
		if(bShow)
		{
			if(_hwnd != NULL)
			{
				SendMessageW(_hwnd, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
			}

		}
		else
		{
			if(_hwnd != NULL)
			{
				SendMessageW(_hwnd, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
			}
		}
#ifndef _DEBUG
	}
#endif

	return S_OK;
}

STDAPI CCandidateWindow::IsShown(BOOL *pbShow)
{
	if(pbShow == NULL)
	{
		return E_INVALIDARG;
	}

	*pbShow = IsWindowVisible(_hwnd);

	return S_OK;
}

STDAPI CCandidateWindow::GetUpdatedFlags(DWORD *pdwFlags)
{
	if(pdwFlags == NULL)
	{
		return E_INVALIDARG;
	}

	*pdwFlags = _dwFlags;

	return S_OK;
}

STDAPI CCandidateWindow::GetDocumentMgr(ITfDocumentMgr **ppdim)
{
	if(ppdim == NULL)
	{
		return E_INVALIDARG;
	}

	*ppdim = NULL;

	return S_OK;
}

STDAPI CCandidateWindow::GetCount(UINT *puCount)
{
	if(puCount == NULL)
	{
		return E_INVALIDARG;
	}

	*puCount = _uCount;

	return S_OK;
}

STDAPI CCandidateWindow::GetSelection(UINT *puIndex)
{
	if(puIndex == NULL)
	{
		return E_INVALIDARG;
	}

	*puIndex = _uIndex;

	return S_OK;
}

STDAPI CCandidateWindow::GetString(UINT uIndex, BSTR *pstr)
{
	if(pstr == NULL)
	{
		return E_INVALIDARG;
	}

	if(uIndex < _CandStr.size())
	{
		*pstr = SysAllocString(_CandStr[uIndex].c_str());
	}
	else
	{
		*pstr = SysAllocString(L"");
	}

	return S_OK;
}

STDAPI CCandidateWindow::GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt)
{
	UINT i;
	HRESULT hr = S_OK;

	if(puPageCnt == NULL)
	{
		return E_INVALIDARG;
	}

	if(uSize >= _uPageCnt)
	{
		uSize = _uPageCnt;
	}
	else
	{
		hr = S_FALSE;
	}

	if(pIndex != NULL)
	{
		for(i=0; i<uSize; i++)
		{
			*pIndex = _PageInex[i];
			pIndex++;
		}
	}

	*puPageCnt = _uPageCnt;
	return hr;
}

STDAPI CCandidateWindow::SetPageIndex(UINT *pIndex, UINT uPageCnt)
{
	UINT uCandCnt, i, j, k;

	if(pIndex == NULL)
	{
		return E_INVALIDARG;
	}

	for(j=0; j<uPageCnt-1; j++)
	{
		uCandCnt = pIndex[j + 1] - pIndex[j];
		if(uCandCnt > MAX_SELKEY_C)
		{
			return E_INVALIDARG;
		}
	}

	_PageInex.clear();
	_CandCount.clear();
	_CandStr.clear();
	j = 0;
	k = 0;
	for(j=0; j<uPageCnt; j++)
	{
		if(j < (uPageCnt - 1))
		{
			uCandCnt = pIndex[j + 1] - pIndex[j];
		}
		else
		{
			uCandCnt = _uCount - k;
		}

		pIndex[j] = k;
		_PageInex.push_back(k);
		_CandCount.push_back(uCandCnt);

		for(i=0; i<uCandCnt; i++)
		{
			if(k == _uCount)
			{
				break;
			}

			_CandStr.push_back(_pTextService->selkey[(i % MAX_SELKEY_C)][0]);
			_CandStr[k].append(markNo + _pTextService->candidates[ _uShowedCount + k ].first.first);
			
			if(_pTextService->c_annotation &&
				!_pTextService->candidates[ _uShowedCount + k ].first.second.empty())
			{
				_CandStr[k].append(markAnnotation +
					_pTextService->candidates[ _uShowedCount + k ].first.second);
			}

			++k;
		}
	}

	_uPageCnt = uPageCnt;
	return S_OK;
}

STDAPI CCandidateWindow::GetCurrentPage(UINT *puPage)
{
	UINT i;

	if(puPage == NULL)
	{
		return E_INVALIDARG;
	}

	*puPage = 0;

	if(_uPageCnt == 0)
	{
		return E_UNEXPECTED;
	}

	if(_uPageCnt == 1)
	{
		*puPage = 0;
		return S_OK;
	}

	for(i=1; i<_uPageCnt; i++)
	{
		if(_PageInex[i] > _uIndex)
		{
			break;
		}
	}

	*puPage = i - 1;
	return S_OK;
}

STDAPI CCandidateWindow::SetSelection(UINT nIndex)
{
	UINT uOldPage, uNewPage;

	if(nIndex >= _uCount)
	{
		return E_INVALIDARG;
	}

	GetCurrentPage(&uOldPage);
	_uIndex = nIndex;
	GetCurrentPage(&uNewPage);

	_dwFlags = TF_CLUIE_SELECTION;
	if(uNewPage != uOldPage)
	{
		_dwFlags |= TF_CLUIE_CURRENTPAGE;
	}

	_UpdateUIElement();
	return S_OK;
}

STDAPI CCandidateWindow::Finalize()
{
	if(_pCandidateList)
	{
		_pCandidateList->_EndCandidateList();
	}
	return S_OK;
}

STDAPI CCandidateWindow::Abort()
{
	if(_pCandidateList)
	{
		_pCandidateList->_EndCandidateList();
	}
	return S_OK;
}
