
#include "input.h"

// input.dll dummy functions

UINT WINAPI EnumEnabledLayoutOrTip(
	_In_opt_ LPCWSTR            pszUserReg,
	_In_opt_ LPCWSTR            pszSystemReg,
	_In_opt_ LPCWSTR            pszSoftwareReg,
	_Out_    LAYOUTORTIPPROFILE *pLayoutOrTipProfile,
	_In_     UINT               uBufLength
) {
	return 0;
}

UINT WINAPI EnumLayoutOrTipForSetup(
	_In_  LANGID      langid,
	_Out_ LAYOUTORTIP *pLayoutOrTip,
	_In_  UINT        uBufLength,
	_In_  DWORD       dwFlags
) {
	return 0;
}

BOOL WINAPI InstallLayoutOrTip(
	_In_ LPCWSTR psz,
	_In_ DWORD   dwFlags
) {
	return TRUE;
}

BOOL WINAPI InstallLayoutOrTipUserReg(
	_In_opt_ LPCWSTR pszUserReg,
	_In_opt_ LPCWSTR pszSystemReg,
	_In_opt_ LPCWSTR pszSoftwareReg,
	_In_     LPCWSTR psz,
	_In_     DWORD   dwFlags
) {
	return TRUE;
}

HRESULT WINAPI QueryLayoutOrTipString(
	_In_ LPCWSTR psz,
	_In_ DWORD   dwFlags
) {
	return S_OK;
}

HRESULT WINAPI QueryLayoutOrTipStringUserReg(
	_In_ LPCWSTR pszUserReg,
	_In_ LPCWSTR pszSystemReg,
	_In_ LPCWSTR pszSoftwareReg,
	_In_ LPCWSTR psz,
	_In_ DWORD   dwFlags
) {
	return S_OK;
}

BOOL WINAPI SaveDefaultUserInputSettings(
	_In_ HWND hwndParent,
	_In_ HKEY hSourceRegKey
) {
	return TRUE;
}

BOOL WINAPI SaveSystemAcctInputSettings(
	_In_ HWND hwndParent,
	_In_ HKEY hSourceRegKey
) {
	return TRUE;
}

BOOL WINAPI SetDefaultLayoutOrTip(
	_In_ LPCWSTR psz,
	_In_ DWORD   dwFlags
) {
	return TRUE;
}

BOOL WINAPI SetDefaultLayoutOrTipUserReg(
	_In_opt_ LPCWSTR pszUserReg,
	_In_opt_ LPCWSTR pszSystemReg,
	_In_opt_ LPCWSTR pszSoftwareReg,
	_In_     LPCWSTR psz,
	_In_     DWORD   dwFlags
) {
	return TRUE;
}
