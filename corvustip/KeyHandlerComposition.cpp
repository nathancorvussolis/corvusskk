
#include "corvustip.h"
#include "TextService.h"
#include "CandidateList.h"

static LPCWSTR markMidashi = L"▽";
static LPCWSTR markHenkan = L"▼";
static LPCWSTR markOkuri = L"*";

static LPCWSTR markSP = L" ";
static LPCWSTR markAnnotation = L";";

HRESULT CTextService::_Update(TfEditCookie ec, ITfContext *pContext, BOOL fixed, BOOL back)
{
	std::wstring composition;
	WCHAR candidatecount[16];
	WCHAR useraddmode = REQ_USER_ADD_1;

	if(showentry &&
		(	(fixed && showcandlist) || 
			(c_untilcandlist == 0) || 
			(candidx + 1 < c_untilcandlist) || 
			(candidates.size() + 1 == c_untilcandlist)	))
	{
		if(!candidates.empty() && candidx < candidates.size())
		{
			if(!fixed && !c_nomodemark)
			{
				composition.append(markHenkan);
			}

			composition.append(candidates[candidx].first.first);

			if(accompidx != 0)
			{
				composition.append(kana.substr(accompidx + 1));
				useraddmode = REQ_USER_ADD_0;
			}

			if(!fixed && c_annotation && !c_annotatlst &&
				!candidates[candidx].first.second.empty())
			{
				composition.append(markAnnotation + candidates[candidx].first.second);
			}

			if(!fixed && c_untilcandlist == 0 && c_dispcandnum)
			{
				composition.append(L" (");
				_snwprintf_s(candidatecount, _TRUNCATE, L"%u", (UINT)candidx + 1);
				composition.append(candidatecount);
				composition.append(L"/");
				_snwprintf_s(candidatecount, _TRUNCATE, L"%u", (UINT)candidates.size());
				composition.append(candidatecount);
				composition.append(L")");
			}

			if(!fixed && c_nomodemark && composition.empty())
			{
				composition.append(markSP);
			}

			//ユーザ辞書登録
			if(fixed && !candidates[candidx].second.first.empty())
			{
				//数値変換タイプ0～3の候補は数値を#にした見出し語 それ以外は見出し語そのまま
				if(std::regex_match(candidates[candidx].second.first, std::wregex(L".*#[0-3].*")))
				{
					_AddUserDic(searchkey,
						candidates[candidx].second.first, candidates[candidx].second.second,
						useraddmode);
				}
				else
				{
					_AddUserDic(searchkeyorg,
						candidates[candidx].second.first, candidates[candidx].second.second,
						useraddmode);
				}
			}
		}
		else
		{
			//候補なし or 候補が尽きた
			if(!fixed)
			{
				if(c_nomodemark)
				{
					if(kana.empty())
					{
						composition.append(markSP);
					}
				}
				else
				{
					composition.append(markHenkan);
				}
			}

			if(accompidx == 0)
			{
				composition.append(kana);
			}
			else
			{
				composition.append(kana.substr(0, accompidx));
				if(!fixed && !c_nomodemark)
				{
					composition.append(markOkuri);
				}
				composition.append(kana.substr(accompidx + 1));
			}

			//辞書登録ウィンドウを表示可能なら表示する
			if(pContext == NULL || _ShowCandidateList(ec, pContext, TRUE) != S_OK)
			{
				//辞書登録中の検索または表示不可なら▽モードに戻す
				//ただし候補無しなら１回だけ▼で表示させる(_NextConv()にて、candidx = 0 となる)
				if(!candidates.empty())
				{
					if(c_delokuricncl && accompidx != 0)
					{
						kana = kana.substr(0, accompidx);
						accompidx = 0;
					}
					candidx = 0;
					showentry = FALSE;
					_Update(ec, pContext, fixed);
					return S_OK;
				}
			}
		}
	}
	else
	{
		if(inputkey)
		{
			if(!fixed)
			{
				if(c_nomodemark)
				{
					if(kana.empty() && roman.empty())
					{
						composition.append(markSP);
					}
				}
				else
				{
					if(showentry && (candidx + 1 == c_untilcandlist))
					{
						composition.append(markHenkan);
					}
					else
					{
						composition.append(markMidashi);
					}
				}
			}

			if(!roman.empty() || !kana.empty())
			{
				if(accompidx == 0)
				{
					composition.append(kana);
				}
				else
				{
					composition.append(kana.substr(0, accompidx));
					if(!fixed && !c_nomodemark)
					{
						composition.append(markOkuri);
					}
					if(accompidx + 1 < kana.size())
					{
						composition.append(kana.substr(accompidx + 1));
					}
				}
				if(!fixed && !roman.empty())
				{
					composition.append(roman);
				}
			}
		}
		else
		{
			if(!kana.empty())
			{
				composition.append(kana);
			}
			else
			{
				if(!fixed)
				{
					composition.append(roman);
				}
			}
		}
	}

	if(fixed && back && c_backincenter && !composition.empty())
	{
		//結合文字は考慮しない
		if(composition.size() >= 2 &&
			_IsSurrogatePair(composition[composition.size() - 2], composition[composition.size() - 1]))
		{
			composition.pop_back();
			composition.pop_back();
		}
		else
		{
			composition.pop_back();
		}
	}

	if(inputkey && !fixed && !showcandlist && showentry &&
		(((c_untilcandlist != 1) && (candidx + 1 == c_untilcandlist)) || (c_untilcandlist == 1)) &&
		(candidates.size() + 1 != c_untilcandlist))
	{
		if(pContext == NULL && _pCandidateList != NULL)	//辞書登録用
		{
			showcandlist = TRUE;
			candidx = 0;
			_pCandidateList->_SetTextRegword(L"", FALSE, TRUE);
			return S_OK;
		}
		else
		{
			//候補一覧表示開始
			showcandlist = TRUE;
			candidx = 0;
			_ShowCandidateList(ec, pContext, FALSE);
		}
	}

	if(pContext == NULL && _pCandidateList != NULL)	//辞書登録用
	{
		_pCandidateList->_SetTextRegword(composition, fixed, FALSE);
		return S_OK;
	}
	else
	{
		return _SetText(ec, pContext, composition, fixed);
	}
}

