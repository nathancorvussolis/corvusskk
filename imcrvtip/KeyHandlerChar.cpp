
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"

HRESULT CTextService::_HandleChar(TfEditCookie ec, ITfContext *pContext, WCHAR ch, WCHAR chO)
{
	ROMAN_KANA_CONV rkc;
	ASCII_JLATIN_CONV ajc;
	HRESULT ret = S_OK;
	std::wstring roman_conv;

	if(showentry)
	{
		_HandleCharReturn(ec, pContext);
	}

	if(accompidx != 0 && accompidx == kana.size() && chO != L'\0')
	{
		kana.push_back(chO);
	}

	switch(inputmode)
	{
	case im_hiragana:
	case im_katakana:
		if(abbrevmode)
		{
			roman.clear();
			kana.push_back(ch);
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
					default:
						break;
					}

					_Update(ec, pContext);
					break;
				}

				switch(inputmode)
				{
				case im_hiragana:
					kana.append(rkc.hiragana);
					break;
				case im_katakana:
					kana.append(rkc.katakana);
					break;
				default:
					break;
				}

				roman.clear();

				if(!inputkey)
				{
					_Update(ec, pContext, TRUE);
					_TerminateComposition(ec, pContext);
					kana.clear();
					if(rkc.soku)
					{
						roman.push_back(ch);
						_Update(ec, pContext);
					}
				}
				else
				{
					if(!kana.empty() && accompidx != 0 && !rkc.soku && !c_nookuriconv && !rkc.wait)
					{
						showentry = TRUE;
						_StartConv();
					}
					else if(rkc.soku)
					{
						roman.push_back(ch);
					}
					_Update(ec, pContext);
				}
				break;
			
			case E_PENDING:	//途中まで一致
				roman.push_back(ch);
				_Update(ec, pContext);
				break;
			
			case E_ABORT:	//不一致
				roman.clear();
				if(accompidx != 0 && accompidx + 1 == kana.size())
				{
					kana.pop_back();	//送りローマ字削除
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
		switch(ret)
		{
		case S_OK:		//一致
			kana.assign(ajc.jlatin);
			_HandleCharReturn(ec, pContext);
			break;
		case E_PENDING:	//途中まで一致
		case E_ABORT:	//不一致
			roman.clear();
			_HandleCharReturn(ec, pContext);
			break;
		}
		break;

	case im_ascii:
		kana.push_back(ch);
		_HandleCharReturn(ec, pContext);
		break;

	default:
		break;
	}

	return ret;
}

HRESULT CTextService::_HandleCharReturn(TfEditCookie ec, ITfContext *pContext, BOOL back)
{
	_Update(ec, pContext, TRUE, back);
	_TerminateComposition(ec, pContext);
	_ResetStatus();

	return S_OK;
}
