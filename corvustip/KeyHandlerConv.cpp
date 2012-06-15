
#include "corvustip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "convtype.h"

WCHAR CTextService::_GetCh(WPARAM wParam)
{
	BYTE keystate[256];
	WCHAR szU[4];
	WCHAR u;
	
	GetKeyboardState(keystate);

	switch(inputmode)
	{
	case im_hiragana:
	case im_katakana:
		keystate[VK_CAPITAL] = 0;
	default:
		break;
	}

	int retu = ToUnicode((UINT)wParam, 0, keystate, szU, _countof(szU), 0);
	if(retu != 1)
	{
		u = L'\0';
	}
	else
	{
		u = szU[0];
	}

	return u;
}

BYTE CTextService::_GetSf(WPARAM wParam, WCHAR ch)
{
	BYTE k = SKK_NULL;

	if(ch == L'\0')
	{
		switch(wParam)
		{
		case VK_LEFT:
			k = SKK_LEFT;
			break;
		case VK_UP:
			k = SKK_UP;
			break;
		case VK_RIGHT:
			k = SKK_RIGHT;
			break;
		case VK_DOWN:
			k = SKK_DOWN;
			break;
		case VK_BACK:
			k = SKK_BACK;
			break;
		case VK_DELETE:
			k = SKK_DELETE;
			break;
		default:
			break;
		}
	}
	else if(ch < KEYMAPNUM)
	{
		switch(inputmode)
		{
		case im_ascii:
		case im_jlatin:
			k = keymap_latin[ch];
			break;
		case im_hiragana:
		case im_katakana:
			k = keymap_jmode[ch];
			break;
		default:
			break;
		}
	}

	return k;
}

HRESULT CTextService::_ConvRomanKana(ROMAN_KANA_CONV *pconv)
{
	size_t i;
	HRESULT ret = E_ABORT;	//一致する可能性なし

	for(i=0; i<ROMAN_KANA_TBL_NUM; i++)
	{
		if(roman_kana_conv[i].roman[0] == L'\0' &&
			roman_kana_conv[i].hiragana[0] == L'\0' &&
			roman_kana_conv[i].katakana[0] == L'\0' &&
			roman_kana_conv[i].katakana_ank[0] == L'\0')
		{
			break;
		}
		if(roman_kana_conv[i].roman[0] == L'\0')
		{
			continue;
		}
		if(wcsncmp(roman_kana_conv[i].roman, pconv->roman, wcslen(pconv->roman)) == 0)
		{
			if(wcsncmp(roman_kana_conv[i].roman, pconv->roman, ROMAN_NUM) == 0)
			{
				*pconv = roman_kana_conv[i];
				ret = S_OK;		//一致
				break;
			}
			ret = E_PENDING;	//途中まで一致
		}
	}

	return ret;
}

HRESULT CTextService::_ConvAsciiJLatin(ASCII_JLATIN_CONV *pconv)
{
	size_t i;
	HRESULT ret = E_ABORT;	//一致する可能性なし

	for(i=0; i<ASCII_JLATIN_TBL_NUM; i++)
	{
		if(ascii_jlatin_conv[i].ascii[0] == L'\0' &&
			ascii_jlatin_conv[i].jlatin[0] == L'\0')
		{
			break;
		}
		if(ascii_jlatin_conv[i].ascii[0] == L'\0')
		{
			continue;
		}
		if(wcsncmp(ascii_jlatin_conv[i].ascii, pconv->ascii, wcslen(pconv->ascii)) == 0)
		{
			if(wcsncmp(ascii_jlatin_conv[i].ascii, pconv->ascii, JLATIN_NUM) == 0)
			{
				*pconv = ascii_jlatin_conv[i];
				ret = S_OK;		//一致
				break;
			}
			ret = E_PENDING;	//途中まで一致
		}
	}

	return ret;
}

void CTextService::_StartConv()
{
	CANDIDATES::iterator candidates_itr;
	CANDIDATES candidates_bak;
	CANDIDATES candidates_num;
	std::wstring ascii;
	std::wstring jlatin;
	std::wstring hiragana;
	std::wstring katakana;
	std::wstring katakana_ank;

	searchkey.clear();
	searchkeyorg.clear();

	//仮名を平仮名にして検索
	if(accompidx != 0)
	{
		_ConvKanaToKana(searchkey, im_hiragana, kana.substr(0, accompidx + 1), inputmode);
	}
	else
	{
		_ConvKanaToKana(searchkey, im_hiragana, kana, inputmode);
	}

	candidates.clear();
	candidates.shrink_to_fit();

	//通常検索
	_ConvDic(REQ_SEARCH);

	searchkeyorg = searchkey;	//オリジナルバックアップ

	candidates_bak = candidates;
	candidates.clear();
	candidates.shrink_to_fit();

	//数値変換検索
	_ConvDicNum();

	candidates_num = candidates;
	candidates.clear();
	candidates.shrink_to_fit();

	if(!candidates_bak.empty())
	{
		for(candidates_itr = candidates_bak.begin(); candidates_itr != candidates_bak.end(); candidates_itr++)
		{
			candidates.push_back(*candidates_itr);
		}
	}
	if(!candidates_num.empty())
	{
		for(candidates_itr = candidates_num.begin(); candidates_itr != candidates_num.end(); candidates_itr++)
		{
			candidates.push_back(*candidates_itr);
		}
	}

	candidx = 0;
}

