
#ifndef INPUT_H
#define INPUT_H

#include <Windows.h>

EXTERN_C
{

typedef struct tagLAYOUTORTIPPROFILE {
	DWORD  dwProfileType;       // InputProcessor or HKL
#define LOTP_INPUTPROCESSOR 1
#define LOTP_KEYBOARDLAYOUT 2
	LANGID langid;              // language id
	CLSID  clsid;               // CLSID of tip
	GUID   guidProfile;         // profile description
	GUID   catid;               // category of tip
	DWORD  dwSubstituteLayout;  // substitute hkl
	DWORD  dwFlags;             // Flags
	WCHAR  szId[MAX_PATH];      // KLID or TIP profile for string
} LAYOUTORTIPPROFILE;

UINT WINAPI EnumEnabledLayoutOrTip(
	_In_opt_ LPCWSTR            pszUserReg,
	_In_opt_ LPCWSTR            pszSystemReg,
	_In_opt_ LPCWSTR            pszSoftwareReg,
	_Out_    LAYOUTORTIPPROFILE *pLayoutOrTipProfile,
	_In_     UINT               uBufLength
);

typedef struct tagLAYOUTORTIP {
	DWORD dwFlags;
#define LOT_DEFAULT    0x0001 // If this is on, this is a default item.
#define LOT_DISABLED   0x0002 // if this is on, this is not enabled.
	WCHAR szId[MAX_PATH]; // Id of the keyboard item in the string format.
	WCHAR szName[MAX_PATH]; // The description of the keyboard item.
} LAYOUTORTIP;

UINT WINAPI EnumLayoutOrTipForSetup(
	_In_  LANGID      langid,
	_Out_ LAYOUTORTIP *pLayoutOrTip,
	_In_  UINT        uBufLength,
	_In_  DWORD       dwFlags
);

#define ILOT_INSTALL                 0x00000000 // Enables the specified keyboard layout and text service.
#define ILOT_UNINSTALL               0x00000001 // Same as ILOT_DISABLED.
#define ILOT_DEFPROFILE              0x00000002 // Sets the specified layout or tip as a default item.
#define ILOT_DEFUSER4                0x00000004 // Changes the setting of .Default.
#define ILOT_SYSLOCALE               0x00000008 // Unused.
#define ILOT_NOLOCALETOENUMERATE     0x00000010 // Unused.
#define ILOT_NOAPPLYTOCURRENTSESSION 0x00000020 // The setting is saved but is not applied to the current session.
#define ILOT_CLEANINSTALL            0x00000040 // Disables all of the current keyboard layouts and text services.
#define ILOT_DISABLED                0x00000080 // Disables the specified keyboard layout and text service.

BOOL WINAPI InstallLayoutOrTip(
	_In_ LPCWSTR psz,
	_In_ DWORD   dwFlags
);

BOOL WINAPI InstallLayoutOrTipUserReg(
	_In_opt_ LPCWSTR pszUserReg,
	_In_opt_ LPCWSTR pszSystemReg,
	_In_opt_ LPCWSTR pszSoftwareReg,
	_In_     LPCWSTR psz,
	_In_     DWORD   dwFlags
);

HRESULT WINAPI QueryLayoutOrTipString(
	_In_ LPCWSTR psz,
	_In_ DWORD   dwFlags // This must be 0.
);

HRESULT WINAPI QueryLayoutOrTipStringUserReg(
	_In_ LPCWSTR pszUserReg,
	_In_ LPCWSTR pszSystemReg,
	_In_ LPCWSTR pszSoftwareReg,
	_In_ LPCWSTR psz,
	_In_ DWORD   dwFlags // This must be 0.
);

BOOL WINAPI SaveDefaultUserInputSettings(
	_In_ HWND hwndParent,
	_In_ HKEY hSourceRegKey
);

BOOL WINAPI SaveSystemAcctInputSettings(
	_In_ HWND hwndParent,
	_In_ HKEY hSourceRegKey
);

// Stores the setting in the registry but dose not update the runtime keyboard setting of the current session.
// If the alternative registry path is set in SetDefaultLayoutOrTipUserReg, this flag should be set.
#define SDLOT_NOAPPLYTOCURRENTSESSION 0x00000001
// Applies the setting immediately on the current thread.
#define SDLOT_APPLYTOCURRENTTHREAD    0x00000002

BOOL WINAPI SetDefaultLayoutOrTip(
	_In_ LPCWSTR psz,
	_In_ DWORD   dwFlags
);

BOOL WINAPI SetDefaultLayoutOrTipUserReg(
	_In_opt_ LPCWSTR pszUserReg,
	_In_opt_ LPCWSTR pszSystemReg,
	_In_opt_ LPCWSTR pszSoftwareReg,
	_In_     LPCWSTR psz,
	_In_     DWORD   dwFlags
);

}

#endif //INPUT_H
