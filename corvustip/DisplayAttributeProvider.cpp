
#include "corvustip.h"
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
	else if(IsEqualGUID(guid, c_guidDisplayAttributeConverted))
	{
		*ppInfo = new CDisplayAttributeInfoConverted();

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

BOOL CTextService::_SetCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext, TfGuidAtom gaDisplayAttribute)
{
	ITfRange *pRangeComposition;
	ITfProperty *pDisplayAttributeProperty;
	HRESULT hr = E_FAIL;

	if(_pComposition->GetRange(&pRangeComposition) == S_OK)
	{
		if(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) == S_OK)
		{
			VARIANT var;
			var.vt = VT_I4;
			var.lVal = gaDisplayAttribute;
			hr = pDisplayAttributeProperty->SetValue(ec, pRangeComposition, &var);
			pDisplayAttributeProperty->Release();
		}
		pRangeComposition->Release();
	}

	return (hr == S_OK);
}

BOOL CTextService::_InitDisplayAttributeGuidAtom()
{
	ITfCategoryMgr *pCategoryMgr;
	HRESULT hr = E_FAIL;

	if(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER,
	                    IID_ITfCategoryMgr, (void**)&pCategoryMgr) == S_OK)
	{
		hr = pCategoryMgr->RegisterGUID(c_guidDisplayAttributeInput, &_gaDisplayAttributeInput);
		hr = pCategoryMgr->RegisterGUID(c_guidDisplayAttributeConverted, &_gaDisplayAttributeConverted);
		pCategoryMgr->Release();
	}

	return (hr == S_OK);
}
