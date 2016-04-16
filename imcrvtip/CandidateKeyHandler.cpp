
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "CandidateWindow.h"
#include "InputModeWindow.h"

HRESULT CCandidateWindow::_OnKeyDown(UINT uVKey)
{
	UINT i, page, index;
	WCHAR ch;
	BYTE sf;

	if(_pCandidateWindow != nullptr && !_preEnd)
	{
		return _pCandidateWindow->_OnKeyDown(uVKey);
	}

	//辞書登録モード
	if(regword)
	{
		_OnKeyDownRegword(uVKey);
		return S_OK;
	}

	_GetChSf(uVKey, ch, sf);

	//複数補完/複数動的補完
	if(_comp)
	{
		switch(sf)
		{
		case SKK_NEXT_COMP:
			if(candidx == (size_t)-1)
			{
				candidx = 0;
				_InvokeSfHandler(SKK_NEXT_COMP);
			}
			else
			{
				_NextComp();
			}
			break;
		case SKK_PREV_COMP:
			if(candidx != (size_t)-1)
			{
				_PrevComp();
			}
			break;
		default:
			_InvokeKeyHandler(uVKey);
			break;
		}

		return S_OK;
	}

	//候補選択
	switch(sf)
	{
	case SKK_CANCEL:
		if(_pCandidateList != nullptr)
		{
			if(!regword)
			{
				if(_pCandidateWindowParent == nullptr)
				{
					_InvokeSfHandler(SKK_CANCEL);
				}
				else
				{
					if(_reg)
					{
						_RestoreStatusReg();
					}
					_PreEndReq();
					_HandleKey(0, SKK_CANCEL);
					_EndReq();
				}
			}
			else
			{
				_HandleKey(0, SKK_CANCEL);
				_Update();
			}
		}
		break;

	case SKK_BACK:
	case SKK_PREV_CAND:
		_PrevPage();
		break;

	case SKK_NEXT_CAND:
		_NextPage();
		break;

	default:
		_GetChSf(uVKey, ch, sf, VK_KANA);

		for(i = 0; i < MAX_SELKEY_C; i++)
		{
			if(ch == (L'1' + i) ||
				(ch == _pTextService->selkey[i][0][0] && _pTextService->selkey[i][0][0] != L'\0') ||
				(ch == _pTextService->selkey[i][1][0] && _pTextService->selkey[i][1][0] != L'\0'))
			{
				GetCurrentPage(&page);
				if(i < _CandCount[page])
				{
					index = (UINT)(_pTextService->cx_untilcandlist - 1) + _PageIndex[page] + i;
					if(index < _pTextService->candidates.size())
					{
						if(!regword)
						{
							if(_pCandidateWindowParent == nullptr)
							{
								_pTextService->candidx = index;
								_InvokeSfHandler(SKK_ENTER);
							}
							else
							{
								if(_reg)
								{
									_RestoreStatusReg();
								}
								_PreEndReq();
								_pTextService->candidx = index;
								_HandleKey(0, SKK_ENTER);
								_EndReq();
							}
						}
						else
						{
							_pTextService->candidx = index;
							_HandleKey(0, SKK_ENTER);
							_Update();
						}
						break;
					}
				}
			}
		}
		break;
	}

	return S_OK;
}

