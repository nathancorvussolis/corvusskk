
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"

HRESULT CTextService::_HandleChar(TfEditCookie ec, ITfContext *pContext, std::wstring &composition, WCHAR ch, WCHAR chO)
{
	ROMAN_KANA_CONV rkc;
	ASCII_JLATIN_CONV ajc;
	HRESULT ret = S_OK;
	std::wstring roman_conv;

	if(showentry)
	{
		_Update(ec, pContext, composition, TRUE);
		if(pContext == NULL)	//辞書登録用
		{
			composition.clear();
		}
		_ResetStatus();
		_HandleCharReturn(ec, pContext);
	}

	if(accompidx != 0 && accompidx == kana.size() && chO != L'\0')
	{
		kana.insert(cursoridx, 1, chO);
		cursoridx++;
	}

	switch(inputmode)
	{
	case im_hiragana:
	case im_katakana:
		if(abbrevmode)
		{
			_HandleCharTerminate(ec, pContext, composition);
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

					_HandleCharTerminate(ec, pContext, composition);
					_Update(ec, pContext);
					break;
				}

				switch(inputmode)
				{
				case im_hiragana:
					kana.insert(cursoridx, rkc.hiragana);
					if(accompidx != 0 && cursoridx < accompidx)
					{
						accompidx += wcslen(rkc.hiragana);
					}
					cursoridx += wcslen(rkc.hiragana);
					break;
				case im_katakana:
					kana.insert(cursoridx, rkc.katakana);
					if(accompidx != 0 && cursoridx < accompidx)
					{
						accompidx += wcslen(rkc.katakana);
					}
					cursoridx += wcslen(rkc.katakana);
					break;
				default:
					break;
				}

				roman.clear();

				if(!inputkey)
				{
					_HandleCharTerminate(ec, pContext, composition);	//候補＋仮名
					if(composition.empty())
					{
						_HandleCharReturn(ec, pContext);	//仮名のみ
					}
					kana.clear();
					cursoridx = 0;
					if(rkc.soku)
					{
						roman.push_back(ch);
						_Update(ec, pContext);
					}
				}
				else
				{
					_HandleCharTerminate(ec, pContext, composition);
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
				_HandleCharTerminate(ec, pContext, composition);
				roman.push_back(ch);
				_Update(ec, pContext);
				break;
			
			case E_ABORT:	//不一致
				_HandleCharTerminate(ec, pContext, composition);
				roman.clear();
				if(accompidx != 0 && accompidx + 1 == cursoridx)
				{
					kana.erase(cursoridx - 1, 1);	//送りローマ字削除
					cursoridx--;
					if(accompidx != kana.size())
					{
						accompidx = 0;
					}
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

HRESULT CTextService::_HandleCharTerminate(TfEditCookie ec, ITfContext *pContext, std::wstring &composition)
{
	if(!composition.empty())
	{
		_Update(ec, pContext, composition, TRUE);
		_TerminateComposition(ec, pContext);
	}

	return S_OK;
}
