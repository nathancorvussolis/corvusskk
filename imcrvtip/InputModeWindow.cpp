
#include "imcrvtip.h"
#include "TextService.h"
#include "InputModeWindow.h"

#define INPUTMODE_TIMER_ID		0x54ab516b

#define IM_MARGIN_X 2
#define IM_MARGIN_Y 2

class CInputModeWindowGetTextExtEditSession : public CEditSessionBase
{
public:
	CInputModeWindowGetTextExtEditSession(CTextService *pTextService, ITfContext *pContext,
		ITfContextView *pContextView, CInputModeWindow *pInputModeWindow) : CEditSessionBase(pTextService, pContext)
	{
		_pInputModeWindow = pInputModeWindow;
		_pContextView = pContextView;
	}

	~CInputModeWindowGetTextExtEditSession()
	{
		_pInputModeWindow.Release();
		_pContextView.Release();
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		TF_SELECTION tfSelection = {};
		ULONG cFetched = 0;
		if (FAILED(_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched)))
		{
			return E_FAIL;
		}

		CComPtr<ITfRange> pRangeSelection;
		pRangeSelection.Attach(tfSelection.range);

		if (cFetched != 1)
		{
			return E_FAIL;
		}

		RECT rc = {};
		BOOL fClipped;
		if (FAILED(_pContextView->GetTextExt(ec, tfSelection.range, &rc, &fClipped)))
		{
			return E_FAIL;
		}

		//ignore abnormal position (from CUAS ?)
		if ((rc.top == rc.bottom) && ((rc.right - rc.left) == 1))
		{
			return E_FAIL;
		}

		POINT pt = {};
		pt.x = rc.left;
		pt.y = rc.bottom;
		HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

		MONITORINFO mi = {};
		mi.cbSize = sizeof(mi);
		GetMonitorInfoW(hMonitor, &mi);

		RECT rw = {};
		_pInputModeWindow->_GetRect(&rw);
		LONG height = rw.bottom - rw.top;
		LONG width = rw.right - rw.left;

		if (rc.left < mi.rcWork.left)
		{
			rc.left = mi.rcWork.left;
		}

		if (mi.rcWork.right < rc.right)
		{
			rc.left = mi.rcWork.right - width;
		}

		if (mi.rcWork.bottom < rc.top)
		{
			rc.bottom = mi.rcWork.bottom - height - IM_MARGIN_Y;
		}
		else if (mi.rcWork.bottom < (rc.bottom + height + IM_MARGIN_Y))
		{
			rc.bottom = rc.top - height - IM_MARGIN_Y * 2;
		}

		if (rc.bottom < mi.rcWork.top)
		{
			rc.bottom = mi.rcWork.top - IM_MARGIN_Y;
		}

		_pInputModeWindow->_Move(rc.left, rc.bottom + IM_MARGIN_Y);
		_pInputModeWindow->_Show(TRUE);

		return S_OK;
	}

private:
	CComPtr<ITfContextView> _pContextView;
	CComPtr<CInputModeWindow> _pInputModeWindow;
};

CInputModeWindow::CInputModeWindow()
{
	DllAddRef();

	_cRef = 1;

	_pContext = nullptr;
	_dwCookieTextLayoutSink= TF_INVALID_COOKIE;
	_pTextService = nullptr;
	_hwndParent = nullptr;
	_hwnd = nullptr;
	_bCandidateWindow = FALSE;

	HDC hdc = GetDC(nullptr);
	_dpi = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(nullptr, hdc);

	_size = MulDiv(16, _dpi, C_USER_DEFAULT_SCREEN_DPI);
}

CInputModeWindow::~CInputModeWindow()
{
	_Destroy();

	DllRelease();
}

