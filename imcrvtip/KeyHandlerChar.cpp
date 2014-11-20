
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"

HRESULT CTextService::_HandleChar(TfEditCookie ec, ITfContext *pContext, WPARAM wParam, WCHAR ch, WCHAR chO)
{
	ROMAN_KANA_CONV rkc;
	ASCII_JLATIN_CONV ajc;
	HRESULT ret = S_OK;

	if(showentry)
	{
		_HandleCharShift(ec, pContext);
	}

	if((okuriidx != 0) && (okuriidx + 1 == cursoridx))
	{
		if(chO != L'\0')
		{
			kana.replace(okuriidx, 1, 1, chO);	//送りローマ字
		}
	}

	switch(inputmode)
	{
	case im_hiragana:
	case im_katakana:
	case im_katakana_ank:
		if(abbrevmode)
		{
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
			case E_PENDING:	//途中まで一致
				if(rkc.roman[0] != L'\0' && rkc.wait)	//待機
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
			std::wstring roman_conv = roman;
			if(ch != L'\0')
			{
				roman_conv.push_back(ch);
			}
			wcsncpy_s(rkc.roman, roman_conv.c_str(), _TRUNCATE);
			ret = _ConvRomanKana(&rkc);

			//Windows8以降のタッチキーボード
			if(ret == E_ABORT && wParam == VK_PACKET &&
				(ch != TKB_NEXT_PAGE && ch != TKB_PREV_PAGE) &&
				(ch != L'次' && ch != L'前' && ch != L'頁'))
			{
				rkc.roman[0] = rkc.hiragana[0] = rkc.katakana[0] = rkc.katakana_ank[0] = ch;
				rkc.roman[1] = rkc.hiragana[1] = rkc.katakana[1] = rkc.katakana_ank[1] = L'\0';
				rkc.soku = FALSE;
				rkc.wait = FALSE;
				ret = S_OK;
			}

			switch(ret)
			{
			case S_OK:	//一致
				if(rkc.roman[0] != L'\0' && rkc.wait)	//待機
				{
					_HandleCharShift(ec, pContext);

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

					_Update(ec, pContext);
				}
				else
				{
					std::wstring kana_ins;
					switch(inputmode)
					{
					case im_hiragana:
						kana_ins = rkc.hiragana;
						break;
					case im_katakana:
						kana_ins = rkc.katakana;
						break;
					case im_katakana_ank:
						kana_ins = rkc.katakana_ank;
						break;
					default:
						break;
					}

					if(!kana_ins.empty())
					{
						kana.insert(cursoridx, kana_ins);
						if(okuriidx != 0 && cursoridx <= okuriidx)
						{
							okuriidx += kana_ins.size();
						}
						cursoridx += kana_ins.size();
					}

					roman.clear();

					if(inputkey)
					{
						_HandleCharShift(ec, pContext);
						if(cx_begincvokuri && !hintmode &&
							!kana.empty() && okuriidx != 0 && !rkc.soku && !rkc.wait)
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
						_HandleCharShift(ec, pContext);
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
				}
				break;

			case E_PENDING:	//途中まで一致
				if(rkc.roman[0] != L'\0' && rkc.wait)	//待機
				{
					_HandleCharShift(ec, pContext);

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

					_Update(ec, pContext);
				}
				else
				{
					_HandleCharShift(ec, pContext);
					roman.push_back(ch);
					_Update(ec, pContext);
				}
				break;

			case E_ABORT:	//不一致
				_HandleCharShift(ec, pContext);
				roman.clear();
				if(okuriidx != 0 && okuriidx + 1 == cursoridx)
				{
					kana.replace(okuriidx, 1, 1, CHAR_SKK_OKURI);	//送りローマ字
				}
				_Update(ec, pContext);
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
	if(showentry || (!inputkey && !kana.empty() && roman.empty()))
	{
		//leave composition
		cursoridx = kana.size();
		_Update(ec, pContext, TRUE);
		if(pContext != NULL)
		{
			ITfRange *pRange;
			if(_IsComposing() && _pComposition->GetRange(&pRange) == S_OK)
			{
				pRange->Collapse(ec, TF_ANCHOR_END);
				_pComposition->ShiftStart(ec, pRange);
				pRange->Release();
			}
		}
		_ResetStatus();
	}

	return S_OK;
}
