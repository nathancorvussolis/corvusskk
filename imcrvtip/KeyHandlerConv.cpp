
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "convtype.h"

WCHAR CTextService::_GetCh(BYTE vk, BYTE vkoff)
{
	BYTE keystate[256];
	WCHAR ubuff;
	WCHAR u = L'\0';
	
	GetKeyboardState(keystate);

	switch(inputmode)
	{
	case im_hiragana:
	case im_katakana:
	case im_katakana_ank:
		keystate[VK_CAPITAL] = 0;
		if(abbrevmode || purgedicmode)
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

	int retu = ToUnicode(vk, 0, keystate, &ubuff, 1, 0);
	if(retu == 1)
	{
		u = ubuff;
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
		case im_katakana_ank:
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
		case im_katakana_ank:
			k = ckeymap.keyjmode[ch];
			break;
		default:
			break;
		}
	}

	switch(ch)
	{
	case TKB_NEXT_PAGE:
		k = SKK_NEXT_CAND;
		break;
	case TKB_PREV_PAGE:
		k = SKK_PREV_CAND;
		break;
	default:
		break;
	}

	return k;
}

HRESULT CTextService::_ConvRomanKana(ROMAN_KANA_CONV *pconv)
{
	size_t i, count;
	HRESULT ret = E_ABORT;	//一致する可能性なし

	count = roman_kana_conv.size();

	for(i = 0; i < count; i++)
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

	for(i = 0; i < ASCII_JLATIN_TBL_NUM; i++)
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
	CANDIDATES candidates_sel;
	CANDIDATES candidates_hint;
	CANDIDATES::iterator candidates_itr;
	CANDIDATES::iterator candidates_hint_itr;
	std::wstring keyhint, key, hint;
	std::wstring candidate, str;
	size_t accompidx_bak;
	size_t i;

	size_t hintchidx = kana.find_first_of(CHAR_SKK_HINT);

	if(!hintmode || hintchidx == std::wstring::npos)
	{
		_StartSubConv();
	}
	else
	{
		keyhint = kana;

		key = keyhint.substr(0, hintchidx);
		if(accompidx > key.size())
		{
			keyhint = keyhint.substr(0, accompidx + 1);
			accompidx = 0;
		}
		accompidx_bak = accompidx;
		accompidx = 0;
		hint = keyhint.substr(hintchidx + 1);

		//ヒント検索
		kana = hint;
		_StartSubConv();
		candidates_hint = candidates;

		//通常検索
		accompidx = accompidx_bak;
		kana = key;
		cursoridx = kana.size();
		_StartSubConv();

		//ヒント候補の文字を含む通常候補をヒント候補順で抽出
		for(candidates_hint_itr = candidates_hint.begin(); candidates_hint_itr != candidates_hint.end(); candidates_hint_itr++)
		{
			candidate = candidates_hint_itr->first.first;
			for(i = 0; i < candidate.size(); i++)
			{
				str.clear();
				if(i + 1 != candidate.size() && IS_SURROGATE_PAIR(candidate[i], candidate[i + 1]))
				{
					str.push_back(candidate[i]);
					str.push_back(candidate[i + 1]);
					i++;
				}
				else
				{
					str.push_back(candidate[i]);
				}

				for(candidates_itr = candidates.begin(); candidates_itr != candidates.end(); )
				{
					if(candidates_itr->first.first.find(str) != std::wstring::npos)
					{
						candidates_sel.push_back(*candidates_itr);
						candidates_itr = candidates.erase(candidates_itr);
					}
					else
					{
						candidates_itr++;
					}
				}
			}
		}
		candidates = candidates_sel;
	}

	hintmode = FALSE;
}

void CTextService::_StartSubConv()
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
	candorgcnt = 0;

	//通常検索
	_SearchDic(REQ_SEARCH);

	//片仮名変換
	if(cx_addcandktkn && !abbrevmode)
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

	candorgcnt = candidates.size();
	candidates_bak = candidates;
	candidates.clear();
	candidates.shrink_to_fit();

	searchkeyorg = searchkey;	//オリジナルバックアップ

	//数値を#に置換
	searchkey = std::regex_replace(searchkey, std::wregex(L"[0-9]+"), std::wstring(L"#"));
	if(searchkey != searchkeyorg)
	{
		//数値変換検索
		_SearchDic(REQ_SEARCH);
	}

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
		showentry = FALSE;
		candidx = 0;
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
		if(cx_delokuricncl && accompidx != 0)
		{
			kana = kana.substr(0, accompidx);
			accompidx = 0;
			cursoridx = kana.size();
		}
		if(cx_delcvposcncl && accompidx != 0)
		{
			kana.erase(accompidx, 1);
			accompidx = 0;
			cursoridx--;
		}
	}
}

void CTextService::_NextComp()
{
	if(!complement)
	{
		cursoridx = kana.size();
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
		_SearchDic(REQ_COMPLEMENT);

		if(!candidates.empty())
		{
			complement = TRUE;
			candidx = 0;
			_SetComp(candidates[candidx].first.first);
		}
	}
	else
	{
		if(candidx < candidates.size() - 1)
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

	if(cursoridx > kana.size())
	{
		cursoridx = kana.size();
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
				case im_katakana_ank:
					chN = rkc.katakana_ank[0];
					break;
				default:
					break;
				}

				chO = L'\0';
				for(i = 0; i < CONV_POINT_NUM; i++)
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
					kana.insert(cursoridx, 1, ch);
					cursoridx++;
				}
			}

			switch(inputmode)
			{
			case im_hiragana:
				kana.insert(cursoridx, rkc.hiragana);
				cursoridx += wcslen(rkc.hiragana);
				break;
			case im_katakana:
				kana.insert(cursoridx, rkc.katakana);
				cursoridx += wcslen(rkc.katakana);
				break;
			case im_katakana_ank:
				kana.insert(cursoridx, rkc.katakana_ank);
				cursoridx += wcslen(rkc.katakana_ank);
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

	// ( <"n*", ""> -> <"", "ん"> ) SKK_CONV_POINTのみ
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
					kana.insert(cursoridx, rkc.hiragana);
					cursoridx += wcslen(rkc.hiragana);
					break;
				case im_katakana:
					kana.insert(cursoridx, rkc.katakana);
					cursoridx += wcslen(rkc.katakana);
					break;
				case im_katakana_ank:
					kana.insert(cursoridx, rkc.katakana_ank);
					cursoridx += wcslen(rkc.katakana_ank);
					break;
				default:
					break;
				}
				roman.clear();
				return TRUE;	//「nk」etc.
			}
			else
			{
				_ConvNN();
				return TRUE;	//「ka,na,nn」etc.
			}
			break;
		case E_PENDING:	//途中まで一致
			_ConvNN();
			return TRUE;		//「ky,ny」etc.
			break;
		default:
			break;
		}
	}

	if(_ConvNN())
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CTextService::_ConvNN()
{
	ROMAN_KANA_CONV rkc;
	HRESULT ret;
	WCHAR chN;

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
					kana.insert(cursoridx, rkc.hiragana);
					cursoridx += wcslen(rkc.hiragana);
					break;
				case im_katakana:
					kana.insert(cursoridx, rkc.katakana);
					cursoridx += wcslen(rkc.katakana);
					break;
				case im_katakana_ank:
					kana.insert(cursoridx, rkc.katakana_ank);
					cursoridx += wcslen(rkc.katakana_ank);
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
	WCHAR *convkana = NULL;
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

	for(i = 0; i < src.size(); i++)
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

		for(j = 0; j < count; j++)
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
