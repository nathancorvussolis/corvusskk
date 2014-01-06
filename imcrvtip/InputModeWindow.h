
#ifndef INPUTMODEWINDOW_H
#define INPUTMODEWINDOW_H

#include "TextService.h"

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

#endif //INPUTMODEWINDOW_H
