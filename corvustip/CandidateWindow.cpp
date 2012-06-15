
#include "corvustip.h"
#include "TextService.h"
#include "CandidateWindow.h"
#include "CandidateList.h"

static WNDPROC ToolTipWndProcDef = NULL;
static HWND _hwndParent = NULL;

static const WCHAR *markZWSP = L"\u200B";	//U+200B ZERO WIDTH SPACE
static const WCHAR *markAnnotation = L";";

static const WCHAR *markNo = L":";
static const WCHAR *markNoTT = L" ";
static const WCHAR *markCandEnd = L"　";
static const WCHAR *markCursor = L"|";
static const WCHAR *markRegKeyEnd = L"：";
static const WCHAR *markLinkS = L"<a>";
static const WCHAR *markLinkE = L"</a>";

CCandidateWindow::CCandidateWindow(CTextService *pTextService)
{
	_hwnd = NULL;

	_pTextService = pTextService;
	_pTextService->AddRef();

	_pCandidateList = _pTextService->_GetCandidateList();
	_pCandidateList->AddRef();

	_bShow = FALSE;
	_dwFlags = 0;
	_uShowedCount = 0;
	_uCount = 0;
	_uIndex = 0;
	_uPageCnt = 0;
	_CandCount.clear();
	_PageInex.clear();
	_CandStr.clear();

	hFont = NULL;

	_reg = FALSE;

	regword = FALSE;
	regwordfixed = FALSE;
	regwordshowcandlist = FALSE;

	regwordstr.clear();
	regwordstrpos = 0;
	comptext.clear();

	_ClearStatusReg();

	_cRef = 1;
}

CCandidateWindow::~CCandidateWindow()
{
	_pCandidateList->Release();
	_pTextService->Release();
}

