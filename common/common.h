
#ifndef COMMON_H
#define COMMON_H

#define TEXTSERVICE_NAME	L"CorvusSKK"
#define TEXTSERVICE_VER		L"0.7.1"

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

//for resource
#define RC_AUTHOR			"Corvus Solis"
#define RC_PRODUCT			"CorvusSKK"
#define RC_VERSION			"0.7.1"
#define RC_VERSION_D		0,7,1,0

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

extern LPCWSTR RccsUNICODE;
extern LPCWSTR WccsUNICODE;

extern LPCWSTR fnconfigxml;
extern LPCWSTR fnuserdicxml;
extern LPCWSTR fnskkcvdicxml;
extern LPCWSTR fnskkcvdicidx;

typedef struct {
	BYTE digest[16];
} MD5_DIGEST;

void debugout(LPCWSTR format, ...);
BOOL GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen);
BOOL IsVersion6AndOver(OSVERSIONINFOW ovi);
BOOL IsVersion62AndOver(OSVERSIONINFOW ovi);
BOOL SetFileDaclAC(LPCWSTR path);

#endif