STDAPI CInputModeWindow::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_INVALIDARG;
	}

	*ppvObj = nullptr;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextLayoutSink))
	{
		*ppvObj = static_cast<ITfTextLayoutSink *>(this);
	}

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) CInputModeWindow::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) CInputModeWindow::Release()
{
	if (--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CInputModeWindow::OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pContextView)
{
	HRESULT hr;

	if (pContext != _pContext)
	{
		return S_OK;
	}

	switch (lcode)
	{
	case TF_LC_CREATE:
		break;

	case TF_LC_CHANGE:
		try
		{
			CComPtr<ITfEditSession> pEditSession;
			pEditSession.Attach(
				new CInputModeWindowGetTextExtEditSession(_pTextService, pContext, pContextView, this));
			pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
		}
		catch (...)
		{
		}
		break;

	case TF_LC_DESTROY:
		_Destroy();
		break;

	default:
		break;
	}

	return S_OK;
}

HRESULT CInputModeWindow::_AdviseTextLayoutSink()
{
	HRESULT hr = E_FAIL;

	CComPtr<ITfSource> pSource;
	if (SUCCEEDED(_pContext->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
	{
		hr = pSource->AdviseSink(IID_IUNK_ARGS(static_cast<ITfTextLayoutSink *>(this)), &_dwCookieTextLayoutSink);
	}

	return hr;
}

HRESULT CInputModeWindow::_UnadviseTextLayoutSink()
{
	HRESULT hr = E_FAIL;

	if (_pContext != nullptr)
	{
		CComPtr<ITfSource> pSource;
		if (SUCCEEDED(_pContext->QueryInterface(IID_PPV_ARGS(&pSource))) && (pSource != nullptr))
		{
			hr = pSource->UnadviseSink(_dwCookieTextLayoutSink);
		}
	}

	return hr;
}

BOOL CInputModeWindow::_Create(CTextService *pTextService, ITfContext *pContext, BOOL bCandidateWindow, HWND hWnd)
{
	POINT pt = {};

	if (pContext != nullptr)
	{
		_pContext = pContext;
		if (FAILED(_AdviseTextLayoutSink()))
		{
			return FALSE;
		}
	}

	if (!bCandidateWindow && _pContext == nullptr)
	{
		return FALSE;
	}

	_pTextService = pTextService;

	_bCandidateWindow = bCandidateWindow;

	if (_bCandidateWindow)
	{
		_hwndParent = hWnd;
	}
	else
	{
		CComPtr<ITfContextView> pContextView;
		if (SUCCEEDED(_pContext->GetActiveView(&pContextView)) && (pContextView != nullptr))
		{
			if (FAILED(pContextView->GetWnd(&_hwndParent)) || _hwndParent == nullptr)
			{
				_hwndParent = GetFocus();
			}
		}
	}

	_hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		InputModeWindowClass, L"", WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		_hwndParent, nullptr, g_hInst, this);

	if (_hwnd == nullptr)
	{
		return FALSE;
	}

	if (_bCandidateWindow)
	{
		RECT r = {};
		GetClientRect(_hwndParent, &r);
		pt.x = r.left;
		pt.y = r.bottom;
		ClientToScreen(_hwndParent, &pt);
	}

	SetWindowPos(_hwnd, HWND_TOPMOST, pt.x, pt.y + IM_MARGIN_Y,
		_size + IM_MARGIN_X * 2, _size + IM_MARGIN_Y * 2, SWP_NOACTIVATE);

	return TRUE;
}

BOOL CInputModeWindow::_InitClass()
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc = CInputModeWindow::_WindowPreProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = g_hInst;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = InputModeWindowClass;
	wcex.hIconSm = nullptr;

	ATOM atom = RegisterClassExW(&wcex);

	return (atom != 0);
}

void CInputModeWindow::_UninitClass()
{
	UnregisterClassW(InputModeWindowClass, g_hInst);
}

LRESULT CALLBACK CInputModeWindow::_WindowPreProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CInputModeWindow *pInputModeWindow = nullptr;

	switch (uMsg)
	{
	case WM_NCCREATE:
		pInputModeWindow = (CInputModeWindow *)((LPCREATESTRUCTW)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pInputModeWindow);
		break;
	default:
		pInputModeWindow = (CInputModeWindow *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		break;
	}

	if (pInputModeWindow != nullptr)
	{
		return pInputModeWindow->_WindowProc(hWnd, uMsg, wParam, lParam);
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CInputModeWindow::_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HDC hmemdc, hmemdcR;
	HBITMAP hmembmp, bmp, hmembmpR, bmpR;
	HPEN pen, npen, penR, npenR;
	HBRUSH brush, nbrush, brushR, nbrushR;
	HICON hIcon;
	RECT r = {};
	COLORREF color;

	switch (uMsg)
	{
	case WM_CREATE:
		if (!_bCandidateWindow)
		{
			SetTimer(hWnd, INPUTMODE_TIMER_ID, _pTextService->cx_showmodeinltm, nullptr);
		}
		break;
	case WM_TIMER:
		if (wParam == INPUTMODE_TIMER_ID)
		{
			// CAUTION!! killing self
			_pTextService->_EndInputModeWindow();
		}
		break;
	case WM_DESTROY:
		if (!_bCandidateWindow)
		{
			KillTimer(hWnd, INPUTMODE_TIMER_ID);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &r);

		hmemdc = CreateCompatibleDC(hdc);
		hmembmp = CreateCompatibleBitmap(hdc, r.right, r.bottom);
		bmp = (HBITMAP)SelectObject(hmemdc, hmembmp);

		npen = CreatePen(PS_SOLID, 1, _pTextService->cx_mode_colors[CL_COLOR_MF]);
		pen = (HPEN)SelectObject(hmemdc, npen);

		color = RGB(0xFF, 0xFF, 0xFF);
		switch (_pTextService->inputmode)
		{
		case im_direct:
			color = _pTextService->cx_mode_colors[CL_COLOR_DR];
			break;
		case im_hiragana:
			color = _pTextService->cx_mode_colors[CL_COLOR_HR];
			break;
		case im_katakana:
			color = _pTextService->cx_mode_colors[CL_COLOR_KT];
			break;
		case im_katakana_ank:
			color = _pTextService->cx_mode_colors[CL_COLOR_KA];
			break;
		case im_jlatin:
			color = _pTextService->cx_mode_colors[CL_COLOR_JL];
			break;
		case im_ascii:
			color = _pTextService->cx_mode_colors[CL_COLOR_AC];
			break;
		default:
			break;
		}
		nbrush = CreateSolidBrush(color);
		brush = (HBRUSH)SelectObject(hmemdc, nbrush);

		Rectangle(hmemdc, 0, 0, r.right, r.bottom);

		SelectObject(hmemdc, pen);
		SelectObject(hmemdc, brush);

		DeleteObject(npen);
		DeleteObject(nbrush);

		hmemdcR = CreateCompatibleDC(hdc);
		hmembmpR = CreateCompatibleBitmap(hdc, r.right, r.bottom);
		bmpR = (HBITMAP)SelectObject(hmemdcR, hmembmpR);

		penR = (HPEN)SelectObject(hmemdcR, GetStockObject(WHITE_PEN));
		brushR = (HBRUSH)SelectObject(hmemdcR, GetStockObject(WHITE_BRUSH));

		Rectangle(hmemdcR, 0, 0, r.right, r.bottom);

		_pTextService->_GetIcon(&hIcon, MulDiv(16, _dpi, C_USER_DEFAULT_SCREEN_DPI));
		DrawIconEx(hmemdcR, IM_MARGIN_X, IM_MARGIN_Y, hIcon, _size, _size, 0, (HBRUSH)GetStockObject(WHITE_BRUSH), DI_NORMAL);

		SelectObject(hmemdcR, penR);
		SelectObject(hmemdcR, brushR);

		npenR = CreatePen(PS_SOLID, 1, _pTextService->cx_mode_colors[CL_COLOR_MC]);
		penR = (HPEN)SelectObject(hmemdcR, npenR);
		nbrushR = CreateSolidBrush(_pTextService->cx_mode_colors[CL_COLOR_MC]);
		brushR = (HBRUSH)SelectObject(hmemdcR, nbrushR);

		SetROP2(hmemdcR, R2_XORPEN);

		Rectangle(hmemdcR, 0, 0, r.right, r.bottom);

		SelectObject(hmemdcR, penR);
		SelectObject(hmemdcR, brushR);

		DeleteObject(npenR);
		DeleteObject(nbrushR);

		GdiTransparentBlt(hmemdc, 0, 0, r.right, r.bottom, hmemdcR, 0, 0, r.right, r.bottom,
			(_pTextService->cx_mode_colors[CL_COLOR_MC] ^ RGB(0xFF, 0xFF, 0xFF)));

		SelectObject(hmemdcR, bmpR);

		DeleteObject(hmembmpR);
		DeleteObject(hmemdcR);

		BitBlt(hdc, 0, 0, r.right, r.bottom, hmemdc, 0, 0, SRCCOPY);

		SelectObject(hmemdc, bmp);

		DeleteObject(hmembmp);
		DeleteObject(hmemdc);

		EndPaint(hWnd, &ps);
		break;
	case WM_DPICHANGED:
		_dpi = HIWORD(wParam);
		_size = MulDiv(16, _dpi, C_USER_DEFAULT_SCREEN_DPI);
		SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0,
			_size + IM_MARGIN_X * 2, _size + IM_MARGIN_Y * 2, SWP_NOACTIVATE | SWP_NOMOVE);
		_Redraw();
		break;
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	default:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void CInputModeWindow::_Destroy()
{
	if (_hwnd != nullptr)
	{
		DestroyWindow(_hwnd);
		_hwnd = nullptr;
	}

	_UnadviseTextLayoutSink();
	_pContext.Release();

	_pTextService.Release();
}

void CInputModeWindow::_Move(int x, int y)
{
	if (_hwnd != nullptr)
	{
		SetWindowPos(_hwnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

void CInputModeWindow::_Show(BOOL bShow)
{
	if (_hwnd != nullptr)
	{
		SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | (bShow ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
	}
}

void CInputModeWindow::_Redraw()
{
	if (_hwnd != nullptr)
	{
		InvalidateRect(_hwnd, nullptr, FALSE);
		UpdateWindow(_hwnd);
	}
}

void CInputModeWindow::_GetRect(LPRECT lpRect)
{
	if (lpRect != nullptr)
	{
		*lpRect = RECT{};
		if (_hwnd != nullptr)
		{
			GetClientRect(_hwnd, lpRect);
		}
	}
}

class CInputModeWindowEditSession : public CEditSessionBase
{
public:
	CInputModeWindowEditSession(CTextService *pTextService, ITfContext *pContext,
		CInputModeWindow *pInputModeWindow) : CEditSessionBase(pTextService, pContext)
	{
		_pInputModeWindow = pInputModeWindow;
	}

	~CInputModeWindowEditSession()
	{
		_pInputModeWindow.Release();
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		HRESULT hr;

		CComPtr<ITfContextView> pContextView;
		if (SUCCEEDED(_pContext->GetActiveView(&pContextView)) && (pContextView != nullptr))
		{
			try
			{
				CComPtr<ITfEditSession> pEditSession;
				pEditSession.Attach(
					new CInputModeWindowGetTextExtEditSession(_pTextService, _pContext, pContextView, _pInputModeWindow));
				_pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
			}
			catch (...)
			{
			}
		}

		return S_OK;
	}

private:
	CComPtr<CInputModeWindow> _pInputModeWindow;
};

void CTextService::_StartInputModeWindow()
{
	switch (inputmode)
	{
	case im_direct:
	case im_hiragana:
	case im_katakana:
	case im_katakana_ank:
	case im_jlatin:
	case im_ascii:
		{
			_EndInputModeWindow();

			CComPtr<ITfDocumentMgr> pDocumentMgr;
			if (SUCCEEDED(_pThreadMgr->GetFocus(&pDocumentMgr)) && (pDocumentMgr != nullptr))
			{
				CComPtr<ITfContext> pContext;
				if (SUCCEEDED(pDocumentMgr->GetTop(&pContext)) && (pContext != nullptr))
				{
					try
					{
						_pInputModeWindow.Attach(new CInputModeWindow());

						if (_pInputModeWindow->_Create(this, pContext, FALSE, nullptr))
						{
							HRESULT hr = E_FAIL;
							HRESULT hrSession = E_FAIL;

							CComPtr<ITfEditSession> pEditSession;
							pEditSession.Attach(
								new CInputModeWindowEditSession(this, pContext, _pInputModeWindow));
							hr = pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READ, &hrSession);

							if (FAILED(hr) || FAILED(hrSession))
							{
								_EndInputModeWindow();
							}
						}
					}
					catch (...)
					{
						_EndInputModeWindow();
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

void CTextService::_EndInputModeWindow()
{
	if (_pInputModeWindow != nullptr)
	{
		_pInputModeWindow->_Destroy();
	}
	_pInputModeWindow.Release();
}
