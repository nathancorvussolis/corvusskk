
#include "imcrvtip.h"
#include "TextService.h"
#include "DisplayAttributeInfo.h"
#include "EnumDisplayAttributeInfo.h"

STDAPI CTextService::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum)
{
	CEnumDisplayAttributeInfo *pAttributeEnum;

	if(ppEnum == NULL)
	{
		return E_INVALIDARG;
	}

	*ppEnum = NULL;

	pAttributeEnum = new CEnumDisplayAttributeInfo();

	if(pAttributeEnum == NULL)
	{
		return E_OUTOFMEMORY;
	}

	*ppEnum = pAttributeEnum;

	return S_OK;
}

STDAPI CTextService::GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo)
{
	if(ppInfo == NULL)
	{
		return E_INVALIDARG;
	}

	*ppInfo = NULL;

	if(IsEqualGUID(guid, c_guidDisplayAttributeInput))
	{
		*ppInfo = new CDisplayAttributeInfoInput();

		if(*ppInfo == NULL)
		{
			return E_OUTOFMEMORY;
		}
	}
	else if(IsEqualGUID(guid, c_guidDisplayAttributeCandidate))
	{
		*ppInfo = new CDisplayAttributeInfoCandidate();

		if(*ppInfo == NULL)
		{
			return E_OUTOFMEMORY;
		}
	}
	else if(IsEqualGUID(guid, c_guidDisplayAttributeAnnotation))
	{
		*ppInfo = new CDisplayAttributeInfoAnnotation();

		if(*ppInfo == NULL)
		{
			return E_OUTOFMEMORY;
		}
	}
	else
	{
		return E_INVALIDARG;
	}

	return S_OK;
}

void CTextService::_ClearCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext)
{
	ITfRange *pRangeComposition;
	ITfProperty *pDisplayAttributeProperty;

	if(_pComposition->GetRange(&pRangeComposition) == S_OK)
	{
		if(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) == S_OK)
		{
			pDisplayAttributeProperty->Clear(ec, pRangeComposition);
			pDisplayAttributeProperty->Release();
		}
		pRangeComposition->Release();
	}
}

BOOL CTextService::_SetCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext, ITfRange *pRange, TfGuidAtom gaDisplayAttribute)
{
	ITfProperty *pDisplayAttributeProperty;
	HRESULT hr = E_FAIL;

	if(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) == S_OK)
	{
		VARIANT var;
		var.vt = VT_I4;
		var.lVal = gaDisplayAttribute;
		hr = pDisplayAttributeProperty->SetValue(ec, pRange, &var);
		pDisplayAttributeProperty->Release();
	}

	return (hr == S_OK);
}

BOOL CTextService::_InitDisplayAttributeGuidAtom()
{
	ITfCategoryMgr *pCategoryMgr;
	HRESULT hr = E_FAIL;

	if(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr)) == S_OK)
	{
		hr = pCategoryMgr->RegisterGUID(c_guidDisplayAttributeInput, &_gaDisplayAttributeInput);
		hr = pCategoryMgr->RegisterGUID(c_guidDisplayAttributeCandidate, &_gaDisplayAttributeCandidate);
		hr = pCategoryMgr->RegisterGUID(c_guidDisplayAttributeAnnotation, &_gaDisplayAttributeAnnotation);
		pCategoryMgr->Release();
	}

	return (hr == S_OK);
}