HRESULT CTextService::_SetText(TfEditCookie ec, ITfContext *pContext, const std::wstring &text, BOOL fixed)
{
	TF_SELECTION tfSelection;
	ITfRange *pRangeComposition;
	ULONG cFetched;

	if(pContext == NULL && _pCandidateList != NULL)	//辞書登録用
	{
		_pCandidateList->_SetTextRegword(text, fixed, FALSE);
		return S_OK;
	}

	if(!_IsComposing())
	{
		_StartComposition(pContext);
	}

	if(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
	{
		return S_FALSE;
	}

	if(_pComposition->GetRange(&pRangeComposition) == S_OK)
	{
		if(_IsRangeCovered(ec, tfSelection.range, pRangeComposition))
		{
			pRangeComposition->SetText(ec, 0, text.c_str(), (LONG)text.size());
			
			tfSelection.range->ShiftEndToRange(ec, pRangeComposition, TF_ANCHOR_END);
			tfSelection.range->Collapse(ec, TF_ANCHOR_END);

			pContext->SetSelection(ec, 1, &tfSelection);

			//for Excel's PHONETIC function
			if(fixed && !text.empty())
			{
				ITfProperty *pProperty;
				if(pContext->GetProperty(GUID_PROP_READING, &pProperty) == S_OK)
				{
					VARIANT var;
					var.vt = VT_BSTR;
					std::wstring phone(kana);
					if(accompidx == 0)
					{
						switch(inputmode)
						{
						case im_hiragana:
						case im_katakana:
							if(!abbrevmode && kana.size() >= 2)
							{
								if(kana.front() == L'>')
								{
									phone = kana.substr(1);
								}
								else if(kana.back() == L'>')
								{
									phone = kana.substr(0, kana.size() - 1);
								}
							}
							break;
						default:
							break;
						}
					}
					else
					{
						if(kana.size() > (accompidx + 1))
						{
							phone = kana.substr(0, accompidx) + kana.substr(accompidx + 1);
						}
						else if(kana.size() >= accompidx)
						{
							phone = kana.substr(0, accompidx);
						}
					}
					if(!phone.empty())
					{
						var.bstrVal = SysAllocString(phone.c_str());
						pProperty->SetValue(ec, pRangeComposition, &var);
						SysFreeString(var.bstrVal);
					}
					pProperty->Release();
				}
			}
		}
		pRangeComposition->Release();
	}

	tfSelection.range->Release();

	if(showentry)
	{
		_SetCompositionDisplayAttributes(ec, pContext, _gaDisplayAttributeConverted);
	}
	else
	{
		_SetCompositionDisplayAttributes(ec, pContext, _gaDisplayAttributeInput);
	}

	return S_OK;
}

HRESULT CTextService::_ShowCandidateList(TfEditCookie ec, ITfContext *pContext, BOOL reg)
{
	HRESULT hr = E_FAIL;

	if(_pCandidateList == NULL)
	{
		_pCandidateList = new CCandidateList(this);
	}

	ITfDocumentMgr *pDocumentMgr;
	if(pContext->GetDocumentMgr(&pDocumentMgr) == S_OK)
	{
		ITfRange *pRange;
		if(_pComposition->GetRange(&pRange) == S_OK)
		{
			hr = _pCandidateList->_StartCandidateList(_ClientId, pDocumentMgr, pContext, ec, pRange, reg);
			pRange->Release();
		}
		pDocumentMgr->Release();
	}
	return hr;
}
