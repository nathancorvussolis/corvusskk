
#include "corvustip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "LanguageBar.h"

static const TF_PRESERVEDKEY c_PreservedKey0 = { VK_OEM_3/*0xC0*/, TF_MOD_ALT };
static const TF_PRESERVEDKEY c_PreservedKey1 = { VK_KANJI, TF_MOD_IGNORE_ALL_MODIFIER };

static const WCHAR c_PreservedKeyDesc[] = L"OnOff";

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
		return TRUE;
	}

	SHORT ctrl = GetKeyState(VK_CONTROL) & 0x8000;

	WCHAR ch = _GetCh(wParam);
	BYTE sf = _GetSf(wParam, ch);

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
		case SKK_KANA:
		case SKK_JLATIN:
		case SKK_ASCII:
		case SKK_ABBREV:
			return TRUE;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	//無効
	if(ch < KEYMAPNUM)
	{
		if(keymap_void[ch] == SKK_VOID)
		{
			return TRUE;
		}
	}
	//処理しないCtrlキー
	if(ctrl)
	{
		return FALSE;
	}
	// roman input
	if(ch >= L'\x20' && ch <= L'\x7E')
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
	*pfEaten = _IsKeyEaten(pic, wParam);
	return S_OK;
}

STDAPI CTextService::OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	*pfEaten = _IsKeyEaten(pic, wParam);

	if(*pfEaten)
	{
		_InvokeKeyHandler(pic, wParam, lParam, SKK_NULL);
	}
	return S_OK;
}

STDAPI CTextService::OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	*pfEaten = _IsKeyEaten(pic, wParam);
	return S_OK;
}

STDAPI CTextService::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	*pfEaten = _IsKeyEaten(pic, wParam);
	return S_OK;
}

STDAPI CTextService::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten)
{
	if(IsEqualGUID(rguid, c_guidPreservedKeyOnOff))
	{
		BOOL fOpen = _IsKeyboardOpen();
		if(!fOpen)
		{
			exinputmode = im_default;	// -> _KeyboardChanged()
		}
		else
		{
			_ClearComposition();
		}
		_SetKeyboardOpen(fOpen ? FALSE : TRUE);
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
	ITfKeystrokeMgr *pKeystrokeMgr;
	HRESULT hr = E_FAIL;

	if(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) == S_OK)
	{
		hr = pKeystrokeMgr->AdviseKeyEventSink(_ClientId, (ITfKeyEventSink *)this, TRUE);
		pKeystrokeMgr->Release();
	}

	return (hr == S_OK);
}

void CTextService::_UninitKeyEventSink()
{
	ITfKeystrokeMgr *pKeystrokeMgr;

	if(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) == S_OK)
	{
		pKeystrokeMgr->UnadviseKeyEventSink(_ClientId);
		pKeystrokeMgr->Release();
	}
}

BOOL CTextService::_InitPreservedKey()
{
	ITfKeystrokeMgr *pKeystrokeMgr;
	HRESULT hr = E_FAIL;

	if(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) == S_OK)
	{
		hr = pKeystrokeMgr->PreserveKey(_ClientId, c_guidPreservedKeyOnOff,
			&c_PreservedKey0, c_PreservedKeyDesc, (ULONG)wcslen(c_PreservedKeyDesc));

		hr = pKeystrokeMgr->PreserveKey(_ClientId, c_guidPreservedKeyOnOff,
			&c_PreservedKey1, c_PreservedKeyDesc, (ULONG)wcslen(c_PreservedKeyDesc));

		pKeystrokeMgr->Release();
	}

	return (hr == S_OK);
}

void CTextService::_UninitPreservedKey()
{
	ITfKeystrokeMgr *pKeystrokeMgr;

	if(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) == S_OK)
	{
		pKeystrokeMgr->UnpreserveKey(c_guidPreservedKeyOnOff, &c_PreservedKey0);
		pKeystrokeMgr->UnpreserveKey(c_guidPreservedKeyOnOff, &c_PreservedKey1);

		pKeystrokeMgr->Release();
	}
}
