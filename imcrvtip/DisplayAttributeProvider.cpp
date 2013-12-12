
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
	size_t i;

	if(ppInfo == NULL)
	{
		return E_INVALIDARG;
	}

	*ppInfo = NULL;

	for(i = 0; i < _countof(c_gdDisplayAttributeInfo); i++)
	{
		if(IsEqualGUID(guid, c_gdDisplayAttributeInfo[i].guid))
		{
			*ppInfo = new CDisplayAttributeInfo(c_gdDisplayAttributeInfo[i].guid, &display_attribute_info[i]);
			if(*ppInfo == NULL)
			{
				return E_OUTOFMEMORY;
			}
			break;
		}
	}

	if(*ppInfo == NULL)
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
	size_t i;
	const struct {
		const GUID guid;
		TfGuidAtom *patom;
	} displayAttributeAtom[] = {
		{c_guidDisplayAttributeInputMark,	&_gaDisplayAttributeInputMark},
		{c_guidDisplayAttributeInputText,	&_gaDisplayAttributeInputText},
		{c_guidDisplayAttributeInputOkuri,	&_gaDisplayAttributeInputOkuri},
		{c_guidDisplayAttributeConvMark,	&_gaDisplayAttributeConvMark},
		{c_guidDisplayAttributeConvText,	&_gaDisplayAttributeConvText},
		{c_guidDisplayAttributeConvOkuri,	&_gaDisplayAttributeConvOkuri},
		{c_guidDisplayAttributeConvAnnot,	&_gaDisplayAttributeConvAnnot}
	};

	if(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr)) == S_OK)
	{
		for(i = 0; i < _countof(displayAttributeAtom); i++)
		{
			hr = pCategoryMgr->RegisterGUID(displayAttributeAtom[i].guid, displayAttributeAtom[i].patom);
		}
		pCategoryMgr->Release();
	}

	return (hr == S_OK);
}

BOOL CTextService::display_attribute_series[DISPLAYATTRIBUTE_INFO_NUM] =
{
	c_gdDisplayAttributeInfo[0].se,
	c_gdDisplayAttributeInfo[1].se,
	c_gdDisplayAttributeInfo[2].se,
	c_gdDisplayAttributeInfo[3].se,
	c_gdDisplayAttributeInfo[4].se,
	c_gdDisplayAttributeInfo[5].se,
	c_gdDisplayAttributeInfo[6].se
};

TF_DISPLAYATTRIBUTE CTextService::display_attribute_info[DISPLAYATTRIBUTE_INFO_NUM] =
{
	c_gdDisplayAttributeInfo[0].da,
	c_gdDisplayAttributeInfo[1].da,
	c_gdDisplayAttributeInfo[2].da,
	c_gdDisplayAttributeInfo[3].da,
	c_gdDisplayAttributeInfo[4].da,
	c_gdDisplayAttributeInfo[5].da,
	c_gdDisplayAttributeInfo[6].da
};
