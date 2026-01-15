
#include "imcrvtip.h"
#include "TextService.h"
#include "LanguageBar.h"
#include "CandidateList.h"
#include "CandidateWindow.h"
#include "InputModeWindow.h"

CTextService::CTextService()
{
	DllAddRef();

	_cRef = 1;

	_pThreadMgr = nullptr;
	_ClientId = TF_CLIENTID_NULL;
	_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
	_dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
	_dwCompartmentEventSinkOpenCloseCookie = TF_INVALID_COOKIE;
	_dwCompartmentEventSinkInputmodeConversionCookie = TF_INVALID_COOKIE;
	_pTextEditSinkContext = nullptr;
	_dwTextEditSinkCookie = TF_INVALID_COOKIE;
	_pComposition = nullptr;
	_pLangBarItem = nullptr;
	_pLangBarItemI = nullptr;
	_pCandidateList = nullptr;
	_pInputModeWindow = nullptr;

	_gaDisplayAttributeInputMark = 0;
	_gaDisplayAttributeInputText = 0;
	_gaDisplayAttributeInputOkuri = 0;
	_gaDisplayAttributeConvMark = 0;
	_gaDisplayAttributeConvText = 0;
	_gaDisplayAttributeConvOkuri = 0;
	_gaDisplayAttributeConvAnnot = 0;

	_pD2DFactory = nullptr;
	_pD2DDCRT = nullptr;
	for (int i = 0; i < DISPLAY_LIST_COLOR_NUM; i++)
	{
		_pD2DBrush[i] = nullptr;
	}
	_drawtext_option = D2D1_DRAW_TEXT_OPTIONS_NONE;
	_pDWFactory = nullptr;

	_ImmersiveMode = FALSE;
	_UILessMode = FALSE;
	_AppPrivateMode = FALSE;
	_UserPrivateMode = E_FAIL;

	hPipe = INVALID_HANDLE_VALUE;

	inputmode = im_direct;

	ZeroMemory(preservedkey, sizeof(preservedkey));
	ZeroMemory(&privatemodekey, sizeof(privatemodekey));

	_ResetStatus();

	_CreateConfigPath();
	_CreateIpcName();
}

CTextService::~CTextService()
{
	DllRelease();
}

STDAPI CTextService::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_INVALIDARG;
	}

	*ppvObj = nullptr;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor))
	{
		*ppvObj = static_cast<ITfTextInputProcessor *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx))
	{
		*ppvObj = static_cast<ITfTextInputProcessorEx *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
	{
		*ppvObj = static_cast<ITfThreadMgrEventSink *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
	{
		*ppvObj = static_cast<ITfThreadFocusSink *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfCompartmentEventSink))
	{
		*ppvObj = static_cast<ITfCompartmentEventSink *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfTextEditSink))
	{
		*ppvObj = static_cast<ITfTextEditSink *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfKeyEventSink))
	{
		*ppvObj = static_cast<ITfKeyEventSink *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfCompositionSink))
	{
		*ppvObj = static_cast<ITfKeyEventSink *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfDisplayAttributeProvider))
	{
		*ppvObj = static_cast<ITfDisplayAttributeProvider *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfFunctionProvider))
	{
		*ppvObj = static_cast<ITfFunctionProvider *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfFnConfigure))
	{
		*ppvObj = static_cast<ITfFnConfigure *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfFnShowHelp))
	{
		*ppvObj = static_cast<ITfFnShowHelp *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfFnReconversion))
	{
		*ppvObj = static_cast<ITfFnReconversion *>(this);
	}
	else if (IsEqualIID(riid, IID_ITfFnGetPreferredTouchKeyboardLayout))
	{
		*ppvObj = static_cast<ITfFnGetPreferredTouchKeyboardLayout *>(this);
	}

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextService::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) CTextService::Release()
{
	if (--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CTextService::Activate(ITfThreadMgr *ptim, TfClientId tid)
{
	return ActivateEx(ptim, tid, 0);
}

STDAPI CTextService::ActivateEx(ITfThreadMgr *ptim, TfClientId tid, DWORD dwFlags)
{
	_pThreadMgr = ptim;
	_ClientId = tid;

	if (!_IsKeyboardOpen())
	{
		_KeyboardSetDefaultMode();
	}

	if (!_InitThreadMgrEventSink())
	{
		goto exit;
	}

	if (!_InitThreadFocusSink())
	{
		goto exit;
	}

	if (!_InitCompartmentEventSink())
	{
		goto exit;
	}

	{
		CComPtr<ITfDocumentMgr> pDocumentMgr;
		if (SUCCEEDED(_pThreadMgr->GetFocus(&pDocumentMgr)) && (pDocumentMgr != nullptr))
		{
			_InitTextEditSink(pDocumentMgr);
		}
	}

	if (!_InitLanguageBar())
	{
		goto exit;
	}

	if (!_InitKeyEventSink())
	{
		goto exit;
	}

	if (!_InitDisplayAttributeGuidAtom())
	{
		// some applications do not support ITfCategoryMgr interface.
		//goto exit;
	}

	if (!_InitFunctionProvider())
	{
		goto exit;
	}

	_KeyboardOpenCloseChanged(FALSE);

	return S_OK;

exit:
	Deactivate();
	return E_FAIL;
}

STDAPI CTextService::Deactivate()
{
	if (_pThreadMgr == nullptr)
	{
		return S_OK;
	}

	_SaveUserDic();

	_EndCandidateList();

	_EndInputModeWindow();

	_UninitFunctionProvider();

	_UninitPreservedKey(0);
	_UninitPreservedKey(1);
	_UninitPrivateModeKey(0);
	_UninitPrivateModeKey(1);

	_UninitKeyEventSink();

	_UninitLanguageBar();

	_InitTextEditSink(nullptr);

	_UninitCompartmentEventSink();

	_UninitThreadFocusSink();

	_UninitThreadMgrEventSink();

	_UninitD2D();

	_pThreadMgr.Release();
	_ClientId = TF_CLIENTID_NULL;

	return S_OK;
}
