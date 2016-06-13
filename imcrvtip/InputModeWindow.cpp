
#include "imcrvtip.h"
#include "TextService.h"
#include "InputModeWindow.h"

#define INPUTMODE_TIMER_ID		0x54ab516b

class CInputModeWindowGetTextExtEditSession : public CEditSessionBase
{
public:
	CInputModeWindowGetTextExtEditSession(CTextService *pTextService, ITfContext *pContext,
		ITfContextView *pContextView, CInputModeWindow *pInputModeWindow) : CEditSessionBase(pTextService, pContext)
	{
		_pInputModeWindow = pInputModeWindow;
		_pInputModeWindow->AddRef();
		_pContextView = pContextView;
		_pContextView->AddRef();
	}

	~CInputModeWindowGetTextExtEditSession()
	{
		SafeRelease(&_pInputModeWindow);
		SafeRelease(&_pContextView);
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		RECT rs = {-1, -1, -1, -1};
		if(_pContextView->GetScreenExt(&rs) == S_OK &&
			(rs.left == 0 && rs.top == 0 && rs.right == 0 && rs.bottom == 0))
		{
			_pInputModeWindow->_Show(FALSE);
			return FALSE;
		}

		TF_SELECTION tfSelection;
		ULONG cFetched = 0;
		if(_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK)
		{
			return E_FAIL;
		}

		if(cFetched != 1)
		{
			SafeRelease(&tfSelection.range);
			return E_FAIL;
		}

		RECT rc;
		BOOL fClipped;
		if(_pContextView->GetTextExt(ec, tfSelection.range, &rc, &fClipped) == S_OK)
		{
			POINT pt;
			pt.x = rc.left;
			pt.y = rc.bottom;
			HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfoW(hMonitor, &mi);

			RECT rw;
			_pInputModeWindow->_GetRect(&rw);
			LONG height = rw.bottom - rw.top;
			LONG width = rw.right - rw.left;

			if(rc.left < mi.rcWork.left)
			{
				rc.left = mi.rcWork.left;
			}

			if(mi.rcWork.right < rc.right)
			{
				rc.left = mi.rcWork.right - width;
			}

			if(mi.rcWork.bottom < rc.top)
			{
				rc.bottom = mi.rcWork.bottom - height - IM_MERGIN_Y;
			}
			else if(mi.rcWork.bottom < (rc.bottom + height + IM_MERGIN_Y))
			{
				rc.bottom = rc.top - height - IM_MERGIN_Y * 2;
			}

			if(rc.bottom < mi.rcWork.top)
			{
				rc.bottom = mi.rcWork.top - IM_MERGIN_Y;
			}

			_pInputModeWindow->_Move(rc.left, rc.bottom + IM_MERGIN_Y);
			_pInputModeWindow->_Show(TRUE);
		}

		SafeRelease(&tfSelection.range);

		return S_OK;
	}

private:
	ITfContextView *_pContextView;
	CInputModeWindow *_pInputModeWindow;
};

CInputModeWindow::CInputModeWindow()
{
	DllAddRef();

	_cRef = 1;

	_hwnd = nullptr;
	_hwndParent = nullptr;
	_pTextService = nullptr;
	_pContext = nullptr;
	_size = 0;
}

CInputModeWindow::~CInputModeWindow()
{
	_Destroy();

	DllRelease();
}

STDAPI CInputModeWindow::QueryInterface(REFIID riid, void **ppvObj)
{
	if(ppvObj == nullptr)
	{
		return E_INVALIDARG;
	}

	*ppvObj = nullptr;

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
		try
		{
			CInputModeWindowGetTextExtEditSession *pEditSession =
				new CInputModeWindowGetTextExtEditSession(_pTextService, pContext, pContextView, this);
			pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
			SafeRelease(&pEditSession);
		}
		catch(...)
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

	ITfSource *pSource;
	if(_pContext->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
	{
		hr = pSource->AdviseSink(IID_IUNK_ARGS((ITfTextLayoutSink *)this), &_dwCookieTextLayoutSink);
		SafeRelease(&pSource);
	}

	return hr;
}

HRESULT CInputModeWindow::_UnadviseTextLayoutSink()
{
	HRESULT hr = E_FAIL;

	if(_pContext != nullptr)
	{
		ITfSource *pSource;
		if(_pContext->QueryInterface(IID_PPV_ARGS(&pSource)) == S_OK)
		{
			hr = pSource->UnadviseSink(_dwCookieTextLayoutSink);
			SafeRelease(&pSource);
		}
	}

	return hr;
}

BOOL CInputModeWindow::_Create(CTextService *pTextService, ITfContext *pContext, BOOL bCandidateWindow, HWND hWnd)
{
	POINT pt = {0, 0};

	if(pContext != nullptr)
	{
		_pContext = pContext;
		_pContext->AddRef();
		if(_AdviseTextLayoutSink() != S_OK)
		{
			return FALSE;
		}
	}

	if(!bCandidateWindow && _pContext == nullptr)
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
		ITfContextView *pContextView;
		if(_pContext->GetActiveView(&pContextView) == S_OK)
		{
			RECT rs = {-1, -1, -1, -1};
			if(pContextView->GetScreenExt(&rs) == S_OK &&
				(rs.left == 0 && rs.top == 0 && rs.right == 0 && rs.bottom == 0))
			{
				SafeRelease(&pContextView);
				return FALSE;
			}

			if(FAILED(pContextView->GetWnd(&_hwndParent)) || _hwndParent == nullptr)
			{
				_hwndParent = GetFocus();
			}
			SafeRelease(&pContextView);
		}
	}

	_hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		InputModeWindowClass, L"", WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		_hwndParent, nullptr, g_hInst, this);

	if(_hwnd == nullptr)
	{
		return FALSE;
	}

	HDC hdc = GetDC(nullptr);
	_size = MulDiv(16, GetDeviceCaps(hdc, LOGPIXELSY), 96);
	ReleaseDC(nullptr, hdc);

	if(_bCandidateWindow)
	{
		RECT r;
		GetClientRect(_hwndParent, &r);
		pt.x = r.left;
		pt.y = r.bottom;
		ClientToScreen(_hwndParent, &pt);
	}

	SetWindowPos(_hwnd, HWND_TOPMOST, pt.x, pt.y + IM_MERGIN_Y,
		_size + IM_MERGIN_X * 2, _size + IM_MERGIN_Y * 2, SWP_NOACTIVATE);

	return TRUE;
}

