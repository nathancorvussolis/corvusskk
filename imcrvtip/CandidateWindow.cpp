
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "CandidateWindow.h"
#include "InputModeWindow.h"

BOOL CCandidateWindow::_Create(HWND hwndParent, CCandidateWindow *pCandidateWindowParent, DWORD dwUIElementId, UINT depth, BOOL reg, BOOL comp)
{
	_hwndParent = hwndParent;
	_pCandidateWindowParent = pCandidateWindowParent;
	_depth = depth;
	_dwUIElementId = dwUIElementId;

	if(_hwndParent != NULL)
	{
		WNDCLASSEXW wc;
		wc.cbSize = sizeof(wc);
		wc.style = CS_IME | CS_VREDRAW | CS_HREDRAW | CS_DROPSHADOW;
		wc.lpfnWndProc = DefWindowProcW;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(LONG_PTR);
		wc.hInstance = g_hInst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = TextServiceDesc;
		wc.hIconSm = NULL;
		RegisterClassExW(&wc);

		_hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
			TextServiceDesc, NULL, WS_POPUP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			_hwndParent, NULL, g_hInst, NULL);

		if(_hwnd == NULL)
		{
			return FALSE;
		}

		WndProcDef = (WNDPROC)GetWindowLongPtrW(_hwnd, GWLP_WNDPROC);
		if(WndProcDef != 0)
		{
			SetWindowLongPtrW(_hwnd, GWLP_USERDATA, (LONG_PTR)this);
			SetWindowLongPtrW(_hwnd, GWLP_WNDPROC, (LONG_PTR)_WindowPreProc);
			SetWindowPos(_hwnd, NULL, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}

		hFont = _pTextService->hFont;

		if(_pTextService->cx_drawapi && _pTextService->_pD2DFactory != NULL)
		{
			_drawtext_option = _pTextService->_drawtext_option;
			_pD2DFactory = _pTextService->_pD2DFactory;
			_pD2DFactory->AddRef();
			_pD2DDCRT = _pTextService->_pD2DDCRT;
			_pD2DDCRT->AddRef();
			for(int i = 0; i < DISPLAY_COLOR_NUM; i++)
			{
				_pD2DBrush[i] = _pTextService->_pD2DBrush[i];
				_pD2DBrush[i]->AddRef();
			}
			_pDWFactory = _pTextService->_pDWFactory;
			_pDWFactory->AddRef();
			_pDWTF = _pTextService->_pDWTF;
			_pDWTF->AddRef();
		}
	}

	if(_hwnd != NULL && _pTextService->_ShowInputMode)
	{
		try
		{
			_pInputModeWindow = new CInputModeWindow();
			if(!_pInputModeWindow->_Create(_pTextService, NULL, TRUE, _hwnd))
			{
				_pInputModeWindow->_Destroy();
				SafeRelease(&_pInputModeWindow);
			}
		}
		catch(...)
		{
		}
	}

	_reg = reg;
	_comp = comp;

	candidates = _pTextService->candidates;
	candidx = _pTextService->candidx;
	searchkey = _pTextService->searchkey;

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

LRESULT CALLBACK CCandidateWindow::_WindowPreProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	CCandidateWindow *pWindowProc = (CCandidateWindow*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
	if(pWindowProc != NULL)
	{
		ret = pWindowProc->_WindowProc(hWnd, uMsg, wParam, lParam);
	}
	return ret;
}

LRESULT CALLBACK CCandidateWindow::_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT:
		_WindowProcPaint(hWnd);
		break;
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void CCandidateWindow::_Destroy()
{
	if(_hwnd != NULL)
	{
		DestroyWindow(_hwnd);
		_hwnd = NULL;
	}

	if(_pInputModeWindow != NULL)
	{
		_pInputModeWindow->_Destroy();
	}
	SafeRelease(&_pInputModeWindow);

	SafeRelease(&_pDWTF);
	SafeRelease(&_pDWFactory);
	for(int i = 0; i < DISPLAY_COLOR_NUM; i++)
	{
		SafeRelease(&_pD2DBrush[i]);
	}
	SafeRelease(&_pD2DDCRT);
	SafeRelease(&_pD2DFactory);
}

void CCandidateWindow::_Move(LPCRECT lpr)
{
	if(_hwnd != NULL)
	{
		_rect = *lpr;

		_CalcWindowRect();

		if(_pCandidateWindow != NULL)
		{
#ifdef _DEBUG
			RECT rc;
			GetClientRect(_hwnd, &rc);
			rc.left = _rect.left;
			rc.top += _rect.bottom;
			rc.right = _rect.right;
			rc.bottom += _rect.bottom;
			_pCandidateWindow->_Move(&rc);
#else
			_pCandidateWindow->_Move(&_rect);
#endif
		}
	}
}

void CCandidateWindow::_BeginUIElement()
{
	BOOL bShow = TRUE;

	if(!_reg)
	{
		_InitList();
	}

	_Update();

	if((_hwnd == NULL) && (_depth == 0))
	{
		ITfUIElementMgr *pUIElementMgr;
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_PPV_ARGS(&pUIElementMgr)) == S_OK)
		{
			pUIElementMgr->BeginUIElement(this, &bShow, &_dwUIElementId);
			if(!bShow)
			{
				pUIElementMgr->UpdateUIElement(_dwUIElementId);
			}
			SafeRelease(&pUIElementMgr);
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

	if(_bShow)
	{
		if(_hwnd != NULL)
		{
			SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);

			if(_reg)
			{
				if(_pInputModeWindow != NULL)
				{
					_pInputModeWindow->_Show(TRUE);
				}
			}

			if(_depth == 0)
			{
				NotifyWinEvent(EVENT_OBJECT_IME_SHOW, _hwnd, OBJID_CLIENT, CHILDID_SELF);
			}
		}
	}
}

