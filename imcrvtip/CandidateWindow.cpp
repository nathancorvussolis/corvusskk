
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "CandidateWindow.h"
#include "InputModeWindow.h"

#define NEXT_MARGIN_Y 4

BOOL CCandidateWindow::_Create(HWND hwndParent, CCandidateWindow *pCandidateWindowParent, DWORD dwUIElementId, UINT depth, int mode)
{
	_hwndParent = hwndParent;
	_pCandidateWindowParent = pCandidateWindowParent;
	_depth = depth;
	_dwUIElementId = dwUIElementId;

	if(_hwndParent != nullptr)
	{
		_hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
			CandidateWindowClass, L"", WS_POPUP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			_hwndParent, nullptr, g_hInst, this);

		if(_hwnd == nullptr)
		{
			return FALSE;
		}

		hFont = _pTextService->hFont;

		if(_pTextService->cx_drawapi && _pTextService->_pD2DFactory != nullptr)
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

	if(_hwnd != nullptr && _pTextService->_ShowInputMode)
	{
		try
		{
			_pInputModeWindow = new CInputModeWindow();
			if(!_pInputModeWindow->_Create(_pTextService, nullptr, TRUE, _hwnd))
			{
				_pInputModeWindow->_Destroy();
				SafeRelease(&_pInputModeWindow);
			}
		}
		catch(...)
		{
		}
	}

	_mode = mode;

	candidates = _pTextService->candidates;
	candidx = _pTextService->candidx;
	candorgcnt =  _pTextService->candorgcnt;
	searchkey = _pTextService->searchkey;
	searchkeyorg = _pTextService->searchkeyorg;

	if((mode == wm_register) || (mode == wm_delete))
	{
		if(_hwnd == nullptr)
		{
			_ulsingle = TRUE;
		}
	}

	if(mode == wm_register)
	{
		//辞書登録開始
		_regmode = TRUE;
		_regtext.clear();
		_regtextpos = 0;
		_regcomp.clear();
		_regfixed = TRUE;

		_BackUpStatus();
		_ClearStatus();
	}

	return TRUE;
}

BOOL CCandidateWindow::_InitClass()
{
	WNDCLASSEXW wcex;

	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = CCandidateWindow::_WindowPreProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = g_hInst;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = CandidateWindowClass;
	wcex.hIconSm = nullptr;

	ATOM atom = RegisterClassExW(&wcex);

	return (atom != 0);
}

void CCandidateWindow::_UninitClass()
{
	UnregisterClassW(CandidateWindowClass, g_hInst);
}

