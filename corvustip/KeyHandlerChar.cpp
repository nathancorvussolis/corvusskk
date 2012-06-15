
#include "corvustip.h"
#include "TextService.h"
#include "CandidateList.h"

HRESULT CTextService::_HandleChar(TfEditCookie ec, ITfContext *pContext, WCHAR ch, WCHAR chO)
{
	ROMAN_KANA_CONV rkc;
	ASCII_JLATIN_CONV ajc;
	HRESULT ret = S_OK;

	if(showentry)
	{
		_HandleCharReturn(ec, pContext);
	}

	if(accompidx != 0 && accompidx == kana.size())
	{
		kana.push_back(chO);
	}

	roman.push_back(ch);

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
			//ローマ字仮名変換
			wcscpy_s(rkc.roman, roman.c_str());
			ret = _ConvRomanKana(&rkc);
			switch(ret)
			{
			case S_OK:	//一致
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
					if(!kana.empty() && accompidx != 0 && !rkc.soku && !c_nookuriconv)
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
		wcscpy_s(ajc.ascii, roman.c_str());
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
		kana = roman;
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
