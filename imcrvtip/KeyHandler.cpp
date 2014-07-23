
#include "configxml.h"
#include "imcrvtip.h"
#include "EditSession.h"
#include "TextService.h"
#include "LanguageBar.h"
#include "CandidateList.h"

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
	CKeyHandlerEditSession *pEditSession;
	HRESULT hr = E_FAIL;

	pEditSession = new CKeyHandlerEditSession(this, pContext, wParam, bSf);
	if(pEditSession != NULL)
	{
		hr = pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
		pEditSession->Release();
	}

	return hr;
}

HRESULT CTextService::_HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam, BYTE bSf)
{
	size_t i;
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
			_Update(ec, pContext);
			return S_OK;
		}
		break;
	default:
		if(complement)
		{
			complement = FALSE;	//補完終了
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
			case L'Y': case 'y':
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
		if(!abbrevmode && !roman.empty())
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
		if(!abbrevmode)
		{
			if(inputkey && roman.empty() && kana.empty())
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
			if(_ConvN(WCHAR_MAX))
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

				for(i = 0; i < CONV_POINT_NUM; i++)
				{
					if(conv_point[i][0] == L'\0' &&
						conv_point[i][1] == L'\0' &&
						conv_point[i][2] == L'\0')
					{
						break;
					}
					if(ch == conv_point[i][0])
					{
						ch = conv_point[i][1];
						if(!inputkey || !kana.empty())
						{
							chO = conv_point[i][2];
							if(_HandleControl(ec, pContext, SKK_CONV_POINT, ch) == S_OK)
							{
								return S_OK;
							}
						}
						break;
					}
					else if(ch == conv_point[i][1])
					{
						chO = conv_point[i][2];
						break;
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

		//2文字目以降のローマ字で変換位置指定
		if(!roman.empty() && chO != L'\0')
		{
			chO = roman[0];
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
					if(_ConvN(WCHAR_MAX))
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
	if(_pThreadMgr == NULL)
	{
		return;
	}

	_dwActiveFlags = 0;
	_ImmersiveMode = FALSE;
	_UILessMode = FALSE;

	ITfThreadMgrEx *pThreadMgrEx;
	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pThreadMgrEx)) == S_OK)
	{
		pThreadMgrEx->GetActiveFlags(&_dwActiveFlags);
		pThreadMgrEx->Release();
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

void CTextService::_InitFont()
{
	LOGFONTW logfont;
	IDWriteGdiInterop *pDWGI = NULL;
	IDWriteFont *pDWFont = NULL;

	if(hFont == NULL)
	{
		HDC hdc = GetDC(NULL);

		logfont.lfHeight = -MulDiv(cx_fontpoint, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		logfont.lfWidth = 0;
		logfont.lfEscapement = 0;
		logfont.lfOrientation = 0;
		logfont.lfWeight = cx_fontweight;
		logfont.lfItalic = cx_fontitalic;
		logfont.lfUnderline = FALSE;
		logfont.lfStrikeOut = FALSE;
		logfont.lfCharSet = SHIFTJIS_CHARSET;
		logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		logfont.lfQuality = PROOF_QUALITY;
		logfont.lfPitchAndFamily = DEFAULT_PITCH;
		wcscpy_s(logfont.lfFaceName, cx_fontname);
		hFont = CreateFontIndirectW(&logfont);

		ReleaseDC(NULL, hdc);
	}

	if(cx_drawapi && !_UILessMode && (_pD2DFactory == NULL))
	{
		_drawtext_option = (IsVersion63AndOver() && cx_colorfont) ?
			D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT : D2D1_DRAW_TEXT_OPTIONS_NONE;

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_pD2DFactory);

		if(hr == S_OK)
		{
			hr = _pD2DFactory->CreateDCRenderTarget(&c_d2dprops, &_pD2DDCRT);
		}

		if(hr == S_OK)
		{
			for(int i = 0; i < DISPLAY_COLOR_NUM; i++)
			{
				hr = _pD2DDCRT->CreateSolidColorBrush(D2D1::ColorF(SWAPRGB(cx_colors[i])), &_pD2DBrush[i]);
				if(hr != S_OK)
				{
					break;
				}
			}
		}

		if(hr == S_OK)
		{
			hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, IID_PUNK_ARGS(&_pDWFactory));
		}

		if(hr == S_OK)
		{
			hr = _pDWFactory->GetGdiInterop(&pDWGI);
		}

		if(hr == S_OK)
		{
			hr = pDWGI->CreateFontFromLOGFONT(&logfont, &pDWFont);
			if(hr != S_OK)
			{
				pDWGI->Release();
			}
		}

		if(hr == S_OK)
		{
			hr = _pDWFactory->CreateTextFormat(cx_fontname, NULL,
				pDWFont->GetWeight(), pDWFont->GetStyle(), pDWFont->GetStretch(),
				(FLOAT)-logfont.lfHeight, L"ja-jp", &_pDWTF);
			pDWFont->Release();
			pDWGI->Release();
		}

		if(hr == S_OK)
		{
			hr = _pDWTF->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		}

		if(hr != S_OK)
		{
			_UninitFont();

			hFont = CreateFontIndirectW(&logfont);
		}
	}
}

void CTextService::_UninitFont()
{
	if(hFont != NULL)
	{
		DeleteObject(hFont);
		hFont = NULL;
	}

	if(_pDWTF != NULL)
	{
		_pDWTF->Release();
		_pDWTF = NULL;
	}

	if(_pDWFactory != NULL)
	{
		_pDWFactory->Release();
		_pDWFactory = NULL;
	}

	for(int i = 0; i < DISPLAY_COLOR_NUM; i++)
	{
		if(_pD2DBrush[i] != NULL)
		{
			_pD2DBrush[i]->Release();
			_pD2DBrush[i] = NULL;
		}
	}

	if(_pD2DDCRT)
	{
		_pD2DDCRT->Release();
		_pD2DDCRT = NULL;
	}

	if(_pD2DFactory)
	{
		_pD2DFactory->Release();
		_pD2DFactory = NULL;
	}
}
