
#include "imcrvtip.h"
#include "TextService.h"
#include "LanguageBar.h"
#include "convtype.h"

HRESULT CTextService::_HandleControl(TfEditCookie ec, ITfContext *pContext, BYTE sf, WCHAR &ch)
{
	size_t i;
	ASCII_JLATIN_CONV ajc;

	switch(sf)
	{
	case SKK_KANA:
		if(abbrevmode)
		{
			break;
		}
		switch(inputmode)
		{
		case im_hiragana:
			if(inputkey && !showentry)
			{
				if(_ConvN(WCHAR_MAX))
				{
					//カタカナに変換
					_ConvKanaToKana(kana, im_katakana, kana, inputmode);
					_HandleCharReturn(ec, pContext);
					return S_OK;
				}
			}
			else
			{
				_ConvN(WCHAR_MAX);
			}
			if(roman.empty())
			{
				//カタカナモードへ
				inputmode = im_katakana;
				_UpdateLanguageBar();
				_HandleCharReturn(ec, pContext);
				return S_OK;
			}
			break;
		case im_katakana:
			if(inputkey && !showentry)
			{
				if(_ConvN(WCHAR_MAX))
				{
					//ひらがなに変換
					_ConvKanaToKana(kana, im_hiragana, kana, inputmode);
					_HandleCharReturn(ec, pContext);
					return S_OK;
				}
			}
			else
			{
				_ConvN(WCHAR_MAX);
			}
			if(roman.empty())
			{
				//ひらがなモードへ
				inputmode = im_hiragana;
				_UpdateLanguageBar();
				_HandleCharReturn(ec, pContext);
				return S_OK;
			}
			break;
		default:
			break;
		}
		break;

	case SKK_CONV_CHAR:
		if(abbrevmode)
		{
			//全英に変換
			roman = kana;
			kana.clear();
			cursoridx = 0;
			for(i=0; i<roman.size(); i++)
			{
				ajc.ascii[0] = roman[i];
				ajc.ascii[1] = L'\0';
				if(_ConvAsciiJLatin(&ajc) == S_OK)
				{
					kana.insert(cursoridx, ajc.jlatin);
					cursoridx += wcslen(ajc.jlatin);
				}
			}
			_HandleCharReturn(ec, pContext);
			return S_OK;
			break;
		}
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			if(inputkey && !showentry)
			{
				if(_ConvN(WCHAR_MAX))
				{
					//半角ｶﾀｶﾅに変換
					_ConvKanaToKana(kana, im_katakana_ank, kana, inputmode);
					_HandleCharReturn(ec, pContext);
					return S_OK;
				}
			}
			break;
		default:
			break;
		}
		break;

	case SKK_JLATIN:
		if(abbrevmode)
		{
			break;
		}
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			_ConvN(WCHAR_MAX);
			if(roman.empty())
			{
				//全英モードへ
				inputmode = im_jlatin;
				_UpdateLanguageBar();
				_HandleCharReturn(ec, pContext);
				return S_OK;
			}
			break;
		default:
			break;
		}
		break;

	case SKK_ASCII:
		if(abbrevmode)
		{
			break;
		}
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			_ConvN(WCHAR_MAX);
			if(roman.empty())
			{
				//アスキーモードへ
				inputmode = im_ascii;
				_UpdateLanguageBar();
				_HandleCharReturn(ec, pContext);
				return S_OK;
			}
			break;
		default:
			break;
		}
		break;

	case SKK_JMODE:
		switch(inputmode)
		{
		case im_jlatin:
		case im_ascii:
			//ひらがなモードへ
			inputmode = im_hiragana;
			_UpdateLanguageBar();
			break;
		default:
			break;
		}
		return S_OK;
		break;

	case SKK_ABBREV:
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			_ConvN(WCHAR_MAX);
			if((!inputkey && !abbrevmode && roman.empty()) || showentry)
			{
				_HandleCharReturn(ec, pContext);
				//見出し入力開始(abbrev)
				inputkey = TRUE;
				abbrevmode = TRUE;
				_Update(ec, pContext);
				return S_OK;
			}
			break;
		default:
			break;
		}
		break;

	case SKK_AFFIX:
		if(abbrevmode && !showentry)
		{
			break;
		}
		if(showentry || (inputkey && kana.empty() && roman.empty()))
		{
			if(showentry)
			{
				_HandleCharReturn(ec, pContext);
			}
			//見出し入力開始(接尾辞)
			inputkey = TRUE;
			ch = L'>';
			kana.push_back(ch);
			cursoridx++;
			_Update(ec, pContext);
			return S_OK;
		}

		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			if(!inputkey)
			{
				break;
			}
			if(accompidx != 0)
			{
				return S_OK;
			}
			if(!_ConvN(WCHAR_MAX))
			{
				roman.clear();
			}
			if(kana.empty())
			{
				_Update(ec, pContext);
				return S_OK;
			}

			ch = L'>';
			roman.clear();
			kana.push_back(ch);
			cursoridx = kana.size();
			if(!c_nookuriconv)
			{
				//辞書検索開始(接頭辞)
				showentry = TRUE;
				_StartConv();
			}
			_Update(ec, pContext);
			return S_OK;
			break;
		default:
			break;
		}
		break;

	case SKK_NEXT_CAND:
		if(showentry)
		{
			_NextConv();
			_Update(ec, pContext);
		}
		else if(inputkey)
		{
			if(!_ConvN(WCHAR_MAX))
			{
				roman.clear();
			}
			if(accompidx != 0)
			{
				if(accompidx + 1 == cursoridx)
				{
					kana.erase(cursoridx - 1, 1);
					cursoridx--;
					accompidx = 0;
				}
				if(accompidx == kana.size())
				{
					accompidx = 0;
				}
			}
			if(kana.empty())
			{
				_Update(ec, pContext);
				return S_OK;
			}
			//候補表示開始
			showentry = TRUE;
			_StartConv();
			_Update(ec, pContext);
		}
		else
		{
			return E_PENDING;
		}
		return S_OK;
		break;

	case SKK_PREV_CAND:
		if(showentry)
		{
			_PrevConv();
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_PURGE_DIC:
		if(showentry)
		{
			//数値変換タイプ0～3の候補は数値を#にした見出し語 それ以外は見出し語そのまま
			_DelUserDic((accompidx == 0 ? REQ_USER_DEL_1 : REQ_USER_DEL_0),
				(std::regex_match(candidates[candidx].second.first, std::wregex(L".*#[0-3].*")) ?
					searchkey : searchkeyorg),
				candidates[candidx].second.first);

			showentry = FALSE;
			candidx = 0;
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_NEXT_COMP:
		if(inputkey && !showentry)
		{
			_ConvN(WCHAR_MAX);
			_NextComp();
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_PREV_COMP:
		if(inputkey && !showentry)
		{
			_PrevComp();
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_CONV_POINT:
		if(abbrevmode && !showentry)
		{
			break;
		}
		if(showentry)
		{
			_HandleCharReturn(ec, pContext);
		}

		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			if(!inputkey)
			{
				if(_ConvN(ch))
				{
					if(!kana.empty())
					{
						_HandleCharReturn(ec, pContext);
					}
					//見出し入力開始
					inputkey = TRUE;
					_Update(ec, pContext);
				}
			}
			else
			{
				if(_ConvN(ch) && accompidx == 0)
				{
					//送り仮名入力開始
					if(cursoridx == kana.size())
					{
						accompidx = cursoridx;
					}
					else if(cursoridx != 0)
					{
						kana.insert(cursoridx, 1, (roman.empty() ? ch : roman[0]));
						accompidx = cursoridx;
						cursoridx++;
					}
					_Update(ec, pContext);
				}
			}
			if(ch == L'\0')
			{
				return S_OK;
			}
			break;
		default:
			break;
		}
		break;

	case SKK_DIRECT:
		if(inputkey && !showentry)
		{
			if(roman.empty())
			{
				kana.insert(cursoridx, 1, ch);
				cursoridx++;
				_Update(ec, pContext);
				return S_OK;
			}
		}
		break;

	case SKK_ENTER:
		_ConvN(WCHAR_MAX);
		_HandleCharReturn(ec, pContext, (_GetSf(0, ch) == SKK_BACK ? TRUE : FALSE));
		return S_OK;
		break;

	case SKK_CANCEL:
		if(showentry)
		{
			if(c_delokuricncl && accompidx != 0)
			{
				kana = kana.substr(0, accompidx);
				accompidx = 0;
				cursoridx = kana.size();
			}
			showentry = FALSE;
			_Update(ec, pContext);
		}
		else
		{
			kana.clear();
			cursoridx = 0;
			_HandleCharReturn(ec, pContext);
		}
		return S_OK;
		break;

	case SKK_BACK:
		if(showentry)
		{
			if(_HandleControl(ec, pContext, (c_backincenter ? SKK_ENTER : SKK_PREV_CAND), ch) == S_OK)
			{
				return S_OK;
			}
		}
		if(inputkey && roman.empty() && kana.empty())
		{
			_HandleCharReturn(ec, pContext);
			return S_OK;
		}
		if(roman.empty() && accompidx != 0 && accompidx == kana.size())
		{
			accompidx = 0;
			_Update(ec, pContext);
			return S_OK;
		}
		if(!roman.empty())
		{
			roman.pop_back();
		}
		else
		{
			if(!kana.empty())
			{
				// surrogate pair
				if(cursoridx >= 2 && IS_SURROGATE_PAIR(kana[cursoridx - 2], kana[cursoridx - 1]))
				{
					kana.erase(cursoridx - 2, 2);
					cursoridx -= 2;
					if(cursoridx < accompidx)
					{
						accompidx -= 2;
					}
				}
				else if(cursoridx >= 1)
				{
					kana.erase(cursoridx - 1, 1);
					cursoridx--;
					if(accompidx != 0 && cursoridx < accompidx)
					{
						accompidx--;
						if(accompidx == 0)
						{
							kana.erase(0, 1);
						}
					}
				}
			}
		}
		if(accompidx != 0 && accompidx + 1 == cursoridx)
		{
			accompidx = 0;
			kana.erase(cursoridx - 1, 1);
			cursoridx--;
		}

		_Update(ec, pContext);

		if(!inputkey && roman.empty() && kana.empty())
		{
			_HandleCharReturn(ec, pContext);
		}
		return S_OK;
		break;

	case SKK_DELETE:
		if(inputkey && !showentry)
		{
			if(!kana.empty())
			{
				if(accompidx != 0 && accompidx == cursoridx)
				{
					kana.erase(cursoridx, 1);
					accompidx = 0;
				}
				// surrogate pair
				if(kana.size() - cursoridx >= 2 && IS_SURROGATE_PAIR(kana[cursoridx], kana[cursoridx + 1]))
				{
					kana.erase(cursoridx, 2);
					if(accompidx >= 2 && cursoridx < accompidx)
					{
						accompidx -= 2;
						if(accompidx == 0)
						{
							kana.erase(cursoridx, 1);
						}
					}
				}
				else
				{
					kana.erase(cursoridx, 1);
					if(accompidx >= 1 && cursoridx < accompidx)
					{
						accompidx--;
						if(accompidx == 0)
						{
							kana.erase(cursoridx, 1);
						}
					}
				}
			}
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_VOID:
		return S_OK;
		break;

	case SKK_LEFT:
		if(inputkey && !showentry)
		{
			if(!_ConvN(WCHAR_MAX))
			{
				roman.clear();
			}
			if(accompidx != 0)
			{
				if(accompidx + 1 == cursoridx)
				{
					kana.erase(cursoridx - 1, 1);
					cursoridx--;
					accompidx = 0;
				}
				if(accompidx == kana.size())
				{
					accompidx = 0;
				}
			}
			if(kana.empty())
			{
				_Update(ec, pContext);
				return S_OK;
			}
			if(cursoridx > 0)
			{
				// surrogate pair
				if(cursoridx >= 2 && IS_SURROGATE_PAIR(kana[cursoridx - 2], kana[cursoridx - 1]))
				{
					cursoridx -= 2;
				}
				else
				{
					cursoridx--;
				}
				if(accompidx != 0 && accompidx + 1 == cursoridx)
				{
					cursoridx--;
				}
				_Update(ec, pContext);
				return S_OK;
			}
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_UP:
		if(inputkey && !showentry)
		{
			if(!_ConvN(WCHAR_MAX))
			{
				roman.clear();
			}
			if(accompidx != 0)
			{
				if(accompidx + 1 == cursoridx)
				{
					kana.erase(cursoridx - 1, 1);
					cursoridx--;
					accompidx = 0;
				}
				if(accompidx == kana.size())
				{
					accompidx = 0;
				}
			}
			if(kana.empty())
			{
				_Update(ec, pContext);
				return S_OK;
			}
			cursoridx = 0;
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_RIGHT:
		if(inputkey && !showentry)
		{
			if(!_ConvN(WCHAR_MAX))
			{
				roman.clear();
			}
			if(accompidx != 0)
			{
				if(accompidx + 1 == cursoridx)
				{
					kana.erase(cursoridx - 1, 1);
					cursoridx--;
					accompidx = 0;
				}
				if(accompidx == kana.size())
				{
					accompidx = 0;
				}
			}
			if(kana.empty())
			{
				_Update(ec, pContext);
				return S_OK;
			}
			if(cursoridx < kana.size())
			{
				// surrogate pair
				if(kana.size() - cursoridx >= 2 && IS_SURROGATE_PAIR(kana[cursoridx], kana[cursoridx + 1]))
				{
					cursoridx += 2;
				}
				else
				{
					cursoridx++;
				}
				if(accompidx != 0 && accompidx + 1 == cursoridx)
				{
					cursoridx++;
				}
			}
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_DOWN:
		if(inputkey && !showentry)
		{
			if(!_ConvN(WCHAR_MAX))
			{
				roman.clear();
			}
			if(accompidx != 0)
			{
				if(accompidx + 1 == cursoridx)
				{
					kana.erase(cursoridx - 1, 1);
					cursoridx--;
					accompidx = 0;
				}
				if(accompidx == kana.size())
				{
					accompidx = 0;
				}
			}
			if(kana.empty())
			{
				_Update(ec, pContext);
				return S_OK;
			}
			cursoridx = kana.size();
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_PASTE:
		if(inputkey && !showentry)
		{
			if(IsClipboardFormatAvailable(CF_UNICODETEXT))
			{
				HANDLE hCB;
				PWCHAR pwCB;
				if(OpenClipboard(NULL))
				{
					hCB = GetClipboardData(CF_UNICODETEXT);
					if(hCB != NULL)
					{
						pwCB = (PWCHAR)GlobalLock(hCB);
						if(pwCB != NULL)
						{
							if(!_ConvN(WCHAR_MAX))
							{
								roman.clear();
							}
							std::wstring s = pwCB;
							s = std::regex_replace(s, std::wregex(L"\t|\r|\n|\x20"), std::wstring(L""));
							kana.insert(cursoridx, s);
							cursoridx += s.size();
							_Update(ec, pContext);
							GlobalUnlock(hCB);
						}
					}
					CloseClipboard();
				}
			}
		}
		break;

	default:
		break;
	}

	return E_PENDING;
}
