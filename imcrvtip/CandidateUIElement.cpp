
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateWindow.h"
#include "CandidateList.h"

static LPCWSTR markNo = L":";
static LPCWSTR markAnnotation = L";";

CCandidateWindow::CCandidateWindow(CTextService *pTextService)
{
	DllAddRef();

	_cRef = 1;

	_pTextService = pTextService;
	_pTextService->AddRef();

	_pCandidateList = _pTextService->_GetCandidateList();
	_pCandidateList->AddRef();

	_pCandidateWindow = NULL;
	_pCandidateWindowParent = NULL;
	_pInputModeWindow = NULL;

	_hwnd = NULL;
	_hwndParent = NULL;

	_preEnd = FALSE;

	_dwUIElementId = 0;
	_bShow = FALSE;
	_dwFlags = 0;
	_uShowedCount = 0;
	_uCount = 0;
	_uIndex = 0;
	_uPageCnt = 0;
	_CandCount.clear();
	_PageIndex.clear();
	_CandStr.clear();

	hFont = NULL;

	_pD2DFactory = NULL;
	_pD2DDCRT = NULL;
	for(int i = 0; i < DISPLAY_COLOR_NUM; i++)
	{
		_pD2DBrush[i] = NULL;
	}
	_drawtext_option = D2D1_DRAW_TEXT_OPTIONS_NONE;
	_pDWFactory = NULL;
	_pDWTF = NULL;

	_reg = FALSE;

	regword = FALSE;
	regwordul = FALSE;
	regwordfixed = FALSE;

	regwordtext.clear();
	regwordtextpos = 0;
	comptext.clear();

	_ClearStatusReg();
}

CCandidateWindow::~CCandidateWindow()
{
	if(_pCandidateWindow != NULL)
	{
		_pCandidateWindow->_EndUIElement();
		_pCandidateWindow->_Destroy();
	}
	SafeRelease(&_pCandidateWindow);

	SafeRelease(&_pCandidateList);
	SafeRelease(&_pTextService);

	DllRelease();
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

	if(_pCandidateWindow != NULL)
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
				SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
				if(_depth == 0)
				{
					NotifyWinEvent(EVENT_OBJECT_IME_SHOW, _hwnd, OBJID_CLIENT, CHILDID_SELF);
				}
			}

		}
		else
		{
			if(_hwnd != NULL)
			{
				SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);
				if(_depth == 0)
				{
					NotifyWinEvent(EVENT_OBJECT_IME_HIDE, _hwnd, OBJID_CLIENT, CHILDID_SELF);
				}
			}
		}
#ifndef _DEBUG
	}
#endif

	if(_pInputModeWindow != NULL && regword)
	{
		_pInputModeWindow->_Show(bShow);
	}

	return S_OK;
}

STDAPI CCandidateWindow::IsShown(BOOL *pbShow)
{
	if(_pCandidateWindow != NULL)
	{
		return _pCandidateWindow->IsShown(pbShow);
	}

	if(pbShow == NULL)
	{
		return E_INVALIDARG;
	}

	*pbShow = IsWindowVisible(_hwnd);

	return S_OK;
}

STDAPI CCandidateWindow::GetUpdatedFlags(DWORD *pdwFlags)
{
	if(_pCandidateWindow != NULL)
	{
		return _pCandidateWindow->GetUpdatedFlags(pdwFlags);
	}

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
	if(_pCandidateWindow)
	{
		return _pCandidateWindow->GetCount(puCount);
	}

	if(puCount == NULL)
	{
		return E_INVALIDARG;
	}

	if(regwordul)
	{
		*puCount = 1;
	}
	else
	{
		*puCount = _uCount;
	}

	return S_OK;
}

STDAPI CCandidateWindow::GetSelection(UINT *puIndex)
{
	if(_pCandidateWindow)
	{
		return _pCandidateWindow->GetSelection(puIndex);
	}

	if(puIndex == NULL)
	{
		return E_INVALIDARG;
	}

	if(regwordul)
	{
		*puIndex = 0;
	}
	else
	{
		*puIndex = _uIndex;
	}

	return S_OK;
}

