
#include "configxml.h"
#include "imcrvtip.h"
#include "EditSession.h"
#include "TextService.h"

class CKeyHandlerEditSession : public CEditSessionBase
{
public:
	CKeyHandlerEditSession(CTextService *pTextService, ITfContext *pContext, WPARAM wParam, BYTE bSf) : CEditSessionBase(pTextService, pContext)
	{
		_wParam = wParam;
		_bSf = bSf;
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
#ifdef _DEBUG
		_pTextService->_HandleKey(ec, _pContext, _wParam, _bSf);
#else
		__try
		{
			_pTextService->_HandleKey(ec, _pContext, _wParam, _bSf);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			_pTextService->_ResetStatus();
			_pTextService->_ClearComposition();
		}
#endif
		return S_OK;
	}

private:
	WPARAM _wParam;
	BYTE _bSf;
};

HRESULT CTextService::_InvokeKeyHandler(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BYTE bSf)
{
	HRESULT hr = E_FAIL;

	try
	{
		CKeyHandlerEditSession *pEditSession = new CKeyHandlerEditSession(this, pContext, wParam, bSf);
		pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
		SafeRelease(&pEditSession);
	}
	catch(...)
	{
	}

	return hr;
}

HRESULT CTextService::_HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam, BYTE bSf)
{
	BYTE sf;
	WCHAR ch, chO = L'\0';
	HRESULT hrc = E_ABORT;

	if(bSf == SKK_NULL)
	{
		ch = _GetCh((BYTE)wParam);
		sf = _GetSf((BYTE)wParam, ch);
	}
	else
	{
		ch = WCHAR_MAX;
		sf = bSf;
	}

	if(ch == L'\0' && sf == SKK_NULL)
	{
		return S_FALSE;
	}

	//Windows8以降のタッチキーボードのバグ対応
	if(ch == L'次' || ch == L'前' || ch == L'頁')
	{
		return S_FALSE;
	}

	_GetActiveFlags();

	//補完
	switch(sf)
	{
	case SKK_NEXT_COMP:
	case SKK_PREV_COMP:
		break;
	case SKK_CANCEL:
		if(complement)
		{
			complement = FALSE;	//補完終了
			kana = kana.erase(cursoridx);

			if(cx_dynamiccomp || cx_dyncompmulti)
			{
				_DynamicComp(ec, pContext);
			}
			else
			{
				if(cx_stacompmulti)
				{
					_EndCompletionList(ec, pContext);
				}
				_Update(ec, pContext);
			}
			return S_OK;
		}
		break;
	default:
		if(complement)
		{
			complement = FALSE;	//補完終了
			_EndCompletionList(ec, pContext);
		}
		break;
	}

	//辞書削除
	if(purgedicmode)
	{
		switch(sf)
		{
		case SKK_ENTER:
			sf = SKK_PURGE_DIC;
			break;
		case SKK_CANCEL:
			purgedicmode = FALSE;
			_Update(ec, pContext);
			return S_OK;
			break;
		default:
			switch(ch)
			{
			case L'Y': case L'y':
				sf = SKK_PURGE_DIC;
				break;
			case L'N': case L'n':
				purgedicmode = FALSE;
				_Update(ec, pContext);
				return S_OK;
				break;
			default:
				return S_FALSE;
			}
			break;
		}
	}

	BOOL iscomp = _IsComposing();

	//ローマ字仮名変換表を優先させる
	switch(inputmode)
	{
	case im_hiragana:
	case im_katakana:
	case im_katakana_ank:
		if(!abbrevmode && !roman.empty() && ch != L'\0')
		{
			ROMAN_KANA_CONV rkc;
			std::wstring roman_conv = roman;
			roman_conv.push_back(ch);
			wcsncpy_s(rkc.roman, roman_conv.c_str(), _TRUNCATE);
			hrc = _ConvRomanKana(&rkc);
			if(hrc != E_ABORT && !rkc.wait)
			{
				sf = SKK_NULL;
			}
		}
		break;
	default:
		break;
	}

	//skk-sticky-key
	if(sf == SKK_CONV_POINT)
	{
		if(!abbrevmode || showentry)
		{
			if(inputkey && !showentry && roman.empty() && kana.empty())
			{
				//";;" -> ";"
				if(kana.empty() && ch >= L'\x20')
				{
					kana.push_back(ch);
					_HandleCharReturn(ec, pContext);
				}
				return S_OK;
			}
			//"n;" -> "ん▽"
			if(_ConvShift(WCHAR_MAX))
			{
				ch = L'\0';
			}
		}
	}

	//機能処理
	if(_HandleControl(ec, pContext, sf, ch) == S_OK)
	{
		if(pContext != NULL && !iscomp && _IsKeyVoid(ch, (BYTE)wParam))
		{
			_UpdateLanguageBar();
		}
		return S_OK;
	}

	if(pContext != NULL && !iscomp && _IsKeyVoid(ch, (BYTE)wParam))
	{
		_UpdateLanguageBar();
		return S_OK;
	}

	//変換位置指定
	if(ch != L'\0')
	{
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
		case im_katakana_ank:
			if(!abbrevmode || showentry)
			{
				if(hrc != E_ABORT)
				{
					break;
				}

				auto vs_itr = std::lower_bound(conv_point_s.begin(), conv_point_s.end(),
					ch, [] (CONV_POINT m, WCHAR v) { return (m.ch[0] < v); });

				if(vs_itr != conv_point_s.end() && ch == vs_itr->ch[0])
				{
					ch = vs_itr->ch[1];
					if(!inputkey || !kana.empty())
					{
						chO = vs_itr->ch[2];
						if(_HandleControl(ec, pContext, SKK_CONV_POINT, ch) == S_OK)
						{
							return S_OK;
						}
					}
				}
				else
				{
					auto va_itr = std::lower_bound(conv_point_a.begin(), conv_point_a.end(),
						ch, [] (CONV_POINT m, WCHAR v) { return (m.ch[1] < v); });

					if(va_itr != conv_point_a.end() && ch == va_itr->ch[1])
					{
						chO = va_itr->ch[2];
					}
				}
			}
			break;
		default:
			break;
		}
	}

	if(ch >= L'\x20')
	{
		std::wstring romanN = roman;
		WCHAR chON = chO;

		//2文字目以降のローマ字で変換位置指定
		if(!roman.empty() && chO != L'\0')
		{
			auto va_itr = std::lower_bound(conv_point_a.begin(), conv_point_a.end(),
				roman[0], [] (CONV_POINT m, WCHAR v) { return (m.ch[1] < v); });

			if(va_itr != conv_point_a.end() && roman[0] == va_itr->ch[1])
			{
				chO = va_itr->ch[2];
			}
		}

		//文字処理
		if(_HandleChar(ec, pContext, wParam, ch, chO) == E_ABORT)
		{
			switch(inputmode)
			{
			case im_hiragana:
			case im_katakana:
			case im_katakana_ank:
				if(!abbrevmode && !romanN.empty())
				{
					//「ん」または待機中の文字を送り出し
					roman = romanN;
					if(_ConvShift(WCHAR_MAX))
					{
						if(!inputkey)
						{
							_HandleCharShift(ec, pContext);
						}
						else
						{
							_Update(ec, pContext);
						}
					}
					else
					{
						roman.clear();
					}
					//最後の入力で再処理
					chO = chON;
					if(_HandleChar(ec, pContext, wParam, ch, chO) == E_ABORT)
					{
						if(!inputkey)
						{
							_HandleCharReturn(ec, pContext);
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}

	return S_OK;
}

void CTextService::_KeyboardOpenCloseChanged(BOOL showinputmode)
{
	BOOL fOpen = _IsKeyboardOpen();
	if(fOpen)
	{
		_StartManager();

		_ResetStatus();

		_LoadDisplayAttr();
		_LoadBehavior();
		_LoadSelKey();

		_UninitPreservedKey();
		_LoadPreservedKey();
		_InitPreservedKey();

		_LoadCKeyMap(SectionKeyMap);
		_LoadVKeyMap(SectionVKeyMap);
		_LoadConvPoint();
		_LoadKana();
		_LoadJLatin();

		_GetActiveFlags();

		//OnPreservedKey(),CLangBarItemButton::OnClick()経由ならひらがなモード
		//それ以外なら現在のモード
		switch(inputmode)
		{
		case im_disable:
			inputmode = im_hiragana;
			break;
		default:
			_KeyboardInputConversionChanged();
			break;
		}

		_InitFont();
	}
	else
	{
		_UninitFont();

		inputmode = im_default;

		_SaveUserDic();

		_UninitPreservedKey();
		_LoadPreservedKey();
		_InitPreservedKey();

		_ResetStatus();

		_ClearComposition();
	}

	_UpdateLanguageBar(showinputmode);
}

void CTextService::_KeyboardInputConversionChanged()
{
	VARIANT var;
	int inputmode_bak = inputmode;

	if(_GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var) == S_OK)
	{
		if(_IsKeyboardOpen())
		{
			LONG lval = var.lVal & (TF_CONVERSIONMODE_ALPHANUMERIC |
				TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_KATAKANA | TF_CONVERSIONMODE_FULLSHAPE);
			switch(lval)
			{
			case TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_FULLSHAPE:
				inputmode = im_hiragana;
				break;
			case TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_KATAKANA | TF_CONVERSIONMODE_FULLSHAPE:
				inputmode = im_katakana;
				break;
			case TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_KATAKANA:
				inputmode = im_katakana_ank;
				break;
			case TF_CONVERSIONMODE_ALPHANUMERIC | TF_CONVERSIONMODE_FULLSHAPE:
				inputmode = im_jlatin;
				break;
			case TF_CONVERSIONMODE_ALPHANUMERIC:
				inputmode = im_ascii;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		if(!_KeyboardSetDefaultMode())
		{
			var.vt = VT_I4;
			var.lVal = TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_FULLSHAPE | TF_CONVERSIONMODE_ROMAN;
			_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
		}
	}

	if(inputmode != inputmode_bak)
	{
		_ResetStatus();
		_ClearComposition();
		_UpdateLanguageBar();
	}
}

BOOL CTextService::_KeyboardSetDefaultMode()
{
	BOOL open = FALSE;
	BOOL mode = FALSE;
	VARIANT var;

	_ReadBoolValue(SectionBehavior, ValueDefaultMode, open, FALSE);
	if(open)
	{
		_ReadBoolValue(SectionBehavior, ValueDefModeAscii, mode, FALSE);
		if(mode)
		{
			inputmode = im_ascii;
			var.vt = VT_I4;
			var.lVal = TF_CONVERSIONMODE_ALPHANUMERIC;
			_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
		}
		else
		{
			inputmode = im_hiragana;
			var.vt = VT_I4;
			var.lVal = TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_FULLSHAPE | TF_CONVERSIONMODE_ROMAN;
			_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
		}

		if(!_IsKeyboardOpen())
		{
			_SetKeyboardOpen(TRUE);
		}
	}

	return open;
}

BOOL CTextService::_IsKeyVoid(WCHAR ch, BYTE vk)
{
	if(vk < VKEYMAPNUM)
	{
		SHORT vk_shift = GetKeyState(VK_SHIFT) & 0x8000;
		SHORT vk_ctrl = GetKeyState(VK_CONTROL) & 0x8000;
		BYTE k = SKK_NULL;
		if(vk_shift)
		{
			k = vkeymap_shift.keyvoid[vk];
		}
		else if(vk_ctrl)
		{
			k = vkeymap_ctrl.keyvoid[vk];
		}
		else
		{
			k = vkeymap.keyvoid[vk];
		}
		if(k == SKK_VOID)
		{
			return TRUE;
		}
	}

	if(ch < CKEYMAPNUM)
	{
		if(ckeymap.keyvoid[ch] == SKK_VOID)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CTextService::_ResetStatus()
{
	inputkey = FALSE;
	abbrevmode = FALSE;
	showentry = FALSE;
	showcandlist = FALSE;
	complement = FALSE;
	purgedicmode = FALSE;
	hintmode = FALSE;

	searchkey.clear();
	searchkeyorg.clear();

	candidates.clear();
	candidates.shrink_to_fit();
	candidx = 0;

	roman.clear();
	kana.clear();
	okuriidx = 0;

	cursoridx = 0;
}

void CTextService::_GetActiveFlags()
{
	_dwActiveFlags = 0;
	_ImmersiveMode = FALSE;
	_UILessMode = FALSE;

	ITfThreadMgrEx *pThreadMgrEx;
	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pThreadMgrEx)) == S_OK)
	{
		pThreadMgrEx->GetActiveFlags(&_dwActiveFlags);
		SafeRelease(&pThreadMgrEx);
	}

	if((_dwActiveFlags & TF_TMF_IMMERSIVEMODE) != 0)
	{
		_ImmersiveMode = TRUE;
	}

	if((_dwActiveFlags & TF_TMF_UIELEMENTENABLEDONLY) != 0)
	{
		_UILessMode = TRUE;
	}

	_ShowInputMode = !_UILessMode && cx_showmodeinl &&
		(!cx_showmodeimm || (cx_showmodeimm && _ImmersiveMode));
}
