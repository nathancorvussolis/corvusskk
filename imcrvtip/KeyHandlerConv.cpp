
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "convtype.h"

WCHAR CTextService::_GetCh(BYTE vk, BYTE vkoff)
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
		if(abbrevmode)
		{
			keystate[VK_KANA] = 0;
		}
		if(vkoff != 0)
		{
			keystate[vkoff] = 0;
		}
		break;
	case im_jlatin:
	case im_ascii:
		keystate[VK_KANA] = 0;
		break;
	default:
		break;
	}

	int retu = ToUnicode(vk, 0, keystate, szU, _countof(szU), 0);
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

BYTE CTextService::_GetSf(BYTE vk, WCHAR ch)
{
	BYTE k = SKK_NULL;

	if(ch == L'\0' && vk < KEYMAPNUM)
	{
		switch(inputmode)
		{
		case im_ascii:
		case im_jlatin:
			k = vkeymap.keylatin[vk];
			break;
		case im_hiragana:
		case im_katakana:
			k = vkeymap.keyjmode[vk];
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
			k = ckeymap.keylatin[ch];
			break;
		case im_hiragana:
		case im_katakana:
			k = ckeymap.keyjmode[ch];
			break;
		default:
			break;
		}
	}

	return k;
}

HRESULT CTextService::_ConvRomanKana(ROMAN_KANA_CONV *pconv)
{
	size_t i, count;
	HRESULT ret = E_ABORT;	//一致する可能性なし

	count = roman_kana_conv.size();

	for(i=0; i<count; i++)
	{
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
	std::wstring kanaconv;

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

	//片仮名変換
	if(c_addcandktkn && !abbrevmode)
	{
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			if(accompidx != 0)
			{
				_ConvKanaToKana(kanaconv, im_katakana, kana.substr(0, accompidx), inputmode);
			}
			else
			{
				_ConvKanaToKana(kanaconv, im_katakana, kana, inputmode);
			}
			break;
		default:
			break;
		}
	}

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
	if(!kanaconv.empty())
	{
		for(candidates_itr = candidates.begin(); candidates_itr != candidates.end(); candidates_itr++)
		{
			if(candidates_itr->first.first == kanaconv)
			{
				kanaconv.clear();
				break;
			}
		}
		if(!kanaconv.empty())
		{
			candidates.push_back(CANDIDATE(CANDIDATEBASE(kanaconv, L""), CANDIDATEBASE(kanaconv, L"")));
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
		if(c_delokuricncl && accompidx != 0)
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
	ROMAN_KANA_CONV rkc;
	HRESULT ret;
	WCHAR chN;
	WCHAR chO;
	std::wstring roman_conv;
	size_t i;

	if(roman.empty())
	{
		return TRUE;
	}

	//「ん etc.」
	wcsncpy_s(rkc.roman, roman.c_str(), _TRUNCATE);
	ret = _ConvRomanKana(&rkc);
	switch(ret)
	{
	case S_OK:	//一致
		if(rkc.wait)	//待機
		{
			if(accompidx != 0 && accompidx == kana.size())
			{
				chN = L'\0';
				switch(inputmode)
				{
				case im_hiragana:
					chN = rkc.hiragana[0];
					break;
				case im_katakana:
					chN = rkc.katakana[0];
					break;
				default:
					break;
				}

				chO = L'\0';
				for(i=0; i<CONV_POINT_NUM; i++)
				{
					if(conv_point[i][0] == L'\0' &&
						conv_point[i][1] == L'\0' &&
						conv_point[i][2] == L'\0')
					{
						break;
					}
					if(chN == conv_point[i][1])
					{
						chO = conv_point[i][2];
						break;
					}
				}

				if(chO == L'\0')
				{
					accompidx = 0;
				}
				else
				{
					kana.push_back(chO);
				}
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
			return TRUE;
		}
		break;
	default:
		break;
	}

	// ( <"n*", ""> -> <"", "ん"> )
	if(ch != L'\0' && ch != WCHAR_MAX)
	{
		roman_conv = roman;
		roman_conv.push_back(ch);
		wcsncpy_s(rkc.roman, roman_conv.c_str(), _TRUNCATE);
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

	// ( <"nn", ""> -> <"", "ん"> )
	if(roman.size() == 1)
	{
		chN = roman[0];
		rkc.roman[0] = chN;
		rkc.roman[1] = chN;
		rkc.roman[2] = L'\0';
		ret = _ConvRomanKana(&rkc);
		switch(ret)
		{
		case S_OK:	//一致
			if(!rkc.soku &&
				wcscmp(rkc.hiragana, L"ん") == 0 &&
				wcscmp(rkc.katakana, L"ン") == 0 &&
				wcscmp(rkc.katakana_ank, L"ﾝ") == 0)	//「nn soku==0」
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
	size_t i, j, count;
	BOOL exist;
	WCHAR *convkana;
	WCHAR srckana[3];
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

	count = roman_kana_conv.size();

	for(i=0; i<src.size(); i++)
	{
		if(((i + 1) < src.size()) && IS_SURROGATE_PAIR(src[i], src[i + 1]))
		{
			srckana[0] = src[i];
			srckana[1] = src[i + 1];
			srckana[2] = L'\0';
			i++;
		}
		else
		{
			srckana[0] = src[i];
			srckana[1] = L'\0';
		}
		exist = FALSE;

		for(j=0; j<count; j++)
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
