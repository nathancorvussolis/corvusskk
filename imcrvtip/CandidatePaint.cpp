
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "CandidateWindow.h"

#define ENABLE_DRAW_DEBUG_RECT 0

#ifndef _DEBUG
#undef ENABLE_DRAW_DEBUG_RECT
#endif

#define MARGIN_X 2
#define MARGIN_Y 4

const int colors_compback[DISPLAY_LIST_COLOR_NUM] =
{
	CL_COLOR_BG, CL_COLOR_FR, CL_COLOR_CA, CL_COLOR_CO,
	CL_COLOR_SE, CL_COLOR_SC, CL_COLOR_AN, CL_COLOR_NO
};

void CCandidateWindow::_WindowProcPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HDC hmemdc = nullptr;
	HBITMAP hmembmp = nullptr;
	HGDIOBJ bmp = nullptr, font = nullptr;
	RECT rc = {};
	int cx = 0, cy = 0;
	UINT page, count, i;
	LONG height = 0;
	CONST LONG maxwidth = (LONG)MulDiv(_pTextService->cx_maxwidth, _dpi, C_USER_DEFAULT_SCREEN_DPI);

	hdc = BeginPaint(hWnd, &ps);

	GetClientRect(hWnd, &rc);
	cx = rc.right;
	cy = rc.bottom;

	if (_pDWTF != nullptr)
	{
		_pD2DDCRT->BindDC(hdc, &rc);
		_pD2DDCRT->BeginDraw();
		_pD2DDCRT->SetTransform(D2D1::Matrix3x2F::Identity());

		_pD2DDCRT->Clear(D2D1::ColorF(SWAPRGB(_pTextService->cx_list_colors[CL_COLOR_BG])));

		D2D1_RECT_F rd2d = D2D1::RectF(0.5F, 0.5F, ((FLOAT)cx) - 0.5F, ((FLOAT)cy) - 0.5F);
		_pD2DDCRT->DrawRectangle(rd2d, _pD2DBrush[CL_COLOR_FR]);

#if ENABLE_DRAW_DEBUG_RECT
		_pD2DDCRT->DrawLine(
			D2D1::Point2F((FLOAT)maxwidth - 0.5F, 0.0F),
			D2D1::Point2F((FLOAT)maxwidth - 0.5F, (FLOAT)cy),
			_pD2DBrush[CL_COLOR_SE]);
#endif

		DWRITE_TEXT_METRICS dwTM = {};

		if (SUCCEEDED(_GetTextMetrics(L"\x20", &dwTM)))
		{
			height = (LONG)ceil(dwTM.height);
		}
	}
	else
	{
		hmemdc = CreateCompatibleDC(hdc);
		hmembmp = CreateCompatibleBitmap(hdc, cx, cy);
		bmp = SelectObject(hmemdc, hmembmp);

		HPEN npen = CreatePen(PS_SOLID, 1, _pTextService->cx_list_colors[CL_COLOR_FR]);
		HGDIOBJ pen = SelectObject(hmemdc, npen);
		HBRUSH nbrush = CreateSolidBrush(_pTextService->cx_list_colors[CL_COLOR_BG]);
		HGDIOBJ brush = SelectObject(hmemdc, nbrush);

		Rectangle(hmemdc, 0, 0, cx, cy);

		SelectObject(hmemdc, pen);
		SelectObject(hmemdc, brush);

		DeleteObject(npen);
		DeleteObject(nbrush);

#if ENABLE_DRAW_DEBUG_RECT
		HPEN penmw = CreatePen(PS_SOLID, 1, _pTextService->cx_list_colors[CL_COLOR_SE]);
		pen = SelectObject(hmemdc, penmw);

		POINT ptmw[2] = { {maxwidth, 0}, {maxwidth, cy} };
		Polyline(hmemdc, ptmw, 2);

		SelectObject(hmemdc, GetStockObject(BLACK_PEN));
		SelectObject(hmemdc, GetStockObject(NULL_BRUSH));

		DeleteObject(penmw);
#endif

		SetBkMode(hmemdc, TRANSPARENT);

		font = SelectObject(hmemdc, hFont);

		TEXTMETRICW tm = {};
		GetTextMetricsW(hmemdc, &tm);
		height = tm.tmHeight;
	}

	if (_regmode || (_mode == wm_delete))
	{
		RECT r = {
			rc.left + MARGIN_X,
			rc.top + MARGIN_Y,
			rc.right - MARGIN_X,
			rc.bottom - MARGIN_Y
		};

		_PaintWord(hmemdc, &r);
	}
	else if (((_mode == wm_candidate) || (_mode == wm_complement)) && (_CandCount.size() != 0))
	{
		POINT pt = { MARGIN_X, MARGIN_Y };

		GetCurrentPage(&page);
		count = 0;
		for (i = 0; i < page; i++)
		{
			count += _CandCount[i];
		}

		for (i = 0; i < _CandCount[page]; i++)
		{
			LONG width = 0;

			for (int cycle = 0; cycle < DISPLAY_LIST_COLOR_NUM; cycle++)
			{
				std::wstring s = _MakeCandidateString(page, count, i, cycle);

				if (_pDWFactory != nullptr)
				{
					DWRITE_TEXT_METRICS dwTM = {};

					if (SUCCEEDED(_GetTextMetrics(s.c_str(), &dwTM)))
					{
						width += (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
					}
				}
				else
				{
					RECT r = { 0, 0, 1, 1 };

					DrawTextW(hmemdc, s.c_str(), -1, &r,
						DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);

					width += r.right;
				}
			}

			if (_pTextService->cx_verticalcand || (_mode == wm_complement))
			{
				if (i != 0)
				{
					pt.x = MARGIN_X;
					pt.y += height;
				}
			}
			else
			{
				if ((pt.x != MARGIN_X) && (pt.x + width > maxwidth - MARGIN_X))
				{
					pt.x = MARGIN_X;
					pt.y += height;
				}
			}

			RECT r = { pt.x, pt.y, pt.x + width, pt.y + height };

			_PaintCandidate(hmemdc, &r, page, count, i);

			pt.x += width;
		}

		WCHAR strPage[32];
		_snwprintf_s(strPage, _TRUNCATE, L"%s(%u/%u)%s", markNBSP, page + 1, _uPageCnt, markNBSP);

		LONG width = 0;

		if (_pDWFactory != nullptr)
		{
			DWRITE_TEXT_METRICS dwTM = {};

			if (SUCCEEDED(_GetTextMetrics(strPage, &dwTM)))
			{
				width = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
			}
		}
		else
		{
			RECT r = { 0, 0, 1, 1 };

			DrawTextW(hmemdc, strPage, -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);

			width = r.right;
		}

		if (_pTextService->cx_verticalcand || (_mode == wm_complement))
		{
			pt.x = MARGIN_X;
			pt.y += height;
		}
		else
		{
			if ((pt.x != MARGIN_X) && (pt.x + width > maxwidth - MARGIN_X))
			{
				pt.x = MARGIN_X;
				pt.y += height;
			}
		}

		RECT r = { pt.x, pt.y, pt.x + width, pt.y + height };

		if (_pDWTF != nullptr)
		{
			D2D1_RECT_F rd2d = D2D1::RectF((FLOAT)r.left, (FLOAT)r.top, (FLOAT)r.right, (FLOAT)r.bottom);

#if ENABLE_DRAW_DEBUG_RECT
			_pD2DDCRT->DrawRectangle(
				D2D1::RectF(
					rd2d.left + 0.5F,
					rd2d.top + 0.5F,
					rd2d.right - 0.5F,
					rd2d.bottom - 0.5F),
				_pD2DBrush[CL_COLOR_FR]);
#endif

			_pD2DDCRT->DrawText(strPage, (UINT32)wcslen(strPage),
				_pDWTF, &rd2d, _pD2DBrush[CL_COLOR_NO], _drawtext_option);
		}
		else
		{
			SetTextColor(hmemdc, _pTextService->cx_list_colors[CL_COLOR_NO]);
			SetBkMode(hdc, TRANSPARENT);

#if ENABLE_DRAW_DEBUG_RECT
			Rectangle(hmemdc, r.left, r.top, r.right, r.bottom);
#endif

			DrawTextW(hmemdc, strPage, -1, &r,
				DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}
	}

	if (_pDWTF != nullptr)
	{
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
}

std::wstring CCandidateWindow::_MakeRegWordString()
{
	std::wstring s;
	UINT i;

	s.append(markNBSP);
	for (i = 0; i < _depth + 1; i++)
	{
		s.append(markSqbL);
	}
	s.append(L"登録");
	for (i = 0; i < _depth + 1; i++)
	{
		s.append(markSqbR);
	}
	s.append(markNBSP);

	s.append((searchkey_bak.empty() ? searchkeyorg_bak : searchkey_bak) + L"：");

	s.append(_regtext.substr(0, _regtextpos));

	s.append(_regcomp + markCursor);

	s.append(_regtext.substr(_regtextpos) + markNBSP);

	return s;
}

std::wstring CCandidateWindow::_MakeDelWordString()
{
	std::wstring s;

	s.append(markNBSP);
	s.append(markSqbL);
	s.append(L"削除");
	s.append(markSqbR);

	s.append(markNBSP + ((candorgcnt <= candidx) ? searchkey : searchkeyorg) + markNBSP);
	s.append(L"/" + candidates[candidx].second.first);
	if (!candidates[candidx].second.second.empty())
	{
		s.append(markAnnotation + candidates[candidx].second.second);
	}
	s.append(L"/");

	s.append(markNBSP);
	s.append(L"？(Y/n)");
	s.append(markNBSP);

	return s;
}

void CCandidateWindow::_PaintWord(HDC hdc, LPRECT lpr)
{
	std::wstring s;

	if (_regmode)
	{
		s = _MakeRegWordString();
	}
	else if (_mode == wm_delete)
	{
		s = _MakeDelWordString();
	}

	if (_pDWTF != nullptr)
	{
		D2D1_RECT_F rd2d = D2D1::RectF((FLOAT)lpr->left, (FLOAT)lpr->top, (FLOAT)lpr->right, (FLOAT)lpr->bottom);

#if ENABLE_DRAW_DEBUG_RECT
		_pD2DDCRT->DrawRectangle(
			D2D1::RectF(
				rd2d.left + 0.5F,
				rd2d.top + 0.5F,
				rd2d.right - 0.5F,
				rd2d.bottom - 0.5F),
			_pD2DBrush[CL_COLOR_FR]);
#endif

		_pD2DDCRT->DrawText(s.c_str(), (UINT32)s.size(),
			_pDWTF, &rd2d, _pD2DBrush[CL_COLOR_CA], _drawtext_option);
	}
	else
	{
		SetTextColor(hdc, _pTextService->cx_list_colors[CL_COLOR_CA]);
		SetBkMode(hdc, TRANSPARENT);

#if ENABLE_DRAW_DEBUG_RECT
		Rectangle(hdc, lpr->left, lpr->top, lpr->right, lpr->bottom);
#endif

		DrawTextW(hdc, s.c_str(), -1, lpr,
			DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
	}
}

std::wstring CCandidateWindow::_MakeCandidateString(UINT page, UINT count, UINT idx, int cycle)
{
	std::wstring s;
	std::wstring ca = candidates[count + _uShowedCount + idx].first.first;
	std::wstring an = candidates[count + _uShowedCount + idx].first.second;

	int color_cycle = cycle;
	if ((_mode == wm_complement) && (ca.compare(0, searchkey.size(), searchkey) != 0))
	{
		//補完かつ後方一致
		color_cycle = colors_compback[cycle];
	}

	switch (color_cycle)
	{
	case CL_COLOR_BG:
		break;

	case CL_COLOR_FR:
		s.append(markNBSP);
		break;

	case CL_COLOR_SE:
		if (_mode == wm_candidate)
		{
			s.append(_pTextService->selkey[(idx % MAX_SELKEY_C)][0]);
		}
		else
		{
			s.append(searchkey);
		}
		break;

	case CL_COLOR_CO:
		if (_mode == wm_candidate)
		{
			s.append(markNo);
		}
		break;

	case CL_COLOR_CA:
		if (_mode == wm_candidate)
		{
			s.append(std::regex_replace(ca,
				std::wregex(markSP), std::wstring(markNBSP)));
		}
		else
		{
			if (searchkey.size() < ca.size())
			{
				if (ca.compare(0, searchkey.size(), searchkey) == 0)
				{
					//前方一致
					s.append(std::regex_replace(ca.substr(searchkey.size()),
						std::wregex(markSP), std::wstring(markNBSP)));
				}
				else
				{
					//後方一致
					s.append(std::regex_replace(ca.substr(0, ca.size() - searchkey.size()),
						std::wregex(markSP), std::wstring(markNBSP)));
				}
			}
		}
		break;

	case CL_COLOR_SC:
		if (_mode == wm_candidate)
		{
			if (_pTextService->cx_annotation && !an.empty())
			{
				s.append(markAnnotation);
			}
		}
		else
		{
			if (!an.empty())
			{
				s.append(markNBSP);
			}
		}
		break;

	case CL_COLOR_AN:
		if (_mode == wm_candidate)
		{
			if (_pTextService->cx_annotation && !an.empty())
			{
				s.append(std::regex_replace(an,
					std::wregex(markSP), std::wstring(markNBSP)));
			}
		}
		else
		{
			if (!an.empty())
			{
				s.append(std::regex_replace(an,
					std::wregex(markSP), std::wstring(markNBSP)));
			}
		}
		break;

	case CL_COLOR_NO:
		s.append(markNBSP);
		break;

	default:
		break;
	}

	return s;
}

void CCandidateWindow::_PaintCandidate(HDC hdc, LPRECT lpr, UINT page, UINT count, UINT idx)
{
	RECT r = *lpr;
	RECT r_ex = *lpr;
	r_ex.right = r_ex.left;

	std::wstring ca = candidates[count + _uShowedCount + idx].first.first;

	for (int cycle = 0; cycle < DISPLAY_LIST_COLOR_NUM; cycle++)
	{
		std::wstring s = _MakeCandidateString(page, count, idx, cycle);

		int color_cycle = cycle;
		if ((_mode == wm_complement) && (ca.compare(0, searchkey.size(), searchkey) != 0))
		{
			//補完かつ後方一致
			color_cycle = colors_compback[cycle];
		}

		if (_pDWTF != nullptr)
		{
			DWRITE_TEXT_METRICS dwTM = {};

			if (SUCCEEDED(_GetTextMetrics(s.c_str(), &dwTM)))
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
			}

			r.left = r_ex.right;
			r.top = lpr->top;
			r.right = r_ex.right + r.right;
			r.bottom = lpr->bottom;

			r_ex.right = r.right;

			D2D1_RECT_F rd2d = D2D1::RectF((FLOAT)r.left, (FLOAT)r.top, (FLOAT)r.right, (FLOAT)r.bottom);

			if ((_mode == wm_complement) &&
				(count + _uShowedCount + idx == candidx) &&
				(color_cycle == CL_COLOR_SE || color_cycle == CL_COLOR_CA))
			{
				_pD2DDCRT->FillRectangle(&rd2d, _pD2DBrush[CL_COLOR_SE]);
				_pD2DDCRT->DrawText(s.c_str(), (UINT32)s.size(),
					_pDWTF, &rd2d, _pD2DBrush[CL_COLOR_BG], _drawtext_option);
			}
			else
			{
#if ENABLE_DRAW_DEBUG_RECT
				if (!s.empty())
				{
					_pD2DDCRT->DrawRectangle(
						D2D1::RectF(
							rd2d.left + 0.5F,
							rd2d.top + 0.5F,
							rd2d.right - 0.5F,
							rd2d.bottom - 0.5F),
						_pD2DBrush[CL_COLOR_FR]);
				}
#endif

				_pD2DDCRT->DrawText(s.c_str(), (UINT32)s.size(),
					_pDWTF, &rd2d, _pD2DBrush[color_cycle], _drawtext_option);
			}
		}
		else
		{
			r.left = r_ex.right;
			r.right = r_ex.right + 1;

			DrawTextW(hdc, s.c_str(), -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);

			r_ex.right = r.right;

			if ((_mode == wm_complement) &&
				(count + _uShowedCount + idx == candidx) &&
				(color_cycle == CL_COLOR_SE || color_cycle == CL_COLOR_CA))
			{
				SetTextColor(hdc, _pTextService->cx_list_colors[CL_COLOR_BG]);
				SetBkColor(hdc, _pTextService->cx_list_colors[CL_COLOR_SE]);
				SetBkMode(hdc, OPAQUE);
			}
			else
			{
				SetTextColor(hdc, _pTextService->cx_list_colors[color_cycle]);
				SetBkMode(hdc, TRANSPARENT);
			}

#if ENABLE_DRAW_DEBUG_RECT
			Rectangle(hdc, r.left, r.top, r.right, r.bottom);
#endif

			DrawTextW(hdc, s.c_str(), -1, &r,
				DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}
	}
}

void CCandidateWindow::_CalcWindowRect()
{
	HDC hdc = nullptr;
	HGDIOBJ font = nullptr;
	int cx = 0, cy = 0;
	UINT page, count, i;
	LONG height = 0;
	CONST LONG maxwidth = (LONG)MulDiv(_pTextService->cx_maxwidth, _dpi, C_USER_DEFAULT_SCREEN_DPI);

	if (_hwnd == nullptr)
	{
		return;
	}

	if (_pDWFactory != nullptr)
	{
		DWRITE_TEXT_METRICS dwTM = {};

		if (SUCCEEDED(_GetTextMetrics(L"\x20", &dwTM)))
		{
			height = (LONG)ceil(dwTM.height);
		}
	}
	else
	{
		hdc = GetDC(_hwnd);

		font = SelectObject(hdc, hFont);

		TEXTMETRICW tm = {};
		GetTextMetricsW(hdc, &tm);
		height = tm.tmHeight;
	}

	if (_regmode || (_mode == wm_delete))
	{
		LONG width = 0;

		if (_pDWFactory != nullptr)
		{
			DWRITE_TEXT_METRICS dwTM = {};

			if (SUCCEEDED(_GetTextMetrics(disptext.c_str(), &dwTM)))
			{
				width = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
			}
		}
		else
		{
			RECT r = { 0, 0, 1, 1 };

			DrawTextW(hdc, disptext.c_str(), -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);

			width = r.right;
		}

		cx = width + MARGIN_X * 2;
		cy = height + MARGIN_Y * 2;
	}
	else if (((_mode == wm_candidate) || (_mode == wm_complement)) && (_CandCount.size() != 0))
	{
		int xmax = 0;
		POINT pt = { MARGIN_X, MARGIN_Y };

		GetCurrentPage(&page);
		count = 0;
		for (i = 0; i < page; i++)
		{
			count += _CandCount[i];
		}

		for (i = 0; i < _CandCount[page]; i++)
		{
			LONG width = 0;

			for (int cycle = 0; cycle < DISPLAY_LIST_COLOR_NUM; cycle++)
			{
				std::wstring s = _MakeCandidateString(page, count, i, cycle);

				if (_pDWFactory != nullptr)
				{
					DWRITE_TEXT_METRICS dwTM = {};

					if (SUCCEEDED(_GetTextMetrics(s.c_str(), &dwTM)))
					{
						width += (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
					}
				}
				else
				{
					RECT r = { 0, 0, 1, 1 };

					DrawTextW(hdc, s.c_str(), -1, &r,
						DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);

					width += r.right;
				}
			}

			if (_pTextService->cx_verticalcand || (_mode == wm_complement))
			{
				if (i != 0)
				{
					pt.x = MARGIN_X;
					pt.y += height;
				}
			}
			else
			{
				if ((pt.x != MARGIN_X) && (pt.x + width > maxwidth - MARGIN_X))
				{
					pt.x = MARGIN_X;
					pt.y += height;
				}
			}

			pt.x += width;

			if (pt.x > xmax)
			{
				xmax = pt.x;
			}
		}

		WCHAR strPage[32];
		_snwprintf_s(strPage, _TRUNCATE, L"%s(%u/%u)%s", markNBSP, page + 1, _uPageCnt, markNBSP);

		LONG width = 0;

		if (_pDWFactory != nullptr)
		{
			DWRITE_TEXT_METRICS dwTM = {};

			if (SUCCEEDED(_GetTextMetrics(strPage, &dwTM)))
			{
				width = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
			}
		}
		else
		{
			RECT r = { 0, 0, 1, 1 };

			DrawTextW(hdc, strPage, -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);

			width = r.right;
		}

		if (_pTextService->cx_verticalcand || (_mode == wm_complement))
		{
			pt.x = MARGIN_X;
			pt.y += height;
		}
		else
		{
			if ((pt.x != MARGIN_X) && (pt.x + width > maxwidth - MARGIN_X))
			{
				pt.x = MARGIN_X;
				pt.y += height;
			}
		}

		pt.x += width;

		if (pt.x > xmax)
		{
			xmax = pt.x;
		}

		//候補ウィンドウの幅、高さ
		cx = xmax + MARGIN_X;
		cy = pt.y + height + MARGIN_Y;
	}

	if (_pDWFactory == nullptr)
	{
		SelectObject(hdc, font);
		ReleaseDC(_hwnd, hdc);
	}

	//表示位置を算出
	//親ウィンドウの左下が表示されているモニタのワークエリア内に収まるように配置

	POINT mpt = { _rect.left, _rect.bottom };
	HMONITOR hMonitor = MonitorFromPoint(mpt, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = {};
	mi.cbSize = sizeof(mi);
	GetMonitorInfoW(hMonitor, &mi);
	RECT rw = mi.rcWork;
	int x = 0, y = 0;

	if ((rw.right - cx) < _rect.left)
	{
		x = rw.right - cx;
	}
	else if (_rect.left < rw.left)
	{
		x = rw.left;
	}
	else
	{
		x = _rect.left;
	}

	if ((rw.bottom - cy) < _rect.bottom)
	{
		if (_rect.top < rw.bottom)
		{
			y = _rect.top - cy;
		}
		else
		{
			y = rw.bottom - cy;
		}
	}
	else if (_rect.bottom < rw.top)
	{
		y = rw.top;
	}
	else
	{
		y = _rect.bottom;
	}

	if (_vertical)
	{
		if (x < _rect.left)
		{
			x = _rect.left - (_rect.right - _rect.left) - cx;
			if (rw.right < (x + cx))
			{
				x = rw.right - cx;
			}
		}
	}

	SetWindowPos(_hwnd, HWND_TOPMOST, x, y, cx, cy, SWP_NOACTIVATE);

	if (_pInputModeWindow != nullptr)
	{
		RECT r = {};
		_pInputModeWindow->_GetRect(&r);
		_pInputModeWindow->_Move(x + cx - r.right, y + cy + 1);
	}

	if (_pCandidateWindow == nullptr)
	{
		NotifyWinEvent(EVENT_OBJECT_IME_CHANGE, _hwnd, OBJID_CLIENT, CHILDID_SELF);
	}
}

HRESULT CCandidateWindow::_GetTextMetrics(LPCWSTR text, DWRITE_TEXT_METRICS *metrics)
{
	HRESULT hr = E_FAIL;

	if (metrics != nullptr && _pDWTF != nullptr)
	{
		CComPtr<IDWriteTextLayout> pdwTL;
		if (SUCCEEDED(_pDWFactory->CreateTextLayout(text, (UINT32)wcslen(text), _pDWTF, 0.0F, 0.0F, &pdwTL)) && (pdwTL != nullptr))
		{
			hr = pdwTL->GetMetrics(metrics);
		}
	}

	return hr;
}

void CCandidateWindow::_InitFont()
{
	LOGFONTW lf = {};
	lf.lfHeight = -MulDiv(_pTextService->cx_fontpoint, _dpi, C_FONT_LOGICAL_HEIGHT_PPI);
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = _pTextService->cx_fontweight;
	lf.lfItalic = _pTextService->cx_fontitalic;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = SHIFTJIS_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = PROOF_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH;
	wcscpy_s(lf.lfFaceName, _pTextService->cx_fontname);

	if (_pDWFactory == nullptr)
	{
		hFont = CreateFontIndirectW(&lf);
	}
	else
	{
		HRESULT hr = _pDWFactory->CreateTextFormat(
			_pTextService->cx_fontname,
			nullptr,
			static_cast<DWRITE_FONT_WEIGHT>(_pTextService->cx_fontweight),
			(_pTextService->cx_fontitalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL),
			DWRITE_FONT_STRETCH_NORMAL,
			(FLOAT)MulDiv(_pTextService->cx_fontpoint, _dpi, C_FONT_LOGICAL_HEIGHT_PPI),
			L"ja-JP",
			&_pDWTF);

		if (SUCCEEDED(hr))
		{
			hr = _pDWTF->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		}

		if (FAILED(hr))
		{
			_UninitFont();

			//use GDI font
			hFont = CreateFontIndirectW(&lf);
		}
	}
}

void CCandidateWindow::_UninitFont()
{
	if (hFont != nullptr)
	{
		DeleteObject(hFont);
		hFont = nullptr;
	}

	_pDWTF.Release();
}

void CCandidateWindow::_WindowProcDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_dpi = HIWORD(wParam);

	_UninitFont();
	_InitFont();

	_CalcWindowRect();
	_Redraw();
}