void CCandidateWindow::_EndUIElement()
{
	if((_hwnd == NULL) && (_depth == 0))
	{
		ITfUIElementMgr *pUIElementMgr;
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_PPV_ARGS(&pUIElementMgr)) == S_OK)
		{
			pUIElementMgr->EndUIElement(_dwUIElementId);
			SafeRelease(&pUIElementMgr);
		}
	}

	if(_hwnd != NULL)
	{
		SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);

		if(_pInputModeWindow != NULL)
		{
			_pInputModeWindow->_Show(FALSE);
		}

		if(_depth == 0)
		{
			NotifyWinEvent(EVENT_OBJECT_IME_HIDE, _hwnd, OBJID_CLIENT, CHILDID_SELF);
		}
	}

	_bShow = FALSE;
}

BOOL CCandidateWindow::_CanShowUIElement()
{
	BOOL bShow = TRUE;

	ITfUIElementMgr *pUIElementMgr;
	if(_pTextService->_GetThreadMgr()->QueryInterface(IID_PPV_ARGS(&pUIElementMgr)) == S_OK)
	{
		pUIElementMgr->BeginUIElement(this, &bShow, &_dwUIElementId);
		pUIElementMgr->EndUIElement(_dwUIElementId);
		SafeRelease(&pUIElementMgr);
	}

	return bShow;
}

void CCandidateWindow::_Redraw()
{
	if(_hwnd != NULL)
	{
		InvalidateRect(_hwnd, NULL, FALSE);
		UpdateWindow(_hwnd);

		if(_pInputModeWindow != NULL)
		{
			_pInputModeWindow->_Redraw();
		}
	}
}

