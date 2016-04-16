
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "CandidateWindow.h"

#define MERGIN_X 2
#define MERGIN_Y 4

void CCandidateWindow::_WindowProcPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HDC hmemdc = nullptr;
	HBITMAP hmembmp = nullptr;
	HPEN npen;
	HBRUSH nbrush;
	HGDIOBJ bmp = nullptr, font = nullptr, pen, brush;
	RECT r, rc;
	POINT pt;
	int cx, cy, cycle;
	UINT page, count, i;
	std::wstring s;
	WCHAR strPage[32];
	TEXTMETRICW tm;
	D2D1_RECT_F rd2d;
	DWRITE_TEXT_METRICS dwTM;
	LONG height = 0;

	hdc = BeginPaint(hWnd, &ps);

	GetClientRect(hWnd, &r);
	cx = r.right;
	cy = r.bottom;

	if(_pD2DDCRT != nullptr)
	{
		_pD2DDCRT->BindDC(hdc, &r);
		_pD2DDCRT->BeginDraw();
		_pD2DDCRT->SetTransform(D2D1::Matrix3x2F::Identity());
		_pD2DDCRT->Clear(D2D1::ColorF(SWAPRGB(_pTextService->cx_colors[CL_COLOR_BG])));
		rd2d = D2D1::RectF(0.5F, 0.5F, ((FLOAT)cx) - 0.5F, ((FLOAT)cy) - 0.5F);
		_pD2DDCRT->DrawRectangle(rd2d, _pD2DBrush[CL_COLOR_FR]);

		if(_GetTextMetrics(L"\x20", &dwTM) == S_OK)
		{
			height = (LONG)ceil(dwTM.height);
		}
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

		font = SelectObject(hmemdc, hFont);
		GetTextMetricsW(hmemdc, &tm);
		height = tm.tmHeight;
	}

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
			for(cycle = CL_COLOR_BG; cycle < DISPLAY_COLOR_NUM; cycle++)
			{
				s += _MakeCandidateString(page, count, i, cycle);
			}

			r.left = 0;
			r.top = 0;
			r.right = 1;
			r.bottom = 1;

			if(_pDWFactory != nullptr)
			{
				if(_GetTextMetrics(s.c_str(), &dwTM) == S_OK)
				{
					r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
				}
			}
			else
			{
				DrawTextW(hmemdc, s.c_str(), -1, &r,
					DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
			}

			if(_pTextService->cx_verticalcand || _comp)
			{
				if(i != 0)
				{
					pt.x = MERGIN_X;
					pt.y += height;
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
					pt.y += height;
				}
			}

			rc.left = pt.x;
			rc.top = pt.y;
			rc.right = pt.x + r.right;
			rc.bottom = pt.y + height;

			_PaintCandidate(hmemdc, &rc, page, count, i);

			pt.x += r.right;
		}

		_snwprintf_s(strPage, _TRUNCATE, L"%s(%u/%u)%s", markNBSP, page + 1, _uPageCnt, markNBSP);

		r.left = 0;
		r.top = 0;
		r.right = 1;
		r.bottom = 1;

		if(_pDWFactory != nullptr)
		{
			if(_GetTextMetrics(strPage, &dwTM) == S_OK)
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
			}
		}
		else
		{
			DrawTextW(hmemdc, strPage, -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}

		if(_pTextService->cx_verticalcand || _comp)
		{
			pt.x = MERGIN_X;
			pt.y += height;
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
				pt.y += height;
			}
		}

		rc.left = pt.x;
		rc.top = pt.y;
		rc.right = pt.x + r.right;
		rc.bottom = pt.y + height;

		if(_pD2DDCRT != nullptr && _pDWTF != nullptr)
		{
			rd2d = D2D1::RectF((FLOAT)rc.left, (FLOAT)rc.top, (FLOAT)rc.right, (FLOAT)rc.bottom);

			_pD2DDCRT->DrawText(strPage, (UINT32)wcslen(strPage),
				_pDWTF, &rd2d, _pD2DBrush[CL_COLOR_NO], _drawtext_option);
		}
		else
		{
			SetTextColor(hmemdc, _pTextService->cx_colors[CL_COLOR_NO]);
			SetBkMode(hdc, TRANSPARENT);

			DrawTextW(hmemdc, strPage, -1, &rc,
				DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}
	}

	if(_pD2DDCRT != nullptr)
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

	if(_pD2DDCRT != nullptr && _pDWTF != nullptr)
	{
		rd2d = D2D1::RectF((FLOAT)lpr->left, (FLOAT)lpr->top, (FLOAT)lpr->right, (FLOAT)lpr->bottom);

		_pD2DDCRT->DrawText(s.c_str(), (UINT32)s.size(),
			_pDWTF, &rd2d, _pD2DBrush[CL_COLOR_CA], _drawtext_option);
	}
	else
	{
		SetTextColor(hdc, _pTextService->cx_colors[CL_COLOR_CA]);
		SetBkMode(hdc, TRANSPARENT);

		DrawTextW(hdc, s.c_str(), -1, lpr,
			DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
	}
}

