
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "CandidateWindow.h"
#include "InputModeWindow.h"

HRESULT CCandidateWindow::_OnKeyDown(WPARAM wParam)
{
	UINT i, page, index;
	WCHAR ch;
	BYTE sf;

	if (_pCandidateWindow != nullptr && !_preEnd)
	{
		return _pCandidateWindow->_OnKeyDown(wParam);
	}

	//辞書登録モード
	if (_regmode)
	{
		_OnKeyDownRegword(wParam);
		return S_OK;
	}

	_GetChSf(wParam, ch, sf);

	//辞書削除
	if (_mode == wm_delete)
	{
		switch (ch)
		{
		case L'Y': case L'y':
			sf = SKK_ENTER;
			break;
		case L'N': case L'n':
			sf = SKK_CANCEL;
			break;
		default:
			break;
		}

		if (_pTextService->_IsKeyVoid(ch, (BYTE)wParam))
		{
			if (sf == SKK_ENTER)
			{
				return S_OK;
			}
		}

		switch (sf)
		{
		case SKK_ENTER:
			if (_pCandidateWindowParent == nullptr)
			{
				_InvokeSfHandler(SKK_ENTER);
			}
			else
			{
				_PreEndReq();
				_HandleKey(0, SKK_ENTER);
				_EndReq();
			}
			break;
		case SKK_CANCEL:
			if (_pCandidateWindowParent == nullptr)
			{
				_InvokeSfHandler(SKK_CANCEL);
			}
			else
			{
				_PreEndReq();
				_HandleKey(0, SKK_CANCEL);
				_EndReq();
			}
			return S_OK;
			break;
		default:
			break;
		}

		return S_OK;
	}

	//複数補完/複数動的補完
	if (_mode == wm_complement)
	{
		switch (sf)
		{
		case SKK_NEXT_COMP:
			if (candidx == (size_t)-1)
			{
				candidx = 0;
				_InvokeSfHandler(SKK_NEXT_COMP);
			}
			else
			{
				_NextCompPage();
			}
			break;
		case SKK_PREV_COMP:
			if (candidx != (size_t)-1)
			{
				_PrevCompPage();
			}
			break;
		default:
			_InvokeKeyHandler(wParam);
			break;
		}

		return S_OK;
	}

	//候補選択
	switch (sf)
	{
	case SKK_CANCEL:
		if (_pCandidateList != nullptr)
		{
			if (!_regmode)
			{
				if (_pCandidateWindowParent == nullptr)
				{
					_InvokeSfHandler(SKK_CANCEL);
				}
				else
				{
					if (_mode == wm_register)
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
		_PrevConvPage();
		break;

	case SKK_NEXT_CAND:
		_NextConvPage();
		break;

	default:
		_GetChSf(wParam, ch, sf, VK_KANA);

		for (i = 0; i < MAX_SELKEY_C; i++)
		{
			WCHAR dsph = _pTextService->selkey[i].disp[0];
			WCHAR dspl = _pTextService->selkey[i].disp[1];
			WCHAR sp1 = _pTextService->selkey[i].spare1;
			WCHAR sp2 = _pTextService->selkey[i].spare2;

			if (ch == (L'1' + i) ||
				(ch != L'\0' && ((ch == dsph && dspl == L'\0') || ch == sp1 || ch == sp2)))
			{
				GetCurrentPage(&page);
				if (i < _CandCount[page])
				{
					index = (UINT)(_pTextService->cx_untilcandlist - 1) + _PageIndex[page] + i;
					if (index < _pTextService->candidates.size())
					{
						if (!_regmode)
						{
							if (_pCandidateWindowParent == nullptr)
							{
								_pTextService->candidx = index;
								_InvokeSfHandler(SKK_ENTER);
							}
							else
							{
								if (_mode == wm_register)
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

void CCandidateWindow::_OnKeyDownRegword(WPARAM wParam)
{
	WCHAR ch;
	BYTE sf;

	_GetChSf(wParam, ch, sf);

	//確定していないとき
	if (!_regfixed)
	{
		_pTextService->showcandlist = FALSE;	//候補一覧表示をループさせる
		_HandleKey(wParam, SKK_NULL);
		_Update();

		if (_pInputModeWindow != nullptr)
		{
			_pInputModeWindow->_Redraw();
		}
		return;
	}

	if (_pTextService->_IsKeyVoid(ch, (BYTE)wParam))
	{
		_pTextService->_UpdateLanguageBar();

		if (_pInputModeWindow != nullptr)
		{
			_pInputModeWindow->_Redraw();
		}

		if (sf == SKK_ENTER)
		{
			return;
		}
	}

	switch (sf)
	{
	case SKK_ENTER:
		_RestoreStatusReg();
		_ClearStatusReg();

		_ulsingle = FALSE;

		_regfixed = FALSE;
		_regmode = FALSE;

		//スペースのみのとき空として扱う
		if (std::regex_match(_regtext, RegExp(L"^\\s+$")))
		{
			_regtext.clear();
		}

		if (_regtext.empty())	//空のときはキャンセル扱い
		{
			_regtext.clear();
			_regtextpos = 0;

			if (_mode == wm_candidate)
			{
				if (_pInputModeWindow != nullptr)
				{
					_pInputModeWindow->_Show(FALSE);
				}

				_InitList();
				_uIndex = _PageIndex[_PageIndex.size() - 1];
				_Update();
				_UpdateUIElement();
			}
			else
			{
				if (_pCandidateWindowParent == nullptr)
				{
					if (_pTextService->candidates.empty())
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
					if (_pTextService->candidates.empty())
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
			std::wstring convcand;
			std::wstring candidate;
			std::wstring annotation;
			std::wsmatch result;
			std::wstring okurikey;

			//候補と注釈を、行頭以外の最後のセミコロンで分割
			if (std::regex_search(_regtext, result, RegExp(L".+;")))
			{
				candidate = result.str().substr(0, result.str().size() - 1);
				annotation = result.suffix();
			}
			else
			{
				candidate = _regtext;
				annotation.clear();
			}

			if (_pTextService->okuriidx != 0)
			{
				okurikey = _pTextService->kana.substr(_pTextService->okuriidx + 1);
				if (okurikey.size() >= 2 &&
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
			_pTextService->_ConvertWord(REQ_CONVERTCND, _pTextService->searchkeyorg, candidate, okurikey);

			convcand = _pTextService->convword;

			if (_pTextService->searchkey.empty() || convcand.empty())
			{
				//変換見出し語が空文字列または
				//変換済み候補が空文字列であれば未変換見出し語を見出し語とする
				_pTextService->searchkey = _pTextService->searchkeyorg;
			}

			if (convcand.empty())
			{
				convcand = candidate;
			}

			_pTextService->candidates.push_back(std::make_pair(
				std::make_pair(convcand, annotation),
				std::make_pair(candidate, annotation)));
			_pTextService->candidx = _pTextService->candidates.size() - 1;
			_pTextService->candorgcnt = 0;

			_regtext.clear();
			_regtextpos = 0;

			if (_pCandidateWindowParent == nullptr)
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

		_ulsingle = FALSE;

		_regfixed = FALSE;
		_regmode = FALSE;

		_regtext.clear();
		_regtextpos = 0;

		if (_mode == wm_candidate)
		{
			if (_pInputModeWindow != nullptr)
			{
				_pInputModeWindow->_Show(FALSE);
			}

			_InitList();
			_uIndex = _PageIndex[_PageIndex.size() - 1];
			_Update();
			_UpdateUIElement();
		}
		else
		{
			if (_pCandidateWindowParent == nullptr)
			{
				if (_pTextService->candidates.empty())
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
				if (_pTextService->candidates.empty())
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
		if (_regcomp.empty() && _regtextpos > 0 && _regtext.size() > 0)
		{
			// surrogate pair
			if (_regtext.size() >= 2 && _regtextpos >= 2 &&
				IS_SURROGATE_PAIR(_regtext[_regtextpos - 2], _regtext[_regtextpos - 1]))
			{
				_regtextpos -= 2;
				_regtext.erase(_regtext.begin() + _regtextpos);
				_regtext.erase(_regtext.begin() + _regtextpos);
			}
			else
			{
				--_regtextpos;
				_regtext.erase(_regtext.begin() + _regtextpos);
			}
			_Update();
		}
		break;

	case SKK_DELETE:
		if (_regcomp.empty() && _regtextpos < _regtext.size())
		{
			// surrogate pair
			if (_regtext.size() >= _regtextpos + 2 &&
				IS_SURROGATE_PAIR(_regtext[_regtextpos + 0], _regtext[_regtextpos + 1]))
			{
				_regtext.erase(_regtext.begin() + _regtextpos);
				_regtext.erase(_regtext.begin() + _regtextpos);
			}
			else
			{
				_regtext.erase(_regtext.begin() + _regtextpos);
			}
			_Update();
		}
		break;

	case SKK_LEFT:
		if (_regcomp.empty() && _regtextpos > 0 && _regtext.size() > 0)
		{
			// surrogate pair
			if (_regtext.size() >= 2 && _regtextpos >= 2 &&
				IS_SURROGATE_PAIR(_regtext[_regtextpos - 2], _regtext[_regtextpos - 1]))
			{
				_regtextpos -= 2;
			}
			else
			{
				--_regtextpos;
			}
			_Update();
		}
		break;

	case SKK_UP:
		if (_regcomp.empty())
		{
			_regtextpos = 0;
			_Update();
		}
		break;

	case SKK_RIGHT:
		if (_regcomp.empty() && _regtextpos < _regtext.size())
		{
			// surrogate pair
			if (_regtext.size() >= _regtextpos + 2 &&
				IS_SURROGATE_PAIR(_regtext[_regtextpos + 0], _regtext[_regtextpos + 1]))
			{
				_regtextpos += 2;
			}
			else
			{
				++_regtextpos;
			}
			_Update();
		}
		break;

	case SKK_DOWN:
		if (_regcomp.empty())
		{
			_regtextpos = _regtext.size();
			_Update();
		}
		break;

	case SKK_PASTE:
		if (IsClipboardFormatAvailable(CF_UNICODETEXT))
		{
			if (OpenClipboard(nullptr))
			{
				HANDLE hCB = GetClipboardData(CF_UNICODETEXT);
				if (hCB != nullptr)
				{
					LPWSTR pwCB = (LPWSTR)GlobalLock(hCB);
					if (pwCB != nullptr)
					{
						std::wstring scb = std::regex_replace(pwCB, RegExp(L"[\\x00-\\x19]"), L"");
						_regtext.insert(_regtextpos, scb);
						_regtextpos += scb.size();
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
		_HandleKey(wParam, SKK_NULL);

		if (_pInputModeWindow != nullptr)
		{
			_pInputModeWindow->_Redraw();
		}
		break;
	}
}

void CCandidateWindow::_InvokeSfHandler(BYTE sf)
{
	if (_pCandidateList != nullptr)
	{
		_pCandidateList->_InvokeSfHandler(sf);
	}
}

void CCandidateWindow::_InvokeKeyHandler(WPARAM wParam)
{
	if (_pCandidateList != nullptr)
	{
		_pCandidateList->_InvokeKeyHandler(wParam);
	}
}

void CCandidateWindow::_HandleKey(WPARAM wParam, BYTE bSf)
{
	BYTE sf = bSf;
	WCHAR ch = WCHAR_MAX;

	if (_pTextService != nullptr)
	{
		if (bSf == SKK_NULL)
		{
			_GetChSf(wParam, ch, sf);
		}

		_pTextService->_HandleKey(0, nullptr, wParam, sf, ch);
	}
}

void CCandidateWindow::_GetChSf(WPARAM wParam, WCHAR &ch, BYTE &sf, BYTE vkoff)
{
	if (_pTextService != nullptr)
	{
		ch = _pTextService->_GetCh((BYTE)wParam, vkoff);
		sf = _pTextService->_GetSf((BYTE)wParam, ch);
	}
}
