
#include "imcrvtip.h"
#include "TextService.h"
#include "InputModeWindow.h"

#define IMPUTMODE_TIMER_ID		0x54ab516b
#define IMPUTMODE_TIMEOUT_MSEC	3000

CInputModeWindow::CInputModeWindow()
{
	DllAddRef();

	_cRef = 1;

	_hwnd = NULL;
	_hwndParent = NULL;
	_pTextService = NULL;
	_pContext = NULL;
	_size = 0;

	_term = FALSE;
}

CInputModeWindow::~CInputModeWindow()
{
	_Destroy();

	DllRelease();
}

STDAPI CInputModeWindow::QueryInterface(REFIID riid, void **ppvObj)
{
	if(ppvObj == NULL)
	{
		return E_INVALIDARG;
	}

	*ppvObj = NULL;

	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextLayoutSink))
	{
		*ppvObj = (ITfTextLayoutSink *)this;
	}

	if(*ppvObj)
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
	if(--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CInputModeWindow::OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pContextView)
{
	CIMGetTextExtEditSession *pEditSession;
	HRESULT hr;

	if(pContext != _pContext)
	{
		return S_OK;
	}

	switch(lcode)
	{
	case TF_LC_CREATE:
		break;

	case TF_LC_CHANGE:
		pEditSession = new CIMGetTextExtEditSession(_pTextService, pContext, pContextView, this);
		if(pEditSession != NULL)
		{
			pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
			pEditSession->Release();
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
	ITfSource *pSource;
	HRESULT hr = E_FAIL;

	if(_pContext->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
	{
		hr = pSource->AdviseSink(IID_IUNK_ARGS((ITfTextLayoutSink *)this), &_dwCookieTextLayoutSink);
		pSource->Release();
	}

	return hr;
}

HRESULT CInputModeWindow::_UnadviseTextLayoutSink()
{
	ITfSource *pSource;
	HRESULT hr = E_FAIL;

	if(_pContext != NULL)
	{
		if(_pContext->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
		{
			hr = pSource->UnadviseSink(_dwCookieTextLayoutSink);
			pSource->Release();
		}
	}

	return hr;
}

BOOL CInputModeWindow::_Create(CTextService *pTextService, ITfContext *pContext, BOOL bCandidateWindow, HWND hWnd)
{
	WNDCLASSEXW wc;
	HDC hdc;
	RECT r;
	POINT pt = {0, 0};
	ITfContextView *pContextView;

	if(pContext != NULL)
	{
		_pContext = pContext;
		_pContext->AddRef();
		if(_AdviseTextLayoutSink() != S_OK)
		{
			return FALSE;
		}
	}

	if(!bCandidateWindow && _pContext == NULL)
	{
		return FALSE;
	}

	_pTextService = pTextService;
	_pTextService->AddRef();

	_bCandidateWindow = bCandidateWindow;

	if(_bCandidateWindow)
	{
		_hwndParent = hWnd;
	}
	else
	{
		if(_pContext->GetActiveView(&pContextView) == S_OK)
		{
			if(FAILED(pContextView->GetWnd(&_hwndParent)) || _hwndParent == NULL)
			{
				_hwndParent = GetFocus();
			}
			pContextView->Release();
		}
	}

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

	_hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, TextServiceDesc,
		NULL, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
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
		if(!_bCandidateWindow)
		{
			SetTimer(_hwnd, IMPUTMODE_TIMER_ID, IMPUTMODE_TIMEOUT_MSEC, NULL);
		}
	}

	hdc = GetDC(NULL);
	_size = MulDiv(16, GetDeviceCaps(hdc, LOGPIXELSX), 96);
	ReleaseDC(NULL, hdc);

	if(_bCandidateWindow)
	{
		GetClientRect(_hwndParent, &r);
		pt.x = r.left;
		pt.y = r.bottom;
		ClientToScreen(_hwndParent, &pt);
	}

	SetWindowPos(_hwnd, HWND_TOPMOST, pt.x, pt.y + IM_MERGIN_Y,
		_size + IM_MERGIN_X * 2, _size + IM_MERGIN_Y * 2, SWP_NOACTIVATE);

	return TRUE;
}

LRESULT CALLBACK CInputModeWindow::_WindowPreProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	CInputModeWindow *pWindowProc = (CInputModeWindow*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
	if(pWindowProc != NULL)
	{
		ret = pWindowProc->_WindowProc(hWnd, uMsg, wParam, lParam);
	}
	return ret;
}

LRESULT CALLBACK CInputModeWindow::_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HDC hmemdc;
	HBITMAP hmembmp;
	HPEN npen;
	HBRUSH nbrush;
	HGDIOBJ bmp, pen, brush;
	HICON hIcon;
	RECT r;

	switch(uMsg)
	{
	case WM_TIMER:
		if(wParam == IMPUTMODE_TIMER_ID)
		{
			// CAUTION! killing self
			_pTextService->_ClearComposition();
		}
		break;
	case WM_DESTROY:
		if(!_bCandidateWindow)
		{
			KillTimer(hWnd, IMPUTMODE_TIMER_ID);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &r);

		hmemdc = CreateCompatibleDC(hdc);
		hmembmp = CreateCompatibleBitmap(hdc, r.right, r.bottom);
		bmp = SelectObject(hmemdc, hmembmp);

		npen = CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0x00));
		pen = SelectObject(hmemdc, npen);
		nbrush = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
		brush = SelectObject(hmemdc, nbrush);

		Rectangle(hmemdc, 0, 0, r.right, r.bottom);

		_pTextService->_GetIcon(&hIcon);
		DrawIconEx(hmemdc, IM_MERGIN_X, IM_MERGIN_Y, hIcon, _size, _size, 0, nbrush, DI_NORMAL);

		SelectObject(hmemdc, pen);
		SelectObject(hmemdc, brush);

		DeleteObject(npen);
		DeleteObject(nbrush);

		BitBlt(hdc, 0, 0, r.right, r.bottom, hmemdc, 0, 0, SRCCOPY);

		SelectObject(hmemdc, bmp);

		DeleteObject(hmembmp);
		DeleteObject(hmemdc);

		EndPaint(hWnd, &ps);
		break;
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void CInputModeWindow::_Destroy()
{
	if(_hwnd != NULL)
	{
		DestroyWindow(_hwnd);
		_hwnd = NULL;
	}

	if(_pContext != NULL)
	{
		_UnadviseTextLayoutSink();
		_pContext->Release();
		_pContext = NULL;
	}

	if(_pTextService != NULL)
	{
		_pTextService->Release();
		_pTextService = NULL;
	}
}

void CInputModeWindow::_Move(int x, int y)
{
	if(_hwnd != NULL)
	{
		RECT rc;
		GetWindowRect(_hwnd, &rc);
		MoveWindow(_hwnd, x, y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}
}

void CInputModeWindow::_Show(BOOL bShow)
{
	ShowWindow(_hwnd, (bShow ? SW_SHOWNA : SW_HIDE));
}

void CInputModeWindow::_Redraw()
{
	if(_hwnd != NULL)
	{
		InvalidateRect(_hwnd, NULL, FALSE);
		UpdateWindow(_hwnd);
	}
}

void CInputModeWindow::_GetRect(LPRECT lpRect)
{
	if(lpRect != NULL)
	{
		ZeroMemory(lpRect, sizeof(*lpRect));
		if(_hwnd != NULL)
		{
			GetClientRect(_hwnd, lpRect);
		}
	}
}
