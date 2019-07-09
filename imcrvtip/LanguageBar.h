#pragma once

class CLangBarItemButton :
	public ITfLangBarItemButton,
	public ITfSource
{
public:
	CLangBarItemButton(CTextService *pTextService, REFGUID guid);
	~CLangBarItemButton();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITfLangBarItem
	STDMETHODIMP GetInfo(TF_LANGBARITEMINFO *pInfo);
	STDMETHODIMP GetStatus(DWORD *pdwStatus);
	STDMETHODIMP Show(BOOL fShow);
	STDMETHODIMP GetTooltipString(BSTR *pbstrToolTip);

	// ITfLangBarItemButton
	STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT *prcArea);
	STDMETHODIMP InitMenu(ITfMenu *pMenu);
	STDMETHODIMP OnMenuSelect(UINT wID);
	STDMETHODIMP GetIcon(HICON *phIcon);
	STDMETHODIMP GetText(BSTR *pbstrText);

	// ITfSource
	STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie);
	STDMETHODIMP UnadviseSink(DWORD dwCookie);

	HRESULT _Update();
	HRESULT _GetIcon(HICON *phIcon, INT size, BOOL bNT62);

private:
	LONG _cRef;

	CComPtr<CTextService> _pTextService;

	CComPtr<ITfLangBarItemSink> _pLangBarItemSink;
	TF_LANGBARITEMINFO _LangBarItemInfo;
};