STDAPI CCandidateWindow::QueryInterface(REFIID riid, void **ppvObj)
{
	if(ppvObj == NULL)
	{
		return E_INVALIDARG;
	}

	*ppvObj = NULL;

	if(IsEqualIID(riid, IID_IUnknown) ||
	        IsEqualIID(riid, IID_ITfUIElement) ||
	        IsEqualIID(riid, IID_ITfCandidateListUIElement) ||
	        IsEqualIID(riid, IID_ITfCandidateListUIElementBehavior))
	{
		*ppvObj = (ITfCandidateListUIElementBehavior *)this;
	}

	if(*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) CCandidateWindow::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) CCandidateWindow::Release()
{
	if(--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CCandidateWindow::GetDescription(BSTR *bstr)
{
	BSTR bstrDesc;

	if(bstr == NULL)
	{
		return E_INVALIDARG;
	}

	*bstr = NULL;

	bstrDesc = SysAllocString(TextServiceDesc);

	if(bstrDesc == NULL)
	{
		return E_OUTOFMEMORY;
	}

	*bstr = bstrDesc;

	return S_OK;
}

STDAPI CCandidateWindow::GetGUID(GUID *pguid)
{
	if(pguid == NULL)
	{
		return E_INVALIDARG;
	}

	*pguid = c_guidCandidateListUIElement;

	return S_OK;
}

STDAPI CCandidateWindow::Show(BOOL bShow)
{
	if(!_bShow)
	{
		return E_UNEXPECTED;
	}

	if(bShow)
	{
		if(_hwnd != NULL)
		{
			SendMessage(_hwnd, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
		}
	}
	else
	{
		if(_hwnd != NULL)
		{
			SendMessage(_hwnd, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
		}
		_UpdateUIElement();
	}
	return S_OK;
}

STDAPI CCandidateWindow::IsShown(BOOL *pbShow)
{
	if(pbShow == NULL)
	{
		return E_INVALIDARG;
	}

	*pbShow = IsWindowVisible(_hwnd);

	return S_OK;
}

STDAPI CCandidateWindow::GetUpdatedFlags(DWORD *pdwFlags)
{
	if(pdwFlags == NULL)
	{
		return E_INVALIDARG;
	}

	*pdwFlags = _dwFlags;

	return S_OK;
}

STDAPI CCandidateWindow::GetDocumentMgr(ITfDocumentMgr **ppdim)
{
	if(ppdim == NULL)
	{
		return E_INVALIDARG;
	}

	*ppdim = NULL;

	return S_OK;
}

STDAPI CCandidateWindow::GetCount(UINT *puCount)
{
	if(puCount == NULL)
	{
		return E_INVALIDARG;
	}

	*puCount = _uCount;

	return S_OK;
}

STDAPI CCandidateWindow::GetSelection(UINT *puIndex)
{
	if(puIndex == NULL)
	{
		return E_INVALIDARG;
	}

	*puIndex = _uIndex;

	return S_OK;
}

STDAPI CCandidateWindow::GetString(UINT uIndex, BSTR *pstr)
{
	if(pstr == NULL)
	{
		return E_INVALIDARG;
	}

	if(uIndex < _CandStr.size())
	{
		*pstr = SysAllocString(_CandStr[uIndex].c_str());
	}
	else
	{
		*pstr = SysAllocString(L"");
	}

	return S_OK;
}

STDAPI CCandidateWindow::GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt)
{
	UINT i;
	HRESULT hr = S_OK;

	if(puPageCnt == NULL)
	{
		return E_INVALIDARG;
	}

	if(uSize >= _uPageCnt)
	{
		uSize = _uPageCnt;
	}
	else
	{
		hr = S_FALSE;
	}

	if(pIndex != NULL)
	{
		for(i=0; i<uSize; i++)
		{
			*pIndex = _PageInex[i];
			pIndex++;
		}
	}

	*puPageCnt = _uPageCnt;
	return hr;
}

STDAPI CCandidateWindow::SetPageIndex(UINT *pIndex, UINT uPageCnt)
{
	UINT uCandCnt, i, j, k;

	if(pIndex == NULL)
	{
		return E_INVALIDARG;
	}

	for(j=0; j<uPageCnt-1; j++)
	{
		uCandCnt = pIndex[j + 1] - pIndex[j];
		if(uCandCnt > MAX_SELKEY_C)
		{
			return E_INVALIDARG;
		}
	}

	_PageInex.clear();
	_CandCount.clear();
	_CandStr.clear();
	j = 0;
	k = 0;
	for(j=0; j<uPageCnt; j++)
	{
		if(j < (uPageCnt - 1))
		{
			uCandCnt = pIndex[j + 1] - pIndex[j];
		}
		else
		{
			uCandCnt = _uCount - k;
		}

		pIndex[j] = k;
		_PageInex.push_back(k);
		_CandCount.push_back(uCandCnt);

		for(i=0; i<uCandCnt; i++)
		{
			if(k == _uCount)
			{
				break;
			}

			_CandStr.push_back(_pTextService->selkey[(i % MAX_SELKEY_C)][0]);
			_CandStr[k].append(markNo + _pTextService->candidates[ _uShowedCount + k ].first.first);
			
			if(_pTextService->annotation &&
				!_pTextService->candidates[ _uShowedCount + k ].first.second.empty())
			{
				_CandStr[k].append(markAnnotation +
					_pTextService->candidates[ _uShowedCount + k ].first.second);
			}

			++k;
		}
	}

	_uPageCnt = uPageCnt;
	return S_OK;
}

STDAPI CCandidateWindow::GetCurrentPage(UINT *puPage)
{
	UINT i;

	if(puPage == NULL)
	{
		return E_INVALIDARG;
	}

	*puPage = 0;

	if(_uPageCnt == 0)
	{
		return E_UNEXPECTED;
	}

	if(_uPageCnt == 1)
	{
		*puPage = 0;
		return S_OK;
	}

	for(i=1; i<_uPageCnt; i++)
	{
		if(_PageInex[i] > _uIndex)
		{
			break;
		}
	}

	*puPage = i - 1;
	return S_OK;
}

STDAPI CCandidateWindow::SetSelection(UINT nIndex)
{
	UINT uOldPage, uNewPage;

	if(nIndex >= _uCount)
	{
		return E_INVALIDARG;
	}

	GetCurrentPage(&uOldPage);
	_uIndex = nIndex;
	GetCurrentPage(&uNewPage);

	_dwFlags = TF_CLUIE_SELECTION;
	if(uNewPage != uOldPage)
	{
		_dwFlags |= TF_CLUIE_CURRENTPAGE;
	}

	_UpdateUIElement();
	return S_OK;
}

STDAPI CCandidateWindow::Finalize()
{
	if(_pCandidateList)
	{
		_pCandidateList->_EndCandidateList();
	}
	return S_OK;
}

STDAPI CCandidateWindow::Abort()
{
	if(_pCandidateList)
	{
		_pCandidateList->_EndCandidateList();
	}
	return S_OK;
}

LRESULT CALLBACK _WindowProcTT(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CallWindowProc(ToolTipWndProcDef, hwnd, uMsg, wParam, lParam);
	return 0;
}

BOOL CCandidateWindow::_Create(HWND hwndParent, BOOL reg)
{
	_hwndParent = hwndParent;
	
	_hwnd = CreateWindowExW(WS_EX_TOPMOST,
	                       TOOLTIPS_CLASSW, NULL,
	                       WS_POPUP | TTS_NOPREFIX | TTS_NOANIMATE | TTS_NOFADE,
	                       CW_USEDEFAULT, CW_USEDEFAULT,
	                       CW_USEDEFAULT, CW_USEDEFAULT,
	                       _hwndParent, NULL, g_hInst, this);
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

	if(SendMessage(_hwnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFOW) &ti) == FALSE)
	{
		return FALSE;
	}

	if(!_pTextService->visualstyle)
	{
		SetWindowTheme(_hwnd, L" ", L" ");
	}

	//set font
	HDC hdcTT = GetDC(_hwnd);
	hFont = CreateFontW(-MulDiv(_pTextService->fontpoint, GetDeviceCaps(hdcTT, LOGPIXELSY), 72), 0, 0, 0,
		_pTextService->fontweight, _pTextService->fontitalic, FALSE, FALSE, SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH,
		_pTextService->fontname);
	SendMessage(_hwnd, WM_SETFONT, (WPARAM)hFont, 0);
	ReleaseDC(_hwnd, hdcTT);
	//DeleteObject(hFont);	// -> _End()

	//set color
	SendMessage(_hwnd, TTM_SETTIPTEXTCOLOR, (WPARAM)RGB(0,0,0), 0);
	SendMessage(_hwnd, TTM_SETTIPBKCOLOR, (WPARAM)RGB(255,255,255), 0);

	//set max width
	SendMessage(_hwnd, TTM_SETMAXTIPWIDTH, 0, _pTextService->maxwidth);

	//set initial duration
	SendMessage(_hwnd, TTM_SETDELAYTIME, TTDT_INITIAL, 0);

	//set mergin
	#define MERGIN 2
	RECT rect = {MERGIN,MERGIN,MERGIN,MERGIN};
	SendMessage(_hwnd, TTM_SETMARGIN, 0, (LPARAM)&rect);

	//set window procedure
	ToolTipWndProcDef = (WNDPROC)GetWindowLongPtr(_hwnd, GWLP_WNDPROC);
	if(ToolTipWndProcDef != 0)
	{
		SetWindowLongPtr(_hwnd, GWLP_WNDPROC, (LONG_PTR)_WindowProcTT);
		SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	}

	_reg = reg;
	if(reg)
	{
		//辞書登録開始
		regword = TRUE;
		regwordstr.clear();
		regwordstrpos = 0;
		comptext.clear();
		regwordfixed = TRUE;
		regwordshowcandlist = FALSE;

		_BackUpStatus();
		_ClearStatus();
	}	
		
	return TRUE;
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
		SendMessage(_hwnd, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(x, y));
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
	else
	{
		_UpdateTT();
	}

	if(!_bShow)
	{
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
		{
			pUIElementMgr->BeginUIElement(this, &bShow, &_dwUIElementId);
			if(!bShow)
			{
				pUIElementMgr->UpdateUIElement(_dwUIElementId);
			}
			pUIElementMgr->Release();

			_bShow = TRUE;
		}
	}

	OSVERSIONINFO ovi;
	ZeroMemory(&ovi, sizeof(ovi));
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);
	if(ovi.dwMajorVersion < 6)
	{
		_bShow = TRUE;
	}

	if(bShow)
	{
		if(_hwnd != NULL)
		{
			SendMessage(_hwnd, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
		}
	}
}

void CCandidateWindow::_EndUIElement()
{
	if(_bShow)
	{
		ITfUIElementMgr *pUIElementMgr;
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
		{
			pUIElementMgr->EndUIElement(_dwUIElementId);
			pUIElementMgr->Release();
		}
	}

	if(_hwnd != NULL)
	{
		SendMessage(_hwnd, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
	}

	if(hFont != NULL)
	{
		DeleteObject(hFont);
	}

	_bShow = FALSE;
}

HRESULT CCandidateWindow::_OnKeyDown(UINT uVKey)
{
	UINT i, page, index;

	WCHAR ch = _pTextService->_GetCh((WPARAM)uVKey);
	BYTE sf = _pTextService->_GetSf((WPARAM)uVKey, ch);

	//辞書登録モードかつ候補一覧が表示されていない
	if(regword && !regwordshowcandlist)
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
				_pCandidateList->_InvokeSfHandler(SKK_CANCEL);
				_pCandidateList->_EndCandidateList();
			}
			else
			{
				_pTextService->_HandleKey(0, NULL, 0, SKK_CANCEL);
				_UpdateTT();
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
					index = (UINT)(_pTextService->untilcandlist - 1) + _PageInex[page] + i;
					if(index < _pTextService->candidates.size())
					{
						if(!regword)
						{
							_pTextService->candidx = index;
							_pCandidateList->_InvokeSfHandler(SKK_ENTER);
							_pCandidateList->_EndCandidateList();
						}
						else
						{
							_pTextService->candidx = index;
							_pTextService->_HandleKey(0, NULL, 0, SKK_ENTER);
							_UpdateTT();
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

HRESULT CCandidateWindow::_OnKeyUp(UINT uVKey)
{
	return S_OK;
}

void CCandidateWindow::_SetTextRegword(const std::wstring &text, BOOL fixed, BOOL showcandlist)
{
	//CTextService -> CCandidateList -> CCandidateWindow で入力文字列をもらう

	regwordfixed = fixed;

	if(showcandlist && !regwordshowcandlist)
	{
		//辞書登録モードで候補一覧を表示開始
		_InitList();
		_UpdateTT();
	}

	regwordshowcandlist = showcandlist;

	if(fixed)
	{
		comptext.clear();
		regwordstr.insert(regwordstrpos, text);
		regwordstrpos += text.size();
	}
	else
	{
		comptext = text;
		if(comptext.empty())
		{
			regwordfixed = TRUE;
		}
	}
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

void CCandidateWindow::_InitList()
{
	UINT i;

	_uShowedCount = (UINT)_pTextService->untilcandlist - 1;
	_uCount = (UINT)_pTextService->candidates.size() - _uShowedCount;

	_CandStr.clear();
	for(i=0; i<_uCount; i++)
	{
		_CandStr.push_back(_pTextService->selkey[(i % MAX_SELKEY)][0]);
		_CandStr[i].append(markNo + _pTextService->candidates[ _uShowedCount + i ].first.first);

		if(_pTextService->annotation &&
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

	_UpdateTT();
}

void CCandidateWindow::_UpdateUIElement()
{
	if(_bShow && !IsWindowVisible(_hwnd))
	{
		ITfUIElementMgr *pUIElementMgr;
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
		{
			pUIElementMgr->UpdateUIElement(_dwUIElementId);
			pUIElementMgr->Release();
		}
	}

	_UpdateTT();
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
			//候補一覧が独自描画のとき辞書登録せずに▽モードにする
			if(!IsWindowVisible(_hwnd))
			{
				_pCandidateList->_InvokeSfHandler(SKK_CANCEL);
				_pCandidateList->_EndCandidateList();
			}
			else
			{
				if(!regword)
				{
					//辞書登録開始
					regword = TRUE;
					regwordstr.clear();
					regwordstrpos = 0;
					comptext.clear();
					regwordfixed = TRUE;
					regwordshowcandlist = FALSE;

					_BackUpStatus();
					_ClearStatus();
				}
				else
				{
					//辞書登録モードの▽モードに戻る
					_pTextService->_HandleKey(0, NULL, 0, SKK_CANCEL);
				}
				_UpdateTT();
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
				if(_pTextService->untilcandlist == 1)
				{
					_pCandidateList->_InvokeSfHandler(SKK_CANCEL);
					_pCandidateList->_EndCandidateList();
				}
				else
				{
					_pTextService->candidx = _pTextService->untilcandlist - 1;
					_pCandidateList->_InvokeSfHandler(SKK_PREV_CAND);
					_pCandidateList->_EndCandidateList();
				}
			}
			else
			{
				if(_pTextService->untilcandlist == 1)
				{
					_pTextService->_HandleKey(0, NULL, 0, SKK_CANCEL);
				}
				else
				{
					_pTextService->candidx = _pTextService->untilcandlist - 1;
					_pTextService->_HandleKey(0, NULL, 0, SKK_PREV_CAND);
				}
				_UpdateTT();
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

	_UpdateUIElement();
}

void CCandidateWindow::_OnKeyDownRegword(UINT uVKey, BYTE sf)
{
	HANDLE hCB;
	PWCHAR pwCB;
	std::wstring s;
	std::wstring regwordstrconv;
	std::wstring regwordstrcandidate;
	std::wstring regwordstrannotation;
	std::wsmatch result;

	//確定していないとき
	if(!regwordfixed)
	{
		_pTextService->showcandlist = FALSE;	//候補一覧表示をループさせる
		_pTextService->_HandleKey(0, NULL, (WPARAM)uVKey, SKK_NULL);
		_UpdateTT();
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

		regwordshowcandlist = FALSE;
		regwordfixed = FALSE;
		regword = FALSE;

		if(regwordstr.empty())	//空のときはキャンセル扱い
		{
			if(!_reg)
			{
				_InitList();
				_uIndex = _PageInex[_PageInex.size() - 1];
				_UpdateTT();
			}
			else
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
		}
		else
		{
			//候補と注釈を、行頭以外の最後のセミコロンで分割
			if(std::regex_search(regwordstr, result, std::wregex(L".+;")))
			{
				regwordstrcandidate = result.str().substr(0, result.str().size() - 1);
				regwordstrannotation = result.suffix();
			}
			else
			{
				regwordstrcandidate = regwordstr;
				regwordstrannotation.clear();
			}

			//数値変換タイプ0～3の候補は#を数値にした見出し語が表示用 それ以外は見出し語そのまま
			if(std::regex_match(regwordstrcandidate, std::wregex(L".*#[0-3].*")))
			{
				_pTextService->_ConvNum(regwordstrconv, _pTextService->searchkeyorg, regwordstrcandidate);
			}
			else
			{
				regwordstrconv = regwordstrcandidate;
				_pTextService->searchkey = _pTextService->searchkeyorg;
			}

			_pTextService->candidates.push_back(CANDIDATE(
				CANDIDATEBASE(regwordstrconv, regwordstrannotation),
				(CANDIDATEBASE(regwordstrcandidate, regwordstrannotation))));
			_pTextService->candidx = _pTextService->candidates.size() - 1;

			regwordstr.clear();
			regwordstrpos = 0;

			_pCandidateList->_InvokeSfHandler(SKK_ENTER);
			_pCandidateList->_EndCandidateList();
		}
		break;

	case SKK_CANCEL:
		_RestoreStatusReg();
		_ClearStatusReg();

		regwordshowcandlist = FALSE;
		regwordfixed = FALSE;
		regword = FALSE;

		regwordstr.clear();
		regwordstrpos = 0;
		
		if(!_reg)
		{
			_InitList();
			_uIndex = _PageInex[_PageInex.size() - 1];
			_UpdateTT();
		}
		else
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
		break;

	//結合文字は考慮しない

	case SKK_BACK:
		if(comptext.empty() && regwordstrpos > 0 && regwordstr.size() > 0)
		{
			if(regwordstr.size() >= 2 && regwordstrpos >= 2 &&
				_pTextService->_IsSurrogatePair(regwordstr[regwordstrpos - 2], regwordstr[regwordstrpos - 1]))
			{
				regwordstrpos -= 2;
				regwordstr.erase(regwordstr.begin() + regwordstrpos);
				regwordstr.erase(regwordstr.begin() + regwordstrpos);
			}
			else
			{
				--regwordstrpos;
				regwordstr.erase(regwordstr.begin() + regwordstrpos);
			}
			_UpdateTT();
		}
		break;

	case SKK_DELETE:
		if(comptext.empty() && regwordstrpos < regwordstr.size())
		{
			if(regwordstr.size() >= regwordstrpos + 2 &&
				_pTextService->_IsSurrogatePair(regwordstr[regwordstrpos + 0], regwordstr[regwordstrpos + 1]))
			{
				regwordstr.erase(regwordstr.begin() + regwordstrpos);
				regwordstr.erase(regwordstr.begin() + regwordstrpos);
			}
			else
			{
				regwordstr.erase(regwordstr.begin() + regwordstrpos);
			}
			_UpdateTT();
		}
		break;

	case SKK_LEFT:
		if(comptext.empty() && regwordstrpos > 0 && regwordstr.size() > 0)
		{
			if(regwordstr.size() >= 2 && regwordstrpos >= 2 &&
				_pTextService->_IsSurrogatePair(regwordstr[regwordstrpos - 2], regwordstr[regwordstrpos - 1]))
			{
				regwordstrpos -= 2;
			}
			else
			{
				--regwordstrpos;
			}
			_UpdateTT();
		}
		break;

	case SKK_UP:
		if(comptext.empty())
		{
			regwordstrpos = 0;
			_UpdateTT();
		}
		break;

	case SKK_RIGHT:
		if(comptext.empty() && regwordstrpos < regwordstr.size())
		{
			if(regwordstr.size() >= regwordstrpos + 2 &&
				_pTextService->_IsSurrogatePair(regwordstr[regwordstrpos + 0], regwordstr[regwordstrpos + 1]))
			{
				regwordstrpos += 2;
			}
			else
			{
				++regwordstrpos;
			}
			_UpdateTT();
		}
		break;

	case SKK_DOWN:
		if(comptext.empty())
		{
			regwordstrpos = regwordstr.size();
			_UpdateTT();
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
						regwordstr.insert(regwordstrpos, s);
						regwordstrpos += s.size();
						_UpdateTT();
						GlobalUnlock(hCB);
					}
				}
				CloseClipboard();
			}
		}
		break;

	default:
		_pTextService->_HandleKey(0, NULL, (WPARAM)uVKey, SKK_NULL);
		_UpdateTT();
		break;
	}
}

std::wstring CCandidateWindow::_EscapeTags(const std::wstring &text)
{
	return markZWSP + std::regex_replace(text, std::wregex(L"<"), std::wstring(L"<") + markZWSP) + markZWSP;
}

void CCandidateWindow::_UpdateTT()
{
	WCHAR selkey[2];
	WCHAR strPage[32];
	UINT i, page, count;

	if(regword && !regwordshowcandlist)
	{
		strTT.clear();
		strTT.append(markLinkS + _EscapeTags(searchkey_bak) + markLinkE);
		strTT.append(markRegKeyEnd + _EscapeTags(regwordstr.substr(0, regwordstrpos)));
		strTT.append(markLinkS + _EscapeTags(comptext) + markLinkE);
		strTT.append(markCursor + _EscapeTags(regwordstr.substr(regwordstrpos)));
	}
	else
	{
		GetCurrentPage(&page);
		count = 0;
		for(i=0; i<page; i++)
		{
			count += _CandCount[i];
		}

		strTT.clear();
		selkey[1] = L'\0';
		for(i=0; i<_CandCount[page]; i++)
		{
			strTT.append(markLinkS + _EscapeTags(_pTextService->selkey[(i % MAX_SELKEY_C)][0]) + markLinkE);

			strTT.append(markNoTT +
				_EscapeTags(_pTextService->candidates[ count + _uShowedCount + i ].first.first));

			if(_pTextService->annotation &&
				!_pTextService->candidates[ count + _uShowedCount + i ].first.second.empty())
			{
				strTT.append(markAnnotation +
					_EscapeTags(_pTextService->candidates[ count + _uShowedCount + i ].first.second));
			}

			strTT.append(markCandEnd);
		}

		_snwprintf_s(strPage, _TRUNCATE, L"(%u/%u)", page + 1, _uPageCnt);
		strTT.append(strPage);
	}

	ti.lpszText = (LPWSTR)strTT.c_str();
	SendMessage(_hwnd, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
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