HRESULT CCandidateWindow::_OnKeyDown(UINT uVKey)
{
	UINT i, page, index;
	WCHAR ch;
	BYTE sf;

	if(_pCandidateWindow != NULL && !_preEnd)
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

	//複数動的補完
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
		if(_pCandidateList != NULL)
		{
			if(!regword)
			{
				if(_pCandidateWindowParent == NULL)
				{
					_EndCandidateList(SKK_CANCEL);
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
							if(_pCandidateWindowParent == NULL)
							{
								_pTextService->candidx = index;
								_EndCandidateList(SKK_ENTER);
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
		SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}
	if(_pInputModeWindow != NULL)
	{
		_pInputModeWindow->_Show(TRUE);
	}
#endif

	if(_pCandidateWindow != NULL)
	{
		_pCandidateWindow->_Destroy();
	}
	SafeRelease(&_pCandidateWindow);

	if(_hwnd == NULL)
	{
		_dwFlags = TF_CLUIE_DOCUMENTMGR | TF_CLUIE_COUNT | TF_CLUIE_SELECTION |
			TF_CLUIE_STRING | TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;
		_Update();
		_UpdateUIElement();
	}
}

void CCandidateWindow::_UpdateComp()
{
	_comp = TRUE;
	candidates = _pTextService->candidates;
	candidx = _pTextService->candidx;
	searchkey = _pTextService->searchkey;

	_InitList();
	_Update();
	_UpdateUIElement();
}

void CCandidateWindow::_InitList()
{
	UINT i;

	if(!_comp)
	{
		_uShowedCount = (UINT)_pTextService->cx_untilcandlist - 1;
	}
	else
	{
		_uShowedCount = 0;
	}
	_uCount = (UINT)candidates.size() - _uShowedCount;

	_CandStr.clear();
	for(i = 0; i < _uCount; i++)
	{
		if(!_comp)
		{
			_CandStr.push_back(_pTextService->selkey[(i % MAX_SELKEY)][0]);
			_CandStr[i].append(markNo);
		}
		else
		{
			_CandStr.push_back(L"");
		}

		_CandStr[i].append(candidates[_uShowedCount + i].first.first);

		if(_pTextService->cx_annotation &&
			!candidates[_uShowedCount + i].first.second.empty())
		{
			if(!_comp)
			{
				_CandStr[i].append(markAnnotation);
			}
			else
			{
				_CandStr[i].append(markSP);
			}
			_CandStr[i].append(candidates[_uShowedCount + i].first.second);
		}
	}

	_uPageCnt = ((_uCount - (_uCount % MAX_SELKEY)) / MAX_SELKEY) + ((_uCount % MAX_SELKEY) == 0 ? 0 : 1);

	_PageIndex.clear();
	_CandCount.clear();
	for(i = 0; i < _uPageCnt; i++)
	{
		_PageIndex.push_back(i * MAX_SELKEY);
		_CandCount.push_back((i < (_uPageCnt - 1)) ? MAX_SELKEY :
			(((_uCount % MAX_SELKEY) == 0) ? MAX_SELKEY : (_uCount % MAX_SELKEY)));
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
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_PPV_ARGS(&pUIElementMgr)) == S_OK)
		{
			pUIElementMgr->UpdateUIElement(_dwUIElementId);
			SafeRelease(&pUIElementMgr);
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
		if(_pCandidateList != NULL)
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

				if(_pInputModeWindow != NULL)
				{
					_pInputModeWindow->_Show(TRUE);
				}
			}
			else
			{
				_CreateNext(TRUE);
			}

			_Update();
			return;
		}
	}

	_uIndex = _PageIndex[uNewPage];

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
		if(_pCandidateList != NULL)
		{
			if(!regword)
			{
				if(_pTextService->cx_untilcandlist == 1)
				{
					if(_pCandidateWindowParent == NULL)
					{
						_EndCandidateList(SKK_CANCEL);
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
					if(_pCandidateWindowParent == NULL)
					{
						_pTextService->candidx = _pTextService->cx_untilcandlist - 1;
						_EndCandidateList(SKK_PREV_CAND);
					}
					else
					{
						if(_reg)
						{
							_RestoreStatusReg();
						}
						_PreEndReq();
						_pTextService->candidx = _pTextService->cx_untilcandlist - 1;
						_HandleKey(0, SKK_PREV_CAND);
						_EndReq();
					}
				}
			}
			else
			{
				if(_pTextService->cx_untilcandlist == 1)
				{
					_HandleKey(0, SKK_CANCEL);
				}
				else
				{
					_pTextService->candidx = _pTextService->cx_untilcandlist - 1;
					_HandleKey(0, SKK_PREV_CAND);
				}

				_Update();
				_UpdateUIElement();
			}
		}
		return;
	}

	_uIndex = _PageIndex[uNewPage];

	_dwFlags = TF_CLUIE_SELECTION;
	if(uNewPage != uOldPage)
	{
		_dwFlags |= TF_CLUIE_CURRENTPAGE;
	}

	_Update();
	_UpdateUIElement();
}

