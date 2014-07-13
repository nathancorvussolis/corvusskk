
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"

HRESULT CTextService::_HandleChar(TfEditCookie ec, ITfContext *pContext, std::wstring &comptext, WPARAM wParam, WCHAR ch, WCHAR chO)
{
	ROMAN_KANA_CONV rkc;
	ASCII_JLATIN_CONV ajc;
	HRESULT ret = S_OK;
	std::wstring roman_conv;

	if(showentry)
	{
		_Update(ec, pContext, comptext, TRUE);
		if(pContext == NULL)	//辞書登録用
		{
			comptext.clear();
		}
		_ResetStatus();
	}

	if((okuriidx != 0) && (okuriidx + 1 == cursoridx) && (chO != L'\0'))
	{
		kana.replace(okuriidx, 1, 1, chO);	//送りローマ字
	}

	switch(inputmode)
	{
	case im_hiragana:
	case im_katakana:
	case im_katakana_ank:
		if(abbrevmode)
		{
			_HandleCharShift(ec, pContext, comptext);
			roman.clear();
			kana.insert(cursoridx, 1, ch);
			cursoridx++;
			_Update(ec, pContext);
		}
		else
		{
			//ローマ字仮名変換 待機処理
			rkc.roman[0] = ch;
			rkc.roman[1] = L'\0';
			ret = _ConvRomanKana(&rkc);
			switch(ret)
			{
			case S_OK:	//一致
				if(rkc.wait)	//待機
				{
					ch = L'\0';
					switch(inputmode)
					{
					case im_hiragana:
						roman.append(rkc.hiragana);
						break;
					case im_katakana:
						roman.append(rkc.katakana);
						break;
					case im_katakana_ank:
						roman.append(rkc.katakana_ank);
						break;
					default:
						break;
					}
				}
				break;
			default:
				break;
			}

			//ローマ字仮名変換
			roman_conv = roman;
			if(ch != L'\0')
			{
				roman_conv.push_back(ch);
			}
			wcsncpy_s(rkc.roman, roman_conv.c_str(), _TRUNCATE);
			ret = _ConvRomanKana(&rkc);

			if(wParam == VK_PACKET && ret == E_ABORT && ch != TKB_NEXT_PAGE && ch != TKB_PREV_PAGE)
			{
				rkc.hiragana[0] = rkc.katakana[0] = rkc.katakana_ank[0] = ch;
				rkc.hiragana[1] = rkc.katakana[1] = rkc.katakana_ank[1] = L'\0';
				rkc.soku = FALSE;
				rkc.wait = FALSE;
				ret = S_OK;
			}

			switch(ret)
			{
			case S_OK:	//一致
				if(rkc.wait)	//待機
				{
					switch(inputmode)
					{
					case im_hiragana:
						roman.assign(rkc.hiragana);
						break;
					case im_katakana:
						roman.assign(rkc.katakana);
						break;
					case im_katakana_ank:
						roman.assign(rkc.katakana_ank);
						break;
					default:
						break;
					}

					_HandleCharShift(ec, pContext, comptext);
					_Update(ec, pContext);
					break;
				}

				switch(inputmode)
				{
				case im_hiragana:
					kana.insert(cursoridx, rkc.hiragana);
					if(okuriidx != 0 && cursoridx <= okuriidx)
					{
						okuriidx += wcslen(rkc.hiragana);
					}
					cursoridx += wcslen(rkc.hiragana);
					break;
				case im_katakana:
					kana.insert(cursoridx, rkc.katakana);
					if(okuriidx != 0 && cursoridx <= okuriidx)
					{
						okuriidx += wcslen(rkc.katakana);
					}
					cursoridx += wcslen(rkc.katakana);
					break;
				case im_katakana_ank:
					kana.insert(cursoridx, rkc.katakana_ank);
					if(okuriidx != 0 && cursoridx <= okuriidx)
					{
						okuriidx += wcslen(rkc.katakana_ank);
					}
					cursoridx += wcslen(rkc.katakana_ank);
					break;
				default:
					break;
				}

				roman.clear();

				if(inputkey)
				{
					_HandleCharShift(ec, pContext, comptext);
					if(!kana.empty() && okuriidx != 0 && !rkc.soku && cx_begincvokuri && !hintmode && !rkc.wait)
					{
						cursoridx = kana.size();
						showentry = TRUE;
						_StartConv();
					}
					else if(rkc.soku)
					{
						roman.push_back(ch);
					}
					_Update(ec, pContext);
				}
				else
				{
					_HandleCharShift(ec, pContext, comptext);	//候補＋仮名
					if(comptext.empty())
					{
						_HandleCharShift(ec, pContext);	//仮名のみ
					}
					kana.clear();
					cursoridx = 0;
					if(rkc.soku)
					{
						roman.push_back(ch);
						_Update(ec, pContext);
					}
					else
					{
						_HandleCharReturn(ec, pContext);
					}
				}
				break;

			case E_PENDING:	//途中まで一致
				_HandleCharShift(ec, pContext, comptext);
				roman.push_back(ch);
				_Update(ec, pContext);
				break;

			case E_ABORT:	//不一致
				_HandleCharShift(ec, pContext, comptext);
				roman.clear();
				if(okuriidx != 0 && okuriidx + 1 == cursoridx)
				{
					kana.replace(okuriidx, 1, 1, CHAR_SKK_OKURI);	//送りローマ字
					if(!cx_keepinputnor)
					{
						kana.erase(okuriidx, 1);
						okuriidx = 0;
						cursoridx--;
					}
				}
				_Update(ec, pContext);
				if(!inputkey)
				{
					//OnCompositionTerminatedを呼ばないアプリの為にコンポジションを終了
					_HandleCharReturn(ec, pContext);
				}
				break;
			default:
				break;
			}
			break;
		}
		break;

	case im_jlatin:
		//ASCII全英変換
		roman.push_back(ch);
		wcsncpy_s(ajc.ascii, roman.c_str(), _TRUNCATE);
		ret = _ConvAsciiJLatin(&ajc);

		if(wParam == VK_PACKET && ret == E_ABORT && ch != TKB_NEXT_PAGE && ch != TKB_PREV_PAGE)
		{
			ajc.jlatin[0] = ch;
			ajc.jlatin[1] = L'\0';
			ret = S_OK;
		}

		switch(ret)
		{
		case S_OK:		//一致
			kana.assign(ajc.jlatin);
			cursoridx = kana.size();
			_HandleCharReturn(ec, pContext);
			break;
		case E_PENDING:	//途中まで一致
		case E_ABORT:	//不一致
			roman.clear();
			_HandleCharReturn(ec, pContext);
			break;
		default:
			break;
		}
		break;

	case im_ascii:	//かなキーロックONのときのみ
		ajc.ascii[0] = ch;
		ajc.ascii[1] = L'\0';
		kana.assign(ajc.ascii);
		cursoridx = kana.size();
		_HandleCharReturn(ec, pContext);
		break;

	default:
		break;
	}

	return ret;
}

HRESULT CTextService::_HandleCharReturn(TfEditCookie ec, ITfContext *pContext, BOOL back)
{
	//terminate composition
	cursoridx = kana.size();
	_Update(ec, pContext, TRUE, back);
	_TerminateComposition(ec, pContext);
	_ResetStatus();

	return S_OK;
}

HRESULT CTextService::_HandleCharShift(TfEditCookie ec, ITfContext *pContext)
{
	std::wstring comptext;

	_Update(ec, pContext, comptext, TRUE);
	_ResetStatus();
	if(pContext != NULL)
	{
		_HandleCharShift(ec, pContext, comptext);
	}

	return S_OK;
}

HRESULT CTextService::_HandleCharShift(TfEditCookie ec, ITfContext *pContext, std::wstring &comptext)
{
	ITfRange *pRange;

	if(!comptext.empty())
	{
		//leave composition
		cursoridx = kana.size();
		_Update(ec, pContext, comptext, TRUE);
		if(_IsComposing() && _pComposition->GetRange(&pRange) == S_OK)
		{
			pRange->Collapse(ec, TF_ANCHOR_END);
			_pComposition->ShiftStart(ec, pRange);
			pRange->Release();
		}
	}

	return S_OK;
}
