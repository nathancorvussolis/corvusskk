
#include "imcrvtip.h"
#include "TextService.h"
#include "DisplayAttributeInfo.h"
#include "EnumDisplayAttributeInfo.h"

STDAPI CTextService::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum)
{
	CEnumDisplayAttributeInfo *pAttributeEnum;

	if(ppEnum == nullptr)
	{
		return E_INVALIDARG;
	}

	*ppEnum = nullptr;

	try
	{
		pAttributeEnum = new CEnumDisplayAttributeInfo();
	}
	catch(...)
	{
		return E_OUTOFMEMORY;
	}

	*ppEnum = pAttributeEnum;

	return S_OK;
}

STDAPI CTextService::GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo)
{
	if(ppInfo == nullptr)
	{
		return E_INVALIDARG;
	}

	*ppInfo = nullptr;

	for(int i = 0; i < _countof(c_gdDisplayAttributeInfo); i++)
	{
		if(IsEqualGUID(guid, c_gdDisplayAttributeInfo[i].guid))
		{
			try
			{
				*ppInfo = new CDisplayAttributeInfo(c_gdDisplayAttributeInfo[i].guid, &display_attribute_info[i]);
			}
			catch(...)
			{
				return E_OUTOFMEMORY;
			}
			break;
		}
	}

	if(*ppInfo == nullptr)
	{
		return E_INVALIDARG;
	}

	return S_OK;
}

void CTextService::_ClearCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext)
{
	ITfRange *pRange;
	if(_IsComposing() && _pComposition->GetRange(&pRange) == S_OK)
	{
		ITfProperty *pProperty;
		if(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pProperty) == S_OK)
		{
			pProperty->Clear(ec, pRange);
			SafeRelease(&pProperty);
		}
		SafeRelease(&pRange);
	}
}

BOOL CTextService::_SetCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext, ITfRange *pRange, TfGuidAtom gaDisplayAttribute)
{
	HRESULT hr = E_FAIL;

	ITfProperty *pProperty;
	if(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pProperty) == S_OK)
	{
		VARIANT var;
		var.vt = VT_I4;
		var.lVal = gaDisplayAttribute;
		hr = pProperty->SetValue(ec, pRange, &var);
		SafeRelease(&pProperty);
	}

	return (hr == S_OK);
}

BOOL CTextService::_InitDisplayAttributeGuidAtom()
{
	HRESULT hr = E_FAIL;
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

	ITfCategoryMgr *pCategoryMgr;
	if(CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr)) == S_OK)
	{
		for(int i = 0; i < _countof(displayAttributeAtom); i++)
		{
			hr = pCategoryMgr->RegisterGUID(displayAttributeAtom[i].guid, displayAttributeAtom[i].patom);
		}
		SafeRelease(&pCategoryMgr);
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
