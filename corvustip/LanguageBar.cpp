
#include "corvustip.h"
#include "TextService.h"
#include "LanguageBar.h"

#define TEXTSERVICE_LANGBARITEMSINK_COOKIE 0x516b54ab

CLangBarItemButton::CLangBarItemButton(CTextService *pTextService)
{
	DllAddRef();

	_LangBarItemInfo.clsidService = c_clsidTextService;
	_LangBarItemInfo.guidItem = c_guidLangBarItemButton;
	_LangBarItemInfo.dwStyle = TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_TEXTCOLORICON | TF_LBI_STYLE_SHOWNINTRAY;
	_LangBarItemInfo.ulSort = 1;
	wcsncpy_s(_LangBarItemInfo.szDescription, LangbarItemDesc, _TRUNCATE);

	_pLangBarItemSink = NULL;

	_pTextService = pTextService;
	_pTextService->AddRef();

	_cRef = 1;
}

CLangBarItemButton::~CLangBarItemButton()
{
	DllRelease();
	_pTextService->Release();
}

STDAPI CLangBarItemButton::QueryInterface(REFIID riid, void **ppvObj)
{
	if(ppvObj == NULL)
	{
		return E_INVALIDARG;
	}

	*ppvObj = NULL;

	if(IsEqualIID(riid, IID_IUnknown) ||
	        IsEqualIID(riid, IID_ITfLangBarItem) ||
	        IsEqualIID(riid, IID_ITfLangBarItemButton))
	{
		*ppvObj = (ITfLangBarItemButton *)this;
	}
	else if(IsEqualIID(riid, IID_ITfSource))
	{
		*ppvObj = (ITfSource *)this;
	}

	if(*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) CLangBarItemButton::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) CLangBarItemButton::Release()
{
	if(--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CLangBarItemButton::GetInfo(TF_LANGBARITEMINFO *pInfo)
{
	*pInfo = _LangBarItemInfo;
	return S_OK;
}

STDAPI CLangBarItemButton::GetStatus(DWORD *pdwStatus)
{
	*pdwStatus = 0;
	return S_OK;
}

STDAPI CLangBarItemButton::Show(BOOL fShow)
{
	return E_NOTIMPL;
}

STDAPI CLangBarItemButton::GetTooltipString(BSTR *pbstrToolTip)
{
	BSTR bstrToolTip;

	if(pbstrToolTip == NULL)
	{
		return E_INVALIDARG;
	}

	*pbstrToolTip = NULL;

	bstrToolTip = SysAllocString(LangbarItemDesc);

	if(bstrToolTip == NULL)
	{
		return E_OUTOFMEMORY;
	}

	*pbstrToolTip = bstrToolTip;

	return S_OK;
}

STDAPI CLangBarItemButton::OnClick(TfLBIClick click, POINT pt, const RECT *prcArea)
{
	return S_OK;
}

STDAPI CLangBarItemButton::InitMenu(ITfMenu *pMenu)
{
	return S_OK;
}

STDAPI CLangBarItemButton::OnMenuSelect(UINT wID)
{
	return S_OK;
}

STDAPI CLangBarItemButton::GetIcon(HICON *phIcon)
{
	WCHAR *t = L"IDI_IM_DEFAULT";

	if(!_pTextService->_IsKeyboardDisabled() && _pTextService->_IsKeyboardOpen())
	{
		switch(_pTextService->inputmode)
		{
		case im_hiragana:
			t = L"IDI_IM_HIRAGANA";
			break;
		case im_katakana:
			t = L"IDI_IM_KATAKANA";
			break;
		case im_jlatin:
			t = L"IDI_IM_JLATIN";
			break;
		case im_ascii:
			t = L"IDI_IM_ASCII";
			break;
		default:
			break;
		}
	}

	*phIcon = (HICON)LoadImageW(g_hInst, t, IMAGE_ICON, 16, 16, 0);

	return (*phIcon != NULL) ? S_OK : E_FAIL;
}

STDAPI CLangBarItemButton::GetText(BSTR *pbstrText)
{
	BSTR bstrText;

	if(pbstrText == NULL)
	{
		return E_INVALIDARG;
	}

	*pbstrText = NULL;

	bstrText = SysAllocString(LangbarItemDesc);

	if(bstrText == NULL)
	{
		return E_OUTOFMEMORY;
	}

	*pbstrText = bstrText;

	return S_OK;
}

STDAPI CLangBarItemButton::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
	if(!IsEqualIID(IID_ITfLangBarItemSink, riid))
	{
		return CONNECT_E_CANNOTCONNECT;
	}

	if(_pLangBarItemSink != NULL)
	{
		return CONNECT_E_ADVISELIMIT;
	}

	if(punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK)
	{
		_pLangBarItemSink = NULL;
		return E_NOINTERFACE;
	}

	*pdwCookie = TEXTSERVICE_LANGBARITEMSINK_COOKIE;

	return S_OK;
}

STDAPI CLangBarItemButton::UnadviseSink(DWORD dwCookie)
{
	if(dwCookie != TEXTSERVICE_LANGBARITEMSINK_COOKIE)
	{
		return CONNECT_E_NOCONNECTION;
	}

	if(_pLangBarItemSink == NULL)
	{
		return CONNECT_E_NOCONNECTION;
	}

	_pLangBarItemSink->Release();
	_pLangBarItemSink = NULL;

	return S_OK;
}

STDAPI CLangBarItemButton::_Update()
{
	VARIANT var;

	var.vt = VT_I4;
	
	var.lVal = TF_SENTENCEMODE_PHRASEPREDICT;
	_pTextService->_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE, &var);	

	if(_pTextService->_IsKeyboardDisabled() || !_pTextService->_IsKeyboardOpen())
	{
		var.lVal = TF_CONVERSIONMODE_ALPHANUMERIC;
		_pTextService->_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
	}
	else
	{
		switch(_pTextService->inputmode)
		{
		case im_hiragana:
			var.lVal = TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_FULLSHAPE;
			_pTextService->_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
			break;
		case im_katakana:
			var.lVal = TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_KATAKANA | TF_CONVERSIONMODE_FULLSHAPE;
			_pTextService->_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
			break;
		case im_jlatin:
			var.lVal = TF_CONVERSIONMODE_ALPHANUMERIC | TF_CONVERSIONMODE_FULLSHAPE;
			_pTextService->_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
			break;
		case im_ascii:
			var.lVal = TF_CONVERSIONMODE_ALPHANUMERIC;
			_pTextService->_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
			break;
		default:
			var.lVal = TF_CONVERSIONMODE_ALPHANUMERIC;
			_pTextService->_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
			break;
		}
	}

	if(_pLangBarItemSink == NULL)
	{
		return E_FAIL;
	}

	return _pLangBarItemSink->OnUpdate(TF_LBI_ICON | TF_LBI_STATUS);
}

BOOL CTextService::_InitLanguageBar()
{
	ITfLangBarItemMgr *pLangBarItemMgr;
	BOOL fRet = FALSE;

	if(_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) == S_OK)
	{
		_pLangBarItem = new CLangBarItemButton(this);
		if(_pLangBarItem != NULL)
		{
			if(pLangBarItemMgr->AddItem(_pLangBarItem) == S_OK)
			{
				fRet = TRUE;
			}
			else
			{
				_pLangBarItem->Release();
				_pLangBarItem = NULL;
			}
		}
		pLangBarItemMgr->Release();
	}

	return fRet;
}

void CTextService::_UninitLanguageBar()
{
	ITfLangBarItemMgr *pLangBarItemMgr;

	if(_pLangBarItem != NULL)
	{
		if(_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) == S_OK)
		{
			pLangBarItemMgr->RemoveItem(_pLangBarItem);
			pLangBarItemMgr->Release();
		}
		_pLangBarItem->Release();
		_pLangBarItem = NULL;
	}
}

void CTextService::_UpdateLanguageBar()
{
	if(_pLangBarItem != NULL)
	{
		_pLangBarItem->_Update();
	}
}
