
#include "common.h"
#include "corvustip.h"
#include "TextService.h"
#include "CandidateWindow.h"
#include "CandidateList.h"

static LPCWSTR markZWSP = L"\u200B";	//U+200B ZERO WIDTH SPACE
static LPCWSTR markAnnotation = L";";

static LPCWSTR markNo = L":";
static LPCWSTR markNoTT = L" ";
static LPCWSTR markCandEnd = L"　";
static LPCWSTR markCursor = L"|";
static LPCWSTR markRegKeyEnd = L"：";
static LPCWSTR markLinkS = L"<a>";
static LPCWSTR markLinkE = L"</a>";

BOOL CCandidateWindow::_Create(HWND hwndParent, CCandidateWindow *pCandidateWindowParent, DWORD dwUIElementId, UINT depth, BOOL reg)
{
	_hwndParent = hwndParent;
	_pCandidateWindowParent = pCandidateWindowParent;
	_depth = depth;
	_dwUIElementId = dwUIElementId;

	if(_hwndParent != NULL)
	{
		_hwnd = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL,
							   WS_POPUP | TTS_NOPREFIX | TTS_NOANIMATE | TTS_NOFADE,
							   CW_USEDEFAULT, CW_USEDEFAULT,
							   CW_USEDEFAULT, CW_USEDEFAULT,
							   _hwndParent, NULL, g_hInst, NULL);
		if(_hwnd == NULL)
		{
			return FALSE;
		}

		ZeroMemory(&ti, sizeof(TOOLINFOW));
		ti.cbSize = sizeof(TOOLINFOW);
		ti.uFlags = TTF_TRACK | TTF_PARSELINKS;
		ti.hwnd = _hwndParent;
		ti.hinst = g_hInst;
		ti.lpszText = L"";

		if(SendMessageW(_hwnd, TTM_ADDTOOLW, 0, (LPARAM)(LPTOOLINFOW)&ti) == FALSE)
		{
			return FALSE;
		}

		if(!_pTextService->c_visualstyle)
		{
			SetWindowTheme(_hwnd, L" ", L" ");
		}

		//set font
		HDC hdc = GetDC(_hwnd);
		hFont = CreateFontW(-MulDiv(_pTextService->fontpoint, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0,
			_pTextService->fontweight, _pTextService->fontitalic, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH,
			_pTextService->fontname);
		SendMessageW(_hwnd, WM_SETFONT, (WPARAM)hFont, 0);
		ReleaseDC(_hwnd, hdc);
		//DeleteObject(hFont);	// -> _End()

		//set color
		SendMessageW(_hwnd, TTM_SETTIPTEXTCOLOR, (WPARAM)RGB(0,0,0), 0);
		SendMessageW(_hwnd, TTM_SETTIPBKCOLOR, (WPARAM)RGB(255,255,255), 0);

		//set max width
		SendMessageW(_hwnd, TTM_SETMAXTIPWIDTH, 0, _pTextService->maxwidth);

		//set initial duration
		SendMessageW(_hwnd, TTM_SETDELAYTIME, TTDT_INITIAL, 0);

		//set mergin
		#define MERGIN 2
		RECT rect = {MERGIN,MERGIN,MERGIN,MERGIN};
		SendMessageW(_hwnd, TTM_SETMARGIN, 0, (LPARAM)&rect);

		//set window procedure
		WndProcDef = (WNDPROC)GetWindowLongPtrW(_hwnd, GWLP_WNDPROC);
		if(WndProcDef != 0)
		{
			SetWindowLongPtrW(_hwnd, GWLP_USERDATA, (LONG_PTR)this);
			SetWindowLongPtrW(_hwnd, GWLP_WNDPROC, (LONG_PTR)_WindowPreProc);
			SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
		}
	}

	_reg = reg;
	if(reg)
	{
		//辞書登録開始
		if(_hwnd == NULL)
		{
			regwordul = TRUE;
		}
		regword = TRUE;
		regwordtext.clear();
		regwordtextpos = 0;
		comptext.clear();
		regwordfixed = TRUE;

		_BackUpStatus();
		_ClearStatus();
	}
	
	return TRUE;
}