void CTextService::_NextConv()
{
	if(!candidates.empty() && candidx < candidates.size())
	{
		//candidx == candidates.size() となる場合があるが、
		//_Update() で candidx = 0 される
		++candidx;
	}
	else
	{
		if(delokuricncl && accompidx != 0)
		{
			kana = kana.substr(0, accompidx);
			accompidx = 0;
		}
		candidx = 0;
		showentry = FALSE;
	}
}

void CTextService::_PrevConv()
{
	if(candidx > 0)
	{
		--candidx;
	}
	else
	{
		showentry = FALSE;
	}
}

void CTextService::_NextComp()
{
	if(!complement)
	{
		searchkey.clear();
		searchkeyorg.clear();

		if(accompidx == 0)
		{
			if(abbrevmode)
			{
				searchkey = kana;
			}
			else
			{
				_ConvKanaToKana(searchkey, im_hiragana, kana, inputmode);
			}
		}

		if(searchkey.empty())
		{
			return;
		}

		candidates.clear();
		candidates.shrink_to_fit();

		//補完
		_ConvDic(REQ_COMPLEMENT);

		if(!candidates.empty())
		{
			complement = TRUE;
			candidx = 0;
			_SetComp(candidates[candidx].first.first);
		}
	}
	else
	{
		if(candidx >= candidates.size() - 1)
		{
			complement = FALSE;
			_SetComp(searchkey);
		}
		else
		{
			++candidx;
			_SetComp(candidates[candidx].first.first);
		}
	}
}

void CTextService::_PrevComp()
{
	if(complement)
	{
		if(candidx == 0)
		{
			complement = FALSE;
			_SetComp(searchkey);
		}
		else
		{
			--candidx;
			if(candidx < candidates.size())
			{
				_SetComp(candidates[candidx].first.first);
			}
		}
	}
}

void CTextService::_SetComp(const std::wstring &candidate)
{
	kana.clear();

	if(abbrevmode)
	{
		kana = candidate;
	}
	else
	{
		_ConvKanaToKana(kana, inputmode, candidate, im_hiragana);
	}
}

BOOL CTextService::_ConvN(WCHAR ch)
{
	// ( <"n*", ""> -> <"", "ん"> ) or ( <"nn", ""> -> <"", "ん"> )
	ROMAN_KANA_CONV rkc;
	HRESULT ret;
	WCHAR chN;

	if(roman.empty())
	{
		return TRUE;
	}

	if(roman.size() != 1)
	{
		return FALSE;
	}

	chN = roman[0];

	if(ch != WCHAR_MAX)
	{
		// ( <"n*", ""> -> <"", "ん"> )
		rkc.roman[0] = chN;
		rkc.roman[1] = ch;
		rkc.roman[2] = L'\0';
		ret = _ConvRomanKana(&rkc);
		switch(ret)
		{
		case S_OK:		//一致
			if(rkc.soku)	//「n* soku==1」
			{
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
				return TRUE;	//「nk」etc.
			}
			else
			{
				return FALSE;	//「na,ni,nu,ne,no」
			}
			break;
		case E_PENDING:	//途中まで一致
			return FALSE;		//「nya,nyu,nyo」
			break;
		default:
			break;
		}
	}

	// ( <"nn", ""> -> <"", "＊"> )
	if(chN == L'n')
	{
		rkc.roman[0] = chN;
		rkc.roman[1] = chN;
		rkc.roman[2] = L'\0';
		ret = _ConvRomanKana(&rkc);
		switch(ret)
		{
		case S_OK:	//一致
			if(!rkc.soku)	//「nn soku==0」
			{
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
				return TRUE;	//「nn」
			}
			break;
		default:
			break;
		}
	}

	return FALSE;
}

void CTextService::_ConvKanaToKana(std::wstring &dst, int dstmode, const std::wstring &src, int srcmode)
{
	size_t i, j;
	BOOL exist;
	WCHAR *convkana;
	WCHAR srckana[2];
	std::wstring dsttmp;

	switch(srcmode)
	{
	case im_hiragana:
	case im_katakana:
		break;
	default:
		return;
		break;
	}
	switch(dstmode)
	{
	case im_hiragana:
	case im_katakana:
	case im_katakana_ank:
		break;
	default:
		return;
		break;
	}

	for(i=0; i<src.size(); i++)
	{
		srckana[0] = src[i];
		srckana[1] = L'\0';

		exist = FALSE;

		for(j=0; j<ROMAN_KANA_TBL_NUM; j++)
		{
			if(roman_kana_conv[j].roman[0] == L'\0')
			{
				break;
			}

			switch(srcmode)
			{
			case im_hiragana:
				convkana = roman_kana_conv[j].hiragana;
				break;
			case im_katakana:
				convkana = roman_kana_conv[j].katakana;
				break;
			default:
				break;
			}

			if(wcsncmp(convkana, srckana, KANA_NUM) == 0)
			{
				exist = TRUE;
				switch(dstmode)
				{
				case im_hiragana:
					dsttmp.append(roman_kana_conv[j].hiragana);
					break;
				case im_katakana:
					dsttmp.append(roman_kana_conv[j].katakana);
					break;
				case im_katakana_ank:
					dsttmp.append(roman_kana_conv[j].katakana_ank);
					break;
				default:
					break;
				}
				break;
			}
		}

		if(!exist)	//ローマ字仮名変換表に無ければそのまま
		{
			dsttmp.append(srckana);
		}
	}

	dst.assign(dsttmp);
}
