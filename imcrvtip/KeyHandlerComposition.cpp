
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "InputModeWindow.h"

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
	LONG cchCursor = 0;
	LONG cchOkuri = 0;
	size_t i;
	BOOL showmodemark = cx_showmodemark;

	if(pContext == NULL)
	{
		showmodemark = TRUE;
	}

	if(showentry &&
		(	(fixed && showcandlist) ||
			(cx_untilcandlist == 0) ||
			(candidx + 1 < cx_untilcandlist) ||
			(candidates.size() + 1 == cx_untilcandlist)	))
	{
		if(!candidates.empty() && candidx < candidates.size())
		{
			if(!fixed && showmodemark)
			{
				composition.append(markHenkan);
			}

			composition.append(candidates[candidx].first.first);

			if(okuriidx != 0)
			{
				cchOkuri = (LONG)composition.size();
				composition.append(kana.substr(okuriidx + 1));
				useraddmode = REQ_USER_ADD_0;
			}

			cchCursor = (LONG)composition.size();

			if(!fixed)
			{
				if(purgedicmode)
				{
					composition.append(L" [削除?(Y/n)]");
				}
				else
				{
					if(cx_annotation && !cx_annotatlst && !candidates[candidx].first.second.empty())
					{
						composition.append(markAnnotation + candidates[candidx].first.second);
					}

					if(cx_untilcandlist == 0 && cx_dispcandnum)
					{
						composition.append(L" (");
						_snwprintf_s(candidatecount, _TRUNCATE, L"%u", (UINT)candidx + 1);
						composition.append(candidatecount);
						composition.append(L"/");
						_snwprintf_s(candidatecount, _TRUNCATE, L"%u", (UINT)candidates.size());
						composition.append(candidatecount);
						composition.append(L")");
					}

					if(!showmodemark && composition.empty())
					{
						composition.append(markSP);
					}
				}
			}

			//ユーザー辞書登録
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
				if(!showmodemark)
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

			if(okuriidx == 0)
			{
				composition.append(kana);
			}
			else
			{
				composition.append(kana.substr(0, okuriidx));
				cchOkuri = (LONG)composition.size();
				if(!fixed && showmodemark)
				{
					composition.append(markOkuri);
				}
				composition.append(kana.substr(okuriidx + 1));
			}

			cchCursor = (LONG)composition.size();

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
					if(cx_delokuricncl && okuriidx != 0)
					{
						kana = kana.substr(0, okuriidx);
						cchOkuri = (LONG)composition.size();
						okuriidx = 0;
					}
					candidx = 0;
					cursoridx = kana.size();
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
				if(!showmodemark)
				{
					if(kana.empty() && roman.empty())
					{
						composition.append(markSP);
					}
				}
				else
				{
					if(showentry && (candidx + 1 == cx_untilcandlist))
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
				if(okuriidx == 0)
				{
					composition.append(kana);
					if(pContext == NULL && !fixed && cursoridx != kana.size())
					{
						composition.insert(cursoridx + (composition.size() - kana.size()), markCursor);
					}
				}
				else
				{
					composition.append(kana.substr(0, okuriidx));
					cchOkuri = (LONG)composition.size();
					if(!fixed && showmodemark)
					{
						composition.append(markOkuri);
					}
					if(okuriidx + 1 < kana.size())
					{
						composition.append(kana.substr(okuriidx + 1));
					}
					if(pContext == NULL && !fixed && roman.empty() && cursoridx != kana.size())
					{
						if(!showmodemark)
						{
							if(cursoridx < okuriidx)
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
					if(!showmodemark)
					{
						if(okuriidx != 0 && okuriidx < cursoridx)
						{
							if(pContext == NULL && cursoridx != kana.size())
							{
								composition.insert(cursoridx - 1, markCursor);
							}
							if(cx_showroman)
							{
								composition.insert(cursoridx - 1, roman);
							}
							else
							{
								for(i = 0; i < roman.size(); i++)
								{
									composition.insert(cursoridx - 1, markSP);
								}
							}
						}
						else
						{
							if(pContext == NULL && cursoridx != kana.size())
							{
								composition.insert(cursoridx, markCursor);
							}
							if(cx_showroman)
							{
								composition.insert(cursoridx, roman);
							}
							else
							{
								for(i = 0; i < roman.size(); i++)
								{
									composition.insert(cursoridx, markSP);
								}
							}
						}
					}
					else
					{
						if(pContext == NULL && cursoridx != kana.size())
						{
							composition.insert(cursoridx + 1, markCursor);
						}
						if(cx_showroman)
						{
							composition.insert(cursoridx + 1, roman);
						}
						else
						{
							for(i = 0; i < roman.size(); i++)
							{
								composition.insert(cursoridx + 1, markSP);
							}
						}
					}
				}
			}

			if(showentry && (candidx + 1 == cx_untilcandlist))
			{
				cchCursor = (LONG)composition.size();
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
					if(cx_showroman)
					{
						composition.append(roman);
					}
					else
					{
						for(i = 0; i < roman.size(); i++)
						{
							composition.append(markSP);
						}
					}
				}
			}
		}
	}

	if(fixed && back && cx_backincenter && !composition.empty())
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
		(((cx_untilcandlist != 1) && (candidx + 1 == cx_untilcandlist)) || (cx_untilcandlist == 1)) &&
		(candidates.size() + 1 != cx_untilcandlist))
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
			_SetText(ec, pContext, composition, cchCursor, cchOkuri, fixed);
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
		return _SetText(ec, pContext, composition, cchCursor, cchOkuri, fixed);
	}
}