std::wstring CCandidateWindow::_MakeCandidateString(UINT page, UINT count, UINT idx, int cycle)
{
	std::wstring s;

	switch(cycle)
	{
	case CL_COLOR_BG:
		s.append(markNBSP);
		break;

	case CL_COLOR_FR:
		break;

	case CL_COLOR_SE:
		if(!_comp)
		{
			s.append(_pTextService->selkey[(idx % MAX_SELKEY_C)][0]);
		}
		else
		{
			s.append(searchkey);
		}
		break;

	case CL_COLOR_CO:
		if(!_comp)
		{
			s.append(markNo);
		}
		break;

	case CL_COLOR_CA:
		if(!_comp)
		{
			s.append(
				std::regex_replace(candidates[ count + _uShowedCount + idx ].first.first,
				std::wregex(markSP), std::wstring(markNBSP)));
		}
		else
		{
			s.append(
				std::regex_replace(candidates[ count + _uShowedCount + idx ].first.first.substr(searchkey.size()),
				std::wregex(markSP), std::wstring(markNBSP)));
		}
		break;

	case CL_COLOR_SC:
		if(!_comp)
		{
			if(_pTextService->cx_annotation &&
				!candidates[count + _uShowedCount + idx].first.second.empty())
			{
				s.append(markAnnotation);
			}
		}
		else
		{
			if(!candidates[count + _uShowedCount + idx].first.second.empty())
			{
				s.append(markNBSP);
			}
		}
		break;

	case CL_COLOR_AN:
		if(!_comp)
		{
			if(_pTextService->cx_annotation &&
				!candidates[count + _uShowedCount + idx].first.second.empty())
			{
				s.append(
					std::regex_replace(candidates[count + _uShowedCount + idx].first.second,
					std::wregex(markSP), std::wstring(markNBSP)));
			}
		}
		else
		{
			if(!candidates[count + _uShowedCount + idx].first.second.empty())
			{
				s.append(
					std::regex_replace(candidates[count + _uShowedCount + idx].first.second,
					std::wregex(markSP), std::wstring(markNBSP)));
			}
		}

		s.append(markNBSP);
		break;

	case CL_COLOR_NO:
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
	D2D1_RECT_F rd2d;
	DWRITE_TEXT_METRICS dwTM;

	r = *lpr;
	r_ex = *lpr;
	r_ex.right = r_ex.left;

	for(cycle = CL_COLOR_BG; cycle < DISPLAY_COLOR_NUM; cycle++)
	{
		s = _MakeCandidateString(page, count, idx, cycle);

		if(_pD2DDCRT != nullptr && _pDWTF != nullptr)
		{
			if(_GetTextMetrics(s.c_str(), &dwTM) == S_OK)
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
			}

			r.left = r_ex.right;
			r.top = lpr->top;
			r.right = r_ex.right + r.right;
			r.bottom = lpr->bottom;

			r_ex.right = r.right;

			rd2d = D2D1::RectF((FLOAT)r.left, (FLOAT)r.top, (FLOAT)r.right, (FLOAT)r.bottom);

			if(_comp && (count + _uShowedCount + idx == candidx) && (cycle == CL_COLOR_SE || cycle == CL_COLOR_CA))
			{
				_pD2DDCRT->FillRectangle(&rd2d, _pD2DBrush[CL_COLOR_SE]);
				_pD2DDCRT->DrawText(s.c_str(), (UINT32)s.size(),
					_pDWTF, &rd2d, _pD2DBrush[CL_COLOR_BG], _drawtext_option);
			}
			else
			{
				_pD2DDCRT->DrawText(s.c_str(), (UINT32)s.size(),
					_pDWTF, &rd2d, _pD2DBrush[cycle], _drawtext_option);
			}
		}
		else
		{
			r.left = r_ex.right;
			r.right = r_ex.right + 1;

			DrawTextW(hdc, s.c_str(), -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);

			r_ex.right = r.right;

			if(_comp && (count + _uShowedCount + idx == candidx) && (cycle == CL_COLOR_SE || cycle == CL_COLOR_CA))
			{
				SetTextColor(hdc, _pTextService->cx_colors[CL_COLOR_BG]);
				SetBkColor(hdc, _pTextService->cx_colors[CL_COLOR_SE]);
				SetBkMode(hdc, OPAQUE);
			}
			else
			{
				SetTextColor(hdc, _pTextService->cx_colors[cycle]);
				SetBkMode(hdc, TRANSPARENT);
			}

			DrawTextW(hdc, s.c_str(), -1, &r,
				DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}
	}
}

void CCandidateWindow::_CalcWindowRect()
{
	HMONITOR hMonitor;
	MONITORINFO mi;
	HDC hdc = nullptr;
	HGDIOBJ font = nullptr;
	RECT r, rw;
	POINT pt;
	int x, y, cx = 0, cy = 0, xmax = 0, cycle;
	UINT page, count, i;
	std::wstring s;
	WCHAR strPage[32];
	TEXTMETRICW tm;
	DWRITE_TEXT_METRICS dwTM;
	LONG height = 0;

	if(_hwnd == nullptr)
	{
		return;
	}

	pt.x = _rect.left;
	pt.y = _rect.bottom;
	hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	GetMonitorInfoW(hMonitor, &mi);
	rw = mi.rcWork;

	if(_pDWFactory != nullptr)
	{
		if(_GetTextMetrics(L"\x20", &dwTM) == S_OK)
		{
			height = (LONG)ceil(dwTM.height);
		}
	}
	else
	{
		hdc = GetDC(_hwnd);

		font = SelectObject(hdc, hFont);
		GetTextMetricsW(hdc, &tm);
		height = tm.tmHeight;
	}

	ZeroMemory(&r, sizeof(r));
	r.right = _pTextService->cx_maxwidth - MERGIN_X * 2;
	if(r.right <= 0)
	{
		r.right = 1;
	}

	if(regwordul || regword)
	{
		if(_pDWFactory != nullptr)
		{
			if(_GetTextMetrics(disptext.c_str(), &dwTM) == S_OK)
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
			}
		}
		else
		{
			DrawTextW(hdc, disptext.c_str(), -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}

		cx = r.right + MERGIN_X * 2;
		cy = height + MERGIN_Y * 2;
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
			for(cycle = CL_COLOR_BG; cycle < DISPLAY_COLOR_NUM; cycle++)
			{
				s += _MakeCandidateString(page, count, i, cycle);
			}

			r.left = 0;
			r.top = 0;
			r.right = 1;
			r.bottom = 1;

			if(_pDWFactory != nullptr)
			{
				if(_GetTextMetrics(s.c_str(), &dwTM) == S_OK)
				{
					r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
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

		if(_pDWFactory != nullptr)
		{
			if(_GetTextMetrics(strPage, &dwTM) == S_OK)
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
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
			for(cycle = CL_COLOR_BG; cycle < DISPLAY_COLOR_NUM; cycle++)
			{
				s += _MakeCandidateString(page, count, i, cycle);
			}

			r.left = 0;
			r.top = 0;
			r.right = 1;
			r.bottom = 1;

			if(_pDWFactory != nullptr)
			{
				if(_GetTextMetrics(s.c_str(), &dwTM) == S_OK)
				{
					r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
				}
			}
			else
			{
				DrawTextW(hdc, s.c_str(), -1, &r,
					DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
			}

			if(_pTextService->cx_verticalcand || _comp)
			{
				if(i != 0)
				{
					pt.x = 0;
					pt.y += height;
				}
			}
			else
			{
				if(pt.x + r.right > cx)
				{
					pt.x = 0;
					pt.y += height;
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

		if(_pDWFactory != nullptr)
		{
			if(_GetTextMetrics(strPage, &dwTM) == S_OK)
			{
				r.right = (LONG)ceil(dwTM.widthIncludingTrailingWhitespace);
			}
		}
		else
		{
			DrawTextW(hdc, strPage, -1, &r,
				DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_WORDBREAK | DT_NOFULLWIDTHCHARBREAK);
		}

		if(_pTextService->cx_verticalcand || _comp)
		{
			pt.x = 0;
			pt.y += height;
		}
		else
		{
			if(pt.x + r.right > cx)
			{
				pt.x = 0;
				pt.y += height;
			}
		}

		pt.x += r.right;

		if(pt.x > xmax)
		{
			xmax = pt.x;
		}

		//候補ウィンドウの幅、高さ
		cx = xmax + MERGIN_X * 2;
		cy = pt.y + height + MERGIN_Y * 2;
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

	if(_pDWFactory == nullptr)
	{
		SelectObject(hdc, font);
		ReleaseDC(_hwnd, hdc);
	}

	if(_vertical)
	{
		if(x < _rect.left)
		{
			x = _rect.left - (_rect.right - _rect.left) - cx;
			if(rw.right < (x + cx))
			{
				x = rw.right - cx;
			}
		}
	}

	SetWindowPos(_hwnd, HWND_TOPMOST, x, y, cx, cy, SWP_NOACTIVATE);

	if(_pInputModeWindow != nullptr)
	{
		_pInputModeWindow->_GetRect(&r);
		_pInputModeWindow->_Move(x + cx - r.right, y + cy + 1);
	}

	if(_pCandidateWindow == nullptr)
	{
		NotifyWinEvent(EVENT_OBJECT_IME_CHANGE, _hwnd, OBJID_CLIENT, CHILDID_SELF);
	}
}

HRESULT CCandidateWindow::_GetTextMetrics(LPCWSTR text, DWRITE_TEXT_METRICS *metrics)
{
	HRESULT hr = E_FAIL;

	if(metrics != nullptr && _pDWFactory != nullptr && _pDWTF != nullptr)
	{
		IDWriteTextLayout *pdwTL;
		if(_pDWFactory->CreateTextLayout(text, (UINT32)wcslen(text), _pDWTF, 0.0F, 0.0F, &pdwTL) == S_OK)
		{
			hr = pdwTL->GetMetrics(metrics);
			SafeRelease(&pdwTL);
		}
	}

	return hr;
}
