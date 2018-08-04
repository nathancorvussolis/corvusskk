
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"

static LPCWSTR c_PreservedKeyDesc[PRESERVEDKEY_NUM] = {L"ON", L"OFF"};
static const GUID c_guidPreservedKeyOnOff[PRESERVEDKEY_NUM] = {c_guidPreservedKeyOn, c_guidPreservedKeyOff};

BOOL CTextService::_IsKeyEaten(ITfContext *pContext, WPARAM wParam)
{
	if(_IsKeyboardDisabled())
	{
		return FALSE;
	}

	if(!_IsKeyboardOpen())
	{
		return FALSE;
	}

	if(_pCandidateList && _pCandidateList->_IsContextCandidateWindow(pContext))
	{
		return FALSE;
	}

	if(_IsComposing())
	{
		if(inputmode != im_ascii)
		{
			return TRUE;
		}
	}

	SHORT vk_ctrl = GetKeyState(VK_CONTROL) & 0x8000;
	SHORT vk_kana = GetKeyState(VK_KANA) & 0x0001;

	WCHAR ch = _GetCh((BYTE)wParam);
	BYTE sf = _GetSf((BYTE)wParam, ch);

	//確定状態で処理する機能
	switch(inputmode)
	{
	case im_jlatin:
	case im_ascii:
		switch(sf)
		{
		case SKK_JMODE:
			return TRUE;
			break;
		default:
			break;
		}
		break;
	case im_hiragana:
	case im_katakana:
		switch(sf)
		{
		case SKK_CONV_POINT:
		case SKK_KANA:
		case SKK_CONV_CHAR:
		case SKK_JLATIN:
		case SKK_ASCII:
		case SKK_ABBREV:
			return TRUE;
			break;
		case SKK_DIRECT:
			if(cx_setbydirect && !inputkey && roman.empty())
			{
				return FALSE;
			}
			break;
		default:
			break;
		}
		break;
	case im_katakana_ank:
		switch(sf)
		{
		case SKK_KANA:
		case SKK_CONV_CHAR:
		case SKK_JLATIN:
		case SKK_ASCII:
			return TRUE;
			break;
		case SKK_DIRECT:
			if(cx_setbydirect && !inputkey && roman.empty())
			{
				return FALSE;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	//無効
	if(_IsKeyVoid(ch, (BYTE)wParam))
	{
		return TRUE;
	}

	//処理しないCtrlキー
	if(vk_ctrl != 0)
	{
		return FALSE;
	}

	//ASCIIモード、かなキーロックOFF
	if(inputmode == im_ascii && vk_kana == 0)
	{
		return FALSE;
	}

	if(ch >= L'\x20')
	{
		return TRUE;
	}

	return FALSE;
}

STDAPI CTextService::OnSetFocus(BOOL fForeground)
{
	return S_OK;
}

STDAPI CTextService::OnTestKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if(pfEaten == nullptr)
	{
		return E_INVALIDARG;
	}

	*pfEaten = _IsKeyEaten(pic, wParam);

	_EndInputModeWindow();

	if(!_IsKeyboardDisabled() && _IsKeyboardOpen() && !_IsComposing())
	{
		WCHAR ch = _GetCh((BYTE)wParam);
		if(_IsKeyVoid(ch, (BYTE)wParam))
		{
			_GetActiveFlags();
			_UpdateLanguageBar();
		}
	}

	return S_OK;
}

STDAPI CTextService::OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if(pfEaten == nullptr)
	{
		return E_INVALIDARG;
	}

	*pfEaten = _IsKeyEaten(pic, wParam);

	if(*pfEaten)
	{
		_InvokeKeyHandler(pic, wParam, lParam, SKK_NULL);
	}

	return S_OK;
}

STDAPI CTextService::OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if(pfEaten == nullptr)
	{
		return E_INVALIDARG;
	}

	*pfEaten = _IsKeyEaten(pic, wParam);

	return S_OK;
}

STDAPI CTextService::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if(pfEaten == nullptr)
	{
		return E_INVALIDARG;
	}

	*pfEaten = _IsKeyEaten(pic, wParam);

	return S_OK;
}

STDAPI CTextService::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten)
{
	if(pic == nullptr || pfEaten == nullptr)
	{
		return E_INVALIDARG;
	}

	BOOL fOpen = _IsKeyboardOpen();

	if(IsEqualGUID(rguid, c_guidPreservedKeyOn))
	{
		if(!fOpen)
		{
			inputmode = im_disable;

			_SetKeyboardOpen(TRUE);
		}
		else
		{
			_UpdateLanguageBar();
		}

		*pfEaten = TRUE;
	}
	else if(IsEqualGUID(rguid, c_guidPreservedKeyOff))
	{
		if(fOpen)
		{
			_ClearComposition();

			_SetKeyboardOpen(FALSE);
		}
		else
		{
			_UpdateLanguageBar();
		}

		*pfEaten = TRUE;
	}
	else
	{
		*pfEaten = FALSE;
	}

	return S_OK;
}

BOOL CTextService::_InitKeyEventSink()
{
	HRESULT hr = E_FAIL;

	ITfKeystrokeMgr *pKeystrokeMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr))) && (pKeystrokeMgr != nullptr))
	{
		hr = pKeystrokeMgr->AdviseKeyEventSink(_ClientId, (ITfKeyEventSink *)this, TRUE);

		SafeRelease(&pKeystrokeMgr);
	}

	return SUCCEEDED(hr);
}

void CTextService::_UninitKeyEventSink()
{
	ITfKeystrokeMgr *pKeystrokeMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr))) && (pKeystrokeMgr != nullptr))
	{
		pKeystrokeMgr->UnadviseKeyEventSink(_ClientId);

		SafeRelease(&pKeystrokeMgr);
	}
}

BOOL CTextService::_InitPreservedKey(int onoff)
{
	BOOL fRet = TRUE;
	HRESULT hr;

	if(onoff != 0 && onoff != 1)
	{
		return FALSE;
	}

	ITfKeystrokeMgr *pKeystrokeMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr))) && (pKeystrokeMgr != nullptr))
	{
		for(int i = 0; i < MAX_PRESERVEDKEY; i++)
		{
			if(preservedkey[onoff][i].uVKey == 0 && preservedkey[onoff][i].uModifiers == 0)
			{
				break;
			}

			hr = pKeystrokeMgr->PreserveKey(_ClientId, c_guidPreservedKeyOnOff[onoff],
				&preservedkey[onoff][i], c_PreservedKeyDesc[onoff], (ULONG)wcslen(c_PreservedKeyDesc[onoff]));

			if(FAILED(hr))
			{
				fRet = FALSE;
			}
		}

		SafeRelease(&pKeystrokeMgr);
	}
	else
	{
		fRet = FALSE;
	}

	return fRet;
}

void CTextService::_UninitPreservedKey(int onoff)
{
	HRESULT hr;

	if(onoff != 0 && onoff != 1)
	{
		return;
	}

	ITfKeystrokeMgr *pKeystrokeMgr = nullptr;
	if(SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr))) && (pKeystrokeMgr != nullptr))
	{
		for(int i = 0; i < MAX_PRESERVEDKEY; i++)
		{
			if(preservedkey[onoff][i].uVKey == 0 && preservedkey[onoff][i].uModifiers == 0)
			{
				break;
			}

			hr = pKeystrokeMgr->UnpreserveKey(c_guidPreservedKeyOnOff[onoff], &preservedkey[onoff][i]);
		}

		SafeRelease(&pKeystrokeMgr);
	}
}
