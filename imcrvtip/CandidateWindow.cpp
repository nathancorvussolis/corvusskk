
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "CandidateWindow.h"
#include "InputModeWindow.h"

#define MERGIN_X 2
#define MERGIN_Y 4

#define SWAPRGB(rgb) (((rgb & 0x0000FF) << 16) | (rgb & 0x00FF00) | ((rgb >> 16) & 0x0000FF))

static LPCWSTR markNo = L":";
static LPCWSTR markAnnotation = L";";
static LPCWSTR markCandEnd = L"\u3000";
static LPCWSTR markCursor = L"|";
static LPCWSTR markReg = L"登録";
static LPCWSTR markRegL = L"[";
static LPCWSTR markRegR = L"]";
static LPCWSTR markRegKeyEnd = L"：";
static LPCWSTR markSP = L"\x20";
static LPCWSTR markNBSP = L"\u00A0";

BOOL CCandidateWindow::_Create(HWND hwndParent, CCandidateWindow *pCandidateWindowParent, DWORD dwUIElementId, UINT depth, BOOL reg)
{
	HDC hdc;
	LOGFONTW logfont;
	UINT i;
	HRESULT hr;
	IDWriteGdiInterop* pDWGI = NULL;
	IDWriteFont* pDWFont = NULL;

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
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
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

		hdc = GetDC(_hwnd);

		logfont.lfHeight = -MulDiv(_pTextService->cx_fontpoint, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		logfont.lfWidth = 0;
		logfont.lfEscapement = 0;
		logfont.lfOrientation = 0;
		logfont.lfWeight = _pTextService->cx_fontweight;
		logfont.lfItalic = _pTextService->cx_fontitalic;
		logfont.lfUnderline = FALSE;
		logfont.lfStrikeOut = FALSE;
		logfont.lfCharSet = SHIFTJIS_CHARSET;
		logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		logfont.lfQuality = PROOF_QUALITY;
		logfont.lfPitchAndFamily = DEFAULT_PITCH;
		wcscpy_s(logfont.lfFaceName, _pTextService->cx_fontname);
		hFont = CreateFontIndirectW(&logfont);

		if(_pTextService->cx_drawapi)
		{
			_drawtext_option = (IsVersion63AndOver() && _pTextService->cx_colorfont) ?
				D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT :
				D2D1_DRAW_TEXT_OPTIONS_NONE;

			hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_pD2DFactory);

			if(hr == S_OK)
			{
				hr = _pD2DFactory->CreateDCRenderTarget(&c_d2dprops, &_pD2DDCRT);
			}

			if(hr == S_OK)
			{
				for(i = 0; i < 8; i++)
				{
					hr = _pD2DDCRT->CreateSolidColorBrush(D2D1::ColorF(SWAPRGB(_pTextService->cx_colors[i])), &_pD2DBrush[i]);
					if(hr != S_OK)
					{
						break;
					}
				}
			}

			if(hr == S_OK)
			{
				hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, IID_PUNK_ARGS(&_pDWFactory));
			}

			if(hr == S_OK)
			{
				hr = _pDWFactory->GetGdiInterop(&pDWGI);
			}

			if(hr == S_OK)
			{
				hr = pDWGI->CreateFontFromLOGFONT(&logfont, &pDWFont);
				if(hr != S_OK)
				{
					pDWGI->Release();
				}
			}

			if(hr == S_OK)
			{
				hr = _pDWFactory->CreateTextFormat(_pTextService->cx_fontname, NULL,
					pDWFont->GetWeight(), pDWFont->GetStyle(), pDWFont->GetStretch(),
					(FLOAT)-logfont.lfHeight, L"ja-jp", &_pDWTF);
				pDWFont->Release();
				pDWGI->Release();
			}

			if(hr == S_OK)
			{
				hr = _pDWTF->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
			}

			if(hr != S_OK)
			{
				for(i = 0; i < 8; i++)
				{
					if(_pD2DBrush[i] != NULL)
					{
						_pD2DBrush[i]->Release();
						_pD2DBrush[i] = NULL;
					}
				}
				if(_pD2DDCRT != NULL)
				{
					_pD2DDCRT->Release();
					_pD2DDCRT = NULL;
				}
				if(_pD2DFactory != NULL)
				{
					_pD2DFactory->Release();
					_pD2DFactory = NULL;
				}
				if(_pDWTF != NULL)
				{
					_pDWTF->Release();
					_pDWTF = NULL;
				}
				if(_pDWFactory != NULL)
				{
					_pDWFactory->Release();
					_pDWFactory = NULL;
				}
			}
		}

		ReleaseDC(_hwnd, hdc);
	}

	if(_hwnd != NULL && _pTextService->cx_showmodeinl &&
		(!_pTextService->cx_showmodeimm || (_pTextService->cx_showmodeimm && _pTextService->_ImmersiveMode)))
	{
		_pInputModeWindow = new CInputModeWindow();
		if(!_pInputModeWindow->_Create(_pTextService, NULL, TRUE, _hwnd))
		{
			_pInputModeWindow->_Destroy();
			delete _pInputModeWindow;
			_pInputModeWindow = NULL;
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

		if(_pInputModeWindow)
		{
			_pInputModeWindow->_Show(TRUE);
		}
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
	PAINTSTRUCT ps;
	HDC hdc;
	HDC hmemdc = NULL;
	HBITMAP hmembmp = NULL;
	HPEN npen;
	HBRUSH nbrush;
	HGDIOBJ bmp = NULL, font, pen, brush;
	RECT r, rc;
	POINT pt;
	int cx, cy, cycle;
	UINT page, count, i;
	std::wstring s;
	WCHAR strPage[32];
	TEXTMETRICW tm;
	HRESULT hr;
	D2D1_RECT_F rd2d;
	IDWriteTextLayout* pdwTL = NULL;
	DWRITE_TEXT_METRICS dwTM;

	switch(uMsg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &r);
		cx = r.right;
		cy = r.bottom;

		if(_pD2DDCRT)
		{
			hmemdc = hdc;
			_pD2DDCRT->BindDC(hdc, &r);
			_pD2DDCRT->BeginDraw();
			_pD2DDCRT->SetTransform(D2D1::Matrix3x2F::Identity());
			_pD2DDCRT->Clear(D2D1::ColorF(SWAPRGB(_pTextService->cx_colors[CL_COLOR_BG])));
			rd2d = D2D1::RectF(0.5F, 0.5F, ((FLOAT)cx) - 0.5F, ((FLOAT)cy) - 0.5F);
			_pD2DDCRT->DrawRectangle(rd2d, _pD2DBrush[CL_COLOR_FR]);
		}
		else
		{
			hmemdc = CreateCompatibleDC(hdc);
			hmembmp = CreateCompatibleBitmap(hdc, cx, cy);
			bmp = SelectObject(hmemdc, hmembmp);

			npen = CreatePen(PS_SOLID, 1, _pTextService->cx_colors[CL_COLOR_FR]);
			pen = SelectObject(hmemdc, npen);
			nbrush = CreateSolidBrush(_pTextService->cx_colors[CL_COLOR_BG]);
			brush = SelectObject(hmemdc, nbrush);

			Rectangle(hmemdc, 0, 0, cx, cy);

			SelectObject(hmemdc, pen);
			SelectObject(hmemdc, brush);

			DeleteObject(npen);
			DeleteObject(nbrush);

			SetBkMode(hmemdc, TRANSPARENT);
		}

		font = SelectObject(hmemdc, hFont);
		GetTextMetricsW(hmemdc, &tm);

		if(regwordul || regword)
		{
			r.left += MERGIN_X;
			r.top += MERGIN_Y;
			r.right -= MERGIN_X;
			r.bottom -= MERGIN_Y;

			_PaintRegWord(hmemdc, &r);
		}
		else if(_CandCount.size() != 0)
		{
			pt.x = MERGIN_X;
			pt.y = MERGIN_Y;

			GetCurrentPage(&page);
			count = 0;
			for(i = 0; i < page; i++)
			{
				count += _CandCount[i];
			}

			for(i = 0; i < _CandCount[page]; i++)
			{
				s.clear();
				for(cycle = 0; cycle <= 4; cycle++)
				{
					s += _MakeCandidateString(page, count, i, cycle);
				}

				r.left = 0;
				r.top = 0;
				r.right = 1;
				r.bottom = 1;

				if(_pDWFactory)
				{
					hr = _pDWFactory->CreateTextLayout(s.c_str(), (UINT32)s.size(), _pDWTF, 0.0F, 0.0F, &pdwTL);
					if(hr == S_OK)
					{
						hr = pdwTL->GetMetrics(&dwTM);
						pdwTL->Release();
					}
					if(hr == S_OK)
					{
						r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
						r.bottom = (LONG)ceil(dwTM.height);
					}
				}
				else
				{
					DrawTextW(hmemdc, s.c_str(), -1, &r,
						DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
				}

				if(_pTextService->cx_verticalcand)
				{
					if(i != 0)
					{
						pt.x = MERGIN_X;
						pt.y += tm.tmHeight;
					}
				}
				else
				{
					if(pt.x == MERGIN_X && r.right > cx - MERGIN_X)
					{
						cx = r.right;
					}
					else if(pt.x + r.right > cx - MERGIN_X)
					{
						pt.x = MERGIN_X;
						pt.y += tm.tmHeight;
					}
				}

				rc.left = pt.x;
				rc.top = pt.y;
				rc.right = pt.x + r.right;
				rc.bottom = pt.y + tm.tmHeight;

				_PaintCandidate(hmemdc, &rc, page, count, i);

				pt.x += r.right;
			}

			_snwprintf_s(strPage, _TRUNCATE, L"%s(%u/%u)%s", markNBSP, page + 1, _uPageCnt, markNBSP);

			r.left = 0;
			r.top = 0;
			r.right = 1;
			r.bottom = 1;

			if(_pDWFactory)
			{
				hr = _pDWFactory->CreateTextLayout(strPage, (UINT32)wcslen(strPage), _pDWTF, 0.0F, 0.0F, &pdwTL);
				if(hr == S_OK)
				{
					hr = pdwTL->GetMetrics(&dwTM);
					pdwTL->Release();
				}
				if(hr == S_OK)
				{
					r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
					r.bottom = (LONG)ceil(dwTM.height);
				}
			}
			else
			{
				DrawTextW(hmemdc, strPage, -1, &r,
					DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
			}

			if(_pTextService->cx_verticalcand)
			{
				pt.x = MERGIN_X;
				pt.y += tm.tmHeight;
			}
			else
			{
				if(pt.x == MERGIN_X && r.right > cx - MERGIN_X)
				{
					cx = r.right;
				}
				else if(pt.x + r.right > cx - MERGIN_X)
				{
					pt.x = MERGIN_X;
					pt.y += tm.tmHeight;
				}
			}

			rc.left = pt.x;
			rc.top = pt.y;
			rc.right = pt.x + r.right;
			rc.bottom = pt.y + tm.tmHeight;

			if(_pD2DDCRT && _pDWTF)
			{
				rd2d = D2D1::RectF((FLOAT)rc.left, (FLOAT)rc.top, (FLOAT)rc.right, (FLOAT)rc.bottom);

				_pD2DDCRT->DrawText(strPage, (UINT32)wcslen(strPage), _pDWTF, &rd2d, _pD2DBrush[CL_COLOR_NO], _drawtext_option);
			}
			else
			{
				SetTextColor(hmemdc, _pTextService->cx_colors[CL_COLOR_NO]);

				DrawTextW(hmemdc, strPage, -1, &rc,
					DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
			}
		}

		if(_pD2DDCRT)
		{
			SelectObject(hmemdc, font);

			_pD2DDCRT->EndDraw();
		}
		else
		{
			BitBlt(hdc, 0, 0, cx, cy, hmemdc, 0, 0, SRCCOPY);

			SelectObject(hmemdc, font);
			SelectObject(hmemdc, bmp);

			DeleteObject(hmembmp);
			DeleteObject(hmemdc);
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

std::wstring CCandidateWindow::_MakeRegWordString()
{
	std::wstring s;
	UINT i;

	s.append(markNBSP);
	for(i = 0; i < _depth + 1; i++)
	{
		s.append(markRegL);
	}
	s.append(markReg);
	for(i = 0; i < _depth + 1; i++)
	{
		s.append(markRegR);
	}
	s.append(markNBSP);

	s.append((searchkey_bak.empty() ? searchkeyorg_bak : searchkey_bak) + markRegKeyEnd);

	s.append(regwordtext.substr(0, regwordtextpos));

	s.append(comptext + markCursor);

	s.append(regwordtext.substr(regwordtextpos) + markNBSP);

	return s;
}

void CCandidateWindow::_PaintRegWord(HDC hdc, LPRECT lpr)
{
	std::wstring s;
	D2D1_RECT_F rd2d;

	s = _MakeRegWordString();

	if(_pD2DDCRT && _pDWTF)
	{
		rd2d = D2D1::RectF((FLOAT)lpr->left, (FLOAT)lpr->top, (FLOAT)lpr->right, (FLOAT)lpr->bottom);

		_pD2DDCRT->DrawText(s.c_str(), (UINT32)s.size(), _pDWTF, &rd2d, _pD2DBrush[CL_COLOR_CA], _drawtext_option);
	}
	else
	{
		SetTextColor(hdc, _pTextService->cx_colors[CL_COLOR_CA]);

		DrawTextW(hdc, s.c_str(), -1, lpr,
			DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
	}
}

std::wstring CCandidateWindow::_MakeCandidateString(UINT page, UINT count, UINT idx, int cycle)
{
	std::wstring s;

	switch(cycle)
	{
	case 0:
		s.append(markNBSP);
		s.append(_pTextService->selkey[(idx % MAX_SELKEY_C)][0]);
		break;
	case 1:
		s.append(markNo);
		break;
	case 2:
		s.append(
			std::regex_replace(_pTextService->candidates[ count + _uShowedCount + idx ].first.first,
			std::wregex(markSP), std::wstring(markNBSP)));
		break;
	case 3:
		if(_pTextService->cx_annotation &&
			!_pTextService->candidates[count + _uShowedCount + idx].first.second.empty())
		{
			s.append(markAnnotation);
		}
		break;
	case 4:
		if(_pTextService->cx_annotation &&
			!_pTextService->candidates[count + _uShowedCount + idx].first.second.empty())
		{
			s.append(
				std::regex_replace(_pTextService->candidates[count + _uShowedCount + idx].first.second,
				std::wregex(markSP), std::wstring(markNBSP)));
		}
		s.append(markCandEnd);
		break;
	default:
		break;
	}

	return s;
}

void CCandidateWindow::_PaintCandidate(HDC hdc, LPRECT lpr, UINT page, UINT count, UINT idx)
{
	int cycle;
	std::wstring s;
	RECT r, r_ex;
	HRESULT hr;
	D2D1_RECT_F rd2d;
	IDWriteTextLayout* pdwTL = NULL;
	DWRITE_TEXT_METRICS dwTM;

	r = *lpr;
	r_ex = *lpr;
	r_ex.right = r_ex.left;

	for(cycle = 0; cycle <= 4; cycle++)
	{
		s = _MakeCandidateString(page, count, idx, cycle);

		if(_pD2DDCRT && _pDWTF)
		{
			hr = _pDWFactory->CreateTextLayout(s.c_str(), (UINT32)s.size(), _pDWTF, 0.0F, 0.0F, &pdwTL);
			if(hr == S_OK)
			{
				hr = pdwTL->GetMetrics(&dwTM);
				pdwTL->Release();
			}
			if(hr == S_OK)
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
				r.bottom = (LONG)ceil(dwTM.height);
			}

			r.left = r_ex.right;
			r.top = lpr->top;
			r.right = r_ex.right + r.right;
			r.bottom = lpr->bottom;

			rd2d = D2D1::RectF((FLOAT)r.left, (FLOAT)r.top, (FLOAT)r.right, (FLOAT)r.bottom);

			_pD2DDCRT->DrawText(s.c_str(), (UINT32)s.size(), _pDWTF, &rd2d, _pD2DBrush[cycle + 2], _drawtext_option);

			r_ex.right = r.right;
		}
		else
		{
			r.left = r_ex.right;
			r.right = r_ex.right + 1;

			DrawTextW(hdc, s.c_str(), -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);

			r_ex.right = r.right;

			SetTextColor(hdc, _pTextService->cx_colors[cycle + 2]);

			DrawTextW(hdc, s.c_str(), -1, &r,
				DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}
	}
}

void CCandidateWindow::_Destroy()
{
	UINT i;

	if(_hwnd != NULL)
	{
		DestroyWindow(_hwnd);
		_hwnd = NULL;
	}

	if(_pInputModeWindow)
	{
		_pInputModeWindow->_Destroy();
		delete _pInputModeWindow;
		_pInputModeWindow = NULL;
	}

	if(hFont != NULL)
	{
		DeleteObject(hFont);
	}

	for(i = 0; i < 8; i++)
	{
		if(_pD2DBrush[i] != NULL)
		{
			_pD2DBrush[i]->Release();
		}
	}
	if(_pD2DDCRT != NULL)
	{
		_pD2DDCRT->Release();
	}
	if(_pD2DFactory != NULL)
	{
		_pD2DFactory->Release();
	}
	if(_pDWTF != NULL)
	{
		_pDWTF->Release();
	}
	if(_pDWFactory != NULL)
	{
		_pDWFactory->Release();
	}
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
	ITfUIElementMgr *pUIElementMgr;
	BOOL bShow = TRUE;

	if(!_reg)
	{
		_InitList();
	}

	_Update();

	if((_hwnd == NULL) && (_depth == 0))
	{
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_PPV_ARGS(&pUIElementMgr)) == S_OK)
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

	if(_bShow)
	{
		if(_hwnd != NULL)
		{
			SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
			if(_depth == 0)
			{
				NotifyWinEvent(EVENT_OBJECT_IME_SHOW, _hwnd, OBJID_CLIENT, CHILDID_SELF);
			}
		}
	}
}

void CCandidateWindow::_EndUIElement()
{
	ITfUIElementMgr *pUIElementMgr;

	if((_hwnd == NULL) && (_depth == 0))
	{
		if(_pTextService->_GetThreadMgr()->QueryInterface(IID_PPV_ARGS(&pUIElementMgr)) == S_OK)
		{
			pUIElementMgr->EndUIElement(_dwUIElementId);
			pUIElementMgr->Release();
		}
	}

	if(_hwnd != NULL)
	{
		SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);
		if(_depth == 0)
		{
			NotifyWinEvent(EVENT_OBJECT_IME_HIDE, _hwnd, OBJID_CLIENT, CHILDID_SELF);
		}
	}

	_bShow = FALSE;
}

BOOL CCandidateWindow::_CanShowUIElement()
{
	ITfUIElementMgr *pUIElementMgr;
	BOOL bShow = TRUE;

	if(_pTextService->_GetThreadMgr()->QueryInterface(IID_PPV_ARGS(&pUIElementMgr)) == S_OK)
	{
		pUIElementMgr->BeginUIElement(this, &bShow, &_dwUIElementId);
		pUIElementMgr->EndUIElement(_dwUIElementId);
		pUIElementMgr->Release();
	}

	return bShow;
}

void CCandidateWindow::_CalcWindowRect()
{
	HMONITOR hMonitor;
	MONITORINFO mi;
	HDC hdc;
	HGDIOBJ font;
	RECT r, rw;
	POINT pt;
	int x, y, cx = 0, cy = 0, xmax = 0, cycle;
	UINT page, count, i;
	std::wstring s;
	WCHAR strPage[32];
	TEXTMETRICW tm;
	HRESULT hr;
	IDWriteTextLayout* pdwTL = NULL;
	DWRITE_TEXT_METRICS dwTM;

	if(_hwnd == NULL)
	{
		return;
	}

	pt.x = _rect.left;
	pt.y = _rect.bottom;
	hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	mi.cbSize = sizeof(mi);
	GetMonitorInfoW(hMonitor, &mi);
	rw = mi.rcWork;

	hdc = GetDC(_hwnd);

	if(_pD2DDCRT)
	{
		_pD2DDCRT->BindDC(hdc, &rw);
		_pD2DDCRT->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	font = SelectObject(hdc, hFont);
	GetTextMetricsW(hdc, &tm);

	ZeroMemory(&r, sizeof(r));
	r.right = _pTextService->cx_maxwidth - MERGIN_X * 2;
	if(r.right <= 0)
	{
		r.right = 1;
	}

	if(regwordul || regword)
	{
		if(_pDWFactory)
		{
			hr = _pDWFactory->CreateTextLayout(disptext.c_str(), (UINT32)disptext.size(), _pDWTF, 0.0F, 0.0F, &pdwTL);
			if(hr == S_OK)
			{
				hr = pdwTL->GetMetrics(&dwTM);
				pdwTL->Release();
			}
			if(hr == S_OK)
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
				r.bottom = (LONG)ceil(dwTM.height);
			}
		}
		else
		{
			DrawTextW(hdc, disptext.c_str(), -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}

		cx = r.right + MERGIN_X * 2;
		cy = r.bottom + MERGIN_Y * 2;
	}
	else if(_CandCount.size() != 0)
	{
		pt.x = 0;
		pt.y = 0;
		cx = r.right;

		GetCurrentPage(&page);
		count = 0;
		for(i = 0; i < page; i++)
		{
			count += _CandCount[i];
		}

		//最大幅を算出
		for(i = 0; i < _CandCount[page]; i++)
		{
			s.clear();
			for(cycle = 0; cycle <= 4; cycle++)
			{
				s += _MakeCandidateString(page, count, i, cycle);
			}

			r.left = 0;
			r.top = 0;
			r.right = 1;
			r.bottom = 1;

			if(_pDWFactory)
			{
				hr = _pDWFactory->CreateTextLayout(s.c_str(), (UINT32)s.size(), _pDWTF, 0.0F, 0.0F, &pdwTL);
				if(hr == S_OK)
				{
					hr = pdwTL->GetMetrics(&dwTM);
					pdwTL->Release();
				}
				if(hr == S_OK)
				{
					r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
					r.bottom = (LONG)ceil(dwTM.height);
				}
			}
			else
			{
				DrawTextW(hdc, s.c_str(), -1, &r,
					DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
			}

			if(r.right > cx)
			{
				cx = r.right;
			}
		}

		r.left = 0;
		r.top = 0;
		r.right = 1;
		r.bottom = 1;

		_snwprintf_s(strPage, _TRUNCATE, L"%s(%u/%u)%s", markNBSP, page + 1, _uPageCnt, markNBSP);

		if(_pDWFactory)
		{
			hr = _pDWFactory->CreateTextLayout(strPage, (UINT32)wcslen(strPage), _pDWTF, 0.0F, 0.0F, &pdwTL);
			if(hr == S_OK)
			{
				hr = pdwTL->GetMetrics(&dwTM);
				pdwTL->Release();
			}
			if(hr == S_OK)
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
				r.bottom = (LONG)ceil(dwTM.height);
			}
		}
		else
		{
			DrawTextW(hdc, strPage, -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}

		if(r.right > cx)
		{
			cx = r.right;
		}

		//実際の幅、高さを算出
		for(i = 0; i < _CandCount[page]; i++)
		{
			s.clear();
			for(cycle = 0; cycle <= 4; cycle++)
			{
				s += _MakeCandidateString(page, count, i, cycle);
			}

			r.left = 0;
			r.top = 0;
			r.right = 1;
			r.bottom = 1;

			if(_pDWFactory)
			{
				hr = _pDWFactory->CreateTextLayout(s.c_str(), (UINT32)s.size(), _pDWTF, 0.0F, 0.0F, &pdwTL);
				if(hr == S_OK)
				{
					hr = pdwTL->GetMetrics(&dwTM);
					pdwTL->Release();
				}
				if(hr == S_OK)
				{
					r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
					r.bottom = (LONG)ceil(dwTM.height);
				}
			}
			else
			{
				DrawTextW(hdc, s.c_str(), -1, &r,
					DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
			}

			if(_pTextService->cx_verticalcand)
			{
				if(i != 0)
				{
					pt.x = 0;
					pt.y += tm.tmHeight;
				}
			}
			else
			{
				if(pt.x + r.right > cx)
				{
					pt.x = 0;
					pt.y += tm.tmHeight;
				}
			}

			pt.x += r.right;

			if(pt.x > xmax)
			{
				xmax = pt.x;
			}
		}

		_snwprintf_s(strPage, _TRUNCATE, L"%s(%u/%u)%s", markNBSP, page + 1, _uPageCnt, markNBSP);

		r.left = 0;
		r.top = 0;
		r.right = 1;
		r.bottom = 1;

		if(_pDWFactory)
		{
			hr = _pDWFactory->CreateTextLayout(strPage, (UINT32)wcslen(strPage), _pDWTF, 0.0F, 0.0F, &pdwTL);
			if(hr == S_OK)
			{
				hr = pdwTL->GetMetrics(&dwTM);
				pdwTL->Release();
			}
			if(hr == S_OK)
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
				r.bottom = (LONG)ceil(dwTM.height);
			}
		}
		else
		{
			DrawTextW(hdc, strPage, -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}

		if(_pTextService->cx_verticalcand)
		{
			pt.x = 0;
			pt.y += tm.tmHeight;
		}
		else
		{
			if(pt.x + r.right > cx)
			{
				pt.x = 0;
				pt.y += tm.tmHeight;
			}
		}

		pt.x += r.right;

		if(pt.x > xmax)
		{
			xmax = pt.x;
		}

		//候補ウィンドウの幅、高さ
		cx = xmax + MERGIN_X * 2;
		cy = pt.y + tm.tmHeight + MERGIN_Y * 2;
	}

	//表示位置を算出
	if((rw.right - cx) < _rect.left)
	{
		x = rw.right - cx;
	}
	else if(_rect.left < rw.left)
	{
		x = rw.left;
	}
	else
	{
		x = _rect.left;
	}

	if((rw.bottom - cy) < _rect.bottom)
	{
		if(_rect.top < rw.bottom)
		{
			y = _rect.top - cy;
		}
		else
		{
			y = rw.bottom - cy;
		}
	}
	else if(_rect.bottom < rw.top)
	{
		y = rw.top;
	}
	else
	{
		y = _rect.bottom;
	}

	SelectObject(hdc, font);

	ReleaseDC(_hwnd, hdc);

	SetWindowPos(_hwnd, HWND_TOPMOST, x, y, cx, cy, SWP_NOACTIVATE);

	if(_pInputModeWindow)
	{
		_pInputModeWindow->_GetRect(&r);
		_pInputModeWindow->_Move(x + cx - r.right, y + cy + 1);
	}

	if(_pCandidateWindow == NULL)
	{
		NotifyWinEvent(EVENT_OBJECT_IME_CHANGE, _hwnd, OBJID_CLIENT, CHILDID_SELF);
	}
}

void CCandidateWindow::_Redraw()
{
	if(_hwnd != NULL)
	{
		InvalidateRect(_hwnd, NULL, TRUE);
		UpdateWindow(_hwnd);
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

	switch(sf)
	{
	case SKK_CANCEL:
		if(_pCandidateList)
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
					_HandleKey(0, NULL, 0, SKK_CANCEL);
					_EndReq();
				}
			}
			else
			{
				_HandleKey(0, NULL, 0, SKK_CANCEL);
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
								_HandleKey(0, NULL, 0, SKK_ENTER);
								_EndReq();
							}
						}
						else
						{
							_pTextService->candidx = index;
							_HandleKey(0, NULL, 0, SKK_ENTER);
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
	if(_pInputModeWindow)
	{
		_pInputModeWindow->_Show(TRUE);
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

	_uShowedCount = (UINT)_pTextService->cx_untilcandlist - 1;
	_uCount = (UINT)_pTextService->candidates.size() - _uShowedCount;

	_CandStr.clear();
	for(i = 0; i < _uCount; i++)
	{
		_CandStr.push_back(_pTextService->selkey[(i % MAX_SELKEY)][0]);
		_CandStr[i].append(markNo + _pTextService->candidates[_uShowedCount + i].first.first);

		if(_pTextService->cx_annotation &&
			!_pTextService->candidates[_uShowedCount + i].first.second.empty())
		{
			_CandStr[i].append(markAnnotation +
				_pTextService->candidates[_uShowedCount + i].first.second);
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

				if(_pInputModeWindow)
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
		if(_pCandidateList)
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
						_HandleKey(0, NULL, 0, SKK_CANCEL);
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
						_HandleKey(0, NULL, 0, SKK_PREV_CAND);
						_EndReq();
					}
				}
			}
			else
			{
				if(_pTextService->cx_untilcandlist == 1)
				{
					_HandleKey(0, NULL, 0, SKK_CANCEL);
				}
				else
				{
					_pTextService->candidx = _pTextService->cx_untilcandlist - 1;
					_HandleKey(0, NULL, 0, SKK_PREV_CAND);
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

void CCandidateWindow::_OnKeyDownRegword(UINT uVKey)
{
	std::wstring s;
	std::wstring regwordtextconv;
	std::wstring regwordtextcandidate;
	std::wstring regwordtextannotation;
	std::wsmatch result;
	WCHAR ch;
	BYTE sf;

	_GetChSf(uVKey, ch, sf);

	//確定していないとき
	if(!regwordfixed)
	{
		_pTextService->showcandlist = FALSE;	//候補一覧表示をループさせる
		_HandleKey(0, NULL, (WPARAM)uVKey, SKK_NULL);
		_Update();

		if(_pInputModeWindow)
		{
			_pInputModeWindow->_Redraw();
		}
		return;
	}

	if(_pTextService->_IsKeyVoid(ch, (BYTE)uVKey))
	{
		_pTextService->_UpdateLanguageBar();

		if(_pInputModeWindow)
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

				if(_pInputModeWindow)
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
						_HandleKey(0, NULL, 0, SKK_CANCEL);
					}
					else
					{
						_HandleKey(0, NULL, 0, SKK_PREV_CAND);
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

			//候補変換
			_pTextService->_ConvertWord(REQ_CONVERTCND, _pTextService->searchkeyorg, regwordtextcandidate, regwordtextconv);
			if(regwordtextconv.empty() || regwordtextconv == regwordtextcandidate)
			{
				//変換済み候補が空文字列または変化なしであれば未変換見出し語を見出し語とする
				_pTextService->searchkey = _pTextService->searchkeyorg;
			}

			_pTextService->candidates.push_back(CANDIDATE
				(CANDIDATEBASE(regwordtextconv, regwordtextannotation),
				(CANDIDATEBASE(regwordtextcandidate, regwordtextannotation))));
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
				_HandleKey(0, NULL, 0, SKK_ENTER);
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

			if(_pInputModeWindow)
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
					_HandleKey(0, NULL, 0, SKK_CANCEL);
				}
				else
				{
					_HandleKey(0, NULL, 0, SKK_PREV_CAND);
				}
				_EndReq();
			}
		}
		break;

	// surrogate pair

	case SKK_BACK:
		if(comptext.empty() && regwordtextpos > 0 && regwordtext.size() > 0)
		{
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
			HANDLE hCB;
			PWCHAR pwCB;
			if(OpenClipboard(NULL))
			{
				hCB = GetClipboardData(CF_UNICODETEXT);
				if(hCB != NULL)
				{
					pwCB = (PWCHAR)GlobalLock(hCB);
					if(pwCB != NULL)
					{
						s.assign(pwCB);
						s = std::regex_replace(s, std::wregex(L"[\\x00-\\x19]"), std::wstring(L""));
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
		_HandleKey(0, NULL, (WPARAM)uVKey, SKK_NULL);
		_Update();

		if(_pInputModeWindow)
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
			InvalidateRect(_hwnd, NULL, TRUE);
			UpdateWindow(_hwnd);
		}
	}
}

void CCandidateWindow::_EndCandidateList(BYTE sf)
{
	_pCandidateList->_InvokeSfHandler(sf);
	_pCandidateList->_EndCandidateList();
}

HRESULT CCandidateWindow::_HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam, BYTE bSf)
{
	return _pTextService->_HandleKey(ec, pContext, wParam, bSf);
}

void CCandidateWindow::_GetChSf(UINT uVKey, WCHAR &ch, BYTE &sf, BYTE vkoff)
{
	ch = _pTextService->_GetCh(uVKey, vkoff);
	sf = _pTextService->_GetSf(uVKey, ch);
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
	_pCandidateWindow = new CCandidateWindow(_pTextService);
	if(_pCandidateWindow)
	{
		_pCandidateWindow->_Create(_hwndParent, this, _dwUIElementId, _depth + 1, reg);

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

		if(_pInputModeWindow)
		{
			_pInputModeWindow->_Show(FALSE);
		}
#endif
	}
}