HRESULT CTextService::_SetText(TfEditCookie ec, ITfContext *pContext, const std::wstring &text, LONG cchCursor, LONG cchOkuri, BOOL fixed)
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

	if(_pInputModeWindow != NULL)
	{
		_pInputModeWindow->_Destroy();
		delete _pInputModeWindow;
		_pInputModeWindow = NULL;
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
			if(cchCursor == 0)
			{
				cchRes = (LONG)cursoridx - (LONG)kana.size();
				if(!cx_showmodemark && okuriidx != 0 && cursoridx <= okuriidx && cursoridx < kana.size())
				{
					cchRes += 1;
				}
			}
			else
			{
				cchRes = cchCursor - (LONG)text.size();
				if(cchRes > 0)
				{
					cchRes = 0;
				}
				else if(cchRes < -(LONG)text.size())
				{
					cchRes = -(LONG)text.size();
				}
			}

			tfSelection.range->ShiftEndToRange(ec, pRangeComposition, TF_ANCHOR_END);
			tfSelection.range->ShiftStartToRange(ec, pRangeComposition, TF_ANCHOR_END);
			tfSelection.range->ShiftStart(ec, cchRes, &cch, NULL);
			//decide cursor position
			tfSelection.range->Collapse(ec, TF_ANCHOR_START);
			pContext->SetSelection(ec, 1, &tfSelection);

			if(!fixed)
			{
				if(pRangeComposition->Clone(&pRangeClone) == S_OK)
				{
					pRangeClone->ShiftEndToRange(ec, pRangeComposition, TF_ANCHOR_END);
					pRangeClone->ShiftStartToRange(ec, pRangeComposition, TF_ANCHOR_START);

					if(cchCursor == 0)
					{
						if(inputkey)
						{
							_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeInputMark);
							if(cx_showmodemark)
							{
								pRangeClone->ShiftStart(ec, 1, &cch, NULL);
							}
						}

						if(!display_attribute_series[1] || !inputkey)
						{
							_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeInputText);
						}

						if(cchOkuri != 0)
						{
							pRangeClone->ShiftStartToRange(ec, pRangeComposition, TF_ANCHOR_START);
							pRangeClone->ShiftStart(ec, cchOkuri, &cch, NULL);
							if(!display_attribute_series[2])
							{
								_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeInputOkuri);
							}

							if(hintmode && text.find_first_of(CHAR_SKK_HINT) != std::wstring::npos)
							{
								LONG hintpos = (LONG)text.find_first_of(CHAR_SKK_HINT);
								if(cchOkuri < hintpos)
								{
									pRangeClone->ShiftStartToRange(ec, pRangeComposition, TF_ANCHOR_START);
									pRangeClone->ShiftStart(ec, hintpos, &cch, NULL);
									_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeInputText);
								}
							}
						}
					}
					else
					{
						_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeConvMark);
						if(cx_showmodemark)
						{
							pRangeClone->ShiftStart(ec, 1, &cch, NULL);
						}

						if(!display_attribute_series[4])
						{
							_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeConvText);
						}

						if(cchOkuri != 0)
						{
							pRangeClone->ShiftStartToRange(ec, pRangeComposition, TF_ANCHOR_START);
							pRangeClone->ShiftStart(ec, cchOkuri, &cch, NULL);
							if(!display_attribute_series[5])
							{
								_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeConvOkuri);
							}
						}

						pRangeClone->ShiftEndToRange(ec, pRangeComposition, TF_ANCHOR_END);
						pRangeClone->ShiftStartToRange(ec, tfSelection.range, TF_ANCHOR_END);
						if(!display_attribute_series[6])
						{
							_SetCompositionDisplayAttributes(ec, pContext, pRangeClone, _gaDisplayAttributeConvAnnot);
						}
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
					if(okuriidx == 0)
					{
						switch(inputmode)
						{
						case im_hiragana:
						case im_katakana:
						case im_katakana_ank:
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
						if(kana.size() > (okuriidx + 1))
						{
							phone = kana.substr(0, okuriidx) + kana.substr(okuriidx + 1);
						}
						else if(kana.size() >= okuriidx)
						{
							phone = kana.substr(0, okuriidx);
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