STDAPI CCandidateWindow::GetString(UINT uIndex, BSTR *pstr)
{
	if(_pCandidateWindow)
	{
		return _pCandidateWindow->GetString(uIndex, pstr);
	}

	if(pstr == NULL)
	{
		return E_INVALIDARG;
	}

	if(regwordul)
	{
		*pstr = SysAllocString(disptext.c_str());
	}
	else
	{
		if(uIndex < _CandStr.size())
		{
			*pstr = SysAllocString(_CandStr[uIndex].c_str());
		}
		else
		{
			*pstr = SysAllocString(L"");
		}
	}

	return S_OK;
}

STDAPI CCandidateWindow::GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt)
{
	HRESULT hr = S_OK;
	UINT i;

	if(_pCandidateWindow)
	{
		return _pCandidateWindow->GetPageIndex(pIndex, uSize, puPageCnt);
	}

	if(puPageCnt == NULL)
	{
		return E_INVALIDARG;
	}

	if(regwordul)
	{
		if(uSize > 0)
		{
			*pIndex = 0;
		}
		*puPageCnt = 1;
	}
	else
	{
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
			for(i = 0; i < uSize; i++)
			{
				*pIndex = _PageIndex[i];
				pIndex++;
			}
		}

		*puPageCnt = _uPageCnt;
	}

	return hr;
}

STDAPI CCandidateWindow::SetPageIndex(UINT *pIndex, UINT uPageCnt)
{
	UINT uCandCnt, i, j, k;

	if(_pCandidateWindow)
	{
		return _pCandidateWindow->SetPageIndex(pIndex, uPageCnt);
	}

	if(pIndex == NULL)
	{
		return E_INVALIDARG;
	}

	if(regwordul)
	{
		if(uPageCnt > 0)
		{
			*pIndex = 0;
		}
	}
	else
	{
		for(j = 0; j < (uPageCnt - 1); j++)
		{
			uCandCnt = pIndex[j + 1] - pIndex[j];
			if(uCandCnt > MAX_SELKEY_C)
			{
				return E_INVALIDARG;
			}
		}

		_PageIndex.clear();
		_CandCount.clear();
		_CandStr.clear();
		j = 0;
		k = 0;
		for(j = 0; j < uPageCnt; j++)
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
			_PageIndex.push_back(k);
			_CandCount.push_back(uCandCnt);

			for(i = 0; i < uCandCnt; i++)
			{
				if(k == _uCount)
				{
					break;
				}

				_CandStr.push_back(_pTextService->selkey[(i % MAX_SELKEY_C)][0]);
				_CandStr[k].append(markNo + _pTextService->candidates[_uShowedCount + k].first.first);

				if(_pTextService->cx_annotation &&
					!_pTextService->candidates[_uShowedCount + k].first.second.empty())
				{
					_CandStr[k].append(markAnnotation +
						_pTextService->candidates[_uShowedCount + k].first.second);
				}

				++k;
			}
		}

		_uPageCnt = uPageCnt;
	}

	return S_OK;
}

STDAPI CCandidateWindow::GetCurrentPage(UINT *puPage)
{
	UINT i;

	if(_pCandidateWindow)
	{
		return _pCandidateWindow->GetCurrentPage(puPage);
	}

	if(puPage == NULL)
	{
		return E_INVALIDARG;
	}

	if(regwordul)
	{
		*puPage = 0;
	}
	else
	{
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

		for(i = 1; i < _uPageCnt; i++)
		{
			if(_PageIndex[i] > _uIndex)
			{
				break;
			}
		}

		*puPage = i - 1;
	}

	return S_OK;
}

STDAPI CCandidateWindow::SetSelection(UINT nIndex)
{
	UINT uOldPage, uNewPage;

	if(_pCandidateWindow)
	{
		return _pCandidateWindow->SetSelection(nIndex);
	}

	if(nIndex >= _uCount)
	{
		return E_INVALIDARG;
	}

	if(regwordul)
	{
		_Update();
	}
	else
	{
		GetCurrentPage(&uOldPage);
		_uIndex = nIndex;
		GetCurrentPage(&uNewPage);

		_dwFlags = TF_CLUIE_SELECTION;
		if(uNewPage != uOldPage)
		{
			_dwFlags |= TF_CLUIE_CURRENTPAGE;
		}

		_UpdateUIElement();
	}

	return S_OK;
}

STDAPI CCandidateWindow::Finalize()
{
	if(_pCandidateList != NULL)
	{
		_pCandidateList->_EndCandidateList();
	}
	return S_OK;
}

STDAPI CCandidateWindow::Abort()
{
	if(_pCandidateList != NULL)
	{
		_pCandidateList->_EndCandidateList();
	}
	return S_OK;
}
