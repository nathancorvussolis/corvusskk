
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
		return _pTextService->_HandleKey(ec, _pContext, _wParam, _bSf);
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
	WCHAR ch;
	WCHAR chO;
	std::wstring romanN;
	std::wstring composition;	//二重確定防止
	
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

	chO = L'\0';

	if(ch == L'\0' && sf == SKK_NULL)
	{
		return S_FALSE;
	}
	
	switch(sf)
	{
	case SKK_NEXT_COMP:
	case SKK_PREV_COMP:
		break;
	default:
		complement = FALSE;	//補完終了
		break;
	}
	
	if(_HandleControl(ec, pContext, sf, ch) == S_OK)
	{
		return S_OK;
	}

	if(ch != L'\0')
	{
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			if(!abbrevmode || showentry)
			{
				for(i=0; i<CONV_POINT_NUM; i++)
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
		if(!roman.empty() && chO != L'\0')
		{
			chO = roman[0];
		}
		romanN = roman;
		if(_HandleChar(ec, pContext, composition, ch, chO) == E_ABORT)
		{
			//待機処理等
			switch(inputmode)
			{
			case im_hiragana:
			case im_katakana:
				if(!abbrevmode && !romanN.empty())
				{
					roman = romanN;
					if(_ConvN(WCHAR_MAX))
					{
						if(!inputkey)
						{
							_Update(ec, pContext, composition, TRUE);
							if(pContext == NULL)	//辞書登録用
							{
								composition.clear();
							}
							_ResetStatus();
							_HandleCharReturn(ec, pContext);
						}
						else
						{
							_Update(ec, pContext);
						}
						_HandleChar(ec, pContext, composition, ch, chO);
					}
					else
					{
						roman.clear();
						_Update(ec, pContext);
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

void CTextService::_KeyboardChanged()
{
	if(_pThreadMgr == NULL)
	{
		return;
	}

	_dwActiveFlags = 0;
	_ImmersiveMode = FALSE;
	_UILessMode = FALSE;

	ITfThreadMgrEx *pThreadMgrEx;
	if(_pThreadMgr->QueryInterface(IID_ITfThreadMgrEx, (void**)&pThreadMgrEx) == S_OK)
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

	BOOL fOpen = _IsKeyboardOpen();
	if(fOpen)
	{
		//OnPreservedKey()経由ならひらがなモード
		//OnChange()経由なら前回のモード
		switch(exinputmode)
		{
		case im_default:
			inputmode = im_hiragana;
			break;
		default:
			inputmode = exinputmode;
			break;
		}

		_StartManager();

		_ResetStatus();

		_LoadBehavior();
		_LoadSelKey();

		_UninitPreservedKey();
		_LoadPreservedKey();
		_InitPreservedKey();

		_LoadKeyMap(SectionKeyMap, ckeymap);
		_LoadKeyMap(SectionVKeyMap, vkeymap);
		_LoadConvPoint();
		_LoadKana();
		_LoadJLatin();
	}
	else
	{
		exinputmode = inputmode;
		inputmode = im_default;

		if(exinputmode != im_default)
		{
			_SaveUserDic();
		}

		_UninitPreservedKey();
		_LoadPreservedKey();
		_InitPreservedKey();

		_ResetStatus();

		_ClearComposition();
	}

	_UpdateLanguageBar();
}

BOOL CTextService::_IsKeyVoid(WCHAR ch, BYTE vk)
{
	if(ch < KEYMAPNUM)
	{
		if(ckeymap.keyvoid[ch] == SKK_VOID)
		{
			return TRUE;
		}
	}
	if(vk < KEYMAPNUM)
	{
		if(vkeymap.keyvoid[vk] == SKK_VOID)
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

	searchkey.clear();
	searchkeyorg.clear();

	candidates.clear();
	candidates.shrink_to_fit();
	candidx = 0;

	roman.clear();
	kana.clear();
	accompidx = 0;
}