LRESULT CALLBACK CCandidateWindow::_WindowPreProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CCandidateWindow *pCandidateWindow = nullptr;

	switch(uMsg)
	{
	case WM_NCCREATE:
		pCandidateWindow = (CCandidateWindow *)((LPCREATESTRUCTW)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pCandidateWindow);
		break;
	default:
		pCandidateWindow = (CCandidateWindow *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		break;
	}

	if(pCandidateWindow != nullptr)
	{
		return pCandidateWindow->_WindowProc(hWnd, uMsg, wParam, lParam);
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CCandidateWindow::_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT:
		_WindowProcPaint(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DPICHANGED:
		_WindowProcDpiChanged(hWnd, uMsg, wParam, lParam);
		break;
	case WM_ERASEBKGND:
		break;
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	default:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void CCandidateWindow::_Destroy()
{
	if(_hwnd != nullptr)
	{
		DestroyWindow(_hwnd);
		_hwnd = nullptr;
	}

	if(_pInputModeWindow != nullptr)
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

void CCandidateWindow::_Move(LPCRECT lpr, TfEditCookie ec, ITfContext *pContext)
{
	if(_hwnd != nullptr && lpr != nullptr)
	{
		_rect = *lpr;

		//ignore abnormal position (from CUAS ?)
		if((_rect.top == _rect.bottom) && ((_rect.right - _rect.left) == 1))
		{
			return;
		}

		if(ec != TF_INVALID_EDIT_COOKIE && pContext != nullptr)
		{
			_vertical = _pTextService->_GetVertical(ec, pContext);
		}

		if(_vertical)
		{
			LONG w = _rect.right - _rect.left;
			_rect.right += w;
			_rect.left += w;
			_rect.bottom = _rect.top;
		}

		_CalcWindowRect();

		if(_pCandidateWindow != nullptr)
		{
#ifdef _DEBUG
			RECT rc;
			GetClientRect(_hwnd, &rc);
			rc.left = _rect.left;
			rc.top += _rect.bottom;
			rc.right = _rect.right;
			rc.bottom += _rect.bottom + NEXT_MARGIN_Y;
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

	if((_mode == wm_candidate) || (_mode == wm_complement))
	{
		_InitList();
	}

	_Update();

	if((_hwnd == nullptr) && (_depth == 0))
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

	if(_hwnd == nullptr)
	{
		_bShow = FALSE;
	}
	else
	{
		_bShow = bShow;
	}

	if(_bShow)
	{
		if(_hwnd != nullptr)
		{
			SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);

			if(_mode == wm_register)
			{
				if(_pInputModeWindow != nullptr)
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
	if((_hwnd == nullptr) && (_depth == 0))
	{
		ITfUIElementMgr *pUIElementMgr;
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_PPV_ARGS(&pUIElementMgr)) == S_OK)
		{
			pUIElementMgr->EndUIElement(_dwUIElementId);
			SafeRelease(&pUIElementMgr);
		}
	}

	if(_hwnd != nullptr)
	{
		SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);

		if(_pInputModeWindow != nullptr)
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
	if(_hwnd != nullptr)
	{
		InvalidateRect(_hwnd, nullptr, FALSE);
		UpdateWindow(_hwnd);

		if(_pInputModeWindow != nullptr)
		{
			_pInputModeWindow->_Redraw();
		}
	}
}

void CCandidateWindow::_SetText(const std::wstring &text, BOOL fixed, int mode)
{
	//CTextService -> CCandidateList -> CCandidateWindow で入力文字列をもらう

	if(_pCandidateWindow != nullptr && !_preEnd)
	{
		_pCandidateWindow->_SetText(text, fixed, mode);
		return;
	}

	if((mode == wm_candidate) || (mode == wm_register) || (mode == wm_delete))
	{
		_CreateNext(mode);
	}

	if((mode == wm_candidate) || (mode == wm_register) || (mode == wm_none))
	{
		_regfixed = fixed;

		if(fixed)
		{
			_regcomp.clear();
			_regtext.insert(_regtextpos, text);
			_regtextpos += text.size();
		}
		else
		{
			_regcomp = text;
			if(_regcomp.empty())
			{
				_regfixed = TRUE;
			}
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
	if(_hwnd != nullptr)
	{
		SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}
	if(_pInputModeWindow != nullptr)
	{
		_pInputModeWindow->_Show(TRUE);
	}
#endif

	if(_pCandidateWindow != nullptr)
	{
		_pCandidateWindow->_Destroy();
	}
	SafeRelease(&_pCandidateWindow);

	if(_hwnd == nullptr)
	{
		_dwFlags = TF_CLUIE_DOCUMENTMGR | TF_CLUIE_COUNT | TF_CLUIE_SELECTION |
			TF_CLUIE_STRING | TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;
		_Update();
		_UpdateUIElement();
	}
}

void CCandidateWindow::_UpdateComp()
{
	_mode = wm_complement;
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

	if(_mode == wm_candidate)
	{
		_uPageCandNum = MAX_SELKEY;
	}
	else
	{
		_uPageCandNum = _pTextService->cx_compmultinum;
		if(_uPageCandNum > MAX_SELKEY_C || _uPageCandNum < 1)
		{
			_uPageCandNum = MAX_SELKEY;
		}
	}

	if(_mode == wm_candidate)
	{
		_uShowedCount = _pTextService->cx_untilcandlist - 1;
	}
	else
	{
		_uShowedCount = 0;
	}
	_uCount = (UINT)candidates.size() - _uShowedCount;

	_CandStr.clear();
	for(i = 0; i < _uCount; i++)
	{
		if(_mode == wm_candidate)
		{
			_CandStr.push_back(_pTextService->selkey[(i % _uPageCandNum)][0]);
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
			if(_mode == wm_candidate)
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

	_uPageCnt = ((_uCount - (_uCount % _uPageCandNum)) / _uPageCandNum) + ((_uCount % _uPageCandNum) == 0 ? 0 : 1);

	_PageIndex.clear();
	_CandCount.clear();
	for(i = 0; i < _uPageCnt; i++)
	{
		_PageIndex.push_back(i * _uPageCandNum);
		_CandCount.push_back((i < (_uPageCnt - 1)) ? _uPageCandNum :
			(((_uCount % _uPageCandNum) == 0) ? _uPageCandNum : (_uCount % _uPageCandNum)));
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
		if(_pCandidateList != nullptr)
		{
			if(_hwnd == nullptr)
			{
				_ulsingle = TRUE;
			}

			//辞書登録開始
			_regmode = TRUE;
			_regtext.clear();
			_regtextpos = 0;
			_regcomp.clear();
			_regfixed = TRUE;

			_BackUpStatus();
			_ClearStatus();

			if(_pInputModeWindow != nullptr)
			{
				_pInputModeWindow->_Show(TRUE);
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
		if(_pCandidateList != nullptr)
		{
			if(!_regmode)
			{
				if(_pTextService->cx_untilcandlist == 1)
				{
					if(_pCandidateWindowParent == nullptr)
					{
						_InvokeSfHandler(SKK_CANCEL);
					}
					else
					{
						if(_mode == wm_register)
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
					if(_pCandidateWindowParent == nullptr)
					{
						_pTextService->candidx = _pTextService->cx_untilcandlist - 1;
						_InvokeSfHandler(SKK_PREV_CAND);
					}
					else
					{
						if(_mode == wm_register)
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
		if((_pTextService->cx_stacompmulti && !_pTextService->cx_dyncompmulti) ||
			//closed at _DynamicComp
			_pTextService->searchkey.empty())
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

void CCandidateWindow::_Update()
{
	if(_regmode)
	{
		disptext = _MakeRegWordString();
	}
	else if(_mode == wm_delete)
	{
		disptext = _MakeDelWordString();
	}

	if(_ulsingle)
	{
		_dwFlags = TF_CLUIE_COUNT | TF_CLUIE_SELECTION | TF_CLUIE_STRING |
			TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;
		_UpdateUIElement();
	}
	else
	{
		if(_hwnd != nullptr)
		{
			_CalcWindowRect();

			_Redraw();
		}
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
	if(_pCandidateWindowParent != nullptr && !_preEnd)
	{
		_pCandidateWindowParent->_PreEnd();
	}
}

void CCandidateWindow::_EndReq()
{
	if(_pCandidateWindowParent != nullptr && !_preEnd)
	{
		_pCandidateWindowParent->_End();
	}
}

void CCandidateWindow::_CreateNext(int mode)
{
	try
	{
		_pCandidateWindow = new CCandidateWindow(_pTextService, _pCandidateList);
		_pCandidateWindow->_Create(_hwndParent, this, _dwUIElementId, _depth + 1, mode);

#ifdef _DEBUG
		RECT rc;
		GetClientRect(_hwnd, &rc);
		rc.left = _rect.left;
		rc.top += _rect.bottom;
		rc.right = _rect.right;
		rc.bottom += _rect.bottom + NEXT_MARGIN_Y;
		_pCandidateWindow->_Move(&rc);
#else
		_pCandidateWindow->_Move(&_rect);
#endif
		_pCandidateWindow->_BeginUIElement();

#ifndef _DEBUG
		if(_hwnd != nullptr)
		{
			SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);
		}

		if(_pInputModeWindow != nullptr)
		{
			_pInputModeWindow->_Show(FALSE);
		}
#endif
	}
	catch(...)
	{
	}
}
