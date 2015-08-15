
#ifndef INPUTMODEWINDOW_H
#define INPUTMODEWINDOW_H

#include "TextService.h"
#include "EditSession.h"

#define IM_MERGIN_X 2
#define IM_MERGIN_Y 2

class CInputModeWindow : public ITfTextLayoutSink
{
public:
	CInputModeWindow();
	~CInputModeWindow();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITfTextLayoutSink
	STDMETHODIMP OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pContextView);

	BOOL _Create(CTextService *pTextService, ITfContext *pContext, BOOL bCandidateWindow, HWND hWnd);
	void _Destroy();
	static LRESULT CALLBACK _WindowPreProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void _Move(int x, int y);
	void _Show(BOOL bShow);
	void _Redraw();
	void _GetRect(LPRECT lpRect);

	BOOL _term;

private:
	LONG _cRef;

	HRESULT _AdviseTextLayoutSink();
	HRESULT _UnadviseTextLayoutSink();

	ITfContext *_pContext;

	DWORD _dwCookieTextLayoutSink;

	CTextService *_pTextService;
	HWND _hwndParent;
	HWND _hwnd;
	WNDPROC WndProcDef;
	BOOL _bCandidateWindow;

	int _size;
};

class CIMGetTextExtEditSession : public CEditSessionBase
{
public:
	CIMGetTextExtEditSession(CTextService *pTextService, ITfContext *pContext, ITfContextView *pContextView, CInputModeWindow *pInputModeWindow) : CEditSessionBase(pTextService, pContext)
	{
		_pContextView = pContextView;
		_pInputModeWindow = pInputModeWindow;
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		ITfComposition *pComposition = _pTextService->_GetComposition();
		if(pComposition != NULL)
		{
			ITfRange *pRange;
			if(pComposition->GetRange(&pRange) == S_OK)
			{
				RECT rc;
				BOOL fClipped;
				if(_pContextView->GetTextExt(ec, pRange, &rc, &fClipped) == S_OK)
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
				SafeRelease(&pRange);
			}
		}
		return S_OK;
	}

private:
	ITfContextView *_pContextView;
	CInputModeWindow *_pInputModeWindow;
};

#endif //INPUTMODEWINDOW_H
