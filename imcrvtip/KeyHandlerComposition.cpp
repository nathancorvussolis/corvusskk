
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"

static LPCWSTR markMidashi = L"▽";
static LPCWSTR markHenkan = L"▼";
static LPCWSTR markOkuri = L"*";

static LPCWSTR markSP = L"\x20";
static LPCWSTR markAnnotation = L";";
static LPCWSTR markCursor = L"|";

HRESULT CTextService::_Update(TfEditCookie ec, ITfContext *pContext, BOOL fixed, BOOL back)
{
	std::wstring composition;
	return _Update(ec, pContext, composition, fixed, back);
}

HRESULT CTextService::_Update(TfEditCookie ec, ITfContext *pContext, std::wstring &composition, BOOL fixed, BOOL back)
{
	WCHAR candidatecount[16];
	WCHAR useraddmode = REQ_USER_ADD_1;
	LONG cchReq = 0;

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

			cchReq = (LONG)composition.size();

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
				_AddUserDic(useraddmode, ((candorgcnt <= candidx) ? searchkey : searchkeyorg),
					candidates[candidx].second.first, candidates[candidx].second.second);
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

			cchReq = (LONG)composition.size();

			//辞書登録ウィンドウを表示可能なら表示する
			if(pContext == NULL)	//辞書登録用
			{
				_pCandidateList->_SetText(composition, FALSE, FALSE, TRUE);
				return S_OK;
			}

			if(_ShowCandidateList(ec, pContext, TRUE) != S_OK)
			{
				//表示不可のとき▽モードに戻す
				//ただし候補無しのとき１回だけ▼で表示させる(_NextConv()にて、candidx = 0 となる)
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
					if(pContext == NULL && !fixed && cursoridx != kana.size())
					{
						composition.insert(cursoridx + (composition.size() - kana.size()), markCursor);
					}
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
					if(pContext == NULL && !fixed && roman.empty() && cursoridx != kana.size())
					{
						if(c_nomodemark)
						{
							if(cursoridx < accompidx)
							{
								composition.insert(cursoridx, markCursor);
							}
							else
							{
								composition.insert(cursoridx - 1, markCursor);
							}
						}
						else
						{
							composition.insert(cursoridx + 1, markCursor);
						}
					}
				}
				if(!fixed && !roman.empty())
				{
					if(c_nomodemark)
					{
						if(accompidx != 0 && accompidx < cursoridx)
						{
							if(pContext == NULL && cursoridx != kana.size())
							{
								composition.insert(cursoridx - 1, markCursor);
							}
							composition.insert(cursoridx - 1, roman);
						}
						else
						{
							if(pContext == NULL && cursoridx != kana.size())
							{
								composition.insert(cursoridx, markCursor);
							}
							composition.insert(cursoridx, roman);
						}
					}
					else
					{
						if(pContext == NULL && cursoridx != kana.size())
						{
							composition.insert(cursoridx + 1, markCursor);
						}
						composition.insert(cursoridx + 1, roman);
					}
				}
			}

			if(showentry && (candidx + 1 == c_untilcandlist))
			{
				cchReq = (LONG)composition.size();
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
		// surrogate pair
		if(composition.size() >= 2 &&
			IS_SURROGATE_PAIR(composition[composition.size() - 2], composition[composition.size() - 1]))
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
			_pCandidateList->_SetText(composition, FALSE, TRUE, FALSE);
			return S_OK;
		}
		else
		{
			_SetText(ec, pContext, composition, cchReq, fixed);
			//候補一覧表示開始
			showcandlist = TRUE;
			candidx = 0;
			return _ShowCandidateList(ec, pContext, FALSE);
		}
	}

	if(pContext == NULL && _pCandidateList != NULL)	//辞書登録用
	{
		_pCandidateList->_SetText(composition, fixed, FALSE, FALSE);
		return S_OK;
	}
	else
	{
		return _SetText(ec, pContext, composition, cchReq, fixed);
	}
}

HRESULT CTextService::_SetText(TfEditCookie ec, ITfContext *pContext, const std::wstring &text, LONG cchReq, BOOL fixed)
{
	TF_SELECTION tfSelection;
	ITfRange *pRangeComposition;
	ITfRange *pRangeClone;
	ULONG cFetched;
	LONG cch, cchRes;

	if(pContext == NULL && _pCandidateList != NULL)	//辞書登録用
	{
		_pCandidateList->_SetText(text, fixed, FALSE, FALSE);
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
			
			// shift from end to start.
			// shift over mathematical operators (U+2200-U+22FF) is rejected by OneNote.
			if(cchReq == 0)
			{
				tfSelection.range->ShiftEndToRange(ec, pRangeComposition, TF_ANCHOR_END);
				tfSelection.range->ShiftStartToRange(ec, pRangeComposition, TF_ANCHOR_END);

				if(c_nomodemark && accompidx != 0 && cursoridx <= accompidx && cursoridx < kana.size())
				{
					cchRes = (LONG)cursoridx - (LONG)kana.size() + 1;
				}
				else
				{
					cchRes = (LONG)cursoridx - (LONG)kana.size();
				}

				tfSelection.range->ShiftStart(ec, cchRes, &cch, NULL);
			}
			else
			{
				cchRes = cchReq - (LONG)text.size();
				if(cchRes > 0)
				{
					cchRes = 0;
				}
				else if(cchRes < -(LONG)text.size())
				{
					cchRes = -(LONG)text.size();
				}

				tfSelection.range->ShiftEndToRange(ec, pRangeComposition, TF_ANCHOR_END);
				tfSelection.range->ShiftStartToRange(ec, pRangeComposition, TF_ANCHOR_END);
				tfSelection.range->ShiftStart(ec, cchRes, &cch, NULL);
			}

			tfSelection.range->Collapse(ec, TF_ANCHOR_START);
			pContext->SetSelection(ec, 1, &tfSelection);

			if(!fixed)
			{
				if(pRangeComposition->Clone(&pRangeClone) == S_OK)
				{
					if(cchReq == 0)
					{
						_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeInput);
					}
					else
					{
						pRangeClone->ShiftEndToRange(ec, tfSelection.range, TF_ANCHOR_END);
						pRangeClone->ShiftStartToRange(ec, pRangeComposition, TF_ANCHOR_START);
						_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeCandidate);

						pRangeClone->ShiftEndToRange(ec, pRangeComposition, TF_ANCHOR_END);
						pRangeClone->ShiftStartToRange(ec, tfSelection.range, TF_ANCHOR_END);
						_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeAnnotation);
					}
					pRangeClone->Release();
				}
			}

			// for Excel's PHONETIC function
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
