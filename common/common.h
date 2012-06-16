
#ifndef COMMON_H
#define COMMON_H

#define TEXTSERVICE_NAME	L"CorvusSKK"
#define TEXTSERVICE_VER		L"0.7.3"

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

//for resource
#define RC_AUTHOR			"Corvus Solis"
#define RC_PRODUCT			"CorvusSKK"
#define RC_VERSION			"0.7.3"
#define RC_VERSION_D		0,7,3,0

#define MAX_KRNLOBJNAME		256

#define CORVUSSRVEXE		L"corvussrv.exe"
#define CORVUSCNFEXE		L"corvuscnf.exe"
#ifndef _DEBUG
#define CORVUSKRNLOBJ		L"corvus-skk-"
#else
#define CORVUSKRNLOBJ		L"corvus-skk-debug-"
#endif
#define CORVUSSRVMUTEX		CORVUSKRNLOBJ L"srv-"
#define CORVUSCNFMUTEX		CORVUSKRNLOBJ L"cnf-"
#define CORVUSSRVPIPE		L"\\\\.\\pipe\\" CORVUSKRNLOBJ

typedef struct {
	BYTE digest[16];
} MD5_DIGEST;

extern LPCWSTR RccsUNICODE;
extern LPCWSTR WccsUNICODE;

extern LPCWSTR fnconfigxml;
extern LPCWSTR fnuserdicxml;
extern LPCWSTR fnskkcvdicxml;
extern LPCWSTR fnskkcvdicidx;

// for Windows 8
#if 1
#define TF_TMF_IMMERSIVEMODE          0x40000000

#define TF_IPP_CAPS_IMMERSIVESUPPORT            0x00010000
#define TF_IPP_CAPS_SYSTRAYSUPPORT              0x00020000

extern const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT;
extern const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT;
extern const GUID GUID_COMPARTMENT_TKB_THIRDPARTYIME_MODE_ONOFF;
extern const GUID GUID_LBI_INPUTMODE;
extern const GUID GUID_INTEGRATIONSTYLE_SEARCHBOX;
#endif

void debugout(LPCWSTR format, ...);
BOOL GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen);
BOOL IsVersion6AndOver(OSVERSIONINFOW ovi);
BOOL IsVersion62AndOver(OSVERSIONINFOW ovi);
BOOL SetFileDaclAC(LPCWSTR path);

#endif