void CCandidateWindow::_OnKeyDownRegword(UINT uVKey)
{
	WCHAR ch;
	BYTE sf;

	_GetChSf(uVKey, ch, sf);

	//確定していないとき
	if(!regwordfixed)
	{
		_pTextService->showcandlist = FALSE;	//候補一覧表示をループさせる
		_HandleKey((WPARAM)uVKey, SKK_NULL);
		_Update();

		if(_pInputModeWindow != nullptr)
		{
			_pInputModeWindow->_Redraw();
		}
		return;
	}

	if(_pTextService->_IsKeyVoid(ch, (BYTE)uVKey))
	{
		_pTextService->_UpdateLanguageBar();

		if(_pInputModeWindow != nullptr)
		{
			_pInputModeWindow->_Redraw();
		}

		if(sf == SKK_ENTER)
		{
			return;
		}
	}

	switch(sf)
	{
	case SKK_ENTER:
		_RestoreStatusReg();
		_ClearStatusReg();

		regwordfixed = FALSE;
		regwordul = FALSE;
		regword = FALSE;

		//スペースのみのとき空として扱う
		if(std::regex_match(regwordtext, std::wregex(L"^\\s+$")))
		{
			regwordtext.clear();
		}

		if(regwordtext.empty())	//空のときはキャンセル扱い
		{
			regwordtext.clear();
			regwordtextpos = 0;

			if(!_reg)
			{
				_InitList();
				_uIndex = _PageIndex[_PageIndex.size() - 1];
				_Update();
				_UpdateUIElement();

				if(_pInputModeWindow != nullptr)
				{
					_pInputModeWindow->_Show(FALSE);
				}
			}
			else
			{
				if(_pCandidateWindowParent == nullptr)
				{
					if(_pTextService->candidates.empty())
					{
						_InvokeSfHandler(SKK_CANCEL);
					}
					else
					{
						_InvokeSfHandler(SKK_PREV_CAND);
					}
				}
				else
				{
					_PreEndReq();
					if(_pTextService->candidates.empty())
					{
						_HandleKey(0, SKK_CANCEL);
					}
					else
					{
						_HandleKey(0, SKK_PREV_CAND);
					}
					_EndReq();
				}
			}
		}
		else
		{
			std::wstring conv;
			std::wstring candidate;
			std::wstring annotation;
			std::wsmatch result;
			std::wstring okurikey;

			//候補と注釈を、行頭以外の最後のセミコロンで分割
			if(std::regex_search(regwordtext, result, std::wregex(L".+;")))
			{
				candidate = result.str().substr(0, result.str().size() - 1);
				annotation = result.suffix();
			}
			else
			{
				candidate = regwordtext;
				annotation.clear();
			}

			if(_pTextService->okuriidx != 0)
			{
				okurikey = _pTextService->kana.substr(_pTextService->okuriidx + 1);
				if(okurikey.size() >= 2 &&
					IS_SURROGATE_PAIR(okurikey.c_str()[0], okurikey.c_str()[1]))
				{
					okurikey = okurikey.substr(0, 2);
				}
				else
				{
					okurikey = okurikey.substr(0, 1);
				}
			}

			//候補変換
			_pTextService->_ConvertWord(REQ_CONVERTCND, _pTextService->searchkeyorg, candidate, okurikey, conv);

			if(_pTextService->searchkey.empty() || conv.empty())
			{
				//変換見出し語が空文字列または
				//変換済み候補が空文字列であれば未変換見出し語を見出し語とする
				_pTextService->searchkey = _pTextService->searchkeyorg;
			}

			if(conv.empty())
			{
				conv = candidate;
			}

			_pTextService->candidates.push_back(CANDIDATE
				(CANDIDATEBASE(conv, annotation),
				(CANDIDATEBASE(candidate, annotation))));
			_pTextService->candidx = _pTextService->candidates.size() - 1;
			_pTextService->candorgcnt = 0;

			regwordtext.clear();
			regwordtextpos = 0;

			if(_pCandidateWindowParent == nullptr)
			{
				_InvokeSfHandler(SKK_ENTER);
			}
			else
			{
				_PreEndReq();
				_HandleKey(0, SKK_ENTER);
				_EndReq();
			}
		}
		break;

	case SKK_CANCEL:
		_RestoreStatusReg();
		_ClearStatusReg();

		regwordfixed = FALSE;
		regwordul = FALSE;
		regword = FALSE;

		regwordtext.clear();
		regwordtextpos = 0;

		if(!_reg)
		{
			_InitList();
			_uIndex = _PageIndex[_PageIndex.size() - 1];
			_Update();
			_UpdateUIElement();

			if(_pInputModeWindow != nullptr)
			{
				_pInputModeWindow->_Show(FALSE);
			}
		}
		else
		{
			if(_pCandidateWindowParent == nullptr)
			{
				if(_pTextService->candidates.empty())
				{
					_InvokeSfHandler(SKK_CANCEL);
				}
				else
				{
					_InvokeSfHandler(SKK_PREV_CAND);
				}
			}
			else
			{
				_PreEndReq();
				if(_pTextService->candidates.empty())
				{
					_HandleKey(0, SKK_CANCEL);
				}
				else
				{
					_HandleKey(0, SKK_PREV_CAND);
				}
				_EndReq();
			}
		}
		break;

	case SKK_BACK:
		if(comptext.empty() && regwordtextpos > 0 && regwordtext.size() > 0)
		{
			// surrogate pair
			if(regwordtext.size() >= 2 && regwordtextpos >= 2 &&
				IS_SURROGATE_PAIR(regwordtext[regwordtextpos - 2], regwordtext[regwordtextpos - 1]))
			{
				regwordtextpos -= 2;
				regwordtext.erase(regwordtext.begin() + regwordtextpos);
				regwordtext.erase(regwordtext.begin() + regwordtextpos);
			}
			else
			{
				--regwordtextpos;
				regwordtext.erase(regwordtext.begin() + regwordtextpos);
			}
			_Update();
		}
		break;

	case SKK_DELETE:
		if(comptext.empty() && regwordtextpos < regwordtext.size())
		{
			// surrogate pair
			if(regwordtext.size() >= regwordtextpos + 2 &&
				IS_SURROGATE_PAIR(regwordtext[regwordtextpos + 0], regwordtext[regwordtextpos + 1]))
			{
				regwordtext.erase(regwordtext.begin() + regwordtextpos);
				regwordtext.erase(regwordtext.begin() + regwordtextpos);
			}
			else
			{
				regwordtext.erase(regwordtext.begin() + regwordtextpos);
			}
			_Update();
		}
		break;

	case SKK_LEFT:
		if(comptext.empty() && regwordtextpos > 0 && regwordtext.size() > 0)
		{
			// surrogate pair
			if(regwordtext.size() >= 2 && regwordtextpos >= 2 &&
				IS_SURROGATE_PAIR(regwordtext[regwordtextpos - 2], regwordtext[regwordtextpos - 1]))
			{
				regwordtextpos -= 2;
			}
			else
			{
				--regwordtextpos;
			}
			_Update();
		}
		break;

	case SKK_UP:
		if(comptext.empty())
		{
			regwordtextpos = 0;
			_Update();
		}
		break;

	case SKK_RIGHT:
		if(comptext.empty() && regwordtextpos < regwordtext.size())
		{
			// surrogate pair
			if(regwordtext.size() >= regwordtextpos + 2 &&
				IS_SURROGATE_PAIR(regwordtext[regwordtextpos + 0], regwordtext[regwordtextpos + 1]))
			{
				regwordtextpos += 2;
			}
			else
			{
				++regwordtextpos;
			}
			_Update();
		}
		break;

	case SKK_DOWN:
		if(comptext.empty())
		{
			regwordtextpos = regwordtext.size();
			_Update();
		}
		break;

	case SKK_PASTE:
		if(IsClipboardFormatAvailable(CF_UNICODETEXT))
		{
			if(OpenClipboard(nullptr))
			{
				HANDLE hCB = GetClipboardData(CF_UNICODETEXT);
				if(hCB != nullptr)
				{
					LPWSTR pwCB = (LPWSTR)GlobalLock(hCB);
					if(pwCB != nullptr)
					{
						std::wstring scb = std::regex_replace(std::wstring(pwCB),
							std::wregex(L"[\\x00-\\x19]"), std::wstring(L""));
						regwordtext.insert(regwordtextpos, scb);
						regwordtextpos += scb.size();
						_Update();
						_UpdateUIElement();
						GlobalUnlock(hCB);
					}
				}
				CloseClipboard();
			}
		}
		break;

	default:
		_HandleKey((WPARAM)uVKey, SKK_NULL);

		if(_pInputModeWindow != nullptr)
		{
			_pInputModeWindow->_Redraw();
		}
		break;
	}
}

void CCandidateWindow::_InvokeSfHandler(BYTE sf)
{
	if(_pCandidateList != nullptr)
	{
		_pCandidateList->_InvokeSfHandler(sf);
	}
}

void CCandidateWindow::_InvokeKeyHandler(UINT uVKey)
{
	if(_pCandidateList != nullptr)
	{
		_pCandidateList->_InvokeKeyHandler(uVKey);
	}
}

void CCandidateWindow::_HandleKey(WPARAM wParam, BYTE bSf)
{
	if(_pTextService != nullptr)
	{
		_pTextService->_HandleKey(0, nullptr, wParam, bSf);
	}
}

void CCandidateWindow::_GetChSf(UINT uVKey, WCHAR &ch, BYTE &sf, BYTE vkoff)
{
	if(_pTextService != nullptr)
	{
		ch = _pTextService->_GetCh(uVKey, vkoff);
		sf = _pTextService->_GetSf(uVKey, ch);
	}
}