BOOL CInputModeWindow::_InitClass()
{
	WNDCLASSEXW wcex;

	ZeroMemory(&wcex, sizeof(wcex));
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

	switch(uMsg)
	{
	case WM_NCCREATE:
		pInputModeWindow = (CInputModeWindow *)((LPCREATESTRUCTW)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pInputModeWindow);
		break;
	default:
		pInputModeWindow = (CInputModeWindow *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		break;
	}

	if(pInputModeWindow != nullptr)
	{
		return pInputModeWindow->_WindowProc(hWnd, uMsg, wParam, lParam);
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
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
	case WM_CREATE:
		if(!_bCandidateWindow)
		{
			SetTimer(hWnd, INPUTMODE_TIMER_ID, _pTextService->cx_showmodesec * 1000, nullptr);
		}
		break;
	case WM_TIMER:
		if(wParam == INPUTMODE_TIMER_ID)
		{
			// CAUTION!! killing self
			_pTextService->_EndInputModeWindow();
		}
		break;
	case WM_DESTROY:
		if(!_bCandidateWindow)
		{
			KillTimer(hWnd, INPUTMODE_TIMER_ID);
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
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void CInputModeWindow::_Destroy()
{
	if(_hwnd != nullptr)
	{
		DestroyWindow(_hwnd);
		_hwnd = nullptr;
	}

	_UnadviseTextLayoutSink();
	SafeRelease(&_pContext);

	SafeRelease(&_pTextService);
}

void CInputModeWindow::_Move(int x, int y)
{
	if(_hwnd != nullptr)
	{
		SetWindowPos(_hwnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

void CInputModeWindow::_Show(BOOL bShow)
{
	if(_hwnd != nullptr)
	{
		SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | (bShow ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
	}
}

void CInputModeWindow::_Redraw()
{
	if(_hwnd != nullptr)
	{
		InvalidateRect(_hwnd, nullptr, FALSE);
		UpdateWindow(_hwnd);
	}
}

void CInputModeWindow::_GetRect(LPRECT lpRect)
{
	if(lpRect != nullptr)
	{
		ZeroMemory(lpRect, sizeof(*lpRect));
		if(_hwnd != nullptr)
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
		_pInputModeWindow->AddRef();
	}

	~CInputModeWindowEditSession()
	{
		SafeRelease(&_pInputModeWindow);
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		HRESULT hr;

		ITfContextView *pContextView;
		if(_pContext->GetActiveView(&pContextView) == S_OK)
		{
			try
			{
				CInputModeWindowGetTextExtEditSession *pEditSession =
					new CInputModeWindowGetTextExtEditSession(_pTextService, _pContext, pContextView, _pInputModeWindow);
				_pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
				SafeRelease(&pEditSession);
			}
			catch(...)
			{
			}

			SafeRelease(&pContextView);
		}

		return S_OK;
	}

private:
	CInputModeWindow *_pInputModeWindow;
};

void CTextService::_StartInputModeWindow()
{
	switch(inputmode)
	{
	case im_default:
	case im_hiragana:
	case im_katakana:
	case im_katakana_ank:
	case im_jlatin:
	case im_ascii:
		_EndInputModeWindow();

		ITfDocumentMgr *pDocumentMgr;
		if((_pThreadMgr->GetFocus(&pDocumentMgr) == S_OK) && (pDocumentMgr != nullptr))
		{
			ITfContext *pContext;
			if((pDocumentMgr->GetTop(&pContext) == S_OK) && (pContext != nullptr))
			{
				try
				{
					_pInputModeWindow = new CInputModeWindow();

					if(_pInputModeWindow->_Create(this, pContext, FALSE, nullptr))
					{
						HRESULT hr = E_FAIL;
						HRESULT hrSession = E_FAIL;

						try
						{
							CInputModeWindowEditSession *pEditSession = new CInputModeWindowEditSession(this, pContext, _pInputModeWindow);
							// Asynchronous, read-only
							hr = pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_ASYNC | TF_ES_READ, &hrSession);
							SafeRelease(&pEditSession);

							// It is possible that asynchronous requests are treated as synchronous requests.
							if(hr != S_OK || (hrSession != TF_S_ASYNC && hrSession != S_OK))
							{
								_EndInputModeWindow();
							}
						}
						catch(...)
						{
						}
					}
				}
				catch(...)
				{
				}

				SafeRelease(&pContext);
			}

			SafeRelease(&pDocumentMgr);
		}
		break;
	default:
		break;
	}
}

void CTextService::_EndInputModeWindow()
{
	if(_pInputModeWindow != nullptr)
	{
		_pInputModeWindow->_Destroy();
	}
	SafeRelease(&_pInputModeWindow);
}