LRESULT CALLBACK CCandidateWindow::_WindowPreProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	CCandidateWindow *pWindowProc = (CCandidateWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if(pWindowProc != NULL)
	{
		ret = pWindowProc->_WindowProc(hwnd, uMsg, wParam, lParam);
	}
	return ret;
}

LRESULT CALLBACK CCandidateWindow::_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CallWindowProcW(WndProcDef, hwnd, uMsg, wParam, lParam);
	return 0;
}

void CCandidateWindow::_Destroy()
{
	if(_hwnd != NULL)
	{
		DestroyWindow(_hwnd);
		_hwnd = NULL;
	}
}

void CCandidateWindow::_Move(int x, int y)
{
	if(_hwnd != NULL)
	{
		_pt.x = x;
		_pt.y = y;

		SendMessageW(_hwnd, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(x, y));

		if(_pCandidateWindow != NULL)
		{
#ifdef _DEBUG
			RECT rc;
			GetClientRect(_hwnd, &rc);
			_pCandidateWindow->_Move(_pt.x, _pt.y + rc.bottom);
#else
			_pCandidateWindow->_Move(_pt.x, _pt.y);
#endif
		}
	}
}

void CCandidateWindow::_BeginUIElement()
{
	ITfUIElementMgr *pUIElementMgr;
	BOOL bShow = TRUE;

	if(!_reg)
	{
		_InitList();
	}

	_Update();

	if((_hwnd == NULL) && (_depth == 0))
	{
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
		{
			pUIElementMgr->BeginUIElement(this, &bShow, &_dwUIElementId);
			if(!bShow)
			{
				pUIElementMgr->UpdateUIElement(_dwUIElementId);
			}
			pUIElementMgr->Release();
		}
	}

	if(_hwnd == NULL)
	{
		_bShow = FALSE;
	}
	else
	{
		_bShow = bShow;
	}

	if(!IsVersion6AndOver(g_ovi))
	{
		_bShow = TRUE;
	}

	if(_bShow)
	{
		if(_hwnd != NULL)
		{
			SendMessageW(_hwnd, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
		}
	}
}

void CCandidateWindow::_EndUIElement()
{
	ITfUIElementMgr *pUIElementMgr;

	if((_hwnd == NULL) && (_depth == 0))
	{
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
		{
			pUIElementMgr->EndUIElement(_dwUIElementId);
			pUIElementMgr->Release();
		}
	}

	if(_hwnd != NULL)
	{
		SendMessageW(_hwnd, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
	}

	if(hFont != NULL)
	{
		DeleteObject(hFont);
	}

	_bShow = FALSE;
}

BOOL CCandidateWindow::_CanShowUIElement()
{
	ITfUIElementMgr *pUIElementMgr;
	BOOL bShow = TRUE;

	if(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
	{
		pUIElementMgr->BeginUIElement(this, &bShow, &_dwUIElementId);
		pUIElementMgr->EndUIElement(_dwUIElementId);
		pUIElementMgr->Release();
	}

	return bShow;
}

HRESULT CCandidateWindow::_OnKeyDown(UINT uVKey)
{
	UINT i, page, index;

	if(_pCandidateWindow != NULL && !_preEnd)
	{
		return _pCandidateWindow->_OnKeyDown(uVKey);
	}

	WCHAR ch = _pTextService->_GetCh((WPARAM)uVKey);
	BYTE sf = _pTextService->_GetSf((WPARAM)uVKey, ch);

	//辞書登録モード
	if(regword)
	{
		_OnKeyDownRegword(uVKey, sf);
		return S_OK;
	}

	switch(sf)
	{
	case SKK_CANCEL:
		if(_pCandidateList)
		{
			if(!regword)
			{
				if(_pCandidateWindowParent == NULL)
				{
					_pCandidateList->_InvokeSfHandler(SKK_CANCEL);
					_pCandidateList->_EndCandidateList();
				}
				else
				{
					if(_reg)
					{
						_RestoreStatusReg();
					}
					_PreEndReq();
					_pTextService->_HandleKey(0, NULL, 0, SKK_CANCEL);
					_EndReq();
				}
			}
			else
			{
				_pTextService->_HandleKey(0, NULL, 0, SKK_CANCEL);
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
		for(i=0; i<MAX_SELKEY_C; i++)
		{
			if(ch == (L'1' + i) ||
				(ch == _pTextService->selkey[i][0][0] && _pTextService->selkey[i][0][0] != L'\0') ||
				(ch == _pTextService->selkey[i][1][0] && _pTextService->selkey[i][1][0] != L'\0'))
			{
				GetCurrentPage(&page);
				if(i < _CandCount[page])
				{
					index = (UINT)(_pTextService->c_untilcandlist - 1) + _PageInex[page] + i;
					if(index < _pTextService->candidates.size())
					{
						if(!regword)
						{
							if(_pCandidateWindowParent == NULL)
							{
								_pTextService->candidx = index;
								_pCandidateList->_InvokeSfHandler(SKK_ENTER);
								_pCandidateList->_EndCandidateList();
							}
							else
							{
								if(_reg)
								{
									_RestoreStatusReg();
								}
								_PreEndReq();
								_pTextService->candidx = index;
								_pTextService->_HandleKey(0, NULL, 0, SKK_ENTER);
								_EndReq();
							}
						}
						else
						{
							_pTextService->candidx = index;
							_pTextService->_HandleKey(0, NULL, 0, SKK_ENTER);
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

void CCandidateWindow::_SetText(const std::wstring &text, BOOL fixed, BOOL showcandlist, BOOL showreg)
{
	//CTextService -> CCandidateList -> CCandidateWindow で入力文字列をもらう

	if(_pCandidateWindow != NULL && !_preEnd)
	{
		_pCandidateWindow->_SetText(text, fixed, showcandlist, showreg);
		return;
	}

	if(showreg)
	{
		_CreateNext(TRUE);
	}

	if(showcandlist)
	{
		_CreateNext(FALSE);
	}

	regwordfixed = fixed;

	if(fixed)
	{
		comptext.clear();
		regwordtext.insert(regwordtextpos, text);
		regwordtextpos += text.size();
	}
	else
	{
		comptext = text;
		if(comptext.empty())
		{
			regwordfixed = TRUE;
		}
	}

	_Update();
}

void CCandidateWindow::_PreEnd()
{
	_preEnd = TRUE;
}

void CCandidateWindow::_End()
{
	_preEnd = FALSE;

#ifndef _DEBUG
	if(_hwnd != NULL)
	{
		SendMessageW(_hwnd, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
	}
#endif

	if(_pCandidateWindow != NULL)
	{
		_pCandidateWindow->_Destroy();
		_pCandidateWindow->Release();
		_pCandidateWindow = NULL;
	}

	if(_hwnd == NULL)
	{
		_dwFlags = TF_CLUIE_DOCUMENTMGR | TF_CLUIE_COUNT | TF_CLUIE_SELECTION |
			TF_CLUIE_STRING | TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;
		_Update();
		_UpdateUIElement();
	}
}

void CCandidateWindow::_InitList()
{
	UINT i;

	_uShowedCount = (UINT)_pTextService->c_untilcandlist - 1;
	_uCount = (UINT)_pTextService->candidates.size() - _uShowedCount;

	_CandStr.clear();
	for(i=0; i<_uCount; i++)
	{
		_CandStr.push_back(_pTextService->selkey[(i % MAX_SELKEY)][0]);
		_CandStr[i].append(markNo + _pTextService->candidates[ _uShowedCount + i ].first.first);

		if(_pTextService->c_annotation &&
			!_pTextService->candidates[ _uShowedCount + i ].first.second.empty())
		{
			_CandStr[i].append(markAnnotation +
				_pTextService->candidates[ _uShowedCount + i ].first.second);
		}
	}

	_uPageCnt = ((_uCount - (_uCount % MAX_SELKEY)) / MAX_SELKEY) + ((_uCount % MAX_SELKEY) == 0 ? 0 : 1);

	_PageInex.clear();
	_CandCount.clear();
	for(i=0; i<_uPageCnt; i++)
	{
		_PageInex.push_back(i * MAX_SELKEY);
		_CandCount.push_back( (i < (_uPageCnt - 1)) ? MAX_SELKEY :
			(((_uCount % MAX_SELKEY) == 0) ? MAX_SELKEY : (_uCount % MAX_SELKEY)) );
	}

	_uIndex = 0;

	_dwFlags = TF_CLUIE_DOCUMENTMGR | TF_CLUIE_COUNT | TF_CLUIE_SELECTION |
		TF_CLUIE_STRING | TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;
}

void CCandidateWindow::_UpdateUIElement()
{
	if(!_bShow)
	{
		ITfUIElementMgr *pUIElementMgr;
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
		{
			pUIElementMgr->UpdateUIElement(_dwUIElementId);
			pUIElementMgr->Release();
		}
	}
}

void CCandidateWindow::_NextPage()
{
	UINT uOldPage, uNewPage;

	GetCurrentPage(&uOldPage);
	uNewPage = uOldPage + 1;
	if(uNewPage >= _uPageCnt)
	{
		if(_pCandidateList)
		{
			if((_pTextService->_dwActiveFlags & TF_TMF_IMMERSIVEMODE) && (_hwnd != NULL))
			{
				//辞書登録せずに▽モードにする
				_pCandidateList->_InvokeSfHandler(SKK_CANCEL);
				_pCandidateList->_EndCandidateList();
			}
			else
			{
				if(_hwnd == NULL)
				{
					regwordul = TRUE;
				}

				if(!regword)
				{
					//辞書登録開始
					regword = TRUE;
					regwordtext.clear();
					regwordtextpos = 0;
					comptext.clear();
					regwordfixed = TRUE;

					_BackUpStatus();
					_ClearStatus();
				}
				else
				{
					_CreateNext(TRUE);
				}

				_Update();
			}
			return;
		}
	}

	_uIndex = _PageInex[uNewPage];

	_dwFlags = TF_CLUIE_SELECTION;
	if(uNewPage != uOldPage)
	{
		_dwFlags |= TF_CLUIE_CURRENTPAGE;
	}

	_Update();
	_UpdateUIElement();
}

void CCandidateWindow::_PrevPage()
{
	UINT uOldPage, uNewPage;

	GetCurrentPage(&uOldPage);
	if(uOldPage > 0)
	{
		uNewPage = uOldPage - 1;
	}
	else
	{
		if(_pCandidateList)
		{
			if(!regword)
			{
				if(_pTextService->c_untilcandlist == 1)
				{
					if(_pCandidateWindowParent == NULL)
					{
						_pCandidateList->_InvokeSfHandler(SKK_CANCEL);
						_pCandidateList->_EndCandidateList();
					}
					else
					{
						if(_reg)
						{
							_RestoreStatusReg();
						}
						_PreEndReq();
						_pTextService->_HandleKey(0, NULL, 0, SKK_CANCEL);
						_EndReq();
					}
				}
				else
				{
					if(_pCandidateWindowParent == NULL)
					{
						_pTextService->candidx = _pTextService->c_untilcandlist - 1;
						_pCandidateList->_InvokeSfHandler(SKK_PREV_CAND);
						_pCandidateList->_EndCandidateList();
					}
					else
					{
						if(_reg)
						{
							_RestoreStatusReg();
						}
						_PreEndReq();
						_pTextService->candidx = _pTextService->c_untilcandlist - 1;
						_pTextService->_HandleKey(0, NULL, 0, SKK_PREV_CAND);
						_EndReq();
					}
				}
			}
			else
			{
				if(_pTextService->c_untilcandlist == 1)
				{
					_pTextService->_HandleKey(0, NULL, 0, SKK_CANCEL);
				}
				else
				{
					_pTextService->candidx = _pTextService->c_untilcandlist - 1;
					_pTextService->_HandleKey(0, NULL, 0, SKK_PREV_CAND);
				}
				
				_Update();
				_UpdateUIElement();
			}
		}
		return;
	}

	_uIndex = _PageInex[uNewPage];

	_dwFlags = TF_CLUIE_SELECTION;
	if(uNewPage != uOldPage)
	{
		_dwFlags |= TF_CLUIE_CURRENTPAGE;
	}

	_Update();
	_UpdateUIElement();
}

void CCandidateWindow::_OnKeyDownRegword(UINT uVKey, BYTE sf)
{
	HANDLE hCB;
	PWCHAR pwCB;
	std::wstring s;
	std::wstring regwordtextconv;
	std::wstring regwordtextcandidate;
	std::wstring regwordtextannotation;
	std::wsmatch result;

	//確定していないとき
	if(!regwordfixed)
	{
		_pTextService->showcandlist = FALSE;	//候補一覧表示をループさせる
		_pTextService->_HandleKey(0, NULL, (WPARAM)uVKey, SKK_NULL);
		_Update();
		return;
	}

	switch(sf)
	{
	case SKK_JMODE:
		_pTextService->_HandleKey(0, NULL, 0, SKK_JMODE);
		break;

	case SKK_ENTER:
		_RestoreStatusReg();
		_ClearStatusReg();

		regwordfixed = FALSE;
		regwordul = FALSE;
		regword = FALSE;

		if(regwordtext.empty())	//空のときはキャンセル扱い
		{
			if(!_reg)
			{
				_InitList();
				_uIndex = _PageInex[_PageInex.size() - 1];
				_Update();
			}
			else
			{
				if(_pCandidateWindowParent == NULL)
				{
					if(_pTextService->candidates.empty())
					{
						_pCandidateList->_InvokeSfHandler(SKK_CANCEL);
					}
					else
					{
						_pCandidateList->_InvokeSfHandler(SKK_PREV_CAND);
					}
					_pCandidateList->_EndCandidateList();
				}
				else
				{
					if(_reg)
					{
						_RestoreStatusReg();
					}
					_PreEndReq();
					if(_pTextService->candidates.empty())
					{
						_pTextService->_HandleKey(0, NULL, 0, SKK_CANCEL);
					}
					else
					{
						_pTextService->_HandleKey(0, NULL, 0, SKK_PREV_CAND);
					}
					_EndReq();
				}
			}
		}
		else
		{
			//候補と注釈を、行頭以外の最後のセミコロンで分割
			if(std::regex_search(regwordtext, result, std::wregex(L".+;")))
			{
				regwordtextcandidate = result.str().substr(0, result.str().size() - 1);
				regwordtextannotation = result.suffix();
			}
			else
			{
				regwordtextcandidate = regwordtext;
				regwordtextannotation.clear();
			}

			//数値変換タイプ0～3の候補は#を数値にした見出し語が表示用 それ以外は見出し語そのまま
			if(std::regex_match(regwordtextcandidate, std::wregex(L".*#[0-3].*")))
			{
				_pTextService->_ConvNum(regwordtextconv, _pTextService->searchkeyorg, regwordtextcandidate);
			}
			else
			{
				regwordtextconv = regwordtextcandidate;
				_pTextService->searchkey = _pTextService->searchkeyorg;
			}

			_pTextService->candidates.push_back(CANDIDATE(
				CANDIDATEBASE(regwordtextconv, regwordtextannotation),
				(CANDIDATEBASE(regwordtextcandidate, regwordtextannotation))));
			_pTextService->candidx = _pTextService->candidates.size() - 1;

			regwordtext.clear();
			regwordtextpos = 0;

			if(_pCandidateWindowParent == NULL)
			{
				_pCandidateList->_InvokeSfHandler(SKK_ENTER);
				_pCandidateList->_EndCandidateList();
			}
			else
			{
				_PreEndReq();
				_pTextService->_HandleKey(0, NULL, 0, SKK_ENTER);
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
			_uIndex = _PageInex[_PageInex.size() - 1];
			_Update();
			_UpdateUIElement();
		}
		else
		{
			if(_pCandidateWindowParent == NULL)
			{
				if(_pTextService->candidates.empty())
				{
					_pCandidateList->_InvokeSfHandler(SKK_CANCEL);
				}
				else
				{
					_pCandidateList->_InvokeSfHandler(SKK_PREV_CAND);
				}
				_pCandidateList->_EndCandidateList();
			}
			else
			{
				_PreEndReq();
				if(_pTextService->candidates.empty())
				{
					_pTextService->_HandleKey(0, NULL, 0, SKK_CANCEL);
				}
				else
				{
					_pTextService->_HandleKey(0, NULL, 0, SKK_PREV_CAND);
				}
				_EndReq();
			}
		}
		break;

	//結合文字は考慮しない

	case SKK_BACK:
		if(comptext.empty() && regwordtextpos > 0 && regwordtext.size() > 0)
		{
			if(regwordtext.size() >= 2 && regwordtextpos >= 2 &&
				_pTextService->_IsSurrogatePair(regwordtext[regwordtextpos - 2], regwordtext[regwordtextpos - 1]))
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
			if(regwordtext.size() >= regwordtextpos + 2 &&
				_pTextService->_IsSurrogatePair(regwordtext[regwordtextpos + 0], regwordtext[regwordtextpos + 1]))
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
			if(regwordtext.size() >= 2 && regwordtextpos >= 2 &&
				_pTextService->_IsSurrogatePair(regwordtext[regwordtextpos - 2], regwordtext[regwordtextpos - 1]))
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
			if(regwordtext.size() >= regwordtextpos + 2 &&
				_pTextService->_IsSurrogatePair(regwordtext[regwordtextpos + 0], regwordtext[regwordtextpos + 1]))
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
			if(OpenClipboard(NULL))
			{
				hCB = GetClipboardData(CF_UNICODETEXT);
				if(hCB != NULL) {
					pwCB = (PWCHAR)GlobalLock(hCB);
					if(pwCB != NULL) {
						s.assign(pwCB);
						s = std::regex_replace(s, std::wregex(L"\t|\r|\n"), std::wstring(L""));
						regwordtext.insert(regwordtextpos, s);
						regwordtextpos += s.size();
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
		_pTextService->_HandleKey(0, NULL, (WPARAM)uVKey, SKK_NULL);
		_Update();
		break;
	}
}

std::wstring CCandidateWindow::_EscapeTags(const std::wstring &text)
{
	return markZWSP + std::regex_replace(text, std::wregex(L"<"), std::wstring(L"<") + markZWSP) + markZWSP;
}

void CCandidateWindow::_Update()
{
	WCHAR selkey[2];
	WCHAR strPage[32];
	UINT i, page, count;

	if(regwordul)
	{
		disptext.clear();
		_snwprintf_s(strPage, _TRUNCATE, L"[%u] ", _depth);
		disptext.append(strPage + searchkey_bak);
		disptext.append(markRegKeyEnd + regwordtext.substr(0, regwordtextpos));
		if(!comptext.empty())
		{
			disptext.append(markCursor + comptext);
		}
		disptext.append(markCursor + regwordtext.substr(regwordtextpos));

		_dwFlags = TF_CLUIE_COUNT | TF_CLUIE_SELECTION | TF_CLUIE_STRING |
			TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;
		_UpdateUIElement();
	}
	else
	{
		if(regword)
		{
			disptext.clear();
			_snwprintf_s(strPage, _TRUNCATE, L"[%u] ", _depth);
			disptext.append(strPage);
			disptext.append(markLinkS + _EscapeTags(searchkey_bak) + markLinkE);
			disptext.append(markRegKeyEnd + _EscapeTags(regwordtext.substr(0, regwordtextpos)));
			disptext.append(markLinkS + _EscapeTags(comptext) + markLinkE);
			disptext.append(markCursor + _EscapeTags(regwordtext.substr(regwordtextpos)));
		}
		else
		{
			GetCurrentPage(&page);
			count = 0;
			for(i=0; i<page; i++)
			{
				count += _CandCount[i];
			}

			disptext.clear();
			selkey[1] = L'\0';
			for(i=0; i<_CandCount[page]; i++)
			{
				disptext.append(markLinkS + _EscapeTags(_pTextService->selkey[(i % MAX_SELKEY_C)][0]) + markLinkE);

				disptext.append(markNoTT +
					_EscapeTags(_pTextService->candidates[ count + _uShowedCount + i ].first.first));

				if(_pTextService->c_annotation &&
					!_pTextService->candidates[ count + _uShowedCount + i ].first.second.empty())
				{
					disptext.append(markAnnotation +
						_EscapeTags(_pTextService->candidates[ count + _uShowedCount + i ].first.second));
				}

				disptext.append(markCandEnd);
			}

			_snwprintf_s(strPage, _TRUNCATE, L"(%u/%u)", page + 1, _uPageCnt);
			disptext.append(strPage);
		}

		ti.lpszText = (LPWSTR)disptext.c_str();
		if(_hwnd != NULL)
		{
			SendMessageW(_hwnd, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
		}
	}
}

void CCandidateWindow::_BackUpStatus()
{
	inputmode_bak = _pTextService->inputmode;
	abbrevmode_bak = _pTextService->abbrevmode;
	kana_bak = _pTextService->kana;
	accompidx_bak = _pTextService->accompidx;
	searchkey_bak = _pTextService->searchkey;
	searchkeyorg_bak = _pTextService->searchkeyorg;
	candidates_bak = _pTextService->candidates;
	candidx_bak = _pTextService->candidx;
}

void CCandidateWindow::_ClearStatus()
{
	//_pTextService->inputmode //そのまま
	_pTextService->abbrevmode = FALSE;
	_pTextService->kana.clear();
	_pTextService->accompidx = 0;
	_pTextService->searchkey.clear();
	_pTextService->searchkeyorg.clear();
	_pTextService->candidates.clear();
	_pTextService->candidx = 0;
	_pTextService->showcandlist = FALSE;
	_pTextService->showentry = FALSE;
	_pTextService->inputkey = FALSE;
}

void CCandidateWindow::_RestoreStatusReg()
{
	_pTextService->inputmode = inputmode_bak;
	_pTextService->_UpdateLanguageBar();
	_pTextService->abbrevmode = abbrevmode_bak;
	_pTextService->kana = kana_bak;
	_pTextService->accompidx = accompidx_bak;
	_pTextService->searchkey = searchkey_bak;
	_pTextService->searchkeyorg = searchkeyorg_bak;
	_pTextService->candidates = candidates_bak;
	_pTextService->candidx = candidx_bak;
	_pTextService->showcandlist = TRUE;
	_pTextService->showentry = TRUE;
	_pTextService->inputkey = TRUE;
}

void CCandidateWindow::_ClearStatusReg()
{
	inputmode_bak = im_default;
	abbrevmode_bak = FALSE;
	kana_bak.clear();
	accompidx_bak = 0;
	searchkey_bak.clear();
	searchkeyorg_bak.clear();
	candidates_bak.clear();
	candidx_bak = 0;
}

void CCandidateWindow::_PreEndReq()
{
	if(_pCandidateWindowParent != NULL && !_preEnd)
	{
		_pCandidateWindowParent->_PreEnd();
	}
}

void CCandidateWindow::_EndReq()
{
	if(_pCandidateWindowParent != NULL && !_preEnd)
	{
		_pCandidateWindowParent->_End();
	}
}

void CCandidateWindow::_CreateNext(BOOL reg)
{
	RECT rc;

	GetClientRect(_hwnd, &rc);
	_pCandidateWindow = new CCandidateWindow(_pTextService);
	if(_pCandidateWindow)
	{
		_pCandidateWindow->_Create(_hwndParent, this, _dwUIElementId, _depth + 1, reg);

#ifdef _DEBUG
		_pCandidateWindow->_Move(_pt.x, _pt.y + rc.bottom);
#else
		_pCandidateWindow->_Move(_pt.x, _pt.y);
#endif

		_pCandidateWindow->_BeginUIElement();

#ifndef _DEBUG
		if(_hwnd != NULL)
		{
			SendMessageW(_hwnd, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
		}
#endif
	}
}