void CCandidateWindow::_NextComp()
{
	UINT uOldPage, uNewPage;

	GetCurrentPage(&uOldPage);

	if(_uIndex + 1 >= _uCount)
	{
		return;
	}

	_InvokeSfHandler(SKK_NEXT_COMP);

	candidx++;

	_uIndex++;
	GetCurrentPage(&uNewPage);

	_dwFlags = TF_CLUIE_SELECTION;
	if(uNewPage != uOldPage)
	{
		_dwFlags |= TF_CLUIE_CURRENTPAGE;
	}

	_Update();
	_UpdateUIElement();
}

void CCandidateWindow::_PrevComp()
{
	UINT uOldPage, uNewPage;

	GetCurrentPage(&uOldPage);

	if(_uIndex == 0)
	{
		if(_pTextService->cx_stacompmulti && !_pTextService->cx_dyncompmulti)
		{
			_InvokeSfHandler(SKK_PREV_COMP);
			return;
		}
	}

	_InvokeSfHandler(SKK_PREV_COMP);

	if(_uIndex == 0)
	{
		candidx = (size_t)-1;
		_InitList();
		_Update();
		_UpdateUIElement();
		return;
	}

	candidx--;

	_uIndex--;
	GetCurrentPage(&uNewPage);

	_dwFlags = TF_CLUIE_SELECTION;
	if(uNewPage != uOldPage)
	{
		_dwFlags |= TF_CLUIE_CURRENTPAGE;
	}

	_Update();
	_UpdateUIElement();
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

		if(_pInputModeWindow != NULL)
		{
			_pInputModeWindow->_Redraw();
		}
		return;
	}

	if(_pTextService->_IsKeyVoid(ch, (BYTE)uVKey))
	{
		_pTextService->_UpdateLanguageBar();

		if(_pInputModeWindow != NULL)
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

				if(_pInputModeWindow != NULL)
				{
					_pInputModeWindow->_Show(FALSE);
				}
			}
			else
			{
				if(_pCandidateWindowParent == NULL)
				{
					if(_pTextService->candidates.empty())
					{
						_EndCandidateList(SKK_CANCEL);
					}
					else
					{
						_EndCandidateList(SKK_PREV_CAND);
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

			if(_pCandidateWindowParent == NULL)
			{
				_EndCandidateList(SKK_ENTER);
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

			if(_pInputModeWindow != NULL)
			{
				_pInputModeWindow->_Show(FALSE);
			}
		}
		else
		{
			if(_pCandidateWindowParent == NULL)
			{
				if(_pTextService->candidates.empty())
				{
					_EndCandidateList(SKK_CANCEL);
				}
				else
				{
					_EndCandidateList(SKK_PREV_CAND);
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
			if(OpenClipboard(NULL))
			{
				HANDLE hCB = GetClipboardData(CF_UNICODETEXT);
				if(hCB != NULL)
				{
					PWCHAR pwCB = (PWCHAR)GlobalLock(hCB);
					if(pwCB != NULL)
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

		if(_pInputModeWindow != NULL)
		{
			_pInputModeWindow->_Redraw();
		}
		break;
	}
}

void CCandidateWindow::_Update()
{
	if(regwordul || regword)
	{
		disptext = _MakeRegWordString();
	}

	if(regwordul)
	{
		_dwFlags = TF_CLUIE_COUNT | TF_CLUIE_SELECTION | TF_CLUIE_STRING |
			TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;
		_UpdateUIElement();
	}
	else
	{
		if(_hwnd != NULL)
		{
			_CalcWindowRect();

			InvalidateRect(_hwnd, NULL, FALSE);
			UpdateWindow(_hwnd);

			if(_pInputModeWindow != NULL)
			{
				_pInputModeWindow->_Redraw();
			}
		}
	}
}

void CCandidateWindow::_EndCandidateList(BYTE sf)
{
	_InvokeSfHandler(sf);

	if(_pTextService != NULL)
	{
		_pTextService->showcandlist = FALSE;
	}

	//直後に複数動的補完を表示する場合使い回しする
	//_InvokeSfHandler() ---> _UpdateComp() -> _comp = TRUE
	if(!_comp)
	{
		if(_pCandidateList != NULL)
		{
			_pCandidateList->_EndCandidateList();
		}
	}
}

void CCandidateWindow::_InvokeSfHandler(BYTE sf)
{
	if(_pCandidateList != NULL)
	{
		_pCandidateList->_InvokeSfHandler(sf);
	}
}

void CCandidateWindow::_InvokeKeyHandler(UINT uVKey)
{
	if(_pCandidateList != NULL)
	{
		_pCandidateList->_InvokeKeyHandler(uVKey);
	}
}

void CCandidateWindow::_HandleKey(WPARAM wParam, BYTE bSf)
{
	if(_pTextService != NULL)
	{
		_pTextService->_HandleKey(0, NULL, wParam, bSf);
	}
}

void CCandidateWindow::_GetChSf(UINT uVKey, WCHAR &ch, BYTE &sf, BYTE vkoff)
{
	if(_pTextService != NULL)
	{
		ch = _pTextService->_GetCh(uVKey, vkoff);
		sf = _pTextService->_GetSf(uVKey, ch);
	}
}

void CCandidateWindow::_BackUpStatus()
{
	inputmode_bak = _pTextService->inputmode;
	abbrevmode_bak = _pTextService->abbrevmode;
	kana_bak = _pTextService->kana;
	okuriidx_bak = _pTextService->okuriidx;
	cursoridx_bak = _pTextService->cursoridx;
	searchkey_bak = _pTextService->searchkey;
	searchkeyorg_bak = _pTextService->searchkeyorg;
	candidates_bak = _pTextService->candidates;
	candidx_bak = _pTextService->candidx;
	candorgcnt_bak = _pTextService->candorgcnt;
}

void CCandidateWindow::_ClearStatus()
{
	//_pTextService->inputmode //そのまま
	_pTextService->abbrevmode = FALSE;
	_pTextService->kana.clear();
	_pTextService->okuriidx = 0;
	_pTextService->cursoridx = 0;
	_pTextService->searchkey.clear();
	_pTextService->searchkeyorg.clear();
	_pTextService->candidates.clear();
	_pTextService->candidx = 0;
	_pTextService->candorgcnt = 0;
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
	_pTextService->okuriidx = okuriidx_bak;
	_pTextService->cursoridx = cursoridx_bak;
	_pTextService->searchkey = searchkey_bak;
	_pTextService->searchkeyorg = searchkeyorg_bak;
	_pTextService->candidates = candidates_bak;
	_pTextService->candidx = candidx_bak;
	_pTextService->candorgcnt = candorgcnt_bak;
	_pTextService->showcandlist = TRUE;
	_pTextService->showentry = TRUE;
	_pTextService->inputkey = TRUE;
}

void CCandidateWindow::_ClearStatusReg()
{
	inputmode_bak = im_default;
	abbrevmode_bak = FALSE;
	kana_bak.clear();
	okuriidx_bak = 0;
	cursoridx_bak = 0;
	searchkey_bak.clear();
	searchkeyorg_bak.clear();
	candidates_bak.clear();
	candidx_bak = 0;
	candorgcnt_bak = 0;
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
	try
	{
		_pCandidateWindow = new CCandidateWindow(_pTextService, _pCandidateList);
		_pCandidateWindow->_Create(_hwndParent, this, _dwUIElementId, _depth + 1, reg, FALSE);

#ifdef _DEBUG
		RECT rc;
		GetClientRect(_hwnd, &rc);
		rc.left = _rect.left;
		rc.top += _rect.bottom;
		rc.right = _rect.right;
		rc.bottom += _rect.bottom;
		_pCandidateWindow->_Move(&rc);
#else
		_pCandidateWindow->_Move(&_rect);
#endif
		_pCandidateWindow->_BeginUIElement();

#ifndef _DEBUG
		if(_hwnd != NULL)
		{
			SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);
		}

		if(_pInputModeWindow != NULL)
		{
			_pInputModeWindow->_Show(FALSE);
		}
#endif
	}
	catch(...)
	{
	}
}
